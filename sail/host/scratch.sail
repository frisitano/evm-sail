/*md
# Scratch arena

Host storage for the executor's contiguous scratch byte arena. Sail owns
the allocation cursor; the host only stores bytes at that explicit offset.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Stores a materialized byte list at an explicit arena offset; the
    flag reports acceptance. The axiom behind [scratch_push_bytes][]. */
val host_scratch_store_bytes = impure { c: "scratch_store_bytes" } : (bits(64), list(byte), bits(64)) -> bool
/*! Stores a source-backed slice at an explicit arena offset; the axiom
    behind [scratch_push_slice][]. */
val host_scratch_store_slice = impure { c: "scratch_store_slice" } : (bits(64), ByteSlice) -> bool
/*! Discards arena contents from the given offset on; the axiom behind
    [scratch_rewind][] and [scratch_reset][]. */
val host_scratch_truncate = impure { c: "scratch_truncate" } : bits(64) -> unit
