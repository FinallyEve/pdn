#pragma once

#ifdef NATIVE_BUILD

#include <string>
#include <vector>
#include "cli/commands/command-result.hpp"

namespace cli {

/**
 * Help command handlers.
 */
class HelpCommands {
public:
    static CommandResult cmdHelp(const std::vector<std::string>& /*tokens*/) {
        CommandResult result;
        result.message = "Keys: LEFT/RIGHT=select, UP/DOWN=buttons | Cmds: help, quit, list, select, add, b/l, b2/l2, cable, peer, display, mirror, captions, reboot, role, games, stats, progress, colors, difficulty, demo, duel, rematch";
        return result;
    }

    static CommandResult cmdHelp2(const std::vector<std::string>& /*tokens*/) {
        CommandResult result;
        result.message = "add [hunter|bounty|npc <game>|challenge <game>] - add device | cable <a> <b> - connect | peer <src> <dst> <type> - send packet | reboot [dev] - restart device | games - list games | stats [dev] [summary|detailed|streaks|reset] - FDN game statistics | progress [dev] - Konami grid | colors [dev] - color profiles | difficulty [dev|reset] - show/reset auto-scaling | demo [game] [easy|hard] - play demo mode | duel [history|record|series] - duel tracking | rematch - rematch last opponent";
        return result;
    }

    static CommandResult cmdQuit(const std::vector<std::string>& /*tokens*/) {
        CommandResult result;
        result.shouldQuit = true;
        result.message = "Exiting...";
        return result;
    }
};

} // namespace cli

#endif // NATIVE_BUILD
