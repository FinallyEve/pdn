#include "game/konami-states/game-launch-state.hpp"
#include "device/device.hpp"
#include "device/drivers/logger.hpp"
#include "game/difficulty-scaler.hpp"
#include "game/sequence-provider.hpp"
#include "state/state-machine.hpp"
#include "game/minigame.hpp"
#include "game/difficulty-helpers.hpp"

static const char* TAG = "GameLaunchState";

GameLaunchState::GameLaunchState(int stateId, GameType gameType, LaunchMode mode) :
    State(stateId),
    gameType(gameType),
    launchMode(mode),
    gameLaunched(false),
    gameResumed(false),
    transitionToButtonAwardedState(false),
    transitionToBoonAwardedState(false),
    transitionToGameOverState(false),
    playerWon(false)
{
}

GameLaunchState::~GameLaunchState() {
}

void GameLaunchState::onStateMounted(Device* PDN) {
    gameLaunched = false;
    gameResumed = false;
    transitionToButtonAwardedState = false;
    transitionToBoonAwardedState = false;
    transitionToGameOverState = false;
    playerWon = false;

    LOG_I(TAG, "GameLaunchState mounted - game=%d, mode=%d",
          static_cast<int>(gameType), static_cast<int>(launchMode));

    // Display game name and mode
    PDN->getDisplay()->invalidateScreen();
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT);
    PDN->getDisplay()->drawText(getGameDisplayName(gameType), 10, 20);
    PDN->getDisplay()->drawText(getModeDisplayText(), 10, 35);
    PDN->getDisplay()->drawText("LAUNCHING...", 10, 50);
    PDN->getDisplay()->render();

    displayTimer.setTimer(DISPLAY_DURATION_MS);
}

void GameLaunchState::onStateLoop(Device* PDN) {
    displayTimer.updateTime();

    // Wait for display duration before launching
    if (!gameLaunched && displayTimer.expired()) {
        launchGame(PDN);
        gameLaunched = true;
    }

    // After game resumes, check outcome and transition
    if (gameResumed) {
        readOutcome(PDN);

        // Route based on mode and outcome
        switch (launchMode) {
            case LaunchMode::EASY_FIRST_ENCOUNTER:
                if (playerWon) {
                    transitionToButtonAwardedState = true;
                } else {
                    transitionToGameOverState = true;
                }
                break;

            case LaunchMode::HARD_FIRST_ENCOUNTER:
                if (playerWon) {
                    transitionToBoonAwardedState = true;
                } else {
                    transitionToGameOverState = true;
                }
                break;

            case LaunchMode::EASY_REPLAY:
            case LaunchMode::MASTERY_REPLAY:
                // No rewards on replay
                transitionToGameOverState = true;
                break;
        }
    }
}

void GameLaunchState::onStateDismounted(Device* PDN) {
    gameLaunched = false;
    gameResumed = false;
    transitionToButtonAwardedState = false;
    transitionToBoonAwardedState = false;
    transitionToGameOverState = false;
    playerWon = false;
    displayTimer.invalidate();
}

std::unique_ptr<Snapshot> GameLaunchState::onStatePaused(Device* PDN) {
    auto snapshot = std::make_unique<GameLaunchSnapshot>();
    snapshot->gameLaunched = gameLaunched;
    snapshot->gameResumed = gameResumed;
    return snapshot;
}

void GameLaunchState::onStateResumed(Device* PDN, Snapshot* snapshot) {
    if (snapshot) {
        auto* gameLaunchSnapshot = static_cast<GameLaunchSnapshot*>(snapshot);
        if (gameLaunchSnapshot) {
            gameLaunched = gameLaunchSnapshot->gameLaunched;
            gameResumed = true;  // Mark as resumed so we can check outcome
            LOG_I(TAG, "GameLaunchState resumed - checking outcome");
        }
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

void GameLaunchState::launchGame(Device* PDN) {
    LOG_I(TAG, "Launching game %d with mode %d",
          static_cast<int>(gameType), static_cast<int>(launchMode));

    // Set difficulty based on launch mode
    bool hardMode;
    switch (launchMode) {
        case LaunchMode::EASY_FIRST_ENCOUNTER:
        case LaunchMode::EASY_REPLAY:
            hardMode = false;
            break;
        case LaunchMode::HARD_FIRST_ENCOUNTER:
        case LaunchMode::MASTERY_REPLAY:
            hardMode = true;
            break;
    }

    // Get the app ID for the game type
    int appId;
    switch (gameType) {
        case GameType::SIGNAL_ECHO:       appId = 1; break;
        case GameType::GHOST_RUNNER:      appId = 2; break;
        case GameType::SPIKE_VECTOR:      appId = 3; break;
        case GameType::FIREWALL_DECRYPT:  appId = 4; break;
        case GameType::CIPHER_PATH:       appId = 5; break;
        case GameType::EXPLOIT_SEQUENCER: appId = 6; break;
        case GameType::BREACH_DEFENSE:    appId = 7; break;
        default:                          appId = 1; break;
    }

    // Configure minigame with managed mode enabled
    StateMachine* app = PDN->getApp(StateId(appId));
    MiniGame* minigame = static_cast<MiniGame*>(app);
    if (minigame) {
        // Set managedMode = true on the game config
        // Use appropriate config helpers based on game type
        switch (gameType) {
            case GameType::SIGNAL_ECHO: {
                auto* game = static_cast<SignalEcho*>(minigame);
                game->getConfig() = hardMode ? SIGNAL_ECHO_HARD : SIGNAL_ECHO_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::GHOST_RUNNER: {
                auto* game = static_cast<GhostRunner*>(minigame);
                game->getConfig() = hardMode ? GHOST_RUNNER_HARD : GHOST_RUNNER_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::SPIKE_VECTOR: {
                auto* game = static_cast<SpikeVector*>(minigame);
                game->getConfig() = hardMode ? SPIKE_VECTOR_HARD : SPIKE_VECTOR_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::FIREWALL_DECRYPT: {
                auto* game = static_cast<FirewallDecrypt*>(minigame);
                game->getConfig() = hardMode ? FIREWALL_DECRYPT_HARD : FIREWALL_DECRYPT_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::CIPHER_PATH: {
                auto* game = static_cast<CipherPath*>(minigame);
                game->getConfig() = hardMode ? CIPHER_PATH_HARD : CIPHER_PATH_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::EXPLOIT_SEQUENCER: {
                auto* game = static_cast<ExploitSequencer*>(minigame);
                game->getConfig() = hardMode ? EXPLOIT_SEQUENCER_HARD : EXPLOIT_SEQUENCER_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            case GameType::BREACH_DEFENSE: {
                auto* game = static_cast<BreachDefense*>(minigame);
                game->getConfig() = hardMode ? BREACH_DEFENSE_HARD : BREACH_DEFENSE_EASY;
                game->getConfig().managedMode = true;
                break;
            }
            default:
                break;
        }
        LOG_I(TAG, "Configured game with hardMode=%d, managedMode=true", hardMode);
    }

    // Launch the app
    PDN->setActiveApp(StateId(appId));
}

void GameLaunchState::readOutcome(Device* PDN) {
    // Get the completed app
    int appId;
    switch (gameType) {
        case GameType::SIGNAL_ECHO:       appId = 1; break;
        case GameType::GHOST_RUNNER:      appId = 2; break;
        case GameType::SPIKE_VECTOR:      appId = 3; break;
        case GameType::FIREWALL_DECRYPT:  appId = 4; break;
        case GameType::CIPHER_PATH:       appId = 5; break;
        case GameType::EXPLOIT_SEQUENCER: appId = 6; break;
        case GameType::BREACH_DEFENSE:    appId = 7; break;
        default:                          appId = 1; break;
    }

    StateMachine* stateMachineApp = PDN->getApp(StateId(appId));
    MiniGame* completedApp = static_cast<MiniGame*>(stateMachineApp);
    if (completedApp) {
        const MiniGameOutcome& outcome = completedApp->getOutcome();
        playerWon = (outcome.result == MiniGameResult::WON);
        LOG_I(TAG, "Game outcome: %s", playerWon ? "WIN" : "LOSS");
    } else {
        LOG_W(TAG, "Could not read outcome from app %d", appId);
        playerWon = false;
    }
}

const char* GameLaunchState::getModeDisplayText() {
    switch (launchMode) {
        case LaunchMode::EASY_FIRST_ENCOUNTER: return "[ EASY MODE ]";
        case LaunchMode::EASY_REPLAY:          return "[ EASY REPLAY ]";
        case LaunchMode::HARD_FIRST_ENCOUNTER: return "[ HARD MODE ]";
        case LaunchMode::MASTERY_REPLAY:       return "[ MASTERY ]";
        default:                               return "[ UNKNOWN ]";
    }
}
