/* The host owns only the shared backing arena. Sail carries each frame's    */
/* absolute arena base and relative established height explicitly.           */

/*md
# Per-frame memory

The volatile byte array each call frame addresses from zero (YP §9.4.1):
flat, byte-addressed, zero-initialized, and lazily expanding in 32-byte
words. `MLOAD`/`MSTORE`/`MSTORE8`/`MSIZE` and the `*COPY` opcodes read and
write it; expansion past the high-water mark is charged as
memory-expansion gas.

Frames form a stack in Sail: a sub-call begins at the parent base plus its
established height and starts with height zero. The child base and saved parent
height recover the parent base on return, leaving dead child bytes outside the
parent's established range. A later expansion zeroes that gap before it becomes
visible. The host therefore needs no frame stack and no transaction-clear
operation.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The byte at absolute arena position `off`. */
val mem_read_byte = impure { c: "mem_read_byte" } : memory_base -> byte
/*! Writes one byte at absolute arena position `off` (`MSTORE8`). */
val mem_write_byte = impure { c: "mem_write_byte" } : (memory_base, byte) -> unit

/* range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY): endpoints
 * already live in host-interface regions, so copies never cross the
 * interface. */
/*! Materializes an EVM-memory extent whose protocol gas has already been
    charged. The host zeroes `[base + established, base + required)` and
    returns the resulting absolute source slice. Failure to realize this
    established precondition is a host implementation failure, not an EVM
    out-of-gas result. */
val mem_expand = impure { c: "mem_expand" } : forall 'required, 0 <= 'required & 'required <= memory_region_bound. (
        memory_base,
        memory_height,
        int('required),
    ) -> EvmMemorySliceLength('required)

/*! Copies an absolute byte range within the shared arena; overlapping
    ranges behave as one atomic move (`MCOPY` and the destination side of
    the `*COPY` opcodes). */
val mem_move = impure { c: "mem_move" } : (memory_base, memory_base, memory_length) -> unit

/* word access: MLOAD/MSTORE read/write the 32-byte big-endian range at off in
 * ONE crossing (YP 9.4.1) */
/*! Reads the 32-byte big-endian word at absolute `off` in one interface crossing
    (`MLOAD`). */
val mem_load_word = impure { c: "mem_load_word" } : memory_base -> word
/*! Writes the 32-byte big-endian word at absolute `off` in one interface
    crossing (`MSTORE`). */
val mem_store_word = impure { c: "mem_store_word" } : (memory_base, word) -> unit
