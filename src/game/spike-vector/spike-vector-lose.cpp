#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/spike-vector/spike-vector-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "SpikeVectorLose";

SpikeVectorLose::SpikeVectorLose(SpikeVector* game) :
    BaseLoseState<SpikeVector, SPIKE_INTRO>(game, SPIKE_LOSE)
{
}

LEDState SpikeVectorLose::getLoseLedState() const {
    return SPIKE_VECTOR_LOSE_STATE;
}

void SpikeVectorLose::logDefeat(int score) const {
    auto& session = game->getSession();
    LOG_I(TAG, "SPIKE IMPACT — score=%d, hits=%d", score, session.hits);
}
