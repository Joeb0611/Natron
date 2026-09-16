#!/usr/bin/env bash
# Unix-facing wrapper around a single mingw64 tool (moc.exe, python.exe, …).
set -euo pipefail
tool="${1:?usage: mingw-wine-tool.sh <name.exe> [args...]}"
shift
MINGW64="${NATRON_MINGW64:-/mingw64}"
here="$(cd "$(dirname "$0")" && pwd)"
exec "${here}/mingw-wine-run.sh" "${MINGW64}/bin/${tool}" "$@"
