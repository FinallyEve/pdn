#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "device/drivers/logger.hpp"
#include "game/minigame.hpp"

/*
 * BaseLoseState — Template base class for all minigame defeat screens.
 *
 * Provides common lose logic:
 * - Sets MiniGameOutcome to LOST with score
 * - Displays defeat text (and optionally score)
 * - Starts defeat LED animation (typically red fade)
 * - Triggers defeat haptic feedback (typically max buzz)
 * - Waits for LOSE_DISPLAY_MS then:
 *   - In standalone mode: transitions back to intro
 *   - In managed mode: calls Device::returnToPreviousApp()
 *
 * Template parameters:
 * - GameT: The minigame type (e.g., GhostRunner, SignalEcho)
 * - IntroStateId: The state ID to transition to for standalone replay
 *
 * Derived classes must override:
 * - defeatText() — returns the defeat message (e.g., "GHOST CAUGHT")
 * - getLoseLedState() — returns the LED state for defeat animation
 *
 * Optionally override:
 * - getLoseDisplayMs() — returns defeat screen duration (default 3000ms)
 * - getDefeatTextX() / getDefeatTextY() — text position (defaults 10, 30)
 * - showScoreOnLose() — returns true to display score (default false)
 * - getScoreX() / getScoreY() — score position if shown
 * - getHapticIntensity() — haptic feedback strength (default 255 = max)
 */
template <typename GameT, int IntroStateId>
class BaseLoseState : public State {
public:
    explicit BaseLoseState(GameT* game, int stateId) :
        State(stateId),
        game(game)
    {
    }

    virtual ~BaseLoseState() {
        game = nullptr;
    }

    void onStateMounted(Device* PDN) override {
        transitionToIntroState = false;

        auto& session = game->getSession();

        MiniGameOutcome loseOutcome;
        loseOutcome.result = MiniGameResult::LOST;
        loseOutcome.score = session.score;
        loseOutcome.hardMode = false;
        game->setOutcome(loseOutcome);

        logDefeat(session.score);

        // Display defeat screen
        PDN->getDisplay()->invalidateScreen();

        // Check if defeat text spans multiple lines
        const char* line1 = nullptr;
        const char* line2 = nullptr;
        getDefeatTextLines(line1, line2);

        if (line2 != nullptr) {
            // Two-line defeat text
            PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
                ->drawText(line1, getDefeatTextX(), getDefeatTextY())
                ->drawText(line2, getDefeatText2X(), getDefeatText2Y());
        } else {
            // Single-line defeat text
            PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
                ->drawText(defeatText(), getDefeatTextX(), getDefeatTextY());
        }

        // Optionally display score
        if (showScoreOnLose()) {
            char scoreStr[16];
            snprintf(scoreStr, sizeof(scoreStr), "Score: %d", session.score);
            PDN->getDisplay()->drawText(scoreStr, getScoreX(), getScoreY());
        }

        PDN->getDisplay()->render();

        // Defeat LED animation
        AnimationConfig animConfig = getLoseAnimationConfig();
        PDN->getLightManager()->startAnimation(animConfig);

        // Defeat haptic
        PDN->getHaptics()->setIntensity(getHapticIntensity());

        loseTimer.setTimer(getLoseDisplayMs());
    }

    void onStateLoop(Device* PDN) override {
        if (loseTimer.expired()) {
            PDN->getHaptics()->off();
            if (!game->getConfig().managedMode) {
                // In standalone mode, restart the game
                transitionToIntroState = true;
            } else {
                // In managed mode, return to the previous app
                PDN->returnToPreviousApp();
            }
        }
    }

    void onStateDismounted(Device* PDN) override {
        loseTimer.invalidate();
        transitionToIntroState = false;
        PDN->getHaptics()->off();
    }

    bool transitionToIntro() const {
        return transitionToIntroState;
    }

    bool isTerminalState() const override {
        return game->getConfig().managedMode;
    }

protected:
    // Pure virtual methods — derived classes MUST override
    virtual const char* defeatText() const = 0;
    virtual LEDState getLoseLedState() const = 0;

    // Virtual methods with defaults — derived classes CAN override
    virtual int getLoseDisplayMs() const { return 3000; }
    virtual int getDefeatTextX() const { return 10; }
    virtual int getDefeatTextY() const { return 30; }
    virtual int getDefeatText2X() const { return 30; }
    virtual int getDefeatText2Y() const { return 40; }
    virtual int getScoreX() const { return 30; }
    virtual int getScoreY() const { return 55; }
    virtual int getHapticIntensity() const { return 255; }
    virtual bool showScoreOnLose() const { return false; }

    // For multi-line defeat text (e.g., "FIREWALL" / "INTACT")
    virtual void getDefeatTextLines(const char*& line1, const char*& line2) const {
        line1 = defeatText();
        line2 = nullptr;
    }

    virtual AnimationConfig getLoseAnimationConfig() const {
        AnimationConfig config;
        config.type = AnimationType::IDLE;
        config.speed = 8;
        config.curve = EaseCurve::LINEAR;
        config.initialState = getLoseLedState();
        config.loopDelayMs = 0;
        config.loop = true;
        return config;
    }

    virtual void logDefeat(int score) const {
        // Default: no logging (games can override if they want custom logging)
    }

    GameT* game;
    SimpleTimer loseTimer;
    bool transitionToIntroState = false;
};
