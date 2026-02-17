#!/usr/bin/env bash
# pre-dispatch-check.sh — Gate check before dispatching new wave
# Returns 0 if safe to dispatch, 1 if blocked
set -euo pipefail

REPO="FinallyEve/pdn"
FLEET_STATE="$(git rev-parse --show-toplevel)/.fleet-state.json"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "🔍 Pre-Dispatch Safety Check"
echo "=============================="

# Check 1: Main CI Health
echo ""
echo "1️⃣ Checking main branch CI status..."
if ! bash "$SCRIPT_DIR/ci-health-check.sh" "$FLEET_STATE"; then
  echo "⛔ BLOCKED: Main branch CI is broken. Do not dispatch new agents."
  echo "   Fix main branch before dispatching new work."
  exit 1
fi

# Check 2: P0 Issues
echo ""
echo "2️⃣ Checking for P0 issues..."
P0_COUNT=$(gh issue list -R "$REPO" --label P0 --state open --json number --jq '. | length')
if [ "$P0_COUNT" -gt 0 ]; then
  echo "⚠️  WARNING: $P0_COUNT open P0 issue(s) found"
  gh issue list -R "$REPO" --label P0 --state open --json number,title --jq '.[] | "   #\(.number): \(.title)"'
  echo "   Consider resolving P0 issues before dispatching new agents."
  echo "   Continue? (P0s don't block dispatch, but should be prioritized)"
else
  echo "✅ No open P0 issues"
fi

# Check 3: Fleet State - Active Agents
echo ""
echo "3️⃣ Checking fleet state for active agents..."
if [ -f "$FLEET_STATE" ]; then
  # Check if there are agents still running from previous wave
  ACTIVE_AGENTS=$(jq -r '.agents // [] | map(select(.status == "running" or .status == "in_progress")) | length' "$FLEET_STATE" 2>/dev/null || echo "0")

  if [ "$ACTIVE_AGENTS" -gt 0 ]; then
    echo "⚠️  WARNING: $ACTIVE_AGENTS agent(s) still active from previous wave"
    jq -r '.agents // [] | map(select(.status == "running" or .status == "in_progress")) | .[] | "   Agent \(.id): \(.task // "unknown") - \(.status)"' "$FLEET_STATE" 2>/dev/null || true
    echo "   Consider waiting for agents to complete or manually updating fleet-state.json"
    echo "   Dispatching now may cause conflicts."
  else
    echo "✅ No active agents in fleet-state.json"
  fi

  # Show main CI status from fleet-state
  MAIN_STATUS=$(jq -r '.main_ci_status.status // "unknown"' "$FLEET_STATE" 2>/dev/null || echo "unknown")
  MAIN_SINCE=$(jq -r '.main_ci_status.since // "unknown"' "$FLEET_STATE" 2>/dev/null || echo "unknown")
  echo "   Main CI status: $MAIN_STATUS (as of $MAIN_SINCE)"
else
  echo "⚠️  WARNING: fleet-state.json not found at $FLEET_STATE"
  echo "   This is the first wave or fleet-state was deleted."
fi

# Check 4: Merge Queue Status
echo ""
echo "4️⃣ Checking merge queue status..."
QUEUE_SIZE=$(gh api repos/FinallyEve/pdn/pulls --jq '[.[] | select(.labels[].name == "merge-queue")] | length' 2>/dev/null || echo "0")
if [ "$QUEUE_SIZE" -gt 3 ]; then
  echo "⚠️  WARNING: $QUEUE_SIZE PRs in merge queue"
  echo "   Large merge queue may cause delays. Consider waiting for queue to clear."
else
  echo "✅ Merge queue size: $QUEUE_SIZE (acceptable)"
fi

# Final Decision
echo ""
echo "=============================="
echo "✅ PRE-DISPATCH CHECK PASSED"
echo "   Safe to dispatch new wave."
echo ""
exit 0
