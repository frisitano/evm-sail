/* ======================================================================== */
/* HOST I/O: HOST CRYPTO, PRECOMPILES, STATELESS SSZ INPUT                  */
/*                                                                          */
/* Everything the model obtains from OUTSIDE itself -- the boundary between */
/* the EVM specification and its environment. Three layers:                 */
/*                                                                          */
/* (1) FFI BINDING -- direct host hash helpers and staged precompile input. */
/* The model commits to WHAT is serialized or executed; C supplies          */
/* pointer/length calls to host crypto and precompile implementations.      */
/*                                                                          */
/* (2) HOST CRYPTO -- keccak256 (the Ethereum hash: KECCAK256, trie/RLP     */
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

/* ===================== (1) FFI BINDING ================================= */

val precompile_input_byte_ffi = impure { _: "precompile_input_byte" } : bits(64) -> bits(8)
val precompile_stage_tx = impure { _: "precompile_stage_tx" } : bits(64) -> unit
val precompile_run_staged_to_returndata = impure { _: "precompile_run_staged_to_returndata" } : unit -> bits(64)
val precompile_run_host_input = impure { _: "precompile_run_host_input" } : bits(64) -> bits(64)
val precompile_ok = impure { _: "precompile_ok" } : unit -> bits(8)
val precompile_out = impure { _: "precompile_out" } : bits(64) -> bits(8)
val host_bytes_reset = impure { _: "host_bytes_reset" } : unit -> unit
val host_bytes_append_byte = impure { _: "host_bytes_append_byte" } : bits(8) -> unit
val host_bytes_append8 = impure { _: "host_bytes_append8" } : bits(64) -> unit
val host_keccak_input = impure { _: "host_keccak_input" } : unit -> unit
val host_sha256_input = impure { _: "host_sha256_input" } : unit -> unit
val host_keccak_witness = impure { _: "host_keccak_witness" } : (witness_offset, witness_len) -> unit
val host_keccak_memory = impure { _: "host_keccak_memory" } : (memory_offset, memory_len) -> unit
val host_sha256_request_digest = impure { _: "host_sha256_request_digest" } : (bits(64), witness_offset, witness_len) -> unit
val host_keccak_word = impure { _: "host_keccak_word" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
val host_keccak_address = impure { _: "host_keccak_address" } : (bits(64), bits(64), bits(64)) -> unit
val host_sha256_pair_words = impure { _: "host_sha256_pair_words" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val host_hash_word = impure { _: "host_hash_word" } : bits(64) -> bits(64)

function host_bytes_append_list(msg : list(byte)) -> unit = match msg {
    b0 :: (b1 :: (b2 :: (b3 :: (b4 :: (b5 :: (b6 :: (b7 :: r))))))) => {
        host_bytes_append8(append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7))))))));
        host_bytes_append_list(r)
    },
    b :: r => {
        host_bytes_append_byte(b);
        host_bytes_append_list(r)
    },
    [||] => (),
}

function host_digest() -> bits(256) = {
    let w0 : bits(64) = host_hash_word(get_slice_int(64, 0, 0));
    let w1 : bits(64) = host_hash_word(get_slice_int(64, 1, 0));
    let w2 : bits(64) = host_hash_word(get_slice_int(64, 2, 0));
    let w3 : bits(64) = host_hash_word(get_slice_int(64, 3, 0));
    append(w0, append(w1, append(w2, w3)))
}

function keccak256_source(src : byte_source) -> bits(256) = match src {
    ByteListSource(msg) => {
        host_bytes_reset();
        host_bytes_append_list(msg);
        host_keccak_input();
        let h = host_digest();
        host_bytes_reset();
        h
    },
    WitnessSource(s) => {
        host_keccak_witness(s.off, s.len);
        host_digest()
    },
    MemorySource(s) => {
        host_keccak_memory(s.off, s.len);
        host_digest()
    },
    WordSource(w) => {
        host_keccak_word(w[255 .. 192], w[191 .. 128], w[127 .. 64], w[63 .. 0]);
        host_digest()
    },
    AddressSource(a) => {
        host_keccak_address(a[159 .. 96], a[95 .. 32], sail_zero_extend(a[31 .. 0], 64));
        host_digest()
    },
}

function sha256_request_digest(request_type : int, s : witness_slice) -> bits(256) = {
    host_sha256_request_digest(get_slice_int(64, request_type, 0), s.off, s.len);
    host_digest()
}

/* ===================== (2) HOST CRYPTO ================================= */

/* The i-th byte of the STAGED precompile input, zero past the end. A
 * precompile's input is staged once (a snapshot of the caller's memory slice
 * for a CALL, or the tx data for a tx-to-precompile); gas inspection (the
 * MODEXP length header, the BLAKE2F rounds count) reads only the leading
 * bytes it needs, directly from the staged buffer. */
function precompile_input_byte(i : int) -> byte = precompile_input_byte_ffi(get_slice_int(64, i, 0))

/* Stage a precompile's input from the streamed tx buffer (a transaction
 * sent directly to a precompile; CALL-family staging is memory-sourced --
 * see stage_precompile_input in host/memory.sail). */
function stage_precompile_txd(num : int) -> unit = precompile_stage_tx(get_slice_int(64, num, 0))

/* Run the staged precompile (input already snapshotted from memory or tx data).
 * Output is written directly into pending returndata.
 * Returns false on precompile failure, which the interpreter maps to a CALL
 * result of 0 with empty returndata. */
function run_precompile_staged() -> bool = {
    let _ = precompile_run_staged_to_returndata();
    unsigned(precompile_ok()) == 1
}

/* Read `n` bytes from the most recent precompile scratch output into a list. */
function precompile_read(n : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = precompile_out(get_slice_int(64, i, 0)) :: out
    };
    list_reverse(out)
}

/* List-in / list-out precompile wrapper for tx-auth helpers and vector tests.
 * The list is first materialized in the host byte scratch; C then invokes the
 * same pointer/length precompile path used by staged execution. */
function run_precompile(num : int, input : list(byte)) -> (bool, list(byte)) = {
    host_bytes_reset();
    host_bytes_append_list(input);
    let n = unsigned(precompile_run_host_input(get_slice_int(64, num, 0)));
    let ok = unsigned(precompile_ok()) == 1;
    let out = if not_bool(ok) then {
            [||]
        } else {
            precompile_read(n)
        };
    host_bytes_reset();
    (ok, out)
}

/* ======================================================================== */
/* HASH PRIMITIVES + CREATE ADDRESS DERIVATION                              */
/*                                                                          */
/* keccak256 and sha256 route through the host byte scratch and host-crypto */
/* pointer/length boundary. The model defines what is hashed; the host      */
/* computes the digest behind the standard accelerator header.              */
/* ======================================================================== */

/* Keccak-256, the Ethereum hash function: the KECCAK256 opcode, trie/RLP
 * node references, code hashes, and CREATE2 init-code commitments. id 0. */
function keccak256(msg : list(byte)) -> bits(256) = keccak256_source(ByteListSource(msg))

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

/* SHA-256 (id 2): the precompile 0x02 hash and the SSZ merkleization hash --
 * distinct from the keccak256 used throughout execution. */
function sha256(msg : list(byte)) -> bits(256) = {
    host_bytes_reset();
    host_bytes_append_list(msg);
    host_sha256_input();
    let h = host_digest();
    host_bytes_reset();
    h
}

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) = {
    host_sha256_pair_words(
        a[255 .. 192],
        a[191 .. 128],
        a[127 .. 64],
        a[63 .. 0],
        b[255 .. 192],
        b[191 .. 128],
        b[127 .. 64],
        b[63 .. 0],
    );
    host_digest()
}

/* SHA-256 self-check against the standard "" and "abc" reference vectors. */
function sha256_selfcheck() -> unit = {
    assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855, "sha256('') on guest");
    assert(
        sha256([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}

/* ===================== (3) STATELESS SSZ INPUT ========================= */

val ssz_src_len = impure { _: "ssz_src_len" } : unit -> bits(64)
val ssz_src_byte = impure { _: "ssz_src_byte" } : int -> bits(8)

/* bulk readers: n (<=8) bytes from the input in one FFI call (LE / BE) */
val ssz_src_le = impure { _: "ssz_src_le" } : (int, int) -> bits(64)
val ssz_src_be = impure { _: "ssz_src_be" } : (int, int) -> bits(64)

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
