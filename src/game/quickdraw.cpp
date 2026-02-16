#include "../../include/game/quickdraw.hpp"
#include <memory>

Quickdraw::Quickdraw(Player* player, Device* PDN, QuickdrawWirelessManager* quickdrawWirelessManager, RemoteDebugManager* remoteDebugManager): StateMachine(QUICKDRAW_APP_ID) {
    this->player = player;
    this->quickdrawWirelessManager = quickdrawWirelessManager;
    this->remoteDebugManager = remoteDebugManager;
    this->wirelessManager = PDN->getWirelessManager();
    this->matchManager = std::make_unique<MatchManager>();
    this->storageManager = PDN->getStorage();
    this->peerComms = PDN->getPeerComms();
    this->progressManager = std::make_unique<ProgressManager>();
    this->progressManager->initialize(player, storageManager);
    this->fdnResultManager = std::make_unique<FdnResultManager>();
    this->fdnResultManager->initialize(storageManager);
    PDN->setActiveComms(player->isHunter() ? SerialIdentifier::OUTPUT_JACK : SerialIdentifier::INPUT_JACK);
}

Quickdraw::~Quickdraw() {
    // Clean up states before managers to ensure proper destruction order.
    // Base class StateMachine will try to delete them too, but the vector will be empty.
    for (auto state: stateMap) {
        delete state;
    }
    stateMap.clear();

    player = nullptr;
    quickdrawWirelessManager = nullptr;
    storageManager = nullptr;
    peerComms = nullptr;
    matches.clear();
    // matchManager, progressManager, and fdnResultManager deleted automatically by unique_ptr
}

void Quickdraw::populateStateMap() {
    matchManager->initialize(player, storageManager, peerComms, quickdrawWirelessManager);

    PlayerRegistration* playerRegistration = std::make_unique<PlayerRegistration>(player, matchManager.get()).release();
    stateMap.push_back(playerRegistration);

    FetchUserDataState* fetchUserData = std::make_unique<FetchUserDataState>(player, wirelessManager, remoteDebugManager, progressManager.get()).release();
    stateMap.push_back(fetchUserData);

    WelcomeMessage* welcomeMessage = std::make_unique<WelcomeMessage>(player).release();
    stateMap.push_back(welcomeMessage);

    ConfirmOfflineState* confirmOffline = std::make_unique<ConfirmOfflineState>(player).release();
    stateMap.push_back(confirmOffline);

    ChooseRoleState* chooseRole = std::make_unique<ChooseRoleState>(player).release();
    stateMap.push_back(chooseRole);

    AwakenSequence* awakenSequence = std::make_unique<AwakenSequence>(player).release();
    stateMap.push_back(awakenSequence);

    Idle* idle = std::make_unique<Idle>(player, matchManager.get(), quickdrawWirelessManager, progressManager.get()).release();
    stateMap.push_back(idle);

    HandshakeInitiateState* handshakeInitiate = std::make_unique<HandshakeInitiateState>(player).release();
    stateMap.push_back(handshakeInitiate);

    BountySendConnectionConfirmedState* bountySendCC = std::make_unique<BountySendConnectionConfirmedState>(player, matchManager.get(), quickdrawWirelessManager).release();
    stateMap.push_back(bountySendCC);

    HunterSendIdState* hunterSendId = std::make_unique<HunterSendIdState>(player, matchManager.get(), quickdrawWirelessManager).release();
    stateMap.push_back(hunterSendId);

    ConnectionSuccessful* connectionSuccessful = std::make_unique<ConnectionSuccessful>(player).release();
    stateMap.push_back(connectionSuccessful);

    DuelCountdown* duelCountdown = std::make_unique<DuelCountdown>(player, matchManager.get()).release();
    stateMap.push_back(duelCountdown);

    Duel* duel = std::make_unique<Duel>(player, matchManager.get(), quickdrawWirelessManager).release();
    stateMap.push_back(duel);

    DuelPushed* duelPushed = std::make_unique<DuelPushed>(player, matchManager.get()).release();
    stateMap.push_back(duelPushed);

    DuelReceivedResult* duelReceivedResult = std::make_unique<DuelReceivedResult>(player, matchManager.get(), quickdrawWirelessManager).release();
    stateMap.push_back(duelReceivedResult);

    DuelResult* duelResult = std::make_unique<DuelResult>(player, matchManager.get(), quickdrawWirelessManager).release();
    stateMap.push_back(duelResult);

    Win* win = std::make_unique<Win>(player).release();
    stateMap.push_back(win);

    Lose* lose = std::make_unique<Lose>(player).release();
    stateMap.push_back(lose);

    Sleep* sleep = std::make_unique<Sleep>(player).release();
    stateMap.push_back(sleep);

    UploadMatchesState* uploadMatches = std::make_unique<UploadMatchesState>(player, wirelessManager, matchManager.get(), fdnResultManager.get()).release();
    stateMap.push_back(uploadMatches);

    FdnDetected* fdnDetected = std::make_unique<FdnDetected>(player, &difficultyScaler).release();
    stateMap.push_back(fdnDetected);

    FdnReencounter* fdnReencounter = std::make_unique<FdnReencounter>(player, &difficultyScaler).release();
    stateMap.push_back(fdnReencounter);

    FdnComplete* fdnComplete = std::make_unique<FdnComplete>(player, progressManager.get(), fdnResultManager.get(), &difficultyScaler).release();
    stateMap.push_back(fdnComplete);

    ColorProfilePrompt* colorProfilePrompt = std::make_unique<ColorProfilePrompt>(player, progressManager.get()).release();
    stateMap.push_back(colorProfilePrompt);

    ColorProfilePicker* colorProfilePicker = std::make_unique<ColorProfilePicker>(player, progressManager.get()).release();
    stateMap.push_back(colorProfilePicker);

    BoonAwarded* boonAwarded = std::make_unique<BoonAwarded>(player, progressManager.get()).release();
    stateMap.push_back(boonAwarded);

    KonamiPuzzle* konamiPuzzle = std::make_unique<KonamiPuzzle>(player).release();
    stateMap.push_back(konamiPuzzle);

    ConnectionLost* connectionLost = std::make_unique<ConnectionLost>(player).release();
    stateMap.push_back(connectionLost);

    KonamiCodeEntry* konamiCodeEntry = std::make_unique<KonamiCodeEntry>(player).release();
    stateMap.push_back(konamiCodeEntry);

    KonamiCodeAccepted* konamiCodeAccepted = std::make_unique<KonamiCodeAccepted>(player, progressManager.get()).release();
    stateMap.push_back(konamiCodeAccepted);

    KonamiCodeRejected* konamiCodeRejected = std::make_unique<KonamiCodeRejected>(player).release();
    stateMap.push_back(konamiCodeRejected);

    GameOverReturnIdle* gameOverReturnIdle = std::make_unique<GameOverReturnIdle>(player).release();
    stateMap.push_back(gameOverReturnIdle);

    playerRegistration->addTransition(
        new StateTransition(
            std::bind(&PlayerRegistration::transitionToUserFetch, playerRegistration),
            fetchUserData));

    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToConfirmOffline, fetchUserData),
            confirmOffline));

    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToUploadMatches, fetchUserData),
            uploadMatches));

    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToWelcomeMessage, fetchUserData),
            welcomeMessage));

    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToPlayerRegistration, fetchUserData),
            playerRegistration));

    confirmOffline->addTransition(
        new StateTransition(
            std::bind(&ConfirmOfflineState::transitionToChooseRole, confirmOffline),
            chooseRole));

    confirmOffline->addTransition(
        new StateTransition(
            std::bind(&ConfirmOfflineState::transitionToPlayerRegistration, confirmOffline),
            playerRegistration));

    chooseRole->addTransition(
        new StateTransition(
            std::bind(&ChooseRoleState::transitionToAllegiancePicker, chooseRole),
            welcomeMessage));

    welcomeMessage->addTransition(
        new StateTransition(
            std::bind(&WelcomeMessage::transitionToGameplay, welcomeMessage),
            awakenSequence));

    awakenSequence->addTransition(
        new StateTransition(
            std::bind(&AwakenSequence::transitionToIdle, awakenSequence),
            idle));

    idle->addTransition(
        new StateTransition(
            std::bind(&Idle::transitionToColorPicker, idle),
            colorProfilePicker));

    idle->addTransition(
        new StateTransition(
            std::bind(&Idle::isFdnDetected, idle),
            fdnDetected));

    idle->addTransition(
        new StateTransition(
            std::bind(&Idle::transitionToHandshake, idle),
            handshakeInitiate));

    fdnDetected->addTransition(
        new StateTransition(
            std::bind(&FdnDetected::transitionToKonamiPuzzle, fdnDetected),
            konamiPuzzle));

    fdnDetected->addTransition(
        new StateTransition(
            std::bind(&FdnDetected::transitionToReencounter, fdnDetected),
            fdnReencounter));

    fdnDetected->addTransition(
        new StateTransition(
            std::bind(&FdnDetected::transitionToFdnComplete, fdnDetected),
            fdnComplete));

    fdnDetected->addTransition(
        new StateTransition(
            std::bind(&FdnDetected::transitionToConnectionLost, fdnDetected),
            connectionLost));

    fdnDetected->addTransition(
        new StateTransition(
            std::bind(&FdnDetected::transitionToIdle, fdnDetected),
            idle));

    connectionLost->addTransition(
        new StateTransition(
            std::bind(&ConnectionLost::transitionToIdle, connectionLost),
            idle));

    fdnReencounter->addTransition(
        new StateTransition(
            std::bind(&FdnReencounter::transitionToFdnComplete, fdnReencounter),
            fdnComplete));

    fdnReencounter->addTransition(
        new StateTransition(
            std::bind(&FdnReencounter::transitionToIdle, fdnReencounter),
            idle));

    fdnComplete->addTransition(
        new StateTransition(
            std::bind(&FdnComplete::transitionToBoonAwarded, fdnComplete),
            boonAwarded));

    fdnComplete->addTransition(
        new StateTransition(
            std::bind(&FdnComplete::transitionToColorPrompt, fdnComplete),
            colorProfilePrompt));

    fdnComplete->addTransition(
        new StateTransition(
            std::bind(&FdnComplete::transitionToIdle, fdnComplete),
            idle));

    colorProfilePrompt->addTransition(
        new StateTransition(
            std::bind(&ColorProfilePrompt::transitionToIdle, colorProfilePrompt),
            idle));

    colorProfilePicker->addTransition(
        new StateTransition(
            std::bind(&ColorProfilePicker::transitionToIdle, colorProfilePicker),
            idle));

    boonAwarded->addTransition(
        new StateTransition(
            std::bind(&BoonAwarded::transitionToColorPrompt, boonAwarded),
            colorProfilePrompt));

    konamiPuzzle->addTransition(
        new StateTransition(
            std::bind(&KonamiPuzzle::transitionToIdle, konamiPuzzle),
            idle));

    konamiCodeEntry->addTransition(
        new StateTransition(
            std::bind(&KonamiCodeEntry::transitionToAccepted, konamiCodeEntry),
            konamiCodeAccepted));

    konamiCodeEntry->addTransition(
        new StateTransition(
            std::bind(&KonamiCodeEntry::transitionToGameOver, konamiCodeEntry),
            gameOverReturnIdle));

    konamiCodeAccepted->addTransition(
        new StateTransition(
            std::bind(&KonamiCodeAccepted::transitionToReturnQuickdraw, konamiCodeAccepted),
            idle));

    konamiCodeRejected->addTransition(
        new StateTransition(
            std::bind(&KonamiCodeRejected::transitionToReturnQuickdraw, konamiCodeRejected),
            idle));

    gameOverReturnIdle->addTransition(
        new StateTransition(
            std::bind(&GameOverReturnIdle::transitionToReturnQuickdraw, gameOverReturnIdle),
            idle));

    handshakeInitiate->addTransition(
        new StateTransition(
            std::bind(&HandshakeInitiateState::transitionToBountySendCC, handshakeInitiate),
            bountySendCC));
    
    handshakeInitiate->addTransition(
        new StateTransition(
            std::bind(&HandshakeInitiateState::transitionToHunterSendId, handshakeInitiate),
            hunterSendId));
    
    handshakeInitiate->addTransition(
        new StateTransition(
            std::bind(&BaseHandshakeState::transitionToIdle, handshakeInitiate),
            idle));
    
    bountySendCC->addTransition(
        new StateTransition(
            std::bind(&BountySendConnectionConfirmedState::transitionToConnectionSuccessful, bountySendCC),
            connectionSuccessful));
    
    bountySendCC->addTransition(
        new StateTransition(
            std::bind(&BaseHandshakeState::transitionToIdle, bountySendCC),
            idle));
    
    hunterSendId->addTransition(
        new StateTransition(
            std::bind(&HunterSendIdState::transitionToConnectionSuccessful, hunterSendId),
            connectionSuccessful));
    
    hunterSendId->addTransition(
        new StateTransition(
            std::bind(&BaseHandshakeState::transitionToIdle, hunterSendId),
            idle));

    connectionSuccessful->addTransition(
        new StateTransition(
            std::bind(&ConnectionSuccessful::transitionToCountdown, connectionSuccessful),
            duelCountdown));

    duelCountdown->addTransition(
        new StateTransition(
            std::bind(&DuelCountdown::shallWeBattle, duelCountdown),
            duel));

    duel->addTransition(
        new StateTransition(
            std::bind(&Duel::transitionToIdle, duel),
            idle));

    duel->addTransition(
        new StateTransition(
            std::bind(&Duel::transitionToDuelReceivedResult, duel),
            duelReceivedResult));

    duel->addTransition(
        new StateTransition(
            std::bind(&Duel::transitionToDuelPushed, duel),
            duelPushed));

    duelPushed->addTransition(
        new StateTransition(
            std::bind(&DuelPushed::transitionToDuelResult, duelPushed),
            duelResult));

    duelReceivedResult->addTransition(
        new StateTransition(
            std::bind(&DuelReceivedResult::transitionToDuelResult, duelReceivedResult),
            duelResult));

    duelResult->addTransition(
        new StateTransition(
            std::bind(&DuelResult::transitionToWin, duelResult),
            win));

    duelResult->addTransition(
        new StateTransition(
            std::bind(&DuelResult::transitionToLose, duelResult),
            lose));

    win->addTransition(
        new StateTransition(
            std::bind(&Win::resetGame, win),
            uploadMatches));

    lose->addTransition(
        new StateTransition(
            std::bind(&Lose::resetGame, lose),
            uploadMatches));

    uploadMatches->addTransition(
        new StateTransition(
            std::bind(&UploadMatchesState::transitionToSleep, uploadMatches),
            sleep));

    uploadMatches->addTransition(
        new StateTransition(
            std::bind(&UploadMatchesState::transitionToPlayerRegistration, uploadMatches),
            playerRegistration));

    sleep->addTransition(
        new StateTransition(
            std::bind(&Sleep::transitionToAwakenSequence, sleep),
            awakenSequence));

    stateMap.push_back(playerRegistration);
    stateMap.push_back(fetchUserData);
    stateMap.push_back(confirmOffline);
    stateMap.push_back(chooseRole);
    stateMap.push_back(welcomeMessage);
    stateMap.push_back(awakenSequence);
    stateMap.push_back(idle);
    stateMap.push_back(handshakeInitiate);
    stateMap.push_back(bountySendCC);
    stateMap.push_back(hunterSendId);
    stateMap.push_back(connectionSuccessful);
    stateMap.push_back(duelCountdown);
    stateMap.push_back(duel);
    stateMap.push_back(duelPushed);
    stateMap.push_back(duelReceivedResult);
    stateMap.push_back(duelResult);
    stateMap.push_back(win);
    stateMap.push_back(lose);
    stateMap.push_back(uploadMatches);
    stateMap.push_back(sleep);
    stateMap.push_back(fdnDetected);
    stateMap.push_back(fdnReencounter);
    stateMap.push_back(fdnComplete);
    stateMap.push_back(colorProfilePrompt);
    stateMap.push_back(colorProfilePicker);
    stateMap.push_back(boonAwarded);
    stateMap.push_back(konamiPuzzle);
    stateMap.push_back(connectionLost);
    stateMap.push_back(konamiCodeEntry);
    stateMap.push_back(konamiCodeAccepted);
    stateMap.push_back(konamiCodeRejected);
    stateMap.push_back(gameOverReturnIdle);
}

Image Quickdraw::getImageForAllegiance(Allegiance allegiance, ImageType whichImage) {
    return getCollectionForAllegiance(allegiance)->at(whichImage);
}
