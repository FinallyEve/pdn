#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include "cli/cli-device.hpp"
#include "cli/commands/command-result.hpp"

namespace cli {

/**
 * Device management commands for CLI simulator.
 * Handles device listing, selection, state inspection, and lifecycle operations.
 */
class DeviceCommands {
public:
    /**
     * List all devices and their connection status.
     * Usage: list
     */
    static CommandResult cmdList(const std::vector<std::string>& tokens,
                                 const std::vector<DeviceInstance>& devices,
                                 int selectedDevice);

    /**
     * Select a device by ID or index.
     * Usage: select <device_id or index>
     */
    static CommandResult cmdSelect(const std::vector<std::string>& tokens,
                                   const std::vector<DeviceInstance>& devices,
                                   int& selectedDevice,
                                   int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Add a new device (player, NPC, or challenge).
     * Usage: add [hunter|bounty|npc <game>|challenge <game>]
     */
    static CommandResult cmdAdd(const std::vector<std::string>& tokens,
                                std::vector<DeviceInstance>& devices,
                                int& selectedDevice);

    /**
     * Show or set device role (Hunter/Bounty).
     * Usage: role [device] | role all
     */
    static CommandResult cmdRole(const std::vector<std::string>& tokens,
                                 const std::vector<DeviceInstance>& devices,
                                 int selectedDevice,
                                 int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Reboot a device (return to FetchUserData state).
     * Usage: reboot [device]
     */
    static CommandResult cmdReboot(const std::vector<std::string>& tokens,
                                   std::vector<DeviceInstance>& devices,
                                   int selectedDevice,
                                   int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Show current state of a device.
     * Usage: state [device]
     */
    static CommandResult cmdState(const std::vector<std::string>& tokens,
                                  std::vector<DeviceInstance>& devices,
                                  int selectedDevice,
                                  int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int),
                                  const char* (*getStateNameFunc)(int, DeviceType, GameType));
};

} // namespace cli

#endif // NATIVE_BUILD
