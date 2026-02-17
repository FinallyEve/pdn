# CI Health Tracking System

## Overview

The CI health tracking system monitors the main branch CI status and provides safety gates for the multi-agent fleet orchestrator. This prevents agents from dispatching work against a broken main branch, which would result in wasted effort and cascading failures.

## System Components

### 1. CI Health Check Script

**Location**: `scripts/ci-health-check.sh`

**Purpose**: Checks the latest CI run on main branch and updates fleet-state.json

**Usage**:
```bash
# Check CI health and update fleet-state.json
bash scripts/ci-health-check.sh

# Specify custom fleet-state.json location
bash scripts/ci-health-check.sh /path/to/.fleet-state.json
```

**Exit Codes**:
- `0` - Main is green (CI passing)
- `1` - Main is broken (CI failing or unknown)

**Output**:
```bash
✅ Main is GREEN (run 123456, 2026-02-17T10:30:00Z)
Updated fleet-state.json main_ci_status=green
```

### 2. Pre-Dispatch Check Script

**Location**: `scripts/pre-dispatch-check.sh`

**Purpose**: Comprehensive safety gate run by orchestrator before dispatching new waves

**Checks Performed**:
1. **Main CI Health** - Blocks if main is broken
2. **P0 Issues** - Warns if critical issues exist (non-blocking)
3. **Active Agents** - Warns if agents from previous wave are still running
4. **Merge Queue Size** - Warns if queue is large (>3 PRs)

**Usage**:
```bash
# Run pre-dispatch check
bash scripts/pre-dispatch-check.sh

# Integrate into orchestrator workflow
if bash scripts/pre-dispatch-check.sh; then
  echo "Safe to dispatch wave"
  # Dispatch agents...
else
  echo "Dispatch blocked - fix issues first"
  exit 1
fi
```

**Exit Codes**:
- `0` - Safe to dispatch
- `1` - Blocked (main CI broken)

## Fleet State Schema

### `main_ci_status` Field

The `.fleet-state.json` file tracks main branch CI health:

```json
{
  "main_ci_status": {
    "status": "green",
    "since": "2026-02-17T10:30:00Z",
    "last_run": 123456
  },
  "agents": [
    {
      "id": "claude-agent-01",
      "task": "feature/new-feature",
      "status": "running",
      "started": "2026-02-17T11:00:00Z"
    }
  ]
}
```

**Fields**:
- `status`: `"green"` (passing) or `"broken"` (failing/unknown)
- `since`: ISO 8601 timestamp of when this status was observed
- `last_run`: GitHub Actions run ID for reference

### Agent Status Values

- `"running"` - Agent actively working
- `"in_progress"` - Agent dispatched but not completed
- `"completed"` - Agent finished successfully
- `"failed"` - Agent encountered errors

## Integration with Orchestrator

### Workflow Integration Points

**1. Before Dispatching New Wave**
```bash
# Step 1: Check if safe to dispatch
if ! bash scripts/pre-dispatch-check.sh; then
  echo "❌ Cannot dispatch - resolve blockers first"
  exit 1
fi

# Step 2: Dispatch agents
for agent in wave-agents/*; do
  dispatch_agent "$agent"
done

# Step 3: Update fleet-state.json with new agents
```

**2. After Main Branch Merge**
```bash
# Step 1: Wait for CI to complete
echo "Waiting for CI checks on main..."
sleep 60

# Step 2: Update CI health status
bash scripts/ci-health-check.sh

# Step 3: If green, safe to continue next wave
```

**3. Periodic Health Monitoring**
```bash
# Run every 15 minutes via cron or systemd timer
*/15 * * * * cd /path/to/pdn && bash scripts/ci-health-check.sh
```

## Severity System Integration

The pre-dispatch check integrates with GitHub issue labels for severity tracking:

| Label | Meaning | Dispatch Behavior |
|-------|---------|-------------------|
| `P0` | Critical - system broken | Warns, does not block |
| `P1` | High priority - major feature broken | No action |
| `P2` | Medium priority - minor feature broken | No action |
| `P3` | Low priority - cosmetic or nice-to-have | No action |

**Note**: Only broken main CI **blocks** dispatch. P0 issues warn but don't block, as they may be known issues being actively worked on.

## Error Handling

### Main CI Broken

**Symptom**: `ci-health-check.sh` returns exit code 1

**Resolution**:
1. Check latest CI run: `gh run list -R FinallyEve/pdn --branch main --limit 1`
2. View failure logs: `gh run view <run-id>`
3. Fix the issue or revert the breaking commit
4. Wait for CI to pass
5. Re-run `ci-health-check.sh` to confirm green status

### Fleet State Corruption

**Symptom**: `.fleet-state.json` is malformed or missing

**Resolution**:
```bash
# Backup existing state
cp .fleet-state.json .fleet-state.json.bak

# Regenerate with ci-health-check
bash scripts/ci-health-check.sh

# Manually add agent entries if needed
jq '.agents = []' .fleet-state.json | sponge .fleet-state.json
```

### Rate Limiting

**Symptom**: `gh` commands fail with HTTP 429

**Resolution**:
- Wait 60 seconds between API calls
- Use `--limit 1` to minimize API usage
- Check rate limit status: `gh api rate_limit`

## Troubleshooting

### Script Fails with "command not found"

**Cause**: `gh` (GitHub CLI) or `jq` not installed

**Fix**:
```bash
# Install gh
sudo apt install gh

# Install jq
sudo apt install jq
```

### Script Shows "unknown" Status

**Cause**: No "PR Checks" workflow found on main branch

**Fix**:
- Verify workflow name: `gh run list -R FinallyEve/pdn --branch main --json name`
- Update script if workflow name differs

### Fleet State Not Updating

**Cause**: Permission denied writing to `.fleet-state.json`

**Fix**:
```bash
# Check permissions
ls -la .fleet-state.json

# Fix if needed
chmod 664 .fleet-state.json
```

## Best Practices

1. **Run pre-dispatch-check.sh before every wave** - This is mandatory for orchestrator
2. **Update CI health after merges** - Run `ci-health-check.sh` after merging to main
3. **Monitor fleet-state.json** - Keep it under version control (add to .gitignore if contains sensitive data)
4. **Respect blocking conditions** - Do not override main CI blocks without human approval
5. **Handle warnings appropriately** - P0 warnings should be triaged but don't require immediate blocking

## Future Enhancements

Potential improvements for this system:

- **Slack/Discord notifications** when main breaks
- **Historical CI health tracking** in separate JSON file
- **Per-agent CI health tracking** (which agents are blocked by main breakage)
- **Auto-retry mechanism** for transient CI failures
- **Integration with Mergify** to pause merge queue when main is broken

---

**Related Issues**: #322 (Fleet state monitoring)
**Last Updated**: 2026-02-17
