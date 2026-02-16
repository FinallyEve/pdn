#include <gtest/gtest.h>
#include "device-mock.hpp"
#include "state/state-machine.hpp"
#include "state/state.hpp"

// Simple test state for lifecycle testing
class TestState : public State {
public:
    explicit TestState(int id) : State(id), mountCount(0), loopCount(0), dismountCount(0) {}

    void onStateMounted(Device* PDN) override {
        mountCount++;
    }

    void onStateLoop(Device* PDN) override {
        loopCount++;
    }

    void onStateDismounted(Device* PDN) override {
        dismountCount++;
    }

    int mountCount;
    int loopCount;
    int dismountCount;
};

// Simple test state machine
class TestStateMachine : public StateMachine {
public:
    explicit TestStateMachine(int id) : StateMachine(id) {}

    void populateStateMap() override {
        // Simple state machine with one state
        stateMap.push_back(new TestState(1));
    }

    TestState* getTestState() {
        return static_cast<TestState*>(stateMap[0]);
    }
};

class DeviceLifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        device = new MockDevice();
    }

    void TearDown() override {
        delete device;
    }

    MockDevice* device;
};

// Test 1: loadAppConfig with valid app initializes correctly
TEST_F(DeviceLifecycleTest, LoadAppConfigValidApp) {
    AppConfig config;
    StateId appId(100);
    StateMachine* app = new TestStateMachine(100);

    config[appId] = app;

    device->loadAppConfig(config, appId);

    // App should be mounted
    EXPECT_EQ(device->getActiveAppId(), appId);
    EXPECT_NE(device->getActiveApp(), nullptr);
    EXPECT_TRUE(app->hasLaunched());
}

// Test 2: loadAppConfig with missing app ID logs error
TEST_F(DeviceLifecycleTest, LoadAppConfigMissingApp) {
    AppConfig config;
    StateId appId(100);
    StateId missingId(999);
    StateMachine* app = new TestStateMachine(100);

    config[appId] = app;

    // Try to load with a missing app ID - should not crash
    device->loadAppConfig(config, missingId);

    EXPECT_EQ(device->getActiveAppId(), missingId);
    EXPECT_EQ(device->getActiveApp(), nullptr);
}

// Test 3: setActiveApp transitions between apps
TEST_F(DeviceLifecycleTest, SetActiveAppTransitions) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);
    EXPECT_EQ(device->getActiveAppId(), app1Id);
    EXPECT_TRUE(app1->hasLaunched());

    // Switch to app2
    device->setActiveApp(app2Id);
    EXPECT_EQ(device->getActiveAppId(), app2Id);
    EXPECT_TRUE(app2->hasLaunched());
    EXPECT_TRUE(app1->isPaused());
}

// Test 4: returnToPreviousApp returns to previous app
TEST_F(DeviceLifecycleTest, ReturnToPreviousApp) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);
    device->setActiveApp(app2Id);

    // Return to previous app
    device->returnToPreviousApp();
    EXPECT_EQ(device->getActiveAppId(), app1Id);
}

// Test 5: getApp returns correct app or nullptr
TEST_F(DeviceLifecycleTest, GetAppReturnsCorrectApp) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);
    StateId missingId(999);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);

    // Get existing apps
    EXPECT_EQ(device->getApp(app1Id), app1);
    EXPECT_EQ(device->getApp(app2Id), app2);

    // Get missing app
    EXPECT_EQ(device->getApp(missingId), nullptr);
}

// Test 6: getActiveApp returns current app
TEST_F(DeviceLifecycleTest, GetActiveAppReturnsCurrent) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);
    EXPECT_EQ(device->getActiveApp(), app1);

    device->setActiveApp(app2Id);
    EXPECT_EQ(device->getActiveApp(), app2);
}

// Test 7: Device loop calls active app's onStateLoop
TEST_F(DeviceLifecycleTest, LoopCallsActiveApp) {
    AppConfig config;
    StateId appId(100);
    TestStateMachine* app = new TestStateMachine(100);

    config[appId] = app;
    device->loadAppConfig(config, appId);

    TestState* testState = app->getTestState();
    int initialLoopCount = testState->loopCount;

    // Call device loop
    device->loop();

    // State's loop count should have incremented
    EXPECT_GT(testState->loopCount, initialLoopCount);
}

// Test 8: setActiveApp with invalid ID logs error
TEST_F(DeviceLifecycleTest, SetActiveAppInvalidId) {
    AppConfig config;
    StateId appId(100);
    StateId invalidId(999);

    TestStateMachine* app = new TestStateMachine(100);
    config[appId] = app;

    device->loadAppConfig(config, appId);

    // Try to set invalid app - should not crash
    device->setActiveApp(invalidId);

    // Should still be on the original app
    EXPECT_EQ(device->getActiveAppId(), appId);
}

// Test 9: Apps are properly paused and resumed
TEST_F(DeviceLifecycleTest, AppPauseResumeLifecycle) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);
    EXPECT_FALSE(app1->isPaused());

    // Switch to app2 - app1 should be paused
    device->setActiveApp(app2Id);
    EXPECT_TRUE(app1->isPaused());
    EXPECT_FALSE(app2->isPaused());

    // Return to app1 - it should be resumed
    device->returnToPreviousApp();
    EXPECT_FALSE(app1->isPaused());
}

// Test 10: Destructor cleanup order is safe
TEST_F(DeviceLifecycleTest, DestructorCleanupSafe) {
    AppConfig config;
    StateId appId(100);
    TestStateMachine* app = new TestStateMachine(100);

    config[appId] = app;

    MockDevice* tempDevice = new MockDevice();
    tempDevice->loadAppConfig(config, appId);

    // Destructor should safely clean up - should not crash
    delete tempDevice;

    // If we got here, the test passed
    SUCCEED();
}

// Test 11: setActiveApp on already paused app resumes it
TEST_F(DeviceLifecycleTest, SetActiveAppResumesIfPaused) {
    AppConfig config;
    StateId app1Id(100);
    StateId app2Id(200);

    TestStateMachine* app1 = new TestStateMachine(100);
    TestStateMachine* app2 = new TestStateMachine(200);

    config[app1Id] = app1;
    config[app2Id] = app2;

    device->loadAppConfig(config, app1Id);
    device->setActiveApp(app2Id);

    // app1 is now paused
    EXPECT_TRUE(app1->isPaused());

    // Switch back to app1 - should resume it
    device->setActiveApp(app1Id);
    EXPECT_FALSE(app1->isPaused());
}
