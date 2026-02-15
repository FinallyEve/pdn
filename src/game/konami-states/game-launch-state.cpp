#include "game/konami-states/game-launch-state.hpp"
#include "game/difficulty-helpers.hpp"
#include "device/device.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "GameLaunchState";

GameLaunchState::GameLaunchState(int stateId, int fdnIndex, LaunchMode mode, Player* player) :
    State(stateId),
    fdnIndex(fdnIndex),
    mode(mode),
    player(player),
    gameType(fdnGameTypeToGameType(fdnIndex))
{
}

GameLaunchState::~GameLaunchState() {
    player = nullptr;
}

void GameLaunchState::onStateMounted(Device* PDN) {
    transitionToButtonAwardedState = false;
    transitionToBoonAwardedState = false;
    transitionToGameOverState = false;
    gameLaunched = false;
    gameReturned = false;

    int appId = getAppIdForGame(gameType);
    if (appId < 0) {
        LOG_W(TAG, "No app registered for game type %d", static_cast<int>(gameType));
        transitionToGameOverState = true;
        return;
    }

    auto* game = static_cast<MiniGame*>(PDN->getApp(StateId(appId)));
    if (!game) {
        LOG_W(TAG, "App %d not found", appId);
        transitionToGameOverState = true;
        return;
    }

    // Apply difficulty based on launch mode
    bool hardMode = (mode == LaunchMode::HARD_LAUNCH);
    bool isRecreational = (mode == LaunchMode::EASY_REPLAY);

    const char* modeStr = hardMode ? "HARD" : (isRecreational ? "EASY_REPLAY" : "EASY_FIRST");
    LOG_I(TAG, "Launching %s in %s mode (fdnIndex=%d)",
           getGameDisplayName(gameType), modeStr, fdnIndex);

    // Apply difficulty config via the scaler system
    // For hard mode: scale = 1.0 (maximum difficulty)
    // For easy mode: scale = 0.0 (minimum difficulty)
    float scale = hardMode ? 1.0f : 0.0f;

    switch (gameType) {
        case GameType::SIGNAL_ECHO: {
            auto* echo = static_cast<SignalEcho*>(game);
            echo->getConfig() = makeScaledSignalEchoConfig(scale, true);
            break;
        }
        case GameType::GHOST_RUNNER: {
            auto* gr = static_cast<GhostRunner*>(game);
            gr->getConfig() = makeScaledGhostRunnerConfig(scale, true);
            break;
        }
        case GameType::SPIKE_VECTOR: {
            auto* sv = static_cast<SpikeVector*>(game);
            sv->getConfig() = makeScaledSpikeVectorConfig(scale, true);
            break;
        }
        case GameType::FIREWALL_DECRYPT: {
            auto* fw = static_cast<FirewallDecrypt*>(game);
            fw->getConfig() = makeScaledFirewallDecryptConfig(scale, true);
            break;
        }
        case GameType::CIPHER_PATH: {
            auto* cp = static_cast<CipherPath*>(game);
            cp->getConfig() = makeScaledCipherPathConfig(scale, true);
            break;
        }
        case GameType::EXPLOIT_SEQUENCER: {
            auto* es = static_cast<ExploitSequencer*>(game);
            es->getConfig() = makeScaledExploitSequencerConfig(scale, true);
            break;
        }
        case GameType::BREACH_DEFENSE: {
            auto* bd = static_cast<BreachDefense*>(game);
            bd->getConfig() = makeScaledBreachDefenseConfig(scale, true);
            break;
        }
        default:
            break;
    }

    player->setRecreationalMode(isRecreational);
    game->resetGame();
    PDN->setActiveApp(StateId(appId));
    gameLaunched = true;
}

void GameLaunchState::onStateLoop(Device* PDN) {
    // After game returns, transitions are handled via onStateResumed
    // This loop only runs if the game hasn't been launched or something went wrong
    (void)PDN;
}

void GameLaunchState::onStateDismounted(Device* PDN) {
    gameLaunched = false;
    gameReturned = false;
    (void)PDN;
}

void GameLaunchState::onStateResumed(Device* PDN, Snapshot* snapshot) {
    (void)snapshot;
    gameReturned = true;

    // The minigame has finished — check outcome
    int appId = getAppIdForGame(gameType);
    if (appId < 0) {
        transitionToGameOverState = true;
        return;
    }

    auto* game = static_cast<MiniGame*>(PDN->getApp(StateId(appId)));
    if (!game) {
        transitionToGameOverState = true;
        return;
    }

    const MiniGameOutcome& outcome = game->getOutcome();

    if (outcome.result == MiniGameResult::WON) {
        LOG_I(TAG, "Player WON %s!", getGameDisplayName(gameType));

        if (mode == LaunchMode::EASY_FIRST) {
            // First encounter win: award button
            transitionToButtonAwardedState = true;
        } else if (mode == LaunchMode::HARD_LAUNCH) {
            // Hard mode win: award boon
            transitionToBoonAwardedState = true;
        } else {
            // Recreational replay — no reward
            transitionToGameOverState = true;
        }
    } else {
        LOG_I(TAG, "Player LOST %s", getGameDisplayName(gameType));
        transitionToGameOverState = true;
    }
}

bool GameLaunchState::transitionToButtonAwarded() {
    return transitionToButtonAwardedState;
}

bool GameLaunchState::transitionToBoonAwarded() {
    return transitionToBoonAwardedState;
}

bool GameLaunchState::transitionToGameOver() {
    return transitionToGameOverState;
}
