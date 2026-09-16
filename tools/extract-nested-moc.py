#!/usr/bin/env python3
"""Keep only one QObject metaobject from a moc output that processed includes."""
from __future__ import annotations

import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 4:
        print(
            "usage: extract-nested-moc.py <moc.cpp> <out.cpp> <class_tag>",
            file=sys.stderr,
        )
        return 2
    src, dst, tag = sys.argv[1], sys.argv[2], sys.argv[3]
    text = Path(src).read_text(encoding="utf-8", errors="replace")
    marker = f"struct qt_meta_stringdata_{tag}_t"
    idx = text.find(marker)
    if idx < 0:
        print(f"extract-nested-moc: {marker} not found in {src}", file=sys.stderr)
        return 1
    begin = text.find("QT_BEGIN_MOC_NAMESPACE")
    if begin < 0:
        preamble, rest = "", text
    else:
        preamble, rest = text[:begin], text[begin:]
        idx = rest.find(marker)
        rest = rest[idx:]
        preamble += (
            "QT_BEGIN_MOC_NAMESPACE\n"
            "QT_WARNING_PUSH\n"
            "QT_WARNING_DISABLE_DEPRECATED\n"
        )
    Path(dst).write_text(preamble + rest, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
