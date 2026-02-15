#include "game/breach-defense/breach-defense-states.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "game/breach-defense/breach-defense-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "BreachDefenseLose";

BreachDefenseLose::BreachDefenseLose(BreachDefense* game) :
    BaseLoseState<BreachDefense, BREACH_INTRO>(game, BREACH_LOSE)
{
}

LEDState BreachDefenseLose::getLoseLedState() const {
    return BREACH_DEFENSE_LOSE_STATE;
}

void BreachDefenseLose::logDefeat(int score) const {
    auto& session = game->getSession();
    LOG_I(TAG, "BREACH OPEN — score=%d, breaches=%d", score, session.breaches);
}
