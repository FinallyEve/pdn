#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "game/base-states/base-intro-state.hpp"
#include "game/base-states/base-win-state.hpp"
#include "game/base-states/base-lose-state.hpp"

class BreachDefense;

/*
 * Breach Defense state IDs — offset to 700+ to avoid collisions.
 */
enum BreachDefenseStateId {
    BREACH_INTRO    = 700,
    BREACH_WIN      = 701,
    BREACH_LOSE     = 702,
    BREACH_SHOW     = 703,
    BREACH_GAMEPLAY = 704,
    BREACH_EVALUATE = 705,
};

/*
 * BreachDefenseIntro — Title screen. Shows "BREACH DEFENSE" and subtitle.
 * Resets session and seeds RNG. Transitions to Show after timer.
 */
class BreachDefenseIntro : public BaseIntroState<BreachDefense, BREACH_SHOW> {
public:
    explicit BreachDefenseIntro(BreachDefense* game);

    bool transitionToShow() const { return this->transitionCondition(); }

protected:
    const char* introTitle() const override { return "BREACH DEFENSE"; }
    const char* introSubtext() const override { return "Hold the line."; }
    LEDState getIdleLedState() const override;
};

/*
 * BreachDefenseShow — Threat announcement screen.
 * Displays "Threat X of Y", generates random threat lane,
 * resets threat position. Transitions to Gameplay after timer.
 */
class BreachDefenseShow : public State {
public:
    explicit BreachDefenseShow(BreachDefense* game);
    ~BreachDefenseShow() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToGameplay();

private:
    BreachDefense* game;
    SimpleTimer showTimer;
    static constexpr int SHOW_DURATION_MS = 1500;
    bool transitionToGameplayState = false;
};

/*
 * BreachDefenseGameplay — Core defense gameplay.
 * Threat advances on timer. Player moves shield with UP/DOWN buttons.
 * When threat reaches the end, transitions to Evaluate.
 */
class BreachDefenseGameplay : public State {
public:
    explicit BreachDefenseGameplay(BreachDefense* game);
    ~BreachDefenseGameplay() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToEvaluate();

    BreachDefense* game;

private:
    SimpleTimer threatTimer;
    bool transitionToEvaluateState = false;
};

/*
 * BreachDefenseEvaluate — Checks if shield blocked the threat.
 * Block: score += 100. Breach: breaches++.
 * Routes to Show (next threat), Win (all survived), or Lose (too many breaches).
 */
class BreachDefenseEvaluate : public State {
public:
    explicit BreachDefenseEvaluate(BreachDefense* game);
    ~BreachDefenseEvaluate() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToShow();
    bool transitionToWin();
    bool transitionToLose();

private:
    BreachDefense* game;
    SimpleTimer evalTimer;
    static constexpr int EVAL_DURATION_MS = 500;
    bool transitionToShowState = false;
    bool transitionToWinState = false;
    bool transitionToLoseState = false;
};

/*
 * BreachDefenseWin — Victory screen. Shows "BREACH BLOCKED".
 * Sets outcome to WON. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class BreachDefenseWin : public BaseWinState<BreachDefense, BREACH_INTRO> {
public:
    explicit BreachDefenseWin(BreachDefense* game);

protected:
    const char* victoryText() const override { return "BREACH BLOCKED"; }
    LEDState getWinLedState() const override;
    bool computeHardMode() const override;

    void logVictory(int score, bool isHard) const override;
};

/*
 * BreachDefenseLose — Defeat screen. Shows "BREACH OPEN".
 * Sets outcome to LOST. In standalone mode, transitions back to Intro.
 * In managed mode, calls Device::returnToPreviousApp().
 */
class BreachDefenseLose : public BaseLoseState<BreachDefense, BREACH_INTRO> {
public:
    explicit BreachDefenseLose(BreachDefense* game);

protected:
    const char* defeatText() const override { return "BREACH OPEN"; }
    LEDState getLoseLedState() const override;

    void logDefeat(int score) const override;
};
