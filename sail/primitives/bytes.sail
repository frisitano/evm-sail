
/*md
# Typed byte regions

Unmaterialized byte sequences. The specification's bulk data lives in named
host regions. A slice's nominal type identifies its region; the semantic value
contains only a source coordinate and a length. There is deliberately no
generic slice and no runtime source discriminant.

The standard C ABI lowers the coordinate to an offset in the named region. The
fixed-capacity optimized C ABI lowers the same coordinate to a validated
absolute pointer, after allocating every mutable region before Sail execution.
This representation choice is not part of the protocol semantics: subslicing
is coordinate addition in both builds, and the nominal type retains provenance.

## Constants

The fixed lengths identify addresses, words, limbs, and double words.
*/
/*! A range in the immutable stateless-input envelope. Its source is carried
    by the nominal type rather than a runtime field. */
struct StatelessInputSliceFields('off : Int, 'len : Int), stateless_input_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type StatelessInputSlice = {
    'off 'len,
    stateless_input_valid_range('off, 'len).
    StatelessInputSliceFields('off, 'len)
}

type StatelessInputSliceLength('required : Int) = {
    'off 'len,
    stateless_input_valid_range('off, 'len) & 'len == 'required.
    StatelessInputSliceFields('off, 'len)
}

type StatelessInputSliceAtLeast('minimum : Int) = {
    'off 'len,
    stateless_input_valid_range('off, 'len) & 0 <= 'minimum & 'minimum <= 'len.
    StatelessInputSliceFields('off, 'len)
}

type StatelessInputSliceAtMost('maximum : Int) = {
    'off 'len,
    stateless_input_valid_range('off, 'len) & 0 <= 'maximum & 'len <= 'maximum.
    StatelessInputSliceFields('off, 'len)
}

/*! A range in the executor's reusable scratch arena. Its source is carried by
    the nominal type rather than a runtime field. */
struct ScratchSliceFields('off : Int, 'len : Int), scratch_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type ScratchSlice = {
    'off 'len,
    scratch_valid_range('off, 'len).
    ScratchSliceFields('off, 'len)
}

type ScratchSliceLength('required : Int) = {
    'off 'len,
    scratch_valid_range('off, 'len) & 'len == 'required.
    ScratchSliceFields('off, 'len)
}

type ScratchSliceAtLeast('minimum : Int) = {
    'off 'len,
    scratch_valid_range('off, 'len) & 0 <= 'minimum & 'minimum <= 'len.
    ScratchSliceFields('off, 'len)
}

/*! A range in the shared EVM-memory arena. Its coordinate is absolute within
    the arena (an offset in the standard ABI and a stable pointer in the
    optimized ABI), so a suspended parent's range remains valid while a child
    frame is active. */
struct EvmMemorySliceFields('off : Int, 'len : Int), memory_region_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type EvmMemorySlice = {
    'off 'len,
    memory_region_valid_range('off, 'len).
    EvmMemorySliceFields('off, 'len)
}

type EvmMemorySliceLength('required : Int) = {
    'off 'len,
    memory_region_valid_range('off, 'len) & 'len == 'required.
    EvmMemorySliceFields('off, 'len)
}

type EvmMemorySliceAtLeast('minimum : Int) = {
    'off 'len,
    memory_region_valid_range('off, 'len) & 0 <= 'minimum & 'minimum <= 'len.
    EvmMemorySliceFields('off, 'len)
}

/*! A range in the content-addressed executable-code arena. */
struct CodeRegionSliceFields('off : Int, 'len : Int), code_region_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type CodeRegionSlice = {
    'off 'len,
    code_region_valid_range('off, 'len).
    CodeRegionSliceFields('off, 'len)
}

/*! A range in the transaction-log data arena. */
struct LogDataSliceFields('off : Int, 'len : Int), log_data_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type LogDataSlice = {
    'off 'len,
    log_data_valid_range('off, 'len).
    LogDataSliceFields('off, 'len)
}

/*! A range in the frame-output buffer. */
struct OutputSliceFields('off : Int, 'len : Int), output_region_valid_range('off, 'len) = {
    bytes : int('off),
    len : int('len),
}

type OutputSlice = {
    'off 'len,
    output_region_valid_range('off, 'len).
    OutputSliceFields('off, 'len)
}

/*! Calldata is either the immutable top-level transaction input or a frozen
    range of the suspended caller's memory. The variants state the only two
    protocol-valid provenances instead of exposing the host's region enum. */
union CalldataSlice = {
    InputCalldata : StatelessInputSlice,
    MemoryCalldata : EvmMemorySlice,
}

function calldata_slice_length(s : CalldataSlice) -> source_length =
    match s {
        InputCalldata(bytes) => bytes.len,
        MemoryCalldata(bytes) => bytes.len,
    }

function stateless_input_slice_length(s : StatelessInputSlice) -> source_length =
    s.len

overload region_slice_length = {
    stateless_input_slice_length,
    calldata_slice_length
}

/*! The result of extending the executor scratch arena to the requested
    high-water mark. */
union ScratchRegionResult('required : Int), 0 <= 'required = {
    ScratchRegionReady : ScratchSliceLength('required),
    ScratchRegionFailed : unit,
}

let ADDRESS_BYTE_LENGTH : int(20) = 20
let WORD_BYTE_LENGTH : int(32) = 32
let EIGHT_BYTE_LENGTH : int(8) = 8
let DOUBLE_WORD_BYTE_LENGTH : int(64) = 64

val stateless_input_slice : forall 'off 'len, stateless_input_valid_range('off, 'len). (int('off), int('len)) -> StatelessInputSliceFields(
        'off,
        'len,
    )
function stateless_input_slice(off, len) =
    struct { bytes = off, len = len }

val scratch_slice : forall 'off 'len, scratch_valid_range('off, 'len). (int('off), int('len)) -> ScratchSliceFields(
        'off,
        'len,
    )
function scratch_slice(off, len) =
    struct { bytes = off, len = len }

val evm_memory_slice : forall 'off 'len, memory_region_valid_range('off, 'len). (int('off), int('len)) -> EvmMemorySliceFields(
        'off,
        'len,
    )
function evm_memory_slice(off, len) =
    struct { bytes = off, len = len }

val code_region_slice : forall 'off 'len, code_region_valid_range('off, 'len). (int('off), int('len)) -> CodeRegionSliceFields(
        'off,
        'len,
    )
function code_region_slice(off, len) =
    struct { bytes = off, len = len }

val log_data_slice : forall 'off 'len, log_data_valid_range('off, 'len). (int('off), int('len)) -> LogDataSliceFields(
        'off,
        'len,
    )
function log_data_slice(off, len) =
    struct { bytes = off, len = len }

val output_slice : forall 'off 'len, output_region_valid_range('off, 'len). (int('off), int('len)) -> OutputSliceFields(
        'off,
        'len,
    )
function output_slice(off, len) =
    struct { bytes = off, len = len }

let EMPTY_STATELESS_INPUT_SLICE : StatelessInputSliceFields(0, 0) = stateless_input_slice(0, 0)
let EMPTY_SCRATCH_SLICE : ScratchSliceFields(0, 0) = scratch_slice(0, 0)
let EMPTY_EVM_MEMORY_SLICE : EvmMemorySliceFields(0, 0) = evm_memory_slice(0, 0)
let EMPTY_CODE_REGION_SLICE : CodeRegionSliceFields(0, 0) = code_region_slice(0, 0)
let EMPTY_LOG_DATA_SLICE : LogDataSliceFields(0, 0) = log_data_slice(0, 0)
let EMPTY_OUTPUT_SLICE : OutputSliceFields(0, 0) = output_slice(0, 0)
let EMPTY_CALLDATA : CalldataSlice = InputCalldata(EMPTY_STATELESS_INPUT_SLICE)

/*! A relative sub-range of a slice, within the same source. */
val stateless_input_sub_slice : forall 'base 'source_len 'off 'len,
                                     stateless_input_valid_range('base, 'source_len)
                                  &  0
                                  <= 'off
                                  &  0
                                  <= 'len
                                  &  'off
                                  +  'len
                                  <= 'source_len. (StatelessInputSliceFields('base, 'source_len), int('off), int('len)) -> StatelessInputSliceFields(
        'base + 'off,
        'len,
    )
function stateless_input_sub_slice(s, off, len) =
    struct { bytes = s.bytes + off, len = len }

val scratch_sub_slice : forall 'base 'source_len 'off 'len,
                          scratch_valid_range('base, 'source_len) & 0 <= 'off & 0 <= 'len & 'off + 'len <= 'source_len. (
        ScratchSliceFields('base, 'source_len),
        int('off),
        int('len),
    ) -> ScratchSliceFields('base + 'off, 'len)
function scratch_sub_slice(s, off, len) =
    struct { bytes = s.bytes + off, len = len }

val memory_sub_slice : forall 'base 'source_len 'off 'len,
                         memory_region_valid_range('base, 'source_len) & 0 <= 'off & 0 <= 'len & 'off + 'len <= 'source_len. (
        EvmMemorySliceFields('base, 'source_len),
        int('off),
        int('len),
    ) -> EvmMemorySliceFields('base + 'off, 'len)
function memory_sub_slice(s, off, len) =
    struct { bytes = s.bytes + off, len = len }

/* There is deliberately no code_region_sub_slice: a code-region offset is an
 * opaque host handle, so derived slices must never be built by offset
 * arithmetic. Code accesses pass the whole slice plus a relative index. */

val log_data_sub_slice : forall 'base 'source_len 'off 'len,
                           log_data_valid_range('base, 'source_len) & 0 <= 'off & 0 <= 'len & 'off + 'len <= 'source_len. (
        LogDataSliceFields('base, 'source_len),
        int('off),
        int('len),
    ) -> LogDataSliceFields('base + 'off, 'len)
function log_data_sub_slice(s, off, len) =
    struct { bytes = s.bytes + off, len = len }

/*! A contained calldata sub-range, preserving whether the bytes came from
    top-level input or suspended caller memory. */
val calldata_sub_slice : forall 'off 'len, source_valid_length('off) & source_valid_length('len). (
        CalldataSlice,
        int('off),
        int('len),
    ) -> CalldataSlice
function calldata_sub_slice(s, off, len) = {
    match s {
        InputCalldata(bytes) => if off + len <= bytes.len then {
            InputCalldata(stateless_input_sub_slice(bytes, off, len))
        } else {
            assert(false, "calldata sub-slice bounds");
            EMPTY_CALLDATA
        },
        MemoryCalldata(bytes) => if off + len <= bytes.len then {
            MemoryCalldata(memory_sub_slice(bytes, off, len))
        } else {
            assert(false, "calldata sub-slice bounds");
            EMPTY_CALLDATA
        },
    }
}

overload sub_slice = {
    stateless_input_sub_slice,
    scratch_sub_slice,
    memory_sub_slice,
    log_data_sub_slice,
    calldata_sub_slice
}

/*! The suffix of a slice beginning at a contained relative offset. */
val stateless_input_slice_suffix : forall 'base 'source_len 'relative,
                                     stateless_input_valid_range('base, 'source_len) & 0 <= 'relative & 'relative <= 'source_len. (
        StatelessInputSliceFields('base, 'source_len),
        int('relative),
    ) -> StatelessInputSliceFields('base + 'relative, 'source_len - 'relative)
function stateless_input_slice_suffix(s, off) =
    struct { bytes = s.bytes + off, len = s.len - off }

val scratch_slice_suffix : forall 'base 'source_len 'relative,
                             scratch_valid_range('base, 'source_len) & 0 <= 'relative & 'relative <= 'source_len. (
        ScratchSliceFields('base, 'source_len),
        int('relative),
    ) -> ScratchSliceFields('base + 'relative, 'source_len - 'relative)
function scratch_slice_suffix(s, off) =
    struct { bytes = s.bytes + off, len = s.len - off }

overload slice_suffix = {
    stateless_input_slice_suffix,
    scratch_slice_suffix
}

/*! A log payload retained by the host: either an existing byte slice or the
    canonical big-endian bytes of one EVM word used by system logs. */
union LogData = {
    /* an EVM-memory payload copied into retained log storage */
    LogDataMemory : EvmMemorySlice,
    /* the canonical big-endian bytes of one EVM word */
    LogDataWord : word,
}
