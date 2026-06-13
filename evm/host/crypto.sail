/* ======================================================================== */
/* HOST crypto functions: the EVM precompiles and staged-input access        */
/*                                                                           */
/* Crypto is host mechanism, like memory and storage: the model defines      */
/* WHAT is hashed/verified (and all gas); the host computes it. keccak256    */
/* and sha256 are defined where they are used (lib/keccak.sail,              */
/* lib/sha256.sail) over the same binding. The acc_* byte-stream protocol    */
/* itself is confined to host/ffi.sail -- nothing outside the host files     */
/* speaks it.                                                                */
/* ======================================================================== */

/* The i-th byte of the STAGED precompile input, zero past the end. A
 * precompile's input is staged once (a snapshot of the caller's memory slice,
 * or the tx data); gas inspection (modexp header, blake2f rounds) reads the
 * few bytes it needs O(1) -- no Sail byte list. */
function precompile_input_byte(i : int) -> byte = acc_in_byte(get_slice_int(64, i, 0))

/* stage a precompile's input from the streamed tx buffer (tx-to-precompile;
 * CALL-family staging is memory-sourced: stage_precompile_input, memory.sail) */
function stage_precompile_txd(num : int) -> unit = acc_begin_txd(get_slice_int(64, num, 0))

/* run the staged precompile (input already in the accelerator: acc_begin_mem
 * for a memory slice, acc_begin+acc_push_all for a list). The output stays in
 * the accelerator buffer for returndata adoption (hr_capture_acc). On failure
 * the CALL returns 0 (handled by the interpreter). */
function run_precompile_staged() -> bool = {
    let _ = acc_exec();
    unsigned(acc_ok()) == 1
}

/* list-in/list-out wrapper (vector tests; the EVM uses the staged path) */
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
/* Hash primitives + CREATE address derivation                              */
/*                                                                           */
/* keccak256/sha256 route through the same accelerator boundary as the       */
/* precompiles (acc_hash; ids 0 = keccak256, 2 = sha256). Native links the    */
/* reference impl; a real zkVM swaps in a native keccak/sha256 precompile     */
/* behind the standard header. The pure-Sail reference permutations live in   */
/* git history. (Consolidated here from the former lib/keccak.sail +          */
/* lib/sha256.sail, which were thin wrappers once the permutations moved to C.) */
/* ======================================================================== */

/* Keccak-256, the Ethereum hash (KECCAK256 opcode, trie/RLP, CREATE2). id 0. */
function keccak256(msg : list(byte)) -> bits(256) = acc_hash(0, msg)

/* big-endian byte serializations used by RLP, the tries, and address derivation */
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

/* RLP encoding of the nonce for CREATE address derivation. */
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

/* CREATE (YP): address = keccak256(rlp([sender, nonce]))[12:] */
function create_address(sender : address, nonce : int) -> address = {
    let items = list_append(list_append([|0x94|], address_to_bytes(sender)), rlp_nonce(nonce));
    let payload = list_append([|get_slice_int(8, 192 + list_len(items), 0)|], items);
    word_to_address(keccak256(payload))
}

/* CREATE2 (EIP-1014): address = keccak256(0xff ++ sender ++ salt ++ keccak(init))[12:] */
function create2_address(sender : address, salt : word, init_hash : hash) -> address = {
    var buf : list(byte) = [|0xff|];
    buf = list_append(buf, address_to_bytes(sender));
    buf = list_append(buf, word_to_bytes32(salt));
    buf = list_append(buf, word_to_bytes32(init_hash));
    word_to_address(keccak256(buf))
}

/* SHA-256 (id 2): the precompile 0x02 hash and the SSZ merkleization hash
 * (distinct from the execution keccak). */
function sha256(msg : list(byte)) -> bits(256) = acc_hash(2, msg)

/* SSZ merkle parent: SHA-256(left ++ right), both 32-byte chunks */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
    acc_hash(2, list_append(word_to_bytes32(a), word_to_bytes32(b)))

/* self-check: SHA-256 reference vectors (run on the guest) */
function sha256_selfcheck() -> unit = {
    assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855, "sha256('') on guest");
    assert(
        sha256([|0x61, 0x62, 0x63|]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
        "sha256('abc') on guest"
    )
}
