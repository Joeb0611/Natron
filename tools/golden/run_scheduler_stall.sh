#!/usr/bin/env bash
# Local workstream A regression: short headless render must not stall or
# write zero frames (NatronGitHub/Natron#248). Does not use GitHub Actions
# and does not set NATRONPLUS_CI_ENABLED.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TESTS="${NATRON_TESTS:-$ROOT/build/Tests/Tests}"
TIMEOUT_SECS="${SCHEDULER_STALL_TIMEOUT_SECS:-60}"
LOG="${SCHEDULER_STALL_LOG:-$ROOT/../logs/scheduler-stall.log}"
FILTER="${SCHEDULER_STALL_FILTER:-SchedulerAbort.*}"

mkdir -p "$(dirname "$LOG")"

if [[ ! -x "$TESTS" ]]; then
  echo "SCHEDULER_FAIL: Tests binary not found at $TESTS" >&2
  exit 1
fi

if ! command -v timeout >/dev/null 2>&1; then
  echo "SCHEDULER_FAIL: GNU timeout is required to detect stalls" >&2
  exit 1
fi

set +e
timeout --signal=KILL "$TIMEOUT_SECS" "$TESTS" \
  --gtest_filter="$FILTER" \
  >"$LOG" 2>&1
status=$?
set -e

if [[ "$status" -eq 124 ]]; then
  echo "SCHEDULER_FAIL: Tests stalled (timeout ${TIMEOUT_SECS}s)" >&2
  tail -n 80 "$LOG" >&2 || true
  exit 1
fi
if [[ "$status" -ne 0 ]]; then
  echo "SCHEDULER_FAIL: Tests exited $status" >&2
  tail -n 80 "$LOG" >&2 || true
  exit "$status"
fi

if grep -Eqi 'wrote zero frames|stalled' "$LOG"; then
  echo "SCHEDULER_FAIL: stall or zero-frame assertion in log" >&2
  tail -n 80 "$LOG" >&2 || true
  exit 1
fi

echo "Scheduler abort/stall regression passed. Log: $LOG"
grep -E '\[  (PASSED|FAILED)  \]|SchedulerAbort' "$LOG" || true
