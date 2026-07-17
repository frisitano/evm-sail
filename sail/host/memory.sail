/* ======================================================================== */
/* PER-FRAME EVM MEMORY                                                     */
/*                                                                          */
/* The volatile byte array each call frame addresses from zero (Yellow      */
/* Paper section 9.4.1): flat, byte-addressed, zero-initialized, and lazily */
/* expanding in 32-byte words. MLOAD/MSTORE/MSTORE8/MSIZE and the *COPY     */
/* opcodes read and write it; expansion past the high-water mark is charged */
/* as memory-expansion gas.                                                 */
/*                                                                          */
/* Frames form a stack in the backing arena. Sail owns the active frame's   */
/* absolute ByteSlice and saves/restores it with the other frame registers; */
/* this boundary only allocates, establishes, and releases arena frames.    */
/* ======================================================================== */

val mem_read_byte = impure { c: "mem_read_byte" } : byte_quantity -> bits(8)
val mem_write_byte = impure { c: "mem_write_byte" } : (byte_quantity, bits(8)) -> unit

/* call-frame memory lifecycle (the C side owns the bytes) */
val mem_clear = impure { c: "mem_clear" } : unit -> unit
val mem_frame_enter = impure { c: "mem_frame_enter" } : unit -> byte_quantity
val mem_frame_leave = impure { c: "mem_frame_leave" } : unit -> unit
val mem_expand = impure { c: "mem_expand" } : byte_quantity -> bool

/* range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY): endpoints
 * already live in C buffers, so copies stay C-side. */
val mem_move = impure { c: "mem_move" } : (byte_quantity, byte_quantity, byte_quantity) -> unit

/* word access: MLOAD/MSTORE read/write the 32-byte big-endian range at off in
 * ONE crossing (YP 9.4.1; EELS memory_read_bytes / memory_write) */
val mem_load_word = impure { c: "mem_load_word" } : byte_quantity -> word
val mem_store_word = impure { c: "mem_store_word" } : (byte_quantity, word) -> unit
