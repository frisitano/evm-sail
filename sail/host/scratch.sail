
/*md
# Scratch arena

Host storage for the executor's contiguous scratch byte arena. Sail owns
the allocation cursor; the host only stores bytes at that explicit offset.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Ensures that `len` bytes can be appended at the current explicit arena
    offset without changing the Sail-owned high-water mark. */
val host_scratch_reserve = impure { c: "scratch_reserve_at" } : forall 'off 'len,
                                                                  host_valid_access('off) & host_valid_access('len). (
        int('off),
        int('len),
    ) -> bool

/*! Stores one byte at an explicit arena offset. */
val host_scratch_store_byte = impure { c: "scratch_store_byte" } : forall 'off, host_valid_access('off). (
        int('off),
        byte,
    ) -> ScratchRegionResult('off + 1)

/*! Stores a source-backed slice at an explicit arena offset. Success carries
    the resulting arena high-water mark. */
val host_scratch_store_stateless_input = impure { c: "scratch_store_stateless_input" } : forall 'dst 'off 'len,
                                                                                           host_valid_access('dst) &
                                                                                               host_valid_range(
                                                                                                   'off,
                                                                                                   'len,
                                                                                               ). (
        int('dst),
        StatelessInputSliceFields('off, 'len),
    ) -> ScratchRegionResult('dst + 'len)
val host_scratch_store_scratch = impure { c: "scratch_store_scratch" } : forall 'dst 'off 'len,
                                                                           host_valid_access('dst) &
                                                                               host_valid_range('off, 'len). (
        int('dst),
        ScratchSliceFields('off, 'len),
    ) -> ScratchRegionResult('dst + 'len)
val host_scratch_store_log_data = impure { c: "scratch_store_log_data" } : forall 'dst 'off 'len,
                                                                             host_valid_access('dst) &
                                                                                 host_valid_range('off, 'len). (
        int('dst),
        LogDataSliceFields('off, 'len),
    ) -> ScratchRegionResult('dst + 'len)
val host_scratch_store_output = impure { c: "scratch_store_output" } : forall 'dst 'off 'len,
                                                                         host_valid_access('dst) &
                                                                             host_valid_range('off, 'len). (
        int('dst),
        OutputSliceFields('off, 'len),
    ) -> ScratchRegionResult('dst + 'len)
/*! Stores a fixed 20-byte address in wire order. */
val host_scratch_store_address = impure { c: "scratch_store_address" } : forall 'off, host_valid_access('off). (
        int('off),
        address,
    ) -> ScratchRegionResult('off + 20)
/*! Stores the first `len` wire-order bytes of a fixed 32-byte value. */
val host_scratch_store_b256 = impure { c: "scratch_store_b256" } : forall 'off 'len,
                                                                     host_valid_access('off) & 0 <= 'len & 'len <= 32. (
        int('off),
        b256,
        int('len),
    ) -> ScratchRegionResult('off + 'len)
/*! Stores a decreasing fixed 256-byte vector in canonical wire order. */
val host_scratch_store_fixed_bytes_256 = impure { c: "scratch_store_fixed_bytes_256" } : forall 'off,
                                                                                           host_valid_access('off). (
        int('off),
        vector(256, dec, byte),
    ) -> ScratchRegionResult('off + 256)
/*! Stores the low `len` bytes of a word in canonical big-endian order. */
val host_scratch_store_word = impure { c: "scratch_store_word" } : forall 'off 'len,
                                                                     host_valid_access('off) & 0 <= 'len & 'len <= 32. (
        int('off),
        word,
        int('len),
    ) -> ScratchRegionResult('off + 'len)
/*! Discards arena contents from the given offset on; the axiom behind
    [scratch_rewind][] and [scratch_reset][]. */
val host_scratch_truncate = impure { c: "scratch_truncate" } : source_pointer -> unit
