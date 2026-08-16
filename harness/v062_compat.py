"""Narrow SSZ adapter from pinned EELS Amsterdam to tests-zkevm v0.6.2.

The pinned EELS revision has the three pre-EIP-8282 execution-request lists.
The retained v0.6.2 fixtures and EVM Sail decoder have two additional builder
request lists. State-test synthesis cannot produce either new request, so this
module appends exactly two empty lists. EELS still independently executes the
legacy-schema input through run_stateless_guest; only the expanded request
root is recomputed for the v0.6.2 output container.
"""

from remerkleable.basic import boolean, uint64
from remerkleable.byte_arrays import Bytes32, ByteVector
from remerkleable.complex import Container
from remerkleable.complex import List as SszList

from ethereum.forks.amsterdam.stateless_guest import run_stateless_guest
from ethereum.forks.amsterdam.stateless_host import (
    deserialize_stateless_output,
    serialize_stateless_input,
)
from ethereum.forks.amsterdam.stateless_ssz import (
    MAX_BLOB_COMMITMENTS_PER_BLOCK,
    MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD,
    MAX_DEPOSIT_REQUESTS_PER_PAYLOAD,
    MAX_PUBLIC_KEYS,
    MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD,
    PUBLIC_KEY_BYTES,
    STATELESS_INPUT_SCHEMA_ID_BYTES,
    SszChainConfig,
    SszConsolidationRequest,
    SszDepositRequest,
    SszExecutionPayload,
    SszExecutionWitness,
    SszStatelessValidationResult,
    SszWithdrawalRequest,
    stateless_input_to_ssz,
)


MAX_BUILDER_DEPOSIT_REQUESTS = 2**6
MAX_BUILDER_EXIT_REQUESTS = 2**4


class SszBuilderDepositRequest(Container):
    pubkey: ByteVector[48]
    withdrawal_credentials: Bytes32
    amount: uint64
    signature: ByteVector[96]


class SszBuilderExitRequest(Container):
    source_address: ByteVector[20]
    pubkey: ByteVector[48]


class SszExecutionRequestsV062(Container):
    deposits: SszList[SszDepositRequest, MAX_DEPOSIT_REQUESTS_PER_PAYLOAD]
    withdrawals: SszList[SszWithdrawalRequest, MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD]
    consolidations: SszList[
        SszConsolidationRequest, MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD
    ]
    builder_deposits: SszList[SszBuilderDepositRequest, MAX_BUILDER_DEPOSIT_REQUESTS]
    builder_exits: SszList[SszBuilderExitRequest, MAX_BUILDER_EXIT_REQUESTS]


class SszNewPayloadRequestV062(Container):
    execution_payload: SszExecutionPayload
    versioned_hashes: SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK]
    parent_beacon_block_root: Bytes32
    execution_requests: SszExecutionRequestsV062


class SszStatelessInputV062(Container):
    new_payload_request: SszNewPayloadRequestV062
    witness: SszExecutionWitness
    chain_config: SszChainConfig
    public_keys: SszList[ByteVector[PUBLIC_KEY_BYTES], MAX_PUBLIC_KEYS]


def expand_execution_requests(base_requests):
    """Preserve EELS requests and append the two known-empty v0.6.2 lists."""
    return SszExecutionRequestsV062(
        deposits=base_requests.deposits,
        withdrawals=base_requests.withdrawals,
        consolidations=base_requests.consolidations,
        builder_deposits=SszList[
            SszBuilderDepositRequest, MAX_BUILDER_DEPOSIT_REQUESTS
        ](),
        builder_exits=SszList[SszBuilderExitRequest, MAX_BUILDER_EXIT_REQUESTS](),
    )


def reference_validation(encoded_legacy_input, runner=run_stateless_guest):
    """Run and decode the independent EELS guest oracle."""
    return deserialize_stateless_output(runner(encoded_legacy_input))


def serialize_v062_pair(stateless_input, transition_success, protocol_fork):
    """Return expanded input plus output carrying the EELS guest verdict."""
    base = stateless_input_to_ssz(stateless_input)
    reference = reference_validation(serialize_stateless_input(stateless_input))
    if bool(reference.successful_validation) != transition_success:
        raise RuntimeError(
            "EELS transition and run_stateless_guest validation disagree"
        )

    base_request = base.new_payload_request
    new_payload_request = SszNewPayloadRequestV062(
        execution_payload=base_request.execution_payload,
        versioned_hashes=base_request.versioned_hashes,
        parent_beacon_block_root=base_request.parent_beacon_block_root,
        execution_requests=expand_execution_requests(base_request.execution_requests),
    )
    encoded_input = SszStatelessInputV062(
        new_payload_request=new_payload_request,
        witness=base.witness,
        chain_config=base.chain_config,
        public_keys=base.public_keys,
    )
    encoded_output = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(bytes(new_payload_request.hash_tree_root())),
        successful_validation=boolean(reference.successful_validation),
        chain_config=base.chain_config,
    )
    schema = bytearray(STATELESS_INPUT_SCHEMA_ID_BYTES)
    schema[0] = int(protocol_fork)
    return (
        bytes(schema) + bytes(encoded_input.encode_bytes()),
        bytes(encoded_output.encode_bytes()),
    )
