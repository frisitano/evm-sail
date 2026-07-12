/* ======================================================================== */
/* RETURNDATA                                                               */
/*                                                                          */
/* The per-frame return buffer (EIP-211, mu_o): the bytes the most recent   */
/* child frame returned or a precompile produced, readable by               */
/* RETURNDATASIZE / RETURNDATACOPY and cleared per sub-call. A child's      */
/* output is CAPTURED from its memory as pending, then either adopted into  */
/* the parent's current buffer (normal return / REVERT payload) or          */
/* discarded. The bytes live in C (ffi/returndata.c); proof targets see the */
/* vals as bodyless axioms over an abstract per-frame byte-list model.      */
/* ======================================================================== */

val returndata_reset_all = impure { c: "returndata_reset_all" } : unit -> unit
val returndata_clear_current = impure { c: "returndata_clear_current" } : unit -> unit
val returndata_discard_pending = impure { c: "returndata_discard_pending" } : unit -> unit
val returndata_capture_memory = impure { c: "returndata_capture_memory" } : (bits(64), bits(64)) -> unit
val returndata_adopt_pending = impure { c: "returndata_adopt_pending" } : unit -> unit
val returndata_current_len = impure { c: "returndata_current_len" } : unit -> bits(64)
val returndata_pending_len = impure { c: "returndata_pending_len" } : unit -> bits(64)
val returndata_pending_byte_at = impure { c: "returndata_pending_byte" } : bits(64) -> bits(8)
val returndata_copy_current_to_memory = impure { c: "returndata_copy_current_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val returndata_copy_current_prefix_to_memory = impure { c: "returndata_copy_current_prefix_to_memory" } : (
        bits(64),
        bits(64),
    ) -> unit

function returndata_size() -> int = unsigned(returndata_current_len())
function returndata_capture(off : int, len : int) -> unit =
    returndata_capture_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_copy_to_mem(dst : int, off : int, len : int) -> unit =
    returndata_copy_current_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, off, 0), get_slice_int(64, len, 0))
function returndata_out_region(dst : int, want : int) -> unit =
    returndata_copy_current_prefix_to_memory(get_slice_int(64, dst, 0), get_slice_int(64, want, 0))

function returndata_pending_size() -> int = unsigned(returndata_pending_len())
function returndata_pending_byte(i : int) -> byte =
    returndata_pending_byte_at(get_slice_int(64, i, 0))

/* Immutable only until the pending buffer is adopted or discarded. Code
 * indexing and code-db insertion consume this view synchronously. */
function returndata_pending_slice() -> ByteSlice =
    byte_slice(ReturndataSource, 0, returndata_pending_size())

/* Materialize the pending child output (the init-code return value a CREATE
 * deploys as the new account's code) as a Sail list. */
function returndata_pending_bytes() -> list(byte) = {
    let n = returndata_pending_size();
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        out = returndata_pending_byte(n - 1 - i) :: out
    };
    out
}
