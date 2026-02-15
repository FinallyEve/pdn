#include "game/signal-echo/signal-echo-states.hpp"
#include "game/signal-echo/signal-echo.hpp"
#include "game/signal-echo/signal-echo-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "EchoWin";

EchoWin::EchoWin(SignalEcho* game) :
    BaseWinState<SignalEcho, ECHO_INTRO>(game, ECHO_WIN)
{
}

LEDState EchoWin::getWinLedState() const {
    return SIGNAL_ECHO_WIN_STATE;
}

bool EchoWin::computeHardMode() const {
    auto& config = game->getConfig();
    return (config.allowedMistakes <= 1 && config.sequenceLength >= 8);
}

void EchoWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "ACCESS GRANTED — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
