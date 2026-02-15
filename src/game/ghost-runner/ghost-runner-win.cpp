#include "game/ghost-runner/ghost-runner-states.hpp"
#include "game/ghost-runner/ghost-runner.hpp"
#include "game/ghost-runner/ghost-runner-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "GhostRunnerWin";

GhostRunnerWin::GhostRunnerWin(GhostRunner* game) :
    BaseWinState<GhostRunner, GHOST_INTRO>(game, GHOST_WIN)
{
}

LEDState GhostRunnerWin::getWinLedState() const {
    return GHOST_RUNNER_WIN_STATE;
}

bool GhostRunnerWin::computeHardMode() const {
    auto& config = game->getConfig();
    int zoneWidth = config.targetZoneEnd - config.targetZoneStart;
    return (config.missesAllowed <= 1 && zoneWidth <= 16);
}

void GhostRunnerWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "RUN COMPLETE — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
