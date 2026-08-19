#!/usr/bin/env python3
"""Run focused EVM-memory regressions through both native C backends.

The fixture is generated in the workspace temporary directory. The harness
asks EELS to build the matching Amsterdam block and reference output, so this
test does not retain or hand-author an expected state root.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EELS_REVISION = "3f2859dc162243997f9f460fea5451a7ce7011b5"
DEFAULT_EELS_ROOT = ROOT.parents[2] / "execution-specs" / ".worktrees" / "evm-sail-v062"
DEFAULT_EELS_PY = ROOT.parents[2] / "execution-specs" / ".venv" / "bin" / "python3"

SENDER = "0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b"
SECRET_KEY = "0x45a915e4d060149eb4365960e6a7a45f334393093061116b197e3240065ff2d8"
LOWER = 0x1000000000000000000000000000000000000000
PARENT = 0x2000000000000000000000000000000000000000
SUCCESS_CHILD = 0x3000000000000000000000000000000000000000
REVERTING_CHILD = 0x4000000000000000000000000000000000000000
SIBLING_CHECK = 0x5000000000000000000000000000000000000000
EMPTY_TARGET = 0x6000000000000000000000000000000000000000
CREATE_TARGET = 0x7000000000000000000000000000000000000000


def push(value: int, width: int | None = None) -> bytes:
    if value < 0:
        raise ValueError("EVM PUSH values must be non-negative")
    if width is None:
        width = max(1, (value.bit_length() + 7) // 8)
    if not 1 <= width <= 32 or value >= 1 << (8 * width):
        raise ValueError(f"value {value} does not fit PUSH{width}")
    return bytes([0x5F + width]) + value.to_bytes(width, "big")


def mstore8(offset: int, value: int) -> bytes:
    return push(value) + push(offset) + b"\x53"


def mload(offset: int) -> bytes:
    return push(offset) + b"\x51"


def call(address: int, input_offset: int = 0, input_size: int = 0) -> bytes:
    # CALL pops gas, address, value, input offset/size, output offset/size.
    return b"".join(
        (
            push(0),
            push(0),
            push(input_size),
            push(input_offset),
            push(0),
            push(address, 20),
            push(1_000_000, 3),
            b"\xf1",
        )
    )


def create(value: int, initcode_offset: int, initcode_size: int) -> bytes:
    # CREATE pops value, initcode offset, then initcode size.
    return b"".join(
        (
            push(initcode_size),
            push(initcode_offset),
            push(value),
            b"\xf0",
        )
    )


def store_boolean(slot: int) -> bytes:
    """Store the Boolean already on top of the stack in ``slot``."""
    return push(slot) + b"\x55"


def lower_access_code() -> bytes:
    """Grow high, then use lower MLOAD/MSTORE/KECCAK/CALL input ranges."""
    return b"".join(
        (
            mstore8(0x100, 0xAA),
            mload(0),
            b"\x50",  # POP
            push(0x1234),
            push(0x20),
            b"\x52",  # MSTORE
            push(0x20),
            push(0),
            b"\x20",  # KECCAK256
            b"\x50",
            call(EMPTY_TARGET, input_offset=0, input_size=0x20),
            b"\x50",
            b"\x00",  # STOP
        )
    )


def lower_create_code() -> bytes:
    """Grow high, then execute initcode from the established low prefix."""
    initcode = b"\x5f\x5f\xf3"  # PUSH0; PUSH0; RETURN (empty runtime code)
    return b"".join(
        (
            mstore8(0x100, 0xAA),
            *(mstore8(offset, byte) for offset, byte in enumerate(initcode)),
            create(0, 0, len(initcode)),
            b"\x15\x15",  # ISZERO twice: one exactly when CREATE succeeded
            store_boolean(0),
            b"\x00",
        )
    )


def parent_code() -> bytes:
    marker_word = 0xAA << 248
    return b"".join(
        (
            # Establish parent memory through byte 0x100.
            mstore8(0, 0xAA),
            mstore8(0x100, 0xDD),
            # A successful child writes at its frame-relative zero.
            call(SUCCESS_CHILD),
            store_boolean(0),
            # Returning from the child must preserve the parent's low memory.
            mload(0),
            push(marker_word, 32),
            b"\x14",  # EQ
            store_boolean(1),
            # The sibling starts at the same absolute base and must see zeros.
            call(SIBLING_CHECK),
            store_boolean(2),
            # Parent growth reclaims and zeroes the former child frame.
            mload(0x120),
            b"\x15",  # ISZERO
            store_boolean(3),
            # A reverting child must restore the same parent cursor/state.
            call(REVERTING_CHILD),
            b"\x15",
            store_boolean(4),
            mload(0),
            push(marker_word, 32),
            b"\x14",
            store_boolean(5),
            # Reclaim the reverting child's first word and verify zeroing.
            mload(0x140),
            b"\x15",
            store_boolean(6),
            b"\x00",
        )
    )


def account(code: bytes = b"") -> dict[str, object]:
    return {
        "nonce": "0x00",
        "balance": "0x00",
        "code": "0x" + code.hex(),
        "storage": {},
    }


def fixture_case(target: int, code_accounts: dict[int, bytes]) -> dict[str, object]:
    pre = {f"0x{address:040x}": account(code) for address, code in code_accounts.items()}
    pre[SENDER] = {
        "nonce": "0x00",
        "balance": "0x3635c9adc5dea00000",
        "code": "0x",
        "storage": {},
    }
    return {
        "_info": {
            "fixture_format": "state_test",
            "comment": "generated memory-frame regression",
        },
        "env": {
            "currentCoinbase": "0x2adc25665018aa1fe0e6bc666dac8fc2697ff9ba",
            "currentGasLimit": "0x02540be400",
            "currentNumber": "0x01",
            "currentTimestamp": "0x03e8",
            "currentRandom": "0x" + "00" * 32,
            "currentBaseFee": "0x07",
        },
        "pre": pre,
        "transaction": {
            "nonce": "0x00",
            "gasPrice": "0x0a",
            "gasLimit": ["0x989680"],
            "to": f"0x{target:040x}",
            "value": ["0x00"],
            "data": ["0x"],
            "sender": SENDER,
            "secretKey": SECRET_KEY,
        },
        # The harness needs only the indexed case selector here. EELS derives
        # the authoritative post-state and byte-exact stateless output.
        "post": {"Amsterdam": [{"indexes": {"data": 0, "gas": 0, "value": 0}, "state": {}}]},
        "config": {"chainid": "0x01"},
    }


def build_fixture() -> dict[str, object]:
    lower_accounts = {LOWER: lower_access_code()}
    create_accounts = {CREATE_TARGET: lower_create_code()}
    frame_accounts = {
        PARENT: parent_code(),
        SUCCESS_CHILD: mstore8(0, 0xBB) + b"\x00",
        REVERTING_CHILD: mstore8(0, 0xCC) + push(0) + push(0) + b"\xfd",
        SIBLING_CHECK: mload(0) + b"\x15" + store_boolean(0) + b"\x00",
    }
    return {
        "memory/lower-access-after-growth": fixture_case(LOWER, lower_accounts),
        "memory/lower-create-initcode-after-growth": fixture_case(CREATE_TARGET, create_accounts),
        "memory/nested-frames-and-resume": fixture_case(PARENT, frame_accounts),
    }


def harness_environment() -> dict[str, str]:
    environment = os.environ.copy()
    root = Path(environment.get("EXECSPECS_ROOT", DEFAULT_EELS_ROOT)).resolve()
    # Preserve the venv's python symlink: resolving it selects the underlying
    # base interpreter and drops the venv site-packages.
    interpreter = Path(os.path.abspath(environment.get("EXECSPECS_PY", DEFAULT_EELS_PY)))
    if not interpreter.is_file() or not (root / "src").is_dir():
        raise RuntimeError(
            "memory regressions require the compatible execution-specs checkout; "
            f"expected {root} with interpreter {interpreter}. Set EXECSPECS_ROOT "
            "and EXECSPECS_PY to a checkout at revision " + EELS_REVISION
        )
    try:
        revision = subprocess.run(
            ["git", "-C", str(root), "rev-parse", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError) as error:
        raise RuntimeError(f"cannot verify execution-specs provenance at {root}") from error
    if revision != EELS_REVISION:
        raise RuntimeError(
            f"incompatible execution-specs revision at {root}: got {revision}, "
            f"expected {EELS_REVISION}. Set EXECSPECS_ROOT and EXECSPECS_PY "
            "to the pinned v0.6.2-compatible checkout"
        )
    environment["EXECSPECS_ROOT"] = str(root)
    environment["EXECSPECS_PY"] = str(interpreter)
    return environment


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--build",
        action="append",
        choices=("standard", "optimized"),
        help="backend to test; repeat to select both (default: both)",
    )
    parser.add_argument("--no-rebuild", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    temp_root = Path(os.environ.get("AGENT_TMPDIR", ROOT / ".agent-tmp"))
    temp_root.mkdir(parents=True, exist_ok=True)
    builds = args.build or ["standard", "optimized"]
    environment = harness_environment()
    subprocess.run(
        [environment["EXECSPECS_PY"], "-m", "devtools.harness.v062_compat_probe"],
        cwd=ROOT,
        env=environment,
        check=True,
    )
    with tempfile.TemporaryDirectory(prefix="memory-frames.", dir=temp_root) as directory:
        fixture_path = Path(directory) / "memory_frames.json"
        fixture_path.write_text(json.dumps(build_fixture(), indent=2) + "\n")
        for build in builds:
            command = [
                sys.executable,
                "-m",
                "devtools.harness.cli",
                str(fixture_path),
                "--build",
                build,
                "--quiet",
            ]
            if not args.no_rebuild:
                command.append("--rebuild")
            if args.verbose:
                command.extend(("--verbose", "--debug"))
            subprocess.run(command, cwd=ROOT, env=environment, check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
