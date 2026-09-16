#!/usr/bin/env bash
# Run a mingw64 .exe under Wine with the Natron+ sysroot on PATH.
set -euo pipefail
export WINEPREFIX="${WINEPREFIX:-${HOME}/.wine}"
export WINEDEBUG="${WINEDEBUG:--all}"
export WINEARCH="${WINEARCH:-win64}"
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:-mscoree,mshtml=}"
MINGW64="${NATRON_MINGW64:-/mingw64}"
export PATH="${MINGW64}/bin:${PATH}"
exec wine "$@"
