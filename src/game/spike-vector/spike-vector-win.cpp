#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/spike-vector/spike-vector-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "SpikeVectorWin";

SpikeVectorWin::SpikeVectorWin(SpikeVector* game) :
    BaseWinState<SpikeVector, SPIKE_INTRO>(game, SPIKE_WIN)
{
}

LEDState SpikeVectorWin::getWinLedState() const {
    return SPIKE_VECTOR_WIN_STATE;
}

bool SpikeVectorWin::computeHardMode() const {
    auto& config = game->getConfig();
    return (config.hitsAllowed <= 1 && config.numPositions >= 7);
}

void SpikeVectorWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "VECTOR CLEAR — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
