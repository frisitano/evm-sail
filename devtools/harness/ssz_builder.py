#!/usr/bin/env python3
"""Synthesize v21 stateless pairs with the pinned EELS blockchain filler.

Run under the execution-specs test environment. EELS owns predeploys, parent
headers, signing, witnesses, BALs, execution, and SSZ serialization. No output
commitment or verdict is rewritten by this adapter.
"""

import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

from devtools.paths import REPO_ROOT

EELS_REVISION = "59c1473ddfba341babaeca1c97a50e996f639a0b"
EXECSPECS_ROOT = Path(os.environ.get("EXECSPECS_ROOT", REPO_ROOT.parent / "execution-specs"))
_verified = False


def _verify_source():
    global _verified
    if _verified:
        return
    revision = subprocess.check_output(
        ["git", "-C", str(EXECSPECS_ROOT), "rev-parse", "HEAD"], text=True
    ).strip()
    if revision != EELS_REVISION:
        raise RuntimeError(f"execution-specs must be {EELS_REVISION}, got {revision}")
    import ethereum
    import execution_testing

    for module in (ethereum, execution_testing):
        if not Path(module.__file__).resolve().is_relative_to(EXECSPECS_ROOT.resolve()):
            raise RuntimeError(f"{module.__name__} is not loaded from pinned execution-specs")
    _verified = True


def _chain_id(case: dict[str, Any]) -> int:
    value = case.get("config", {}).get("chainid", case["env"].get("currentChainId", 1))
    return int(value, 16) if isinstance(value, str) else int(value)


def _transaction(case: dict[str, Any], chain_id: int) -> dict[str, Any]:
    tx, idx = case["tx"], case["idx"]
    values = {
        "nonce": tx["nonce"],
        "gasLimit": tx["gasLimit"][idx["gas"]],
        "data": tx["data"][idx["data"]],
        "to": tx.get("to") or None,
        "value": tx["value"][idx["value"]],
        "chainId": chain_id,
    }
    for key in (
        "gasPrice",
        "maxFeePerGas",
        "maxPriorityFeePerGas",
        "maxFeePerBlobGas",
        "blobVersionedHashes",
        "authorizationList",
    ):
        if key in tx:
            values[key] = tx[key]
    if "accessLists" in tx:
        values["accessList"] = tx["accessLists"][idx["data"]]
    if case.get("txbytes"):
        raise ValueError("raw signed state-test synthesis is not yet supported by the v21 filler")
    values["secretKey"] = tx["secretKey"]
    return values


def build_guest(case):
    """Return the independent reference's unmodified input/output bytes."""
    _verify_source()
    from execution_testing import Alloc, Environment, Transaction
    from execution_testing.client_clis.clis.execution_specs import ExecutionSpecsTransitionTool
    from execution_testing.fixtures.blockchain import BlockchainFixture
    from execution_testing.forks import Amsterdam
    from execution_testing.specs.state import StateTest

    if case["fork"] != "Amsterdam":
        raise ValueError("v21 stateless synthesis currently supports Amsterdam only")
    chain_id = _chain_id(case)
    test = StateTest(
        fork=Amsterdam,
        env=Environment.model_validate(case["env"]),
        pre=Alloc.model_validate(case["pre"]),
        post=Alloc.model_validate(case.get("expected_post", {})),
        tx=Transaction.model_validate(_transaction(case, chain_id)),
        chain_id=chain_id,
    )
    tool = ExecutionSpecsTransitionTool()
    try:
        fixture = test.generate(tool, BlockchainFixture).fixture
        if len(fixture.blocks) != 1:
            raise RuntimeError("state-test synthesis must produce exactly one block")
        block = fixture.blocks[0]
        if block.stateless_input_bytes is None or block.stateless_output_bytes is None:
            raise RuntimeError("EELS did not produce a stateless reference pair")
        return bytes(block.stateless_input_bytes), bytes(block.stateless_output_bytes)
    finally:
        tool.shutdown()


def _serve():
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            inp, exp = build_guest(json.loads(line))
            response = {"input": inp.hex(), "expected": exp.hex()}
        except Exception as error:
            response = {"err": f"{type(error).__name__}: {error}"}
        print(json.dumps(response), flush=True)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--serve":
        _serve()
    else:
        inp, exp = build_guest(json.loads(Path(sys.argv[1]).read_text()))
        print(json.dumps({"input": inp.hex(), "expected": exp.hex()}))
