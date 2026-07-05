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

val code_db_frame_set_stored_code_ffi = impure { _: "code_db_frame_set_stored_code" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_frame_set_stored_code(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_frame_set_stored_code_ffi(
    x0,
    x1,
    x2,
    x3,
)

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

val code_db_read_delegation_ffi = impure { _: "code_db_read_delegation" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(
        168,
    )
function code_db_read_delegation(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(168) = code_db_read_delegation_ffi(
    x0,
    x1,
    x2,
    x3,
)

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

val code_db_stream_code_begin_ffi = impure { _: "code_db_stream_code_begin" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(
        64,
    )
function code_db_stream_code_begin(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_stream_code_begin_ffi(
    x0,
    x1,
    x2,
    x3,
)

val code_db_stored_code_length_ffi = impure { _: "code_db_stored_code_length" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
function code_db_stored_code_length(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bits(64) = code_db_stored_code_length_ffi(
    x0,
    x1,
    x2,
    x3,
)

val code_db_copy_stored_code_to_memory_ffi = impure { _: "code_db_copy_stored_code_to_memory" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
function code_db_copy_stored_code_to_memory(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
) -> (
    unit
) =
    code_db_copy_stored_code_to_memory_ffi(x0, x1, x2, x3, x4, x5, x6)

val code_db_stream_code_byte_ffi = impure { _: "code_db_stream_code_byte" } : bits(8) -> unit
function code_db_stream_code_byte(x0 : bits(8)) -> unit = code_db_stream_code_byte_ffi(x0)
