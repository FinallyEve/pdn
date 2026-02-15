#include "game/konami-states/konami-reward-states.hpp"
#include "game/konami-states/game-launch-state.hpp"
#include "game/quickdraw.hpp"
#include "device/device.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "KonamiReward";

// ============================================================================
// KmgButtonAwarded
// ============================================================================

KmgButtonAwarded::KmgButtonAwarded(int stateId, Player* player, ProgressManager* progressManager) :
    State(stateId),
    player(player),
    progressManager(progressManager)
{
}

KmgButtonAwarded::~KmgButtonAwarded() {
    player = nullptr;
    progressManager = nullptr;
}

void KmgButtonAwarded::onStateMounted(Device* PDN) {
    transitionToGameOverReturnState = false;

    // Convert FDN index to GameType and get the reward button
    GameType gameType = fdnGameTypeToGameType(fdnIndex);
    KonamiButton reward = getRewardForGame(gameType);
    uint8_t buttonIndex = static_cast<uint8_t>(reward);

    // Award the button
    player->unlockKonamiButton(buttonIndex);

    // Save progress
    if (progressManager) {
        progressManager->saveProgress();
    }

    // Count total buttons
    int buttonCount = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (player->hasUnlockedButton(i)) buttonCount++;
    }

    LOG_I(TAG, "Button awarded: %s (%d/7)", getKonamiButtonName(reward), buttonCount);

    // Display victory
    PDN->getDisplay()->invalidateScreen();
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT);
    PDN->getDisplay()->drawText("BUTTON UNLOCKED!", 5, 10);
    PDN->getDisplay()->drawText(getKonamiButtonName(reward), 40, 25);

    char progress[32];
    snprintf(progress, sizeof(progress), "%d of 7 collected", buttonCount);
    PDN->getDisplay()->drawText(progress, 15, 40);

    if (buttonCount >= 7) {
        PDN->getDisplay()->drawText("Find the 8th FDN!", 5, 55);
    }
    PDN->getDisplay()->render();

    // Victory LED
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 0;
    config.initialState = LEDState();
    config.initialState.transmitLight = LEDState::SingleLEDState(LEDColor(0, 255, 0), 255);
    PDN->getLightManager()->startAnimation(config);

    PDN->getHaptics()->setIntensity(200);

    displayTimer.setTimer(DISPLAY_DURATION_MS);
}

void KmgButtonAwarded::onStateLoop(Device* PDN) {
    displayTimer.updateTime();

    if (displayTimer.expired()) {
        transitionToGameOverReturnState = true;
    }
    (void)PDN;
}

void KmgButtonAwarded::onStateDismounted(Device* PDN) {
    displayTimer.invalidate();
    PDN->getHaptics()->off();
    PDN->getLightManager()->stopAnimation();
    PDN->getLightManager()->clear();
}

bool KmgButtonAwarded::transitionToGameOverReturn() {
    return transitionToGameOverReturnState;
}

// ============================================================================
// KonamiBoonAwarded
// ============================================================================

KonamiBoonAwarded::KonamiBoonAwarded(int stateId, Player* player, ProgressManager* progressManager) :
    State(stateId),
    player(player),
    progressManager(progressManager)
{
}

KonamiBoonAwarded::~KonamiBoonAwarded() {
    player = nullptr;
    progressManager = nullptr;
}

void KonamiBoonAwarded::onStateMounted(Device* PDN) {
    transitionToGameOverReturnState = false;

    // Convert FDN index to GameType
    GameType gameType = fdnGameTypeToGameType(fdnIndex);

    // Award color profile eligibility
    player->addColorProfileEligibility(static_cast<int>(gameType));

    // Save progress
    if (progressManager) {
        progressManager->saveProgress();
    }

    LOG_I(TAG, "Boon awarded: %s color profile", getGameDisplayName(gameType));

    // Display victory
    PDN->getDisplay()->invalidateScreen();
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT);
    PDN->getDisplay()->drawText("POWER STOLEN!", 10, 10);
    PDN->getDisplay()->drawText(getGameDisplayName(gameType), 5, 25);
    PDN->getDisplay()->drawText("Color palette", 15, 40);
    PDN->getDisplay()->drawText("unlocked!", 25, 50);
    PDN->getDisplay()->render();

    // Gold LED
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 0;
    config.initialState = LEDState();
    config.initialState.transmitLight = LEDState::SingleLEDState(LEDColor(255, 215, 0), 255);
    PDN->getLightManager()->startAnimation(config);

    PDN->getHaptics()->setIntensity(255);

    displayTimer.setTimer(DISPLAY_DURATION_MS);
}

void KonamiBoonAwarded::onStateLoop(Device* PDN) {
    displayTimer.updateTime();

    if (displayTimer.expired()) {
        transitionToGameOverReturnState = true;
    }
    (void)PDN;
}

void KonamiBoonAwarded::onStateDismounted(Device* PDN) {
    displayTimer.invalidate();
    PDN->getHaptics()->off();
    PDN->getLightManager()->stopAnimation();
    PDN->getLightManager()->clear();
}

bool KonamiBoonAwarded::transitionToGameOverReturn() {
    return transitionToGameOverReturnState;
}

// ============================================================================
// KonamiGameOverReturn
// ============================================================================

KonamiGameOverReturn::KonamiGameOverReturn(int stateId) :
    State(stateId)
{
}

KonamiGameOverReturn::~KonamiGameOverReturn() {
}

void KonamiGameOverReturn::onStateMounted(Device* PDN) {
    transitionToReturnState = false;

    PDN->getDisplay()->invalidateScreen();
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT);
    PDN->getDisplay()->drawText("RETURNING...", 15, 30);
    PDN->getDisplay()->render();

    displayTimer.setTimer(DISPLAY_DURATION_MS);
}

void KonamiGameOverReturn::onStateLoop(Device* PDN) {
    displayTimer.updateTime();

    if (displayTimer.expired()) {
        transitionToReturnState = true;
    }
    (void)PDN;
}

void KonamiGameOverReturn::onStateDismounted(Device* PDN) {
    displayTimer.invalidate();

    // Return to Quickdraw
    PDN->setActiveApp(StateId(QUICKDRAW_APP_ID));
}

bool KonamiGameOverReturn::transitionToReturn() {
    return transitionToReturnState;
}
