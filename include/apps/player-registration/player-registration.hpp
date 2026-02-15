#pragma once
#include "state/state-machine.hpp"
#include "apps/player-registration/player-registration-states.hpp"

class PlayerRegistrationApp : public StateMachine {
public:
    PlayerRegistrationApp(Player* player, MatchManager* matchManager,
                          WirelessManager* wirelessManager,
                          RemoteDebugManager* remoteDebugManager,
                          ProgressManager* progressManager);
    ~PlayerRegistrationApp() override;
    void populateStateMap() override;
    bool readyForGameplay() const;

private:
    Player* player;
    MatchManager* matchManager;
    WirelessManager* wirelessManager;
    RemoteDebugManager* remoteDebugManager;
    ProgressManager* progressManager;
    bool registrationComplete = false;
};
