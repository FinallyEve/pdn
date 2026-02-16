#include "game/konami-metagame/konami-metagame.hpp"
#include "game/konami-metagame/konami-metagame-states.hpp"
#include "game/konami-states/konami-handshake.hpp"
#include "game/konami-states/game-launch-state.hpp"
#include "game/konami-states/konami-reward-states.hpp"
#include "game/konami-states/konami-code-entry.hpp"
#include "game/konami-states/konami-code-result.hpp"
#include "game/konami-states/mastery-replay.hpp"
#include "game/progress-manager.hpp"
#include <memory>

KonamiMetaGame::KonamiMetaGame(Player* player) :
    StateMachine(KONAMI_METAGAME_APP_ID),
    player(player)
{
}

KonamiMetaGame::~KonamiMetaGame() {
    player = nullptr;
}

void KonamiMetaGame::populateStateMap() {
    /*
     * KonamiMetaGame State Map - 35 states total
     *
     * Layout:
     * [0] = Handshake (routing brain)
     * [1-7] = EasyLaunch (first encounter, awards button on win)
     * [8-14] = ReplayEasy (button already earned, no rewards)
     * [15-21] = HardLaunch (hard mode, awards boon on win)
     * [22-28] = MasteryReplay (mode select menu for mastered games)
     * [29] = ButtonAwarded (celebration + save)
     * [30] = BoonAwarded (celebration + save)
     * [31] = GameOverReturn (returns to Quickdraw Idle)
     * [32] = CodeEntry (13-button Konami code input)
     * [33] = CodeAccepted (unlocks hard mode)
     * [34] = CodeRejected (insufficient buttons)
     */

    // Get ProgressManager for reward states
    ProgressManager* progressManager = nullptr;  // TODO: Wire from device context

    // [0] Handshake - routing brain
    stateMap.push_back(std::make_unique<KonamiHandshake>(player).release());

    // [1-7] EasyLaunch - first encounter with each game
    // Order: SIGNAL_ECHO, GHOST_RUNNER, SPIKE_VECTOR, FIREWALL_DECRYPT, CIPHER_PATH, EXPLOIT_SEQUENCER, BREACH_DEFENSE
    const GameType easyGames[7] = {
        GameType::SIGNAL_ECHO, GameType::GHOST_RUNNER, GameType::SPIKE_VECTOR,
        GameType::FIREWALL_DECRYPT, GameType::CIPHER_PATH, GameType::EXPLOIT_SEQUENCER,
        GameType::BREACH_DEFENSE
    };
    for (int i = 0; i < 7; i++) {
        stateMap.push_back(std::make_unique<GameLaunchState>(1 + i, easyGames[i], LaunchMode::EASY_FIRST_ENCOUNTER).release());
    }

    // [8-14] ReplayEasy - button already earned
    for (int i = 0; i < 7; i++) {
        stateMap.push_back(std::make_unique<GameLaunchState>(8 + i, easyGames[i], LaunchMode::EASY_REPLAY).release());
    }

    // [15-21] HardLaunch - hard mode unlocked
    for (int i = 0; i < 7; i++) {
        stateMap.push_back(std::make_unique<GameLaunchState>(15 + i, easyGames[i], LaunchMode::HARD_FIRST_ENCOUNTER).release());
    }

    // [22-28] MasteryReplay - mode select menu
    for (int i = 0; i < 7; i++) {
        stateMap.push_back(std::make_unique<MasteryReplay>(22 + i, easyGames[i]).release());
    }

    // [29] ButtonAwarded
    stateMap.push_back(std::make_unique<KmgButtonAwarded>(player, progressManager).release());

    // [30] BoonAwarded
    stateMap.push_back(std::make_unique<KonamiBoonAwarded>(player, progressManager).release());

    // [31] GameOverReturn
    stateMap.push_back(std::make_unique<KonamiGameOverReturn>(player).release());

    // [32] CodeEntry
    stateMap.push_back(std::make_unique<KonamiCodeEntry>(player).release());

    // [33] CodeAccepted
    stateMap.push_back(std::make_unique<KonamiCodeAccepted>(player, progressManager).release());

    // [34] CodeRejected
    stateMap.push_back(std::make_unique<KonamiCodeRejected>(player).release());

    // Wire up all transitions
    wireTransitions();
}

void KonamiMetaGame::wireTransitions() {
    /*
     * Transition wiring for all 35 states
     */

    // [0] Handshake - uses getTargetStateIndex() for dynamic routing
    // Transitions handled by onStateLoop checking shouldTransition()
    // and using setCurrentState(getTargetStateIndex())

    // [1-7] EasyLaunch transitions
    for (int i = 1; i <= 7; i++) {
        GameLaunchState* state = static_cast<GameLaunchState*>(stateMap[i]);
        if (state) {
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToButtonAwarded(); },
                stateMap[29]  // ButtonAwarded
            ));
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToGameOver(); },
                stateMap[31]  // GameOverReturn
            ));
        }
    }

    // [8-14] ReplayEasy transitions - always go to GameOverReturn
    for (int i = 8; i <= 14; i++) {
        GameLaunchState* state = static_cast<GameLaunchState*>(stateMap[i]);
        if (state) {
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToGameOver(); },
                stateMap[31]  // GameOverReturn
            ));
        }
    }

    // [15-21] HardLaunch transitions
    for (int i = 15; i <= 21; i++) {
        GameLaunchState* state = static_cast<GameLaunchState*>(stateMap[i]);
        if (state) {
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToBoonAwarded(); },
                stateMap[30]  // BoonAwarded
            ));
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToGameOver(); },
                stateMap[31]  // GameOverReturn
            ));
        }
    }

    // [22-28] MasteryReplay transitions
    for (int i = 22; i <= 28; i++) {
        MasteryReplay* state = static_cast<MasteryReplay*>(stateMap[i]);
        if (state) {
            int gameIndex = i - 22;
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToEasyMode(); },
                stateMap[8 + (i - 22)]  // ReplayEasy[gameIndex]
            ));
            state->addTransition(new StateTransition(
                [state]() { return state->transitionToHardMode(); },
                stateMap[15 + (i - 22)]  // HardLaunch[gameIndex]
            ));
        }
    }

    // [29] ButtonAwarded → GameOverReturn
    KmgButtonAwarded* buttonAwarded = static_cast<KmgButtonAwarded*>(stateMap[29]);
    if (buttonAwarded) {
        buttonAwarded->addTransition(new StateTransition(
            [buttonAwarded]() { return buttonAwarded->transitionToGameOver(); },
            stateMap[31]  // GameOverReturn
        ));
    }

    // [30] BoonAwarded → GameOverReturn
    KonamiBoonAwarded* boonAwarded = static_cast<KonamiBoonAwarded*>(stateMap[30]);
    if (boonAwarded) {
        boonAwarded->addTransition(new StateTransition(
            [boonAwarded]() { return boonAwarded->transitionToGameOver(); },
            stateMap[31]  // GameOverReturn
        ));
    }

    // [31] GameOverReturn - no transitions, calls setActiveApp(0) in onStateLoop

    // [32] CodeEntry transitions
    KonamiCodeEntry* codeEntry = static_cast<KonamiCodeEntry*>(stateMap[32]);
    if (codeEntry) {
        codeEntry->addTransition(new StateTransition(
            [codeEntry]() { return codeEntry->transitionToAccepted(); },
            stateMap[33]  // CodeAccepted
        ));
        codeEntry->addTransition(new StateTransition(
            [codeEntry]() { return codeEntry->transitionToGameOver(); },
            stateMap[31]  // GameOverReturn
        ));
    }

    // [33] CodeAccepted → returns to Quickdraw via setActiveApp
    KonamiCodeAccepted* codeAccepted = static_cast<KonamiCodeAccepted*>(stateMap[33]);
    if (codeAccepted) {
        codeAccepted->addTransition(new StateTransition(
            [codeAccepted]() { return codeAccepted->transitionToReturnQuickdraw(); },
            stateMap[31]  // GameOverReturn (which then calls setActiveApp)
        ));
    }

    // [34] CodeRejected → returns to Quickdraw via setActiveApp
    KonamiCodeRejected* codeRejected = static_cast<KonamiCodeRejected*>(stateMap[34]);
    if (codeRejected) {
        codeRejected->addTransition(new StateTransition(
            [codeRejected]() { return codeRejected->transitionToReturnQuickdraw(); },
            stateMap[31]  // GameOverReturn
        ));
    }
}

void KonamiMetaGame::onStateLoop(Device* PDN) {
    // Special handling for Handshake state (index 0) before base onStateLoop
    // Handshake uses dynamic routing via getTargetStateIndex()
    if (currentState && currentState->getStateId() == 0) {
        KonamiHandshake* handshake = static_cast<KonamiHandshake*>(currentState);
        if (handshake && handshake->shouldTransition()) {
            int targetIndex = handshake->getTargetStateIndex();
            if (targetIndex >= 0 && targetIndex < static_cast<int>(stateMap.size())) {
                // Use skipToState to properly transition
                skipToState(PDN, targetIndex);
                return;  // Don't call base onStateLoop after manual transition
            }
        }
    }

    // Call base implementation for standard state machine flow
    StateMachine::onStateLoop(PDN);
}
