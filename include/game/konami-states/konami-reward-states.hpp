#pragma once

#include "state/state.hpp"
#include "game/player.hpp"
#include "game/fdn-game-type.hpp"
#include "game/progress-manager.hpp"
#include "device/device-types.hpp"
#include "utils/simple-timer.hpp"

/*
 * KmgButtonAwarded — Shown after first easy win within KonamiMetaGame.
 *
 * Awards the KonamiButton for the game that was just beaten.
 * Displays which button was earned and total progress.
 * Transitions to GameOverReturn after display timeout.
 *
 * Named KmgButtonAwarded to avoid ODR conflict with the older
 * KonamiButtonAwarded in quickdraw-states/ (used by legacy tests).
 */
class KmgButtonAwarded : public State {
public:
    KmgButtonAwarded(int stateId, Player* player, ProgressManager* progressManager);
    ~KmgButtonAwarded() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;

    bool transitionToGameOverReturn();

    // Set by the launching state before transition
    void setFdnIndex(int index) { fdnIndex = index; }

private:
    Player* player;
    ProgressManager* progressManager;
    SimpleTimer displayTimer;
    static constexpr int DISPLAY_DURATION_MS = 4000;
    bool transitionToGameOverReturnState = false;
    int fdnIndex = 0;
};

/*
 * KonamiBoonAwarded — Shown after hard mode win within KonamiMetaGame.
 *
 * Awards the color profile for the game that was just beaten on hard.
 * Displays which color palette was earned.
 * Transitions to GameOverReturn after display timeout.
 */
class KonamiBoonAwarded : public State {
public:
    KonamiBoonAwarded(int stateId, Player* player, ProgressManager* progressManager);
    ~KonamiBoonAwarded() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;

    bool transitionToGameOverReturn();

    // Set by the launching state before transition
    void setFdnIndex(int index) { fdnIndex = index; }

private:
    Player* player;
    ProgressManager* progressManager;
    SimpleTimer displayTimer;
    static constexpr int DISPLAY_DURATION_MS = 5000;
    bool transitionToGameOverReturnState = false;
    int fdnIndex = 0;
};

/*
 * KonamiGameOverReturn — Simple timeout that returns to Quickdraw.
 *
 * Displays a brief message, then returns to the previous app (Quickdraw)
 * after a short timeout.
 */
class KonamiGameOverReturn : public State {
public:
    explicit KonamiGameOverReturn(int stateId);
    ~KonamiGameOverReturn() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;

    bool transitionToReturn();

private:
    SimpleTimer displayTimer;
    static constexpr int DISPLAY_DURATION_MS = 2000;
    bool transitionToReturnState = false;
};
