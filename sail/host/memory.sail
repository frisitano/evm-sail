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
/* transaction. This file is ONLY the boundary -- the bodyless axioms with  */
/* read-after-write and LIFO frame laws; the EVM's view (out-of-gas guard,  */
/* high-water tracking, MSIZE, the opcode helpers) lives in                 */
/* evm/machine.sail where the frame context is defined.                     */
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

/* word access: MLOAD/MSTORE read/write the 32-byte big-endian range at off in
 * ONE crossing (YP 9.4.1; EELS memory_read_bytes / memory_write) */
val mem_load_word = impure { c: "mem_load_word" } : bits(64) -> word
val mem_store_word = impure { c: "mem_store_word" } : (bits(64), word) -> unit

/* Beyond-any-source sentinel (2^24 > any code/calldata/returndata length):
 * huge source offsets clamp here so the 64-bit truncation at the FFI cannot
 * wrap them back into real data; a past-end source reads as zeros /
 * zero-fills its destination (EELS buffer_read). */
let SOURCE_PAST_END : int = 16777216

function clamp_source_offset(off : int) -> int = if off > SOURCE_PAST_END then { SOURCE_PAST_END } else { off }
