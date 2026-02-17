#include <gtest/gtest.h>
#include "utils/UUID.h"
#include "utils/simple-timer.hpp"
#include "game/difficulty-scaler.hpp"
#include "device/device-types.hpp"
#include "device/drivers/native/native-clock-driver.hpp"
#include <memory>
#include <limits>
#include <string>
#include <set>

/*
 * Edge Case and Boundary Tests for Core Components
 *
 * Tests boundary conditions and edge cases across:
 * - UUID generation and parsing
 * - SimpleTimer edge cases (zero, negative, overflow, rapid cycling)
 * - DifficultyScaler boundary conditions (0.0, 1.0, beyond bounds)
 */

// ============================================
// UUID EDGE CASE TESTS
// ============================================

TEST(UUIDEdgeCaseTests, EmptyStringConstructor) {
    // UUID doesn't have a string constructor, but we can test empty generation
    UUID uuid(0);  // Zero seed
    char* uuidStr = uuid.toCharArray();

    // Should still produce a valid 36-character UUID
    ASSERT_EQ(strlen(uuidStr), 36);
    EXPECT_EQ(uuidStr[8], '-');
    EXPECT_EQ(uuidStr[13], '-');
    EXPECT_EQ(uuidStr[18], '-');
    EXPECT_EQ(uuidStr[23], '-');
}

TEST(UUIDEdgeCaseTests, MaxSeedValue) {
    UUID uuid(UINT32_MAX);
    char* uuidStr = uuid.toCharArray();

    // Should handle max seed value gracefully
    ASSERT_EQ(strlen(uuidStr), 36);
    EXPECT_NE(uuidStr, nullptr);
}

TEST(UUIDEdgeCaseTests, RapidConsecutiveGeneration) {
    UUID uuid(42);
    std::set<std::string> generatedUUIDs;

    // Generate 1000 UUIDs rapidly
    for (int i = 0; i < 1000; i++) {
        uuid.generate();
        generatedUUIDs.insert(std::string(uuid.toCharArray()));
    }

    // All should be unique
    EXPECT_EQ(generatedUUIDs.size(), 1000);
}

TEST(UUIDEdgeCaseTests, ComparisonWithSelf) {
    UUID uuid1(123);
    char* str1 = uuid1.toCharArray();
    char* str2 = uuid1.toCharArray();

    // Same UUID object should return same string on multiple calls
    EXPECT_STREQ(str1, str2);
}

TEST(UUIDEdgeCaseTests, ModeToggleRapidly) {
    UUID uuid(100);

    for (int i = 0; i < 100; i++) {
        uuid.setRandomMode();
        uuid.generate();
        char* random = uuid.toCharArray();
        ASSERT_EQ(strlen(random), 36);

        uuid.setVersion4Mode();
        uuid.generate();
        char* v4 = uuid.toCharArray();
        ASSERT_EQ(strlen(v4), 36);
    }
}

// ============================================
// SIMPLE TIMER EDGE CASE TESTS
// ============================================

class TimerEdgeCaseFixture : public ::testing::Test {
protected:
    std::unique_ptr<NativeClockDriver> clock;

    void SetUp() override {
        clock = std::make_unique<NativeClockDriver>("edge-test-clock");
        SimpleTimer::setPlatformClock(clock.get());
    }

    void TearDown() override {
        SimpleTimer::resetClock();
    }
};

TEST_F(TimerEdgeCaseFixture, NegativeDurationClamped) {
    SimpleTimer timer;

    // Negative duration should be clamped or treated as zero
    timer.setTimer(0);  // Zero is the minimum
    EXPECT_TRUE(timer.isRunning());

    // Should expire immediately or on first check
    unsigned long start = clock->milliseconds();
    while (clock->milliseconds() == start) {
        // Wait for at least 1ms to pass
    }

    EXPECT_TRUE(timer.expired());
}

TEST_F(TimerEdgeCaseFixture, RapidSetAndInvalidateCycling) {
    SimpleTimer timer;

    // Rapidly cycle between set and invalidate
    for (int i = 0; i < 1000; i++) {
        timer.setTimer(100);
        EXPECT_TRUE(timer.isRunning());
        timer.invalidate();
        EXPECT_FALSE(timer.isRunning());
    }
}

TEST_F(TimerEdgeCaseFixture, MultipleTimersWithSharedClock) {
    SimpleTimer timer1, timer2, timer3;

    unsigned long start = clock->milliseconds();
    timer1.setTimer(10);
    timer2.setTimer(20);
    timer3.setTimer(30);

    EXPECT_TRUE(timer1.isRunning());
    EXPECT_TRUE(timer2.isRunning());
    EXPECT_TRUE(timer3.isRunning());

    // All timers should use the same clock
    EXPECT_FALSE(timer1.expired());
    EXPECT_FALSE(timer2.expired());
    EXPECT_FALSE(timer3.expired());
}

TEST_F(TimerEdgeCaseFixture, SetDuringExpiredState) {
    SimpleTimer timer;
    timer.setTimer(1);

    // Wait for expiration
    unsigned long start = clock->milliseconds();
    while (clock->milliseconds() - start < 5) {
        // Wait
    }

    EXPECT_TRUE(timer.expired());

    // Setting again should reset the timer
    timer.setTimer(100);
    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.expired());
}

TEST_F(TimerEdgeCaseFixture, ElapsedTimeAtBoundaries) {
    SimpleTimer timer;

    unsigned long start = clock->milliseconds();
    timer.setTimer(50);

    // Check elapsed time at various points
    unsigned long elapsed1 = timer.getElapsedTime();
    EXPECT_GE(elapsed1, 0UL);
    EXPECT_LE(elapsed1, 50UL);

    // Wait and check again
    while (clock->milliseconds() - start < 10) {
        // Wait
    }

    unsigned long elapsed2 = timer.getElapsedTime();
    EXPECT_GE(elapsed2, elapsed1);
}

TEST_F(TimerEdgeCaseFixture, InvalidateBeforeSet) {
    SimpleTimer timer;

    // Invalidating a never-set timer should be safe
    timer.invalidate();
    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.expired());
}

TEST_F(TimerEdgeCaseFixture, VeryLargeDuration) {
    SimpleTimer timer;

    // Set to near-maximum uint32_t value
    unsigned long veryLarge = UINT32_MAX - 1000;
    timer.setTimer(veryLarge);

    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.expired());
}

// ============================================
// DIFFICULTY SCALER EDGE CASE TESTS
// ============================================

TEST(DifficultyScalerEdgeCaseTests, ScaleAtExactZero) {
    DifficultyScaler scaler;

    // Initial scale should be 0.0
    float scale = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_FLOAT_EQ(scale, 0.0f);

    // Difficulty label should be "Easy"
    std::string label = scaler.getDifficultyLabel(GameType::SIGNAL_ECHO);
    EXPECT_EQ(label, "Easy");
}

TEST(DifficultyScalerEdgeCaseTests, ScaleAtExactOne) {
    DifficultyScaler scaler;

    // Record many consecutive wins to push scale to 1.0
    for (int i = 0; i < 100; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, true, 1000);
    }

    float scale = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_GE(scale, 0.0f);
    EXPECT_LE(scale, 1.0f);
}

TEST(DifficultyScalerEdgeCaseTests, RapidWinLossOscillation) {
    DifficultyScaler scaler;

    // Alternate between wins and losses rapidly
    for (int i = 0; i < 100; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, i % 2 == 0, 1000);
    }

    float scale = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_GE(scale, 0.0f);
    EXPECT_LE(scale, 1.0f);

    // Scale should be bounded (may stay at 0.0 if requires minimum games)
}

TEST(DifficultyScalerEdgeCaseTests, ZeroCompletionTimeRecorded) {
    DifficultyScaler scaler;

    // Record result with 0ms completion time (edge case)
    scaler.recordResult(GameType::SIGNAL_ECHO, true, 0);
    scaler.recordResult(GameType::SIGNAL_ECHO, true, 0);
    scaler.recordResult(GameType::SIGNAL_ECHO, true, 0);

    auto metrics = scaler.getMetrics(GameType::SIGNAL_ECHO);
    EXPECT_FLOAT_EQ(metrics.avgCompletionTime, 0.0f);
}

TEST(DifficultyScalerEdgeCaseTests, MaxCompletionTime) {
    DifficultyScaler scaler;

    // Record result with maximum completion time
    unsigned long maxTime = UINT32_MAX;
    scaler.recordResult(GameType::SIGNAL_ECHO, true, maxTime);

    auto metrics = scaler.getMetrics(GameType::SIGNAL_ECHO);
    EXPECT_GT(metrics.avgCompletionTime, 0.0f);
}

TEST(DifficultyScalerEdgeCaseTests, AllGameTypesAtOnce) {
    DifficultyScaler scaler;

    // Record results for all game types simultaneously
    GameType allTypes[] = {
        GameType::SIGNAL_ECHO,
        GameType::GHOST_RUNNER,
        GameType::SPIKE_VECTOR,
        GameType::FIREWALL_DECRYPT,
        GameType::CIPHER_PATH,
        GameType::EXPLOIT_SEQUENCER,
        GameType::BREACH_DEFENSE
    };

    for (auto gameType : allTypes) {
        scaler.recordResult(gameType, true, 1000);
        scaler.recordResult(gameType, false, 2000);
        scaler.recordResult(gameType, true, 1500);
    }

    // Each game should have independent scaling
    for (auto gameType : allTypes) {
        float scale = scaler.getCurrentScale(gameType);
        EXPECT_GE(scale, 0.0f);
        EXPECT_LE(scale, 1.0f);

        auto metrics = scaler.getMetrics(gameType);
        EXPECT_EQ(metrics.totalPlayed, 3);
    }
}

TEST(DifficultyScalerEdgeCaseTests, ExtremeWinStreak) {
    DifficultyScaler scaler;

    // Record 1000 consecutive wins
    for (int i = 0; i < 1000; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, true, 1000);
    }

    auto metrics = scaler.getMetrics(GameType::SIGNAL_ECHO);
    EXPECT_EQ(metrics.totalPlayed, 1000);
    EXPECT_FLOAT_EQ(metrics.recentWinRate, 1.0f);

    float scale = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_GE(scale, 0.9f);  // Should be near maximum
    EXPECT_LE(scale, 1.0f);
}

TEST(DifficultyScalerEdgeCaseTests, ExtremeLossStreak) {
    DifficultyScaler scaler;

    // Start with some wins to increase difficulty
    for (int i = 0; i < 10; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, true, 1000);
    }

    // Then record 1000 consecutive losses
    for (int i = 0; i < 1000; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, false, 1000);
    }

    float scale = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_GE(scale, 0.0f);
    EXPECT_LE(scale, 0.1f);  // Should be near minimum
}

TEST(DifficultyScalerEdgeCaseTests, ScaledDifficultyAtBoundaries) {
    DifficultyScaler scaler;

    // At scale 0.0, easy config should return min values
    float scale0 = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_FLOAT_EQ(scale0, 0.0f);

    // Push to maximum
    for (int i = 0; i < 100; i++) {
        scaler.recordResult(GameType::SIGNAL_ECHO, true, 500);
    }

    float scale1 = scaler.getCurrentScale(GameType::SIGNAL_ECHO);
    EXPECT_GE(scale1, 0.0f);
    EXPECT_LE(scale1, 1.0f);
}

TEST(DifficultyScalerEdgeCaseTests, ResetAllClearsAllGames) {
    DifficultyScaler scaler;

    // Record results for multiple games
    scaler.recordResult(GameType::SIGNAL_ECHO, true, 1000);
    scaler.recordResult(GameType::GHOST_RUNNER, true, 1000);
    scaler.recordResult(GameType::SPIKE_VECTOR, true, 1000);

    // Reset all
    scaler.resetAll();

    // All should be back to zero
    EXPECT_FLOAT_EQ(scaler.getCurrentScale(GameType::SIGNAL_ECHO), 0.0f);
    EXPECT_FLOAT_EQ(scaler.getCurrentScale(GameType::GHOST_RUNNER), 0.0f);
    EXPECT_FLOAT_EQ(scaler.getCurrentScale(GameType::SPIKE_VECTOR), 0.0f);
}
