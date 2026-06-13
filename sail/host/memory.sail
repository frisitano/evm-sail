/* ======================================================================== */
/* PER-FRAME EVM MEMORY                                                     */
/*                                                                          */
/* The volatile byte array each call frame addresses from zero (Yellow      */
/* Paper section 9.4.1): flat, byte-addressed, zero-initialized, and lazily */
/* expanding in 32-byte words. MLOAD/MSTORE/MSTORE8/MSIZE and the *COPY     */
/* opcodes read and write it; expansion past the high-water mark is charged */
/* as memory-expansion gas.                                                 */
/*                                                                          */
/* Frames form a stack: a sub-call (CALL/CREATE) receives fresh memory      */
/* (mem_frame_enter) and the caller's is restored on return                 */
/* (mem_frame_leave); mem_clear resets to a single empty frame per          */
/* transaction. The Sail side tracks only the high-water mark (memory_size) */
/* for expansion gas; the bytes themselves live in C (ffi/host_mem.c) for   */
/* O(1) access, since Sail has no in-model mutable byte array.              */
/* ======================================================================== */

val host_mem_read = impure { _: "host_mem_read" } : bits(64) -> bits(8)
val host_mem_write = impure { _: "host_mem_write" } : (bits(64), bits(8)) -> unit
val host_mem_reset = impure { _: "host_mem_reset" } : unit -> unit
val host_mem_push = impure { _: "host_mem_push" } : unit -> unit
val host_mem_pop = impure { _: "host_mem_pop" } : unit -> unit

/* kept only as the type of the (now unused) StackFrame snapshot field */
struct MemByte = { mb_off : int, mb_val : byte }

/* The high-water mark, in bytes: the largest offset ever touched in this
 * frame. MSIZE rounds it up to a word; memory-expansion gas is charged on
 * each growth of mem_size_words (Yellow Paper section 9.4.1, the C_mem term). */
register memory_size : int = 0

/* call-frame memory lifecycle (the C side owns the bytes) */
function mem_clear() -> unit = host_mem_reset()
function mem_frame_enter() -> unit = host_mem_push()
function mem_frame_leave() -> unit = host_mem_pop()

/* --- raw byte access (O(1), C-backed) --- */

function mem_get_byte(off : int) -> byte = host_mem_read(get_slice_int(64, off, 0))

function mem_set_byte(off : int, v : byte) -> unit = {
    host_mem_write(get_slice_int(64, off, 0), v);
    if off + 1 > memory_size then {
        memory_size = off + 1
    }
}

/* --- word access (big-endian, 32 bytes): MLOAD / MSTORE / MSTORE8 --- */

function mem_load(off : int) -> word = {
    var acc : word = word_of_int(0);
    foreach (k from 0 to 31) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(mem_get_byte(off + k), 256))
    };
    acc
}

function mem_store(off : int, w : word) -> unit = {
    foreach (k from 0 to 31) {
        let shift = 8 * (31 - k);
        let b : byte = sail_shiftright(w, shift)[7 .. 0];
        mem_set_byte(off + k, b)
    }
}

function mem_store_byte(off : int, w : word) -> unit =
    mem_set_byte(off, w[7 .. 0])

/* --- byte-range read (for KECCAK / RETURN / LOG / CALL args), as a list --- */

/* Read mem[off .. off+len) into a list, in address order. */
function mem_read_range(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||]; /* prepend from the high end so the list ends in address order */
    foreach (k from 0 to (len - 1)) {
        out = mem_get_byte(off + (len - 1 - k)) :: out
    };
    out
}

/* Current memory size in 32-byte words, rounded up (the MSIZE value, and the
 * unit of memory-expansion gas). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/* ---- range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY):
 * zero/one-copy, since both endpoints already live in the C buffers ---- */
val hm_move = impure { _: "hm_move" } : (bits(64), bits(64), bits(64)) -> unit
val hc_to_mem = impure { _: "hc_to_mem" } : (bits(64), bits(64), bits(64)) -> unit

/* Stage a precompile's input: ONE C-side snapshot of mem[off .. off+len) -- the
 * child message's calldata. A snapshot, not an alias: precompile 0x04 (identity)
 * may have its output written back over a region overlapping its input. */
function stage_precompile_input(id : int, off : int, len : int) -> unit =
    acc_begin_mem(get_slice_int(64, id, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* MCOPY (EIP-5656): overlapping-safe memory-to-memory copy (one C memmove). */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    hm_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* CODECOPY: the current frame's code (already in the C buffer) into memory.
 * Clamp a beyond-2^64 source offset to a beyond-any-code sentinel (see
 * calldata_copy_to_mem): a past-end source must zero-fill the destination. */
function mem_codecopy(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    hc_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}

/* KECCAK256 directly over the memory range [off, off+len): the bytes go from
 * the C memory frame into the C accelerator without an intermediate Sail list,
 * folding the digest big-endian into a word. */
function mem_keccak(off : int, len : int) -> word = {
    acc_begin_mem(get_slice_int(64, 0, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0));
    let _ = acc_exec();
    let w0 : bits(64) = acc_word(get_slice_int(64, 0, 0));
    let w1 : bits(64) = acc_word(get_slice_int(64, 1, 0));
    let w2 : bits(64) = acc_word(get_slice_int(64, 2, 0));
    let w3 : bits(64) = acc_word(get_slice_int(64, 3, 0));
    append(w0, append(w1, append(w2, w3)))
}

/* ---- C-backed RETURNDATA buffer (per call frame; see ffi/acc_shim.c) ----
 * The bytes a child frame returned (RETURN/REVERT) or a precompile produced,
 * read by RETURNDATASIZE/RETURNDATACOPY (EIP-211). Cleared at the start of
 * every sub-call and replaced by the child's output on return. ---- */
val hr_reset = impure { _: "hr_reset" } : unit -> unit
val hr_clear = impure { _: "hr_clear" } : unit -> unit
val hr_discard = impure { _: "hr_discard" } : unit -> unit
val hr_capture = impure { _: "hr_capture" } : (bits(64), bits(64)) -> unit
val hr_capture_acc = impure { _: "hr_capture_acc" } : unit -> unit
val hr_adopt = impure { _: "hr_adopt" } : unit -> unit
val hr_len = impure { _: "hr_len" } : unit -> bits(64)
val hr_pending_len = impure { _: "hr_pending_len" } : unit -> bits(64)
val hr_pending_byte = impure { _: "hr_pending_byte" } : bits(64) -> bits(8)
val hr_to_mem = impure { _: "hr_to_mem" } : (bits(64), bits(64), bits(64)) -> unit
val hr_out_region = impure { _: "hr_out_region" } : (bits(64), bits(64)) -> unit

function returndata_size() -> int = unsigned(hr_len())
function returndata_capture(off : int, len : int) -> unit =
    hr_capture(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    hr_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_out_region(dst : int, want : int) -> unit =
    hr_out_region(get_slice_int(64, dst, 0), get_slice_int(64, want, 0))

/* Materialize the pending child output (the init-code return value a CREATE
 * deploys as the new account's code) as a Sail list. */
function pending_output_list() -> list(byte) = {
    let n = unsigned(hr_pending_len());
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = hr_pending_byte(get_slice_int(64, n - 1 - i, 0)) :: out
    };
    out
}

/* ---- C-backed CALLDATA (per call frame) ----
 * The current frame's input data, read by CALLDATALOAD/CALLDATASIZE/CALLDATACOPY.
 * For a CALL-family child it aliases the slice of the caller's memory passed as
 * args; for the top-level frame it is the transaction input. ---- */
val cd_set = impure { _: "cd_set" } : (bits(64), bits(64)) -> unit
val cd_set_empty = impure { _: "cd_set_empty" } : unit -> unit
val cd_set_tx = impure { _: "cd_set_tx" } : unit -> unit
val txin_begin = impure { _: "txin_begin" } : bits(64) -> unit
val txin_byte = impure { _: "txin_byte" } : bits(8) -> unit
val txin_activate = impure { _: "txin_activate" } : bits(64) -> bits(64)
val cd_len = impure { _: "cd_len" } : unit -> bits(64)
val cd_word = impure { _: "cd_word" } : bits(64) -> word
val cd_to_mem = impure { _: "cd_to_mem" } : (bits(64), bits(64), bits(64)) -> unit
val txd_at = impure { _: "txd_at" } : bits(64) -> bits(8)
val txd_length = impure { _: "txd_length" } : unit -> bits(64)

/* The transaction input, streamed into the C buffer ONCE per tx. Intrinsic-gas
 * inspection (EIP-2028 / EIP-7623 zero vs nonzero byte counting) and length
 * checks read it here, byte-addressed, rather than re-walking a Sail list. */
function txdata_byte(i : int) -> byte = txd_at(get_slice_int(64, i, 0))
function txdata_size() -> int = unsigned(txd_length())

function calldata_word(off : int) -> word = cd_word(get_slice_int(64, off, 0))

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel so the
 * 64-bit truncation cannot wrap back into real calldata: a past-end source
 * offset must zero-fill the whole destination (EELS buffer_read). */
function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    cd_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}
function calldata_set_next(off : int, len : int) -> unit =
    cd_set(get_slice_int(64, off, 0), get_slice_int(64, len, 0))

/* load transaction `idx`'s input bytes into its memory slot (block setup: the
 * runner / stateless decoder / test fixtures call this once per tx). */
function txdata_set_input(idx : int, data : list(byte)) -> unit = {
    txin_begin(get_slice_int(64, idx, 0));
    var rest : list(byte) = data;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                txin_byte(b);
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* select transaction `idx` as the executing tx; returns its input length. The
 * EVM reads the input from memory (txdata_byte / txdata_size / calldata) -- it
 * holds only the index, never the bytes. */
function txdata_activate(idx : int) -> int = unsigned(txin_activate(get_slice_int(64, idx, 0)))
