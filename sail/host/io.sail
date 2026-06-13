/* ======================================================================== */
/* HOST I/O: ACCELERATOR FFI, HOST CRYPTO, STATELESS SSZ INPUT              */
/*                                                                          */
/* Everything the model obtains from OUTSIDE itself -- the boundary between */
/* the EVM specification and its environment. Three layers:                 */
/*                                                                          */
/* (1) FFI BINDING -- the accelerator byte-stream protocol (mechanism only, */
/* no semantics). The model commits to WHAT is hashed/verified; the host    */
/* computes the result. Inputs stream across the C boundary 8 bytes per     */
/* crossing (begin / push / exec / read).                                   */
/*                                                                          */
/* (2) HOST CRYPTO -- keccak256 (the Ethereum hash: KECCAK256, trie/RLP     */
/* node references, CREATE2 salt) and sha256 (precompile 0x02 + SSZ         */
/* merkleization), the EVM precompiles over the same binding, and the       */
/* CREATE (Yellow Paper) / CREATE2 (EIP-1014) address derivations.          */
/*                                                                          */
/* (3) STATELESS INPUT -- decode the canonical SSZ SszStatelessInput from   */
/* the private-input region (ssz_src_*): the execution payload, the         */
/* execution witness (state nodes, codes, headers), the block access list   */
/* (EIP-7928), and the chain config. Mirrors execution-specs                */
/* (projects/zkevm) forks/amsterdam stateless_ssz.py.                       */
/*                                                                          */
/* Host mechanism, like memory and storage. Included early -- the crypto is */
/* used by block_hash, state, kernel, gas, and transaction; the SSZ         */
/* decoders are called only by the outer stateless layers (the lib/mpt.sail */
/* witness reader, zkvm/zkvm_block.sail, the EEST runner).                  */
/* ======================================================================== */

/* The block-access-list key: an (address, slot) pair (EIP-7928). bal_load
 * (below) materializes the permitted access set from the SSZ input, and the
 * kernel's storage guards reject any SLOAD/SSTORE outside it. Defined here
 * because bal_load is its producer and host I/O is included before
 * host/state.sail (which also consumes it). */
struct SlotKey = { sk_addr : address, sk_slot : word }

/* ===================== (1) FFI BINDING ================================= */

val acc_begin = impure { _: "acc_begin" } : bits(64) -> unit
val acc_push = impure { _: "acc_push" } : bits(8) -> unit
val acc_push8 = impure { _: "acc_push8" } : bits(64) -> unit
val acc_exec = impure { _: "acc_exec" } : unit -> bits(64)
val acc_ok = impure { _: "acc_ok" } : unit -> bits(8)
val acc_out = impure { _: "acc_out" } : bits(64) -> bits(8)
val acc_word = impure { _: "acc_word" } : bits(64) -> bits(64)
val acc_in_byte = impure { _: "acc_in_byte" } : bits(64) -> bits(8)
val acc_begin_mem = impure { _: "acc_begin_mem" } : (bits(64), bits(64), bits(64)) -> unit
val acc_begin_txd = impure { _: "acc_begin_txd" } : bits(64) -> unit

/* Feed a message to the host accelerator, 8 bytes per FFI crossing (the
 * sub-8 remainder one byte at a time). */
function acc_push_all(msg : list(byte)) -> unit = match msg {
    b0 :: (b1 :: (b2 :: (b3 :: (b4 :: (b5 :: (b6 :: (b7 :: r))))))) => {
        acc_push8(append(b0, append(b1, append(b2, append(b3, append(b4, append(b5, append(b6, b7))))))));
        acc_push_all(r)
    },
    b :: r => {
        acc_push(b);
        acc_push_all(r)
    },
    [||] => (),
}

/* Read `n` output bytes back from the host accelerator into a list. */
function acc_read(n : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = acc_out(get_slice_int(64, i, 0)) :: out
    };
    list_reverse(out)
}

/* A 32-byte-digest hash over the accelerator (id 0 = keccak256, id 2 =
 * sha256): hash `msg`, folding the 32 output bytes big-endian into a word. */
function acc_hash(id : int, msg : list(byte)) -> bits(256) = {
    acc_begin(get_slice_int(64, id, 0));
    acc_push_all(msg);
    let _ = acc_exec();
    let w0 : bits(64) = acc_word(get_slice_int(64, 0, 0)); /* digest as 4 big-endian 64-bit words */
    let w1 : bits(64) = acc_word(get_slice_int(64, 1, 0));
    let w2 : bits(64) = acc_word(get_slice_int(64, 2, 0));
    let w3 : bits(64) = acc_word(get_slice_int(64, 3, 0));
    append(w0, append(w1, append(w2, w3)))
}

/* ===================== (2) HOST CRYPTO ================================= */

/* The i-th byte of the STAGED precompile input, zero past the end. A
 * precompile's input is staged once (a snapshot of the caller's memory slice
 * for a CALL, or the tx data for a tx-to-precompile); gas inspection (the
 * MODEXP length header, the BLAKE2F rounds count) reads only the leading
 * bytes it needs, directly from the staged buffer. */
function precompile_input_byte(i : int) -> byte = acc_in_byte(get_slice_int(64, i, 0))

/* Stage a precompile's input from the streamed tx buffer (a transaction
 * sent directly to a precompile; CALL-family staging is memory-sourced --
 * see stage_precompile_input in host/memory.sail). */
function stage_precompile_txd(num : int) -> unit = acc_begin_txd(get_slice_int(64, num, 0))

/* Run the staged precompile (input already in the accelerator: acc_begin_mem
 * for a memory slice, acc_begin + acc_push_all for a list). Output stays in
 * the accelerator buffer for returndata adoption (hr_capture_acc). Returns
 * false on precompile failure, which the interpreter maps to a CALL result
 * of 0 with empty returndata. */
function run_precompile_staged() -> bool = {
    let _ = acc_exec();
    unsigned(acc_ok()) == 1
}

/* List-in / list-out precompile wrapper (precompile vector tests; the EVM
 * itself uses the staged path above). */
function run_precompile(num : int, input : list(byte)) -> (bool, list(byte)) = {
    acc_begin(get_slice_int(64, num, 0));
    acc_push_all(input);
    let n = unsigned(acc_exec());
    let ok = unsigned(acc_ok()) == 1;
    if not_bool(ok) then {
        (false, [||])
    } else {
        (true, acc_read(n))
    }
}

/* ======================================================================== */
/* HASH PRIMITIVES + CREATE ADDRESS DERIVATION                              */
/*                                                                          */
/* keccak256 and sha256 route through the same accelerator boundary as the  */
/* precompiles (acc_hash; id 0 = keccak256, id 2 = sha256). The model       */
/* defines what is hashed; the host computes the digest behind the standard */
/* accelerator header.                                                      */
/* ======================================================================== */

/* Keccak-256, the Ethereum hash function: the KECCAK256 opcode, trie/RLP
 * node references, code hashes, and CREATE2 init-code commitments. id 0. */
function keccak256(msg : list(byte)) -> bits(256) = acc_hash(0, msg)

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

/* CREATE address (Yellow Paper): keccak256(rlp([sender, nonce]))[12:] -- the
 * low 20 bytes of the hash of the RLP-encoded (sender, nonce) pair. */
function create_address(sender : address, nonce : int) -> address = {
    let items = list_append(list_append([|0x94|], address_to_bytes(sender)), rlp_nonce(nonce));
    let payload = list_append([|get_slice_int(8, 192 + list_len(items), 0)|], items);
    word_to_address(keccak256(payload))
}

/* CREATE2 address (EIP-1014): the low 20 bytes of
 * keccak256(0xff ++ sender ++ salt ++ keccak256(init_code)) -- a salted,
 * init-code-committed address independent of the sender's nonce. */
function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var buf : list(byte) = [|0xff|];
    buf = list_append(buf, address_to_bytes(sender));
    buf = list_append(buf, word_to_bytes32(salt));
    buf = list_append(buf, word_to_bytes32(init_hash));
    word_to_address(keccak256(buf))
}

/* SHA-256 (id 2): the precompile 0x02 hash and the SSZ merkleization hash --
 * distinct from the keccak256 used throughout execution. */
function sha256(msg : list(byte)) -> bits(256) = acc_hash(2, msg)

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    acc_hash(2, list_append(word_to_bytes32(a), word_to_bytes32(b)))

/* SHA-256 self-check against the standard "" and "abc" reference vectors. */
function sha256_selfcheck() -> unit = {
    assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855, "sha256('') on guest");
    assert(
        sha256([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}

/* ===================== (3) STATELESS SSZ INPUT ========================= */

val ssz_src_len = impure { _: "ssz_src_len" } : unit -> int
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
let PL_BLOCK_NUMBER : int = 404
let PL_GAS_LIMIT : int = 412
let PL_GAS_USED : int = 420
let PL_TIMESTAMP : int = 428
let PL_EXTRA_OFF : int = 436
let PL_BASE_FEE : int = 440
let PL_BLOCK_HASH : int = 472
let PL_TXS_OFF : int = 504
let PL_WDS_OFF : int = 508
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
function pl_gas_limit() -> int = ssz_u64(payload_start() + PL_GAS_LIMIT)
function pl_gas_used() -> int = ssz_u64(payload_start() + PL_GAS_USED)
function pl_timestamp() -> int = ssz_u64(payload_start() + PL_TIMESTAMP)
function pl_fee_recipient() -> address = ssz_addr(payload_start() + PL_FEE_RECIPIENT)
function pl_prev_randao() -> bits(256) = ssz_bytes32(payload_start() + PL_PREV_RANDAO)
function pl_base_fee() -> word = ssz_u256(payload_start() + PL_BASE_FEE)
function pl_state_root() -> bits(256) = ssz_bytes32(payload_start() + PL_STATE_ROOT)
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

/* block_access_list (simplified): u32-LE count, then count*(addr[20] ++ slot[32 BE]).
 * bal_load materializes the permitted (addr,slot) set; the kernel's k_sload/k_sstore
 * panic on any access outside it (k_bal_enable / bal_guard). */
function bal_count() -> int = ssz_u32(pl_bal_start())
function bal_load() -> list(SlotKey) = {
    let base = pl_bal_start() + 4;
    let n = bal_count();
    var xs : list(SlotKey) = [||];
    foreach (i from 0 to (n - 1)) {
        xs = struct { sk_addr = ssz_addr(base + i * 52), sk_slot = ssz_bytes32(base + i * 52 + 20) } :: xs
    };
    xs
}

/* withdrawals: FIXED 44-byte SszWithdrawal elements, span [wds, bal) */
function pl_n_withdrawals() -> int = ediv_int(pl_bal_start() - pl_wds_start(), 44)
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
 * SszForkConfig: fork u64 @ 0 (ProtocolFork index; Amsterdam = 20). */
function cc_chain_id() -> int = ssz_u64(cc_start() + 0)
function cc_active_fork_start() -> int = {
    let c = cc_start();
    c + ssz_u32(c + 8)
}
function cc_fork() -> int = ssz_u64(cc_active_fork_start() + 0)

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
function ssz_witness_node(i : int) -> list(byte) =
    ssz_list_elem(wit_state_start(), wit_n_state(), wit_codes_start(), i)
function ssz_witness_node_span(i : int) -> (int, int) =
    ssz_list_span(wit_state_start(), wit_n_state(), wit_codes_start(), i)
function ssz_witness_code(i : int) -> list(byte) =
    ssz_list_elem(wit_codes_start(), wit_n_codes(), wit_headers_start(), i)
function ssz_witness_code_span(i : int) -> (int, int) =
    ssz_list_span(wit_codes_start(), wit_n_codes(), wit_headers_start(), i)
function ssz_witness_header(i : int) -> list(byte) =
    ssz_list_elem(wit_headers_start(), wit_n_headers(), cc_start(), i)

/* transactions[i] raw bytes (legacy = RLP list; typed = type byte ++ payload) */
