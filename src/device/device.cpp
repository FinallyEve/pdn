#include "device/device.hpp"
#include "state/state-machine.hpp"
#include "device/drivers/logger.hpp"
#include <utility>

#ifdef NATIVE_BUILD
#include "cli/cli-serial-broker.hpp"
#include "device/drivers/native/native-serial-driver.hpp"
#endif

const char* TAG = "Device";

Device::~Device() {
    // shutdownApps() is called here for test mocks and simple Device subclasses.
    // For PDN, shutdownApps() is called first in PDN::~PDN() (while the vtable
    // still has PDN's virtual method implementations). The second call here is
    // a no-op since shutdownApps() clears appConfig after processing.
    shutdownApps();
    driverManager.dismountDrivers();
}

void Device::shutdownApps() {
    for (auto& pair : appConfig) {
        if (pair.second != nullptr) {
            if (pair.second->hasLaunched()) {
                // Call shutdown() to gracefully stop the state machine before destruction.
                // This prevents pure virtual method calls during onStateDismounted().
                pair.second->shutdown(this);
            }
            delete pair.second;
            pair.second = nullptr;
        }
    }
    appConfig.clear();
}

void Device::loadAppConfig(AppConfig config, StateId launchAppId) {
    this->appConfig = std::move(config);
    this->currentAppId = launchAppId;
    if(appConfig.find(currentAppId) == appConfig.end()) {
        LOG_E(TAG, "App %d not found", currentAppId.id);
        return;
    }
    
    appConfig[currentAppId]->onStateMounted(this);
}

void Device::setActiveApp(StateId appId) {
    if(appConfig.find(appId) == appConfig.end()) {
        LOG_E(TAG, "App %d not found", appId.id);
        return;
    }

    previousAppId = currentAppId;
    appConfig[currentAppId]->onStatePaused(this);
    this->currentAppId = appId;
    if(appConfig[appId]->isPaused()) {
        appConfig[appId]->onStateResumed(this, nullptr);
    } else {
        appConfig[appId]->onStateMounted(this);
    }
}

void Device::returnToPreviousApp() {
    setActiveApp(previousAppId);
}

StateMachine* Device::getApp(StateId appId) {
    auto it = appConfig.find(appId);
    return (it != appConfig.end()) ? it->second : nullptr;
}

StateMachine* Device::getActiveApp() {
    auto it = appConfig.find(currentAppId);
    return (it != appConfig.end()) ? it->second : nullptr;
}

void Device::loop() {
    driverManager.execDrivers();
    auto app = appConfig.find(currentAppId);
    if(app != appConfig.end()) {
        app->second->onStateLoop(this);
    }
}

bool DeviceSerial::isSerialConnected() {
#ifdef NATIVE_BUILD
    // CLI simulator: check if cable is connected via SerialCableBroker
    auto* nativeDriver = dynamic_cast<NativeSerialDriver*>(getPrimaryCommsJack());
    if (nativeDriver) {
        int deviceIndex = nativeDriver->getDeviceIndex();
        if (deviceIndex >= 0) {
            int connectedDevice = cli::SerialCableBroker::getInstance().getConnectedDevice(deviceIndex);
            return connectedDevice >= 0;
        }
    }
    return false;  // No device index or not a native driver
#else
    // ESP32 hardware: assume cable is always connected (physical connection)
    return true;
#endif
}
