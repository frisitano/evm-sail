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

/* Freeze a range of the current frame in the absolute memory arena. */
val mem_establish_absolute = impure { c: "mem_establish_absolute" } : (bits(64), bits(64)) -> bits(64)

/*! Freezes `[off, off+len)` of the current frame into the absolute
    memory arena and returns a slice over it (the source form of
    `RETURN`/`REVERT`/`LOG` data). */
function mem_slice(off : int, len : int) -> ByteSlice =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        let absolute = mem_establish_absolute(get_slice_int(64, off, 0), get_slice_int(64, len, 0));
        if absolute == 0xFFFFFFFFFFFFFFFF then {
            EMPTY_SLICE
        } else {
            struct { source = MemoryArenaSource, off = absolute, len = get_slice_int(64, len, 0) }
        }
    }
