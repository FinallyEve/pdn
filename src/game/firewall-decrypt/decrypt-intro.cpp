#include "game/firewall-decrypt/firewall-decrypt-states.hpp"
#include "game/firewall-decrypt/firewall-decrypt.hpp"
#include "game/firewall-decrypt/firewall-decrypt-resources.hpp"

DecryptIntro::DecryptIntro(FirewallDecrypt* game) :
    BaseIntroState<FirewallDecrypt, DECRYPT_SCAN>(game, DECRYPT_INTRO)
{
}

LEDState DecryptIntro::getIdleLedState() const {
    return FIREWALL_DECRYPT_IDLE_STATE;
}

void DecryptIntro::onIntroSetup(Device* PDN) {
    // Setup the first round for Firewall Decrypt
    game->setupRound();
}
