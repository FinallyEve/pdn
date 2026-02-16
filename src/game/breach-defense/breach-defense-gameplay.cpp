#include "game/breach-defense/breach-defense-states.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "device/drivers/logger.hpp"
#include <cstdio>
#include <cstdlib>

static const char* TAG = "BreachDefenseGameplay";

BreachDefenseGameplay::BreachDefenseGameplay(BreachDefense* game) : State(BREACH_GAMEPLAY) {
    this->game = game;
}

BreachDefenseGameplay::~BreachDefenseGameplay() {
    game = nullptr;
}

void BreachDefenseGameplay::onStateMounted(Device* PDN) {
    transitionToWinState = false;
    transitionToLoseState = false;

    auto& session = game->getSession();
    auto& config = game->getConfig();

    // Reset session for fresh gameplay
    session.reset();

    LOG_I(TAG, "Gameplay started — %d lanes, %d total threats",
          config.numLanes, config.totalThreats);

    // Set up button callbacks for shield movement
    parameterizedCallbackFunction upCb = [](void* ctx) {
        auto* state = static_cast<BreachDefenseGameplay*>(ctx);
        auto& sess = state->game->getSession();
        if (sess.shieldLane > 0) {
            sess.shieldLane--;
        }
    };
    PDN->getPrimaryButton()->setButtonPress(upCb, this, ButtonInteraction::CLICK);

    parameterizedCallbackFunction downCb = [](void* ctx) {
        auto* state = static_cast<BreachDefenseGameplay*>(ctx);
        auto& sess = state->game->getSession();
        auto& cfg = state->game->getConfig();
        if (sess.shieldLane < cfg.numLanes - 1) {
            sess.shieldLane++;
        }
    };
    PDN->getSecondaryButton()->setButtonPress(downCb, this, ButtonInteraction::CLICK);

    // Start LED chase animation
    PDN->getLightManager()->startAnimation({
        AnimationType::VERTICAL_CHASE, true, 4, EaseCurve::LINEAR,
        LEDState(), 0
    });

    // Spawn first threat
    session.threats[0] = {rand() % config.numLanes, 0, true};
    session.nextSpawnIndex = 1;
    session.threatTimers[0].setTimer(config.threatSpeedMs);

    // Start spawn timer
    session.spawnTimer.setTimer(config.spawnIntervalMs);
}

void BreachDefenseGameplay::onStateLoop(Device* PDN) {
    auto& session = game->getSession();
    auto& config = game->getConfig();

    // 1. SPAWN CHECK
    if (session.spawnTimer.expired() && session.nextSpawnIndex < config.totalThreats) {
        // Count active threats
        int activeCount = 0;
        for (int i = 0; i < 3; i++) {
            if (session.threats[i].active) activeCount++;
        }

        // Find inactive slot
        if (activeCount < config.maxOverlap) {
            for (int i = 0; i < 3; i++) {
                if (!session.threats[i].active) {
                    session.threats[i] = {rand() % config.numLanes, 0, true};
                    session.threatTimers[i].setTimer(config.threatSpeedMs);
                    session.nextSpawnIndex++;
                    session.spawnTimer.setTimer(config.spawnIntervalMs);
                    break;
                }
            }
        }
    }

    // 2. ADVANCE THREATS
    for (int i = 0; i < 3; i++) {
        if (session.threats[i].active && session.threatTimers[i].expired()) {
            session.threats[i].position++;

            if (session.threats[i].position >= config.threatDistance) {
                // Threat reached defense line — evaluate
                bool blocked = (session.shieldLane == session.threats[i].lane);

                if (blocked) {
                    // BLOCKED
                    session.score += 100;
                    LOG_I(TAG, "BLOCKED! Shield %d == Threat lane %d. Score: %d",
                          session.shieldLane, session.threats[i].lane, session.score);
                    PDN->getHaptics()->setIntensity(150);
                    // XOR flash handled in render
                } else {
                    // BREACH
                    session.breaches++;
                    LOG_I(TAG, "BREACH! Shield %d != Threat lane %d. Breaches: %d/%d",
                          session.shieldLane, session.threats[i].lane,
                          session.breaches, config.missesAllowed);
                    PDN->getHaptics()->setIntensity(255);
                    // Lane invert handled in render
                }

                session.threats[i].active = false;
                session.threatsResolved++;
                session.threatTimers[i].invalidate();
            } else {
                // Reset timer for next step
                session.threatTimers[i].setTimer(config.threatSpeedMs);
            }
        }
    }

    // 3. END CONDITION
    if (session.breaches > config.missesAllowed) {
        transitionToLoseState = true;
        return;
    }
    if (session.threatsResolved >= config.totalThreats) {
        transitionToWinState = true;
        return;
    }

    // 4. RENDER (every frame)
    PDN->getDisplay()->invalidateScreen();

    // --- HUD (y 0-7) ---
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT)->setDrawColor(1);

    // Progress bar (left ~60px)
    int progressWidth = (session.threatsResolved * 50) / config.totalThreats;
    PDN->getDisplay()->drawBox(2, 2, progressWidth, 4);

    // Lives pips (center-right)
    int livesRemaining = config.missesAllowed - session.breaches;
    for (int i = 0; i < config.missesAllowed; i++) {
        int lx = 60 + i * 8;
        if (i < livesRemaining) {
            PDN->getDisplay()->drawBox(lx, 2, 5, 5);
        } else {
            PDN->getDisplay()->drawFrame(lx, 2, 5, 5);
        }
    }

    // Score (right)
    char scoreStr[16];
    snprintf(scoreStr, sizeof(scoreStr), "%d", session.score);
    PDN->getDisplay()->drawText(scoreStr, 96, 2);

    // Separator (y 8)
    PDN->getDisplay()->drawBox(0, 8, 128, 1);

    // --- GAME AREA (y 9-54, 46px) ---
    int gameAreaY = 9;
    int gameAreaHeight = 46;
    int laneHeight = gameAreaHeight / config.numLanes;

    // Lane dividers (horizontal dotted, 2px on / 4px off)
    for (int lane = 1; lane < config.numLanes; lane++) {
        int y = gameAreaY + lane * laneHeight;
        for (int x = 0; x < 128; x += 6) {
            PDN->getDisplay()->drawBox(x, y, 2, 1);
        }
    }

    // Defense line (vertical dashed at x=8, 4px on / 4px off)
    for (int y = gameAreaY; y < gameAreaY + gameAreaHeight; y += 8) {
        PDN->getDisplay()->drawBox(8, y, 1, 4);
    }

    // Shield (drawBox 6px wide, snaps to lane)
    int shieldH = (config.numLanes == 3) ? 12 : 6;
    int shieldY = gameAreaY + session.shieldLane * laneHeight + (laneHeight - shieldH) / 2;
    PDN->getDisplay()->drawBox(2, shieldY, 6, shieldH);

    // Threats (4x4 easy / 3x3 hard, slide right-to-left)
    int threatSize = (config.numLanes == 3) ? 4 : 3;
    for (int i = 0; i < 3; i++) {
        if (session.threats[i].active) {
            int threatX = 124 - (session.threats[i].position * 116 / config.threatDistance);
            int threatY = gameAreaY + session.threats[i].lane * laneHeight + (laneHeight - threatSize) / 2;

            // Check if this threat is at defense line for visual feedback
            bool atDefenseLine = (session.threats[i].position >= config.threatDistance - 1);
            bool blocked = (session.shieldLane == session.threats[i].lane);

            if (atDefenseLine && blocked) {
                // XOR flash for block
                PDN->getDisplay()->setDrawColor(2)->drawBox(threatX, threatY, threatSize, threatSize);
                PDN->getDisplay()->setDrawColor(1);
            } else if (atDefenseLine && !blocked) {
                // Invert lane for breach
                int invY = gameAreaY + session.threats[i].lane * laneHeight;
                PDN->getDisplay()->setDrawColor(2)->drawBox(0, invY, 128, laneHeight);
                PDN->getDisplay()->setDrawColor(1)->drawBox(threatX, threatY, threatSize, threatSize);
            } else {
                PDN->getDisplay()->drawBox(threatX, threatY, threatSize, threatSize);
            }
        }
    }

    // Separator (y 55)
    PDN->getDisplay()->drawBox(0, 55, 128, 1);

    // --- CONTROLS (y 56-63) ---
    PDN->getDisplay()->drawText("[UP]", 2, 58);
    PDN->getDisplay()->drawText("[DOWN]", 90, 58);

    PDN->getDisplay()->render();
}

void BreachDefenseGameplay::onStateDismounted(Device* PDN) {
    // Invalidate all timers
    for (int i = 0; i < 3; i++) {
        game->getSession().threatTimers[i].invalidate();
    }
    game->getSession().spawnTimer.invalidate();

    // Remove button callbacks
    PDN->getPrimaryButton()->removeButtonCallbacks();
    PDN->getSecondaryButton()->removeButtonCallbacks();

    // Clear haptics
    PDN->getHaptics()->off();

    transitionToWinState = false;
    transitionToLoseState = false;
}

bool BreachDefenseGameplay::transitionToWin() {
    return transitionToWinState;
}

bool BreachDefenseGameplay::transitionToLose() {
    return transitionToLoseState;
}
