# Slice 2 headless golden: NatronRenderer on a tiny project.
# Pixel-file hashes need OFX Read/Write plugins (not in this compile).
# This asserts a non-zero frame range, a built-in Dot (Viewer is skipped
# in --background), a saved .ntp, and a content hash.
# Invoked as: NatronRenderer -t tools/golden/golden_render.py
# Ends with os._exit so interpreter mode does not hang in Py_Main.

from __future__ import print_function

import hashlib
import os
import sys
import tempfile

PLUGIN_DOT = "io.github.joeb0611.built-in.Dot"
PLUGIN_VIEWER = "io.github.joeb0611.built-in.Viewer"


def fail(msg):
    print("GOLDEN_FAIL: " + msg, file=sys.stderr)
    sys.stderr.flush()
    sys.stdout.flush()
    os._exit(1)


def main():
    try:
        dot = app.createNode(PLUGIN_DOT)
    except Exception as exc:
        fail("createNode(Dot) raised: %s" % exc)
    if dot is None:
        fail("createNode(Dot) returned None")

    try:
        viewer = app.createNode(PLUGIN_VIEWER)
        if viewer is None:
            print("GOLDEN_WARN: Viewer skipped in background mode (expected)", file=sys.stderr)
    except Exception as exc:
        print("GOLDEN_WARN: Viewer create raised in background: %s" % exc, file=sys.stderr)

    fr = app.getProjectParam("frameRange")
    if fr is not None:
        try:
            fr.setValue(1, 0)
            fr.setValue(3, 1)
        except Exception as exc:
            print("GOLDEN_WARN: could not set frameRange: %s" % exc, file=sys.stderr)

    first = app.timelineGetLeftBound()
    last = app.timelineGetRightBound()
    nframes = last - first + 1
    if nframes <= 0:
        fail("zero frames on the timeline (%s-%s)" % (first, last))

    out_dir = tempfile.mkdtemp(prefix="natronplus-golden-")
    ntp_path = os.path.join(out_dir, "golden.ntp")
    if not app.saveProject(ntp_path):
        fail("saveProject failed for %s" % ntp_path)
    if not os.path.isfile(ntp_path) or os.path.getsize(ntp_path) == 0:
        fail("saved .ntp is missing or empty")

    sha = hashlib.sha256()
    with open(ntp_path, "rb") as fh:
        sha.update(fh.read())
    digest = sha.hexdigest()

    print("GOLDEN_OK")
    print("GOLDEN_FRAMES=%d" % nframes)
    print("GOLDEN_NTP=%s" % ntp_path)
    print("GOLDEN_HASH=%s" % digest)
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(0)


main()
