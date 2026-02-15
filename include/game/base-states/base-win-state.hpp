#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "device/drivers/logger.hpp"
#include "game/minigame.hpp"
#include <string>

/*
 * BaseWinState — Template base class for all minigame victory screens.
 *
 * Provides common win logic:
 * - Sets MiniGameOutcome to WON with score and hard mode flag
 * - Displays victory text and score
 * - Starts celebration LED animation
 * - Triggers celebration haptic feedback
 * - Waits for WIN_DISPLAY_MS then:
 *   - In standalone mode: transitions back to intro
 *   - In managed mode: calls Device::returnToPreviousApp()
 *
 * Template parameters:
 * - GameT: The minigame type (e.g., GhostRunner, SignalEcho)
 * - IntroStateId: The state ID to transition to for standalone replay
 *
 * Derived classes must override:
 * - victoryText() — returns the victory message (e.g., "RUN COMPLETE")
 * - getWinLedState() — returns the LED state for victory animation
 * - computeHardMode() — returns true if this was a hard mode victory
 *
 * Optionally override:
 * - getWinDisplayMs() — returns victory screen duration (default 3000ms)
 * - getVictoryTextX() / getVictoryTextY() — text position (defaults 10, 25)
 * - getScoreX() / getScoreY() — score position (defaults 30, 50)
 * - getHapticIntensity() — haptic feedback strength (default 200)
 */
template <typename GameT, int IntroStateId>
class BaseWinState : public State {
public:
    explicit BaseWinState(GameT* game, int stateId) :
        State(stateId),
        game(game)
    {
    }

    virtual ~BaseWinState() {
        game = nullptr;
    }

    void onStateMounted(Device* PDN) override {
        transitionToIntroState = false;

        auto& session = game->getSession();

        // Determine if this was a hard mode win
        bool isHard = computeHardMode();

        MiniGameOutcome winOutcome;
        winOutcome.result = MiniGameResult::WON;
        winOutcome.score = session.score;
        winOutcome.hardMode = isHard;
        game->setOutcome(winOutcome);

        logVictory(session.score, isHard);

        // Display victory screen
        PDN->getDisplay()->invalidateScreen();
        PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
            ->drawText(victoryText(), getVictoryTextX(), getVictoryTextY());

        // Display score
        std::string scoreStr = "Score: " + std::to_string(session.score);
        PDN->getDisplay()->drawText(scoreStr.c_str(), getScoreX(), getScoreY());
        PDN->getDisplay()->render();

        // Victory LED animation
        AnimationConfig animConfig = getWinAnimationConfig();
        PDN->getLightManager()->startAnimation(animConfig);

        // Celebration haptic
        PDN->getHaptics()->setIntensity(getHapticIntensity());

        winTimer.setTimer(getWinDisplayMs());
    }

    void onStateLoop(Device* PDN) override {
        if (winTimer.expired()) {
            PDN->getHaptics()->off();
            if (!game->getConfig().managedMode) {
                // In standalone mode, restart the game
                transitionToIntroState = true;
            } else {
                // In managed mode, return to the previous app (e.g. Quickdraw)
                PDN->returnToPreviousApp();
            }
        }
    }

    void onStateDismounted(Device* PDN) override {
        winTimer.invalidate();
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
    virtual const char* victoryText() const = 0;
    virtual LEDState getWinLedState() const = 0;
    virtual bool computeHardMode() const = 0;

    // Virtual methods with defaults — derived classes CAN override
    virtual int getWinDisplayMs() const { return 3000; }
    virtual int getVictoryTextX() const { return 10; }
    virtual int getVictoryTextY() const { return 25; }
    virtual int getScoreX() const { return 30; }
    virtual int getScoreY() const { return 50; }
    virtual int getHapticIntensity() const { return 200; }

    virtual AnimationConfig getWinAnimationConfig() const {
        AnimationConfig config;
        config.type = AnimationType::VERTICAL_CHASE;
        config.speed = 5;
        config.curve = EaseCurve::EASE_IN_OUT;
        config.initialState = getWinLedState();
        config.loopDelayMs = 500;
        config.loop = true;
        return config;
    }

    virtual void logVictory(int score, bool isHard) const {
        // Default: no logging (games can override if they want custom logging)
    }

    GameT* game;
    SimpleTimer winTimer;
    bool transitionToIntroState = false;
};
