#pragma once

#include "state/state.hpp"
#include "game/player.hpp"
#include "game/minigame.hpp"
#include "game/difficulty-scaler.hpp"
#include "device/device-types.hpp"
#include "game/fdn-game-type.hpp"
#include "utils/simple-timer.hpp"

/*
 * GameLaunchState — Reusable base state for launching minigames from KonamiMetaGame.
 *
 * Used for 21 of the 35 KonamiMetaGame states:
 * - EasyLaunch[0..6]:  First encounter, EASY config, awards button on win
 * - ReplayEasy[0..6]:  Re-encounter, EASY config, no reward (recreational)
 * - HardLaunch[0..6]:  Post-Konami, HARD config, awards boon/color profile on win
 *
 * Lifecycle:
 * 1. onStateMounted: Look up MiniGame app, apply difficulty, launch via setActiveApp()
 * 2. When minigame calls returnToPreviousApp(), KonamiMetaGame resumes this state
 * 3. onStateResumed: Check game outcome, set transition flags
 * 4. Transitions to ButtonAwarded, BoonAwarded, or GameOverReturn
 */

enum class LaunchMode : uint8_t {
    EASY_FIRST,    // First encounter — awards button on win
    EASY_REPLAY,   // Re-encounter — recreational, no reward
    HARD_LAUNCH    // Hard mode — awards boon/color profile on win
};

/*
 * Convert FdnGameType (0-6) to GameType for app lookup.
 * FdnGameType enum order differs from GameType enum order.
 */
inline GameType fdnGameTypeToGameType(int fdnIndex) {
    switch (fdnIndex) {
        case 0: return GameType::SIGNAL_ECHO;       // FdnGameType::SIGNAL_ECHO = 0
        case 1: return GameType::GHOST_RUNNER;       // FdnGameType::GHOST_RUNNER = 1
        case 2: return GameType::SPIKE_VECTOR;       // FdnGameType::SPIKE_VECTOR = 2
        case 3: return GameType::FIREWALL_DECRYPT;   // FdnGameType::FIREWALL_DECRYPT = 3
        case 4: return GameType::CIPHER_PATH;        // FdnGameType::CIPHER_PATH = 4
        case 5: return GameType::EXPLOIT_SEQUENCER;  // FdnGameType::EXPLOIT_SEQUENCER = 5
        case 6: return GameType::BREACH_DEFENSE;     // FdnGameType::BREACH_DEFENSE = 6
        default: return GameType::QUICKDRAW;         // Invalid — should never happen
    }
}

class GameLaunchState : public State {
public:
    /*
     * @param stateId   State ID within KonamiMetaGame
     * @param fdnIndex  FDN game index (0-6), used to derive GameType
     * @param mode      Launch mode (easy first, easy replay, hard)
     * @param player    Player reference for progress tracking
     */
    GameLaunchState(int stateId, int fdnIndex, LaunchMode mode, Player* player);
    ~GameLaunchState() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    void onStateResumed(Device* PDN, Snapshot* snapshot) override;

    bool transitionToButtonAwarded();
    bool transitionToBoonAwarded();
    bool transitionToGameOver();

private:
    int fdnIndex;
    LaunchMode mode;
    Player* player;
    GameType gameType;

    bool transitionToButtonAwardedState = false;
    bool transitionToBoonAwardedState = false;
    bool transitionToGameOverState = false;
    bool gameLaunched = false;
    bool gameReturned = false;
};
