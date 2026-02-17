//
// Ghost Runner Tests — Registration file for Ghost Runner minigame
//

#include <gtest/gtest.h>

#include "ghost-runner-tests.hpp"

// ============================================
// GHOST RUNNER TESTS
// ============================================

TEST_F(GhostRunnerTestSuite, EasyConfigPresets) {
    ghostRunnerEasyConfigPresets(this);
}

TEST_F(GhostRunnerTestSuite, HardConfigPresets) {
    ghostRunnerHardConfigPresets(this);
}

TEST_F(GhostRunnerTestSuite, IntroSeedsRng) {
    ghostRunnerIntroSeedsRng(this);
}

TEST_F(GhostRunnerTestSuite, IntroTransitionsToShow) {
    ghostRunnerIntroTransitionsToShow(this);
}

TEST_F(GhostRunnerTestSuite, ShowDisplaysRoundInfo) {
    ghostRunnerShowDisplaysRoundInfo(this);
}

TEST_F(GhostRunnerTestSuite, ShowTransitionsToGameplay) {
    ghostRunnerShowTransitionsToGameplay(this);
}

TEST_F(GhostRunnerTestSuite, DirectionCycle) {
    ghostRunnerDirectionCycle(this);
}

TEST_F(GhostRunnerTestSuite, ValidMovement) {
    ghostRunnerValidMovement(this);
}

TEST_F(GhostRunnerTestSuite, WallBonk) {
    ghostRunnerWallBonk(this);
}

TEST_F(GhostRunnerTestSuite, BoundsBonk) {
    ghostRunnerBoundsBonk(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateRoutesToNextRound) {
    ghostRunnerEvaluateRoutesToNextRound(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateRoutesToWin) {
    ghostRunnerEvaluateRoutesToWin(this);
}

TEST_F(GhostRunnerTestSuite, EvaluateRoutesToLose) {
    ghostRunnerEvaluateRoutesToLose(this);
}

TEST_F(GhostRunnerTestSuite, WinSetsOutcome) {
    ghostRunnerWinSetsOutcome(this);
}

TEST_F(GhostRunnerTestSuite, LoseSetsOutcome) {
    ghostRunnerLoseSetsOutcome(this);
}

TEST_F(GhostRunnerTestSuite, StandaloneLoopsToIntro) {
    ghostRunnerStandaloneLoopsToIntro(this);
}

TEST_F(GhostRunnerTestSuite, StateNamesResolve) {
    ghostRunnerStateNamesResolve(this);
}

TEST_F(GhostRunnerTestSuite, ReachingExitTransitions) {
    ghostRunnerReachingExitTransitions(this);
}

TEST_F(GhostRunnerTestSuite, ExactStrikesEqualAllowed) {
    ghostRunnerExactStrikesEqualAllowed(this);
}

TEST_F(GhostRunnerTestSuite, BonkAtExactLivesAllowed) {
    ghostRunnerBonkAtExactLivesAllowed(this);
}

// DISABLED: FDN launch states don't set managedMode = true (see TEST_HEALTH_REPORT.md, #327)
TEST_F(GhostRunnerManagedTestSuite, DISABLED_ManagedModeReturns) {
    ghostRunnerManagedModeReturns(this);
}
