/* Host storage for the executor's contiguous scratch byte arena. Sail owns
 * the allocation cursor; the host only stores bytes at that explicit offset. */

val host_scratch_store_bytes = impure { c: "scratch_store_bytes" } : (byte_quantity, list(byte), byte_quantity) -> bool
val host_scratch_store_slice = impure { c: "scratch_store_slice" } : (byte_quantity, ByteSlice) -> bool
val host_scratch_truncate = impure { c: "scratch_truncate" } : byte_quantity -> unit
