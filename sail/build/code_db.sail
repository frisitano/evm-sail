/* Build backend: C FFI bindings for the content-addressed code store and the
 * active frame code view (ffi/code_db.c). */

/* Concrete C-backed host mechanisms moved out of common spec files. */
val code_db_frame_jumpdest_valid_ffi = impure { _: "code_db_frame_jumpdest_valid" } : bits(64) -> bool
function code_db_frame_jumpdest_valid(x0 : bits(64)) -> bool = code_db_frame_jumpdest_valid_ffi(x0)

val code_db_frame_code_byte_ffi = impure { _: "code_db_frame_code_byte" } : bits(64) -> bits(8)
function code_db_frame_code_byte(x0 : bits(64)) -> bits(8) = code_db_frame_code_byte_ffi(x0)

val code_db_frame_push_immediate_word_ffi = impure { _: "code_db_frame_push_immediate_word" } : (bits(64), bits(64)) -> word
function code_db_frame_push_immediate_word(x0 : bits(64), x1 : bits(64)) -> word = code_db_frame_push_immediate_word_ffi(
    x0,
    x1,
)

val code_db_frame_set_stored_code_ffi = impure { _: "code_db_frame_set_stored_code" } : hash -> bits(64)
function code_db_frame_set_stored_code(h : hash) -> bits(64) = code_db_frame_set_stored_code_ffi(h)

val code_db_prepare_child_frame_memory_code_ffi = impure { _: "code_db_prepare_child_frame_memory_code" } : (
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_prepare_child_frame_memory_code(x0 : bits(64), x1 : bits(64)) -> bits(64) = code_db_prepare_child_frame_memory_code_ffi(
    x0,
    x1,
)

val code_db_frame_set_tx_input_code_ffi = impure { _: "code_db_frame_set_tx_input_code" } : unit -> bits(64)
function code_db_frame_set_tx_input_code() -> bits(64) = code_db_frame_set_tx_input_code_ffi()

val code_db_frame_clear_code_ffi = impure { _: "code_db_frame_clear_code" } : unit -> unit
function code_db_frame_clear_code() -> unit = code_db_frame_clear_code_ffi()

val code_db_frame_code_length_ffi = impure { _: "code_db_frame_code_length" } : unit -> bits(64)
function code_db_frame_code_length() -> bits(64) = code_db_frame_code_length_ffi()

val code_db_read_delegation_ffi = impure { _: "code_db_read_delegation" } : hash -> bits(168)
function code_db_read_delegation(h : hash) -> bits(168) = code_db_read_delegation_ffi(h)

val code_db_copy_frame_code_to_memory_ffi = impure { _: "code_db_copy_frame_code_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_frame_code_to_memory(x0 : bits(64), x1 : bits(64), x2 : bits(64)) -> unit = code_db_copy_frame_code_to_memory_ffi(
    x0,
    x1,
    x2,
)

val code_db_calldata_load_word_ffi = impure { _: "code_db_calldata_load_word" } : bits(64) -> word
function code_db_calldata_load_word(x0 : bits(64)) -> word = code_db_calldata_load_word_ffi(x0)

val code_db_stream_code_begin_ffi = impure { _: "code_db_stream_code_begin" } : hash -> bits(64)
function code_db_stream_code_begin(h : hash) -> bits(64) = code_db_stream_code_begin_ffi(h)

val code_db_stored_code_length_ffi = impure { _: "code_db_stored_code_length" } : hash -> bits(64)
function code_db_stored_code_length(h : hash) -> bits(64) = code_db_stored_code_length_ffi(h)

val code_db_copy_stored_code_to_memory_ffi = impure { _: "code_db_copy_stored_code_to_memory" } : (
        hash,
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_stored_code_to_memory(h : hash, dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    code_db_copy_stored_code_to_memory_ffi(h, dst, off, len)

val code_db_stream_code_byte_ffi = impure { _: "code_db_stream_code_byte" } : bits(8) -> unit
function code_db_stream_code_byte(x0 : bits(8)) -> unit = code_db_stream_code_byte_ffi(x0)
