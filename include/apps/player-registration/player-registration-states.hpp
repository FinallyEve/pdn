#pragma once

#include "game/player.hpp"
#include "utils/simple-timer.hpp"
#include "state/state.hpp"
#include "wireless/quickdraw-wireless-manager.hpp"
#include "wireless/remote-debug-manager.hpp"
#include "game/match-manager.hpp"
#include "device/drivers/http-client-interface.hpp"
#include "game/quickdraw-resources.hpp"
#include "device/device-types.hpp"
#include <cstdlib>
#include <queue>
#include <string>
#include <vector>

// Forward declarations
class ProgressManager;

enum PlayerRegistrationStateId {
    PLAYER_REGISTRATION = 0,
    FETCH_USER_DATA = 1,
    CONFIRM_OFFLINE = 2,
    CHOOSE_ROLE = 3,
    WELCOME_MESSAGE = 4
};

class PlayerRegistration : public State {
public:
    PlayerRegistration(Player* player, MatchManager* matchManager);
    ~PlayerRegistration();

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN) override;
    void onStateDismounted(Device *PDN) override;
    bool transitionToUserFetch();

private:
    bool transitionToUserFetchState = false;
    bool shouldRender = false;
    Player* player;
    MatchManager* matchManager;
    int currentDigit = 0;
    int currentDigitIndex = 0;
    static constexpr int DIGIT_COUNT = 4;
    int inputId[DIGIT_COUNT] = {0, 0, 0, 0};
};

class FetchUserDataState : public State {
public:
    FetchUserDataState(Player* player, WirelessManager* wirelessManager, RemoteDebugManager* remoteDebugManager, ProgressManager* progressManager);
    ~FetchUserDataState();

    bool transitionToConfirmOffline();
    bool transitionToWelcomeMessage();
    bool transitionToUploadMatches();
    bool transitionToPlayerRegistration();
    void showLoadingGlyphs(Device *PDN);
    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN) override;
    void onStateDismounted(Device *PDN) override;

private:
    RemoteDebugManager* remoteDebugManager;
    ProgressManager* progressManager;
    bool transitionToPlayerRegistrationState = false;
    bool transitionToConfirmOfflineState = false;
    bool transitionToWelcomeMessageState = false;
    bool transitionToUploadMatchesState = false;
    WirelessManager* wirelessManager;
    bool isFetchingUserData = false;
    Player* player;
    SimpleTimer userDataFetchTimer;
    const int USER_DATA_FETCH_TIMEOUT = 10000;
};

class ConfirmOfflineState : public State {
public:
    explicit ConfirmOfflineState(Player* player);
    ~ConfirmOfflineState();

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN) override;
    void onStateDismounted(Device *PDN) override;
    bool transitionToChooseRole();
    bool transitionToPlayerRegistration();
    void renderUi(Device *PDN);
    int getDigitGlyphForIDIndex(int index);

private:
    Player* player;
    int uiPage = 0;
    const int UI_PAGE_COUNT = 3;
    const int UI_PAGE_TIMEOUT = 3000;
    bool finishedPaging = false;
    int menuIndex = 0;
    bool displayIsDirty = false;
    const int MENU_ITEM_COUNT = 2;
    SimpleTimer uiPageTimer;
    bool transitionToChooseRoleState = false;
    bool transitionToPlayerRegistrationState = false;
    parameterizedCallbackFunction confirmCallback;
    parameterizedCallbackFunction cancelCallback;
};

class ChooseRoleState : public State {
public:
    explicit ChooseRoleState(Player* player);
    ~ChooseRoleState();

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN) override;
    void onStateDismounted(Device *PDN) override;
    bool transitionToWelcomeMessage();
    void renderUi(Device *PDN);

private:
    Player* player;
    bool transitionToWelcomeMessageState = false;
    bool displayIsDirty = false;
    bool hunterSelected = true;
};

class WelcomeMessage : public State {
public:
    explicit WelcomeMessage(Player* player);
    ~WelcomeMessage();

    void onStateMounted(Device *PDN) override;
    void onStateLoop(Device *PDN) override;
    void onStateDismounted(Device *PDN) override;
    void renderWelcomeMessage(Device *PDN);
    bool transitionToGameplay();

private:
    Player* player;
    SimpleTimer welcomeMessageTimer;
    bool transitionToAwakenSequenceState = false;
    const int WELCOME_MESSAGE_TIMEOUT = 5000;
};
