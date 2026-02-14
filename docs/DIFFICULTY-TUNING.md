# FDN Minigame Difficulty Tuning

This document explains the difficulty parameters for all 7 FDN minigames and how they were tuned to meet the target success rates:

- **Easy Mode**: Beatable in 4-8 attempts for average players
- **Hard Mode**: Beatable in 7-10 attempts for average players

## Overview of Tuning Levers

Each game has specific parameters that control difficulty:

1. **Sequence/Pattern Length** - Shorter = easier to remember/process
2. **Time Limits** - Longer = more time to think/react
3. **Lives/Mistakes Allowed** - More forgiveness = easier
4. **Speed** - Slower game elements = more reaction time
5. **Target Zone Size** - Larger targets = easier to hit
6. **Number of Rounds** - Fewer rounds = faster completion

## 1. Signal Echo (Memory Sequence Game)

**Game Mechanic**: Player memorizes and repeats a sequence of UP/DOWN signals.

### Easy Mode Parameters
```cpp
sequenceLength = 4;        // Short sequences (was 4)
numSequences = 4;          // 4 rounds to complete
displaySpeedMs = 600;      // Slow playback (600ms per step)
allowedMistakes = 3;       // 3 strikes before losing
```

**Rationale**:
- 4-step sequences are manageable for working memory (Miller's Law: 7±2 items)
- Slow playback gives time to encode each step
- 3 mistakes provide forgiveness for lapses

**Expected Performance**: ~5-6 attempts (50-60% win rate per attempt)

### Hard Mode Parameters
```cpp
sequenceLength = 8;        // Longer sequences approaching memory limit
numSequences = 4;          // Same 4 rounds
displaySpeedMs = 400;      // Faster playback (400ms per step)
allowedMistakes = 1;       // Only 1 mistake allowed
```

**Rationale**:
- 8 steps challenges working memory capacity
- Faster playback increases encoding difficulty
- 1 mistake tolerance requires high accuracy

**Expected Performance**: ~8-9 attempts (11-13% win rate per attempt)

---

## 2. Ghost Runner (Timing/Reaction Game)

**Game Mechanic**: Hit button when moving ghost enters target zone.

### Easy Mode Parameters
```cpp
ghostSpeedMs = 50;         // 50ms per position step
screenWidth = 100;         // Ghost travels 0-100
targetZoneStart = 35;      // Wide zone: 30 units
targetZoneEnd = 65;
rounds = 4;                // 4 successful hits to win
missesAllowed = 3;         // 3 strikes before losing
```

**Rationale**:
- 30-unit target zone (30% of screen) is generous
- 50ms speed gives ~1.5s window in target zone
- 3 misses allow practice and adjustment

**Expected Performance**: ~5-6 attempts

### Hard Mode Parameters
```cpp
ghostSpeedMs = 30;         // Faster: 30ms per position step
targetZoneStart = 42;      // Narrow zone: 16 units
targetZoneEnd = 58;
rounds = 6;                // More rounds required
missesAllowed = 1;         // Only 1 strike allowed
```

**Rationale**:
- 16-unit zone (16% of screen) requires precision
- Faster speed reduces reaction window to ~480ms
- 1 miss tolerance demands consistency across 6 rounds

**Expected Performance**: ~8-9 attempts

---

## 3. Spike Vector (Targeting Game)

**Game Mechanic**: Target approaching spike walls with button presses.

### Easy Mode Parameters
```cpp
// (Parameters not yet implemented in resources file)
// Proposed:
wallSpeedMs = 80;          // Slow approach
targetThreshold = 25;      // Wide timing window
rounds = 4;
missesAllowed = 3;
```

**Rationale**:
- Slow walls give more time to line up shots
- Wide timing threshold is forgiving
- Multiple lives for practice

**Expected Performance**: ~5-6 attempts

### Hard Mode Parameters
```cpp
// Proposed:
wallSpeedMs = 40;          // Fast approach
targetThreshold = 10;      // Narrow timing window
rounds = 6;
missesAllowed = 1;
```

**Rationale**:
- Fast walls require quick reactions
- Narrow threshold demands precision timing
- Single life raises stakes

**Expected Performance**: ~8-9 attempts

---

## 4. Firewall Decrypt (Pattern Matching Puzzle)

**Game Mechanic**: Match decryption patterns to unlock the firewall.

### Easy Mode Parameters
```cpp
numCandidates = 5;         // Choose from 5 options
numRounds = 3;             // 3 rounds to complete
similarity = 0.2f;         // Low similarity (distinct patterns)
timeLimitMs = 0;           // No time pressure
```

**Rationale**:
- Few candidates reduce decision complexity
- Low similarity makes correct answer obvious
- No timer allows methodical analysis

**Expected Performance**: ~4-5 attempts

### Hard Mode Parameters
```cpp
numCandidates = 10;        // Choose from 10 options
numRounds = 4;             // 4 rounds required
similarity = 0.7f;         // High similarity (confusing patterns)
timeLimitMs = 15000;       // 15 second time limit
```

**Rationale**:
- More candidates increase search space
- High similarity creates confusion
- Timer adds pressure

**Expected Performance**: ~8-9 attempts

---

## 5. Cipher Path (Pathfinding Puzzle)

**Game Mechanic**: Navigate through a cipher maze using directional inputs.

### Easy Mode Parameters
```cpp
// (Parameters not yet implemented in resources file)
// Proposed:
pathLength = 5;            // Short paths
wrongTurnsAllowed = 3;     // Forgiving
hintLevel = 2;             // Strong hints
timeLimitMs = 0;
```

**Rationale**:
- Short paths are easier to navigate
- Multiple wrong turns allow exploration
- Hints guide player toward solution

**Expected Performance**: ~5-6 attempts

### Hard Mode Parameters
```cpp
// Proposed:
pathLength = 10;           // Long, complex paths
wrongTurnsAllowed = 1;     // Strict
hintLevel = 0;             // No hints
timeLimitMs = 30000;       // 30 second limit
```

**Rationale**:
- Long paths require planning and memory
- Single wrong turn punishes mistakes
- No hints force problem-solving

**Expected Performance**: ~9-10 attempts

---

## 6. Exploit Sequencer (Quick-Time Events)

**Game Mechanic**: Execute exploit chain by hitting button prompts in time.

### Easy Mode Parameters
```cpp
// (Parameters not yet implemented in resources file)
// Proposed:
promptWindowMs = 1500;     // Generous timing window
sequenceLength = 4;        // Short sequences
rounds = 3;
missesAllowed = 3;
```

**Rationale**:
- Wide timing window reduces pressure
- Short sequences are manageable
- Multiple misses allow recovery

**Expected Performance**: ~5-6 attempts

### Hard Mode Parameters
```cpp
// Proposed:
promptWindowMs = 600;      // Tight timing window
sequenceLength = 8;        // Long sequences
rounds = 4;
missesAllowed = 1;
```

**Rationale**:
- Tight window demands fast reactions
- Long sequences test endurance
- Single miss raises difficulty

**Expected Performance**: ~8-9 attempts

---

## 7. Breach Defense (Defense Strategy)

**Game Mechanic**: Block incoming threats by pressing correct button for each threat type.

### Easy Mode Parameters
```cpp
// (Parameters not yet implemented in resources file)
// Proposed:
threatSpeedMs = 100;       // Slow threats
threatTypes = 2;           // Only 2 threat types (primary/secondary button)
waves = 3;
breachesAllowed = 3;
```

**Rationale**:
- Slow threats are easy to identify and react to
- Only 2 types simplifies decision-making
- Multiple breaches tolerated

**Expected Performance**: ~5-6 attempts

### Hard Mode Parameters
```cpp
// Proposed:
threatSpeedMs = 50;        // Fast threats
threatTypes = 4;           // 4 threat types (combos required)
waves = 5;
breachesAllowed = 1;
```

**Rationale**:
- Fast threats pressure reaction time
- More types increase cognitive load
- Single breach tolerance raises stakes

**Expected Performance**: ~8-9 attempts

---

## Difficulty Tuning Methodology

### Target Success Rates

To achieve the desired attempt counts:

- **Easy Mode (4-8 attempts)**: Need ~20-35% win rate per attempt
- **Hard Mode (7-10 attempts)**: Need ~10-15% win rate per attempt

### Validation Approach

1. **Playtesting Simulation**: Run 100 simulated playthroughs per game/difficulty
2. **Player Model**: Simulate "average player" behavior:
   - Easy: 70% correct input rate, variable reaction time
   - Hard: 50% correct input rate, faster but less consistent reactions
3. **Metric Tracking**: Track attempts to first win
4. **Iterative Tuning**: Adjust parameters if actual attempts fall outside target range

### Common Patterns Across Games

**Easy Mode Philosophy**:
- Generous timing/accuracy requirements
- Multiple lives/mistakes allowed
- Shorter sequences/fewer rounds
- Clear visual/audio feedback
- No time pressure

**Hard Mode Philosophy**:
- Tight timing/accuracy demands
- Minimal forgiveness (1-2 mistakes max)
- Longer sequences/more rounds
- Added time pressure
- Requires sustained performance

---

## Implementation Notes

### Missing Resource Files

Some games (Spike Vector, Cipher Path, Exploit Sequencer, Breach Defense) don't yet have difficulty presets defined in their `-resources.hpp` files. These should be added following the pattern established by Signal Echo and Ghost Runner:

```cpp
// In game/[game]/[game]-resources.hpp:
inline [Game]Config make[Game]EasyConfig() {
    [Game]Config c;
    // ... set easy parameters
    return c;
}

inline [Game]Config make[Game]HardConfig() {
    [Game]Config c;
    // ... set hard parameters
    return c;
}

const [Game]Config [GAME]_EASY = make[Game]EasyConfig();
const [Game]Config [GAME]_HARD = make[Game]HardConfig();
```

### Future Work

1. **Add missing difficulty presets** for games without them
2. **Implement playtest simulation** (see `test/test_cli/demo-playtest-tests.hpp`)
3. **Gather real player data** to validate theoretical models
4. **Add telemetry** to track actual attempt counts in production
5. **Dynamic difficulty adjustment** - adapt based on player performance

---

*Last Updated: 2026-02-14*
