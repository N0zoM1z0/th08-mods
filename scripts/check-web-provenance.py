#!/usr/bin/env python3
"""Reject retail game payloads from Web-port source and staged artifacts.

This is a distribution boundary, not a license detector. It prevents the
known required TH08 archives, original executables, and common retail archive
containers from entering the Git tree. Browser runtime data must remain a
user-selected local File and must never be copied into a deployable directory.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
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
WEB_SHELL = ROOT / "src" / "modern" / "web" / "shell.html"
ALLOWED_WEB_ARTIFACTS = {
    "_headers",
    "_redirects",
    "th08-web.html",
    "th08-web.js",
    "th08-web.wasm",
    "th08-web-icon.png",
}


def tracked_paths() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [Path(raw.decode()) for raw in result.stdout.split(b"\0") if raw]


def forbidden(path: Path) -> bool:
    return path.name.lower() in FORBIDDEN_NAMES or path.suffix.lower() in FORBIDDEN_SUFFIXES


def artifact_paths(root: Path) -> list[Path]:
    if not root.exists():
        return []
    if not root.is_dir():
        raise ValueError(f"artifact path is not a directory: {root}")
    return [path for path in root.rglob("*") if path.is_file() or path.is_symlink()]


def save_boundary_violations() -> list[str]:
    if not WEB_SHELL.is_file():
        return [f"missing Web launcher: {WEB_SHELL.relative_to(ROOT)}"]

    source = WEB_SHELL.read_text(encoding="utf-8")
    violations: list[str] = []
    if re.search(r'Module\.FS\.mount\s*\([^;]*?["\']/game["\']', source, re.DOTALL):
        violations.append("persistent or external filesystems must never be mounted at the /game root")
    has_save_mount = re.search(
        r'const\s+persistenceMounts\s*=\s*\[\s*["\']/save["\']', source, re.DOTALL
    ) and re.search(
        r'Module\.FS\.mount\s*\(\s*Module\.IDBFS\s*,.*?,\s*mount\s*\)',
        source,
        re.DOTALL,
    )
    if not has_save_mount:
        violations.append("the browser save filesystem must be isolated at /save")

    for declaration in ("persistentFiles", "persistentDirectories"):
        match = re.search(rf"const\s+{declaration}\s*=\s*\[(.*?)\];", source, re.DOTALL)
        if match is None:
            violations.append(f"missing explicit {declaration} allowlist")
            continue
        names = re.findall(r'["\']([^"\']+)["\']', match.group(1))
        forbidden_names = sorted(name for name in names if name.lower() in FORBIDDEN_NAMES)
        if forbidden_names:
            violations.append(
                f"{declaration} contains forbidden retail names: {', '.join(forbidden_names)}"
            )

    if re.search(
        r'Module\.FS\.writeFile\s*\(\s*["\']/save/(?:th08|thbgm)\.dat["\']',
        source,
        re.IGNORECASE,
    ):
        violations.append("the launcher writes a retail archive name into /save")
    return violations


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--artifact",
        action="append",
        type=Path,
        default=[],
        help="also inspect a staged Web artifact directory (repeatable)",
    )
    args = parser.parse_args()

    tracked_violations = [path.as_posix() for path in tracked_paths() if forbidden(path)]
    boundary_violations = save_boundary_violations()
    artifact_roots = args.artifact
    default_artifact = ROOT / "build" / "web-dist"
    if not artifact_roots and default_artifact.exists():
        artifact_roots = [default_artifact]

    artifact_violations: list[str] = []
    try:
        for root in artifact_roots:
            resolved = root if root.is_absolute() else ROOT / root
            for path in artifact_paths(resolved):
                relative = path.relative_to(resolved).as_posix()
                if path.is_symlink():
                    artifact_violations.append(f"{path} (symbolic links are not deployable)")
                elif relative not in ALLOWED_WEB_ARTIFACTS:
                    artifact_violations.append(f"{path} (not in the Web artifact allowlist)")
                elif forbidden(path):
                    artifact_violations.append(str(path))
            missing = sorted(ALLOWED_WEB_ARTIFACTS - {
                path.relative_to(resolved).as_posix()
                for path in artifact_paths(resolved)
                if path.is_file() and not path.is_symlink()
            })
            artifact_violations.extend(
                f"{resolved / name} (required Web artifact is missing)" for name in missing
            )
    except ValueError as error:
        parser.error(str(error))

    if tracked_violations or artifact_violations or boundary_violations:
        if tracked_violations:
            print("error: tracked retail/archive payloads violate Web provenance:", file=sys.stderr)
            for path in tracked_violations:
                print(f"  {path}", file=sys.stderr)
        if artifact_violations:
            print("error: deployable Web artifact boundary violations:", file=sys.stderr)
            for path in artifact_violations:
                print(f"  {path}", file=sys.stderr)
        if boundary_violations:
            print("error: browser save isolation violates Web provenance:", file=sys.stderr)
            for violation in boundary_violations:
                print(f"  {violation}", file=sys.stderr)
        print(
            "Supply th08.dat and thbgm.dat only through the browser's local file picker.",
            file=sys.stderr,
        )
        return 1

    if artifact_roots:
        print("Web provenance check passed: source and staged artifacts contain no forbidden payloads")
    else:
        print("Web provenance check passed: no forbidden retail/archive payload is tracked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
