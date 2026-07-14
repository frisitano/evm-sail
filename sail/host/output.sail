/* Host-owned storage for a frame or precompile output that must outlive the
 * mutable EVM memory from which it was produced. EVM returndata state and
 * frame transitions remain entirely in Sail. */

val output_buffer_store = impure { c: "output_buffer_store" } : ByteSlice -> bool
val output_buffer_store_word = impure { c: "output_buffer_store_word" } : word -> bool
val output_buffer_store_words = impure { c: "output_buffer_store_words" } : (word, word) -> bool
val public_output_write = impure { c: "el_emit_out" } : byte -> unit

function output_buffer_slice(len : int) -> ByteSlice =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        byte_slice(OutputSource, 0, len)
    }

function freeze_output(data : ByteSlice) -> ByteSlice = {
    let len = unsigned(data.len);
    if len == 0 then {
        EMPTY_SLICE
    } else {
        if output_buffer_store(data) then {
            output_buffer_slice(len)
        } else {
            EMPTY_SLICE
        }
    }
}

function output_buffer_word(value : word) -> ByteSlice = {
    if output_buffer_store_word(value) then {
        output_buffer_slice(32)
    } else {
        EMPTY_SLICE
    }
}

function output_buffer_words(first : word, second : word) -> ByteSlice = {
    if output_buffer_store_words(first, second) then {
        output_buffer_slice(64)
    } else {
        EMPTY_SLICE
    }
}
