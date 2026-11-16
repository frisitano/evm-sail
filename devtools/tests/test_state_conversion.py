"""Ensure state-to-block synthesis preserves log, receipt, and state assertions."""

from __future__ import annotations

import argparse
import copy
import subprocess
from typing import Any, cast

from devtools.tests.test_memory_frames import LOWER, ROOT, fixture_case, harness_environment


def reference_checks() -> None:
    from execution_testing import Alloc, Environment, Transaction, forks
    from execution_testing.client_clis.clis.execution_specs import ExecutionSpecsTransitionTool
    from execution_testing.fixtures.state import StateFixture
    from execution_testing.specs.state import StateTest

    from devtools.harness.ssz_builder import _transaction, build_guest

    def state_case(
        code: bytes,
        zero_fee: bool = False,
        overridden_hash: bool = False,
        invalid_gas: bool = False,
    ) -> dict[str, Any]:
        fixture = fixture_case(LOWER, {LOWER: code})
        env = cast(dict[str, Any], fixture["env"]).copy()
        env.pop("currentRandom")
        env.pop("currentBaseFee")
        env["currentDifficulty"] = "0x020000"
        if overridden_hash:
            env["blockHashes"] = {"0x00": "0x" + "42" * 32}
        tx = cast(dict[str, Any], fixture["transaction"])
        if zero_fee:
            tx["gasPrice"] = "0x00"
        if invalid_gas:
            tx["gasLimit"] = ["0x01"]
        case = {
            "fork": "Berlin",
            "env": env,
            "pre": fixture["pre"],
            "tx": tx,
            "idx": {"gas": 0, "data": 0, "value": 0},
            "config": fixture["config"],
        }
        tool = ExecutionSpecsTransitionTool()
        try:
            transaction = Transaction.model_validate(cast(Any, _transaction)(case, 1))
            if invalid_gas:
                from execution_testing.exceptions import TransactionException

                transaction.error = TransactionException.INTRINSIC_GAS_TOO_LOW
                case["expected_exception"] = str(transaction.error)
            fixture = (
                StateTest(
                    fork=forks.Berlin,
                    env=Environment.model_validate(env),
                    pre=Alloc.model_validate(case["pre"]),
                    post=Alloc.model_validate({}),
                    tx=transaction,
                    chain_id=1,
                    is_exception_test=invalid_gas,
                )
                .generate(tool, StateFixture)
                .fixture
            )
        finally:
            tool.shutdown()
        post = next(iter(fixture.post.values()))[0]
        case.update(
            expected_post=post.state.model_dump(mode="json", by_alias=True),
            expected_state_root=str(post.state_root),
            expected_logs_hash=str(post.logs_hash),
            expected_receipt=(
                post.receipt.model_dump(mode="json", by_alias=True) if post.receipt else None
            ),
        )
        return case

    control = state_case(b"\x00", zero_fee=True)
    _, output = cast(Any, build_guest)(control)
    assert output[32] == 1
    print("PASS state conversion control", flush=True)

    _, output = cast(Any, build_guest)(state_case(b"\x00", invalid_gas=True))
    assert output[32] == 0
    print("PASS rejected pre-Paris transaction earns no mining reward", flush=True)

    def rejected(case: dict[str, Any], reason: str) -> None:
        try:
            cast(Any, build_guest)(case)
        except ValueError as error:
            assert reason in str(error), error
        else:
            raise AssertionError(f"conversion did not preserve {reason}")
        print(f"PASS rejected changed {reason}", flush=True)

    # Both executions leave identical persistent state, but log different
    # ancestor hashes. A post-state comparison alone cannot catch this.
    rejected(
        state_case(bytes.fromhex("60004060005260206000a000"), overridden_hash=True),
        "transaction logs",
    )
    # With zero fees, STOP versus REVERT changes the receipt while leaving
    # persistent state and logs unchanged.
    code = bytes.fromhex("6000407f" + "42" * 32 + "1461002e5760006000fd5b00")
    rejected(state_case(code, zero_fee=True, overridden_hash=True), "transaction receipt")
    altered = copy.deepcopy(control)
    altered["expected_receipt"]["status"] = not altered["expected_receipt"]["status"]
    rejected(altered, "original fixture receipt")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference-check", action="store_true")
    args = parser.parse_args()
    if args.reference_check:
        reference_checks()
    else:
        environment = harness_environment()
        subprocess.run(
            [
                environment["EXECSPECS_PY"],
                "-m",
                "devtools.tests.test_state_conversion",
                "--reference-check",
            ],
            cwd=ROOT,
            env=environment,
            check=True,
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
