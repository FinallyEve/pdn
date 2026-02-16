#!/usr/bin/env bash
# ============================================================
# PDN FDN Minigame Play Aliases
# Source this file:  source demos/play/pdn-play.sh
# ============================================================
#
# CONTROLS (in the TUI):
#   LEFT/RIGHT  = select device
#   UP arrow    = primary button (main action)
#   DOWN arrow  = secondary button (alt action)
#   Type text   = CLI commands (bottom bar)
#
# EASY MODE:  Launches immediately, game starts on its own.
# HARD MODE:  Launches the TUI. You type 2 commands, then play.
#
# ============================================================

# Auto-detect PDN project root
if [[ -f ".pio/build/native_cli/program" ]]; then
    PDN_SIM=".pio/build/native_cli/program"
elif [[ -n "$PDN_DIR" && -f "$PDN_DIR/.pio/build/native_cli/program" ]]; then
    PDN_SIM="$PDN_DIR/.pio/build/native_cli/program"
elif [[ -f "$HOME/pdn/.pio/build/native_cli/program" ]]; then
    PDN_SIM="$HOME/pdn/.pio/build/native_cli/program"
else
    echo "WARNING: PDN simulator binary not found. Run 'pio run -e native_cli' first."
    echo "Set PDN_DIR to your pdn project root, or cd into it before sourcing."
    PDN_SIM="echo 'PDN simulator not found. Build with: pio run -e native_cli' #"
fi

# ============================================================
# EASY MODE — auto-cable, game starts immediately
# Just press buttons when the game loads!
# ============================================================

# Ghost Runner: Press UP when ghost enters the highlighted zone
#   4 rounds, 3 misses allowed. Ghost moves at 50ms/step.
alias ghost-runner-easy='$PDN_SIM 1 --npc ghost-runner --auto-cable'

# Spike Vector: UP=move cursor up, DOWN=move cursor down
#   Position cursor to match gap in advancing wall.
#   5 waves, 3 hits allowed across 5 lanes.
alias spike-vector-easy='$PDN_SIM 1 --npc spike-vector --auto-cable'

# Firewall Decrypt: UP=scroll candidates, DOWN=confirm selection
#   Match the target address from a list of 5 candidates.
#   3 rounds, one wrong answer = lose.
alias firewall-decrypt-easy='$PDN_SIM 1 --npc firewall-decrypt --auto-cable'

# Cipher Path: UP=guess UP direction, DOWN=guess DOWN direction
#   Navigate 6-cell path. Correct guess advances, wrong wastes a move.
#   2 rounds, 12-move budget per round.
alias cipher-path-easy='$PDN_SIM 1 --npc cipher-path --auto-cable'

# Exploit Sequencer: Press UP when scrolling symbol reaches the marker
#   Timing-based QTE. Wide window (+/-15 units).
#   2 sequences x 2 exploits, 3 fails allowed.
alias exploit-sequencer-easy='$PDN_SIM 1 --npc exploit-sequencer --auto-cable'

# Breach Defense: UP=move shield up, DOWN=move shield down
#   Block threats advancing in 3 lanes.
#   6 threats, 3 breaches allowed.
alias breach-defense-easy='$PDN_SIM 1 --npc breach-defense --auto-cable'

# Signal Echo: Watch the sequence, then repeat it
#   UP=UP signal, DOWN=DOWN signal
#   4 signals x 4 sequences, 3 mistakes allowed.
alias signal-echo-easy='$PDN_SIM 1 --npc signal-echo --auto-cable'


# ============================================================
# HARD MODE — requires 2 setup commands after TUI loads
#
# When the TUI appears, type these commands:
#   1. konami 0 <VALUE>    (pre-sets button to force re-encounter)
#   2. cable 0 1           (connects to NPC, triggers re-encounter menu)
#   3. Press UP to confirm HARD (cursor starts on HARD)
#   4. Game launches in hard mode!
#
# The konami bitmask value is unique per game (sets the specific
# button that game awards, tricking the system into re-encounter).
# ============================================================

_pdn_hard_launch() {
    local game=$1
    local konami_val=$2
    local game_display=$3
    local instructions=$4

    echo ""
    echo "=== $game_display (HARD MODE) ==="
    echo ""
    echo "When the TUI loads, type these 2 commands:"
    echo "  1. konami 0 $konami_val"
    echo "  2. cable 0 1"
    echo ""
    echo "Then at the re-encounter menu:"
    echo "  3. Press UP arrow to confirm HARD"
    echo ""
    echo "Gameplay: $instructions"
    echo ""
    echo "Starting in 2 seconds..."
    sleep 2
    $PDN_SIM 1 --npc "$game"
}

# Ghost Runner HARD: 6 rounds, zone 42-58 (narrow!), 1 miss allowed
#   Ghost moves at 30ms/step (fast). Tight timing required.
alias ghost-runner-hard='_pdn_hard_launch ghost-runner 64 "GHOST RUNNER" "Press UP when ghost is in the narrow zone. 6 rounds, 1 miss allowed."'

# Spike Vector HARD: 8 waves, 7 lanes, 1 hit allowed, 20ms/step wall speed
alias spike-vector-hard='_pdn_hard_launch spike-vector 2 "SPIKE VECTOR" "UP/DOWN to dodge. 8 waves, 7 lanes, 1 hit allowed. Wall moves 2x faster."'

# Firewall Decrypt HARD: 4 rounds, 10 candidates, subtle decoys, 15s time limit
alias firewall-decrypt-hard='_pdn_hard_launch firewall-decrypt 4 "FIREWALL DECRYPT" "UP=scroll, DOWN=confirm. 10 similar candidates, 15s time limit per round."'

# Cipher Path HARD: 4 rounds, 10-cell grid, 14-move budget (tight!)
alias cipher-path-hard='_pdn_hard_launch cipher-path 8 "CIPHER PATH" "UP/DOWN to guess direction. 10-cell path, only 14 moves. 4 rounds."'

# Exploit Sequencer HARD: 4 seq x 4 exploits, window +/-6, 25ms/step, 1 fail allowed
alias exploit-sequencer-hard='_pdn_hard_launch exploit-sequencer 16 "EXPLOIT SEQUENCER" "Press UP at exact moment. Tiny window (+/-6), 1 fail allowed. 16 total exploits."'

# Breach Defense HARD: 12 threats, 5 lanes, 1 breach allowed, 20ms/step
alias breach-defense-hard='_pdn_hard_launch breach-defense 32 "BREACH DEFENSE" "UP/DOWN to position shield. 12 threats, 5 lanes, 1 breach allowed. 2x speed."'

# Signal Echo HARD: 8 signals x 4 sequences, 400ms display, 1 mistake allowed
alias signal-echo-hard='_pdn_hard_launch signal-echo 1 "SIGNAL ECHO" "Memorize 8-signal sequence (fast!), then repeat. 1 mistake allowed."'


# ============================================================
# KONAMI METAGAME — the 8th FDN
#
# Requires all 7 buttons. When the TUI loads:
#   1. konami 0 <PROGRESS>   (set how many buttons unlocked)
#   2. cable 0 1             (connect to Konami FDN)
#
# The metagame only activates when ALL 7 buttons are collected
# (konami 0 127). With fewer buttons, it plays a regular game.
#
# CODE ENTRY: UP=cycle through unlocked buttons, DOWN=confirm
# The Konami Code: UP UP DOWN DOWN LEFT RIGHT LEFT RIGHT B A B A START
# (13 inputs total)
# ============================================================

_pdn_konami_launch() {
    local progress=${1:-127}
    local buttons_set=$(python3 -c "print(bin($progress & 0x7F).count('1'))" 2>/dev/null || echo "?")

    echo ""
    echo "=== KONAMI METAGAME ==="
    echo ""
    echo "Konami progress will be set to: $progress ($buttons_set/7 buttons)"
    echo ""
    if [[ $progress -ge 127 ]]; then
        echo "All 7 buttons unlocked! You'll enter the CODE ENTRY puzzle."
        echo ""
        echo "When the TUI loads, type these 2 commands:"
        echo "  1. konami 0 127"
        echo "  2. cable 0 1"
        echo ""
        echo "CODE ENTRY controls:"
        echo "  UP arrow   = cycle through unlocked buttons"
        echo "  DOWN arrow = confirm selected button"
        echo ""
        echo "The Konami Code (13 inputs):"
        echo "  UP UP DOWN DOWN LEFT RIGHT LEFT RIGHT B A B A START"
    else
        echo "With <7 buttons, connecting to any NPC plays a regular game."
        echo "The Konami FDN only activates at 127 (all 7)."
        echo ""
        echo "When the TUI loads:"
        echo "  1. konami 0 $progress"
        echo "  2. cable 0 1"
    fi
    echo ""
    echo "Starting in 2 seconds..."
    sleep 2
    $PDN_SIM 1 --npc konami-code
}

# Default: all 7 buttons, ready for code entry
alias konami-metagame='_pdn_konami_launch 127'

# With specific button count (pass bitmask as argument)
# Usage: konami-metagame-custom <bitmask>
#   1=UP only, 3=UP+DOWN, 7=UP+DOWN+LEFT, 15=+RIGHT, 31=+B, 63=+A, 127=all
alias konami-metagame-custom='_pdn_konami_launch'


# ============================================================
# QUICK REFERENCE
# ============================================================

pdn-games() {
    echo ""
    echo "=== PDN FDN MINIGAME QUICK REFERENCE ==="
    echo ""
    echo "EASY MODE (auto-starts):"
    echo "  ghost-runner-easy       Timing: press UP in the zone"
    echo "  spike-vector-easy       Navigation: UP/DOWN to dodge walls"
    echo "  firewall-decrypt-easy   Selection: UP=scroll, DOWN=confirm"
    echo "  cipher-path-easy        Direction: UP/DOWN to guess path"
    echo "  exploit-sequencer-easy  Timing: press UP at the marker"
    echo "  breach-defense-easy     Navigation: UP/DOWN to block threats"
    echo "  signal-echo-easy        Memory: watch sequence, then repeat"
    echo ""
    echo "HARD MODE (type 2 setup commands, then play):"
    echo "  ghost-runner-hard       6 rounds, narrow zone, 1 miss"
    echo "  spike-vector-hard       8 waves, 7 lanes, 1 hit"
    echo "  firewall-decrypt-hard   10 candidates, 15s limit, subtle decoys"
    echo "  cipher-path-hard        10-cell path, 14 moves, 4 rounds"
    echo "  exploit-sequencer-hard  Tiny window, 16 exploits, 1 fail"
    echo "  breach-defense-hard     12 threats, 5 lanes, 1 breach"
    echo "  signal-echo-hard        8-signal sequence, 1 mistake"
    echo ""
    echo "METAGAME:"
    echo "  konami-metagame         Code entry (all 7 buttons pre-set)"
    echo "  konami-metagame-custom <N>  Custom progress bitmask"
    echo ""
    echo "TUI CONTROLS:  UP=primary  DOWN=secondary  LEFT/RIGHT=select device"
    echo ""
    echo "KONAMI BITMASK:"
    echo "  1=UP(SignalEcho) 2=DOWN(SpikeVector) 4=LEFT(FirewallDecrypt)"
    echo "  8=RIGHT(CipherPath) 16=B(ExploitSeq) 32=A(BreachDefense)"
    echo "  64=START(GhostRunner) | 127=all | 255=all+hardmode"
    echo ""
}

echo "PDN play aliases loaded! Type 'pdn-games' for quick reference."
