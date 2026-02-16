#include <gtest/gtest.h>
#include "device/light-manager.hpp"
#include "device/idle-animation.hpp"
#include "device/countdown-animation.hpp"
#include "device/vertical-chase-animation.hpp"
#include "device/drivers/light-interface.hpp"
#include <vector>

// Track light strip calls for testing
struct LightCall {
    LightIdentifier lightSet;
    uint8_t index;
    LEDState::SingleLEDState state;
};

class MockLightStrip : public LightStrip {
public:
    MockLightStrip() : globalBrightness_(255), fps_(0) {}

    void setLight(LightIdentifier lightSet, uint8_t index, LEDState::SingleLEDState color) override {
        calls.push_back({lightSet, index, color});
    }

    void setLightBrightness(LightIdentifier lightSet, uint8_t index, uint8_t brightness) override {
        // Not needed for these tests
    }

    void setGlobalBrightness(uint8_t brightness) override {
        globalBrightness_ = brightness;
    }

    LEDState::SingleLEDState getLight(LightIdentifier lightSet, uint8_t index) override {
        return LEDState::SingleLEDState();
    }

    void fade(LightIdentifier lightSet, uint8_t fadeAmount) override {
        // Not needed for these tests
    }

    void addToLight(LightIdentifier lightSet, uint8_t index, LEDState::SingleLEDState color) override {
        // Not needed for these tests
    }

    void setFPS(uint8_t fps) override {
        fps_ = fps;
    }

    uint8_t getFPS() const override {
        return fps_;
    }

    void clearCalls() {
        calls.clear();
    }

    std::vector<LightCall> calls;
    uint8_t globalBrightness_;
    uint8_t fps_;
};

class LightManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockStrip = new MockLightStrip();
        lightManager = new LightManager(*mockStrip);
    }

    void TearDown() override {
        delete lightManager;
        delete mockStrip;
    }

    MockLightStrip* mockStrip;
    LightManager* lightManager;
};

// Test 1: Animation starts and stops correctly
TEST_F(LightManagerTest, AnimationStartStop) {
    EXPECT_FALSE(lightManager->isAnimating());

    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    lightManager->startAnimation(config);
    EXPECT_TRUE(lightManager->isAnimating());
    EXPECT_EQ(lightManager->getCurrentAnimation(), AnimationType::IDLE);

    lightManager->stopAnimation();
    EXPECT_FALSE(lightManager->isAnimating());
    EXPECT_TRUE(lightManager->isAnimationComplete());
}

// Test 2: Multiple animation types can be started
TEST_F(LightManagerTest, MultipleAnimationTypes) {
    AnimationConfig idleConfig;
    idleConfig.type = AnimationType::IDLE;
    idleConfig.loop = false;
    idleConfig.speed = 16;

    lightManager->startAnimation(idleConfig);
    EXPECT_EQ(lightManager->getCurrentAnimation(), AnimationType::IDLE);

    AnimationConfig countdownConfig;
    countdownConfig.type = AnimationType::COUNTDOWN;
    countdownConfig.loop = false;
    countdownConfig.speed = 16;

    lightManager->startAnimation(countdownConfig);
    EXPECT_EQ(lightManager->getCurrentAnimation(), AnimationType::COUNTDOWN);
}

// Test 3: Animation pause and resume
TEST_F(LightManagerTest, AnimationPauseResume) {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    lightManager->startAnimation(config);
    EXPECT_FALSE(lightManager->isPaused());

    lightManager->pauseAnimation();
    EXPECT_TRUE(lightManager->isPaused());

    lightManager->resumeAnimation();
    EXPECT_FALSE(lightManager->isPaused());
}

// Test 4: Loop processes animation frames
TEST_F(LightManagerTest, LoopProcessesFrames) {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    LEDState initialState;
    initialState.leftLights[0] = LEDState::SingleLEDState(LEDColor(255, 0, 0), 255);
    config.initialState = initialState;

    lightManager->startAnimation(config);
    mockStrip->clearCalls();

    // Call loop to process animation
    lightManager->loop();

    // Should have made calls to set lights (6 grip + 13 display = 19 total)
    EXPECT_EQ(mockStrip->calls.size(), 19);
}

// Test 5: Clear clears all LEDs
TEST_F(LightManagerTest, ClearResetsLights) {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    lightManager->startAnimation(config);
    lightManager->loop();
    mockStrip->clearCalls();

    lightManager->clear();

    // Clear should apply an empty LED state (19 calls)
    EXPECT_EQ(mockStrip->calls.size(), 19);

    // All calls should have zero brightness
    for (const auto& call : mockStrip->calls) {
        EXPECT_EQ(call.state.brightness, 0);
    }
}

// Test 6: Global brightness is set correctly
TEST_F(LightManagerTest, GlobalBrightnessSet) {
    EXPECT_EQ(mockStrip->globalBrightness_, 255);

    lightManager->setGlobalBrightness(128);
    EXPECT_EQ(mockStrip->globalBrightness_, 128);

    lightManager->setGlobalBrightness(0);
    EXPECT_EQ(mockStrip->globalBrightness_, 0);
}

// Test 7: Paused animation doesn't update
TEST_F(LightManagerTest, PausedAnimationNoUpdate) {
    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    lightManager->startAnimation(config);
    lightManager->pauseAnimation();
    mockStrip->clearCalls();

    // Call loop multiple times while paused
    lightManager->loop();
    lightManager->loop();
    lightManager->loop();

    // Should not have made any light updates while paused
    EXPECT_EQ(mockStrip->calls.size(), 0);
}

// Test 8: Starting new animation cleans up old one
TEST_F(LightManagerTest, NewAnimationCleansUpOld) {
    AnimationConfig config1;
    config1.type = AnimationType::IDLE;
    config1.loop = false;
    config1.speed = 16;

    lightManager->startAnimation(config1);
    EXPECT_EQ(lightManager->getCurrentAnimation(), AnimationType::IDLE);

    AnimationConfig config2;
    config2.type = AnimationType::COUNTDOWN;
    config2.loop = false;
    config2.speed = 16;

    // Starting new animation should clean up old one
    lightManager->startAnimation(config2);
    EXPECT_EQ(lightManager->getCurrentAnimation(), AnimationType::COUNTDOWN);
    EXPECT_TRUE(lightManager->isAnimating());
}

// Test 9: isAnimationComplete returns correct state
TEST_F(LightManagerTest, IsAnimationCompleteCorrect) {
    // Initially, no animation is running, so it should be "complete"
    EXPECT_TRUE(lightManager->isAnimationComplete());

    AnimationConfig config;
    config.type = AnimationType::IDLE;
    config.loop = false;
    config.speed = 16;

    lightManager->startAnimation(config);
    EXPECT_FALSE(lightManager->isAnimationComplete());

    lightManager->stopAnimation();
    EXPECT_TRUE(lightManager->isAnimationComplete());
}

// Test 10: Stopping animation with no active animation is safe
TEST_F(LightManagerTest, StopWithNoAnimationSafe) {
    EXPECT_FALSE(lightManager->isAnimating());

    // Should not crash or cause issues
    lightManager->stopAnimation();

    EXPECT_FALSE(lightManager->isAnimating());
    EXPECT_TRUE(lightManager->isAnimationComplete());
}

// Test 11: Pause/Resume with no animation is safe
TEST_F(LightManagerTest, PauseResumeWithNoAnimationSafe) {
    EXPECT_FALSE(lightManager->isAnimating());

    // Should not crash
    lightManager->pauseAnimation();
    lightManager->resumeAnimation();

    EXPECT_FALSE(lightManager->isAnimating());
}
