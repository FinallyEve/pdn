#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/spike-vector/spike-vector-resources.hpp"
#include "device/drivers/logger.hpp"
#include <string>

static const char* TAG = "SpikeVectorGameplay";

SpikeVectorGameplay::SpikeVectorGameplay(SpikeVector* game) : State(SPIKE_GAMEPLAY) {
    this->game = game;
}

SpikeVectorGameplay::~SpikeVectorGameplay() {
    game = nullptr;
}

void SpikeVectorGameplay::onStateMounted(Device* PDN) {
    transitionToEvaluateState = false;
    upButtonPressed = false;
    downButtonPressed = false;
    hitThisLevel = false;

    auto& session = game->getSession();
    auto& config = game->getConfig();

    LOG_I(TAG, "Gameplay started, level %d with %zu walls",
          session.currentLevel + 1, session.currentGaps.size());

    // Set up button callbacks with press indicators
    parameterizedCallbackFunction upPressCallback = [](void* ctx) {
        auto* state = static_cast<SpikeVectorGameplay*>(ctx);
        auto& sess = state->game->getSession();
        auto& conf = state->game->getConfig();
        if (sess.cursorPosition > 0) {
            sess.cursorPosition--;
        }
        state->upButtonPressed = true;
    };

    parameterizedCallbackFunction upReleaseCallback = [](void* ctx) {
        auto* state = static_cast<SpikeVectorGameplay*>(ctx);
        state->upButtonPressed = false;
    };

    parameterizedCallbackFunction downPressCallback = [](void* ctx) {
        auto* state = static_cast<SpikeVectorGameplay*>(ctx);
        auto& sess = state->game->getSession();
        auto& conf = state->game->getConfig();
        if (sess.cursorPosition < conf.numLanes - 1) {
            sess.cursorPosition++;
        }
        state->downButtonPressed = true;
    };

    parameterizedCallbackFunction downReleaseCallback = [](void* ctx) {
        auto* state = static_cast<SpikeVectorGameplay*>(ctx);
        state->downButtonPressed = false;
    };

    PDN->getPrimaryButton()->setButtonPress(upPressCallback, this, ButtonInteraction::CLICK);
    PDN->getPrimaryButton()->setButtonPress(upReleaseCallback, this, ButtonInteraction::RELEASE);
    PDN->getSecondaryButton()->setButtonPress(downPressCallback, this, ButtonInteraction::CLICK);
    PDN->getSecondaryButton()->setButtonPress(downReleaseCallback, this, ButtonInteraction::RELEASE);

    // Start LED chase animation
    AnimationConfig animConfig;
    animConfig.type = AnimationType::VERTICAL_CHASE;
    animConfig.speed = 8;
    animConfig.curve = EaseCurve::LINEAR;
    animConfig.initialState = SPIKE_VECTOR_GAMEPLAY_STATE;
    animConfig.loopDelayMs = 0;
    animConfig.loop = true;
    PDN->getLightManager()->startAnimation(animConfig);

    // Calculate speed for this level
    int speedLevel = config.minSpeed + (session.currentLevel % (config.maxSpeed - config.minSpeed + 1));
    if (speedLevel > config.maxSpeed) speedLevel = config.maxSpeed;
    int scrollSpeedMs = SPIKE_VECTOR_SPEED_TABLE[speedLevel - 1];

    // Start scroll timer
    scrollTimer.setTimer(scrollSpeedMs);
}

void SpikeVectorGameplay::onStateLoop(Device* PDN) {
    auto& session = game->getSession();
    auto& config = game->getConfig();

    // Scroll walls on timer
    if (scrollTimer.expired()) {
        session.wallScrollOffset++;

        // Check if current wall passed the collision zone
        if (session.currentWallIndex < static_cast<int>(session.currentGaps.size())) {
            // Calculate wall X position (starts at 128, scrolls left)
            int wallBaseX = 128 - session.wallScrollOffset +
                           (session.currentWallIndex * config.WALL_UNIT);

            // Check if wall completely passed the player cursor (left edge of screen)
            if (wallBaseX + config.WALL_WIDTH < 10) {
                // Check collision with this wall
                int gapLane = session.currentGaps[session.currentWallIndex];
                if (session.cursorPosition != gapLane && !hitThisLevel) {
                    // Hit detected!
                    session.hits++;
                    hitThisLevel = true;

                    // Haptic pulse
                    PDN->getHaptics()->setIntensity(255);

                    // Red LED flash (handled by LED state briefly)

                    LOG_I(TAG, "Hit! Wall %d, gap at %d, player at %d",
                          session.currentWallIndex, gapLane, session.cursorPosition);
                }

                session.currentWallIndex++;
            }
        }

        // Check if all walls have passed
        if (session.currentWallIndex >= static_cast<int>(session.currentGaps.size())) {
            int lastWallBaseX = 128 - session.wallScrollOffset +
                               ((static_cast<int>(session.currentGaps.size()) - 1) * config.WALL_UNIT);
            if (lastWallBaseX + config.WALL_WIDTH < 0) {
                // All walls passed — level complete
                transitionToEvaluateState = true;
                return;
            }
        }

        // Recalculate speed timer
        int speedLevel = config.minSpeed + (session.currentLevel % (config.maxSpeed - config.minSpeed + 1));
        if (speedLevel > config.maxSpeed) speedLevel = config.maxSpeed;
        int scrollSpeedMs = SPIKE_VECTOR_SPEED_TABLE[speedLevel - 1];
        scrollTimer.setTimer(scrollSpeedMs);
    }

    // Render display
    PDN->getDisplay()->invalidateScreen();

    // Calculate layout dimensions
    int laneHeight = config.LANE_AREA_HEIGHT / config.numLanes;
    int centerY = config.HUD_HEIGHT + config.SEPARATOR_HEIGHT;

    // Draw HUD (top 8 pixels)
    std::string levelStr = "L" + std::to_string(session.currentLevel + 1) +
                           "/" + std::to_string(config.levels);
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
        ->drawText(levelStr.c_str(), 2, 2);

    // Draw lives indicators
    for (int i = 0; i < config.hitsAllowed; i++) {
        int lx = 50 + i * 8;
        if (i < config.hitsAllowed - session.hits) {
            PDN->getDisplay()->setDrawColor(1)->drawBox(lx, 2, 5, 5);
        } else {
            PDN->getDisplay()->setDrawColor(1)->drawFrame(lx, 2, 5, 5);
        }
    }

    // Draw separator line
    PDN->getDisplay()->setDrawColor(1)
        ->drawBox(0, config.HUD_HEIGHT, 128, config.SEPARATOR_HEIGHT);

    // Draw lane dividers (tick marks between lanes)
    for (int i = 1; i < config.numLanes; i++) {
        int dividerY = centerY + (i * laneHeight);
        for (int x = 0; x < 128; x += 5) {
            PDN->getDisplay()->drawBox(x, dividerY - 1, 3, 1);
        }
    }

    // Draw walls
    for (size_t i = 0; i < session.currentGaps.size(); i++) {
        int wallBaseX = 128 - session.wallScrollOffset + (i * config.WALL_UNIT);

        // Only draw if on screen
        if (wallBaseX + config.WALL_WIDTH >= 0 && wallBaseX < 128) {
            int gapLane = session.currentGaps[i];

            // Draw wall segments for each lane except gap
            for (int lane = 0; lane < config.numLanes; lane++) {
                if (lane != gapLane) {
                    int laneY = centerY + (lane * laneHeight);
                    PDN->getDisplay()->setDrawColor(1)
                        ->drawBox(wallBaseX, laneY, config.WALL_WIDTH, laneHeight);
                }
            }
        }
    }

    // Draw player cursor (left edge) - simple triangle
    int cursorY = centerY + (session.cursorPosition * laneHeight) + (laneHeight / 2) - 3;
    // Draw a simple right-pointing triangle using boxes
    PDN->getDisplay()->setDrawColor(1)
        ->drawBox(2, cursorY + 3, 1, 1)
        ->drawBox(3, cursorY + 2, 1, 3)
        ->drawBox(4, cursorY + 1, 1, 5)
        ->drawBox(5, cursorY, 1, 7);

    // Draw button indicators (right edge)
    int controlsX = 128 - config.CONTROLS_WIDTH;
    int upY = centerY + 5;
    int downY = centerY + config.LANE_AREA_HEIGHT - 12;

    if (upButtonPressed) {
        // Inverted (filled)
        PDN->getDisplay()->setDrawColor(1)->drawBox(controlsX, upY, 7, 7);
        PDN->getDisplay()->setDrawColor(0)->setGlyphMode(FontMode::TEXT)
            ->drawText("U", controlsX + 1, upY + 1);
        PDN->getDisplay()->setDrawColor(1);
    } else {
        // Normal
        PDN->getDisplay()->setDrawColor(1)->drawFrame(controlsX, upY, 7, 7);
        PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
            ->drawText("U", controlsX + 1, upY + 1);
    }

    if (downButtonPressed) {
        // Inverted (filled)
        PDN->getDisplay()->setDrawColor(1)->drawBox(controlsX, downY, 7, 7);
        PDN->getDisplay()->setDrawColor(0)->setGlyphMode(FontMode::TEXT)
            ->drawText("D", controlsX + 1, downY + 1);
        PDN->getDisplay()->setDrawColor(1);
    } else {
        // Normal
        PDN->getDisplay()->setDrawColor(1)->drawFrame(controlsX, downY, 7, 7);
        PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
            ->drawText("D", controlsX + 1, downY + 1);
    }

    PDN->getDisplay()->render();
}

void SpikeVectorGameplay::onStateDismounted(Device* PDN) {
    scrollTimer.invalidate();
    transitionToEvaluateState = false;
    upButtonPressed = false;
    downButtonPressed = false;
    PDN->getPrimaryButton()->removeButtonCallbacks();
    PDN->getSecondaryButton()->removeButtonCallbacks();
}

bool SpikeVectorGameplay::transitionToEvaluate() {
    return transitionToEvaluateState;
}
