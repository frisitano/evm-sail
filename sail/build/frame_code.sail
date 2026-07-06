/* Build backend: C FFI bindings for the active frame code view
 * (ffi/code_db.c; the frame's code bytes and JUMPDEST bitmap live C-side,
 * only the length is mirrored into Sail). */

val frame_code_bind_stored_ffi = impure { _: "frame_code_bind_stored" } : hash -> bits(64)
function frame_code_bind_stored(h : hash) -> bits(64) = frame_code_bind_stored_ffi(h)

val frame_code_bind_memory_ffi = impure { _: "frame_code_bind_memory" } : (bits(64), bits(64)) -> bits(64)
function frame_code_bind_memory(off : bits(64), len : bits(64)) -> bits(64) = frame_code_bind_memory_ffi(off, len)

val frame_code_bind_tx_input_ffi = impure { _: "frame_code_bind_tx_input" } : unit -> bits(64)
function frame_code_bind_tx_input() -> bits(64) = frame_code_bind_tx_input_ffi()

val frame_code_clear_ffi = impure { _: "frame_code_clear" } : unit -> unit
function frame_code_clear() -> unit = frame_code_clear_ffi()

val frame_code_length_ffi = impure { _: "frame_code_length" } : unit -> bits(64)
function frame_code_length() -> bits(64) = frame_code_length_ffi()

val frame_code_byte_ffi = impure { _: "frame_code_byte" } : bits(64) -> bits(8)
function frame_code_byte(i : bits(64)) -> bits(8) = frame_code_byte_ffi(i)

val frame_jumpdest_valid_ffi = impure { _: "frame_jumpdest_valid" } : bits(64) -> bool
function frame_jumpdest_valid(i : bits(64)) -> bool = frame_jumpdest_valid_ffi(i)

val frame_push_immediate_word_ffi = impure { _: "frame_push_immediate_word" } : (bits(64), bits(64)) -> word
function frame_push_immediate_word(i : bits(64), n : bits(64)) -> word = frame_push_immediate_word_ffi(i, n)

val frame_code_copy_to_memory_ffi = impure { _: "frame_code_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit
function frame_code_copy_to_memory(dst : bits(64), off : bits(64), len : bits(64)) -> unit =
    frame_code_copy_to_memory_ffi(dst, off, len)
