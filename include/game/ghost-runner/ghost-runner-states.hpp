#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "game/base-states/base-intro-state.hpp"
#include "game/base-states/base-win-state.hpp"
#include "game/base-states/base-lose-state.hpp"

class GhostRunner;

/*
 * Ghost Runner state IDs — offset to 300+ to avoid collisions.
 */
enum GhostRunnerStateId {
    GHOST_INTRO    = 300,
    GHOST_WIN      = 301,
    GHOST_LOSE     = 302,
    GHOST_SHOW     = 303,
    GHOST_GAMEPLAY = 304,
    GHOST_EVALUATE = 305,
};

/*
 * GhostRunnerIntro — Title screen. Shows "GHOST RUNNER" and subtitle.
 * Seeds RNG, resets session, transitions to GhostRunnerShow after timer.
 */
class GhostRunnerIntro : public BaseIntroState<GhostRunner, GHOST_SHOW> {
public:
    explicit GhostRunnerIntro(GhostRunner* game);

    bool transitionToShow() const { return this->transitionCondition(); }

protected:
    const char* introTitle() const override { return "GHOST RUNNER"; }
    const char* introSubtext() const override { return "Phase through."; }
    LEDState getIdleLedState() const override;
};

/*
 * GhostRunnerShow — Brief round display. Shows current round info.
 * Transitions to GhostRunnerGameplay after SHOW_DURATION_MS.
 */
class GhostRunnerShow : public State {
public:
    explicit GhostRunnerShow(GhostRunner* game);
    ~GhostRunnerShow() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToGameplay();

private:
    GhostRunner* game;
    SimpleTimer showTimer;
    static constexpr int SHOW_DURATION_MS = 1500;
    bool transitionToGameplayState = false;
};

/*
 * GhostRunnerGameplay — The core gameplay state.
 * Ghost position advances on a timer (ghostSpeedMs per step).
 * Player presses PRIMARY button to attempt a catch.
 * Transitions to GhostRunnerEvaluate when player presses or ghost
 * reaches end of screen (timeout).
 */
class GhostRunnerGameplay : public State {
public:
    explicit GhostRunnerGameplay(GhostRunner* game);
    ~GhostRunnerGameplay() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToEvaluate();

private:
    GhostRunner* game;
    SimpleTimer ghostStepTimer;
    bool transitionToEvaluateState = false;
};

/*
 * GhostRunnerEvaluate — Brief transition state. Checks round outcome:
 * - If press was in target zone: hit (advance round, add score)
 * - If press was outside zone or timeout: strike
 * - If strikes > missesAllowed: transition to GhostRunnerLose
 * - If all rounds completed: transition to GhostRunnerWin
 * - Otherwise: advance to next round, transition to GhostRunnerShow
 */
class GhostRunnerEvaluate : public State {
public:
    explicit GhostRunnerEvaluate(GhostRunner* game);
    ~GhostRunnerEvaluate() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToShow();
    bool transitionToWin();
    bool transitionToLose();

private:
    GhostRunner* game;
    bool transitionToShowState = false;
    bool transitionToWinState = false;
    bool transitionToLoseState = false;
};

/*
 * GhostRunnerWin — Victory screen. Shows "RUN COMPLETE".
 * Sets outcome to WON. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class GhostRunnerWin : public BaseWinState<GhostRunner, GHOST_INTRO> {
public:
    explicit GhostRunnerWin(GhostRunner* game);

protected:
    const char* victoryText() const override { return "RUN COMPLETE"; }
    LEDState getWinLedState() const override;
    bool computeHardMode() const override;

    void logVictory(int score, bool isHard) const override;
};

/*
 * GhostRunnerLose — Defeat screen. Shows "GHOST CAUGHT".
 * Sets outcome to LOST. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class GhostRunnerLose : public BaseLoseState<GhostRunner, GHOST_INTRO> {
public:
    explicit GhostRunnerLose(GhostRunner* game);

protected:
    const char* defeatText() const override { return "GHOST CAUGHT"; }
    LEDState getLoseLedState() const override;

    void logDefeat(int score) const override;
};
