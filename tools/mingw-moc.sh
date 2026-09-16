#!/usr/bin/env bash
# Wine moc.exe plus a filter that drops metaobjects from included headers.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"

out=""
input=""
args=("$@")
i=0
while [[ $i -lt ${#args[@]} ]]; do
    a="${args[$i]}"
    if [[ "$a" == "-o" && $((i + 1)) -lt ${#args[@]} ]]; then
        i=$((i + 1))
        out="${args[$i]}"
    elif [[ "$a" == -o?* ]]; then
        out="${a:2}"
    elif [[ "$a" != -* && "$a" != @* ]]; then
        input="$a"
    fi
    i=$((i + 1))
done

"${here}/mingw-wine-tool.sh" moc.exe "$@"

if [[ -n "$out" && -n "$input" && -f "$out" && -f "$input" ]]; then
    python3 "${here}/filter-automoc.py" "$out" "$input"
fi
