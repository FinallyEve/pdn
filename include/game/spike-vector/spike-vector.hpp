#pragma once

#include "game/minigame.hpp"
#include "game/spike-vector/spike-vector-states.hpp"
#include <cstdlib>
#include <vector>

constexpr int SPIKE_VECTOR_APP_ID = 5;

// Speed level to milliseconds-per-pixel mapping
// Speed 1 (slowest) = 60ms, Speed 8 (fastest) = 15ms
static const int SPIKE_VECTOR_SPEED_TABLE[8] = {
    60, 52, 45, 37, 30, 25, 20, 15
};

struct SpikeVectorConfig {
    // Visual layout constants
    static constexpr int SCREEN_WIDTH = 128;
    static constexpr int SCREEN_HEIGHT = 64;
    static constexpr int HUD_HEIGHT = 8;
    static constexpr int SEPARATOR_HEIGHT = 1;
    static constexpr int CONTROLS_WIDTH = 8;
    static constexpr int LANE_AREA_HEIGHT = 45;

    // Wall formation parameters
    static constexpr int WALL_WIDTH = 6;
    static constexpr int WALL_SPACING = 14;    // edge-to-edge spacing
    static constexpr int WALL_UNIT = 20;       // WALL_WIDTH + WALL_SPACING

    // Gameplay parameters
    int numLanes = 5;                  // 5 for easy, 7 for hard
    int levels = 5;                    // total levels to complete
    int minWallsPerLevel = 5;          // minimum walls per level
    int maxWallsPerLevel = 8;          // maximum walls per level
    int minSpeed = 1;                  // minimum speed level (1-8)
    int maxSpeed = 5;                  // maximum speed level (1-8)
    int hitsAllowed = 3;               // hits before losing
    int startPosition = 2;             // starting lane position

    // Gap generation bounds
    int minGapDistance = 1;            // minimum distance between consecutive gaps
    int maxGapDistance = 2;            // maximum distance between consecutive gaps (varies by level)

    unsigned long rngSeed = 0;         // 0 = random, nonzero = deterministic
    bool managedMode = false;
};

struct SpikeVectorSession {
    int cursorPosition = 2;            // current player lane position
    int currentLevel = 0;              // current level (0-indexed)
    int hits = 0;                      // total hits taken
    int score = 0;                     // current score

    // Wall formation tracking
    std::vector<int> currentGaps;      // gap positions for current level
    int currentWallIndex = 0;          // which wall is currently passing
    int wallScrollOffset = 0;          // pixel offset for scrolling (0 = offscreen right)

    void reset() {
        cursorPosition = 2;
        currentLevel = 0;
        hits = 0;
        score = 0;
        currentGaps.clear();
        currentWallIndex = 0;
        wallScrollOffset = 0;
    }
};

inline SpikeVectorConfig makeSpikeVectorEasyConfig() {
    SpikeVectorConfig c;
    c.numLanes = 5;
    c.levels = 5;
    c.minWallsPerLevel = 5;
    c.maxWallsPerLevel = 8;
    c.minSpeed = 1;              // speeds 1-5
    c.maxSpeed = 5;
    c.hitsAllowed = 3;           // generous
    c.startPosition = 2;
    c.minGapDistance = 1;
    c.maxGapDistance = 2;        // starts at 2, scales to 4 by level 5
    return c;
}

inline SpikeVectorConfig makeSpikeVectorHardConfig() {
    SpikeVectorConfig c;
    c.numLanes = 7;
    c.levels = 5;
    c.minWallsPerLevel = 8;
    c.maxWallsPerLevel = 12;
    c.minSpeed = 4;              // speeds 4-8 (overlaps with easy at 4-5)
    c.maxSpeed = 8;
    c.hitsAllowed = 1;           // almost no margin
    c.startPosition = 3;
    c.minGapDistance = 1;
    c.maxGapDistance = 6;        // full lane range from start
    return c;
}

const SpikeVectorConfig SPIKE_VECTOR_EASY = makeSpikeVectorEasyConfig();
const SpikeVectorConfig SPIKE_VECTOR_HARD = makeSpikeVectorHardConfig();

class SpikeVector : public MiniGame {
public:
    explicit SpikeVector(SpikeVectorConfig config) :
        MiniGame(SPIKE_VECTOR_APP_ID, GameType::SPIKE_VECTOR, "SPIKE VECTOR"),
        config(config)
    {
    }

    void populateStateMap() override;
    void resetGame() override;

    SpikeVectorConfig& getConfig() { return config; }
    SpikeVectorSession& getSession() { return session; }

private:
    SpikeVectorConfig config;
    SpikeVectorSession session;
};
