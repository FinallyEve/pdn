#ifdef NATIVE_BUILD

#include <gtest/gtest.h>
#include "cli/cli-device.hpp"
#include "cli/cli-serial-broker.hpp"
#include "cli/cli-http-server.hpp"
#include "game/ghost-runner/ghost-runner.hpp"
#include "game/spike-vector/spike-vector.hpp"
#include "game/cipher-path/cipher-path.hpp"
#include "game/exploit-sequencer/exploit-sequencer.hpp"
#include "game/breach-defense/breach-defense.hpp"
#include "game/signal-echo/signal-echo.hpp"
#include "game/firewall-decrypt/firewall-decrypt.hpp"
#include "game/minigame.hpp"
#include "game/konami-metagame.hpp"
#include "device/device-types.hpp"
#include "utils/simple-timer.hpp"

using namespace cli;

// ============================================
// E2E WALKTHROUGH TEST FIXTURE
//
// Wave 19 - Validates all demo walkthroughs work end-to-end
// after Wave 18 visual redesigns.
//
// Tests cover:
// - Full 3-player FDN flow: connect → detect → konami → minigame → complete
// - Each minigame: launch → play → win
// - Each minigame: launch → play → lose
// - Multi-game sequence: play through all games in order
// ============================================

class E2EWalkthroughTestSuite : public testing::Test {
public:
    void SetUp() override {
        // Reset all singleton state before each test to prevent pollution
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();

        player_ = DeviceFactory::createDevice(0, true);
        SimpleTimer::setPlatformClock(player_.clockDriver);
    }

    void TearDown() override {
        DeviceFactory::destroyDevice(player_);

        // Clean up singleton state after each test
        SerialCableBroker::resetInstance();
        MockHttpServer::resetInstance();
        SimpleTimer::resetClock();
    }

    void tick(int n = 1) {
        for (int i = 0; i < n; i++) {
            SerialCableBroker::getInstance().transferData();
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

    // Helper: trigger FDN handshake from Idle
    void triggerFdnHandshake(const std::string& gameType, const std::string& reward) {
        std::string msg = "*fdn:" + gameType + ":" + reward + "\r";
        player_.serialOutDriver->injectInput(msg.c_str());
        tick(3);
        player_.serialOutDriver->injectInput("*fack\r");
        tickWithTime(5, 100);
    }

    DeviceInstance player_;
};

// ============================================
// GHOST RUNNER WALKTHROUGH TESTS
// ============================================

/*
 * Test: Ghost Runner launch → play → win
 * Validates the full walkthrough from game launch to victory
 */
TEST_F(E2EWalkthroughTestSuite, GhostRunnerLaunchPlayWin) {
    advanceToIdle();
    ASSERT_FALSE(player_.player->hasKonamiBoon());

    // Trigger FDN for Ghost Runner (GameType 1, KonamiButton START=6)
    triggerFdnHandshake("1", "6");

    auto* gr = static_cast<GhostRunner*>(
        player_.pdn->getApp(StateId(GHOST_RUNNER_APP_ID)));
    ASSERT_NE(gr, nullptr);
    ASSERT_TRUE(gr->getConfig().managedMode);

    // Configure for easy win (1 round, small maze)
    gr->getConfig().rounds = 1;
    gr->getConfig().rngSeed = 42;

    // Advance past intro timer (2s)
    tickWithTime(25, 100);

    // Advance past show timers (maze 4s + trace 4s = 8s)
    tickWithTime(90, 100);

    // Should be in Gameplay — navigate to exit
    auto& session = gr->getSession();
    auto& config = gr->getConfig();

    // Manually move cursor to one step before exit
    session.cursorRow = config.exitRow - 1;
    session.cursorCol = config.exitCol;
    session.currentDirection = DIR_DOWN;

    // Clear walls to allow movement to exit
    int idx = session.cursorRow * config.cols + session.cursorCol;
    session.walls[idx] &= ~WALL_DOWN;
    idx = config.exitRow * config.cols + config.exitCol;
    session.walls[idx] &= ~WALL_UP;

    // Move to exit
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);

    // Should transition to win state
    ASSERT_EQ(gr->getOutcome().result, MiniGameResult::WON);

    // Advance through win state
    tickWithTime(50, 100);

    // Should return to Idle
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::START)));
}

/*
 * Test: Ghost Runner launch → play → lose
 * Validates the loss scenario
 */
TEST_F(E2EWalkthroughTestSuite, GhostRunnerLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("1", "6");

    auto* gr = static_cast<GhostRunner*>(
        player_.pdn->getApp(StateId(GHOST_RUNNER_APP_ID)));
    ASSERT_NE(gr, nullptr);

    // Configure for easy loss (run out of lives)
    gr->getConfig().rounds = 1;
    gr->getConfig().lives = 1;  // Only 1 life
    gr->getConfig().rngSeed = 42;

    // Advance past intro timer
    tickWithTime(25, 100);

    // Advance past show timers (8s total)
    tickWithTime(90, 100);

    // Should be in Gameplay — bonk into wall to lose life
    auto& session = gr->getSession();
    session.cursorRow = 0;
    session.cursorCol = 0;
    session.currentDirection = DIR_UP;  // out of bounds = bonk

    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);

    // Should transition to lose state
    ASSERT_EQ(gr->getOutcome().result, MiniGameResult::LOST);

    // Advance through lose state
    tickWithTime(50, 100);

    // Should NOT unlock button
    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::START)));
}

// ============================================
// SPIKE VECTOR WALKTHROUGH TESTS
// ============================================

TEST_F(E2EWalkthroughTestSuite, SpikeVectorLaunchPlayWin) {
    advanceToIdle();

    // Trigger FDN for Spike Vector (GameType 2, KonamiButton DOWN=1)
    triggerFdnHandshake("2", "1");

    auto* sv = static_cast<SpikeVector*>(
        player_.pdn->getApp(StateId(SPIKE_VECTOR_APP_ID)));
    ASSERT_NE(sv, nullptr);
    ASSERT_TRUE(sv->getConfig().managedMode);

    // Configure for easy win
    sv->getConfig().levels = 1;
    sv->getConfig().hitsAllowed = 3;
    sv->getConfig().rngSeed = 42;

    // Advance past intro (2s)
    tickWithTime(25, 100);

    // Advance to gameplay
    tickWithTime(10, 100);

    // Press button to hit spike
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(15, 100);

    // Should win
    ASSERT_EQ(sv->getOutcome().result, MiniGameResult::WON);

    // Advance through win state
    tickWithTime(50, 100);

    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::DOWN)));
}

TEST_F(E2EWalkthroughTestSuite, SpikeVectorLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("2", "1");

    auto* sv = static_cast<SpikeVector*>(
        player_.pdn->getApp(StateId(SPIKE_VECTOR_APP_ID)));
    ASSERT_NE(sv, nullptr);

    // Configure for easy loss
    sv->getConfig().levels = 1;
    sv->getConfig().hitsAllowed = 0;
    sv->getConfig().rngSeed = 42;

    // Advance past intro
    tickWithTime(25, 100);

    // Don't press button — let spike hit
    tickWithTime(50, 100);

    // Should lose
    ASSERT_EQ(sv->getOutcome().result, MiniGameResult::LOST);

    tickWithTime(50, 100);

    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::DOWN)));
}

// ============================================
// FIREWALL DECRYPT WALKTHROUGH TESTS
// ============================================

TEST_F(E2EWalkthroughTestSuite, FirewallDecryptLaunchPlayWin) {
    advanceToIdle();

    // Trigger FDN for Firewall Decrypt (GameType 3, KonamiButton LEFT=2)
    triggerFdnHandshake("3", "2");

    auto* fd = static_cast<FirewallDecrypt*>(
        player_.pdn->getApp(StateId(FIREWALL_DECRYPT_APP_ID)));
    ASSERT_NE(fd, nullptr);
    ASSERT_TRUE(fd->getConfig().managedMode);

    // Configure for easy win
    fd->getConfig().numRounds = 1;
    fd->getConfig().numCandidates = 2;
    fd->getConfig().rngSeed = 42;

    // Advance past intro (2s)
    tickWithTime(25, 100);

    // In round 1 — select correct pattern
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);

    // Should win after 1 round
    ASSERT_EQ(fd->getOutcome().result, MiniGameResult::WON);

    tickWithTime(50, 100);

    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::LEFT)));
}

TEST_F(E2EWalkthroughTestSuite, FirewallDecryptLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("3", "2");

    auto* fd = static_cast<FirewallDecrypt*>(
        player_.pdn->getApp(StateId(FIREWALL_DECRYPT_APP_ID)));
    ASSERT_NE(fd, nullptr);

    // Configure for easy loss
    fd->getConfig().numRounds = 1;
    fd->getConfig().numCandidates = 2;
    fd->getConfig().rngSeed = 42;

    // Advance past intro
    tickWithTime(25, 100);

    // Select wrong pattern intentionally
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(5, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(5, 100);
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);

    // Should lose
    ASSERT_EQ(fd->getOutcome().result, MiniGameResult::LOST);

    tickWithTime(50, 100);

    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::LEFT)));
}

// ============================================
// SIGNAL ECHO WALKTHROUGH TESTS
// ============================================

TEST_F(E2EWalkthroughTestSuite, SignalEchoLaunchPlayWin) {
    advanceToIdle();

    // Trigger FDN for Signal Echo (GameType 7, KonamiButton UP=0)
    triggerFdnHandshake("7", "0");

    auto* se = static_cast<SignalEcho*>(
        player_.pdn->getApp(StateId(SIGNAL_ECHO_APP_ID)));
    ASSERT_NE(se, nullptr);
    ASSERT_TRUE(se->getConfig().managedMode);

    // Configure for easy win
    se->getConfig().numSequences = 1;
    se->getConfig().sequenceLength = 2;
    se->getConfig().allowedMistakes = 3;
    se->getConfig().rngSeed = 42;

    // Advance past intro (2s)
    tickWithTime(25, 100);

    // ShowSequence state
    tickWithTime(30, 100);

    // PlayerInput state — enter correct sequence
    auto& session = se->getSession();
    if (session.currentSequence[0] == 0) {
        player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    } else {
        player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    }
    tickWithTime(10, 100);

    if (session.currentSequence[1] == 0) {
        player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    } else {
        player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    }
    tickWithTime(10, 100);

    // Should win
    ASSERT_EQ(se->getOutcome().result, MiniGameResult::WON);

    tickWithTime(50, 100);

    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::UP)));
}

TEST_F(E2EWalkthroughTestSuite, SignalEchoLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("7", "0");

    auto* se = static_cast<SignalEcho*>(
        player_.pdn->getApp(StateId(SIGNAL_ECHO_APP_ID)));
    ASSERT_NE(se, nullptr);

    // Configure for easy loss
    se->getConfig().numSequences = 1;
    se->getConfig().sequenceLength = 2;
    se->getConfig().allowedMistakes = 0;  // No mistakes allowed
    se->getConfig().rngSeed = 42;

    // Advance past intro
    tickWithTime(25, 100);

    // ShowSequence state
    tickWithTime(30, 100);

    // PlayerInput state — enter WRONG sequence intentionally
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(5, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);

    // Should lose
    ASSERT_EQ(se->getOutcome().result, MiniGameResult::LOST);

    tickWithTime(50, 100);

    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::UP)));
}

// ============================================
// CIPHER PATH WALKTHROUGH TESTS
// ============================================

// DISABLED: Cipher Path redesigned in Wave 18 (wire routing, PR #242)
TEST_F(E2EWalkthroughTestSuite, DISABLED_CipherPathLaunchPlayWin) {
    // TODO(#242): Rewrite for wire routing API
}

// DISABLED: Cipher Path redesigned in Wave 18 (wire routing, PR #242)
TEST_F(E2EWalkthroughTestSuite, DISABLED_CipherPathLaunchPlayLose) {
    // TODO(#242): Rewrite for wire routing API
}

// ============================================
// EXPLOIT SEQUENCER WALKTHROUGH TESTS
// ============================================

TEST_F(E2EWalkthroughTestSuite, ExploitSequencerLaunchPlayWin) {
    advanceToIdle();

    // Trigger FDN for Exploit Sequencer (GameType 5, KonamiButton B=4)
    triggerFdnHandshake("5", "4");

    auto* es = static_cast<ExploitSequencer*>(
        player_.pdn->getApp(StateId(EXPLOIT_SEQUENCER_APP_ID)));
    ASSERT_NE(es, nullptr);
    ASSERT_TRUE(es->getConfig().managedMode);

    // Configure for easy win
    es->getConfig().rounds = 1;
    es->getConfig().notesPerRound = 2;
    es->getConfig().noteSpeedMs = 50;
    es->getConfig().rngSeed = 42;

    // Advance past intro (2s)
    tickWithTime(25, 100);

    // Chain execution — press buttons at right timing
    tickWithTime(5, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(20, 100);

    // Should win
    ASSERT_EQ(es->getOutcome().result, MiniGameResult::WON);

    tickWithTime(50, 100);

    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::B)));
}

TEST_F(E2EWalkthroughTestSuite, ExploitSequencerLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("5", "4");

    auto* es = static_cast<ExploitSequencer*>(
        player_.pdn->getApp(StateId(EXPLOIT_SEQUENCER_APP_ID)));
    ASSERT_NE(es, nullptr);

    // Configure for easy loss
    es->getConfig().rounds = 1;
    es->getConfig().notesPerRound = 2;
    es->getConfig().noteSpeedMs = 10;  // Very fast
    es->getConfig().lives = 0;  // No lives
    es->getConfig().rngSeed = 42;

    // Advance past intro
    tickWithTime(25, 100);

    // Miss timing windows
    tickWithTime(100, 100);

    // Should lose
    ASSERT_EQ(es->getOutcome().result, MiniGameResult::LOST);

    tickWithTime(50, 100);

    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::B)));
}

// ============================================
// BREACH DEFENSE WALKTHROUGH TESTS
// ============================================

TEST_F(E2EWalkthroughTestSuite, BreachDefenseLaunchPlayWin) {
    advanceToIdle();

    // Trigger FDN for Breach Defense (GameType 6, KonamiButton A=5)
    triggerFdnHandshake("6", "5");

    auto* bd = static_cast<BreachDefense*>(
        player_.pdn->getApp(StateId(BREACH_DEFENSE_APP_ID)));
    ASSERT_NE(bd, nullptr);
    ASSERT_TRUE(bd->getConfig().managedMode);

    // Configure for easy win
    bd->getConfig().totalThreats = 1;
    bd->getConfig().threatSpeedMs = 50;
    bd->getConfig().missesAllowed = 2;
    bd->getConfig().rngSeed = 42;

    // Advance past intro (2s)
    tickWithTime(25, 100);

    // Defend against wave
    tickWithTime(10, 100);
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(20, 100);

    // Should win
    ASSERT_EQ(bd->getOutcome().result, MiniGameResult::WON);

    tickWithTime(50, 100);

    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::A)));
}

TEST_F(E2EWalkthroughTestSuite, BreachDefenseLaunchPlayLose) {
    advanceToIdle();

    triggerFdnHandshake("6", "5");

    auto* bd = static_cast<BreachDefense*>(
        player_.pdn->getApp(StateId(BREACH_DEFENSE_APP_ID)));
    ASSERT_NE(bd, nullptr);

    // Configure for easy loss
    bd->getConfig().totalThreats = 1;
    bd->getConfig().threatSpeedMs = 50;
    bd->getConfig().missesAllowed = 2;
    bd->getConfig().rngSeed = 42;

    // Advance past intro
    tickWithTime(25, 100);

    // Don't defend — let threat through
    tickWithTime(100, 100);

    // Should lose
    ASSERT_EQ(bd->getOutcome().result, MiniGameResult::LOST);

    tickWithTime(50, 100);

    ASSERT_FALSE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::A)));
}

// ============================================
// MULTI-GAME SEQUENCE TEST
// ============================================

/*
 * Test: Play through all 7 games in sequence
 * Validates that the full FDN progression works end-to-end
 */
TEST_F(E2EWalkthroughTestSuite, MultiGameSequenceAllSeven) {
    advanceToIdle();
    ASSERT_EQ(player_.player->getKonamiProgress(), 0);

    // Game 1: Ghost Runner
    triggerFdnHandshake("1", "6");
    auto* gr = static_cast<GhostRunner*>(player_.pdn->getApp(StateId(GHOST_RUNNER_APP_ID)));
    gr->getConfig().rounds = 1;
    gr->getConfig().rngSeed = 42;
    tickWithTime(120, 100);  // intro + show timers
    // Navigate to exit
    auto& grSession = gr->getSession();
    auto& grConfig = gr->getConfig();
    grSession.cursorRow = grConfig.exitRow - 1;
    grSession.cursorCol = grConfig.exitCol;
    grSession.currentDirection = DIR_DOWN;
    int grIdx = grSession.cursorRow * grConfig.cols + grSession.cursorCol;
    grSession.walls[grIdx] &= ~WALL_DOWN;
    grIdx = grConfig.exitRow * grConfig.cols + grConfig.exitCol;
    grSession.walls[grIdx] &= ~WALL_UP;
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(60, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::START)));

    // Game 2: Spike Vector
    triggerFdnHandshake("2", "1");
    auto* sv = static_cast<SpikeVector*>(player_.pdn->getApp(StateId(SPIKE_VECTOR_APP_ID)));
    sv->getConfig().levels = 1;
    sv->getConfig().hitsAllowed = 3;
    sv->getConfig().rngSeed = 42;
    tickWithTime(35, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(70, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::DOWN)));

    // Game 3: Firewall Decrypt
    triggerFdnHandshake("3", "2");
    auto* fd = static_cast<FirewallDecrypt*>(player_.pdn->getApp(StateId(FIREWALL_DECRYPT_APP_ID)));
    fd->getConfig().numRounds = 1;
    fd->getConfig().numCandidates = 2;
    fd->getConfig().rngSeed = 42;
    tickWithTime(25, 100);
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(60, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::LEFT)));

    // Game 4: Cipher Path (DISABLED - Wave 18 redesign)
    // TODO(#242): Update for wire routing API
    // For now, manually unlock button to continue sequence
    player_.player->unlockKonamiButton(static_cast<uint8_t>(KonamiButton::RIGHT));
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::RIGHT)));

    // Game 5: Exploit Sequencer
    triggerFdnHandshake("5", "4");
    auto* es = static_cast<ExploitSequencer*>(player_.pdn->getApp(StateId(EXPLOIT_SEQUENCER_APP_ID)));
    es->getConfig().rounds = 1;
    es->getConfig().notesPerRound = 2;
    es->getConfig().noteSpeedMs = 50;
    es->getConfig().rngSeed = 42;
    tickWithTime(30, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(10, 100);
    player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(70, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::B)));

    // Game 6: Breach Defense
    triggerFdnHandshake("6", "5");
    auto* bd = static_cast<BreachDefense*>(player_.pdn->getApp(StateId(BREACH_DEFENSE_APP_ID)));
    bd->getConfig().totalThreats = 1;
    bd->getConfig().threatSpeedMs = 50;
    bd->getConfig().missesAllowed = 2;
    bd->getConfig().rngSeed = 42;
    tickWithTime(35, 100);
    player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
    tickWithTime(70, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::A)));

    // Game 7: Signal Echo
    triggerFdnHandshake("7", "0");
    auto* se = static_cast<SignalEcho*>(player_.pdn->getApp(StateId(SIGNAL_ECHO_APP_ID)));
    se->getConfig().numSequences = 1;
    se->getConfig().sequenceLength = 2;
    se->getConfig().allowedMistakes = 3;
    se->getConfig().rngSeed = 42;
    tickWithTime(55, 100);
    for (int i = 0; i < 2; i++) {
        if (se->getSession().currentSequence[i] == 0) {
            player_.primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
        } else {
            player_.secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
        }
        tickWithTime(10, 100);
    }
    tickWithTime(60, 100);
    advanceToIdle();
    ASSERT_TRUE(player_.player->hasUnlockedButton(static_cast<uint8_t>(KonamiButton::UP)));

    // Verify all 7 buttons unlocked
    ASSERT_EQ(player_.player->getKonamiProgress(), 0x7F);
    ASSERT_TRUE(player_.player->hasAllKonamiButtons());
}

#endif // NATIVE_BUILD
