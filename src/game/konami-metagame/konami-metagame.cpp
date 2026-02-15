#include "game/konami-metagame/konami-metagame.hpp"
#include "game/konami-metagame/konami-metagame-states.hpp"
#include "game/konami-states/konami-handshake.hpp"
#include "game/konami-states/game-launch-state.hpp"
#include "game/konami-states/mastery-replay.hpp"
#include "game/konami-states/konami-reward-states.hpp"
#include "game/konami-states/konami-code-entry.hpp"
#include "game/konami-states/konami-code-result.hpp"
#include "device/device-types.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "KonamiMetaGame";

KonamiMetaGame::KonamiMetaGame(Player* player, ProgressManager* progressManager) :
    StateMachine(KONAMI_METAGAME_APP_ID),
    player(player),
    progressManager(progressManager)
{
}

KonamiMetaGame::~KonamiMetaGame() {
    player = nullptr;
    progressManager = nullptr;
}

void KonamiMetaGame::populateStateMap() {
    /*
     * 35-state layout:
     * [0]     = KonamiHandshake — routes to correct state based on player progress
     * [1-7]   = EasyLaunch[0..6] — first encounter, EASY mode
     * [8-14]  = ReplayEasy[0..6] — replay with no reward
     * [15-21] = HardLaunch[0..6] — hard mode after Konami code
     * [22-28] = MasteryReplay[0..6] — mode selection (easy/hard)
     * [29]    = ButtonAwarded — shows earned button
     * [30]    = BoonAwarded — shows earned color profile
     * [31]    = GameOverReturn — returns to Quickdraw
     * [32]    = CodeEntry — 13-button Konami code puzzle
     * [33]    = CodeAccepted — hard mode unlocked
     * [34]    = CodeRejected — not enough buttons
     */

    // === State 0: KonamiHandshake ===
    auto* handshake = new KonamiHandshake(player);

    // === States 1-7: EasyLaunch (first encounter, EASY mode) ===
    GameLaunchState* easyLaunch[7];
    for (int i = 0; i < 7; i++) {
        easyLaunch[i] = new GameLaunchState(
            KONAMI_EASY_LAUNCH_START + i, i, LaunchMode::EASY_FIRST, player);
    }

    // === States 8-14: ReplayEasy (re-encounter, no reward) ===
    GameLaunchState* replayEasy[7];
    for (int i = 0; i < 7; i++) {
        replayEasy[i] = new GameLaunchState(
            KONAMI_REPLAY_EASY_START + i, i, LaunchMode::EASY_REPLAY, player);
    }

    // === States 15-21: HardLaunch (post-Konami, hard mode) ===
    GameLaunchState* hardLaunch[7];
    for (int i = 0; i < 7; i++) {
        hardLaunch[i] = new GameLaunchState(
            KONAMI_HARD_LAUNCH_START + i, i, LaunchMode::HARD_LAUNCH, player);
    }

    // === States 22-28: MasteryReplay (mode selection) ===
    MasteryReplay* masteryReplay[7];
    for (int i = 0; i < 7; i++) {
        GameType gameType = fdnGameTypeToGameType(i);
        masteryReplay[i] = new MasteryReplay(KONAMI_MASTERY_REPLAY_START + i, gameType);
    }

    // === State 29: ButtonAwarded ===
    auto* buttonAwarded = new KmgButtonAwarded(KONAMI_BUTTON_AWARDED, player, progressManager);

    // === State 30: BoonAwarded ===
    auto* boonAwarded = new KonamiBoonAwarded(KONAMI_BOON_AWARDED, player, progressManager);

    // === State 31: GameOverReturn ===
    auto* gameOverReturn = new KonamiGameOverReturn(KONAMI_GAME_OVER_RETURN);

    // === State 32: CodeEntry ===
    auto* codeEntry = new KonamiCodeEntry(player);

    // === State 33: CodeAccepted ===
    auto* codeAccepted = new KonamiCodeAccepted(player, progressManager);

    // === State 34: CodeRejected ===
    auto* codeRejected = new KonamiCodeRejected(player);

    // ================================================================
    // TRANSITIONS
    // ================================================================

    // KonamiHandshake uses skipToState() via a dynamic transition
    // It needs a transition for each possible target state
    // Since KonamiHandshake routes by index, we use a single transition
    // that checks shouldTransition() and skips to getTargetStateIndex()
    // This is handled by a special transition that calls skipToState()
    // on the KonamiMetaGame state machine.
    //
    // For simplicity, we wire transitions from Handshake to all possible
    // first targets. But since state machine only checks transitions
    // sequentially and commits the first match, and KonamiHandshake
    // needs dynamic routing, we need a workaround.
    //
    // Solution: We override onStateLoop in KonamiMetaGame to handle
    // KonamiHandshake's dynamic routing via skipToState().
    // This is simpler than wiring 35 transitions from state 0.

    // For all GameLaunchStates: transitions to reward or game over states
    for (int i = 0; i < 7; i++) {
        // EasyLaunch transitions
        easyLaunch[i]->addTransition(new StateTransition(
            std::bind(&GameLaunchState::transitionToButtonAwarded, easyLaunch[i]),
            buttonAwarded));
        easyLaunch[i]->addTransition(new StateTransition(
            std::bind(&GameLaunchState::transitionToGameOver, easyLaunch[i]),
            gameOverReturn));

        // ReplayEasy transitions (no button, just game over)
        replayEasy[i]->addTransition(new StateTransition(
            std::bind(&GameLaunchState::transitionToGameOver, replayEasy[i]),
            gameOverReturn));

        // HardLaunch transitions
        hardLaunch[i]->addTransition(new StateTransition(
            std::bind(&GameLaunchState::transitionToBoonAwarded, hardLaunch[i]),
            boonAwarded));
        hardLaunch[i]->addTransition(new StateTransition(
            std::bind(&GameLaunchState::transitionToGameOver, hardLaunch[i]),
            gameOverReturn));

        // MasteryReplay transitions to replay easy or hard launch
        masteryReplay[i]->addTransition(new StateTransition(
            std::bind(&MasteryReplay::transitionToEasyMode, masteryReplay[i]),
            replayEasy[i]));
        masteryReplay[i]->addTransition(new StateTransition(
            std::bind(&MasteryReplay::transitionToHardMode, masteryReplay[i]),
            hardLaunch[i]));
    }

    // ButtonAwarded → GameOverReturn
    buttonAwarded->addTransition(new StateTransition(
        std::bind(&KmgButtonAwarded::transitionToGameOverReturn, buttonAwarded),
        gameOverReturn));

    // BoonAwarded → GameOverReturn
    boonAwarded->addTransition(new StateTransition(
        std::bind(&KonamiBoonAwarded::transitionToGameOverReturn, boonAwarded),
        gameOverReturn));

    // GameOverReturn → terminal (dismount returns to Quickdraw)
    // GameOverReturn doesn't need a transition — it calls setActiveApp(QUICKDRAW) in dismount

    // CodeEntry → CodeAccepted or GameOverReturn (timeout)
    codeEntry->addTransition(new StateTransition(
        std::bind(&KonamiCodeEntry::transitionToAccepted, codeEntry),
        codeAccepted));
    codeEntry->addTransition(new StateTransition(
        std::bind(&KonamiCodeEntry::transitionToGameOver, codeEntry),
        gameOverReturn));

    // CodeAccepted → GameOverReturn (after display)
    codeAccepted->addTransition(new StateTransition(
        std::bind(&KonamiCodeAccepted::transitionToReturnQuickdraw, codeAccepted),
        gameOverReturn));

    // CodeRejected → GameOverReturn (after display)
    codeRejected->addTransition(new StateTransition(
        std::bind(&KonamiCodeRejected::transitionToReturnQuickdraw, codeRejected),
        gameOverReturn));

    // ================================================================
    // PUSH TO STATE MAP (order must match state indices!)
    // ================================================================

    stateMap.push_back(handshake);         // [0]

    for (int i = 0; i < 7; i++) {
        stateMap.push_back(easyLaunch[i]); // [1-7]
    }

    for (int i = 0; i < 7; i++) {
        stateMap.push_back(replayEasy[i]); // [8-14]
    }

    for (int i = 0; i < 7; i++) {
        stateMap.push_back(hardLaunch[i]); // [15-21]
    }

    for (int i = 0; i < 7; i++) {
        stateMap.push_back(masteryReplay[i]); // [22-28]
    }

    stateMap.push_back(buttonAwarded);     // [29]
    stateMap.push_back(boonAwarded);       // [30]
    stateMap.push_back(gameOverReturn);    // [31]
    stateMap.push_back(codeEntry);         // [32]
    stateMap.push_back(codeAccepted);      // [33]
    stateMap.push_back(codeRejected);      // [34]
}

void KonamiMetaGame::onStateLoop(Device* PDN) {
    /*
     * Override the default StateMachine loop to handle KonamiHandshake's
     * dynamic routing. KonamiHandshake determines the target state index
     * at runtime (not via static transitions), so we intercept here and
     * use skipToState() to jump directly to the calculated index.
     *
     * For all other states, we delegate to the standard loop.
     */
    if (currentState && currentState->getStateId() == KONAMI_HANDSHAKE) {
        currentState->onStateLoop(PDN);

        auto* handshake = static_cast<KonamiHandshake*>(currentState);
        if (handshake->shouldTransition()) {
            int targetIndex = handshake->getTargetStateIndex();

            if (targetIndex >= 0 && targetIndex < static_cast<int>(stateMap.size())) {
                LOG_I(TAG, "Handshake routing to state index %d", targetIndex);

                // Set fdnIndex on reward states so they know which game to award
                // The FDN game index is embedded in the target index for launch states
                int fdnIndex = -1;
                if (targetIndex >= KONAMI_EASY_LAUNCH_START && targetIndex <= KONAMI_EASY_LAUNCH_END) {
                    fdnIndex = targetIndex - KONAMI_EASY_LAUNCH_START;
                } else if (targetIndex >= KONAMI_REPLAY_EASY_START && targetIndex <= KONAMI_REPLAY_EASY_END) {
                    fdnIndex = targetIndex - KONAMI_REPLAY_EASY_START;
                } else if (targetIndex >= KONAMI_HARD_LAUNCH_START && targetIndex <= KONAMI_HARD_LAUNCH_END) {
                    fdnIndex = targetIndex - KONAMI_HARD_LAUNCH_START;
                } else if (targetIndex >= KONAMI_MASTERY_REPLAY_START && targetIndex <= KONAMI_MASTERY_REPLAY_END) {
                    fdnIndex = targetIndex - KONAMI_MASTERY_REPLAY_START;
                }

                // Set fdnIndex on reward states for when they're reached
                if (fdnIndex >= 0) {
                    auto* btnAwarded = static_cast<KmgButtonAwarded*>(stateMap[KONAMI_BUTTON_AWARDED]);
                    btnAwarded->setFdnIndex(fdnIndex);
                    auto* boonAwrd = static_cast<KonamiBoonAwarded*>(stateMap[KONAMI_BOON_AWARDED]);
                    boonAwrd->setFdnIndex(fdnIndex);
                }

                skipToState(PDN, targetIndex);
            } else {
                LOG_E(TAG, "Invalid target state index: %d", targetIndex);
            }
        }
        return;
    }

    // Default behavior for all other states
    StateMachine::onStateLoop(PDN);
}
