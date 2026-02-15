#include "game/cipher-path/cipher-path-states.hpp"
#include "game/cipher-path/cipher-path.hpp"
#include "game/cipher-path/cipher-path-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "CipherPathWin";

CipherPathWin::CipherPathWin(CipherPath* game) :
    BaseWinState<CipherPath, CIPHER_INTRO>(game, CIPHER_WIN)
{
}

LEDState CipherPathWin::getWinLedState() const {
    return CIPHER_PATH_WIN_STATE;
}

bool CipherPathWin::computeHardMode() const {
    auto& config = game->getConfig();
    return (config.gridSize >= 10 && config.moveBudget <= 14);
}

AnimationConfig CipherPathWin::getWinAnimationConfig() const {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.speed = 20;
    config.curve = EaseCurve::LINEAR;
    config.initialState = getWinLedState();
    config.loopDelayMs = 0;
    config.loop = true;
    return config;
}

void CipherPathWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "PATH DECODED — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
