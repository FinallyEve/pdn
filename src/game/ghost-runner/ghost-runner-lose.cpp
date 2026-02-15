#include "game/ghost-runner/ghost-runner-states.hpp"
#include "game/ghost-runner/ghost-runner.hpp"
#include "game/ghost-runner/ghost-runner-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "GhostRunnerLose";

GhostRunnerLose::GhostRunnerLose(GhostRunner* game) :
    BaseLoseState<GhostRunner, GHOST_INTRO>(game, GHOST_LOSE)
{
}

LEDState GhostRunnerLose::getLoseLedState() const {
    return GHOST_RUNNER_LOSE_STATE;
}

void GhostRunnerLose::logDefeat(int score) const {
    auto& session = game->getSession();
    LOG_I(TAG, "GHOST CAUGHT — score=%d, strikes=%d", score, session.strikes);
}
