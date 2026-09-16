#!/usr/bin/env python3
"""Drop QObject metaobjects that moc emitted from included headers.

Wine/MSYS2 moc follows #includes and also emits base classes (NodeGroup,
OutputEffectInstance, …). Those already have their own moc_*.cpp, so the
extra copies fail the MinGW link with multiple definition.

Keep a generated class when the primary source *defines* `class Name` /
`struct Name` (not a forward declaration). Nested tags Parent__Child stay
if any path component is a local class.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

# Qualified nested defs: `struct ViewerInstance::ViewerInstancePrivate {`
# Stop at a single colon (bases) but not `::`.
CLASS_DEF = re.compile(
    r"\b(?:class|struct)\s+(?:[A-Z_][A-Z0-9_]*\s+)*(?:[A-Za-z_]\w*::)*([A-Za-z_]\w*)\s*(?::(?!:)|\{)",
    re.MULTILINE,
)
COMMENT_RE = re.compile(r"//.*?$|/\*.*?\*/", re.MULTILINE | re.DOTALL)
INCLUDE_RE = re.compile(r"^\s*#\s*include\b.*$", re.MULTILINE)
STRINGDATA_RE = re.compile(r"struct qt_meta_stringdata_([A-Za-z0-9_]+)_t\b")


def strip_noise(text: str) -> str:
    text = COMMENT_RE.sub("", text)
    text = INCLUDE_RE.sub("", text)
    return text


def local_classes(header_text: str) -> set[str]:
    return set(CLASS_DEF.findall(strip_noise(header_text)))


def tag_is_local(tag: str, names: set[str]) -> bool:
    return any(part in names for part in tag.split("__") if part)


def split_moc_classes(body: str) -> list[tuple[str, str]]:
    matches = list(STRINGDATA_RE.finditer(body))
    if not matches:
        return []
    chunks: list[tuple[str, str]] = []
    for i, m in enumerate(matches):
        end = matches[i + 1].start() if i + 1 < len(matches) else len(body)
        chunks.append((m.group(1), body[m.start() : end]))
    return chunks


def filter_moc(moc_text: str, names: set[str]) -> str:
    begin = moc_text.find("QT_BEGIN_MOC_NAMESPACE")
    if begin < 0:
        return moc_text
    preamble = moc_text[:begin]
    rest = moc_text[begin:]
    ns_end = rest.find("QT_END_MOC_NAMESPACE")
    if ns_end < 0:
        body, tail = rest, ""
    else:
        body, tail = rest[:ns_end], rest[ns_end:]
    inner = body
    marker = "QT_WARNING_DISABLE_DEPRECATED"
    idx = inner.find(marker)
    if idx >= 0:
        inner = inner[idx + len(marker) :].lstrip("\n")
    chunks = split_moc_classes(inner)
    if not chunks:
        return moc_text
    kept = [(tag, chunk) for tag, chunk in chunks if tag_is_local(tag, names)]
    if not kept:
        return moc_text
    if len(kept) == len(chunks):
        return moc_text
    rebuilt = (
        preamble
        + "QT_BEGIN_MOC_NAMESPACE\n"
        + "QT_WARNING_PUSH\n"
        + "QT_WARNING_DISABLE_DEPRECATED\n"
        + "".join(chunk for _, chunk in kept)
        + tail
    )
    return rebuilt


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: filter-automoc.py <moc.cpp> <source.h>", file=sys.stderr)
        return 2
    moc_path = Path(sys.argv[1])
    src_path = Path(sys.argv[2])
    if not moc_path.is_file() or not src_path.is_file():
        return 0
    src = src_path.read_text(encoding="utf-8", errors="replace")
    names = local_classes(src)
    if not names:
        return 0
    original = moc_path.read_text(encoding="utf-8", errors="replace")
    filtered = filter_moc(original, names)
    if filtered != original:
        moc_path.write_text(filtered, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
