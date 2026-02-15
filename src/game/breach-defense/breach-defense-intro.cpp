#include "game/breach-defense/breach-defense-states.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "game/breach-defense/breach-defense-resources.hpp"

BreachDefenseIntro::BreachDefenseIntro(BreachDefense* game) :
    BaseIntroState<BreachDefense, BREACH_SHOW>(game, BREACH_INTRO)
{
}

LEDState BreachDefenseIntro::getIdleLedState() const {
    return BREACH_DEFENSE_IDLE_STATE;
}

AnimationConfig BreachDefenseIntro::getIntroAnimationConfig() const {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.speed = 2;
    config.curve = EaseCurve::EASE_IN_OUT;
    config.initialState = LEDState(); // Default empty state, not getIdleLedState()
    config.loopDelayMs = 0;
    config.loop = true;
    return config;
}
