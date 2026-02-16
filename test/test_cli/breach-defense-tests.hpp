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
        player_ = DeviceFactory::createDevice(0, true);
        SimpleTimer::setPlatformClock(player_.clockDriver);
    }

    void TearDown() override {
        DeviceFactory::destroyDevice(player_);
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
// TEST FUNCTIONS
// ============================================

/*
 * Test 1: Verify EASY config presets (including new fields).
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
 * Test 2: Verify HARD config presets (including new fields).
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

/*
 * Test 3: Intro resets session to defaults.
 */
void breachDefenseIntroResetsSession(BreachDefenseTestSuite* suite) {
    // Dirty the session
    auto& sess = suite->game_->getSession();
    sess.score = 999;
    sess.breaches = 5;
    sess.nextSpawnIndex = 10;
    sess.shieldLane = 4;

    // Skip to intro (index 0)
    suite->game_->skipToState(suite->device_.pdn, 0);
    suite->tick(1);

    ASSERT_EQ(sess.score, 0);
    ASSERT_EQ(sess.breaches, 0);
    ASSERT_EQ(sess.nextSpawnIndex, 0);
    ASSERT_EQ(sess.shieldLane, 0);
}

/*
 * Test 4: Intro transitions to Gameplay after timer (new 4-state flow).
 */
void breachDefenseIntroTransitionsToGameplay(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 0);
    suite->tick(1);
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_INTRO);

    // Advance past 2s intro timer
    suite->tickWithTime(25, 100);

    // Should transition directly to Gameplay (not Show)
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_GAMEPLAY);
}

/*
 * Test 5: Gameplay spawns threats automatically.
 */
void breachDefenseGameplaySpawnsThreats(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatDistance = 1000;  // Keep threats alive
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    auto& sess = suite->game_->getSession();

    // First threat spawned on mount
    ASSERT_EQ(sess.nextSpawnIndex, 1);
    ASSERT_TRUE(sess.threats[0].active);

    // Advance past spawn interval
    suite->tickWithTime(20, 100);

    // Second threat should have spawned
    ASSERT_GE(sess.nextSpawnIndex, 2);
}

/*
 * Test 6: Threats advance with time.
 */
void breachDefenseThreatAdvancesWithTime(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 10;
    suite->game_->getConfig().threatDistance = 1000;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    int initialPos = suite->game_->getSession().threats[0].position;

    // Advance a few ticks
    suite->tickWithTime(5, 15);

    ASSERT_GT(suite->game_->getSession().threats[0].position, initialPos);
}

/*
 * Test 7: Shield moves with button presses.
 */
void breachDefenseShieldMovesUpDown(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().numLanes = 5;
    suite->game_->getConfig().threatDistance = 10000;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    auto& sess = suite->game_->getSession();
    sess.shieldLane = 2;

    // Press DOWN (secondary button) — should increase lane
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(sess.shieldLane, 3);

    // Press UP (primary button) — should decrease lane
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(sess.shieldLane, 2);
}

/*
 * Test 8: Correct block — shield matches threat lane, score increases.
 */
void breachDefenseCorrectBlock(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().totalThreats = 1;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    // Set shield to match first threat lane
    auto& sess = suite->game_->getSession();
    sess.shieldLane = sess.threats[0].lane;

    // Advance until threat arrives
    suite->tickWithTime(30, 10);

    // Should have evaluated — check score
    ASSERT_EQ(sess.score, 100);
    ASSERT_EQ(sess.breaches, 0);
}

/*
 * Test 9: Missed threat — shield in wrong lane, breach counted.
 */
void breachDefenseMissedThreat(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 10;
    suite->game_->getConfig().totalThreats = 1;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    // Set shield to NOT match threat lane
    auto& sess = suite->game_->getSession();
    int threatLane = sess.threats[0].lane;
    sess.shieldLane = (threatLane == 0) ? 1 : 0;

    // Advance until threat arrives
    suite->tickWithTime(30, 10);

    // Should have evaluated — check breach counted
    ASSERT_EQ(sess.score, 0);
    ASSERT_EQ(sess.breaches, 1);
}

/*
 * Test 10: Gameplay routes to Win when all threats survived.
 */
void breachDefenseGameplayRoutesToWin(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().totalThreats = 1;
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 5;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    // Match shield to threat
    auto& sess = suite->game_->getSession();
    sess.shieldLane = sess.threats[0].lane;

    // Let threat arrive
    suite->tickWithTime(30, 10);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_WIN);
}

/*
 * Test 11: Gameplay routes to Lose when too many breaches.
 */
void breachDefenseGameplayRoutesToLose(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().totalThreats = 1;
    suite->game_->getConfig().missesAllowed = 0;
    suite->game_->getConfig().threatSpeedMs = 5;
    suite->game_->getConfig().threatDistance = 5;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    // Ensure shield does NOT match threat
    auto& sess = suite->game_->getSession();
    int threatLane = sess.threats[0].lane;
    sess.shieldLane = (threatLane == 0) ? 1 : 0;

    // Let threat arrive
    suite->tickWithTime(30, 10);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_LOSE);
}

/*
 * Test 12: Win state sets outcome to WON.
 */
void breachDefenseWinSetsOutcome(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Win is index 2
    suite->tick(1);

    ASSERT_EQ(suite->game_->getOutcome().result, MiniGameResult::WON);
}

/*
 * Test 13: Lose state sets outcome to LOST.
 */
void breachDefenseLoseSetsOutcome(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 3);  // Lose is index 3
    suite->tick(1);

    ASSERT_EQ(suite->game_->getOutcome().result, MiniGameResult::LOST);
}

/*
 * Test 14: In standalone mode, Win loops back to Intro after timer.
 */
void breachDefenseStandaloneLoopsToIntro(BreachDefenseTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Win
    suite->tick(1);
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_WIN);

    // Advance past 3s win timer
    suite->tickWithTime(35, 100);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), BREACH_INTRO);
}

/*
 * Test 15: State names resolve correctly (4 states).
 */
void breachDefenseStateNamesResolve(BreachDefenseTestSuite* suite) {
    ASSERT_STREQ(getBreachDefenseStateName(BREACH_INTRO), "BreachDefenseIntro");
    ASSERT_STREQ(getBreachDefenseStateName(BREACH_WIN), "BreachDefenseWin");
    ASSERT_STREQ(getBreachDefenseStateName(BREACH_LOSE), "BreachDefenseLose");
    ASSERT_STREQ(getBreachDefenseStateName(BREACH_GAMEPLAY), "BreachDefenseGameplay");

    // Verify getStateName routes correctly
    ASSERT_STREQ(getStateName(BREACH_INTRO), "BreachDefenseIntro");
    ASSERT_STREQ(getStateName(BREACH_GAMEPLAY), "BreachDefenseGameplay");
}

/*
 * Test 16: Multiple threats can be active simultaneously.
 */
void breachDefenseMultipleThreatsActive(BreachDefenseTestSuite* suite) {
    suite->game_->getConfig().spawnIntervalMs = 100;
    suite->game_->getConfig().maxOverlap = 3;
    suite->game_->getConfig().threatDistance = 1000;  // Keep alive
    suite->game_->getConfig().totalThreats = 5;

    suite->game_->skipToState(suite->device_.pdn, 1);  // Gameplay
    suite->tick(1);

    // Advance to spawn more threats
    suite->tickWithTime(30, 50);

    auto& sess = suite->game_->getSession();
    int activeCount = 0;
    for (int i = 0; i < 3; i++) {
        if (sess.threats[i].active) activeCount++;
    }

    ASSERT_GE(activeCount, 2) << "Should have multiple active threats";
}

/*
 * Test 17: Managed mode — FDN launches Breach Defense and returns.
 */
void breachDefenseManagedModeReturns(BreachDefenseManagedTestSuite* suite) {
    suite->advanceToIdle();

    // Trigger FDN handshake for Breach Defense (GameType 8, KonamiButton A=5)
    suite->player_.serialOutDriver->injectInput("*fdn:8:5\r");
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

    // Configure very short game
    bd->getConfig().threatSpeedMs = 5;
    bd->getConfig().threatDistance = 3;
    bd->getConfig().totalThreats = 1;
    bd->getConfig().missesAllowed = 3;

    // Advance past intro timer (2s)
    suite->tickWithTime(25, 100);

    // Should be in Gameplay by now
    int stateId = bd->getCurrentState()->getStateId();
    ASSERT_EQ(stateId, BREACH_GAMEPLAY);

    // Match shield and let threat complete
    bd->getSession().shieldLane = bd->getSession().threats[0].lane;
    suite->tickWithTime(20, 20);

    // Should be in Win now
    stateId = bd->getCurrentState()->getStateId();
    if (stateId == BREACH_WIN) {
        ASSERT_EQ(bd->getOutcome().result, MiniGameResult::WON);
        // Advance past win timer (3s)
        suite->tickWithTime(35, 100);
    }

    // Should have returned to FdnComplete
    ASSERT_EQ(suite->getPlayerStateId(), FDN_COMPLETE);
}

#endif // NATIVE_BUILD
