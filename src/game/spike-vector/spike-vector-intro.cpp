#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/spike-vector/spike-vector-resources.hpp"

SpikeVectorIntro::SpikeVectorIntro(SpikeVector* game) :
    BaseIntroState<SpikeVector, SPIKE_SHOW>(game, SPIKE_INTRO)
{
}

LEDState SpikeVectorIntro::getIdleLedState() const {
    return SPIKE_VECTOR_IDLE_STATE;
}
