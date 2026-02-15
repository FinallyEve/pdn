#include "game/signal-echo/signal-echo-states.hpp"
#include "game/signal-echo/signal-echo.hpp"
#include "game/signal-echo/signal-echo-resources.hpp"

EchoIntro::EchoIntro(SignalEcho* game) :
    BaseIntroState<SignalEcho, ECHO_SHOW_SEQUENCE>(game, ECHO_INTRO)
{
}

LEDState EchoIntro::getIdleLedState() const {
    return SIGNAL_ECHO_IDLE_STATE;
}

void EchoIntro::onIntroSetup(Device* PDN) {
    // Generate the first sequence for Signal Echo
    game->getSession().currentSequence = game->generateSequence(
        game->getConfig().sequenceLength);
}
