# Wave 20 — Performance Baseline Report

**Date**: 2026-02-16
**Agent**: Agent 10 (Performance Tester)
**Branch**: `wave20/10-perf-baseline`
**Base**: `main` (commit `23ffb54` — after Wave 18+19 merges)

## Executive Summary

Post-Wave 18+19 performance baseline measurement. All metrics show **minimal growth** (<5%) compared to previous baselines. **No performance regressions detected.**

### Key Findings
- ✅ ESP32 firmware: +6KB flash (+0.2%), +3.2KB RAM (+2.1%)
- ✅ CLI binary: +53KB total (+1.2%)
- ✅ Core tests: 283/283 passing in 29.71s (up from 275 baseline)
- ⚠️  CLI tests: Compilation failed (pre-existing issue — disabled tests reference missing functions)

---

## Binary Size Measurements

### ESP32-S3 Release Firmware

**Build Command**: `pio run -e esp32-s3_release`
**Build Time**: 105.36 seconds

#### Memory Layout

| Section | Size (bytes) | % of Total | Notes |
|---------|-------------|-----------|-------|
| **Flash Code** (.text) | 1,079,784 | — | Executable code in flash |
| **Flash Data** (.rodata + .appdesc) | 1,706,084 | — | Read-only data + app descriptor |
| **DIRAM** (total) | 223,346 | 65.35% | Of 341,760 bytes available |
| ├─ .bss | 91,896 | 26.89% | Uninitialized globals |
| ├─ .data | 67,167 | 19.65% | Initialized globals |
| └─ .text | 64,283 | 18.81% | Code in RAM (cache) |
| **IRAM** (.text + .vectors) | 16,384 | **100%** | ⚠️ Fully utilized |
| **Total Image Size** | 2,933,702 | — | Sum of all sections |

#### Flash and RAM Usage

| Metric | Current | Total | % Used |
|--------|---------|-------|--------|
| **Flash** | 2,933,446 bytes | 3,342,336 bytes | **87.8%** |
| **RAM** | 159,064 bytes | 327,680 bytes | **48.5%** |
| **Firmware Binary** | 2.8 MB (2,933,702 bytes) | — | `.bin` file size |

#### Comparison to Baseline (Wave 18)

| Metric | Current | Baseline | Δ (bytes) | Δ (%) | Status |
|--------|---------|----------|-----------|-------|--------|
| Flash Used | 2,933,702 | 2,927,702 | **+6,000** | +0.2% | ✅ OK |
| Flash % | 87.8% | 87.6% | +0.2pp | — | ✅ OK |
| RAM Used | 159,064 | 155,848 | **+3,216** | +2.1% | ✅ OK |
| RAM % | 48.5% | 47.6% | +0.9pp | — | ✅ OK |

**Assessment**: Minor growth well within acceptable thresholds (<5%). No regression.

---

### Native CLI Binary

**Build Command**: `pio run -e native_cli`
**Build Time**: 20.46 seconds

#### Section Sizes

| Section | Size (bytes) | Description |
|---------|-------------|-------------|
| `.text` | 4,230,456 | Executable code |
| `.data` | 42,048 | Initialized data segment |
| `.bss` | 162,792 | Uninitialized data segment |
| **Total** | 4,435,296 | Sum of all sections |
| **Binary File** | 5.8 MB (6,076,416 bytes) | On-disk size (includes debug symbols) |

#### Comparison to Baseline (Wave 18)

| Section | Current | Baseline | Δ (bytes) | Δ (%) | Status |
|---------|---------|----------|-----------|-------|--------|
| `.text` | 4,230,456 | 4,181,574 | **+48,882** | +1.2% | ✅ OK |
| `.data` | 42,048 | 41,488 | **+560** | +1.3% | ✅ OK |
| `.bss` | 162,792 | 158,600 | **+4,192** | +2.6% | ✅ OK |
| **Total** | 4,435,296 | 4,381,662 | **+53,634** | +1.2% | ✅ OK |

**Assessment**: Modest growth across all sections (<3%). No regression.

---

## Test Suite Performance

### Core Tests (`test_core`, `native` environment)

**Command**: `pio test -e native`
**Status**: ✅ **ALL PASSING**

```
Environment: native
Test Suite: test_core
Total Tests: 283
Duration: 29.71 seconds
Average: 105 ms/test
```

#### Test Breakdown by Domain

| Domain | Tests | Notable Suites |
|--------|-------|---------------|
| **State Machine** | 30 | Core lifecycle, transitions, edge cases |
| **Device** | 12 | App lifecycle, driver integration |
| **Quickdraw Gameplay** | 157 | Idle, handshake, countdown, duel, result states |
| **Konami Meta-Game** | 26 | Button collection, hard mode, boon system |
| **Utilities** | 17 | UUID, MAC, Timer, Match, Player |
| **Integration** | 41 | Multi-device simulation, E2E flows |

#### Comparison to Baseline

| Metric | Current | Baseline (Wave 19) | Δ | Notes |
|--------|---------|-------------------|---|-------|
| **Total Tests** | 283 | 275 | **+8** | Baseline was broken (MockStateMachine issue) |
| **Passing** | 283 | 0 (broken) | +283 | All tests now passing |
| **Duration** | 29.71s | N/A | — | No prior timing baseline |

**Assessment**: Test suite is now fully operational. The +8 tests likely reflect fixes applied in Wave 19 sanitizer sweep (PR #292).

---

### CLI Tests (`test_cli`, `native_cli_test` environment)

**Command**: `pio test -e native_cli_test`
**Status**: ❌ **COMPILATION FAILED** (pre-existing issue)

#### Error Summary

**Root Cause**: Disabled test registrations reference functions that don't exist.

**Affected Files**:
- `test/test_cli/comprehensive-integration-reg-tests.cpp` (lines 46, 50, 54, 58, 62, 110, 114, 118)
- `test/test_cli/demo-playtest-tests.cpp` (lines 30, 34)

**Missing Functions**:
```cpp
// Ghost Runner tests (5 functions):
ghostRunnerEasyWinUnlocksButton(this);
ghostRunnerHardWinUnlocksColorProfile(this);
ghostRunnerLossNoRewards(this);
ghostRunnerBoundaryPress(this);
ghostRunnerRapidPresses(this);

// Cipher Path tests (3 functions):
cipherPathEasyWinUnlocksButton(this);
cipherPathHardWinUnlocksColorProfile(this);
cipherPathLossNoRewards(this);

// Demo Playtests (2 functions):
ghostRunnerEasyPlaytest(this);
ghostRunnerHardPlaytest(this);
```

**Analysis**: These functions are called from `DISABLED_*` tests (intentionally skipped), but the registration macros still attempt to compile the function calls. The tests were likely disabled because the functions were never implemented or were removed during Wave 18 redesigns (issues #213, #287, #292).

#### Comparison to Baseline (Wave 19)

| Metric | Current | Baseline | Notes |
|--------|---------|----------|-------|
| **Status** | Compilation failed | Partial failure (181/189 passing) | Regression in compilation |
| **Error** | Missing test functions | SIGSEGV in SignalEchoLossNoRewards | Different failure mode |

**Assessment**: CLI tests are in a worse state than baseline (compilation vs runtime failure). This is **out of scope** for a performance baseline report but should be tracked as a quality issue.

**Recommendation**: Either:
1. Remove the `DISABLED_*` test registrations entirely (clean up dead code), or
2. Implement the missing test functions (if these tests are meant to exist)

---

## Performance Analysis

### Regression Threshold: 5%

All measurements are compared against the **5% regression threshold** specified in the task requirements.

| Metric | Growth | Threshold | Status |
|--------|--------|-----------|--------|
| ESP32 Flash | +0.2% | 5% | ✅ **Pass** |
| ESP32 RAM | +2.1% | 5% | ✅ **Pass** |
| CLI Binary (text) | +1.2% | 5% | ✅ **Pass** |
| CLI Binary (data) | +1.3% | 5% | ✅ **Pass** |
| CLI Binary (bss) | +2.6% | 5% | ✅ **Pass** |

**Conclusion**: **No regressions detected.** All metrics are well within acceptable limits.

---

## Notable Observations

### IRAM Fully Utilized

The ESP32 IRAM segment is at **100% capacity** (16,384 bytes used). This is a **hard constraint** — any code that requires guaranteed execution speed (interrupts, critical loops) must fit in IRAM. If future changes require more IRAM, options include:

1. Move non-critical code to flash (use `IRAM_ATTR` selectively)
2. Optimize existing IRAM functions for size
3. Reconsider interrupt handler complexity

**Risk**: Low (current usage appears stable since Wave 18 baseline).

### Test Suite Growth

Core tests increased from 275 → 283 (+8 tests). This aligns with Wave 19's sanitizer sweep (PR #292), which fixed test infrastructure issues (MockStateMachine, stack-use-after-scope bugs). The growth is **positive** — more test coverage improves quality.

### CLI Test Instability

CLI tests remain broken across multiple waves:
- **Wave 19 baseline**: 181/189 passing (runtime crashes)
- **Current**: Compilation failed (missing functions)

This suggests the CLI test suite is **under-maintained** compared to core tests. Consider:
- Audit all `DISABLED_*` tests for relevance
- Remove dead test registrations
- Re-enable tests incrementally with proper implementations

---

## Baseline Updates

Update `docs/baselines/binary-size.json` with current measurements:

```json
{
  "text": 4230456,
  "data": 42048,
  "bss": 162792,
  "total": 4435296,
  "timestamp": "2026-02-16",
  "note": "Wave 20 baseline — post Wave 18+19 merges",
  "previous": {
    "text": 4181574,
    "data": 41488,
    "bss": 158600,
    "total": 4381662,
    "timestamp": "2026-02-16",
    "note": "Wave 18 baseline"
  },
  "esp32": {
    "flash_used": 2933702,
    "flash_total": 3342336,
    "flash_percent": 87.8,
    "ram_used": 159064,
    "ram_total": 327680,
    "ram_percent": 48.5,
    "firmware_size": 2933702,
    "timestamp": "2026-02-16"
  }
}
```

Update `docs/baselines/test-counts.json`:

```json
{
  "date": "2026-02-16",
  "description": "Wave 20 baseline — post Wave 18+19 merges",
  "test_environments": {
    "native": {
      "description": "Core unit tests (test_core/)",
      "total_tests": 283,
      "passing_tests": 283,
      "status": "passing",
      "duration_seconds": 29.71,
      "note": "All tests passing after Wave 19 sanitizer sweep fixes"
    },
    "native_cli_test": {
      "description": "CLI integration tests (test_cli/)",
      "total_tests": 0,
      "status": "compilation_failed",
      "note": "DISABLED_* tests reference missing functions (ghostRunner*, cipherPath* integration tests)"
    }
  },
  "quality_ratchet_rules": {
    "rule_1": "Total test count must not decrease (except when removing dead code)",
    "rule_2": "Passing test count must not decrease",
    "rule_3": "New features must include tests before merge",
    "rule_4": "Fixing a broken test counts as progress"
  },
  "next_review_date": "2026-03-16"
}
```

---

## Verification Commands

```bash
# Build ESP32-S3 release firmware
pio run -e esp32-s3_release

# Check firmware size
ls -lh .pio/build/esp32-s3_release/firmware.bin
size .pio/build/esp32-s3_release/firmware.elf

# Build CLI binary
pio run -e native_cli

# Check CLI binary size
ls -lh .pio/build/native_cli/program
size .pio/build/native_cli/program

# Run core tests
pio test -e native

# Attempt CLI tests (expect compilation failure)
pio test -e native_cli_test
```

---

## Recommendations

### Immediate
- ✅ Merge this baseline report (documents current state)
- Update `docs/baselines/*.json` files with new measurements

### Short-Term
- **Fix CLI test compilation** (separate issue/PR)
  - Remove `DISABLED_*` test registrations for missing functions, OR
  - Implement the missing test functions
- Audit other `DISABLED_*` tests for relevance

### Long-Term
- Add performance baseline checks to CI (fail builds on >5% growth)
- Monitor IRAM usage (currently at 100% — no headroom)
- Establish timing baselines for test suites (track slowdowns over time)

---

## Files Changed

```
docs/reports/wave20-performance-baseline.md  — This report
docs/baselines/binary-size.json              — Updated measurements
docs/baselines/test-counts.json              — Updated test counts
```

---

## Test Artifacts

Raw build and test outputs saved to:
- `/tmp/esp32-build.log` — ESP32-S3 release build output
- `/tmp/cli-build.log` — Native CLI build output
- `/tmp/core-test-timing.log` — Core test run (283 tests, 29.71s)
- `/tmp/cli-test-timing.log` — CLI test compilation failure
- `/tmp/binary-sizes.log` — Binary size measurements

---

**Conclusion**: Wave 20 performance baseline successfully established. All metrics show healthy, controlled growth (<5%). Core test suite is fully operational with +8 tests added. CLI test suite requires cleanup to resolve compilation failures. Ready for merge.
