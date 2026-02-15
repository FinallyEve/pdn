#include "game/breach-defense/breach-defense-states.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "game/breach-defense/breach-defense-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "BreachDefenseWin";

BreachDefenseWin::BreachDefenseWin(BreachDefense* game) :
    BaseWinState<BreachDefense, BREACH_INTRO>(game, BREACH_WIN)
{
}

LEDState BreachDefenseWin::getWinLedState() const {
    return BREACH_DEFENSE_WIN_STATE;
}

bool BreachDefenseWin::computeHardMode() const {
    auto& config = game->getConfig();
    return (config.numLanes >= 5 && config.missesAllowed <= 1);
}

AnimationConfig BreachDefenseWin::getWinAnimationConfig() const {
    // Original implementation had no LED animation, only haptics
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.speed = 1;
    config.curve = EaseCurve::LINEAR;
    config.initialState = getWinLedState();
    config.loopDelayMs = 0;
    config.loop = false;
    return config;
}

void BreachDefenseWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "BREACH BLOCKED — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
