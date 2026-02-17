#pragma once

#ifdef NATIVE_BUILD

#include <gtest/gtest.h>
#include "cli/cli-device.hpp"
#include "cli/cli-serial-broker.hpp"
#include "cli/cli-http-server.hpp"
#include "game/ghost-runner/ghost-runner.hpp"
#include "game/ghost-runner/ghost-runner-states.hpp"
#include "game/minigame.hpp"
#include "device/device-types.hpp"
#include "utils/simple-timer.hpp"
#include <cstdint>

using namespace cli;

// ============================================
// GHOST RUNNER TEST SUITE
// ============================================

class GhostRunnerTestSuite : public testing::Test {
public:
    void SetUp() override {
        // Reset all singleton state before each test to prevent pollution
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();

        device_ = DeviceFactory::createGameDevice(0, "ghost-runner");
        SimpleTimer::setPlatformClock(device_.clockDriver);
        game_ = static_cast<GhostRunner*>(device_.game);

        // Re-enter intro so timers use the correct clock
        // (SimpleTimer clock was null during createGameDevice initialization)
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
    GhostRunner* game_ = nullptr;
};

// ============================================
// GHOST RUNNER MANAGED MODE TEST SUITE
// ============================================

class GhostRunnerManagedTestSuite : public testing::Test {
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
        return player_.game->getCurrentStateId();
    }

    GhostRunner* getGhostRunner() {
        return static_cast<GhostRunner*>(
            player_.pdn->getApp(StateId(GHOST_RUNNER_APP_ID)));
    }

    DeviceInstance player_;
};

// ============================================
// CONFIG PRESET TESTS
// ============================================

/*
 * Test: Easy config has 5x3 maze, 4 rounds, 3 lives, longer preview times.
 */
void ghostRunnerEasyConfigPresets(GhostRunnerTestSuite* suite) {
    GhostRunnerConfig easy = GHOST_RUNNER_EASY;
    ASSERT_EQ(easy.cols, 5);
    ASSERT_EQ(easy.rows, 3);
    ASSERT_EQ(easy.rounds, 4);
    ASSERT_EQ(easy.lives, 3);
    ASSERT_EQ(easy.previewMazeMs, 4000);
    ASSERT_EQ(easy.previewTraceMs, 4000);
    ASSERT_EQ(easy.bonkFlashMs, 1000);
    ASSERT_EQ(easy.startRow, 0);
    ASSERT_EQ(easy.startCol, 0);
    ASSERT_EQ(easy.exitRow, 2);
    ASSERT_EQ(easy.exitCol, 4);
}

/*
 * Test: Hard config has 7x5 maze, 6 rounds, 1 life, shorter preview times.
 */
void ghostRunnerHardConfigPresets(GhostRunnerTestSuite* suite) {
    GhostRunnerConfig hard = GHOST_RUNNER_HARD;
    ASSERT_EQ(hard.cols, 7);
    ASSERT_EQ(hard.rows, 5);
    ASSERT_EQ(hard.rounds, 6);
    ASSERT_EQ(hard.lives, 1);
    ASSERT_EQ(hard.previewMazeMs, 2500);
    ASSERT_EQ(hard.previewTraceMs, 3000);
    ASSERT_EQ(hard.bonkFlashMs, 500);
    ASSERT_EQ(hard.exitRow, 4);
    ASSERT_EQ(hard.exitCol, 6);
}

// ============================================
// INTRO STATE TESTS
// ============================================

/*
 * Test: Intro resets session on mount (score, stats, lives restored, cursor/maze cleared).
 */
void ghostRunnerIntroSeedsRng(GhostRunnerTestSuite* suite) {
    // Dirty the session before entering intro
    suite->game_->getSession().score = 999;
    suite->game_->getSession().currentRound = 10;
    suite->game_->getSession().livesRemaining = 0;
    suite->game_->getSession().cursorRow = 5;
    suite->game_->getSession().cursorCol = 5;
    suite->game_->getSession().stepsUsed = 50;
    suite->game_->getSession().bonkCount = 10;

    // Re-enter intro (skipToState 0 = Intro)
    suite->game_->skipToState(suite->device_.pdn, 0);
    suite->tick(1);

    auto& session = suite->game_->getSession();
    ASSERT_EQ(session.score, 0);
    ASSERT_EQ(session.currentRound, 0);
    ASSERT_EQ(session.livesRemaining, 3);  // restored to config.lives
    ASSERT_EQ(session.cursorRow, 0);
    ASSERT_EQ(session.cursorCol, 0);
    ASSERT_EQ(session.stepsUsed, 0);
    ASSERT_EQ(session.bonkCount, 0);
    ASSERT_EQ(session.solutionLength, 0);
}

/*
 * Test: Intro transitions to Show after timer expires.
 * Intro timer is 2000ms, so we need to advance past that.
 */
void ghostRunnerIntroTransitionsToShow(GhostRunnerTestSuite* suite) {
    State* state = suite->game_->getCurrentState();
    ASSERT_NE(state, nullptr);
    ASSERT_EQ(state->getStateId(), GHOST_INTRO);

    // Advance past 2000ms intro timer
    suite->tickWithTime(25, 100);

    state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_SHOW);
}

// ============================================
// SHOW STATE TESTS
// ============================================

/*
 * Test: Show state is entered and displays round info.
 */
void ghostRunnerShowDisplaysRoundInfo(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 1);  // index 1 = Show

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_SHOW);
}

/*
 * Test: Show transitions to Gameplay after timer (maze + trace preview).
 */
void ghostRunnerShowTransitionsToGameplay(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 1);  // index 1 = Show
    suite->tick(1);

    ASSERT_EQ(suite->game_->getCurrentStateId(), GHOST_SHOW);

    // Advance past maze preview (4000ms) + trace preview (4000ms) = 8000ms
    suite->tickWithTime(90, 100);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_GAMEPLAY);
}

// ============================================
// GAMEPLAY STATE TESTS
// ============================================

/*
 * Test: PRIMARY button cycles direction clockwise (UP→RIGHT→DOWN→LEFT).
 */
void ghostRunnerDirectionCycle(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // index 2 = Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    ASSERT_EQ(session.currentDirection, DIR_RIGHT);  // starts facing right

    // Press PRIMARY to cycle direction
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_DOWN);

    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_LEFT);

    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_UP);

    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_RIGHT);  // wraps around
}

/*
 * Test: SECONDARY button moves cursor in current direction (no walls).
 */
void ghostRunnerValidMovement(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;
    suite->game_->seedRng(42);

    // Generate maze first
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show state generates maze
    suite->tick(1);
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    int initialRow = session.cursorRow;
    int initialCol = session.cursorCol;
    int initialSteps = session.stepsUsed;

    // Face RIGHT and move (if no wall)
    session.currentDirection = DIR_RIGHT;

    // Check if there's a wall to the right
    int cellIdx = initialRow * suite->game_->getConfig().cols + initialCol;
    bool hasWallRight = (session.walls[cellIdx] & WALL_RIGHT) != 0;

    if (!hasWallRight && initialCol < suite->game_->getConfig().cols - 1) {
        suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
        suite->tick(1);

        ASSERT_EQ(session.cursorCol, initialCol + 1);
        ASSERT_EQ(session.cursorRow, initialRow);
        ASSERT_EQ(session.stepsUsed, initialSteps + 1);
    }
}

/*
 * Test: Moving into a wall causes bonk (loses life, doesn't move).
 */
void ghostRunnerWallBonk(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;
    suite->game_->seedRng(42);

    // Generate maze
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show
    suite->tick(1);
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Manually create a wall in a known direction to guarantee bonk
    int cellIdx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[cellIdx] |= WALL_UP;  // Add wall above current position

    int livesBeforeBonk = session.livesRemaining;
    int bonksBeforeBonk = session.bonkCount;
    int rowBeforeBonk = session.cursorRow;
    int colBeforeBonk = session.cursorCol;

    // Face UP and try to move into wall
    session.currentDirection = DIR_UP;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    // Should have lost a life but not moved
    ASSERT_EQ(session.livesRemaining, livesBeforeBonk - 1);
    ASSERT_EQ(session.bonkCount, bonksBeforeBonk + 1);
    ASSERT_EQ(session.cursorRow, rowBeforeBonk);  // didn't move
    ASSERT_EQ(session.cursorCol, colBeforeBonk);
    ASSERT_TRUE(session.mazeFlashActive);  // bonk triggers maze flash
}

/*
 * Test: Moving out of bounds causes bonk.
 */
void ghostRunnerBoundsBonk(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Move cursor to top-left corner (0, 0)
    session.cursorRow = 0;
    session.cursorCol = 0;
    int livesBeforeBonk = session.livesRemaining;

    // Try to move up (out of bounds)
    session.currentDirection = DIR_UP;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);

    // Should have bonked
    ASSERT_EQ(session.livesRemaining, livesBeforeBonk - 1);
    ASSERT_EQ(session.cursorRow, 0);  // didn't move
    ASSERT_EQ(session.cursorCol, 0);
}

// ============================================
// EVALUATE STATE TESTS
// ============================================

/*
 * Test: Evaluate routes to next round (Show) when round completes with lives remaining.
 */
void ghostRunnerEvaluateRoutesToNextRound(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    session.currentRound = 0;
    session.livesRemaining = 2;  // still alive
    session.score = 100;
    config.rounds = 4;

    suite->game_->skipToState(suite->device_.pdn, 3);  // index 3 = Evaluate
    suite->tick(3);

    ASSERT_EQ(session.currentRound, 1) << "Round should advance";

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_SHOW) << "Should go to Show for next round";
}

/*
 * Test: Evaluate routes to Win when all rounds completed.
 */
void ghostRunnerEvaluateRoutesToWin(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    session.currentRound = 3;   // Last round (0-indexed, rounds=4)
    session.livesRemaining = 1;
    session.score = 200;
    config.rounds = 4;

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
    suite->tick(3);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_WIN);
}

/*
 * Test: Evaluate routes to Lose when no lives remaining.
 */
void ghostRunnerEvaluateRoutesToLose(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    session.currentRound = 0;
    session.livesRemaining = 0;  // No lives left
    session.bonkCount = 5;
    config.rounds = 4;

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
    suite->tick(3);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_LOSE);
}

// ============================================
// OUTCOME TESTS
// ============================================

/*
 * Test: Win state sets outcome to WON with score and correct hardMode.
 */
void ghostRunnerWinSetsOutcome(GhostRunnerTestSuite* suite) {
    suite->game_->getSession().score = 300;
    suite->game_->getConfig().cols = 5;  // EASY mode
    suite->game_->skipToState(suite->device_.pdn, 4);  // index 4 = Win
    suite->tick(1);

    const MiniGameOutcome& outcome = suite->game_->getOutcome();
    ASSERT_EQ(outcome.result, MiniGameResult::WON);
    ASSERT_EQ(outcome.score, 300);
    // Easy config: cols=5 -> hardMode=false
    ASSERT_FALSE(outcome.hardMode);
}

/*
 * Test: Lose state sets outcome to LOST.
 */
void ghostRunnerLoseSetsOutcome(GhostRunnerTestSuite* suite) {
    suite->game_->getSession().score = 100;
    suite->game_->skipToState(suite->device_.pdn, 5);  // index 5 = Lose
    suite->tick(1);

    const MiniGameOutcome& outcome = suite->game_->getOutcome();
    ASSERT_EQ(outcome.result, MiniGameResult::LOST);
    ASSERT_EQ(outcome.score, 100);
}

// ============================================
// MODE TESTS
// ============================================

/*
 * Test: In standalone mode, Win loops back to Intro.
 */
void ghostRunnerStandaloneLoopsToIntro(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().managedMode = false;
    suite->game_->skipToState(suite->device_.pdn, 4);  // Win

    // Advance past 3s win display timer
    suite->tickWithTime(40, 100);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_INTRO);
}

/*
 * Test: In managed mode (FDN), Win returns to FdnComplete.
 */
void ghostRunnerManagedModeReturns(GhostRunnerManagedTestSuite* suite) {
    suite->advanceToIdle();

    // Trigger FDN handshake for Ghost Runner (FdnGameType 1, KonamiButton START=6)
    suite->player_.serialOutDriver->injectInput("*fdn:1:6\r");
    for (int i = 0; i < 3; i++) {
        SerialCableBroker::getInstance().transferData();
        suite->player_.pdn->loop();
    }
    ASSERT_EQ(suite->getPlayerStateId(), FDN_DETECTED);

    // Complete handshake
    suite->player_.serialOutDriver->injectInput("*fack\r");
    suite->tickWithTime(5, 100);

    // Should be in Ghost Runner now
    auto* gr = suite->getGhostRunner();
    ASSERT_NE(gr, nullptr);
    ASSERT_TRUE(gr->getConfig().managedMode);

    // Configure for quick win — small maze, 1 round
    gr->getConfig().cols = 3;
    gr->getConfig().rows = 2;
    gr->getConfig().rounds = 1;
    gr->getConfig().rngSeed = 42;
    gr->getConfig().exitRow = 1;
    gr->getConfig().exitCol = 2;

    // Advance past intro timer (2s)
    suite->tickWithTime(25, 100);

    // Should be in Show state (generates maze)
    ASSERT_EQ(gr->getCurrentStateId(), GHOST_SHOW);

    // Advance past show timer (4s + 4s preview)
    suite->tickWithTime(90, 100);

    // Should be in Gameplay
    ASSERT_EQ(gr->getCurrentStateId(), GHOST_GAMEPLAY);

    // Manually place cursor at exit to win immediately
    auto& session = gr->getSession();
    session.cursorRow = gr->getConfig().exitRow;
    session.cursorCol = gr->getConfig().exitCol;

    suite->tick(5);

    // Should transition to Win
    suite->tickWithTime(5, 100);
    ASSERT_EQ(gr->getCurrentStateId(), GHOST_WIN);
    ASSERT_EQ(gr->getOutcome().result, MiniGameResult::WON);

    // Advance past win timer (3s)
    suite->tickWithTime(35, 100);

    // Should return to Quickdraw's FdnComplete state
    ASSERT_EQ(suite->getPlayerStateId(), FDN_COMPLETE);
}

// ============================================
// EDGE CASE TESTS
// ============================================

/*
 * Test: Reaching exit cell triggers transition to Evaluate.
 */
void ghostRunnerReachingExitTransitions(GhostRunnerTestSuite* suite) {
    // Go through Show to generate maze, then to Gameplay
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show
    suite->tick(1);
    suite->tickWithTime(90, 100);  // Wait for show to complete
    ASSERT_EQ(suite->game_->getCurrentStateId(), GHOST_GAMEPLAY);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Place cursor one cell before exit (to the left of exit)
    session.cursorRow = config.exitRow;
    session.cursorCol = config.exitCol - 1;

    // Clear any wall blocking movement to exit
    int cellIdx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[cellIdx] &= ~WALL_RIGHT;

    // Face right and move to exit
    session.currentDirection = DIR_RIGHT;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    State* state = suite->game_->getCurrentState();
    EXPECT_EQ(state->getStateId(), GHOST_EVALUATE) << "Should transition to Evaluate when reaching exit";
}

/*
 * Test: Having exactly 1 life remaining doesn't lose (only 0 causes loss).
 */
void ghostRunnerExactStrikesEqualAllowed(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rounds = 5;

    auto& session = suite->game_->getSession();
    session.currentRound = 0;
    session.livesRemaining = 1;  // Exactly 1 life left

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
    suite->tick(3);

    // 1 life remaining -> should continue to next round, not lose
    State* state = suite->game_->getCurrentState();
    EXPECT_NE(state->getStateId(), GHOST_LOSE) << "Should not lose with 1 life remaining";
    EXPECT_EQ(state->getStateId(), GHOST_SHOW) << "Should advance to next round";
    EXPECT_EQ(session.currentRound, 1);
}

/*
 * Test: Bonk with exactly 1 life remaining reduces to 0 lives, triggers lose.
 */
void ghostRunnerBonkAtExactLivesAllowed(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    session.livesRemaining = 1;  // Only 1 life left

    // Add a wall to guarantee bonk
    int cellIdx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[cellIdx] |= WALL_UP;

    // Try to move into wall (will bonk and lose last life)
    session.currentDirection = DIR_UP;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    EXPECT_EQ(session.livesRemaining, 0);

    // Should transition to evaluate/lose
    State* state = suite->game_->getCurrentState();
    EXPECT_TRUE(state->getStateId() == GHOST_EVALUATE || state->getStateId() == GHOST_LOSE)
        << "Should transition to evaluate/lose with 0 lives";
}

// ============================================
// STATE NAME TESTS
// ============================================

/*
 * Test: All Ghost Runner state names resolve correctly.
 */
void ghostRunnerStateNamesResolve(GhostRunnerTestSuite* suite) {
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_INTRO), "GhostRunnerIntro");
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_WIN), "GhostRunnerWin");
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_LOSE), "GhostRunnerLose");
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_SHOW), "GhostRunnerShow");
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_GAMEPLAY), "GhostRunnerGameplay");
    ASSERT_STREQ(getGhostRunnerStateName(GHOST_EVALUATE), "GhostRunnerEvaluate");

    // Verify getStateName routes correctly
    ASSERT_STREQ(getStateName(GHOST_INTRO), "GhostRunnerIntro");
    ASSERT_STREQ(getStateName(GHOST_GAMEPLAY), "GhostRunnerGameplay");
}

#endif // NATIVE_BUILD
