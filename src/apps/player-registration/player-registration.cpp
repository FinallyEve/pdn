#include "apps/player-registration/player-registration.hpp"
#include "game/progress-manager.hpp"

/*
 * PlayerRegistrationApp - Standalone player registration flow
 *
 * This app encapsulates the player registration states that were previously
 * part of Quickdraw. It provides a clean separation of concerns and allows
 * registration to be reused across different game modes.
 *
 * Current Status (Wave 10, Phase 1):
 * - App structure created
 * - State wiring defined
 * - Relies on existing Quickdraw state implementations
 *
 * TODO (Phase 2):
 * - Move state implementations from src/game/quickdraw-states/ to src/apps/player-registration/
 * - Update ChooseRoleState to transition directly to WelcomeMessage (skip AllegiancePicker)
 * - Remove AllegiancePickerState references from Quickdraw
 * - Wire Quickdraw to use PlayerRegistrationApp
 */

PlayerRegistrationApp::PlayerRegistrationApp(Player* player, MatchManager* matchManager,
                                             WirelessManager* wirelessManager,
                                             RemoteDebugManager* remoteDebugManager,
                                             ProgressManager* progressManager)
    : StateMachine(0) {
    this->player = player;
    this->matchManager = matchManager;
    this->wirelessManager = wirelessManager;
    this->remoteDebugManager = remoteDebugManager;
    this->progressManager = progressManager;
}

PlayerRegistrationApp::~PlayerRegistrationApp() {
    player = nullptr;
    matchManager = nullptr;
    wirelessManager = nullptr;
    remoteDebugManager = nullptr;
    progressManager = nullptr;
}

void PlayerRegistrationApp::populateStateMap() {
    // Create registration states (currently using Quickdraw implementations)
    PlayerRegistration* playerRegistration = new PlayerRegistration(player, matchManager);
    FetchUserDataState* fetchUserData = new FetchUserDataState(player, wirelessManager, remoteDebugManager, progressManager);
    ConfirmOfflineState* confirmOffline = new ConfirmOfflineState(player);
    ChooseRoleState* chooseRole = new ChooseRoleState(player);
    WelcomeMessage* welcomeMessage = new WelcomeMessage(player);

    // PlayerRegistration → FetchUserData
    playerRegistration->addTransition(
        new StateTransition(
            std::bind(&PlayerRegistration::transitionToUserFetch, playerRegistration),
            fetchUserData));

    // FetchUserData → ConfirmOffline (offline path)
    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToConfirmOffline, fetchUserData),
            confirmOffline));

    // FetchUserData → WelcomeMessage (online path)
    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToWelcomeMessage, fetchUserData),
            welcomeMessage));

    // FetchUserData → PlayerRegistration (retry path)
    fetchUserData->addTransition(
        new StateTransition(
            std::bind(&FetchUserDataState::transitionToPlayerRegistration, fetchUserData),
            playerRegistration));

    // ConfirmOffline → ChooseRole
    confirmOffline->addTransition(
        new StateTransition(
            std::bind(&ConfirmOfflineState::transitionToChooseRole, confirmOffline),
            chooseRole));

    // ConfirmOffline → PlayerRegistration (reset path)
    confirmOffline->addTransition(
        new StateTransition(
            std::bind(&ConfirmOfflineState::transitionToPlayerRegistration, confirmOffline),
            playerRegistration));

    // ChooseRole → WelcomeMessage
    // NOTE: Phase 2 will update ChooseRoleState to have transitionToWelcomeMessage()
    // For now, this transition is commented out because ChooseRoleState::transitionToAllegiancePicker
    // still exists and we haven't updated the state yet.
    // Phase 2 will uncomment this and add the proper transition method.
    /*
    chooseRole->addTransition(
        new StateTransition(
            std::bind(&ChooseRoleState::transitionToWelcomeMessage, chooseRole),
            welcomeMessage));
    */

    // Add all states to the state map
    stateMap.push_back(playerRegistration);
    stateMap.push_back(fetchUserData);
    stateMap.push_back(confirmOffline);
    stateMap.push_back(chooseRole);
    stateMap.push_back(welcomeMessage);
}

bool PlayerRegistrationApp::readyForGameplay() const {
    // Phase 2 will implement proper completion signaling
    return registrationComplete;
}
