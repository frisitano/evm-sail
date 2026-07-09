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
/* for expansion gas; the bytes themselves live in C (ffi/memory.c) for     */
/* O(1) access, since Sail has no in-model mutable byte array.              */
/* ======================================================================== */

/* ======================================================================== */
/* C-BACKED HOST BINDINGS (per-frame execution buffers)                     */
/*                                                                          */
/* The extern declarations for the per-call-frame C buffers this file and   */
/* the evm layer drive: the operand stack (ffi/stack.c), EVM memory +       */
/* returndata + calldata + tx-input staging (ffi/memory.c, returndata.c),   */
/* and the active frame code view (ffi/code_db.c). Declared here, before    */
/* this file's own byte primitives and every evm-layer consumer. The `c:`   */
/* extern binds ONLY the C executable; proof targets see bodyless           */
/* axioms, so each stands for its abstract per-frame list-of-words model.   */
/* read_byte_slice (bottom) has no C binding: it is the spec-side read      */
/* axiom, uncalled on the build path.                                       */
/* ======================================================================== */

/* ---- operand stack (ffi/stack.c): per-frame flat word arrays, O(1)
 * push/pop/peek/set. hs_enter on a sub-call, hs_leave on return, hs_reset per
 * transaction. ---- */
val hs_reset = impure { c: "hs_reset" } : unit -> unit
val hs_enter = impure { c: "hs_enter" } : unit -> unit
val hs_leave = impure { c: "hs_leave" } : unit -> unit
val hs_depth = impure { c: "hs_depth" } : unit -> bits(64)
val hs_push_w = impure { c: "hs_push_w" } : word -> unit
val hs_pop = impure { c: "hs_pop" } : unit -> unit
val hs_peek_w = impure { c: "hs_peek_w" } : bits(64) -> word
val hs_set_w = impure { c: "hs_set_w" } : (bits(64), word) -> unit

/* ---- EVM memory (ffi/memory.c): read-after-write, LIFO frame push/pop. ---- */
val host_mem_read = impure { c: "host_mem_read" } : bits(64) -> bits(8)
val host_mem_write = impure { c: "host_mem_write" } : (bits(64), bits(8)) -> unit
val host_mem_reset = impure { c: "host_mem_reset" } : unit -> unit
val host_mem_push = impure { c: "host_mem_push" } : unit -> unit
val host_mem_pop = impure { c: "host_mem_pop" } : unit -> unit

/* range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY): endpoints
 * already live in C buffers, so copies stay C-side. */
val hm_move = impure { c: "hm_move" } : (bits(64), bits(64), bits(64)) -> unit

/* ---- RETURNDATA buffer (per frame; ffi/returndata.c): the bytes a child frame
 * returned or a precompile produced (EIP-211), cleared per sub-call. ---- */
val returndata_reset_all = impure { c: "returndata_reset_all" } : unit -> unit
val returndata_clear_current = impure { c: "returndata_clear_current" } : unit -> unit
val returndata_discard_pending = impure { c: "returndata_discard_pending" } : unit -> unit
val returndata_capture_memory = impure { c: "returndata_capture_memory" } : (bits(64), bits(64)) -> unit
val returndata_adopt_pending = impure { c: "returndata_adopt_pending" } : unit -> unit
val returndata_current_len = impure { c: "returndata_current_len" } : unit -> bits(64)
val returndata_pending_len = impure { c: "returndata_pending_len" } : unit -> bits(64)
val returndata_pending_byte_at = impure { c: "returndata_pending_byte" } : bits(64) -> bits(8)
val returndata_copy_current_to_memory = impure { c: "returndata_copy_current_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val returndata_copy_current_prefix_to_memory = impure { c: "returndata_copy_current_prefix_to_memory" } : (
        bits(64),
        bits(64),
    ) -> unit

/* ---- CALLDATA (per frame; ffi/code_db.c frame-buffer machinery): the frame's
 * input data (CALLDATALOAD/SIZE/COPY). ---- */
val cd_set = impure { c: "cd_set" } : (bits(64), bits(64)) -> unit
val cd_set_empty = impure { c: "cd_set_empty" } : unit -> unit
val cd_set_tx = impure { c: "cd_set_tx" } : unit -> unit

/* Byte-loop-free tx-input staging (each done C-side in one call).
 * txdata_set_source stages a byte source (the witness span / native-runner
 * input) and txdata_set_word a 32-byte word (a block system call); each returns
 * the staged length. txd_count_nonzero counts nonzero staged bytes for the
 * EIP-2028 / EIP-7623 calldata gas. */
val txin_set_from_source = impure { c: "txin_set_from_source" } : (bits(64), bits(64), bits(64)) -> bits(64)
function txdata_set_source(s : ByteSlice) -> int =
    unsigned(txin_set_from_source(byte_source_kind(s.source), s.off, s.len))
val txin_set_word = impure { c: "txin_set_word" } : word -> bits(64)
function txdata_set_word(w : word) -> int = unsigned(txin_set_word(w))
val txd_count_nonzero = impure { c: "txd_count_nonzero" } : unit -> bits(64)

/* Streamed tx-input staging: retained ONLY for the native-runner pre-state
 * decode path (a scalar at a time); the stateless guest stages the span in one
 * memcpy. */
val txin_begin = impure { c: "txin_begin" } : unit -> unit
val txin_byte = impure { c: "txin_byte" } : bits(8) -> unit
val cd_to_mem = impure { c: "cd_to_mem" } : (bits(64), bits(64), bits(64)) -> unit

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded past end). */
val calldata_load_word = impure { c: "calldata_load_word" } : bits(64) -> word
val txd_at = impure { c: "txd_at" } : bits(64) -> bits(8)
val txd_length = impure { c: "txd_length" } : unit -> bits(64)

/* ---- active frame code view (ffi/code_db.c, sharing frame-buffer machinery):
 * frame_code_bind_* binds where the frame's code comes from -- a store entry by
 * codeHash, a caller-memory slice (CREATE init code, copied before the frame
 * switch so executing code never aliases mutable memory), or the tx input --
 * and returns the bound code's length. ---- */
val frame_code_bind_stored = impure { c: "frame_code_bind_stored" } : hash -> bits(64)
val frame_code_bind_memory = impure { c: "frame_code_bind_memory" } : (bits(64), bits(64)) -> bits(64)
val frame_code_bind_tx_input = impure { c: "frame_code_bind_tx_input" } : unit -> bits(64)
val frame_code_clear = impure { c: "frame_code_clear" } : unit -> unit
val frame_code_length = impure { c: "frame_code_length" } : unit -> bits(64)
val frame_code_byte = impure { c: "frame_code_byte" } : bits(64) -> bits(8)
val frame_jumpdest_valid = impure { c: "frame_jumpdest_valid" } : bits(64) -> bool
val frame_push_immediate_word = impure { c: "frame_push_immediate_word" } : (bits(64), bits(64)) -> word
val frame_code_copy_to_memory = impure { c: "frame_code_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit

/* Source -> bytes: the read side of the byte-source interface (no C binding).
 * Build hashes/stages each source C-side without materializing, so this is
 * uncalled there; spec uses it as the abstract read axiom the crypto and
 * precompile refinements are written over. Law: read_byte_slice(s) yields
 * exactly s.len bytes of source s starting at offset s.off. */
val read_byte_slice : ByteSlice -> list(byte)

/* kept only as the type of the (now unused) StackFrame snapshot field */
struct MemByte = { offset : int, value : byte }

/* The high-water mark, in bytes: the largest offset ever touched in this
 * frame. MSIZE rounds it up to a word; memory-expansion gas is charged on
 * each growth of mem_size_words (Yellow Paper section 9.4.1, the C_mem term). */
register memory_size : int = 0

/* call-frame memory lifecycle (the C side owns the bytes) */
function mem_clear() -> unit = host_mem_reset()
function mem_frame_enter() -> unit = host_mem_push()
function mem_frame_leave() -> unit = host_mem_pop()

/* --- raw byte access (O(1), C-backed) --- */

/* frame liveness (defined in evm/machine.sail); forward-declared so the byte
 * primitives below can no-op when the frame has exceptionally halted. */
val is_running : unit -> bool

/* A memory access only follows a successful charge_expansion; if that charge ran
 * the frame out of gas (exc_halt sets the status but does not abort the opcode
 * handler), the access must NOT touch memory -- otherwise an out-of-gas write to
 * a huge offset (e.g. MSTORE at 2^32) would grow the C buffer to gigabytes. */
function mem_get_byte(off : int) -> byte =
    if is_running() then {
        host_mem_read(get_slice_int(64, off, 0))
    } else {
        0x00
    }

function mem_set_byte(off : int, v : byte) -> unit =
    if is_running() then {
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

/* MCOPY (EIP-5656): overlapping-safe memory-to-memory copy (one C memmove). */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    hm_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* CODECOPY: the current frame's code (already in the C buffer) into memory.
 * Clamp a beyond-2^64 source offset to a beyond-any-code sentinel (see
 * calldata_copy_to_mem): a past-end source must zero-fill the destination. */
function mem_codecopy(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    frame_code_copy_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}

/* KECCAK256 over the memory range [off, off+len). Semantically, the caller
 * reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : int, len : int) -> word =
    bytes_keccak(memory_byte_slice(off, len))

function returndata_size() -> int = unsigned(returndata_current_len())
function returndata_capture(off : int, len : int) -> unit =
    returndata_capture_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    returndata_copy_current_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_out_region(dst : int, want : int) -> unit =
    returndata_copy_current_prefix_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, want, 0))

function returndata_pending_size() -> int = unsigned(returndata_pending_len())
function returndata_pending_byte(i : int) -> byte =
    returndata_pending_byte_at(get_slice_int(64, i, 0))

/* Materialize the pending child output (the init-code return value a CREATE
 * deploys as the new account's code) as a Sail list. */
function returndata_pending_bytes() -> list(byte) = {
    let n = returndata_pending_size();
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = returndata_pending_byte(n - 1 - i) :: out
    };
    out
}

/* The transaction input, streamed into the C buffer ONCE per tx. Intrinsic-gas
 * inspection (EIP-2028 / EIP-7623 zero vs nonzero byte counting) and length
 * checks read it here, byte-addressed, rather than re-walking a Sail list. */
function txdata_byte(i : int) -> byte = txd_at(get_slice_int(64, i, 0))
function txdata_size() -> int = unsigned(txd_length())
function txdata_read_range(off : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = txdata_byte(off + (len - 1 - k)) :: out
    };
    out
}

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel (as
 * calldata_copy_to_mem does): the 64-bit truncation below would otherwise wrap a
 * huge offset back into real calldata. A past-end CALLDATALOAD reads all zeros. */
function calldata_word(off : int) -> word =
    calldata_load_word(get_slice_int(64, if off > 16777216 then { 16777216 } else { off }, 0))

/* clamp a beyond-2^64 source offset to a beyond-any-calldata sentinel so the
 * 64-bit truncation cannot wrap back into real calldata: a past-end source
 * offset must zero-fill the whole destination (EELS buffer_read). */
function calldata_copy_to_mem(dst : int, off : int, len : int) -> unit = {
    let o = if off > 16777216 then { 16777216 } else { off };
    cd_to_mem(get_slice_int(64, dst, 0), get_slice_int(64, o, 0), get_slice_int(64, len, 0))
}
function calldata_set_next(off : int, len : int) -> unit =
    cd_set(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
