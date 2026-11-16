"""Provision pinned EELS, fill, export, audit and replay all historical state tests."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from devtools.harness.ssz_builder import EELS_REVISION
from devtools.paths import REPO_ROOT


def run(command: list[str], *, cwd: Path, env: dict[str, str], log: Path) -> int:
    print(f"{cwd}: {' '.join(command)} -> {log}", flush=True)
    with log.open("w") as output:
        return subprocess.run(
            command, cwd=cwd, env=env, stdout=output, stderr=subprocess.STDOUT
        ).returncode


def require(command: list[str], *, cwd: Path, env: dict[str, str], log: Path) -> None:
    if run(command, cwd=cwd, env=env, log=log):
        raise RuntimeError(f"command failed; see {log}")


def tree_digest(directory: Path, pattern: str) -> str:
    digest = hashlib.sha256()
    for path in sorted(directory.rglob(pattern)):
        digest.update(str(path.relative_to(directory)).encode())
        digest.update(path.read_bytes())
    return digest.hexdigest()


def verify_audit(report: dict[str, object], group: str, limits: dict[str, object]) -> None:
    """Conversion limits never excuse a mismatch, duplicate or changed inventory."""
    expected = limits[group]
    assert isinstance(expected, dict)
    missing = report["missing"]
    assert isinstance(missing, list)
    actual = sorted((Path(source).name, case) for source, case in missing)
    allowed = sorted((row["file"], row["case"]) for row in expected["conversion_limits"])
    if (
        report["failures"]
        or report["duplicates"]
        or actual != allowed
        or report["expected_cases"] != expected["source_cases"]
        or report["exported_rows"] != expected["source_cases"] - len(allowed)
    ):
        raise RuntimeError(f"{group}: historical inventory/provenance audit failed")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--directory", type=Path, default=REPO_ROOT / ".agent-tmp/historical-suite")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    root = args.directory.resolve()
    root.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ)
    specs = root / "execution-specs"
    if not specs.exists():
        require(
            [
                "git",
                "-c",
                "core.hooksPath=/dev/null",
                "clone",
                "--no-checkout",
                "https://github.com/ethereum/execution-specs.git",
                str(specs),
            ],
            cwd=root,
            env=env,
            log=root / "clone.log",
        )
        require(
            ["git", "-c", "core.hooksPath=/dev/null", "checkout", "--detach", EELS_REVISION],
            cwd=specs,
            env=env,
            log=root / "checkout.log",
        )
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=specs, text=True).strip()
    dirty = subprocess.check_output(
        ["git", "status", "--porcelain", "--untracked-files=no"], cwd=specs, text=True
    ).strip()
    if revision != EELS_REVISION or dirty:
        raise RuntimeError(f"{specs}: expected clean pinned checkout {EELS_REVISION}")
    env.pop("UV_PROJECT_ENVIRONMENT", None)
    require(
        ["uv", "sync", "--frozen", "--group", "test", "--python", "3.12"],
        cwd=specs,
        env=env,
        log=root / "environment.log",
    )
    python = specs / ".venv/bin/python"
    env.update(
        EXECSPECS_ROOT=str(specs),
        EXECSPECS_PY=str(python),
        PYTHONPATH=str(REPO_ROOT),
        EVM_NATIVE_RESOURCES="historical",
    )
    # Native library existence is not a source-freshness check. Rebuild once
    # per backend in the parent before workers reuse it, independently of
    # the reference-fixture cache.
    for backend in ("standard", "optimized"):
        require(
            [
                sys.executable,
                "-c",
                "import sys; from devtools.harness.guest import load_guest; "
                "load_guest(rebuild=True, build_mode=sys.argv[1])",
                backend,
            ],
            cwd=REPO_ROOT,
            env=env,
            log=root / f"{backend}-build.log",
        )
    limits = json.loads(
        (REPO_ROOT / "config/corpora/historical-conversion-limits.json").read_text()
    )
    if limits["eels_revision"] != EELS_REVISION:
        raise RuntimeError("conversion inventory must match the pinned EELS revision")
    identity = {
        "eels_revision": EELS_REVISION,
        "harness": tree_digest(REPO_ROOT / "devtools/harness", "*.py"),
        "selection": hashlib.sha256(
            (REPO_ROOT / "devtools/tests/full_fork_selection.py").read_bytes()
        ).hexdigest(),
        "limits": limits,
    }
    key = hashlib.sha256(json.dumps(identity, sort_keys=True).encode()).hexdigest()
    cache = root / key
    cache.mkdir(exist_ok=True)
    for group, selection in (
        ("main", ["--from", "Berlin", "--until", "Amsterdam"]),
        (
            "extra",
            [
                "-p",
                "devtools.tests.full_fork_selection",
                "--fork",
                "ArrowGlacier,GrayGlacier,BPO1,BPO2",
            ],
        ),
    ):
        completed = cache / f"{group}.json"
        if completed.exists():
            saved = json.loads(completed.read_text())
            fixture_dir = Path(saved["directory"])
            if (
                tree_digest(fixture_dir / "source/state_tests", "*.json") != saved["sources"]
                or tree_digest(fixture_dir / "ssz", "*.jsonl") != saved["exports"]
            ):
                raise RuntimeError(f"{completed}: cached fixtures changed; use a new --directory")
        else:
            # Failed attempts remain available for inspection; never append to old exports.
            fixture_dir = Path(tempfile.mkdtemp(prefix=f"{group}-", dir=cache))
            require(
                [
                    str(specs / ".venv/bin/fill"),
                    *selection,
                    "-m",
                    "state_test",
                    "-n",
                    str(args.jobs),
                    "--no-html",
                    "--output",
                    str(fixture_dir / "source"),
                    "tests",
                ],
                cwd=specs,
                env=env,
                log=fixture_dir / "fill.log",
            )
            status = run(
                [
                    sys.executable,
                    "-m",
                    "devtools.harness.cli",
                    "--build",
                    "optimized",
                    "--jobs",
                    str(args.jobs),
                    "--quiet",
                    "--save-synthesized",
                    str(fixture_dir / "ssz"),
                    str(fixture_dir / "source/state_tests"),
                ],
                cwd=REPO_ROOT,
                env=env,
                log=fixture_dir / "export.log",
            )
            if status not in (0, 1):
                raise RuntimeError(f"{group}: export crashed; inspect {fixture_dir}")
        report = fixture_dir / "audit.json"
        status = run(
            [
                str(python),
                "-m",
                "devtools.tests.audit_synthesized",
                str(fixture_dir / "ssz"),
                "--sources",
                str(fixture_dir / "source/state_tests"),
                "--report",
                str(report),
            ],
            cwd=REPO_ROOT,
            env=env,
            log=fixture_dir / "audit.log",
        )
        if status not in (0, 1):
            raise RuntimeError(f"{group}: audit crashed; inspect {fixture_dir}")
        verify_audit(json.loads(report.read_text()), group, limits)
        # Replay every export, even after a cached generation. No model failure is allowed.
        for backend in ("standard", "optimized"):
            require(
                [
                    sys.executable,
                    "-m",
                    "devtools.harness.cli",
                    "--build",
                    backend,
                    "--jobs",
                    str(args.jobs),
                    "--quiet",
                    str(fixture_dir / "ssz"),
                ],
                cwd=REPO_ROOT,
                env=env,
                log=fixture_dir / f"{backend}.log",
            )
        completed.write_text(
            json.dumps(
                {
                    "directory": str(fixture_dir),
                    "identity": identity,
                    "sources": tree_digest(fixture_dir / "source/state_tests", "*.json"),
                    "exports": tree_digest(fixture_dir / "ssz", "*.jsonl"),
                },
                indent=2,
            )
            + "\n"
        )
        print(f"{group}: full replay passed; conversion limits listed in {report}", flush=True)


if __name__ == "__main__":
    main()
