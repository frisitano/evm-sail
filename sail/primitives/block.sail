/* ======================================================================== */
/* PRIMITIVES: BLOCK-LEVEL TYPES                                            */
/*                                                                          */
/* BlockHeader is pure data. Its k_header register lives in                 */
/* host/kernel/environment.sail alongside the EIP-4895 withdrawal record.   */
/* ======================================================================== */

type LogsBloom = vector(256, dec, byte)

let EMPTY_LOGS_BLOOM : LogsBloom = vector_init(256, 0x00)

function logs_bloom_equal(a : LogsBloom, b : LogsBloom) -> bool = {
    var i : int = 0;
    var equal : bool = true;
    while equal & (i < 256) do {
        let index = unsigned(get_slice_int(8, i, 0));
        equal = a[index] == b[index];
        i = i + 1
    };
    equal
}

function logs_bloom_bytes(bloom : LogsBloom) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 255) {
        out = bloom[unsigned(get_slice_int(8, 255 - k, 0))] :: out
    };
    out
}

struct BlockHeader = {
    number : int,
    timestamp : int,
    gas_limit : int,
    gas_used : int,
    prev_randao : word,
    base_fee : word,
    blob_gas_used : u64, /* EELS U64 */
    excess_blob_gas : u64, /* EELS U64 */
    state_root : bits(256),
    receipts_root : bits(256),
    logs_bloom : LogsBloom,
    fee_recipient : address,
    parent_hash : bits(256),
    parent_beacon_block_root : bits(256),
    slot_number : u64, /* EELS U64 (EIP-7843) */
    extra_data : ByteSlice, /* reference into the payload (RLP-encoded whole for
                               the header hash, never inspected) */
}

let EMPTY_OMMER_HASH : hash = 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347

struct Withdrawal = {
    index : int,
    validator_index : int,
    address : address,
    amount : int,
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
