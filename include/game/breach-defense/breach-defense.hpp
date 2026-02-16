#pragma once

#include "game/minigame.hpp"
#include "game/breach-defense/breach-defense-states.hpp"
#include "utils/simple-timer.hpp"
#include <cstdlib>

constexpr int BREACH_DEFENSE_APP_ID = 8;

struct BreachDefenseConfig {
    int numLanes = 3;             // number of defense lanes (0 to numLanes-1)
    int threatSpeedMs = 40;       // ms per threat step
    int threatDistance = 100;      // threat travels 0 to threatDistance
    int totalThreats = 8;         // total threats to survive
    int missesAllowed = 2;        // breaches before losing
    int spawnIntervalMs = 1500;   // ms between threat spawns
    int maxOverlap = 2;           // max concurrent active threats
    unsigned long rngSeed = 0;
    bool managedMode = false;
};

struct Threat {
    int lane = 0;
    int position = 0;
    bool active = false;
};

struct BreachDefenseSession {
    int shieldLane = 0;           // player's current shield position
    Threat threats[3];            // max 3 concurrent threats
    int nextSpawnIndex = 0;       // next threat to spawn (0-based)
    int threatsResolved = 0;      // threats blocked or breached
    int breaches = 0;             // damage taken
    int score = 0;
    SimpleTimer spawnTimer;
    SimpleTimer threatTimers[3];

    void reset() {
        shieldLane = 0;
        nextSpawnIndex = 0;
        threatsResolved = 0;
        breaches = 0;
        score = 0;
        for (int i = 0; i < 3; i++) {
            threats[i] = {0, 0, false};
            threatTimers[i].invalidate();
        }
        spawnTimer.invalidate();
    }
};

inline BreachDefenseConfig makeBreachDefenseEasyConfig() {
    BreachDefenseConfig c;
    c.numLanes = 3;
    c.threatSpeedMs = 40;
    c.threatDistance = 100;
    c.totalThreats = 6;
    c.missesAllowed = 3;
    c.spawnIntervalMs = 1500;
    c.maxOverlap = 2;
    return c;
}

inline BreachDefenseConfig makeBreachDefenseHardConfig() {
    BreachDefenseConfig c;
    c.numLanes = 5;
    c.threatSpeedMs = 20;
    c.threatDistance = 100;
    c.totalThreats = 12;
    c.missesAllowed = 1;
    c.spawnIntervalMs = 700;
    c.maxOverlap = 3;
    return c;
}

const BreachDefenseConfig BREACH_DEFENSE_EASY = makeBreachDefenseEasyConfig();
const BreachDefenseConfig BREACH_DEFENSE_HARD = makeBreachDefenseHardConfig();

class BreachDefense : public MiniGame {
public:
    explicit BreachDefense(BreachDefenseConfig config) :
        MiniGame(BREACH_DEFENSE_APP_ID, GameType::BREACH_DEFENSE, "BREACH DEFENSE"),
        config(config)
    {
    }

    void populateStateMap() override;
    void resetGame() override;

    BreachDefenseConfig& getConfig() { return config; }
    BreachDefenseSession& getSession() { return session; }

private:
    BreachDefenseConfig config;
    BreachDefenseSession session;
};
