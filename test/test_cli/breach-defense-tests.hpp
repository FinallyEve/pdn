#pragma once

#ifdef NATIVE_BUILD

#include <gtest/gtest.h>
#include "cli/cli-device.hpp"
#include "cli/cli-serial-broker.hpp"
#include "cli/cli-http-server.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "game/breach-defense/breach-defense-states.hpp"
#include "game/minigame.hpp"
#include "device/device-types.hpp"
#include "utils/simple-timer.hpp"
#include <cstdint>

using namespace cli;

// ============================================
// BREACH DEFENSE STANDALONE TEST SUITE
// ============================================

class BreachDefenseTestSuite : public testing::Test {
public:
    void SetUp() override {
        // Reset all singleton state before each test to prevent pollution
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();

        device_ = DeviceFactory::createGameDevice(0, "breach-defense");
        SimpleTimer::setPlatformClock(device_.clockDriver);
        game_ = static_cast<BreachDefense*>(device_.game);

        // Re-enter intro so timers use the correct clock
        game_->skipToState(device_.pdn, 0);
        device_.pdn->loop();
    }

    void TearDown() override {
        DeviceFactory::destroyDevice(device_);

        // Clean up singleton state after each test
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();
    }

    void tick(int n = 1) {
        for (int i = 0; i < n; i++) {
            device_.pdn->loop();
        }
    }

    void tickWithTime(int n, int delayMs) {
        for (int i = 0; i < n; i++) {
            device_.clockDriver->advance(delayMs);
            device_.pdn->loop();
        }
    }

    DeviceInstance device_;
    BreachDefense* game_ = nullptr;
};

// ============================================
// BREACH DEFENSE MANAGED TEST SUITE (FDN)
// ============================================

class BreachDefenseManagedTestSuite : public testing::Test {
public:
    void SetUp() override {
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();

        player_ = DeviceFactory::createDevice(0, true);
        SimpleTimer::setPlatformClock(player_.clockDriver);
    }

    void TearDown() override {
        DeviceFactory::destroyDevice(player_);

        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();
    }

    void tick(int n = 1) {
        for (int i = 0; i < n; i++) {
            player_.pdn->loop();
        }
    }

    void tickWithTime(int n, int delayMs) {
        for (int i = 0; i < n; i++) {
            player_.clockDriver->advance(delayMs);
            player_.pdn->loop();
        }
    }

    void advanceToIdle() {
        player_.game->skipToState(player_.pdn, 6);
        player_.pdn->loop();
    }

    int getPlayerStateId() {
        return player_.game->getCurrentState()->getStateId();
    }

    BreachDefense* getBreachDefense() {
        return static_cast<BreachDefense*>(
            player_.pdn->getApp(StateId(BREACH_DEFENSE_APP_ID)));
    }

    DeviceInstance player_;
};

// ============================================
// CONFIG PRESET TESTS
// ============================================

/*
 * Test: EASY config presets — 3 lanes, 6 threats, 3 breaches allowed, 2 max overlap.
 */
void breachDefenseEasyConfigPresets(BreachDefenseTestSuite* suite) {
    BreachDefenseConfig easy = makeBreachDefenseEasyConfig();
    ASSERT_EQ(easy.numLanes, 3);
    ASSERT_EQ(easy.threatSpeedMs, 40);
    ASSERT_EQ(easy.threatDistance, 100);
    ASSERT_EQ(easy.totalThreats, 6);
    ASSERT_EQ(easy.missesAllowed, 3);
    ASSERT_EQ(easy.spawnIntervalMs, 1500);
    ASSERT_EQ(easy.maxOverlap, 2);
}

/*
 * Test: HARD config presets — 5 lanes, 12 threats, 1 breach allowed, 3 max overlap.
 */
void breachDefenseHardConfigPresets(BreachDefenseTestSuite* suite) {
    BreachDefenseConfig hard = makeBreachDefenseHardConfig();
    ASSERT_EQ(hard.numLanes, 5);
    ASSERT_EQ(hard.threatSpeedMs, 20);
    ASSERT_EQ(hard.threatDistance, 100);
    ASSERT_EQ(hard.totalThreats, 12);
    ASSERT_EQ(hard.missesAllowed, 1);
    ASSERT_EQ(hard.spawnIntervalMs, 700);
    ASSERT_EQ(hard.maxOverlap, 3);
}

// ============================================
// INTRO STATE TESTS
// ============================================

/*
 * Test: Intro resets session to defaults.
 */
void breachDefenseIntroResetsSession(BreachDefenseTestSuite* suite) {
    // Dirty the session
    auto& sess = suite->game_->getSession();
    sess.score = 999;
    sess.breaches = 5;
    sess.shieldLane = 4;
    sess.nextSpawnIndex = 10;
    sess.threatsResolved = 8;
    sess.threats[0].active = true;

    // Re-enter intro
    suite->game_->skipToState(suite->device_.pdn, 0);
    suite->tick(1);

    ASSERT_EQ(sess.score, 0);
    ASSERT_EQ(sess.breaches, 0);
    ASSERT_EQ(sess.shieldLane, 0);
    ASSERT_EQ(sess.nextSpawnIndex, 0);
    ASSERT_EQ(sess.threatsResolved, 0);
    ASSERT_FALSE(sess.threats[0].active);
}

/*
 * Test: Intro transitions to Gameplay after 2s timer.
 */
void breachDefenseIntroTransitionsToGameplay(BreachDefenseTestSuite* suite) {
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_INTRO);

    // Advance past 2s intro timer
    suite->tickWithTime(25, 100);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_GAMEPLAY);
}

// ============================================
// GAMEPLAY STATE TESTS — THREAT MECHANICS
// ============================================

/*
 * Test: First threat spawns immediately on gameplay mount.
 */
void breachDefenseFirstThreatSpawnsImmediately(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay is index 1
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    ASSERT_EQ(sess.nextSpawnIndex, 1);  // First threat spawned
    ASSERT_TRUE(sess.threats[0].active);
    ASSERT_GE(sess.threats[0].lane, 0);
    ASSERT_LT(sess.threats[0].lane, suite->game_->getConfig().numLanes);
    ASSERT_EQ(sess.threats[0].position, 0);
}

/*
 * Test: Threats spawn on fixed rhythm (spawnIntervalMs).
 */
void breachDefenseThreatSpawnRhythm(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().spawnIntervalMs = 500;
    suite->game_->getConfig().totalThreats = 5;
    suite->game_->getConfig().threatDistance = 10000;  // Large to prevent arrival

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    ASSERT_EQ(sess.nextSpawnIndex, 1);  // First threat spawned

    // Advance just under spawn interval — no new spawn
    suite->tickWithTime(5, 90);
    ASSERT_EQ(sess.nextSpawnIndex, 1);

    // Advance past spawn interval — second threat spawns
    suite->tickWithTime(2, 100);
    ASSERT_EQ(sess.nextSpawnIndex, 2);
    ASSERT_TRUE(sess.threats[1].active);
}

/*
 * Test: Max overlap constraint (EASY: 2, HARD: 3).
 */
void breachDefenseMaxOverlapConstraint(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().spawnIntervalMs = 100;
    suite->game_->getConfig().maxOverlap = 2;
    suite->game_->getConfig().totalThreats = 5;
    suite->game_->getConfig().threatDistance = 10000;  // Prevent arrival

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    // First spawn immediate
    auto& sess = suite->game_->getSession();
    ASSERT_EQ(sess.nextSpawnIndex, 1);

    // Second spawn after interval
    suite->tickWithTime(2, 100);
    ASSERT_EQ(sess.nextSpawnIndex, 2);

    // Try to spawn third — should block due to maxOverlap=2
    suite->tickWithTime(2, 100);
    ASSERT_EQ(sess.nextSpawnIndex, 2);  // Still 2, third spawn blocked
}

/*
 * Test: Threat position advances with time.
 */
void breachDefenseThreatAdvancesWithTime(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 10;
    suite->game_->getConfig().threatDistance = 1000;  // Large distance

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    int initialPos = sess.threats[0].position;

    // Advance several steps
    suite->tickWithTime(5, 15);

    ASSERT_GT(sess.threats[0].position, initialPos);
}

// ============================================
// GAMEPLAY STATE TESTS — SHIELD MECHANICS
// ============================================

/*
 * Test: Shield moves UP (primary button decreases lane).
 */
void breachDefenseShieldMovesUp(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().numLanes = 5;
    suite->game_->getConfig().threatDistance = 10000;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    sess.shieldLane = 2;

    // Press UP (primary button)
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    ASSERT_EQ(sess.shieldLane, 1);
}

/*
 * Test: Shield moves DOWN (secondary button increases lane).
 */
void breachDefenseShieldMovesDown(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().numLanes = 5;
    suite->game_->getConfig().threatDistance = 10000;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    sess.shieldLane = 2;

    // Press DOWN (secondary button)
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    ASSERT_EQ(sess.shieldLane, 3);
}

/*
 * Test: Shield clamped at bottom boundary (lane 0).
 */
void breachDefenseShieldClampedAtBottom(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().numLanes = 3;
    suite->game_->getConfig().threatDistance = 10000;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    sess.shieldLane = 0;

    // Press UP — should stay at 0
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    ASSERT_EQ(sess.shieldLane, 0);
}

/*
 * Test: Shield clamped at top boundary (lane numLanes-1).
 */
void breachDefenseShieldClampedAtTop(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().numLanes = 3;
    suite->game_->getConfig().threatDistance = 10000;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    sess.shieldLane = 2;  // Max for 3 lanes

    // Press DOWN — should stay at 2
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    ASSERT_EQ(sess.shieldLane, 2);
}

// ============================================
// GAMEPLAY STATE TESTS — INLINE EVALUATION
// ============================================

/*
 * Test: Threat blocked (shield matches lane) — score +100, no breach.
 */
void breachDefenseCorrectBlock(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().totalThreats = 6;
    suite->game_->getConfig().missesAllowed = 3;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    // Match shield to threat lane
    sess.shieldLane = sess.threats[0].lane;

    // Advance until threat arrives
    suite->tickWithTime(30, 10);

    // Should have evaluated — check score
    ASSERT_EQ(sess.score, 100);
    ASSERT_EQ(sess.breaches, 0);
    ASSERT_FALSE(sess.threats[0].active);
    ASSERT_EQ(sess.threatsResolved, 1);
}

/*
 * Test: Missed threat (shield in wrong lane) — breach counted, no score.
 */
void breachDefenseMissedThreat(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().totalThreats = 6;
    suite->game_->getConfig().missesAllowed = 3;
    suite->game_->getConfig().numLanes = 3;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    // Set shield to NOT match threat lane
    if (sess.threats[0].lane == 0) {
        sess.shieldLane = 1;
    } else {
        sess.shieldLane = 0;
    }

    // Advance until threat arrives
    suite->tickWithTime(30, 10);

    // Should have evaluated — check breach counted
    ASSERT_EQ(sess.score, 0);
    ASSERT_EQ(sess.breaches, 1);
    ASSERT_FALSE(sess.threats[0].active);
    ASSERT_EQ(sess.threatsResolved, 1);
}

/*
 * Test: Multiple threats can overlap and evaluate independently.
 */
void breachDefenseMultipleThreatOverlap(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 10;
    suite->game_->getConfig().threatDistance = 20;
    suite->game_->getConfig().spawnIntervalMs = 50;
    suite->game_->getConfig().totalThreats = 6;
    suite->game_->getConfig().maxOverlap = 2;
    suite->game_->getConfig().numLanes = 3;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();

    // Let first threat spawn and advance a bit
    suite->tickWithTime(3, 20);

    // Second threat should spawn
    ASSERT_EQ(sess.nextSpawnIndex, 2);
    ASSERT_TRUE(sess.threats[0].active);
    ASSERT_TRUE(sess.threats[1].active);

    // Both should advance independently
    int pos0 = sess.threats[0].position;
    int pos1 = sess.threats[1].position;
    ASSERT_GT(pos0, pos1);  // First threat should be further along
}

// ============================================
// WIN/LOSE CONDITION TESTS
// ============================================

/*
 * Test: Win condition — all threats survived with acceptable breaches.
 */
void breachDefenseWinCondition(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().totalThreats = 2;
    suite->game_->getConfig().missesAllowed = 3;
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().spawnIntervalMs = 100;
    suite->game_->getConfig().maxOverlap = 2;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();

    // Block first threat
    sess.shieldLane = sess.threats[0].lane;
    suite->tickWithTime(30, 10);

    // Wait for second threat to spawn
    suite->tickWithTime(15, 10);

    // Block second threat
    int activeThreatIdx = -1;
    for (int i = 0; i < 3; i++) {
        if (sess.threats[i].active) {
            activeThreatIdx = i;
            break;
        }
    }
    ASSERT_NE(activeThreatIdx, -1);
    sess.shieldLane = sess.threats[activeThreatIdx].lane;
    suite->tickWithTime(30, 10);

    // Should transition to Win
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_WIN);
}

/*
 * Test: Lose condition — too many breaches (> missesAllowed).
 */
void breachDefenseLoseCondition(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().totalThreats = 6;
    suite->game_->getConfig().missesAllowed = 0;  // No misses allowed
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().numLanes = 3;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();

    // Ensure shield does NOT match threat
    if (sess.threats[0].lane == 0) {
        sess.shieldLane = 1;
    } else {
        sess.shieldLane = 0;
    }

    // Let threat arrive and breach
    suite->tickWithTime(30, 10);

    // Should transition to Lose (1 breach > 0 allowed)
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_LOSE);
}

/*
 * Test: Exact breaches equal to missesAllowed doesn't lose (only > causes loss).
 */
void breachDefenseExactBreachesEqualAllowed(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().totalThreats = 3;
    suite->game_->getConfig().missesAllowed = 2;
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().spawnIntervalMs = 100;
    suite->game_->getConfig().numLanes = 3;

    suite->game_->skipToState(suite->device_.pdn, 1);
    suite->tick(1);

    auto& sess = suite->game_->getSession();

    // Miss first threat
    if (sess.threats[0].lane == 0) {
        sess.shieldLane = 1;
    } else {
        sess.shieldLane = 0;
    }
    suite->tickWithTime(30, 10);

    // Wait for second threat
    suite->tickWithTime(15, 10);

    // Miss second threat
    int activeThreatIdx = -1;
    for (int i = 0; i < 3; i++) {
        if (sess.threats[i].active) {
            activeThreatIdx = i;
            break;
        }
    }
    ASSERT_NE(activeThreatIdx, -1);
    if (sess.threats[activeThreatIdx].lane == 0) {
        sess.shieldLane = 1;
    } else {
        sess.shieldLane = 0;
    }
    suite->tickWithTime(30, 10);

    // 2 breaches == 2 allowed — should still be in Gameplay
    ASSERT_EQ(sess.breaches, 2);
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_GAMEPLAY);
}

// ============================================
// WIN/LOSE STATE TESTS
// ============================================

/*
 * Test: Win state sets outcome to WON.
 */
void breachDefenseWinSetsOutcome(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Win is index 2
    suite->tick(1);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_WIN);
    ASSERT_EQ(suite->game_->getOutcome().result, MiniGameResult::WON);
}

/*
 * Test: Lose state sets outcome to LOST.
 */
void breachDefenseLoseSetsOutcome(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 3);  // Lose is index 3
    suite->tick(1);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_LOSE);
    ASSERT_EQ(suite->game_->getOutcome().result, MiniGameResult::LOST);
}

/*
 * Test: In standalone mode, Win loops back to Intro after timer.
 */
void breachDefenseStandaloneWinLoopsToIntro(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Win
    suite->tick(1);
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_WIN);

    // Advance past 3s win timer
    suite->tickWithTime(35, 100);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_INTRO);
}

/*
 * Test: In standalone mode, Lose loops back to Intro after timer.
 */
void breachDefenseStandaloneLoseLoopsToIntro(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 3);  // Lose
    suite->tick(1);
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_LOSE);

    // Advance past 3s lose timer
    suite->tickWithTime(35, 100);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_INTRO);
}

// ============================================
// DIFFICULTY TESTS
// ============================================

/*
 * Test: EASY difficulty — 3 lanes, slower speed, more breaches allowed.
 */
void breachDefenseEasyDifficulty(BreachDefenseTestSuite* suite) {
    // Device created with default EASY config
    auto& config = suite->game_->getConfig();
    ASSERT_EQ(config.numLanes, 3);
    ASSERT_EQ(config.totalThreats, 6);
    ASSERT_EQ(config.missesAllowed, 3);
    ASSERT_EQ(config.maxOverlap, 2);
}

/*
 * Test: HARD difficulty — 5 lanes, faster speed, fewer breaches allowed.
 */
void breachDefenseHardDifficulty(BreachDefenseTestSuite* suite) {
    // Destroy easy device and create hard
    DeviceFactory::destroyDevice(suite->device_);
    suite->device_ = DeviceFactory::createGameDevice(0, "breach-defense-hard");
    SimpleTimer::setPlatformClock(suite->device_.clockDriver);
    suite->game_ = static_cast<BreachDefense*>(suite->device_.game);

    auto& config = suite->game_->getConfig();
    ASSERT_EQ(config.numLanes, 5);
    ASSERT_EQ(config.totalThreats, 12);
    ASSERT_EQ(config.missesAllowed, 1);
    ASSERT_EQ(config.maxOverlap, 3);
}

// ============================================
// INTEGRATION TESTS — MANAGED MODE (FDN)
// ============================================

/*
 * Test: Managed mode — FDN launches Breach Defense and returns to Quickdraw.
 */
void breachDefenseManagedModeReturns(BreachDefenseManagedTestSuite* suite) {
    suite->advanceToIdle();

    // Trigger FDN handshake for Breach Defense (GameType 6, KonamiButton A=5)
    suite->player_.serialOutDriver->injectInput("*fdn:6:5\r");
    for (int i = 0; i < 3; i++) {
        SerialCableBroker::getInstance().transferData();
        suite->player_.pdn->loop();
    }
    ASSERT_EQ(suite->getPlayerStateId(), FDN_DETECTED);

    // Complete handshake
    suite->player_.serialOutDriver->injectInput("*fack\r");
    suite->tickWithTime(5, 100);

    // Should be in Breach Defense now
    auto* bd = suite->getBreachDefense();
    ASSERT_NE(bd, nullptr);
    ASSERT_TRUE(bd->getConfig().managedMode);

    // Configure very short game for quick playthrough
    bd->getConfig().threatSpeedMs = 5;
    bd->getConfig().threatDistance = 5;
    bd->getConfig().totalThreats = 2;
    bd->getConfig().missesAllowed = 3;
    bd->getConfig().spawnIntervalMs = 50;

    // Advance past intro timer (2s)
    suite->tickWithTime(25, 100);

    // Should be in Gameplay
    ASSERT_EQ(bd->getCurrentState()->getStateId(), BREACH_GAMEPLAY);

    // Block both threats
    auto& sess = bd->getSession();
    for (int threatNum = 0; threatNum < 2; threatNum++) {
        // Find active threat
        int idx = -1;
        for (int i = 0; i < 3; i++) {
            if (sess.threats[i].active) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            sess.shieldLane = sess.threats[idx].lane;
        }
        suite->tickWithTime(20, 10);
    }

    // Should be in Win now
    ASSERT_EQ(bd->getCurrentState()->getStateId(), BREACH_WIN);
    ASSERT_EQ(bd->getOutcome().result, MiniGameResult::WON);

    // Advance past win timer (3s)
    suite->tickWithTime(35, 100);

    // Should have returned to Quickdraw's FdnComplete state
    ASSERT_EQ(suite->getPlayerStateId(), FDN_COMPLETE);
}

#endif // NATIVE_BUILD
