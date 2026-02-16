#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include "cli/cli-device.hpp"
#include "cli/commands/command-result.hpp"
#include "game/progress-manager.hpp"

namespace cli {

/**
 * Stats-related commands for CLI simulator.
 * Handles FDN game statistics tracking, summaries, detailed stats, streaks, and resets.
 */
class StatsCommands {
public:
    /**
     * Main stats dispatcher - routes to appropriate subcommand handler.
     * Usage: stats [device] [summary|detailed|streaks|reset]
     */
    static CommandResult cmdStats(const std::vector<std::string>& tokens,
                                  std::vector<DeviceInstance>& devices,
                                  int selectedDevice,
                                  int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));

    /**
     * Display summary statistics table for all FDN games.
     * Shows W/L, win rate, avg/best times, streaks.
     * Usage: stats [device] summary (or just 'stats')
     */
    static CommandResult cmdStatsSummary(DeviceInstance& dev,
                                         int deviceIndex,
                                         const char* (*getGameDisplayNameFunc)(GameType));

    /**
     * Display detailed per-game statistics broken down by difficulty.
     * Shows easy and hard mode stats separately.
     * Usage: stats [device] detailed
     */
    static CommandResult cmdStatsDetailed(DeviceInstance& dev,
                                          int deviceIndex,
                                          const char* (*getGameDisplayNameFunc)(GameType));

    /**
     * Display current and best streaks for all FDN games.
     * Usage: stats [device] streaks
     */
    static CommandResult cmdStatsStreaks(DeviceInstance& dev,
                                         int deviceIndex,
                                         const char* (*getGameDisplayNameFunc)(GameType));

    /**
     * Reset all FDN game statistics for a device.
     * Usage: stats reset [device]
     */
    static CommandResult cmdStatsReset(const std::vector<std::string>& tokens,
                                       std::vector<DeviceInstance>& devices,
                                       int selectedDevice,
                                       int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int));
};

} // namespace cli

#endif // NATIVE_BUILD
