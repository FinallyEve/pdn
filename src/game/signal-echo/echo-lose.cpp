#include "game/signal-echo/signal-echo-states.hpp"
#include "game/signal-echo/signal-echo.hpp"
#include "game/signal-echo/signal-echo-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "EchoLose";

EchoLose::EchoLose(SignalEcho* game) :
    BaseLoseState<SignalEcho, ECHO_INTRO>(game, ECHO_LOSE)
{
}

LEDState EchoLose::getLoseLedState() const {
    return SIGNAL_ECHO_LOSE_STATE;
}

void EchoLose::logDefeat(int score) const {
    auto& session = game->getSession();
    LOG_I(TAG, "SIGNAL LOST — score=%d, mistakes=%d", score, session.mistakes);
}
