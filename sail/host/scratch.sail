
/*md
# Scratch arena

Host storage for the executor's contiguous scratch byte arena. Sail owns
the allocation cursor; the host only stores bytes at that explicit offset.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Stores a materialized byte list at an explicit arena offset. Success
    carries the resulting arena high-water mark. */
val host_scratch_store_bytes = impure { c: "scratch_store_bytes" } : forall 'off 'len,
                                                                       host_valid_access('off) & host_valid_access('len). (
        int('off),
        list(byte),
        int('len),
    ) -> ByteRegionResult('off + 'len)

/*! Stores a source-backed slice at an explicit arena offset. Success carries
    the resulting arena high-water mark. */
val host_scratch_store_slice = impure { c: "scratch_store_slice" } : forall 'dst 'off 'len,
                                                                       host_valid_access('dst) &
                                                                           host_valid_range('off, 'len). (
        int('dst),
        ByteSliceFields('off, 'len),
    ) -> ByteRegionResult('dst + 'len)
/*! Stores the first `len` wire-order bytes of a fixed 32-byte value. */
val host_scratch_store_b256 = impure { c: "scratch_store_b256" } : forall 'off 'len,
                                                                     host_valid_access('off) & 0 <= 'len & 'len <= 32. (
        int('off),
        b256,
        int('len),
    ) -> ByteRegionResult('off + 'len)
/*! Discards arena contents from the given offset on; the axiom behind
    [scratch_rewind][] and [scratch_reset][]. */
val host_scratch_truncate = impure { c: "scratch_truncate" } : source_pointer -> unit
