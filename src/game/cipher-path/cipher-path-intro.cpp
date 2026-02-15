#include "game/cipher-path/cipher-path-states.hpp"
#include "game/cipher-path/cipher-path.hpp"
#include "game/cipher-path/cipher-path-resources.hpp"

CipherPathIntro::CipherPathIntro(CipherPath* game) :
    BaseIntroState<CipherPath, CIPHER_SHOW>(game, CIPHER_INTRO)
{
}

LEDState CipherPathIntro::getIdleLedState() const {
    return CIPHER_PATH_IDLE_STATE;
}
