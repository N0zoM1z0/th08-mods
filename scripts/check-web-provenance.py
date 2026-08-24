#!/usr/bin/env python3
"""Reject tracked retail game payloads from the Web-port source tree.

This is a distribution boundary, not a license detector. It prevents the
known required TH08 archives, original executables, and common retail archive
containers from entering the Git tree. Browser runtime data must remain a
user-selected local File and must never be copied into a deployable directory.
"""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN_NAMES = {
    "th08.dat",
    "thbgm.dat",
    "th08.exe",
    "custom.exe",
    "replayview.exe",
}
FORBIDDEN_SUFFIXES = {".dat", ".exe", ".dll", ".zip", ".rar", ".7z", ".iso", ".cab"}


def tracked_paths() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [Path(raw.decode()) for raw in result.stdout.split(b"\0") if raw]


def main() -> int:
    violations: list[str] = []
    for path in tracked_paths():
        name = path.name.lower()
        if name in FORBIDDEN_NAMES or path.suffix.lower() in FORBIDDEN_SUFFIXES:
            violations.append(path.as_posix())

    if violations:
        print("error: tracked retail/archive payloads violate Web provenance:", file=sys.stderr)
        for path in violations:
            print(f"  {path}", file=sys.stderr)
        print(
            "Supply th08.dat and thbgm.dat only through the browser's local file picker.",
            file=sys.stderr,
        )
        return 1

    print("Web provenance check passed: no forbidden retail/archive payload is tracked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
