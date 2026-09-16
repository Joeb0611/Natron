#!/usr/bin/env python3
"""Generate Natron+ app icons (plus mark, not the upstream Natron lettermark)."""
from __future__ import annotations

import struct
import zlib
from pathlib import Path

SIZE = 256
PLUS = 28
ARM = 118


def is_plus(x: int, y: int) -> bool:
    cx = cy = SIZE // 2
    return (abs(x - cx) < PLUS and abs(y - cy) < ARM) or (
        abs(y - cy) < PLUS and abs(x - cx) < ARM
    )


def is_plate(x: int, y: int) -> bool:
    margin = 24
    rx = ry = 40
    if x < margin or y < margin or x >= SIZE - margin or y >= SIZE - margin:
        return False
    # crude rounded-rect by ignoring far corners
    dx = min(x - margin, SIZE - margin - 1 - x)
    dy = min(y - margin, SIZE - margin - 1 - y)
    if dx < rx and dy < ry:
        return (rx - dx) ** 2 + (ry - dy) ** 2 <= rx * ry
    return True


def pixels() -> list[tuple[int, int, int, int]]:
    out = []
    bg = (20, 22, 28, 255)
    plate = (30, 36, 48, 255)
    plus = (232, 217, 168, 255)
    stroke = (200, 176, 122, 255)
    for y in range(SIZE):
        for x in range(SIZE):
            if is_plus(x, y) and is_plate(x, y):
                out.append(plus)
            elif is_plate(x, y):
                # 6px inner stroke
                if not is_plate(x - 6, y) or not is_plate(x + 6, y) or not is_plate(
                    x, y - 6
                ) or not is_plate(x, y + 6):
                    out.append(stroke)
                else:
                    out.append(plate)
            else:
                out.append(bg)
    return out


def write_png(path: Path, pix: list[tuple[int, int, int, int]]) -> None:
    raw = b"".join(b"\x00" + bytes(c for p in pix[y * SIZE : (y + 1) * SIZE] for c in p) for y in range(SIZE))

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", SIZE, SIZE, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    path.write_bytes(png)


def write_xpm(path: Path, pix: list[tuple[int, int, int, int]]) -> None:
    palette = {
        (20, 22, 28, 255): (" ", "#14161c"),
        (30, 36, 48, 255): (".", "#1e2430"),
        (200, 176, 122, 255): ("+", "#c8b07a"),
        (232, 217, 168, 255): ("X", "#e8d9a8"),
    }
    lines = [
        "/* XPM */",
        "static char * natronplusIcon[] = {",
        f'"{SIZE} {SIZE} {len(palette)} 1",',
    ]
    for (rgba, (ch, hexcol)) in palette.items():
        lines.append(f'"{ch} c {hexcol}",')
    for y in range(SIZE):
        row = []
        for x in range(SIZE):
            row.append(palette[pix[y * SIZE + x]][0])
        comma = "," if y + 1 < SIZE else ""
        lines.append('"' + "".join(row) + '"' + comma)
    lines.append("};")
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def write_ico(path: Path, png_bytes: bytes) -> None:
    # PNG-in-ICO (Vista+)
    header = struct.pack("<HHH", 0, 1, 1)
    entry = struct.pack("<BBBBHHII", 0, 0, 0, 0, 1, 32, len(png_bytes), 22)
    path.write_bytes(header + entry + png_bytes)


def main() -> None:
    root = Path(__file__).resolve().parents[1] / "Gui" / "Resources" / "Images"
    pix = pixels()
    png_path = root / "natronplusIcon256.png"
    write_png(png_path, pix)
    write_xpm(root / "natronplusIcon.xpm", pix)
    write_ico(root / "natronplusIcon256.ico", png_path.read_bytes())
    print(f"wrote icons in {root}")


if __name__ == "__main__":
    main()
