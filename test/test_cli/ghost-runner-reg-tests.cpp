//
// Ghost Runner Tests — Registration file for Ghost Runner minigame
//
// Tests rewritten for Wave 18 Memory Maze redesign.
// Old rhythm game API (lanes, notes, ghostSpeedMs) replaced with
// maze navigation API (cursorRow/Col, walls[], solutionPath[]).
//

#include <gtest/gtest.h>
#include "ghost-runner-tests.hpp"

// ============================================
// CONFIG PRESET TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, EasyConfigPresets) {
    ghostRunnerEasyConfigPresets(this);
}

TEST_F(GhostRunnerTestSuite, HardConfigPresets) {
    ghostRunnerHardConfigPresets(this);
}

// ============================================
// INTRO STATE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, IntroResetsSession) {
    ghostRunnerIntroResetsSession(this);
}

TEST_F(GhostRunnerTestSuite, IntroTransitionsToShow) {
    ghostRunnerIntroTransitionsToShow(this);
}

// ============================================
// SHOW STATE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, ShowGeneratesMaze) {
    ghostRunnerShowGeneratesMaze(this);
}

TEST_F(GhostRunnerTestSuite, ShowFindsSolution) {
    ghostRunnerShowFindsSolution(this);
}

TEST_F(GhostRunnerTestSuite, ShowTransitionsToGameplay) {
    ghostRunnerShowTransitionsToGameplay(this);
}

// ============================================
// GAMEPLAY STATE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, PrimaryCyclesDirection) {
    ghostRunnerPrimaryCyclesDirection(this);
}

TEST_F(GhostRunnerTestSuite, SecondaryMovesValidDirection) {
    ghostRunnerSecondaryMovesValidDirection(this);
}

TEST_F(GhostRunnerTestSuite, WallCollisionBonks) {
    ghostRunnerWallCollisionBonks(this);
}

TEST_F(GhostRunnerTestSuite, BonkFlashTimer) {
    ghostRunnerBonkFlashTimer(this);
}

TEST_F(GhostRunnerTestSuite, ReachingExitTransitions) {
    ghostRunnerReachingExitTransitions(this);
}

// ============================================
// EVALUATE STATE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, EvaluateAdvancesRound) {
    ghostRunnerEvaluateAdvancesRound(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateRoutesToWin) {
    ghostRunnerEvaluateRoutesToWin(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateRoutesToLose) {
    ghostRunnerEvaluateRoutesToLose(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateCalculatesScore) {
    ghostRunnerEvaluateCalculatesScore(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateScoreNoBonusOnBonk) {
    ghostRunnerEvaluateScoreNoBonusOnBonk(this);
}

// ============================================
// OUTCOME TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, WinSetsOutcome) {
    ghostRunnerWinSetsOutcome(this);
}

TEST_F(GhostRunnerTestSuite, WinDetectsHardMode) {
    ghostRunnerWinDetectsHardMode(this);
}

TEST_F(GhostRunnerTestSuite, LoseSetsOutcome) {
    ghostRunnerLoseSetsOutcome(this);
}

// ============================================
// MODE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, StandaloneLoopsToIntro) {
    ghostRunnerStandaloneLoopsToIntro(this);
}

TEST_F(GhostRunnerManagedTestSuite, ManagedModeReturns) {
    ghostRunnerManagedModeReturns(this);
}

// ============================================
// EDGE CASE TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, OutOfBoundsBonks) {
    ghostRunnerOutOfBoundsBonks(this);
}

TEST_F(GhostRunnerTestSuite, ZeroLivesTransitionsToLose) {
    ghostRunnerZeroLivesTransitionsToLose(this);
}

TEST_F(GhostRunnerTestSuite, SessionResetClearsArrays) {
    ghostRunnerSessionResetClearsArrays(this);
}

// ============================================
// STATE NAME TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, StateNamesResolve) {
    ghostRunnerStateNamesResolve(this);
}
