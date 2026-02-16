//
// Breach Defense Tests — Registration file for Breach Defense minigame
//
// Wave 20 Rewrite: Tests updated for Wave 18 combo redesign (#231)
// New 4-state flow: Intro→Gameplay→Win/Lose with inline evaluation
// Combo mechanics: multi-threat overlap, fixed rhythm spawning, continuous rendering
//

#include <gtest/gtest.h>
#include "breach-defense-tests.hpp"

// ============================================
// CONFIG PRESET TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, EasyConfigPresets) {
    breachDefenseEasyConfigPresets(this);
}

TEST_F(BreachDefenseTestSuite, HardConfigPresets) {
    breachDefenseHardConfigPresets(this);
}

// ============================================
// INTRO STATE TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, IntroResetsSession) {
    breachDefenseIntroResetsSession(this);
}

TEST_F(BreachDefenseTestSuite, IntroTransitionsToGameplay) {
    breachDefenseIntroTransitionsToGameplay(this);
}

// ============================================
// GAMEPLAY STATE TESTS — THREAT MECHANICS
// ============================================

TEST_F(BreachDefenseTestSuite, FirstThreatSpawnsImmediately) {
    breachDefenseFirstThreatSpawnsImmediately(this);
}

TEST_F(BreachDefenseTestSuite, ThreatSpawnRhythm) {
    breachDefenseThreatSpawnRhythm(this);
}

TEST_F(BreachDefenseTestSuite, MaxOverlapConstraint) {
    breachDefenseMaxOverlapConstraint(this);
}

TEST_F(BreachDefenseTestSuite, ThreatAdvancesWithTime) {
    breachDefenseThreatAdvancesWithTime(this);
}

// ============================================
// GAMEPLAY STATE TESTS — SHIELD MECHANICS
// ============================================

TEST_F(BreachDefenseTestSuite, ShieldMovesUp) {
    breachDefenseShieldMovesUp(this);
}

TEST_F(BreachDefenseTestSuite, ShieldMovesDown) {
    breachDefenseShieldMovesDown(this);
}

TEST_F(BreachDefenseTestSuite, ShieldClampedAtBottom) {
    breachDefenseShieldClampedAtBottom(this);
}

TEST_F(BreachDefenseTestSuite, ShieldClampedAtTop) {
    breachDefenseShieldClampedAtTop(this);
}

// ============================================
// GAMEPLAY STATE TESTS — INLINE EVALUATION
// ============================================

TEST_F(BreachDefenseTestSuite, CorrectBlock) {
    breachDefenseCorrectBlock(this);
}

TEST_F(BreachDefenseTestSuite, MissedThreat) {
    breachDefenseMissedThreat(this);
}

TEST_F(BreachDefenseTestSuite, MultipleThreatOverlap) {
    breachDefenseMultipleThreatOverlap(this);
}

// ============================================
// WIN/LOSE CONDITION TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, WinCondition) {
    breachDefenseWinCondition(this);
}

TEST_F(BreachDefenseTestSuite, LoseCondition) {
    breachDefenseLoseCondition(this);
}

TEST_F(BreachDefenseTestSuite, ExactBreachesEqualAllowed) {
    breachDefenseExactBreachesEqualAllowed(this);
}

// ============================================
// WIN/LOSE STATE TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, WinSetsOutcome) {
    breachDefenseWinSetsOutcome(this);
}

TEST_F(BreachDefenseTestSuite, LoseSetsOutcome) {
    breachDefenseLoseSetsOutcome(this);
}

TEST_F(BreachDefenseTestSuite, StandaloneWinLoopsToIntro) {
    breachDefenseStandaloneWinLoopsToIntro(this);
}

TEST_F(BreachDefenseTestSuite, StandaloneLoseLoopsToIntro) {
    breachDefenseStandaloneLoseLoopsToIntro(this);
}

// ============================================
// DIFFICULTY TESTS
// ============================================

TEST_F(BreachDefenseTestSuite, EasyDifficulty) {
    breachDefenseEasyDifficulty(this);
}

TEST_F(BreachDefenseTestSuite, HardDifficulty) {
    breachDefenseHardDifficulty(this);
}

// ============================================
// INTEGRATION TESTS — MANAGED MODE (FDN)
// ============================================

TEST_F(BreachDefenseManagedTestSuite, ManagedModeReturns) {
    breachDefenseManagedModeReturns(this);
}
