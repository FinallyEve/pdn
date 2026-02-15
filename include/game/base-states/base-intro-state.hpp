#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "device/drivers/logger.hpp"
#include "game/minigame.hpp"

/*
 * BaseIntroState — Template base class for all minigame intro screens.
 *
 * Provides common intro logic:
 * - Displays title and subtitle text
 * - Resets game session and seeds RNG
 * - Starts idle LED animation
 * - Waits for INTRO_DURATION_MS then transitions to next state
 *
 * Template parameters:
 * - GameT: The minigame type (e.g., GhostRunner, SignalEcho)
 * - NextStateId: The state ID to transition to after intro
 *
 * Derived classes must override:
 * - introTitle() — returns the game title string (e.g., "GHOST RUNNER")
 * - introSubtext() — returns the subtitle string (e.g., "Phase through.")
 * - getIdleLedState() — returns the LED state for idle animation
 *
 * Optionally override:
 * - getIntroDurationMs() — returns intro display duration (default 2000ms)
 * - onIntroSetup(Device*) — for game-specific intro setup (called after common setup)
 */
template <typename GameT, int NextStateId>
class BaseIntroState : public State {
public:
    explicit BaseIntroState(GameT* game, int stateId) :
        State(stateId),
        game(game)
    {
    }

    virtual ~BaseIntroState() {
        game = nullptr;
    }

    void onStateMounted(Device* PDN) override {
        transitionToNextState = false;

        // Reset session for a fresh game
        game->getSession().reset();
        game->resetGame();

        PlatformClock* clock = SimpleTimer::getPlatformClock();
        game->setStartTime(clock != nullptr ? clock->milliseconds() : 0);

        // Seed RNG for this run
        game->seedRng(game->getConfig().rngSeed);

        // Call game-specific intro setup (e.g., generate first sequence for Signal Echo)
        onIntroSetup(PDN);

        // Display title screen
        PDN->getDisplay()->invalidateScreen();
        PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
            ->drawText(introTitle(), getTitleX(), getTitleY())
            ->drawText(introSubtext(), getSubtextX(), getSubtextY());
        PDN->getDisplay()->render();

        // Start idle LED animation
        AnimationConfig config;
        config.type = AnimationType::IDLE;
        config.speed = 16;
        config.curve = EaseCurve::LINEAR;
        config.initialState = getIdleLedState();
        config.loopDelayMs = 0;
        config.loop = true;
        PDN->getLightManager()->startAnimation(config);

        // Start intro timer
        introTimer.setTimer(getIntroDurationMs());
    }

    void onStateLoop(Device* PDN) override {
        if (introTimer.expired()) {
            transitionToNextState = true;
        }
    }

    void onStateDismounted(Device* PDN) override {
        introTimer.invalidate();
        transitionToNextState = false;
    }

    bool transitionCondition() const {
        return transitionToNextState;
    }

protected:
    // Pure virtual methods — derived classes MUST override
    virtual const char* introTitle() const = 0;
    virtual const char* introSubtext() const = 0;
    virtual LEDState getIdleLedState() const = 0;

    // Virtual methods with defaults — derived classes CAN override
    virtual int getIntroDurationMs() const { return 2000; }
    virtual int getTitleX() const { return 10; }
    virtual int getTitleY() const { return 20; }
    virtual int getSubtextX() const { return 10; }
    virtual int getSubtextY() const { return 45; }
    virtual void onIntroSetup(Device* PDN) { /* default: no additional setup */ }

    GameT* game;
    SimpleTimer introTimer;
    bool transitionToNextState = false;
};
