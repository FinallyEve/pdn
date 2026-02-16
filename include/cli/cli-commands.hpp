#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>

#include "cli/cli-device.hpp"
#include "cli/cli-renderer.hpp"
#include "cli/cli-serial-broker.hpp"
#include "device/drivers/native/native-peer-broker.hpp"
#include "game/quickdraw.hpp"
#include "game/progress-manager.hpp"
#include "game/difficulty-scaler.hpp"

// Command handler modules
#include "cli/commands/command-result.hpp"
#include "cli/commands/help-commands.hpp"
#include "cli/commands/button-commands.hpp"
#include "cli/commands/display-commands.hpp"
#include "cli/commands/cable-commands.hpp"
#include "cli/commands/device-commands.hpp"
#include "cli/commands/network-commands.hpp"
#include "cli/commands/game-commands.hpp"
#include "cli/commands/stats-commands.hpp"
#include "cli/commands/duel-commands.hpp"
#include "cli/commands/debug-commands.hpp"

// External cycling state (defined in cli-main.cpp)
extern bool g_panelCycling;
extern int g_panelCycleInterval;
extern std::chrono::steady_clock::time_point g_panelCycleLastSwitch;

extern bool g_stateCycling;
extern int g_stateCycleDevice;
extern int g_stateCycleInterval;
extern int g_stateCycleStep;
extern std::chrono::steady_clock::time_point g_stateCycleLastSwitch;

namespace cli {

/**
 * Command processor for CLI simulator.
 * Parses and executes user commands.
 */
class CommandProcessor {
public:
    CommandProcessor() = default;

    /**
     * Parse and execute a command string.
     */
    CommandResult execute(const std::string& cmd,
                          std::vector<DeviceInstance>& devices,
                          int& selectedDevice,
                          Renderer& renderer) {
        CommandResult result;

        if (cmd.empty()) {
            return result;
        }

        std::vector<std::string> tokens = tokenize(cmd);
        if (tokens.empty()) {
            return result;
        }

        const std::string& command = tokens[0];

        // Dispatch to command handlers
        if (command == "help" || command == "h" || command == "?") {
            return HelpCommands::cmdHelp(tokens);
        }
        if (command == "help2") {
            return HelpCommands::cmdHelp2(tokens);
        }
        if (command == "quit" || command == "q" || command == "exit") {
            return HelpCommands::cmdQuit(tokens);
        }
        if (command == "list" || command == "ls") {
            return DeviceCommands::cmdList(tokens, devices, selectedDevice);
        }
        if (command == "select" || command == "sel") {
            return DeviceCommands::cmdSelect(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "b" || command == "button" || command == "click") {
            return ButtonCommands::cmdButton1Click(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "l" || command == "long" || command == "longpress") {
            return ButtonCommands::cmdButton1Long(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "b2" || command == "button2" || command == "click2") {
            return ButtonCommands::cmdButton2Click(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "l2" || command == "long2" || command == "longpress2") {
            return ButtonCommands::cmdButton2Long(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "state" || command == "st") {
            return DeviceCommands::cmdState(tokens, devices, selectedDevice, findDevice, getStateName);
        }
        if (command == "cable" || command == "connect" || command == "c") {
            return CableCommands::cmdCable(tokens, devices, findDevice);
        }
        if (command == "peer" || command == "packet" || command == "espnow") {
            return NetworkCommands::cmdPeer(tokens, devices, findDevice, parseMacString, parseHexData);
        }
        if (command == "inject") {
            return NetworkCommands::cmdInject(tokens, devices, findDevice, parseMacString, parseHexData);
        }
        if (command == "add" || command == "new") {
            return DeviceCommands::cmdAdd(tokens, devices, selectedDevice);
        }
        if (command == "mirror" || command == "m") {
            return DisplayCommands::cmdMirror(tokens, renderer);
        }
        if (command == "captions" || command == "cap") {
            return DisplayCommands::cmdCaptions(tokens, renderer);
        }
        if (command == "display" || command == "d") {
            return DisplayCommands::cmdDisplay(tokens, renderer);
        }
        if (command == "reboot" || command == "restart") {
            return DeviceCommands::cmdReboot(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "role" || command == "roles") {
            return DeviceCommands::cmdRole(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "konami") {
            return GameCommands::cmdKonami(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "games") {
            return GameCommands::cmdGames(tokens);
        }
        if (command == "stats" || command == "info") {
            return StatsCommands::cmdStats(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "progress" || command == "prog") {
            return GameCommands::cmdProgress(tokens, devices, selectedDevice, findDevice, getGameDisplayName);
        }
        if (command == "colors" || command == "profiles") {
            return GameCommands::cmdColors(tokens, devices, selectedDevice, findDevice, getGameDisplayName);
        }
        if (command == "difficulty" || command == "diff") {
            return GameCommands::cmdDifficulty(tokens, devices, selectedDevice, findDevice);
        }
        if (command == "demo") {
            return GameCommands::cmdDemo(tokens, devices, selectedDevice, parseGameName, getGameDisplayName);
        }
        if (command == "debug") {
            return DebugCommands::cmdDebug(tokens, devices, selectedDevice, renderer, findDevice, getStateName,
                                           g_panelCycling, g_panelCycleInterval, g_panelCycleLastSwitch,
                                           g_stateCycling, g_stateCycleDevice, g_stateCycleInterval,
                                           g_stateCycleStep, g_stateCycleLastSwitch);
        }
        if (command == "duel") {
            return DuelCommands::cmdDuel(tokens, devices, selectedDevice);
        }
        if (command == "rematch" || command == "r") {
            return DuelCommands::cmdRematch(tokens, devices, selectedDevice, findDevice);
        }

        result.message = "Unknown command: " + command + " (try 'help')";
        return result;
    }

private:
    // ==================== UTILITY FUNCTIONS ====================

    /**
     * Tokenize a command string by spaces.
     */
    static std::vector<std::string> tokenize(const std::string& cmd) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : cmd) {
            if (c == ' ') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        return tokens;
    }

    /**
     * Find device by ID or index string.
     */
    static int findDevice(const std::string& target,
                          const std::vector<DeviceInstance>& devices,
                          int defaultDevice) {
        for (size_t i = 0; i < devices.size(); i++) {
            if (devices[i].deviceId == target || std::to_string(i) == target) {
                return static_cast<int>(i);
            }
        }
        return defaultDevice;
    }

    /**
     * Parse a MAC address string like "02:00:00:00:00:01" into bytes.
     */
    static void parseMacString(const std::string& macStr, uint8_t* macOut) {
        unsigned int bytes[6];
        sscanf(macStr.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
               &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]);
        for (int i = 0; i < 6; i++) {
            macOut[i] = static_cast<uint8_t>(bytes[i]);
        }
    }

    /**
     * Parse hex data from tokens starting at the given index.
     */
    static std::vector<uint8_t> parseHexData(const std::vector<std::string>& tokens, size_t startIndex) {
        std::vector<uint8_t> data;
        for (size_t i = startIndex; i < tokens.size(); i++) {
            unsigned int byte;
            if (tokens[i].find("0x") == 0 || tokens[i].find("0X") == 0) {
                byte = std::strtoul(tokens[i].c_str() + 2, nullptr, 16);
            } else {
                byte = std::strtoul(tokens[i].c_str(), nullptr, 16);
            }
            data.push_back(static_cast<uint8_t>(byte));
        }
        return data;
    }
};

} // namespace cli

#endif // NATIVE_BUILD
