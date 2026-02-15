#include "game/spike-vector/spike-vector-states.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/spike-vector/spike-vector-resources.hpp"
#include "device/drivers/logger.hpp"
#include <cstdlib>
#include <string>

static const char* TAG = "SpikeVectorShow";

SpikeVectorShow::SpikeVectorShow(SpikeVector* game) : State(SPIKE_SHOW) {
    this->game = game;
}

SpikeVectorShow::~SpikeVectorShow() {
    game = nullptr;
}

// Helper function to generate gap array with constrained distances
static std::vector<int> generateGapArray(int numWalls, int numLanes, int minGapDist, int maxGapDist) {
    std::vector<int> gaps;
    if (numWalls == 0) return gaps;

    // Start with random position
    int currentGap = rand() % numLanes;
    gaps.push_back(currentGap);

    // Generate remaining gaps with distance constraints
    for (int i = 1; i < numWalls; i++) {
        // Random direction (up or down)
        int direction = (rand() % 2 == 0) ? 1 : -1;

        // Random distance within bounds
        int distance = minGapDist + (rand() % (maxGapDist - minGapDist + 1));

        // Apply distance with wrapping
        int nextGap = currentGap + (direction * distance);

        // Wrap around if out of bounds
        while (nextGap < 0) nextGap += numLanes;
        while (nextGap >= numLanes) nextGap -= numLanes;

        gaps.push_back(nextGap);
        currentGap = nextGap;
    }

    return gaps;
}

// Helper function to calculate max gap distance for current level (easy mode only)
static int calculateMaxGapDistance(int level, int baseMaxGap, bool isEasyMode, int numLanes) {
    if (!isEasyMode) {
        return numLanes - 1;  // Full range for hard mode
    }

    // Easy mode progression: 2→2→3→3→4 across 5 levels
    if (level == 0 || level == 1) return 2;
    if (level == 2 || level == 3) return 3;
    return 4;
}

void SpikeVectorShow::onStateMounted(Device* PDN) {
    transitionToGameplayState = false;

    auto& session = game->getSession();
    auto& config = game->getConfig();

    // Calculate effective max gap distance for this level
    bool isEasyMode = (config.numLanes == 5);
    int effectiveMaxGap = calculateMaxGapDistance(session.currentLevel,
                                                   config.maxGapDistance,
                                                   isEasyMode,
                                                   config.numLanes);

    // Generate wall count for this level
    int numWalls = config.minWallsPerLevel +
                   (rand() % (config.maxWallsPerLevel - config.minWallsPerLevel + 1));

    // Generate gap positions for this level's walls
    session.currentGaps = generateGapArray(numWalls, config.numLanes,
                                           config.minGapDistance, effectiveMaxGap);
    session.currentWallIndex = 0;
    session.wallScrollOffset = 0;
    session.cursorPosition = config.startPosition;

    LOG_I(TAG, "Level %d of %d: %d walls, %zu gaps generated",
          session.currentLevel + 1, config.levels, numWalls, session.currentGaps.size());

    // Display level info with progress pips
    PDN->getDisplay()->invalidateScreen();

    // Draw progress pips (■■■□□) centered
    int totalPips = config.levels;
    int pipSize = 6;
    int pipSpacing = 10;
    int totalWidth = (totalPips * pipSpacing) - (pipSpacing - pipSize);
    int startX = (128 - totalWidth) / 2;

    for (int i = 0; i < totalPips; i++) {
        int pipX = startX + (i * pipSpacing);
        int pipY = 18;

        if (i < session.currentLevel) {
            // Completed level — filled box
            PDN->getDisplay()->setDrawColor(1)->drawBox(pipX, pipY, pipSize, pipSize);
        } else if (i == session.currentLevel) {
            // Current level — frame
            PDN->getDisplay()->setDrawColor(1)->drawFrame(pipX, pipY, pipSize, pipSize);
        } else {
            // Future level — smaller frame
            PDN->getDisplay()->setDrawColor(1)->drawFrame(pipX + 1, pipY + 1, pipSize - 2, pipSize - 2);
        }
    }

    // Show lives remaining
    int livesRemaining = config.hitsAllowed - session.hits;
    std::string livesStr = "Lives: " + std::to_string(livesRemaining);
    PDN->getDisplay()->setGlyphMode(FontMode::TEXT)
        ->drawText(livesStr.c_str(), 40, 45);

    PDN->getDisplay()->render();

    // Haptic pulse feedback
    PDN->getHaptics()->setIntensity(100);

    showTimer.setTimer(SHOW_DURATION_MS);
}

void SpikeVectorShow::onStateLoop(Device* PDN) {
    if (showTimer.expired()) {
        PDN->getHaptics()->off();
        transitionToGameplayState = true;
    }
}

void SpikeVectorShow::onStateDismounted(Device* PDN) {
    showTimer.invalidate();
    transitionToGameplayState = false;
    PDN->getHaptics()->off();
}

bool SpikeVectorShow::transitionToGameplay() {
    return transitionToGameplayState;
}
