/* Build backend: C FFI bindings for the private-input oracle and the guest
 * public-output trace. */

val ssz_src_len_ffi = impure { _: "ssz_src_len" } : unit -> bits(64)
val ssz_src_byte_ffi = impure { _: "ssz_src_byte" } : int -> bits(8)
val ssz_src_le_ffi = impure { _: "ssz_src_le" } : (int, int) -> bits(64)
val ssz_src_be_ffi = impure { _: "ssz_src_be" } : (int, int) -> bits(64)
val emit_out_ffi = impure { _: "el_emit_out" } : bits(8) -> unit

function ssz_src_len() -> bits(64) = ssz_src_len_ffi()
function ssz_src_byte(i : int) -> bits(8) = ssz_src_byte_ffi(i)
function ssz_src_le(b : int, n : int) -> bits(64) = ssz_src_le_ffi(b, n)
function ssz_src_be(b : int, n : int) -> bits(64) = ssz_src_be_ffi(b, n)
function emit_out(b : bits(8)) -> unit = emit_out_ffi(b)
