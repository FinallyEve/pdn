#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include <cstring>
#include "cli/cli-device.hpp"
#include "cli/commands/command-result.hpp"
#include "device/drivers/native/native-peer-broker.hpp"

namespace cli {

/**
 * Network/Peer command handlers.
 */
class NetworkCommands {
public:
    static CommandResult cmdPeer(const std::vector<std::string>& tokens,
                                 const std::vector<DeviceInstance>& devices,
                                 int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int),
                                 void (*parseMacStringFunc)(const std::string&, uint8_t*),
                                 std::vector<uint8_t> (*parseHexDataFunc)(const std::vector<std::string>&, size_t)) {
        CommandResult result;

        if (tokens.size() < 4) {
            result.message = "Usage: peer <src> <dst|broadcast> <type> [hexdata]";
            return result;
        }

        int srcDevice = findDeviceFunc(tokens[1], devices, -1);
        if (srcDevice < 0) {
            result.message = "Invalid source device: " + tokens[1];
            return result;
        }

        // Get destination MAC
        uint8_t dstMac[6];
        bool isBroadcast = (tokens[2] == "broadcast" || tokens[2] == "bc" || tokens[2] == "*");
        int dstDevice = -1;

        if (isBroadcast) {
            std::memcpy(dstMac, NativePeerBroker::getInstance().getBroadcastAddress(), 6);
        } else {
            dstDevice = findDeviceFunc(tokens[2], devices, -1);
            if (dstDevice < 0) {
                result.message = "Invalid destination device: " + tokens[2];
                return result;
            }
            std::string macStr = devices[dstDevice].peerCommsDriver->getMacString();
            parseMacStringFunc(macStr, dstMac);
        }

        // Parse packet type
        int pktTypeInt = std::atoi(tokens[3].c_str());
        PktType packetType = static_cast<PktType>(pktTypeInt);

        // Parse optional hex data
        std::vector<uint8_t> data = parseHexDataFunc(tokens, 4);

        // Get source MAC and send
        std::string srcMacStr = devices[srcDevice].peerCommsDriver->getMacString();
        uint8_t srcMac[6];
        parseMacStringFunc(srcMacStr, srcMac);

        NativePeerBroker::getInstance().sendPacket(
            srcMac, dstMac, packetType,
            data.empty() ? nullptr : data.data(),
            data.size());

        std::string dstStr = isBroadcast ? "broadcast" : devices[dstDevice].deviceId;
        result.message = "Sent packet type " + std::to_string(pktTypeInt) +
                         " from " + devices[srcDevice].deviceId +
                         " to " + dstStr +
                         " (" + std::to_string(data.size()) + " bytes)";
        return result;
    }

    static CommandResult cmdInject(const std::vector<std::string>& tokens,
                                   const std::vector<DeviceInstance>& devices,
                                   int (*findDeviceFunc)(const std::string&, const std::vector<DeviceInstance>&, int),
                                   void (*parseMacStringFunc)(const std::string&, uint8_t*),
                                   std::vector<uint8_t> (*parseHexDataFunc)(const std::vector<std::string>&, size_t)) {
        CommandResult result;

        if (tokens.size() < 3) {
            result.message = "Usage: inject <dst> <type> [hexdata] - inject from external source";
            return result;
        }

        int dstDevice = findDeviceFunc(tokens[1], devices, -1);
        if (dstDevice < 0) {
            result.message = "Invalid destination device: " + tokens[1];
            return result;
        }

        int pktTypeInt = std::atoi(tokens[2].c_str());
        PktType packetType = static_cast<PktType>(pktTypeInt);

        std::vector<uint8_t> data = parseHexDataFunc(tokens, 3);

        // Create a fake external MAC address
        uint8_t externalMac[6] = {0xEE, 0xEE, 0xEE, 0x00, 0x00, 0x01};

        // Get destination MAC
        std::string dstMacStr = devices[dstDevice].peerCommsDriver->getMacString();
        uint8_t dstMac[6];
        parseMacStringFunc(dstMacStr, dstMac);

        NativePeerBroker::getInstance().sendPacket(
            externalMac, dstMac, packetType,
            data.empty() ? nullptr : data.data(),
            data.size());

        result.message = "Injected packet type " + std::to_string(pktTypeInt) +
                         " to " + devices[dstDevice].deviceId +
                         " (" + std::to_string(data.size()) + " bytes)";
        return result;
    }
};

} // namespace cli

#endif // NATIVE_BUILD
