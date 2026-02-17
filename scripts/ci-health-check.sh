#!/usr/bin/env bash
# ci-health-check.sh — Check main branch CI status and update fleet-state
set -euo pipefail

REPO="FinallyEve/pdn"
FLEET_STATE="${1:-$(git rev-parse --show-toplevel)/.fleet-state.json}"

# Get latest CI run on main
# Note: gh run list doesn't support --branch, so we get all runs and filter for main branch
LATEST=$(gh api repos/"$REPO"/actions/runs?branch=main\&per_page=10 \
  --jq '[.workflow_runs[] | select(.name == "PR Checks")] | .[0]')

CONCLUSION=$(echo "$LATEST" | jq -r '.conclusion // "unknown"')
RUN_ID=$(echo "$LATEST" | jq -r '.id')
CREATED=$(echo "$LATEST" | jq -r '.created_at')

if [ "$CONCLUSION" = "success" ]; then
  STATUS="green"
  echo "✅ Main is GREEN (run $RUN_ID, $CREATED)"
else
  STATUS="broken"
  echo "❌ Main is BROKEN — conclusion: $CONCLUSION (run $RUN_ID, $CREATED)"
fi

# Update fleet-state.json
if [ -f "$FLEET_STATE" ]; then
  TMP=$(mktemp)
  jq --arg status "$STATUS" --arg since "$CREATED" --arg run "$RUN_ID" \
    '.main_ci_status = {status: $status, since: $since, last_run: ($run | tonumber)}' \
    "$FLEET_STATE" > "$TMP" && mv "$TMP" "$FLEET_STATE"
  echo "Updated fleet-state.json main_ci_status=$STATUS"
else
  # Create fleet-state.json if it doesn't exist
  TMP=$(mktemp)
  jq -n --arg status "$STATUS" --arg since "$CREATED" --arg run "$RUN_ID" \
    '{main_ci_status: {status: $status, since: $since, last_run: ($run | tonumber)}}' > "$TMP" && mv "$TMP" "$FLEET_STATE"
  echo "Created fleet-state.json with main_ci_status=$STATUS"
fi

[ "$STATUS" = "green" ] && exit 0 || exit 1
