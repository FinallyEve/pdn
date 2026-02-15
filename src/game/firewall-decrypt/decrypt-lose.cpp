#include "game/firewall-decrypt/firewall-decrypt-states.hpp"
#include "game/firewall-decrypt/firewall-decrypt.hpp"
#include "game/firewall-decrypt/firewall-decrypt-resources.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "DecryptLose";

DecryptLose::DecryptLose(FirewallDecrypt* game) :
    BaseLoseState<FirewallDecrypt, DECRYPT_INTRO>(game, DECRYPT_LOSE)
{
}

LEDState DecryptLose::getLoseLedState() const {
    return FIREWALL_DECRYPT_LOSE_STATE;
}

void DecryptLose::logDefeat(int score) const {
    LOG_I(TAG, "FIREWALL INTACT — score=%d", score);
}
