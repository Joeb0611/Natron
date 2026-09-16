#!/usr/bin/env bash
# Local Slice 2 golden: NatronRenderer + timeout (stall => fail).
# Does not use GitHub Actions and does not set NATRONPLUS_CI_ENABLED.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
RENDERER="${NATRON_RENDERER:-$ROOT/build/Renderer/NatronRenderer}"
SCRIPT="$ROOT/tools/golden/golden_render.py"
TIMEOUT_SECS="${GOLDEN_TIMEOUT_SECS:-90}"
LOG="${GOLDEN_LOG:-$ROOT/../logs/golden-render.log}"

mkdir -p "$(dirname "$LOG")"

if [[ ! -x "$RENDERER" ]]; then
  echo "GOLDEN_FAIL: NatronRenderer not found at $RENDERER" >&2
  exit 1
fi

if ! command -v timeout >/dev/null 2>&1; then
  echo "GOLDEN_FAIL: GNU timeout is required to detect stalls" >&2
  exit 1
fi

set +e
PYTHONUNBUFFERED=1 timeout --signal=KILL "$TIMEOUT_SECS" "$RENDERER" \
  --background --no-settings -t "$SCRIPT" \
  >"$LOG" 2>&1
status=$?
set -e

if [[ "$status" -eq 124 ]]; then
  echo "GOLDEN_FAIL: NatronRenderer stalled (timeout ${TIMEOUT_SECS}s)" >&2
  tail -n 40 "$LOG" >&2 || true
  exit 1
fi
if [[ "$status" -ne 0 ]]; then
  echo "GOLDEN_FAIL: NatronRenderer exited $status" >&2
  tail -n 40 "$LOG" >&2 || true
  exit "$status"
fi

if ! grep -q '^GOLDEN_OK$' "$LOG"; then
  echo "GOLDEN_FAIL: missing GOLDEN_OK" >&2
  tail -n 40 "$LOG" >&2 || true
  exit 1
fi
if grep -q '^GOLDEN_FRAMES=0$' "$LOG"; then
  echo "GOLDEN_FAIL: wrote zero frames" >&2
  exit 1
fi
if ! grep -qE '^GOLDEN_HASH=[0-9a-f]{64}$' "$LOG"; then
  echo "GOLDEN_FAIL: missing project hash" >&2
  exit 1
fi

echo "Golden render passed. Log: $LOG"
grep -E '^GOLDEN_' "$LOG" || true
