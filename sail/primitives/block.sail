/* ======================================================================== */
/* PRIMITIVES: BLOCK-LEVEL TYPES                                            */
/*                                                                          */
/* BlockHeader is pure data. Its k_header register lives in                 */
/* host/kernel/environment.sail alongside the EIP-4895 withdrawal record.   */
/* ======================================================================== */

type LogsBloom = vector(32, dec, limb)

let EMPTY_LOGS_BLOOM : LogsBloom = vector_init(32, LIMB_ZERO)

function logs_bloom_equal(a : LogsBloom, b : LogsBloom) -> bool = {
    var equal : bool = true;
    foreach (i from 0 to 31) {
        equal = equal & (a[i] == b[i])
    };
    equal
}

function logs_bloom_bytes(bloom : LogsBloom) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        let limb = bloom[31 - k];
        out = limb[7 .. 0] :: out;
        out = limb[15 .. 8] :: out;
        out = limb[23 .. 16] :: out;
        out = limb[31 .. 24] :: out;
        out = limb[39 .. 32] :: out;
        out = limb[47 .. 40] :: out;
        out = limb[55 .. 48] :: out;
        out = limb[63 .. 56] :: out
    };
    out
}

struct BlockHeader = {
    number : block_number,
    timestamp : block_timestamp,
    gas_limit : gas,
    gas_used : gas,
    prev_randao : word,
    base_fee : word,
    blob_gas_used : blob_gas,
    excess_blob_gas : blob_gas,
    state_root : bits(256),
    receipts_root : bits(256),
    logs_bloom : LogsBloom,
    fee_recipient : address,
    parent_hash : bits(256),
    parent_beacon_block_root : bits(256),
    slot_number : slot_number,
    extra_data : ByteSlice,
}

let EMPTY_OMMER_HASH : hash = 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347

struct Withdrawal = {
    index : withdrawal_index,
    validator_index : validator_index,
    address : address,
    amount : withdrawal_amount,
}

/* The semantic block structure remains explicit while its potentially large
 * body fields stay source-backed until their individual elements are needed. */
struct BlockBody = {
    transactions : SszListRef,
    withdrawals : SszListRef,
    block_access_list : ByteSlice,
}

struct Block = {
    header : BlockHeader,
    body : BlockBody,
}

struct ExecutionPayload = {
    expected_block_hash : hash,
    block : Block,
}

/* Raw EIP-7685 request bodies. The request-type bytes are added only while
 * computing the header commitment; the SSZ input carries one field per type. */
struct ExecutionRequests = {
    deposits : ByteSlice,
    withdrawals : ByteSlice,
    consolidations : ByteSlice,
}

let EMPTY_EXECUTION_REQUESTS : ExecutionRequests = struct {
        deposits = EMPTY_SLICE,
        withdrawals = EMPTY_SLICE,
        consolidations = EMPTY_SLICE,
    }
