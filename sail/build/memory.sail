/* Build backend: C FFI bindings for per-frame EVM memory, returndata,
 * calldata, and the transaction-input view (ffi/memory.c,
 * ffi/returndata.c). */

val host_mem_read_ffi = impure { _: "host_mem_read" } : bits(64) -> bits(8)
function host_mem_read(x0 : bits(64)) -> bits(8) = host_mem_read_ffi(x0)

val host_mem_write_ffi = impure { _: "host_mem_write" } : (bits(64), bits(8)) -> unit
function host_mem_write(x0 : bits(64), x1 : bits(8)) -> unit = host_mem_write_ffi(x0, x1)

val host_mem_reset_ffi = impure { _: "host_mem_reset" } : unit -> unit
function host_mem_reset() -> unit = host_mem_reset_ffi()

val host_mem_push_ffi = impure { _: "host_mem_push" } : unit -> unit
function host_mem_push() -> unit = host_mem_push_ffi()

val host_mem_pop_ffi = impure { _: "host_mem_pop" } : unit -> unit
function host_mem_pop() -> unit = host_mem_pop_ffi()

val hm_move_ffi = impure { _: "hm_move" } : (bits(64), bits(64), bits(64)) -> unit
function hm_move(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = hm_move_ffi(x0, x1, x2)

val returndata_reset_all_ffi = impure { _: "returndata_reset_all" } : unit -> unit
function returndata_reset_all() -> unit = returndata_reset_all_ffi()

val returndata_clear_current_ffi = impure { _: "returndata_clear_current" } : unit -> unit
function returndata_clear_current() -> unit = returndata_clear_current_ffi()

val returndata_discard_pending_ffi = impure { _: "returndata_discard_pending" } : unit -> unit
function returndata_discard_pending() -> unit = returndata_discard_pending_ffi()

val returndata_capture_memory_ffi = impure { _: "returndata_capture_memory" } : (bits(64), bits(64)) -> unit
function returndata_capture_memory(x0 : bits(64), x1 : bits(64)) -> unit = returndata_capture_memory_ffi(x0, x1)

val returndata_adopt_pending_ffi = impure { _: "returndata_adopt_pending" } : unit -> unit
function returndata_adopt_pending() -> unit = returndata_adopt_pending_ffi()

val returndata_current_len_ffi = impure { _: "returndata_current_len" } : unit -> bits(64)
function returndata_current_len() -> bits(64) = returndata_current_len_ffi()

val returndata_pending_len_ffi = impure { _: "returndata_pending_len" } : unit -> bits(64)
function returndata_pending_len() -> bits(64) = returndata_pending_len_ffi()

val returndata_pending_byte_at_ffi = impure { _: "returndata_pending_byte" } : bits(64) -> bits(8)
function returndata_pending_byte_at(x0 : bits(64)) -> bits(8) = returndata_pending_byte_at_ffi(x0)

val returndata_copy_current_to_memory_ffi = impure { _: "returndata_copy_current_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function returndata_copy_current_to_memory(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = returndata_copy_current_to_memory_ffi(
    x0,
    x1,
    x2,
)

val returndata_copy_current_prefix_to_memory_ffi = impure { _: "returndata_copy_current_prefix_to_memory" } : (
        bits(64),
        bits(64),
    ) -> unit
function returndata_copy_current_prefix_to_memory(x0 : bits(64), x1 : bits(64)) -> unit = returndata_copy_current_prefix_to_memory_ffi(
    x0,
    x1,
)

val cd_set_ffi = impure { _: "cd_set" } : (bits(64), bits(64)) -> unit
function cd_set(x0 : bits(64), x1 : bits(64)) -> unit = cd_set_ffi(x0, x1)

val cd_set_empty_ffi = impure { _: "cd_set_empty" } : unit -> unit
function cd_set_empty() -> unit = cd_set_empty_ffi()

val cd_set_tx_ffi = impure { _: "cd_set_tx" } : unit -> unit
function cd_set_tx() -> unit = cd_set_tx_ffi()

val txin_begin_ffi = impure { _: "txin_begin" } : unit -> unit
function txin_begin() -> unit = txin_begin_ffi()

val txin_byte_ffi = impure { _: "txin_byte" } : bits(8) -> unit
function txin_byte(x0 : bits(8)) -> unit = txin_byte_ffi(x0)

val cd_to_mem_ffi = impure { _: "cd_to_mem" } : (bits(64), bits(64), bits(64)) -> unit
function cd_to_mem(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = cd_to_mem_ffi(x0, x1, x2)

val calldata_load_word_ffi = impure { _: "calldata_load_word" } : bits(64) -> word
function calldata_load_word(i : bits(64)) -> word = calldata_load_word_ffi(i)

val txd_at_ffi = impure { _: "txd_at" } : bits(64) -> bits(8)
function txd_at(x0 : bits(64)) -> bits(8) = txd_at_ffi(x0)

val txd_length_ffi = impure { _: "txd_length" } : unit -> bits(64)
function txd_length() -> bits(64) = txd_length_ffi()
