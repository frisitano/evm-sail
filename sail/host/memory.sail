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
/* O(1) access, since Sail has no in-model mutable byte array. Proof        */
/* targets see the vals as bodyless axioms with read-after-write and LIFO   */
/* frame push/pop laws.                                                     */
/* ======================================================================== */

val mem_read_byte = impure { c: "mem_read_byte" } : bits(64) -> bits(8)
val mem_write_byte = impure { c: "mem_write_byte" } : (bits(64), bits(8)) -> unit

/* call-frame memory lifecycle (the C side owns the bytes) */
val mem_clear = impure { c: "mem_clear" } : unit -> unit
val mem_frame_enter = impure { c: "mem_frame_enter" } : unit -> unit
val mem_frame_leave = impure { c: "mem_frame_leave" } : unit -> unit

/* range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY): endpoints
 * already live in C buffers, so copies stay C-side. */
val mem_move = impure { c: "mem_move" } : (bits(64), bits(64), bits(64)) -> unit

/* The high-water mark, in bytes: the largest offset ever touched in this
 * frame. MSIZE rounds it up to a word; memory-expansion gas is charged on
 * each growth of mem_size_words (Yellow Paper section 9.4.1, the C_mem term). */
register memory_size : int = 0

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
        mem_read_byte(get_slice_int(64, off, 0))
    } else {
        0x00
    }

function mem_set_byte(off : int, v : byte) -> unit =
    if is_running() then {
        mem_write_byte(get_slice_int(64, off, 0), v);
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

/* Current memory size in 32-byte words, rounded up (the MSIZE value, and the
 * unit of memory-expansion gas). */
function mem_size_words() -> int = ediv_int(memory_size + 31, 32)

/* MCOPY (EIP-5656): overlapping-safe memory-to-memory copy (one C memmove). */
function mem_mcopy(dst : int, src : int, len : int) -> unit =
    mem_move(get_slice_int(64, dst, 0), get_slice_int(64, src, 0), get_slice_int(64, len, 0))

/* Beyond-any-source sentinel (2^24 > any code/calldata/returndata length):
 * huge source offsets clamp here so the 64-bit truncation at the FFI cannot
 * wrap them back into real data; a past-end source reads as zeros /
 * zero-fills its destination (EELS buffer_read). */
let SOURCE_PAST_END : int = 16777216

function clamp_source_offset(off : int) -> int = if off > SOURCE_PAST_END then { SOURCE_PAST_END } else { off }

/* CODECOPY: the current frame's code (already in the C buffer) into memory. */
function mem_codecopy(dst : int, off : int, len : int) -> unit =
    frame_code_copy_to_memory(
        get_slice_int(64, dst, 0),
        get_slice_int(64, clamp_source_offset(off), 0),
        get_slice_int(64, len, 0),
    )

/* KECCAK256 over the memory range [off, off+len). Semantically, the caller
 * reads the addressed bytes and hashes that byte list. */
function mem_keccak(off : int, len : int) -> word =
    keccak256(memory_byte_slice(off, len))
