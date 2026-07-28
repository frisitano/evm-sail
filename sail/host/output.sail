
/*md
# The output buffer

Host-owned storage for a frame or precompile output that must outlive the
mutable EVM memory from which it was produced. EVM returndata state and
frame transitions remain entirely in Sail.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Copies frame memory into the host output buffer; the flag reports
    acceptance. */
val output_buffer_store_memory = impure { c: "output_buffer_store_memory" } : MemorySlice -> bool
/*! Copies top-level calldata into the host output buffer. This is used by the
    identity precompile without first materializing the input in EVM memory. */
val output_buffer_store_input = impure { c: "output_buffer_store_input" } : StatelessInputSlice -> bool
/*! Stores one 32-byte word as the buffer contents; the axiom behind
    [output_buffer_word][]. */
val output_buffer_store_word = impure { c: "output_buffer_store_word" } : word -> bool
/*! Stores two consecutive words as the buffer contents; the axiom
    behind [output_buffer_words][]. */
val output_buffer_store_words = impure { c: "output_buffer_store_words" } : (word, word) -> bool
/*! Appends one byte to the guest's public output — the committed result
    stream the verifier sees. */
val public_output_write = impure { c: "public_output_write" } : ScratchSlice -> bool

/*! A slice over the first `len` bytes of the output buffer. */
val output_buffer_slice : forall 'len, source_valid_length('len). int('len) -> OutputSlice
function output_buffer_slice(len) =
    if len == 0 then {
        EMPTY_OUTPUT_SLICE
    } else {
        output_slice(0, len)
    }

/*! Copies frame output into the host buffer so it survives frame
    teardown; the canonical returndata source. */
function freeze_memory_output(data : MemorySlice) -> OutputSlice = {
    let len = data.len;
    if len == 0 then {
        EMPTY_OUTPUT_SLICE
    } else {
        if output_buffer_store_memory(data) then {
            output_buffer_slice(len)
        } else {
            EMPTY_OUTPUT_SLICE
        }
    }
}

function freeze_input_output(data : StatelessInputSlice) -> OutputSlice = {
    let len = data.len;
    if len == 0 then {
        EMPTY_OUTPUT_SLICE
    } else {
        if output_buffer_store_input(data) then {
            output_buffer_slice(len)
        } else {
            EMPTY_OUTPUT_SLICE
        }
    }
}

function freeze_calldata_output(data : CalldataSlice) -> OutputSlice =
    match data {
        InputCalldata(bytes) => freeze_input_output(bytes),
        MemoryCalldata(bytes) => freeze_memory_output(bytes),
    }

overload freeze_output = {freeze_memory_output, freeze_calldata_output}

/*! Stores one word as the output (32-byte precompile results). */
function output_buffer_word(value : word) -> OutputSlice = {
    if output_buffer_store_word(value) then {
        output_buffer_slice(WORD_BYTE_LENGTH)
    } else {
        EMPTY_OUTPUT_SLICE
    }
}

/*! Stores two words as the output (64-byte precompile results, e.g.
    `ecrecover`-style pairs). */
function output_buffer_words(first : word, second : word) -> OutputSlice = {
    if output_buffer_store_words(first, second) then {
        output_buffer_slice(DOUBLE_WORD_BYTE_LENGTH)
    } else {
        EMPTY_OUTPUT_SLICE
    }
}
