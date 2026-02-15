#include "game/konami-states/konami-handshake.hpp"
#include "game/konami-states/game-launch-state.hpp"
#include "device/device.hpp"
#include "device/drivers/logger.hpp"
#include <cstdint>

static const char* TAG = "KonamiHandshake";

KonamiHandshake::KonamiHandshake(Player* player) :
    State(0),
    player(player),
    fdnGameType(FdnGameType::SIGNAL_ECHO),
    gameTypeReceived(false),
    targetStateIndex(-1),
    transitionReady(false)
{
}

KonamiHandshake::~KonamiHandshake() {
    player = nullptr;
}

void KonamiHandshake::onStateMounted(Device* PDN) {
    gameTypeReceived = false;
    transitionReady = false;
    targetStateIndex = -1;

    LOG_I(TAG, "KonamiHandshake mounted");

    // Read game type from Player (set by FdnDetected before launching KonamiMetaGame)
    int lastGameType = player->getLastFdnGameType();
    if (lastGameType >= 0 && lastGameType <= 7) {
        fdnGameType = static_cast<FdnGameType>(lastGameType);
        gameTypeReceived = true;
        LOG_I(TAG, "Read FDN game type from player: %d", lastGameType);
    } else {
        LOG_W(TAG, "No valid FDN game type on player (%d) — listening on serial", lastGameType);
    }

    // Also register serial callback as fallback (for direct serial testing)
    PDN->setOnStringReceivedCallback([this](const std::string& message) {
        handleSerialMessage(message);
    });

    // Display routing state
    PDN->getDisplay()->invalidateScreen();
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT);
    PDN->getDisplay()->drawText("KONAMI", 35, 20);
    PDN->getDisplay()->drawText("ROUTING...", 20, 35);
    PDN->getDisplay()->render();
}

void KonamiHandshake::onStateLoop(Device* PDN) {
    if (gameTypeReceived && !transitionReady) {
        targetStateIndex = calculateTargetState(fdnGameType);

        if (targetStateIndex >= 0) {
            LOG_I(TAG, "Routing to state index: %d", targetStateIndex);
            transitionReady = true;
        } else {
            LOG_E(TAG, "Invalid target state calculated - cannot route");
        }
    }
    (void)PDN;
}

void KonamiHandshake::onStateDismounted(Device* PDN) {
    PDN->clearCallbacks();
    gameTypeReceived = false;
    transitionReady = false;
    targetStateIndex = -1;
}

bool KonamiHandshake::shouldTransition() const {
    return transitionReady;
}

int KonamiHandshake::getTargetStateIndex() const {
    return targetStateIndex;
}

void KonamiHandshake::handleSerialMessage(const std::string& message) {
    // Expected format: "fgame:<gameType>" where gameType is 0-7
    if (message.rfind("fgame:", 0) != 0) {
        return;
    }

    std::string gameTypeStr = message.substr(6);

    try {
        int gameTypeInt = std::stoi(gameTypeStr);

        if (gameTypeInt < 0 || gameTypeInt > 7) {
            LOG_W(TAG, "Invalid game type value: %d", gameTypeInt);
            return;
        }

        fdnGameType = static_cast<FdnGameType>(gameTypeInt);
        gameTypeReceived = true;

        LOG_I(TAG, "Received FDN game type via serial: %d", gameTypeInt);
    } catch (const std::exception& e) {
        LOG_W(TAG, "Failed to parse game type from: %s", message.c_str());
    }
}

int KonamiHandshake::calculateTargetState(FdnGameType gameType) {
    // Special case: KONAMI_CODE FDN
    if (gameType == FdnGameType::KONAMI_CODE) {
        if (player->hasAllKonamiButtons()) {
            LOG_I(TAG, "KONAMI_CODE FDN - all buttons collected → CodeEntry");
            return 32;  // KONAMI_CODE_ENTRY state index
        } else {
            LOG_I(TAG, "KONAMI_CODE FDN - incomplete buttons → CodeRejected");
            return 34;  // KONAMI_CODE_REJECTED state index
        }
    }

    // Regular game FDN - get the game index (0-6)
    int gameIndex = static_cast<int>(gameType);

    // Convert FdnGameType to GameType using shared helper
    GameType mappedGameType = fdnGameTypeToGameType(gameIndex);
    KonamiButton reward = getRewardForGame(mappedGameType);
    uint8_t buttonIndex = static_cast<uint8_t>(reward);

    bool hasButton = player->hasUnlockedButton(buttonIndex);
    bool hasBoon = player->hasKonamiBoon();
    bool hardModeUnlocked = player->hasColorProfileEligibility(static_cast<int>(mappedGameType));

    if (hasBoon) {
        int targetIndex = 22 + gameIndex;
        LOG_I(TAG, "hasBoon → MasteryReplay, index=%d", targetIndex);
        return targetIndex;
    }

    if (hardModeUnlocked && !hasBoon) {
        int targetIndex = 15 + gameIndex;
        LOG_I(TAG, "hardModeUnlocked && !hasBoon → HardLaunch, index=%d", targetIndex);
        return targetIndex;
    }

    if (hasButton) {
        int targetIndex = 8 + gameIndex;
        LOG_I(TAG, "hasButton → ReplayEasy, index=%d", targetIndex);
        return targetIndex;
    }

    int targetIndex = 1 + gameIndex;
    LOG_I(TAG, "First encounter → EasyLaunch, index=%d", targetIndex);
    return targetIndex;
}
