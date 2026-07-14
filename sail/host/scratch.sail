/*md
# Scratch arena

Host storage for the executor's contiguous scratch byte arena. Sail owns
the allocation cursor; the host only stores bytes at that explicit offset.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

val host_scratch_store_bytes = impure { c: "scratch_store_bytes" } : (bits(64), list(byte), bits(64)) -> bool
val host_scratch_store_slice = impure { c: "scratch_store_slice" } : (bits(64), ByteSlice) -> bool
val host_scratch_truncate = impure { c: "scratch_truncate" } : bits(64) -> unit
