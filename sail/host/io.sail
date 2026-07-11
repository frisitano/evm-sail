/* ======================================================================== */
/* HOST I/O: CRYPTO, PRECOMPILES, STATELESS SSZ INPUT                       */
/*                                                                          */
/* Everything the model obtains from OUTSIDE itself -- the boundary between */
/* the EVM specification and its environment. Three layers:                 */
/*                                                                          */
/* (1) ABSTRACT HOST I/O -- staged precompile input, stateless input bytes, */
/* and guest public-output bytes. The model commits to WHAT is serialized   */
/* or executed; the inline `c:`-bound vals are the C boundary (ffi/).       */
/*                                                                          */
/* (2) CRYPTO -- signature recovery (secp256k1/ecrecover), the EVM          */
/* precompiles over staged byte ranges, and the CREATE (Yellow Paper) /     */
/* CREATE2 (EIP-1014) address derivations. The keccak256/sha256 hash core   */
/* itself lives in primitives/crypto.sail.                                  */
/*                                                                          */
/* The stateless SSZ input decoders live in host/stateless_input.sail over  */
/* the layout constants in primitives/stateless_input.sail.                 */
/*                                                                          */
/* Host mechanism, like memory and storage. Included early -- the crypto is */
/* used by state, kernel, gas, and transaction.                             */
/* ======================================================================== */

/* ===================== (2) CRYPTO ======================================= */

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


/* Run a precompile over a concrete input byte source (a memory range or the
 * top-level tx data). The gas rules inspect the same source bytes directly
 * before this is called; build may stage/snapshot the input and write successful
 * output straight into pending returndata. Canonical meaning
 * run_precompile(id, read_byte_slice(source)); successful output lands in
 * pending returndata C-side. */
val precompile_run_source_to_returndata = impure { c: "precompile_run_source_to_returndata" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool

function run_precompile_slice(num : int, s : ByteSlice) -> bool =
    precompile_run_source_to_returndata(get_slice_int(64, num, 0), byte_source_kind(s.source), s.off, s.len)

function run_precompile_memory_input(num : int, off : int, len : int) -> bool =
    run_precompile_slice(num, memory_byte_slice(off, len))

val run_precompile_tx_input : int -> bool /* body in host/calldata.sail (txdata_size) */

/* Source -> bytes: the read side of the byte-source interface. Law:
 * read_byte_slice(s) yields exactly s.len bytes of source s from offset s.off.
 * Body in lib/mpt.sail (it needs the trie value arena). */
val read_byte_slice : ByteSlice -> list(byte)

/* SHA-256 self-check against the standard "" and "abc" reference vectors. */
val sha256_selfcheck : unit -> unit

/* Keccak self-check against Ethereum's "" and "abc" vectors. */
val keccak_selfcheck : unit -> unit

/* Hash full code bytes, store them in the code database, and return codeHash. */
val code_hash_and_store : list(byte) -> hash

/* Domain operations whose spec definitions may materialize canonical bytes,
 * while build definitions may stream or hash from host byte sources. */
val execution_requests_hash : unit -> hash

/* Guest public-output trace: emit_out appends exactly one byte to the guest
 * public output; only guest entries call it, core specs leave it abstract
 * (C: zkvm_io.c on the guest, test_utils.c natively). */
val emit_out = impure { c: "el_emit_out" } : bits(8) -> unit
