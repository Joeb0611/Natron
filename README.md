# Natron+

Natron+ is an **independent, GPLv2-or-later fork** of [Natron](https://github.com/NatronGitHub/Natron). The product goal is a node-based compositor that artists can trust on a deadline: more stable and robust than Nuke, without becoming a feature clone of Nuke, Fusion, or After Effects.

This is **not** an official Natron Project build, organization, or release. Credit and source: [natrongithub.github.io](https://natrongithub.github.io). Brand assets of the Natron Project are used here only to *refer to* Natron.

[![GPL2 License](https://img.shields.io/badge/license-GPLv2%2B-blue.svg)](LICENSE.txt)

## Fork origin

| | |
| --- | --- |
| Upstream | https://github.com/NatronGitHub/Natron |
| Branched from | `RB-2.6` at [`3763d805d7d277d10af10025ae41af677682b3e6`](https://github.com/NatronGitHub/Natron/commit/3763d805d7d277d10af10025ae41af677682b3e6) (2026-07-24) |
| Our default branch | `main` |
| Record | [UPSTREAM.md](UPSTREAM.md) |

Internal C++ names, `.ntp` project files, and OpenFX plugin IDs are unchanged so existing scripts keep loading. The product surface (app name, About, splash, icons, reverse-DNS, installer strings) is **Natron+**.

## What this tree is for

Stability first: crashes, hangs, silent render stalls, cache/data loss, and “won’t launch” beat new nodes. Application robustness work starts after this Slice 1 fork is the source of truth.

## Building

CMake is the build we run in CI. qmake remains in the tree for now.

- [GNU/Linux](INSTALL_LINUX.md)
- [macOS](INSTALL_MACOS.md)
- [FreeBSD](INSTALL_FREEBSD.md)
- [Windows](INSTALL_WINDOWS.md)

CI on this repository is **our** GitHub Actions. The first bar is CMake configure + build on Linux and Windows. Matching a four-year-old upstream test log is not success.

## License

GNU GPL v2 or later. See [LICENSE.txt](LICENSE.txt) and [LICENSE_SHORT.txt](LICENSE_SHORT.txt).

Copyright: (C) 2026 Natron+ contributors; (C) 2018–2023 The Natron developers; (C) 2013–2018 INRIA and Alexandre Gauthier-Foichat.

## Issues

Use [this repository’s issue tracker](https://github.com/Joeb0611/Natron/issues). Upstream contribution is optional and never blocks a Natron+ ship.
