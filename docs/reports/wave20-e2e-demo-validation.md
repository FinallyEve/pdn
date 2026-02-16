# Wave 20 E2E Demo Validation — Bug #207 Fix Verification

**Date:** 2026-02-16
**Agent:** Agent 12 (E2E Demo Tester)
**Branch:** `wave20/12-e2e-demo`
**Target:** Verify Bug #207 fix from PR #295

## Objective

Verify that Bug #207 (cable disconnect during minigame causes player to get stuck) is properly fixed in current main after Wave 19 PR #295 merged.

## Bug #207 Summary

**Problem:** When a player disconnects cable mid-minigame, the player gets stuck in the minigame state forever. This was the root cause of cascade failures in multi-player testing (#213).

**Fix (PR #295):** Added cable disconnect detection in CLI main loop. When cable disconnects during a minigame (app IDs 1-7), call `returnToPreviousApp()` to abort the game and return to Quickdraw Idle.

## Test Results

### 1. Duel Demo (`demos/duel.demo`)

**Status:** ✅ PASS

**Command:**
```bash
.pio/build/native_cli/program 2 --script demos/duel.demo
```

**Result:**
- Script completed successfully
- Both players (Hunter + Bounty) reached Idle state
- No hanging or stuck states

**Output:**
```
Device 0010: Hunter → WelcomeMessage → AwakenSequence → Idle
Device 0011: Bounty → WelcomeMessage → AwakenSequence → Idle
```

### 2. All Games Showcase (`demos/all-games-showcase.demo`)

**Status:** ✅ PASS (with notes)

**Command:**
```bash
.pio/build/native_cli/program 1 --script demos/all-games-showcase.demo
```

**Result:**
- Script completed successfully
- All 7 minigames launched in standalone challenge mode
- Games progressed through intro → gameplay/show states
- Multiple games ran simultaneously (expected for standalone challenge devices)
- No cascade failures observed

**Notable Observations:**
- Challenge devices (app ID for standalone minigames) persist across player reboots
- Games continue independently (e.g., GhostRunnerGameplay, SpikeVectorGameplay running concurrently)
- Player device (0010) successfully transitioned between WelcomeMessage, AwakenSequence, and Idle states

### 3. Full Progression Demo (`demos/full-progression.demo`)

**Status:** ❌ FAIL (unrelated to Bug #207)

**Command:**
```bash
.pio/build/native_cli/program 1 --script demos/full-progression.demo
```

**Result:**
- Script completed but games never started
- Player device stuck in `PlayerRegistration` state throughout entire script
- All 7 NPC devices remained in `NpcIdle` state
- No FDN encounters initiated

**Key Observations:**
```
[0010] PlayerRegistration  ← Player stuck here
[7011] NpcIdle             ← Ghost Runner NPC
[7012] NpcIdle             ← Spike Vector NPC
...
[7017] NpcIdle             ← Signal Echo NPC
```

**Root Cause Analysis:**
- Player is running `PlayerRegistrationApp` (app ID 2), not Quickdraw
- The PlayerRegistration app is separate from the Quickdraw game flow
- Without completing registration, player cannot detect FDN signals or enter FDN encounters
- This is **NOT** related to Bug #207 (cable disconnect) — it's a player initialization issue

### 4. Manual Cable Disconnect Test

**Status:** ⚠️ INCONCLUSIVE (player never reached minigame)

**Test Script:**
```bash
# Add player and NPC
add npc spike-vector
cable 0 1
wait 8.5s

# Check state (should be in minigame)
state

# Disconnect cable mid-game
cable -d 0 1
wait 0.5s

# Verify player returned to Idle (NOT stuck in SpikeVectorGameplay)
state
```

**Result:**
- Player remained in `PlayerRegistration` state
- NPC reached `NpcGameActive` state
- Cable disconnect occurred, but no minigame was active to abort
- Cannot verify Bug #207 fix without player reaching minigame state

## Bug #207 Verification: BLOCKED

**Conclusion:** Cannot fully verify Bug #207 fix due to player initialization issue in `full-progression.demo`. The player never progresses past `PlayerRegistration` app, so FDN minigames never launch.

**However:** The all-games-showcase demo (standalone challenge mode) worked correctly, and the duel demo worked correctly. This suggests:
1. Basic game state machines are functioning
2. Challenge mode (standalone minigames) works
3. Player-vs-player flow works
4. The issue is specific to FDN encounters requiring player registration completion

## Comparison with PR #213 Multi-Player Testing

From PR #213's description, the original cascade failure scenario was:
- Player encounters NPC
- Minigame starts
- Cable disconnects mid-game
- Player gets stuck in minigame state
- Subsequent NPCs cannot be detected

**Current behavior:**
- Player cannot start minigame (stuck in PlayerRegistration)
- No cascade failures observed (because games never start)
- This is a different failure mode than Bug #207

## Recommendations

1. **Investigate PlayerRegistration app:**
   - Why does `full-progression.demo` create players in PlayerRegistration app?
   - Does the demo script need to complete registration flow before FDN encounters?
   - Is there a missing initialization step?

2. **Create simplified cable disconnect test:**
   - Use standalone challenge mode (like all-games-showcase)
   - Manually test cable disconnect during active minigame
   - Verify `returnToPreviousApp()` is called

3. **Review demo script expectations:**
   - Are demo scripts written for an older version of the codebase?
   - Do scripts assume automatic player registration completion?

4. **Check PlayerRegistration lifecycle:**
   - When should `registrationComplete` flag be set?
   - What triggers transition from PlayerRegistration to Quickdraw app?

## Files Analyzed

- `demos/full-progression.demo` — Sequential FDN walkthrough script
- `demos/duel.demo` — Player-vs-player combat demo
- `demos/all-games-showcase.demo` — Standalone minigame showcase
- `/tmp/cable-disconnect-test.demo` — Custom cable disconnect test
- `include/apps/player-registration.hpp` — PlayerRegistration app header

## Test Environment

- **Branch:** `wave20/12-e2e-demo` (based on current main)
- **Build:** `native_cli` (CLI simulator)
- **Build Date:** 2026-02-16
- **Commit:** (current main after PR #295 merge)

## Logs

- `/tmp/full-progression.log` — Full progression demo output
- `/tmp/duel.log` — Duel demo output
- `/tmp/all-games-showcase.log` — All games showcase output
- `/tmp/cable-disconnect-test.log` — Manual cable disconnect test output

## Next Steps

1. ❌ **Cannot confirm Bug #207 fix** — Player initialization issue prevents minigame testing
2. ✅ **Basic gameplay works** — Challenge mode and duel mode function correctly
3. ⚠️ **Demo scripts may need updates** — PlayerRegistration app blocking FDN encounters

**Status:** Testing blocked by PlayerRegistration initialization issue. Recommend investigating player setup flow before re-testing Bug #207 fix.
