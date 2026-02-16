#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include "cli/cli-device.hpp"
#include "cli/commands/command-result.hpp"
#include "device/drivers/native/native-button-driver.hpp"

namespace cli {

/**
 * Button simulation command handlers.
 */
class ButtonCommands {
public:
    static CommandResult cmdButton1Click(const std::vector<std::string>& tokens,
                                         std::vector<DeviceInstance>& devices,
                                         int selectedDevice,
                                         int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int)) {
        CommandResult result;
        int targetDevice = selectedDevice;
        if (tokens.size() >= 2) {
            targetDevice = findDeviceFunc(tokens[1], devices, selectedDevice);
        }
        if (targetDevice >= 0 && targetDevice < static_cast<int>(devices.size())) {
            devices[targetDevice].primaryButtonDriver->execCallback(ButtonInteraction::CLICK);
            result.message = "Button1 click on " + devices[targetDevice].deviceId;
        } else {
            result.message = "Invalid device";
        }
        return result;
    }

    static CommandResult cmdButton1Long(const std::vector<std::string>& tokens,
                                        std::vector<DeviceInstance>& devices,
                                        int selectedDevice,
                                        int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int)) {
        CommandResult result;
        int targetDevice = selectedDevice;
        if (tokens.size() >= 2) {
            targetDevice = findDeviceFunc(tokens[1], devices, selectedDevice);
        }
        if (targetDevice >= 0 && targetDevice < static_cast<int>(devices.size())) {
            devices[targetDevice].primaryButtonDriver->execCallback(ButtonInteraction::LONG_PRESS);
            result.message = "Button1 long press on " + devices[targetDevice].deviceId;
        } else {
            result.message = "Invalid device";
        }
        return result;
    }

    static CommandResult cmdButton2Click(const std::vector<std::string>& tokens,
                                         std::vector<DeviceInstance>& devices,
                                         int selectedDevice,
                                         int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int)) {
        CommandResult result;
        int targetDevice = selectedDevice;
        if (tokens.size() >= 2) {
            targetDevice = findDeviceFunc(tokens[1], devices, selectedDevice);
        }
        if (targetDevice >= 0 && targetDevice < static_cast<int>(devices.size())) {
            devices[targetDevice].secondaryButtonDriver->execCallback(ButtonInteraction::CLICK);
            result.message = "Button2 click on " + devices[targetDevice].deviceId;
        } else {
            result.message = "Invalid device";
        }
        return result;
    }

    static CommandResult cmdButton2Long(const std::vector<std::string>& tokens,
                                        std::vector<DeviceInstance>& devices,
                                        int selectedDevice,
                                        int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int)) {
        CommandResult result;
        int targetDevice = selectedDevice;
        if (tokens.size() >= 2) {
            targetDevice = findDeviceFunc(tokens[1], devices, selectedDevice);
        }
        if (targetDevice >= 0 && targetDevice < static_cast<int>(devices.size())) {
            devices[targetDevice].secondaryButtonDriver->execCallback(ButtonInteraction::LONG_PRESS);
            result.message = "Button2 long press on " + devices[targetDevice].deviceId;
        } else {
            result.message = "Invalid device";
        }
        return result;
    }
};

} // namespace cli

#endif // NATIVE_BUILD
