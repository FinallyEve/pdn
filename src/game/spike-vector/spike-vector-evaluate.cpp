#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "device/drivers/logger.hpp"

static const char* TAG = "SpikeVectorEvaluate";

SpikeVectorEvaluate::SpikeVectorEvaluate(SpikeVector* game) : State(SPIKE_EVALUATE) {
    this->game = game;
}

SpikeVectorEvaluate::~SpikeVectorEvaluate() {
    game = nullptr;
}

void SpikeVectorEvaluate::onStateMounted(Device* PDN) {
    transitionToShowState = false;
    transitionToWinState = false;
    transitionToLoseState = false;
    flashCount = 0;
    pipVisible = true;

    auto& session = game->getSession();
    auto& config = game->getConfig();

    // Award score for completing level
    session.score += 100;
    LOG_I(TAG, "Level %d complete! Score: %d, Hits: %d/%d",
          session.currentLevel + 1, session.score, session.hits, config.hitsAllowed);

    // Check for loss condition
    if (session.hits > config.hitsAllowed) {
        transitionToLoseState = true;
        return;
    }

    // Advance level
    session.currentLevel++;

    // Check for win condition
    if (session.currentLevel >= config.levels) {
        transitionToWinState = true;
        return;
    }

    // Start pip flash animation
    flashTimer.setTimer(FLASH_DURATION_MS);
}

void SpikeVectorEvaluate::onStateLoop(Device* PDN) {
    auto& session = game->getSession();
    auto& config = game->getConfig();

    // If transitioning to win/lose, skip animation
    if (transitionToWinState || transitionToLoseState) {
        return;
    }

    // Animate pip flashing
    if (flashTimer.expired()) {
        pipVisible = !pipVisible;
        flashCount++;

        // Draw progress pips with flashing animation
        PDN->getDisplay()->invalidateScreen();

        int totalPips = config.levels;
        int pipSize = 6;
        int pipSpacing = 10;
        int totalWidth = (totalPips * pipSpacing) - (pipSpacing - pipSize);
        int startX = (128 - totalWidth) / 2;

        for (int i = 0; i < totalPips; i++) {
            int pipX = startX + (i * pipSpacing);
            int pipY = 30;

            if (i < session.currentLevel - 1) {
                // Previously completed level — always filled
                PDN->getDisplay()->setDrawColor(1)->drawBox(pipX, pipY, pipSize, pipSize);
            } else if (i == session.currentLevel - 1) {
                // Just completed level — flash
                if (pipVisible) {
                    PDN->getDisplay()->setDrawColor(1)->drawBox(pipX, pipY, pipSize, pipSize);
                } else {
                    PDN->getDisplay()->setDrawColor(1)->drawFrame(pipX, pipY, pipSize, pipSize);
                }
            } else if (i == session.currentLevel) {
                // Next level — frame
                PDN->getDisplay()->setDrawColor(1)->drawFrame(pipX, pipY, pipSize, pipSize);
            } else {
                // Future level — smaller frame
                PDN->getDisplay()->setDrawColor(1)->drawFrame(pipX + 1, pipY + 1, pipSize - 2, pipSize - 2);
            }
        }

        PDN->getDisplay()->render();

        // Check if animation complete
        if (flashCount >= FLASH_CYCLES * 2) {
            // Animation done — transition to next level
            transitionToShowState = true;
            return;
        }

        // Restart flash timer
        flashTimer.setTimer(FLASH_DURATION_MS);
    }
}

void SpikeVectorEvaluate::onStateDismounted(Device* PDN) {
    flashTimer.invalidate();
    transitionToShowState = false;
    transitionToWinState = false;
    transitionToLoseState = false;
}

bool SpikeVectorEvaluate::transitionToShow() {
    return transitionToShowState;
}

bool SpikeVectorEvaluate::transitionToWin() {
    return transitionToWinState;
}

bool SpikeVectorEvaluate::transitionToLose() {
    return transitionToLoseState;
}
