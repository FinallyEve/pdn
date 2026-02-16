#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include "cli/cli-device.hpp"
#include "cli/commands/command-result.hpp"
#include "game/progress-manager.hpp"
#include "game/difficulty-scaler.hpp"
#include "game/quickdraw.hpp"

namespace cli {

/**
 * Game-related commands for CLI simulator.
 * Handles FDN game information, Konami progress, color profiles, difficulty scaling, and demo mode.
 */
class GameCommands {
public:
    /**
     * List all available FDN games.
     * Usage: games
     */
    static CommandResult cmdGames(const std::vector<std::string>& tokens);

    /**
     * Show or modify Konami progress for a device.
     * Usage: konami [device] | konami set <device> <value>
     */
    static CommandResult cmdKonami(const std::vector<std::string>& tokens,
                                   std::vector<DeviceInstance>& devices,
                                   int selectedDevice,
                                   int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Display Konami progress grid for a device.
     * Usage: progress [device]
     */
    static CommandResult cmdProgress(const std::vector<std::string>& tokens,
                                     const std::vector<DeviceInstance>& devices,
                                     int selectedDevice,
                                     int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int),
                                     const char* (*getGameDisplayNameFunc)(GameType));

    /**
     * Show color profile status for a device.
     * Usage: colors [device]
     */
    static CommandResult cmdColors(const std::vector<std::string>& tokens,
                                   const std::vector<DeviceInstance>& devices,
                                   int selectedDevice,
                                   int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int),
                                   const char* (*getGameDisplayNameFunc)(GameType));

    /**
     * Show or reset difficulty auto-scaling for a device.
     * Usage: difficulty [device] | difficulty reset [device]
     */
    static CommandResult cmdDifficulty(const std::vector<std::string>& tokens,
                                       const std::vector<DeviceInstance>& devices,
                                       int selectedDevice,
                                       int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Play FDN games in demo mode.
     * Usage: demo <game> [easy|hard] | demo list | demo all [easy|hard]
     */
    static CommandResult cmdDemo(const std::vector<std::string>& tokens,
                                 std::vector<DeviceInstance>& devices,
                                 int& selectedDevice,
                                 bool (*parseGameNameFunc)(const std::string&, GameType&),
                                 const char* (*getGameDisplayNameFunc)(GameType));
};

} // namespace cli

#endif // NATIVE_BUILD
