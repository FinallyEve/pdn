#include "game/firewall-decrypt/firewall-decrypt-states.hpp"
#include "game/firewall-decrypt/firewall-decrypt.hpp"
#include "game/firewall-decrypt/firewall-decrypt-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "DecryptWin";

DecryptWin::DecryptWin(FirewallDecrypt* game) :
    BaseWinState<FirewallDecrypt, DECRYPT_INTRO>(game, DECRYPT_WIN)
{
}

LEDState DecryptWin::getWinLedState() const {
    return FIREWALL_DECRYPT_WIN_STATE;
}

bool DecryptWin::computeHardMode() const {
    auto& config = game->getConfig();
    return (config.numCandidates >= 10 && config.numRounds >= 4);
}

void DecryptWin::logVictory(int score, bool isHard) const {
    LOG_I(TAG, "DECRYPTED! — score=%d, hardMode=%s",
          score, isHard ? "true" : "false");
}
