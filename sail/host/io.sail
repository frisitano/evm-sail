/* ======================================================================== */
/* HOST I/O: CRYPTO, PRECOMPILES, STATELESS SSZ INPUT                       */
/*                                                                          */
/* Everything the model obtains from OUTSIDE itself -- the boundary between */
/* the EVM specification and its environment. Three layers:                 */
/*                                                                          */
/* (1) ABSTRACT HOST I/O -- staged precompile input, stateless input bytes, */
/* and guest public-output bytes. The model commits to WHAT is serialized   */
/* or executed; build.sail supplies the concrete C-backed bindings.         */
/*                                                                          */
/* (2) CRYPTO -- keccak256 (the Ethereum hash: KECCAK256, trie/RLP          */
/* node references, CREATE2 salt) and sha256 (precompile 0x02 + SSZ         */
/* merkleization), the EVM precompiles over staged byte ranges, and the     */
/* CREATE (Yellow Paper) / CREATE2 (EIP-1014) address derivations.          */
/*                                                                          */
/* (3) STATELESS INPUT -- decode the canonical SSZ SszStatelessInput from   */
/* the private-input region (ssz_src_*): the execution payload, the         */
/* execution witness (state nodes, codes, headers), the block access list   */
/* (EIP-7928), and the chain config. Mirrors execution-specs                */
/* (projects/zkevm) forks/amsterdam stateless_ssz.py.                       */
/*                                                                          */
/* Host mechanism, like memory and storage. Included early -- the crypto is */
/* used by state, kernel, gas, and transaction; the SSZ                     */
/* decoders are called only by the outer stateless layers (the lib/mpt.sail */
/* witness reader, sail/main.sail, the EEST runner).                        */
/* ======================================================================== */

/* The block-access-list key: an (address, slot) pair (EIP-7928). bal_load
 * (below) materializes the permitted access set from the SSZ input, and the
 * kernel's storage guards reject any SLOAD/SSTORE outside it. Defined here
 * because bal_load is its producer and host I/O is included before
 * host/state.sail (which also consumes it). */
struct SlotKey = { address : address, slot : word }

/* ===================== (2) CRYPTO ======================================= */

/* Big-endian byte serializations used by RLP, the tries, and address derivation. */
function word_to_bytes32(w : word) -> list(byte) = {
    var out : list(byte) = [||]; /* O(n): prepend big-endian */
    foreach (k from 0 to 31) {
        out = sail_shiftright(w, 8 * k)[7 .. 0] :: out
    };
    out
}
function address_to_bytes(a : address) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to 19) {
        out = sail_shiftright(a, 8 * k)[7 .. 0] :: out
    };
    out
}

function word_be_len(w : word) -> int = {
    var n : int = 0;
    foreach (k from 0 to 31) {
        if unsigned(sail_shiftright(w, 8 * k)[7 .. 0]) != 0 then {
            n = k + 1
        } else {
            ()
        }
    };
    n
}

function int_be_len(n : int) -> int = if n <= 0 then {
    0
} else {
    1 + int_be_len(ediv_int(n, 256))
}
termination_measure int_be_len(n) = n
function int_to_be_bytes(n : int) -> list(byte) = {
    let len = int_be_len(n);
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = list_append(out, [|get_slice_int(8, emod_int(ediv_int(n, pow2(8 * (len - 1 - k))), 256), 0)|])
    };
    out
}

/* RLP encoding of the account nonce, as used in CREATE address derivation
 * (0 encodes as 0x80; a single byte < 128 is itself; otherwise length-prefixed). */
function rlp_nonce(nonce : int) -> list(byte) =
    if nonce == 0 then {
        [|0x80|]
    } else {
        if nonce < 128 then {
            [|get_slice_int(8, nonce, 0)|]
        } else {
            list_append([|get_slice_int(8, 128 + int_be_len(nonce), 0)|], int_to_be_bytes(nonce))
        }
    }

/* SHA-256 self-check against the standard "" and "abc" reference vectors. */
val sha256_selfcheck : unit -> unit

/* Keccak self-check against Ethereum's "" and "abc" vectors. */
val keccak_selfcheck : unit -> unit

struct BlockHeader = {
    number : int,
    timestamp : int,
    gas_limit : int,
    gas_used : int,
    prev_randao : word,
    base_fee : word,
    blob_gas_used : int,
    excess_blob_gas : int,
    state_root : bits(256),
    receipts_root : bits(256),
    logs_bloom : list(byte),
    block_hash : bits(256),
    fee_recipient : address,
    parent_hash : bits(256),
    parent_beacon_block_root : bits(256),
    slot_number : int,
}

let EMPTY_OMMER_HASH : hash = 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347

/* Hash full code bytes, store them in the code database, and return codeHash. */
val code_hash_and_store : list(byte) -> hash

/* Domain operations whose spec definitions may materialize canonical bytes,
 * while build definitions may stream or hash from host byte sources. */
val bloom_entry_bit : (list(byte), int) -> int
val execution_requests_hash : unit -> hash
val block_header_hash : (BlockHeader, hash, hash, hash, hash) -> hash

/* ===================== (3) STATELESS SSZ INPUT ========================= */

function ib(i : int) -> int = unsigned(ssz_src_byte(i))

/* little-endian uintN (n <= 8 bytes) at absolute input offset b -- one FFI call */
function ssz_uint(b : int, n : int) -> int = unsigned(ssz_src_le(b, n))
function ssz_u32(b : int) -> int = unsigned(ssz_src_le(b, 4))
function ssz_u64(b : int) -> int = unsigned(ssz_src_le(b, 8))

/* big-endian 20-byte address at offset b -- three FFI reads (8+8+4) */
function ssz_addr(b : int) -> address = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 4);
    append(w0, append(w1, w2[31 .. 0]))
}

/* read len bytes at absolute offset b into a list(byte) (O(len), cons+reverse) */
function ssz_bytes(b : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = ssz_src_byte(b + (len - 1 - k)) :: out
    };
    out
}

/* ---- container navigation --------------------------------------------- */

/* SSZ body starts after the 2-byte big-endian schema id */
let SSZ_BODY : int = 2
function ssz_schema_id() -> int = ib(0) * 256 + ib(1)

/* SszStatelessInput: 4 variable fields, offsets at body+{0,4,8,12} */
function npr_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 0)
function witness_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 4)
function cc_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 8)
function cc_end() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 12) /* = public_keys start */

/* SszNewPayloadRequest fixed region: execution_payload offset @0, versioned_hashes
 * offset @4, parent_beacon_block_root @8 (Bytes32), execution_requests offset @40 */
function payload_start() -> int = {
    let n = npr_start();
    n + ssz_u32(n + 0)
}
function payload_end() -> int = {
    let n = npr_start();
    n + ssz_u32(n + 4)
}
function npr_end() -> int = witness_start() /* npr is the first top field */

/* SszExecutionRequests is a container of three variable lists (offsets at
 * req+{0,4,8}); execution_requests is field 3 of SszNewPayloadRequest, whose
 * offset sits at npr+40 (after the 32-byte parent_beacon_block_root @ npr+8). */
function req_start() -> int = npr_start() + ssz_u32(npr_start() + 40)
function dep_start() -> int = req_start() + ssz_u32(req_start() + 0)
function wreq_start() -> int = req_start() + ssz_u32(req_start() + 4)
function creq_start() -> int = req_start() + ssz_u32(req_start() + 8)
function req_end() -> int = npr_end() /* execution_requests is the last npr field */

/* SszExecutionPayload fixed-region field offsets (bytes from payload start) */
let PL_FEE_RECIPIENT : int = 32
let PL_STATE_ROOT : int = 52
let PL_RECEIPTS_ROOT : int = 84
let PL_LOGS_BLOOM : int = 116
let PL_BLOCK_NUMBER : int = 404
let PL_GAS_LIMIT : int = 412
let PL_GAS_USED : int = 420
let PL_TIMESTAMP : int = 428
let PL_EXTRA_OFF : int = 436
let PL_BASE_FEE : int = 440
let PL_BLOCK_HASH : int = 472
let PL_TXS_OFF : int = 504
let PL_WDS_OFF : int = 508
let PL_BLOB_GAS_USED : int = 512
let PL_EXCESS_BLOB_GAS : int = 520
let PL_BAL_OFF : int = 528

/* big-endian 32-byte field (Bytes32, e.g. prev_randao) -- four FFI reads */
function ssz_bytes32(b : int) -> bits(256) = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 8);
    let w3 = ssz_src_be(b + 24, 8);
    append(w0, append(w1, append(w2, w3)))
}

/* little-endian 32-byte uint256 (SSZ) -- four FFI reads, low word first */
function ssz_u256(b : int) -> word = {
    let c0 = ssz_src_le(b + 0, 8);
    let c1 = ssz_src_le(b + 8, 8);
    let c2 = ssz_src_le(b + 16, 8);
    let c3 = ssz_src_le(b + 24, 8);
    append(c3, append(c2, append(c1, c0)))
}

let PL_PREV_RANDAO : int = 372
function pl_block_number() -> int = ssz_u64(payload_start() + PL_BLOCK_NUMBER)
function pl_slot_number() -> int = ssz_u64(payload_start() + 532) /* EIP-7843 */
function pl_gas_limit() -> int = ssz_u64(payload_start() + PL_GAS_LIMIT)
function pl_gas_used() -> int = ssz_u64(payload_start() + PL_GAS_USED)
function pl_timestamp() -> int = ssz_u64(payload_start() + PL_TIMESTAMP)
function pl_fee_recipient() -> address = ssz_addr(payload_start() + PL_FEE_RECIPIENT)
function pl_prev_randao() -> bits(256) = ssz_bytes32(payload_start() + PL_PREV_RANDAO)
function pl_base_fee() -> word = ssz_u256(payload_start() + PL_BASE_FEE)
function pl_blob_gas_used() -> int = ssz_u64(payload_start() + PL_BLOB_GAS_USED)
function pl_excess_blob_gas() -> int = ssz_u64(payload_start() + PL_EXCESS_BLOB_GAS)
function pl_state_root() -> bits(256) = ssz_bytes32(payload_start() + PL_STATE_ROOT)
function pl_receipts_root() -> bits(256) = ssz_bytes32(payload_start() + PL_RECEIPTS_ROOT)
function pl_logs_bloom() -> list(byte) = ssz_bytes(payload_start() + PL_LOGS_BLOOM, 256)
function pl_block_hash() -> bits(256) = ssz_bytes32(payload_start() + PL_BLOCK_HASH)

/* variable sub-regions of the payload (offsets relative to payload start) */
function pl_txs_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_TXS_OFF)
}
function pl_wds_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_WDS_OFF)
}
function pl_bal_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_BAL_OFF)
}
function pl_extra_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_EXTRA_OFF)
}
function pl_extra_data() -> list(byte) =
    ssz_bytes(pl_extra_start(), pl_txs_start() - pl_extra_start())
function pl_block_access_list() -> list(byte) =
    ssz_bytes(pl_bal_start(), payload_end() - pl_bal_start())

/* block_access_list (simplified): u32-LE count, then count*(addr[20] ++ slot[32 BE]).
 * bal_load materializes the permitted (addr,slot) set; the kernel's k_sload/k_sstore
 * panic on any access outside it (k_bal_enable / bal_guard). */
function bal_count() -> int = ssz_u32(pl_bal_start())
function bal_load() -> list(SlotKey) = {
    let base = pl_bal_start() + 4;
    let n = bal_count();
    var xs : list(SlotKey) = [||];
    foreach (i from 0 to (n - 1)) {
        xs = struct { address = ssz_addr(base + i * 52), slot = ssz_bytes32(base + i * 52 + 20) } :: xs
    };
    xs
}

/* withdrawals: FIXED 44-byte SszWithdrawal elements, span [wds, bal) */
function pl_n_withdrawals() -> int = ediv_int(pl_bal_start() - pl_wds_start(), 44)
function withdrawal_index(i : int) -> int = ssz_u64(pl_wds_start() + i * 44 + 0)
function withdrawal_validator_index(i : int) -> int = ssz_u64(pl_wds_start() + i * 44 + 8)
function withdrawal_amount(i : int) -> int = ssz_u64(pl_wds_start() + i * 44 + 36)
function withdrawal_addr(i : int) -> address = ssz_addr(pl_wds_start() + i * 44 + 16)

/* transactions: List[ByteList], span [txs, wds); count via the leading offset */
function pl_n_txs() -> int = {
    let t = pl_txs_start();
    let w = pl_wds_start();
    if t == w then {
        0
    } else {
        ediv_int(ssz_u32(t), 4)
    }
}

/* SszChainConfig: chain_id u64 @ cc+0, active_fork (SszForkConfig) offset @ cc+8;
 * SszForkConfig: fork u64 @ 0 (ProtocolFork index; Amsterdam = 20 in current
 * projects/zkevm fixtures). */
function cc_chain_id() -> int = ssz_u64(cc_start() + 0)
function cc_active_fork_start() -> int = {
    let c = cc_start();
    c + ssz_u32(c + 8)
}
function cc_fork() -> int = ssz_u64(cc_active_fork_start() + 0)

/* SszForkConfig { fork:u64 @0, activation:SszForkActivation (offset @8),
 * blob_schedule (offset @12) }; SszForkActivation { block_number, timestamp :
 * SszOptionalForkActivationValue (List[u64,1], offsets @ activation+{0,4}) }.
 * The active_fork's activation must be ACTIVE for the block (EELS
 * validate_chain_config -> _is_activation_active): at least one of block_number
 * / timestamp set, and any set value must not exceed the payload's -- a FUTURE
 * activation point makes the block invalid. */
function cc_activation_start() -> int = {
    let f = cc_active_fork_start();
    f + ssz_u32(f + 8)
}
function cc_blobsched_start() -> int = {
    let f = cc_active_fork_start();
    f + ssz_u32(f + 12)
}
function cc_activation_active() -> bool = {
    let a = cc_activation_start();
    let bn_start = a + ssz_u32(a + 0);
    let ts_start = a + ssz_u32(a + 4);
    let bs_start = cc_blobsched_start();
    let bn_present = (ts_start - bn_start) >= 8; /* the optional list is non-empty */
    let ts_present = (bs_start - ts_start) >= 8;
    let bn_ok = not_bool(bn_present) | (pl_block_number() >= ssz_u64(bn_start));
    let ts_ok = not_bool(ts_present) | (pl_timestamp() >= ssz_u64(ts_start));
    (bn_present | ts_present) & bn_ok & ts_ok
}

/* SszExecutionWitness: 3 variable List fields, offsets at witness+{0,4,8} */
function wit_state_start() -> int = {
    let w = witness_start();
    w + ssz_u32(w + 0)
}
function wit_codes_start() -> int = {
    let w = witness_start();
    w + ssz_u32(w + 4)
}
function wit_headers_start() -> int = {
    let w = witness_start();
    w + ssz_u32(w + 8)
}
function wit_n_state() -> int = {
    let s = wit_state_start();
    let c = wit_codes_start();
    if s == c then {
        0
    } else {
        ediv_int(ssz_u32(s), 4)
    }
}
function wit_n_codes() -> int = {
    let c = wit_codes_start();
    let h = wit_headers_start();
    if c == h then {
        0
    } else {
        ediv_int(ssz_u32(c), 4)
    }
}
function wit_n_headers() -> int = {
    let h = wit_headers_start();
    let e = cc_start(); /* the witness container ends where chain_config begins */
    if h == e then {
        0
    } else {
        ediv_int(ssz_u32(h), 4)
    }
}

/* element i bytes of a List[ByteList] given its [start, end) span + count */
function ssz_list_elem(start : int, n : int, stop : int, i : int) -> list(byte) = {
    let off_i = ssz_u32(start + i * 4);
    let off_next = if i + 1 < n then {
            ssz_u32(start + (i + 1) * 4)
        } else {
            stop - start
        };
    ssz_bytes(start + off_i, off_next - off_i)
}

/* (start, len) of element i of a List[ByteList] given its [start, stop) span */
function ssz_list_span(start : int, n : int, stop : int, i : int) -> (int, int) = {
    let off_i = ssz_u32(start + i * 4);
    let off_next = if i + 1 < n then {
            ssz_u32(start + (i + 1) * 4)
        } else {
            stop - start
        };
    (start + off_i, off_next - off_i)
}
function ssz_list_slice(start : int, n : int, stop : int, i : int) -> witness_slice = {
    let (off, len) = ssz_list_span(start, n, stop, i);
    witness_slice(off, len)
}
function read_witness_slice(s : witness_slice) -> list(byte) =
    ssz_bytes(unsigned(s.off), unsigned(s.len))
function witness_keccak(s : witness_slice) -> bits(256) =
    bytes_keccak(witness_byte_slice(s))
function sha256_request_digest(request_type : int, s : witness_slice) -> bits(256) =
    bytes_sha256_prefixed(get_slice_int(8, request_type, 0), witness_byte_slice(s))
function ssz_witness_node_span(i : int) -> (int, int) =
    ssz_list_span(wit_state_start(), wit_n_state(), wit_codes_start(), i)
function ssz_witness_node_slice(i : int) -> witness_slice =
    ssz_list_slice(wit_state_start(), wit_n_state(), wit_codes_start(), i)
function ssz_witness_node(i : int) -> list(byte) =
    read_witness_slice(ssz_witness_node_slice(i))
function ssz_witness_code_span(i : int) -> (int, int) =
    ssz_list_span(wit_codes_start(), wit_n_codes(), wit_headers_start(), i)
function ssz_witness_code_slice(i : int) -> witness_slice =
    ssz_list_slice(wit_codes_start(), wit_n_codes(), wit_headers_start(), i)
function ssz_witness_header_span(i : int) -> (int, int) =
    ssz_list_span(wit_headers_start(), wit_n_headers(), cc_start(), i)
function ssz_witness_header_slice(i : int) -> witness_slice =
    ssz_list_slice(wit_headers_start(), wit_n_headers(), cc_start(), i)
function ssz_witness_header(i : int) -> list(byte) =
    read_witness_slice(ssz_witness_header_slice(i))

/* transactions[i] raw bytes (legacy = RLP list; typed = type byte ++ payload) */
