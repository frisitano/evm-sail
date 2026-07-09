"""EEST `consume direct` fixture consumer for the evm-sail zkVM stateless guest.

Registers an `evm-sail` FixtureConsumerTool with the execution-spec-tests
`consume` framework, so the canonical stateless conformance fixtures
(tests-zkevm, `blockchain_test` format) can be driven through the Sail stateless
guest with EEST doing fixture discovery, collection, and reporting.

For each block carrying `stateless_input_bytes`, we run the native guest binary
over those bytes and compare its canonical SSZ output (a hex line) against the
fixture's `stateless_output_bytes`. Output framing is identical between the two
(32-byte new_payload_request_root || successful_validation || ...), so the
comparison is a direct hex equality.

Registering this module (e.g. via a conftest that imports it) adds the tool to
`FixtureConsumerTool.registered_tools`; `consume direct --bin <guest>` then
selects it by matching the binary's `-v` banner against `detect_binary_pattern`.

The guest reserves a large main-thread stack and can be slow / non-terminating
on real fixtures, so every invocation is bounded by a hard timeout
(EVMSAIL_TIMEOUT seconds, default 60). A timeout is reported as a failure and
the child is killed, so it cannot accumulate.
"""

import os
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Any, List, Optional

import pytest
from execution_testing.client_clis.ethereum_cli import EthereumCLI
from execution_testing.client_clis.fixture_consumer_tool import (
    FixtureConsumerTool,
)
from execution_testing.fixtures import BlockchainFixture, FixtureFormat
from execution_testing.fixtures.file import Fixtures

TIMEOUT = float(os.environ.get("EVMSAIL_TIMEOUT", "60"))


class EvmSailEvm(EthereumCLI):
    """Binary locator for the evm-sail native stateless-guest runner."""

    default_binary = Path("zkvm_native")
    detect_binary_pattern = re.compile(r"^evm-sail\b")
    version_flag = "-v"
    cached_version: Optional[str] = None

    def __init__(self, binary: Optional[Path] = None, trace: bool = False):
        """Store the resolved binary path (trace is unused by this guest)."""
        self.binary = binary if binary else self.default_binary
        self.trace = trace


class EvmSailFixtureConsumer(
    EvmSailEvm,
    FixtureConsumerTool,
    fixture_formats=[BlockchainFixture],
):
    """Drive the evm-sail stateless guest over `blockchain_test` fixtures."""

    def consume_fixture(
        self,
        fixture_format: FixtureFormat,
        fixture_path: Path,
        fixture_name: Optional[str] = None,
        debug_output_path: Optional[Path] = None,
    ) -> None:
        """Run the named fixture's stateless blocks through the guest."""
        if fixture_format != BlockchainFixture:
            raise Exception(f"unsupported fixture format: {fixture_format}")

        fixtures = Fixtures.model_validate_json(Path(fixture_path).read_text())
        names = [fixture_name] if fixture_name else list(fixtures.root.keys())

        checked = 0
        for name in names:
            fixture = fixtures[name]
            for i, block in enumerate(getattr(fixture, "blocks", []) or []):
                sib = getattr(block, "stateless_input_bytes", None)
                if sib is None:
                    continue
                got = self._run_guest(
                    bytes(sib), debug_output_path, f"{name} block {i}"
                )
                want = getattr(block, "stateless_output_bytes", None)
                if want is not None:
                    want_hex = bytes(want).hex()
                    if got.lower() != want_hex.lower():
                        raise Exception(
                            f"stateless output mismatch [{name} block {i}]:\n"
                            f"  got  {got}\n"
                            f"  want {want_hex}"
                        )
                checked += 1

        if checked == 0:
            pytest.skip("fixture carries no stateless_input_bytes")

    def _run_guest(
        self,
        input_bytes: bytes,
        debug_output_path: Optional[Path],
        label: str,
    ) -> str:
        """Run the guest over one SSZ input, returning its hex output line."""
        with tempfile.NamedTemporaryFile(suffix=".ssz", delete=False) as tf:
            tf.write(input_bytes)
            tpath = tf.name
        try:
            try:
                result = subprocess.run(
                    [str(self.binary), tpath],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    timeout=TIMEOUT,
                )
            except subprocess.TimeoutExpired:
                raise Exception(
                    f"evm-sail guest TIMEOUT after {TIMEOUT:g}s [{label}]"
                )
        finally:
            os.unlink(tpath)

        if debug_output_path:
            Path(debug_output_path).mkdir(parents=True, exist_ok=True)
            (Path(debug_output_path) / "stdout.txt").write_bytes(result.stdout)
            (Path(debug_output_path) / "stderr.txt").write_bytes(result.stderr)

        if result.returncode != 0:
            raise Exception(
                f"evm-sail guest exit {result.returncode} [{label}]\n"
                f"stderr: {result.stderr.decode(errors='replace')[-300:]}"
            )
        out = result.stdout.decode(errors="replace").strip()
        if not out:
            raise Exception(f"evm-sail guest produced no output [{label}]")
        return out


# Importing this module is enough to register the tool. Re-export the consumer
# class name so a conftest can `from evm_sail_consumer import EvmSailFixtureConsumer`.
__all__ = ["EvmSailFixtureConsumer", "EvmSailEvm"]
