#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"

class SpikeVector;

/*
 * Spike Vector state IDs — offset to 400+ to avoid collisions.
 */
enum SpikeVectorStateId {
    SPIKE_INTRO    = 400,
    SPIKE_WIN      = 401,
    SPIKE_LOSE     = 402,
    SPIKE_SHOW     = 403,
    SPIKE_GAMEPLAY = 404,
    SPIKE_EVALUATE = 405,
};

/*
 * SpikeVectorIntro — Title screen. Shows "SPIKE VECTOR" and subtitle.
 * Resets session, seeds RNG. Transitions to SpikeVectorShow after timer.
 */
class SpikeVectorIntro : public State {
public:
    explicit SpikeVectorIntro(SpikeVector* game);
    ~SpikeVectorIntro() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToShow();

private:
    SpikeVector* game;
    SimpleTimer introTimer;
    static constexpr int INTRO_DURATION_MS = 2000;
    bool transitionToShowState = false;
};

/*
 * SpikeVectorShow — Level info screen. Shows level progress pips and lives.
 * Generates gap array for this level's wall formation.
 * Transitions to SpikeVectorGameplay after SHOW_DURATION_MS.
 */
class SpikeVectorShow : public State {
public:
    explicit SpikeVectorShow(SpikeVector* game);
    ~SpikeVectorShow() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToGameplay();

private:
    SpikeVector* game;
    SimpleTimer showTimer;
    static constexpr int SHOW_DURATION_MS = 1500;
    bool transitionToGameplayState = false;
};

/*
 * SpikeVectorGameplay — Active dodging. Walls scroll left on timer.
 * Primary button = move cursor UP (decrease position).
 * Secondary button = move cursor DOWN (increase position).
 * Collision detected inline. When all walls pass, transitions to SpikeVectorEvaluate.
 */
class SpikeVectorGameplay : public State {
public:
    explicit SpikeVectorGameplay(SpikeVector* game);
    ~SpikeVectorGameplay() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToEvaluate();

    SpikeVector* game;

private:
    SimpleTimer scrollTimer;
    bool transitionToEvaluateState = false;
    bool upButtonPressed = false;
    bool downButtonPressed = false;
    bool hitThisLevel = false;           // track if player was hit this level
};

/*
 * SpikeVectorEvaluate — Level complete logic.
 * Shows level-complete pip flash animation.
 * Routes to Show (next level), Win (all levels cleared), or Lose (too many hits).
 */
class SpikeVectorEvaluate : public State {
public:
    explicit SpikeVectorEvaluate(SpikeVector* game);
    ~SpikeVectorEvaluate() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToShow();
    bool transitionToWin();
    bool transitionToLose();

private:
    SpikeVector* game;
    SimpleTimer flashTimer;
    int flashCount = 0;
    bool pipVisible = true;
    static constexpr int FLASH_DURATION_MS = 150;
    static constexpr int FLASH_CYCLES = 4;
    bool transitionToShowState = false;
    bool transitionToWinState = false;
    bool transitionToLoseState = false;
};

/*
 * SpikeVectorWin — Victory screen. Shows "VECTOR CLEAR".
 * Sets outcome to WON. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class SpikeVectorWin : public State {
public:
    explicit SpikeVectorWin(SpikeVector* game);
    ~SpikeVectorWin() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToIntro();
    bool isTerminalState() const override;

private:
    SpikeVector* game;
    SimpleTimer winTimer;
    static constexpr int WIN_DISPLAY_MS = 3000;
    bool transitionToIntroState = false;
};

/*
 * SpikeVectorLose — Defeat screen. Shows "SPIKE IMPACT".
 * Sets outcome to LOST. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class SpikeVectorLose : public State {
public:
    explicit SpikeVectorLose(SpikeVector* game);
    ~SpikeVectorLose() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToIntro();
    bool isTerminalState() const override;

private:
    SpikeVector* game;
    SimpleTimer loseTimer;
    static constexpr int LOSE_DISPLAY_MS = 3000;
    bool transitionToIntroState = false;
};
