
/*md
# The output buffer

Host-owned storage for a frame or precompile output that must outlive the
mutable EVM memory from which it was produced. EVM returndata state and
frame transitions remain entirely in Sail.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Copies a slice into the host output buffer; the flag reports
    acceptance. The axiom behind [freeze_output][]. */
val output_buffer_store = impure { c: "output_buffer_store" } : ByteSlice -> bool
/*! Stores one 32-byte word as the buffer contents; the axiom behind
    [output_buffer_word][]. */
val output_buffer_store_word = impure { c: "output_buffer_store_word" } : word -> bool
/*! Stores two consecutive words as the buffer contents; the axiom
    behind [output_buffer_words][]. */
val output_buffer_store_words = impure { c: "output_buffer_store_words" } : (word, word) -> bool
/*! Appends one byte to the guest's public output — the committed result
    stream the verifier sees. */
val public_output_write = impure { c: "public_output_write" } : ByteSlice -> bool

/*! A slice over the first `len` bytes of the output buffer. */
val output_buffer_slice : forall 'len, source_valid_length('len). int('len) -> ByteSlice
function output_buffer_slice(len) =
    if len == 0 then {
        EMPTY_SLICE
    } else {
        byte_slice(OutputSource, 0, len)
    }

/*! Copies frame output into the host buffer so it survives frame
    teardown; the canonical returndata source. */
function freeze_output(data : ByteSlice) -> ByteSlice = {
    let len = data.len;
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

/*! Stores one word as the output (32-byte precompile results). */
function output_buffer_word(value : word) -> ByteSlice = {
    if output_buffer_store_word(value) then {
        output_buffer_slice(WORD_BYTE_LENGTH)
    } else {
        EMPTY_SLICE
    }
}

/*! Stores two words as the output (64-byte precompile results, e.g.
    `ecrecover`-style pairs). */
function output_buffer_words(first : word, second : word) -> ByteSlice = {
    if output_buffer_store_words(first, second) then {
        output_buffer_slice(DOUBLE_WORD_BYTE_LENGTH)
    } else {
        EMPTY_SLICE
    }
}
