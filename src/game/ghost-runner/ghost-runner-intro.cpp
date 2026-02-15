#include "game/ghost-runner/ghost-runner-states.hpp"
#include "game/ghost-runner/ghost-runner.hpp"
#include "game/ghost-runner/ghost-runner-resources.hpp"

GhostRunnerIntro::GhostRunnerIntro(GhostRunner* game) :
    BaseIntroState<GhostRunner, GHOST_SHOW>(game, GHOST_INTRO)
{
}

LEDState GhostRunnerIntro::getIdleLedState() const {
    return GHOST_RUNNER_IDLE_STATE;
}
