//
// Cipher Path Tests — Registration file for Cipher Path minigame
//
// Wire routing puzzle tests for Wave 18+ (BioShock-inspired mechanics)
//

#include <gtest/gtest.h>
#include "cipher-path-tests.hpp"

// ============================================
// CONFIG PRESET TESTS
// ============================================

TEST_F(CipherPathTestSuite, EasyConfigPresets) {
    cipherPathEasyConfigPresets(this);
}

TEST_F(CipherPathTestSuite, HardConfigPresets) {
    cipherPathHardConfigPresets(this);
}

TEST_F(CipherPathTestSuite, SessionResetClearsState) {
    cipherPathSessionResetClearsState(this);
}

// ============================================
// INTRO TESTS
// ============================================

TEST_F(CipherPathTestSuite, IntroTransitionsToShow) {
    cipherPathIntroTransitionsToShow(this);
}

// ============================================
// SHOW TESTS
// ============================================

TEST_F(CipherPathTestSuite, ShowGeneratesPath) {
    cipherPathShowGeneratesPath(this);
}

TEST_F(CipherPathTestSuite, ShowScramblesInternalTiles) {
    cipherPathShowScramblesInternalTiles(this);
}

TEST_F(CipherPathTestSuite, ShowTransitionsToGameplay) {
    cipherPathShowTransitionsToGameplay(this);
}

// ============================================
// GAMEPLAY TESTS — TILE ROTATION
// ============================================

TEST_F(CipherPathTestSuite, RotateTileAdvancesRotation) {
    cipherPathRotateTileAdvancesRotation(this);
}

TEST_F(CipherPathTestSuite, CannotRotateTerminals) {
    cipherPathCannotRotateTerminals(this);
}

TEST_F(CipherPathTestSuite, RotateFourTimesReturnsToStart) {
    cipherPathRotateFourTimesReturnsToStart(this);
}

// ============================================
// GAMEPLAY TESTS — CURSOR NAVIGATION
// ============================================

TEST_F(CipherPathTestSuite, NavigateCursorAdvances) {
    cipherPathNavigateCursorAdvances(this);
}

TEST_F(CipherPathTestSuite, CursorWrapsAround) {
    cipherPathCursorWrapsAround(this);
}

// ============================================
// GAMEPLAY TESTS — FLOW MECHANICS
// ============================================

TEST_F(CipherPathTestSuite, FlowStartsAtFirstTile) {
    cipherPathFlowStartsAtFirstTile(this);
}

TEST_F(CipherPathTestSuite, FlowAdvancesWithTime) {
    cipherPathFlowAdvancesWithTime(this);
}

// ============================================
// GAMEPLAY TESTS — WIN/LOSE CONDITIONS
// ============================================

TEST_F(CipherPathTestSuite, CorrectSolutionWins) {
    cipherPathCorrectSolutionWins(this);
}

TEST_F(CipherPathTestSuite, IncorrectRotationLoses) {
    cipherPathIncorrectRotationLoses(this);
}

// ============================================
// EVALUATE TESTS
// ============================================

TEST_F(CipherPathTestSuite, EvaluateRoutesToNextRound) {
    cipherPathEvaluateRoutesToNextRound(this);
}

TEST_F(CipherPathTestSuite, EvaluateRoutesToWin) {
    cipherPathEvaluateRoutesToWin(this);
}

TEST_F(CipherPathTestSuite, EvaluateRoutesToLose) {
    cipherPathEvaluateRoutesToLose(this);
}

// ============================================
// WIN/LOSE OUTCOME TESTS
// ============================================

TEST_F(CipherPathTestSuite, WinSetsOutcome) {
    cipherPathWinSetsOutcome(this);
}

TEST_F(CipherPathTestSuite, LoseSetsOutcome) {
    cipherPathLoseSetsOutcome(this);
}

// ============================================
// STANDALONE LOOP TEST
// ============================================

TEST_F(CipherPathTestSuite, StandaloneLoopsToIntro) {
    cipherPathStandaloneLoopsToIntro(this);
}

// ============================================
// DIFFICULTY TESTS
// ============================================

TEST_F(CipherPathTestSuite, EasyModeSlowerFlow) {
    cipherPathEasyModeSlowerFlow(this);
}

TEST_F(CipherPathTestSuite, HardModeFasterFlow) {
    cipherPathHardModeFasterFlow(this);
}

// Tile connection validation tests removed (see cipher-path-tests.hpp)

// ============================================
// MANAGED MODE TEST (FDN INTEGRATION)
// ============================================

TEST_F(CipherPathManagedTestSuite, ManagedModeReturns) {
    cipherPathManagedModeReturns(this);
}
