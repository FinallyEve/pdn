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
