//
// Breach Defense Tests — Registration file for Breach Defense minigame
//

#include <gtest/gtest.h>

#include "breach-defense-tests.hpp"

// ============================================
// BREACH DEFENSE TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, EasyConfigPresets) {
    breachDefenseEasyConfigPresets(this);
}

TEST_F(BreachDefenseTestSuite, HardConfigPresets) {
    breachDefenseHardConfigPresets(this);
}

TEST_F(BreachDefenseTestSuite, IntroResetsSession) {
    breachDefenseIntroResetsSession(this);
}

TEST_F(BreachDefenseTestSuite, IntroTransitionsToGameplay) {
    breachDefenseIntroTransitionsToGameplay(this);
}

TEST_F(BreachDefenseTestSuite, GameplaySpawnsThreats) {
    breachDefenseGameplaySpawnsThreats(this);
}

TEST_F(BreachDefenseTestSuite, ThreatAdvancesWithTime) {
    breachDefenseThreatAdvancesWithTime(this);
}

TEST_F(BreachDefenseTestSuite, ShieldMovesUpDown) {
    breachDefenseShieldMovesUpDown(this);
}

TEST_F(BreachDefenseTestSuite, CorrectBlock) {
    breachDefenseCorrectBlock(this);
}

TEST_F(BreachDefenseTestSuite, MissedThreat) {
    breachDefenseMissedThreat(this);
}

TEST_F(BreachDefenseTestSuite, GameplayRoutesToWin) {
    breachDefenseGameplayRoutesToWin(this);
}

TEST_F(BreachDefenseTestSuite, GameplayRoutesToLose) {
    breachDefenseGameplayRoutesToLose(this);
}

TEST_F(BreachDefenseTestSuite, WinSetsOutcome) {
    breachDefenseWinSetsOutcome(this);
}

TEST_F(BreachDefenseTestSuite, LoseSetsOutcome) {
    breachDefenseLoseSetsOutcome(this);
}

TEST_F(BreachDefenseTestSuite, StandaloneLoopsToIntro) {
    breachDefenseStandaloneLoopsToIntro(this);
}

TEST_F(BreachDefenseTestSuite, StateNamesResolve) {
    breachDefenseStateNamesResolve(this);
}

TEST_F(BreachDefenseTestSuite, MultipleThreatsActive) {
    breachDefenseMultipleThreatsActive(this);
}

TEST_F(BreachDefenseManagedTestSuite, ManagedModeReturns) {
    breachDefenseManagedModeReturns(this);
}
