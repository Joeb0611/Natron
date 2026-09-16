# Upstream merge-base

Natron+ is an independent fork of [Natron](https://github.com/NatronGitHub/Natron).

| Field | Value |
| --- | --- |
| Upstream remote | `https://github.com/NatronGitHub/Natron` |
| Upstream branch | `RB-2.6` (upstream default at fork time) |
| Upstream commit | [`3763d805d7d277d10af10025ae41af677682b3e6`](https://github.com/NatronGitHub/Natron/commit/3763d805d7d277d10af10025ae41af677682b3e6) |
| Commit date | 2026-07-24 |
| Commit summary | `ci(windows): resolve pacman x264 file conflict with --overwrite` |
| Natron+ default branch | `main` (created from that SHA) |

Keep `upstream` as a read-only remote for optional cherry-picks. We are not a tracking fork by default.

Submodules remain pinned to the revisions recorded in this commit (`libs/OpenFX`, `libs/google-breakpad`, `libs/SequenceParsing`, `Tests/google-test`, `Tests/google-mock`). Plugin repos (`openfx-io`, `openfx-misc`, `openfx-arena`, `openfx-gmic`, `OpenColorIO-Configs`) are not forked in Slice 1.
