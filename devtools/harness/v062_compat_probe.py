#!/usr/bin/env python3
"""Focused structural and oracle checks for the v0.6.2 SSZ adapter."""

import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXECSPECS_ROOT = Path(os.environ["EXECSPECS_ROOT"])
sys.path.insert(0, str(EXECSPECS_ROOT / "src"))

from ethereum.forks.amsterdam.stateless_ssz import (  # noqa: E402
    MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD,
    MAX_DEPOSIT_REQUESTS_PER_PAYLOAD,
    MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD,
    SszChainConfig,
    SszConsolidationRequest,
    SszDepositRequest,
    SszForkActivation,
    SszForkConfig,
    SszStatelessValidationResult,
    SszWithdrawalRequest,
)
from remerkleable.basic import boolean, uint64  # noqa: E402
from remerkleable.byte_arrays import Bytes32  # noqa: E402
from remerkleable.complex import List as SszList  # noqa: E402

from devtools.harness.v062_compat import (  # noqa: E402
    expand_execution_requests,
    reference_validation,
)


def main() -> int:
    legacy_requests = type("LegacyRequests", (), {})()
    legacy_requests.deposits = SszList[SszDepositRequest, MAX_DEPOSIT_REQUESTS_PER_PAYLOAD]()
    legacy_requests.withdrawals = SszList[
        SszWithdrawalRequest, MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD
    ]()
    legacy_requests.consolidations = SszList[
        SszConsolidationRequest, MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD
    ]()
    expanded = expand_execution_requests(legacy_requests)
    encoded_requests = bytes(expanded.encode_bytes())
    assert len(encoded_requests) == 20
    assert encoded_requests == (20).to_bytes(4, "little") * 5

    empty_optional = SszList[uint64, 1]()
    chain_config = SszChainConfig(
        chain_id=uint64(1),
        active_fork=SszForkConfig(
            activation=SszForkActivation(
                block_number=empty_optional,
                timestamp=SszList[uint64, 1](uint64(0)),
            )
        ),
    )
    oracle_output = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(b"\x42" * 32),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    sentinel_input = b"legacy-schema-input"

    def oracle(data):
        assert bytes(data) == sentinel_input
        return bytes(oracle_output.encode_bytes())

    decoded = reference_validation(sentinel_input, runner=oracle)
    assert decoded.successful_validation
    assert bytes(decoded.new_payload_request_root) == b"\x42" * 32
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
