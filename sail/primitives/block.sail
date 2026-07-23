
/*md
# Block types

Block-level data structures. [BlockHeader][type-BlockHeader] is pure data;
its `k_header` register lives in the kernel environment alongside the
EIP-4895 withdrawal record.
*/
/*! The 2048-bit logs bloom filter (YP §4.4.1), as 256 bytes. */
type LogsBloom = vector(256, dec, byte)

let EMPTY_LOGS_BLOOM : LogsBloom = vector_init(256, 0x00)

/*! Byte-wise bloom equality (the header `logs_bloom` check). */
function logs_bloom_equal(a : LogsBloom, b : LogsBloom) -> bool = {
    var equal : bool = true;
    foreach (i from 0 to 255) {
        equal = equal & (a[i] == b[i])
    };
    equal
}

/*! The bloom as a most-significant-first byte list, for RLP encoding. */
function logs_bloom_bytes(bloom : LogsBloom) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 255) {
        out = bloom[k] :: out
    };
    out
}

/*! The execution-payload header fields the model reads and validates
    (YP §4.4). `extra_data` stays a reference into the payload — it is
    RLP-encoded whole for the header hash and never inspected. */
struct BlockHeader = {
    number : block_number,
    timestamp : block_timestamp,
    gas_limit : block_gas_limit,
    gas_used : block_gas,
    prev_randao : word,
    base_fee : word,
    /* EIP-4844: a multiple of GAS_PER_BLOB within the active schedule. */
    blob_gas_used : blob_gas_used,
    /* EIP-4844: the accumulating uint64 excess-blob-gas field. */
    excess_blob_gas : excess_blob_gas,
    state_root : hash,
    receipts_root : hash,
    logs_bloom : LogsBloom,
    fee_recipient : address,
    parent_hash : hash,
    parent_beacon_block_root : hash,
    /* uint64 (EIP-7843) */
    slot_number : slot_number,
    extra_data : ByteSlice,
}

/*! `keccak256(rlp([]))` — the ommers hash of every post-merge block
    (EIP-3675 requires an empty ommers list). */
let EMPTY_OMMER_HASH : hash = B256(to_bytes_le(32, 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347))

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
    block_access_list : ByteSlice,
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

/*! Raw EIP-7685 request bodies. The request-type bytes are added only
    while computing the header commitment; the SSZ input carries one field
    per type. */
struct ExecutionRequests = {
    deposits : ByteSlice,
    withdrawals : ByteSlice,
    consolidations : ByteSlice,
    builder_deposits : ByteSlice,
    builder_exits : ByteSlice,
}

let EMPTY_EXECUTION_REQUESTS : ExecutionRequests = struct {
        deposits = EMPTY_SLICE,
        withdrawals = EMPTY_SLICE,
        consolidations = EMPTY_SLICE,
        builder_deposits = EMPTY_SLICE,
        builder_exits = EMPTY_SLICE,
    }
