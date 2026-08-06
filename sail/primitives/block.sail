
/*md
# Block types

Block-level data structures. [BlockHeader][type-BlockHeader] is pure data;
its `k_header` register lives in the kernel environment alongside the
EIP-4895 withdrawal record.
*/
/*! The 2048-bit logs bloom filter (YP §4.4.1), as 256 bytes. */
type LogsBloom = vector(256, dec, byte)

/*! The payload header's 2048-bit logs bloom in canonical SSZ wire order.
    Keeping the authenticated input range by reference lets consumers that
    already operate on bytes avoid an eager 256-byte materialization. */
type LogsBloomRef = StatelessInputSliceLength(256)

let EMPTY_LOGS_BLOOM : LogsBloom = vector_init(256, 0x00)

/*! Byte-wise bloom equality (the header `logs_bloom` check). */
function logs_bloom_equal(a : LogsBloom, b : LogsBloom) -> bool = {
    var equal : bool = true;
    foreach (i from 0 to 255) {
        equal = equal & (a[i] == b[i])
    };
    equal
}

/*! Materializes a referenced wire-order bloom into the decreasing-index
    semantic vector used by the Yellow Paper equations. */
function logs_bloom_from_ref(reference : LogsBloomRef) -> LogsBloom = {
    var out : LogsBloom = EMPTY_LOGS_BLOOM;
    foreach (i from 0 to 255) {
        out[255 - i] = slice_byte(reference, i)
    };
    out
}

/*! Compares the computed block bloom with the payload-header commitment. */
function logs_bloom_matches_ref(computed : LogsBloom, reference : LogsBloomRef) -> bool =
    logs_bloom_equal(computed, logs_bloom_from_ref(reference))

/*! The execution-payload header fields the model reads and validates
    (YP §4.4). Scalar wire bounds come from the consensus/Amsterdam SSZ
    `ExecutionPayload` schema. `gas_used <= gas_limit` and the active
    blob-schedule rules are execution-protocol constraints checked when the
    payload is admitted. `extra_data` retains the schema's
    `ByteList[MAX_EXTRA_DATA_BYTES]` bound while staying source-backed; it is
    RLP-encoded whole for the header hash and never inspected. The fixed
    `logs_bloom` commitment likewise stays source-backed until a semantic
    consumer explicitly decodes it. */
struct BlockHeader = {
    number : block_number,
    timestamp : block_timestamp,
    gas_limit : block_gas_limit,
    gas_used : block_gas,
    prev_randao : word,
    base_fee : word,
    /* EIP-4844: a multiple of GAS_PER_BLOB within the active schedule. */
    blob_gas_used : blob_gas_used,
    /* EIP-4844 uint64 on the wire; narrowed to the documented reachable-chain
       invariant at the authenticated input boundary. */
    excess_blob_gas : excess_blob_gas,
    state_root : hash,
    receipts_root : hash,
    logs_bloom : LogsBloomRef,
    fee_recipient : address,
    parent_hash : hash,
    parent_beacon_block_root : hash,
    /* uint64 (EIP-7843 and the Amsterdam stateless SSZ schema). */
    slot_number : slot_number,
    extra_data : StatelessInputSliceAtMost(extra_data_length_bound),
}

/*! `keccak256(rlp([]))` — the ommers hash of every post-merge block
    (EIP-3675 requires an empty ommers list). */
let EMPTY_OMMER_HASH : hash = word_to_hash(
    u256(unsigned(0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347)),
)

/*! An EIP-4895 beacon-chain withdrawal: index, validator, recipient,
    and amount in gwei. */
struct Withdrawal = {
    index : withdrawal_index,
    validator_index : validator_index,
    address : address,
    amount : withdrawal_amount,
}

/*! The block body. The semantic structure is explicit while its
    potentially large fields stay source-backed until individual elements
    are needed. */
struct BlockBody = {
    transactions : TransactionListRef,
    withdrawals : WithdrawalListRef,
    block_access_list : StatelessInputSliceAtMost(block_access_list_length_bound),
}

/*! A block: header plus body. */
struct Block = {
    header : BlockHeader,
    body : BlockBody,
}

/*! The execution payload under validation: the block and the block hash
    the consensus layer expects it to commit to. */
struct ExecutionPayload = {
    expected_block_hash : hash,
    block : Block,
}
