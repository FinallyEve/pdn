# PDN Demo Scripts

Pre-scripted demonstrations for the hackathon presentation using the CLI simulator's `--script` mode.

## Running Demos

First, build the CLI simulator:
```bash
pio run -e native_cli
```

Then run any demo script:
```bash
.pio/build/native_cli/program 2 --script demos/<demo-file>.demo
```

Or using the development helper:
```bash
./dev.sh sim 2 --script demos/<demo-file>.demo
```

## Available Demos

### 1. fdn-quickstart.demo (~30-45 seconds)
**Quick introduction to FDN encounters**

Demonstrates:
- Player device initialization
- NPC device creation (Signal Echo minigame)
- Serial cable connection (Player-to-NPC)
- FDN handshake protocol
- Simple minigame interaction
- Konami button unlock system

**Use case:** Quick demo for first-time viewers, shows the core gameplay loop.

```bash
.pio/build/native_cli/program 2 --script demos/fdn-quickstart.demo
```

### 2. duel.demo (~20-30 seconds)
**Player vs Player quickdraw duel**

Demonstrates:
- Two player devices (Hunter vs Bounty roles)
- ESP-NOW peer-to-peer communication
- Duel request/accept flow
- Countdown timer
- Draw mechanic and winner determination

**Use case:** Show the competitive PvP aspect of PDN.

```bash
.pio/build/native_cli/program 2 --script demos/duel.demo
```

### 3. all-games-showcase.demo (~3-4 minutes)
**Quick tour of all 7 minigames**

Demonstrates:
- NPC creation for each minigame type
- Connection and disconnection flow
- Variety of gameplay mechanics
- Different input patterns per game

**Use case:** Show the breadth of minigame content.

**Games shown:**
1. Signal Echo - Pattern memory/echo
2. Ghost Runner - Timing challenge
3. Spike Vector - Rhythm-based movement
4. Firewall Decrypt - Pattern recognition
5. Cipher Path - Navigation sequences
6. Exploit Sequencer - Sequence memorization
7. Breach Defense - Defensive timing

```bash
.pio/build/native_cli/program 2 --script demos/all-games-showcase.demo
```

### 4. full-progression.demo (~5 minutes)
**Complete Konami button collection journey**

Demonstrates:
- Sequential encounters with all 7 minigame NPCs
- Each game awards a specific Konami button
- Full progression tracking (0x00 → 0x7F)
- Konami puzzle unlock condition (all 7 buttons)

**Use case:** Show the full progression system and endgame unlock.

**Button rewards:**
- Ghost Runner → START (bit 6)
- Spike Vector → DOWN (bit 1)
- Firewall Decrypt → LEFT (bit 2)
- Cipher Path → RIGHT (bit 3)
- Exploit Sequencer → B (bit 4)
- Breach Defense → A (bit 5)
- Signal Echo → UP (bit 0)

```bash
.pio/build/native_cli/program 2 --script demos/full-progression.demo
```

## Script Syntax Reference

Demo scripts support the following commands:

### Comments
```bash
# This is a comment line (ignored)
```

### Wait Command
```bash
wait <seconds>   # Pause script execution, but devices continue running
```

### Device Management
```bash
add npc <game-name>      # Spawn an NPC/FDN device running the specified game
add hunter               # Add a Hunter player device
add bounty               # Add a Bounty player device
select <device>          # Select device by index or ID for subsequent commands
```

### Cable Connections (Serial)
```bash
cable <devA> <devB>      # Connect two devices via serial cable
cable -d <devA> <devB>   # Disconnect cable between two devices
cable -l                 # List all active cable connections
```

### Button Inputs
```bash
b <device>               # Press primary button (Button 1 / UP) on device
b2 <device>              # Press secondary button (Button 2 / DOWN) on device
l <device>               # Long-press primary button
l2 <device>              # Long-press secondary button
```

### Peer Communication (ESP-NOW)
```bash
peer <src> <dst> <type> [hexdata]  # Send peer packet from src to dst
```

### State Inspection
```bash
state                    # Show current state of all devices
state <device>           # Show state of specific device
konami                   # Show Konami progress of selected device
konami <device>          # Show Konami progress of specific device (if 1 arg, sets value!)
role                     # Show role of selected device
role all                 # Show roles of all devices
```

### Valid Game Names
For use with `add npc <game-name>`:
- `signal-echo` - Pattern echo/memory game
- `ghost-runner` - Timing-based ghost chase
- `spike-vector` - Rhythm and movement
- `firewall-decrypt` - Pattern matching puzzle
- `cipher-path` - Navigation challenge
- `exploit-sequencer` - Sequence memorization
- `breach-defense` - Defensive timing game

## Timing and Behavior Notes

### FDN Handshake Timing
When a player device connects to an NPC device via cable:
1. NPC broadcasts FDN announcement every 500ms
2. Player detects announcement and sends handshake request
3. NPC responds with acknowledgment
4. Player transitions to minigame state

**Total time:** ~2-5 seconds depending on broadcast alignment

**Demo scripts use:** 5-10 second waits to ensure handshake completes

### Game Completion Timing
- Intro animations: ~2 seconds
- Game show/display phase: ~1-3 seconds
- Gameplay: varies by game (5-30 seconds)
- Win/loss display: ~3 seconds
- Return to idle: ~1 second

### Device Indexing
- Initial devices are created sequentially: 0, 1, 2, ...
- Device **indices** (0, 1, 2) are used in commands, not device IDs (0010, 0011, 7012)
- When you `add npc`, it becomes the next index (e.g., if you have devices 0 and 1, NPC becomes 2)
- The `select` command changes which device is the default target for commands

## Troubleshooting

### Script completes but games don't start
**Cause:** FDN handshake didn't complete in time  
**Fix:** Increase wait times after `cable` commands (try 8-10 seconds)

### "Device has no player" when running konami command
**Cause:** Selected device is an NPC (FDN device), not a player  
**Fix:** Use `select 0` before `konami` command to target the player device

### Button inputs don't seem to work
**Cause:** Button presses are being sent to wrong device, or game is not in input-ready state  
**Fix:** 
- Check device index: `b 0` targets device 0, `b 1` targets device 1
- Add more wait time before button presses to let game reach input phase

### Games show "Idle" state instead of playing
**Cause:** FDN game didn't launch after handshake  
**Fix:**
- Increase wait time between `cable` and button presses
- Check that NPC device was cabled to correct player device
- Use `state` command to see current state of all devices

### Build errors
**Cause:** CLI simulator not built or out of date  
**Fix:**
```bash
pio run -e native_cli
```

### Script file not found
**Cause:** Running from wrong directory or incorrect path  
**Fix:** Run from PDN project root: `/home/ubuntu/pdn/`

## Customizing Demos

To create your own demo scripts:

1. **Start simple:** Begin with 1-2 devices and basic commands
2. **Add generous waits:** Err on the side of longer waits (you can optimize later)
3. **Test incrementally:** Run the script after adding each new section
4. **Use state command:** Add `state` commands to see what's happening
5. **Reference tests:** Look at `test/test_cli/*-tests.hpp` for working input sequences

### Example Template
```bash
# My Custom Demo
# Description here

# Initialize
wait 1

# Add devices
add npc <game-name>
wait 1

# Connect
cable 0 2
wait 8

# Interact
b 0
wait 2
b2 0
wait 5

# Cleanup
cable -d 0 2
wait 1
state
```

## Additional Resources

- **CLI Commands Reference:** Type `help` in interactive mode
- **Game Mechanics:** See `test/test_cli/` for detailed game behavior
- **State Machine:** Check `include/game/quickdraw-states.hpp` and `include/game/fdn-states.hpp`
- **Button Mappings:** See `include/device/device-types.hpp` for Konami button rewards

---

**Last Updated:** 2026-02-14  
**PDN Version:** v0.8+ (CLI tool support, NPC spawning, script mode)
