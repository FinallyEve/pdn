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
        return player_.game->getCurrentState()->getStateId();
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
 * Test: Easy config has correct maze dimensions and timing.
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
    ASSERT_FLOAT_EQ(easy.previewShrinkPerRound, 0.85f);
}

/*
 * Test: Hard config has larger maze and shorter preview times.
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
    ASSERT_FLOAT_EQ(hard.previewShrinkPerRound, 0.75f);
}

// ============================================
// INTRO STATE TESTS
// ============================================

/*
 * Test: Intro resets session state completely.
 */
void ghostRunnerIntroResetsSession(GhostRunnerTestSuite* suite) {
    // Dirty the session before entering intro
    suite->game_->getSession().cursorRow = 5;
    suite->game_->getSession().cursorCol = 3;
    suite->game_->getSession().currentRound = 10;
    suite->game_->getSession().livesRemaining = 0;
    suite->game_->getSession().score = 999;
    suite->game_->getSession().bonkCount = 20;
    suite->game_->getSession().stepsUsed = 50;
    suite->game_->getSession().solutionLength = 10;

    // Re-enter intro (skipToState 0 = Intro)
    suite->game_->skipToState(suite->device_.pdn, 0);
    suite->tick(1);

    auto& session = suite->game_->getSession();
    ASSERT_EQ(session.cursorRow, 0);
    ASSERT_EQ(session.cursorCol, 0);
    ASSERT_EQ(session.currentDirection, DIR_RIGHT);
    ASSERT_EQ(session.currentRound, 0);
    ASSERT_EQ(session.livesRemaining, 3);  // restored to config.lives
    ASSERT_EQ(session.score, 0);
    ASSERT_EQ(session.bonkCount, 0);
    ASSERT_EQ(session.stepsUsed, 0);
    ASSERT_EQ(session.solutionLength, 0);
    ASSERT_FALSE(session.mazeFlashActive);
}

/*
 * Test: Intro transitions to Show after timer expires.
 */
void ghostRunnerIntroTransitionsToShow(GhostRunnerTestSuite* suite) {
    State* state = suite->game_->getCurrentState();
    ASSERT_NE(state, nullptr);
    ASSERT_EQ(state->getStateId(), GHOST_INTRO);

    // Advance past 2s intro timer
    suite->tickWithTime(25, 100);

    state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_SHOW);
}

// ============================================
// SHOW STATE TESTS
// ============================================

/*
 * Test: Show state generates maze with walls.
 */
void ghostRunnerShowGeneratesMaze(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;  // deterministic
    suite->game_->skipToState(suite->device_.pdn, 1);  // index 1 = Show
    suite->tick(1);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_SHOW);

    // Check that walls were generated (not all zeros)
    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    bool hasWalls = false;
    for (int i = 0; i < config.rows * config.cols; i++) {
        if (session.walls[i] != 0) {
            hasWalls = true;
            break;
        }
    }
    ASSERT_TRUE(hasWalls) << "Maze should have walls";
}

/*
 * Test: Show state finds solution path from start to exit.
 */
void ghostRunnerShowFindsSolution(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show
    suite->tick(1);

    auto& session = suite->game_->getSession();

    // Solution should be non-empty
    ASSERT_GT(session.solutionLength, 0) << "Solution should exist";

    // Solution should be reasonable length (not absurdly long)
    ASSERT_LT(session.solutionLength, 50) << "Solution should be sane length";
}

/*
 * Test: Show transitions to Gameplay after preview timers.
 */
void ghostRunnerShowTransitionsToGameplay(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_SHOW);

    // Advance past maze preview (4s) + trace preview (4s) = 8s total
    suite->tickWithTime(90, 100);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_GAMEPLAY);
}

// ============================================
// GAMEPLAY STATE TESTS
// ============================================

/*
 * Test: PRIMARY button cycles direction clockwise.
 */
void ghostRunnerPrimaryCyclesDirection(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    ASSERT_EQ(session.currentDirection, DIR_RIGHT);  // starts facing RIGHT

    // Press PRIMARY to cycle: RIGHT -> DOWN
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_DOWN);

    // Press PRIMARY to cycle: DOWN -> LEFT
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_LEFT);

    // Press PRIMARY to cycle: LEFT -> UP
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_UP);

    // Press PRIMARY to cycle: UP -> RIGHT (wrap around)
    suite->device_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(1);
    ASSERT_EQ(session.currentDirection, DIR_RIGHT);
}

/*
 * Test: SECONDARY button moves in current direction (valid move).
 */
void ghostRunnerSecondaryMovesValidDirection(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show to generate maze
    suite->tickWithTime(90, 100);  // advance to Gameplay

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_GAMEPLAY);
    ASSERT_EQ(session.cursorRow, config.startRow);
    ASSERT_EQ(session.cursorCol, config.startCol);

    int initialSteps = session.stepsUsed;
    int initialRow = session.cursorRow;
    int initialCol = session.cursorCol;

    // Move in a valid direction (depends on maze, but we can try moving)
    // If we hit a wall, the cursor won't move but bonk will trigger
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    // Either moved (stepsUsed increased) or bonked (lives decreased)
    bool moved = (session.stepsUsed > initialSteps);
    bool bonked = (session.bonkCount > 0);
    ASSERT_TRUE(moved || bonked) << "SECONDARY should either move or bonk";
}

/*
 * Test: Hitting a wall triggers bonk (loses life, flashes maze).
 */
void ghostRunnerWallCollisionBonks(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().rngSeed = 42;
    suite->game_->getConfig().lives = 3;
    suite->game_->skipToState(suite->device_.pdn, 1);  // Show
    suite->tickWithTime(90, 100);  // advance to Gameplay

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Manually set cursor position and walls to guarantee a wall collision
    session.cursorRow = 0;
    session.cursorCol = 0;
    session.currentDirection = DIR_UP;  // try to go up from (0,0) -> out of bounds
    session.livesRemaining = 3;
    session.bonkCount = 0;

    // Try to move UP from top-left corner (out of bounds = bonk)
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    // Should have bonked and lost a life
    ASSERT_EQ(session.livesRemaining, 2) << "Should lose 1 life on bonk";
    ASSERT_EQ(session.bonkCount, 1);
    ASSERT_TRUE(session.mazeFlashActive) << "Maze should flash on bonk";
}

/*
 * Test: Bonk flash timer makes maze visible temporarily.
 */
void ghostRunnerBonkFlashTimer(GhostRunnerTestSuite* suite) {
    suite->game_->getConfig().bonkFlashMs = 500;
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();

    // Manually trigger bonk by setting direction to guaranteed wall
    session.cursorRow = 0;
    session.cursorCol = 0;
    session.currentDirection = DIR_LEFT;  // try to go left from (0,0) -> out of bounds

    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    ASSERT_TRUE(session.mazeFlashActive);

    // Advance past bonk flash timer (500ms)
    suite->tickWithTime(10, 60);

    ASSERT_FALSE(session.mazeFlashActive) << "Maze flash should expire";
}

/*
 * Test: Reaching exit cell transitions to Evaluate.
 */
void ghostRunnerReachingExitTransitions(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Manually place cursor at exit
    session.cursorRow = config.exitRow;
    session.cursorCol = config.exitCol;

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_GAMEPLAY);

    // Trigger the check (game checks in moveCallback, but we can force by setting and ticking)
    suite->tick(5);

    // Since we're already at exit, we need to trigger the transition manually by simulating arrival
    // Let's instead move one cell away and then move to exit
    session.cursorRow = config.exitRow - 1;
    session.cursorCol = config.exitCol;
    session.currentDirection = DIR_DOWN;

    // Clear the wall between exit-1 and exit to allow movement
    int idx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[idx] &= ~WALL_DOWN;  // remove down wall
    idx = config.exitRow * config.cols + config.exitCol;
    session.walls[idx] &= ~WALL_UP;  // remove up wall from exit

    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(5);

    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_EVALUATE);
}

// ============================================
// EVALUATE STATE TESTS
// ============================================

/*
 * Test: Evaluate routes to next round when rounds remaining.
 */
void ghostRunnerEvaluateAdvancesRound(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    session.currentRound = 0;
    session.livesRemaining = 2;
    session.solutionLength = 5;
    config.rounds = 4;

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
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
    session.livesRemaining = 2;
    session.solutionLength = 5;
    session.bonkCount = 0;
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
    session.solutionLength = 5;
    config.rounds = 4;

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
    suite->tick(3);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_LOSE);
}

/*
 * Test: Evaluate calculates score correctly (optimal moves + zero-bonk bonus).
 */
void ghostRunnerEvaluateCalculatesScore(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();

    session.currentRound = 0;
    session.livesRemaining = 3;
    session.score = 0;
    session.solutionLength = 8;  // 8 optimal moves
    session.bonkCount = 0;  // zero bonks = bonus

    suite->game_->skipToState(suite->device_.pdn, 3);  // Evaluate
    suite->tick(1);

    // Score = (8 * 100) + 500 bonus = 1300
    ASSERT_EQ(session.score, 1300);
}

/*
 * Test: Evaluate calculates score without bonus when bonks occurred.
 */
void ghostRunnerEvaluateScoreNoBonusOnBonk(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();

    session.currentRound = 0;
    session.livesRemaining = 2;
    session.score = 0;
    session.solutionLength = 6;
    session.bonkCount = 2;  // had bonks = no bonus

    suite->game_->skipToState(suite->device_.pdn, 3);
    suite->tick(1);

    // Score = 6 * 100 = 600 (no bonus)
    ASSERT_EQ(session.score, 600);
}

// ============================================
// OUTCOME TESTS
// ============================================

/*
 * Test: Win state sets outcome to WON with correct hardMode flag.
 */
void ghostRunnerWinSetsOutcome(GhostRunnerTestSuite* suite) {
    suite->game_->getSession().score = 2400;
    suite->game_->getConfig().cols = 5;  // EASY mode
    suite->game_->getConfig().rows = 3;
    suite->game_->skipToState(suite->device_.pdn, 4);  // Win
    suite->tick(1);

    const MiniGameOutcome& outcome = suite->game_->getOutcome();
    ASSERT_EQ(outcome.result, MiniGameResult::WON);
    ASSERT_EQ(outcome.score, 2400);
    ASSERT_FALSE(outcome.hardMode) << "5x3 maze is EASY mode";
}

/*
 * Test: Win state detects hard mode correctly (7x5 maze).
 */
void ghostRunnerWinDetectsHardMode(GhostRunnerTestSuite* suite) {
    suite->game_->getSession().score = 3000;
    suite->game_->getConfig().cols = 7;  // HARD mode
    suite->game_->getConfig().rows = 5;
    suite->game_->skipToState(suite->device_.pdn, 4);  // Win
    suite->tick(1);

    const MiniGameOutcome& outcome = suite->game_->getOutcome();
    ASSERT_EQ(outcome.result, MiniGameResult::WON);
    ASSERT_TRUE(outcome.hardMode) << "7x5 maze is HARD mode";
}

/*
 * Test: Lose state sets outcome to LOST.
 */
void ghostRunnerLoseSetsOutcome(GhostRunnerTestSuite* suite) {
    suite->game_->getSession().score = 500;
    suite->game_->skipToState(suite->device_.pdn, 5);  // Lose
    suite->tick(1);

    const MiniGameOutcome& outcome = suite->game_->getOutcome();
    ASSERT_EQ(outcome.result, MiniGameResult::LOST);
    ASSERT_EQ(outcome.score, 500);
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
    suite->tickWithTime(35, 100);

    State* state = suite->game_->getCurrentState();
    ASSERT_EQ(state->getStateId(), GHOST_INTRO);
}

/*
 * Test: In managed mode (FDN), Win returns to FdnComplete.
 */
void ghostRunnerManagedModeReturns(GhostRunnerManagedTestSuite* suite) {
    suite->advanceToIdle();

    // Trigger FDN handshake for Ghost Runner (GameType 1, KonamiButton START=6)
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

    // Configure for quick win (1 round, small maze)
    gr->getConfig().rounds = 1;
    gr->getConfig().rngSeed = 42;

    // Advance past intro timer (2s)
    suite->tickWithTime(25, 100);
    ASSERT_EQ(gr->getCurrentState()->getStateId(), GHOST_SHOW);

    // Advance past show timers (4s + 4s = 8s)
    suite->tickWithTime(90, 100);
    ASSERT_EQ(gr->getCurrentState()->getStateId(), GHOST_GAMEPLAY);

    // Manually place cursor at exit and trigger transition
    auto& session = gr->getSession();
    auto& config = gr->getConfig();

    // Move cursor one step before exit
    session.cursorRow = config.exitRow - 1;
    session.cursorCol = config.exitCol;
    session.currentDirection = DIR_DOWN;

    // Clear walls to allow movement to exit
    int idx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[idx] &= ~WALL_DOWN;
    idx = config.exitRow * config.cols + config.exitCol;
    session.walls[idx] &= ~WALL_UP;

    // Move to exit
    suite->player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tickWithTime(5, 100);

    // Should be in Evaluate, then Win
    suite->tick(5);
    ASSERT_EQ(gr->getCurrentState()->getStateId(), GHOST_WIN);
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
 * Test: Cursor cannot move out of bounds (bonks instead).
 */
void ghostRunnerOutOfBoundsBonks(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();
    auto& config = suite->game_->getConfig();

    // Place cursor at top-left corner
    session.cursorRow = 0;
    session.cursorCol = 0;
    session.livesRemaining = 3;
    session.bonkCount = 0;

    // Try to move UP (out of bounds)
    session.currentDirection = DIR_UP;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    ASSERT_EQ(session.cursorRow, 0) << "Cursor should not move out of bounds";
    ASSERT_EQ(session.bonkCount, 1);
    ASSERT_EQ(session.livesRemaining, 2);

    // Try to move LEFT (out of bounds)
    session.currentDirection = DIR_LEFT;
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(3);

    ASSERT_EQ(session.cursorCol, 0) << "Cursor should not move out of bounds";
    ASSERT_EQ(session.bonkCount, 2);
    ASSERT_EQ(session.livesRemaining, 1);
}

/*
 * Test: Losing all lives during gameplay transitions to Evaluate then Lose.
 */
void ghostRunnerZeroLivesTransitionsToLose(GhostRunnerTestSuite* suite) {
    suite->game_->skipToState(suite->device_.pdn, 2);  // Gameplay
    suite->tick(1);

    auto& session = suite->game_->getSession();

    session.cursorRow = 0;
    session.cursorCol = 0;
    session.livesRemaining = 1;  // Only 1 life left
    session.currentDirection = DIR_UP;

    // Bonk to lose last life
    suite->device_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    suite->tick(5);

    ASSERT_EQ(session.livesRemaining, 0);

    // Should transition to Evaluate
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_EVALUATE);

    suite->tick(5);

    // Then to Lose
    ASSERT_EQ(suite->game_->getCurrentState()->getStateId(), GHOST_LOSE);
}

/*
 * Test: Session reset clears walls and solution arrays.
 */
void ghostRunnerSessionResetClearsArrays(GhostRunnerTestSuite* suite) {
    auto& session = suite->game_->getSession();

    // Dirty session arrays
    for (int i = 0; i < 10; i++) {
        session.walls[i] = 0xFF;
        session.solutionPath[i] = i;
    }
    session.solutionLength = 10;

    session.reset();

    // Check walls cleared
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(session.walls[i], 0);
        ASSERT_EQ(session.solutionPath[i], 0);
    }
    ASSERT_EQ(session.solutionLength, 0);
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
