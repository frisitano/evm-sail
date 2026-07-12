/* ======================================================================== */
/* PRIMITIVES: BLOCK-LEVEL TYPES                                            */
/*                                                                          */
/* BlockHeader is pure data. Its k_header register lives in                 */
/* host/kernel/environment.sail alongside the EIP-4895 withdrawal record.   */
/* ======================================================================== */

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
    logs_bloom : list(byte),
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

/* The block body: what the block carries besides its header. Transactions and
 * withdrawals are decoded by the single input pass; the block access list
 * (EIP-7928) stays a REFERENCE to its raw SSZ bytes -- it is hashed into the
 * header and size-capped, never decoded into a list. */
struct BlockBody = {
    transactions : list(Transaction),
    withdrawals : list(Withdrawal),
    block_access_list : ByteSlice,
}

struct Block = {
    header : BlockHeader,
    body : BlockBody,
}

/* The engine-API view (EELS ExecutionPayload): the block plus the CLAIMED
 * hash of its consensus header -- validation reconstructs the header (derived
 * trie roots, requests hash, BAL hash) and checks its keccak against this. */
struct ExecutionPayload = {
    block_hash : hash,
    block : Block,
}
