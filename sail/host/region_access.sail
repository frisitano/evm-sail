/*md
# Typed region access

Every host-backed byte span carries only `{off, len}`. Its nominal Sail type
selects the host region and therefore the C operation; no source tag or
runtime resolver crosses the model boundary.

!!! note "Non-normative"
    These declarations are internal executable-specification contracts.
*/

/*! The complete private input supplied to the stateless validator. The host
    acquires its pointer once and all derived spans reuse offsets into it. */
val stateless_input = impure { c: "stateless_input" } : unit -> StatelessInputSlice

/*! Reads the byte at an in-bounds slice-relative offset of the stateless
    input. */
val stateless_input_byte_at = impure { c: "stateless_input_byte_at" } : (StatelessInputSlice, stateless_input_length) -> byte
/*! Reads the byte at an in-bounds slice-relative offset of an EVM memory
    span. */
val memory_slice_byte_at = impure { c: "memory_slice_byte_at" } : (EvmMemorySlice, memory_length) -> byte
/*! Reads the byte at an in-bounds slice-relative offset of a code-region
    span. */
val code_region_byte_at = impure { c: "code_region_byte_at" } : (CodeRegionSlice, code_length) -> byte
/*! Reads the byte at an in-bounds slice-relative offset of a scratch-arena
    span. */
val scratch_slice_byte_at = impure { c: "scratch_slice_byte_at" } : (ScratchSlice, scratch_length) -> byte
/*! Reads the byte at an in-bounds slice-relative offset of a retained-log-data
    span. */
val log_data_slice_byte_at = impure { c: "log_data_slice_byte_at" } : (LogDataSlice, log_data_length) -> byte
/*! Reads the byte at an in-bounds slice-relative offset of an output-buffer
    span. */
val output_slice_byte_at = impure { c: "output_slice_byte_at" } : (OutputSlice, output_length) -> byte

/*! Counts the nonzero bytes of a stateless-input span. */
val stateless_input_count_nonzero = impure { c: "stateless_input_count_nonzero" } : StatelessInputSlice -> stateless_input_length

/*! Reports whether every one of `count` fields of `width` bytes, laid out
    `stride` bytes apart from `start`, is all zero within a stateless-input
    span; a field reaching past the slice end fails the check. */
val stateless_input_strided_zero = impure { c: "stateless_input_strided_zero" } : (
        StatelessInputSlice,
        stateless_input_length,
        stateless_input_length,
        stateless_input_length,
        stateless_input_length,
    ) -> bool
/*! Reports whether every one of `count` fields of `width` bytes, laid out
    `stride` bytes apart from `start`, is all zero within an EVM memory span;
    a field reaching past the slice end fails the check. */
val memory_slice_strided_zero = impure { c: "memory_slice_strided_zero" } : (
        EvmMemorySlice,
        memory_length,
        memory_length,
        memory_length,
        memory_length,
    ) -> bool

/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    the stateless input; bytes past the slice end read as zero. */
val stateless_input_load_word = impure { c: "stateless_input_load_word" } : (
        StatelessInputSlice,
        stateless_input_length,
    ) -> word
/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    an EVM memory span; bytes past the slice end read as zero. */
val memory_slice_load_word = impure { c: "memory_slice_load_word" } : (EvmMemorySlice, memory_length) -> word
/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    a code-region span; bytes past the slice end read as zero. */
val code_region_load_word = impure { c: "code_region_load_word" } : (CodeRegionSlice, code_length) -> word
/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    a scratch-arena span; bytes past the slice end read as zero. */
val scratch_slice_load_word = impure { c: "scratch_slice_load_word" } : (ScratchSlice, scratch_length) -> word
/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    a retained-log-data span; bytes past the slice end read as zero. */
val log_data_slice_load_word = impure { c: "log_data_slice_load_word" } : (LogDataSlice, log_data_length) -> word
/*! Loads the 32-byte big-endian word at an in-bounds slice-relative offset of
    an output-buffer span; bytes past the slice end read as zero. */
val output_slice_load_word = impure { c: "output_slice_load_word" } : (OutputSlice, output_length) -> word

/*! Loads the `n`-byte big-endian field at an in-bounds slice-relative offset
    of the stateless input into the low-order bytes of a word; bytes past the
    slice end read as zero. */
val stateless_input_load_n_word = impure { c: "stateless_input_load_n_word" } : (
        StatelessInputSlice,
        stateless_input_length,
        word_byte_count,
    ) -> word
/*! Loads the `n`-byte big-endian field at an in-bounds slice-relative offset
    of a code-region span into the low-order bytes of a word; bytes past the
    slice end read as zero. */
val code_region_load_n_word = impure { c: "code_region_load_n_word" } : (CodeRegionSlice, code_length, word_byte_count) -> word
/*! Loads the `n`-byte big-endian field at an in-bounds slice-relative offset
    of a scratch-arena span into the low-order bytes of a word; bytes past the
    slice end read as zero. */
val scratch_slice_load_n_word = impure { c: "scratch_slice_load_n_word" } : (
        ScratchSlice,
        scratch_length,
        word_byte_count,
    ) -> word

/*! Copies `len` bytes from a slice-relative offset of the stateless input
    into EVM memory at `dst`, zero-filling bytes past the slice end. */
val stateless_input_copy_to_memory = impure { c: "stateless_input_copy_to_memory" } : (
        StatelessInputSlice,
        memory_base,
        stateless_input_length,
        memory_length,
    ) -> unit
/*! Copies `len` bytes from a slice-relative offset of an EVM memory span
    into EVM memory at `dst`, zero-filling bytes past the slice end. */
val memory_slice_copy_to_memory = impure { c: "memory_slice_copy_to_memory" } : (
        EvmMemorySlice,
        memory_base,
        memory_length,
        memory_length,
    ) -> unit
/*! Copies `len` bytes from a slice-relative offset of a code-region span
    into EVM memory at `dst`, zero-filling bytes past the slice end. */
val code_region_copy_to_memory = impure { c: "code_region_copy_to_memory" } : (
        CodeRegionSlice,
        memory_base,
        code_length,
        memory_length,
    ) -> unit
/*! Copies `len` bytes from a slice-relative offset of an output-buffer span
    into EVM memory at `dst`, zero-filling bytes past the slice end. */
val output_slice_copy_to_memory = impure { c: "output_slice_copy_to_memory" } : (
        OutputSlice,
        memory_base,
        output_length,
        memory_length,
    ) -> unit

/*! Equality operations are declared only for provenance pairs used by the
    protocol rather than through a universal byte-source relation. */
val scratch_input_slices_equal = impure { c: "scratch_input_slices_equal" } : (ScratchSlice, StatelessInputSlice) -> bool
/*! Reports byte-for-byte equality of a retained-log-data span and a
    stateless-input span; spans of unequal length are unequal. */
val log_input_slices_equal = impure { c: "log_input_slices_equal" } : (LogDataSlice, StatelessInputSlice) -> bool
/*! Reports byte-for-byte equality of a stateless-input span and a code-region
    span; spans of unequal length are unequal. */
val input_code_slices_equal = impure { c: "input_code_slices_equal" } : (StatelessInputSlice, CodeRegionSlice) -> bool

overload region_slices_equal = {
    scratch_input_slices_equal,
    log_input_slices_equal,
    input_code_slices_equal
}

/*! Reads the byte at `off` of a stateless-input span, or `0x00` at or past
    the slice end. */
val stateless_input_slice_byte : forall 'off, 0 <= 'off & 'off <= stateless_input_region_bound. (
        StatelessInputSlice,
        int('off),
    ) -> byte
function stateless_input_slice_byte(s, off) =
    if off < s.len then {
        stateless_input_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of an EVM memory span, or `0x00` at or past the
    slice end. */
val memory_slice_byte : forall 'off, 0 <= 'off & 'off <= memory_region_bound. (EvmMemorySlice, int('off)) -> byte
function memory_slice_byte(s, off) =
    if off < s.len then {
        memory_slice_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of a code-region span, or `0x00` at or past the
    slice end. */
val code_slice_byte : forall 'off, 0 <= 'off & 'off <= code_region_bound. (CodeRegionSlice, int('off)) -> byte
function code_slice_byte(s, off) =
    if off < s.len then {
        code_region_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of a scratch-arena span, or `0x00` at or past the
    slice end. */
val scratch_byte : forall 'off, 0 <= 'off & 'off <= scratch_region_bound. (ScratchSlice, int('off)) -> byte
function scratch_byte(s, off) =
    if off < s.len then {
        scratch_slice_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of a retained-log-data span, or `0x00` at or past
    the slice end. */
val log_data_byte : forall 'off, 0 <= 'off & 'off <= log_data_region_bound. (LogDataSlice, int('off)) -> byte
function log_data_byte(s, off) =
    if off < s.len then {
        log_data_slice_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of an output-buffer span, or `0x00` at or past
    the slice end. */
val output_byte : forall 'off, 0 <= 'off & 'off <= output_region_bound. (OutputSlice, int('off)) -> byte
function output_byte(s, off) =
    if off < s.len then {
        output_slice_byte_at(s, off)
    } else {
        0x00
    }

/*! Reads the byte at `off` of calldata from either provenance, or `0x00` at
    or past the slice end. */
val calldata_slice_byte : forall 'off, 0 <= 'off & 'off <= default_host_region_bound. (CalldataSlice, int('off)) -> byte
function calldata_slice_byte(s, off) =
    match s {
        InputCalldata(bytes) => stateless_input_slice_byte(bytes, off),
        MemoryCalldata(bytes) => memory_slice_byte(bytes, off),
    }

overload slice_byte = {
    stateless_input_slice_byte,
    code_slice_byte,
    scratch_byte,
    log_data_byte,
    output_byte,
    calldata_slice_byte
}

function slice_count_nonzero(s : StatelessInputSlice) -> stateless_input_length =
    stateless_input_count_nonzero(s)

val stateless_input_slice_strided_zero : forall 'start 'stride 'width 'count,
                                             source_valid_length('start)
                                           & source_valid_length('stride)
                                           & source_valid_length('width)
                                           & source_valid_length('count). (
        StatelessInputSlice,
        int('start),
        int('stride),
        int('width),
        int('count),
    ) -> bool
function stateless_input_slice_strided_zero(s, start, stride, width, count) =
    stateless_input_strided_zero(s, start, stride, width, count)

val memory_slice_strided_zero_value : forall 'start 'stride 'width 'count,
                                          source_valid_length('start)
                                        & source_valid_length('stride)
                                        & source_valid_length('width)
                                        & source_valid_length('count). (
        EvmMemorySlice,
        int('start),
        int('stride),
        int('width),
        int('count),
    ) -> bool
function memory_slice_strided_zero_value(s, start, stride, width, count) =
    memory_slice_strided_zero(s, start, stride, width, count)

/*! Reports whether every one of `count` fields of `width` bytes, laid out
    `stride` bytes apart from `start`, is all zero within calldata from either
    provenance. */
val slice_strided_zero : forall 'start 'stride 'width 'count,
                             source_valid_length('start)
                           & source_valid_length('stride)
                           & source_valid_length('width)
                           & source_valid_length('count). (
        CalldataSlice,
        int('start),
        int('stride),
        int('width),
        int('count),
    ) -> bool
function slice_strided_zero(s, start, stride, width, count) =
    match s {
        InputCalldata(bytes) => stateless_input_slice_strided_zero(bytes, start, stride, width, count),
        MemoryCalldata(bytes) => memory_slice_strided_zero_value(bytes, start, stride, width, count),
    }

/*! Loads the 32-byte big-endian word at `off` of a stateless-input span, or
    the zero word at or past the slice end. */
val stateless_input_slice_load : forall 'off, source_valid_length('off). (StatelessInputSlice, int('off)) -> word
function stateless_input_slice_load(s, off) =
    if off < s.len then {
        stateless_input_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of an EVM memory span, or the
    zero word at or past the slice end. */
val memory_slice_load : forall 'off, source_valid_length('off). (EvmMemorySlice, int('off)) -> word
function memory_slice_load(s, off) =
    if off < s.len then {
        memory_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of a code-region span, or the
    zero word at or past the slice end. */
val code_slice_load : forall 'off, source_valid_length('off). (CodeRegionSlice, int('off)) -> word
function code_slice_load(s, off) =
    if off < s.len then {
        code_region_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of a scratch-arena span, or
    the zero word at or past the slice end. */
val scratch_slice_load : forall 'off, source_valid_length('off). (ScratchSlice, int('off)) -> word
function scratch_slice_load(s, off) =
    if off < s.len then {
        scratch_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of a retained-log-data span,
    or the zero word at or past the slice end. */
val log_data_slice_load : forall 'off, source_valid_length('off). (LogDataSlice, int('off)) -> word
function log_data_slice_load(s, off) =
    if off < s.len then {
        log_data_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of an output-buffer span, or
    the zero word at or past the slice end. */
val output_slice_load : forall 'off, source_valid_length('off). (OutputSlice, int('off)) -> word
function output_slice_load(s, off) =
    if off < s.len then {
        output_slice_load_word(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the 32-byte big-endian word at `off` of calldata from either
    provenance, or the zero word at or past the slice end. */
val calldata_slice_load : forall 'off, source_valid_length('off). (CalldataSlice, int('off)) -> word
function calldata_slice_load(s, off) =
    match s {
        InputCalldata(bytes) => stateless_input_slice_load(bytes, off),
        MemoryCalldata(bytes) => memory_slice_load(bytes, off),
    }

overload slice_load = {
    stateless_input_slice_load,
    memory_slice_load,
    code_slice_load,
    scratch_slice_load,
    log_data_slice_load,
    output_slice_load,
    calldata_slice_load
}

/*! Loads the word at a 256-bit offset of a stateless-input span; offsets at
    or past the slice end yield the zero word. */
function stateless_input_slice_load_word_offset(s : StatelessInputSlice, off : word) -> word =
    if off < s.len then {
        slice_load(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the word at a 256-bit offset of an EVM memory span; offsets at or
    past the slice end yield the zero word. */
function memory_slice_load_word_offset(s : EvmMemorySlice, off : word) -> word =
    if off < s.len then {
        slice_load(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the word at a 256-bit offset of a code-region span; offsets at or
    past the slice end yield the zero word. */
function code_slice_load_word_offset(s : CodeRegionSlice, off : word) -> word =
    if off < s.len then {
        slice_load(s, off)
    } else {
        ZERO_WORD
    }

/*! Loads the word at a 256-bit offset of calldata from either provenance;
    offsets at or past the slice end yield the zero word. */
function calldata_slice_load_word_offset(s : CalldataSlice, off : word) -> word =
    match s {
        InputCalldata(bytes) => stateless_input_slice_load_word_offset(bytes, off),
        MemoryCalldata(bytes) => memory_slice_load_word_offset(bytes, off),
    }

overload slice_load_word_offset = {
    code_slice_load_word_offset,
    calldata_slice_load_word_offset
}

/*! Loads the `n`-byte big-endian field at `off` of a stateless-input span
    into the low-order bytes of a word, or the zero word at or past the slice
    end. */
val stateless_input_slice_load_n : forall 'off 'n,
                                     0 <= 'off & 'off <= stateless_input_region_bound & 0 <= 'n & 'n <= 32. (
        StatelessInputSlice,
        int('off),
        int('n),
    ) -> word
function stateless_input_slice_load_n(s, off, n) =
    if off < s.len then {
        stateless_input_load_n_word(s, off, n)
    } else {
        ZERO_WORD
    }

/*! Loads the `n`-byte big-endian field at `off` of a code-region span into
    the low-order bytes of a word, or the zero word at or past the slice
    end. */
val code_slice_load_n : forall 'off 'n, 0 <= 'off & 'off <= code_region_bound & 0 <= 'n & 'n <= 32. (
        CodeRegionSlice,
        int('off),
        int('n),
    ) -> word
function code_slice_load_n(s, off, n) =
    if off < s.len then {
        code_region_load_n_word(s, off, n)
    } else {
        ZERO_WORD
    }

/*! Loads the `n`-byte big-endian field at `off` of a scratch-arena span into
    the low-order bytes of a word, or the zero word at or past the slice
    end. */
val scratch_slice_load_n : forall 'off 'n, 0 <= 'off & 'off <= scratch_region_bound & 0 <= 'n & 'n <= 32. (
        ScratchSlice,
        int('off),
        int('n),
    ) -> word
function scratch_slice_load_n(s, off, n) =
    if off < s.len then {
        scratch_slice_load_n_word(s, off, n)
    } else {
        ZERO_WORD
    }

overload slice_load_n = {
    stateless_input_slice_load_n,
    code_slice_load_n,
    scratch_slice_load_n
}

/*! Copies `len` bytes from `off` of a stateless-input span into EVM memory
    at `dst`, zero-filling bytes past the slice end. */
val stateless_input_slice_copy : forall 'dst 'off 'len,
                                      0
                                   <= 'dst
                                   &  'dst
                                   <= memory_region_bound
                                   &  0
                                   <= 'off
                                   &  'off
                                   <= stateless_input_region_bound
                                   &  0
                                   <= 'len
                                   &  'len
                                   <= memory_region_bound. (StatelessInputSlice, int('dst), int('off), int('len)) -> unit
function stateless_input_slice_copy(s, dst, off, len) =
    stateless_input_copy_to_memory(s, dst, off, len)

/*! Copies `len` bytes from `off` of an EVM memory span into EVM memory at
    `dst`, zero-filling bytes past the slice end. */
val memory_slice_copy : forall 'dst 'off 'len,
                             0
                          <= 'dst
                          &  'dst
                          <= memory_region_bound
                          &  0
                          <= 'off
                          &  'off
                          <= memory_region_bound
                          &  0
                          <= 'len
                          &  'len
                          <= memory_region_bound. (EvmMemorySlice, int('dst), int('off), int('len)) -> unit
function memory_slice_copy(s, dst, off, len) =
    memory_slice_copy_to_memory(s, dst, off, len)

/*! Copies `len` bytes from `off` of a code-region span into EVM memory at
    `dst`, zero-filling bytes past the slice end. */
val code_slice_copy : forall 'dst 'off 'len,
                           0
                        <= 'dst
                        &  'dst
                        <= memory_region_bound
                        &  0
                        <= 'off
                        &  'off
                        <= code_region_bound
                        &  0
                        <= 'len
                        &  'len
                        <= memory_region_bound. (CodeRegionSlice, int('dst), int('off), int('len)) -> unit
function code_slice_copy(s, dst, off, len) =
    code_region_copy_to_memory(s, dst, off, len)

/*! Copies `len` bytes from `off` of an output-buffer span into EVM memory at
    `dst`, zero-filling bytes past the slice end. */
val output_slice_copy : forall 'dst 'off 'len,
                             0
                          <= 'dst
                          &  'dst
                          <= memory_region_bound
                          &  0
                          <= 'off
                          &  'off
                          <= output_region_bound
                          &  0
                          <= 'len
                          &  'len
                          <= memory_region_bound. (OutputSlice, int('dst), int('off), int('len)) -> unit
function output_slice_copy(s, dst, off, len) =
    output_slice_copy_to_memory(s, dst, off, len)

overload slice_copy = {
    stateless_input_slice_copy,
    memory_slice_copy,
    code_slice_copy,
    output_slice_copy
}

/*! Copies `len` bytes at a 256-bit offset of a stateless-input span into EVM
    memory at `dst`; an offset at or past the slice end zero-fills the
    destination. */
function stateless_input_slice_copy_word_offset(
    s : StatelessInputSlice,
    dst : memory_base,
    off : word,
    len : memory_length,
) -> (
    unit
) =
    if off < s.len then {
        slice_copy(s, dst, off, len)
    } else {
        stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, 0, len)
    }

/*! Copies `len` bytes at a 256-bit offset of an EVM memory span into EVM
    memory at `dst`; an offset at or past the slice end zero-fills the
    destination. */
function memory_slice_copy_word_offset(s : EvmMemorySlice, dst : memory_base, off : word, len : memory_length) -> unit =
    if off < s.len then {
        slice_copy(s, dst, off, len)
    } else {
        stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, 0, len)
    }

/*! Copies `len` bytes at a 256-bit offset of a code-region span into EVM
    memory at `dst`; an offset at or past the slice end zero-fills the
    destination. */
function code_slice_copy_word_offset(s : CodeRegionSlice, dst : memory_base, off : word, len : memory_length) -> unit =
    if off < s.len then {
        slice_copy(s, dst, off, len)
    } else {
        stateless_input_copy_to_memory(EMPTY_STATELESS_INPUT_SLICE, dst, 0, len)
    }

/*! Copies `len` bytes at a 256-bit offset of calldata from either provenance
    into EVM memory at `dst`; an offset at or past the slice end zero-fills
    the destination. */
function calldata_slice_copy_word_offset(s : CalldataSlice, dst : memory_base, off : word, len : memory_length) -> unit =
    match s {
        InputCalldata(bytes) => stateless_input_slice_copy_word_offset(bytes, dst, off, len),
        MemoryCalldata(bytes) => memory_slice_copy_word_offset(bytes, dst, off, len),
    }

overload slice_copy_word_offset = {
    code_slice_copy_word_offset,
    calldata_slice_copy_word_offset
}
