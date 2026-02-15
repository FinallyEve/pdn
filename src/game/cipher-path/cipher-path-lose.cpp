#include "game/cipher-path/cipher-path-states.hpp"
#include "game/cipher-path/cipher-path.hpp"
#include "game/cipher-path/cipher-path-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "CipherPathLose";

CipherPathLose::CipherPathLose(CipherPath* game) :
    BaseLoseState<CipherPath, CIPHER_INTRO>(game, CIPHER_LOSE)
{
}

LEDState CipherPathLose::getLoseLedState() const {
    return CIPHER_PATH_LOSE_STATE;
}

void CipherPathLose::logDefeat(int score) const {
    auto& session = game->getSession();
    LOG_I(TAG, "PATH LOST — score=%d, round=%d", score, session.currentRound);
}
