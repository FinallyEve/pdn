//
// Full Demo Walkthrough Test Registrations - CI tests for all 7 FDN games
//

#ifdef NATIVE_BUILD

#include <gtest/gtest.h>
#include "full-demo-walkthrough-tests.hpp"

// ============================================
// FULL DEMO WALKTHROUGH TESTS
// ============================================

/**
 * Test: Complete 7-game progression demo.
 *
 * Spawns 1 player + 7 NPCs, plays through all games.
 * Validates no cascade failures, all buttons unlock.
 *
 * Timeout: 5 minutes (enforced by --gtest_timeout)
 */
TEST_F(FullDemoWalkthroughTestSuite, FullDemoAllSevenGames) {
    fullDemoAllSevenGames(this);
}

/**
 * Test: Cable disconnect recovery (Bug #207 fix validation).
 *
 * Disconnects cable mid-game, verifies clean return to Idle.
 * Reconnects to different NPC, verifies new game launches.
 *
 * Validates Wave 19 fix prevents cascade failures.
 */
TEST_F(FullDemoWalkthroughTestSuite, CableDisconnectRecovery) {
    cableDisconnectRecovery(this);
}

/**
 * Test: Cable disconnect at all game phases.
 *
 * Validates disconnect during intro, gameplay, evaluation.
 * All phases should cleanly abort to Idle.
 */
TEST_F(FullDemoWalkthroughTestSuite, CableDisconnectAllPhases) {
    cableDisconnectAllPhases(this);
}

#endif // NATIVE_BUILD
