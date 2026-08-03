/* Frames form a stack in the backing arena. Sail owns the active frame's   */
/* absolute EvmMemorySlice and saves/restores it with the other frame registers; */
/* this boundary only allocates, establishes, and releases arena frames.    */

/*md
# Per-frame memory

The volatile byte array each call frame addresses from zero (YP §9.4.1):
flat, byte-addressed, zero-initialized, and lazily expanding in 32-byte
words. `MLOAD`/`MSTORE`/`MSTORE8`/`MSIZE` and the `*COPY` opcodes read and
write it; expansion past the high-water mark is charged as
memory-expansion gas.

Frames form a stack: a sub-call receives fresh memory
([mem_frame_enter][]) and the caller's is restored on return
([mem_frame_leave][]); [mem_clear][] resets to a single empty frame per
transaction. This module is only the boundary — bodyless axioms with
read-after-write and LIFO frame laws; the EVM's view (out-of-gas guard,
high-water tracking, `MSIZE`, the opcode helpers) lives in the machine
module where the frame context is defined.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The byte at `off` of the current frame's memory; zero when never
    written. */
val mem_read_byte = impure { c: "mem_read_byte" } : memory_pointer -> byte
/*! Writes one byte at `off` of the current frame's memory (`MSTORE8`). */
val mem_write_byte = impure { c: "mem_write_byte" } : (memory_pointer, byte) -> unit

/* call-frame memory lifecycle (the implementation owns the bytes) */
/*! Resets memory to a single empty frame (per-transaction reset). */
val mem_clear = impure { c: "mem_clear" } : unit -> unit
/*! Pushes a fresh, zeroed memory for a sub-call frame. */
val mem_frame_enter = impure { c: "mem_frame_enter" } : unit -> memory_pointer
/*! Pops the current frame's memory, restoring the caller's. */
val mem_frame_leave = impure { c: "mem_frame_leave" } : unit -> unit

/* range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY): endpoints
 * already live in host-interface regions, so copies never cross the
 * interface. */
/*! Materializes an EVM-memory extent whose protocol gas has already been
    charged and returns the current frame's absolute source slice. Failure to
    realize this established precondition is a host implementation failure,
    not an EVM out-of-gas result. */
val mem_expand = impure { c: "mem_expand" } : forall 'required, memory_region_valid_range(0, 'required).
    int('required) -> EvmMemorySliceLength(
        'required,
    )

/*! Copies a byte range within the current frame's memory; overlapping
    ranges behave as one atomic move (`MCOPY` and the destination side of
    the `*COPY` opcodes). */
val mem_move = impure { c: "mem_move" } : (memory_pointer, memory_pointer, memory_length) -> unit

/* word access: MLOAD/MSTORE read/write the 32-byte big-endian range at off in
 * ONE crossing (YP 9.4.1) */
/*! Reads the 32-byte big-endian word at `off` in one interface crossing
    (`MLOAD`). */
val mem_load_word = impure { c: "mem_load_word" } : memory_pointer -> word
/*! Writes the 32-byte big-endian word at `off` in one interface
    crossing (`MSTORE`). */
val mem_store_word = impure { c: "mem_store_word" } : (memory_pointer, word) -> unit
