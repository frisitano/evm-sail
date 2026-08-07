
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
                                                                     0
                                                                  <= 'off
                                                                  &  'off
                                                                  <= scratch_region_bound
                                                                  &  0
                                                                  <= 'len
                                                                  &  'len
                                                                  <= scratch_region_bound. (int('off), int('len)) -> bool

/*! Stores one byte at an explicit arena offset. */
val host_scratch_store_byte = impure { c: "scratch_store_byte" } : forall 'off,
                                                                     0 <= 'off & 'off <= scratch_region_bound. (
        int('off),
        byte,
    ) -> ScratchSliceLength('off + 1)

/*! Stores a source-backed slice at an explicit arena offset. Success carries
    the resulting arena high-water mark. */
val host_scratch_store_stateless_input = impure { c: "scratch_store_stateless_input" } : forall 'dst 'off 'len,
                                                                                              0
                                                                                           <= 'dst
                                                                                           &  'dst
                                                                                           <= scratch_region_bound
                                                                                           &  stateless_input_valid_range(
                                                                                                  'off,
                                                                                                  'len,
                                                                                              ). (
        int('dst),
        StatelessInputSliceFields('off, 'len),
    ) -> ScratchSliceLength('dst + 'len)
/*! Stores an existing arena-backed slice at an explicit arena offset; the
    result carries the new arena high-water mark. */
val host_scratch_store_scratch = impure { c: "scratch_store_scratch" } : forall 'dst 'off 'len,
                                                                              0
                                                                           <= 'dst
                                                                           &  'dst
                                                                           <= scratch_region_bound
                                                                           &  scratch_valid_range('off, 'len). (
        int('dst),
        ScratchSliceFields('off, 'len),
    ) -> ScratchSliceLength('dst + 'len)
/*! Stores a retained-log-data slice at an explicit arena offset; the result
    carries the new arena high-water mark. */
val host_scratch_store_log_data = impure { c: "scratch_store_log_data" } : forall 'dst 'off 'len,
                                                                                0
                                                                             <= 'dst
                                                                             &  'dst
                                                                             <= scratch_region_bound
                                                                             &  log_data_valid_range('off, 'len). (
        int('dst),
        LogDataSliceFields('off, 'len),
    ) -> ScratchSliceLength('dst + 'len)
/*! Stores an output-buffer-backed slice at an explicit arena offset; the
    result carries the new arena high-water mark. */
val host_scratch_store_output = impure { c: "scratch_store_output" } : forall 'dst 'off 'len,
                                                                            0
                                                                         <= 'dst
                                                                         &  'dst
                                                                         <= scratch_region_bound
                                                                         &  output_region_valid_range('off, 'len). (
        int('dst),
        OutputSliceFields('off, 'len),
    ) -> ScratchSliceLength('dst + 'len)
/*! Stores a fixed 20-byte address in wire order. */
val host_scratch_store_address = impure { c: "scratch_store_address" } : forall 'off,
                                                                           0 <= 'off & 'off <= scratch_region_bound. (
        int('off),
        address,
    ) -> ScratchSliceLength('off + 20)
/*! Stores the first `len` wire-order bytes of a fixed 32-byte value. */
val host_scratch_store_b256 = impure { c: "scratch_store_b256" } : forall 'off 'len,
                                                                        0
                                                                     <= 'off
                                                                     &  'off
                                                                     <= scratch_region_bound
                                                                     &  0
                                                                     <= 'len
                                                                     &  'len
                                                                     <= 32. (int('off), b256, int('len)) -> ScratchSliceLength(
        'off + 'len,
    )
/*! Stores a decreasing fixed 256-byte vector in canonical wire order. */
val host_scratch_store_fixed_bytes_256 = impure { c: "scratch_store_fixed_bytes_256" } : forall 'off,
                                                                                              0
                                                                                           <= 'off
                                                                                           &  'off
                                                                                           <= scratch_region_bound. (
        int('off),
        vector(256, dec, byte),
    ) -> ScratchSliceLength('off + 256)
/*! Stores the low `len` bytes of a word in canonical big-endian order. */
val host_scratch_store_word = impure { c: "scratch_store_word" } : forall 'off 'len,
                                                                        0
                                                                     <= 'off
                                                                     &  'off
                                                                     <= scratch_region_bound
                                                                     &  0
                                                                     <= 'len
                                                                     &  'len
                                                                     <= 32. (int('off), word, int('len)) -> ScratchSliceLength(
        'off + 'len,
    )
/*! Discards arena contents from the given offset on; the axiom behind
    [scratch_rewind][] and [scratch_reset][]. */
val host_scratch_truncate = impure { c: "scratch_truncate" } : scratch_pointer -> unit
