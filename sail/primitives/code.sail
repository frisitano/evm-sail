
/* One bit per code byte, with the lowest code offset in the least-significant
 * bit. Analysis writes one completed 256-byte chunk at a time. */
/*md
# Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3).
*/
/*! A 256-byte segment of a PUSH-aware `JUMPDEST` bitmap. */
type JumpdestChunk = bits(256)

/* Opaque handle to one preallocated table; zero is empty. */
/*! A reference to a code's completed `JUMPDEST` analysis; zero denotes
    the empty bitmap. */
type JumpdestRef = bits(64)

/*md
## Constants

`EMPTY_JUMPDEST_CHUNK` and `EMPTY_JUMPDEST_REF` denote empty analysis;
`EMPTY_CODE_SLICE` and `EMPTY_CODE` are the canonical executable-code
placeholders.
*/
let EMPTY_JUMPDEST_CHUNK : JumpdestChunk = 0x0000000000000000000000000000000000000000000000000000000000000000
let EMPTY_JUMPDEST_REF : JumpdestRef = 0x0000000000000000

/*! A source-backed executable byte span. Its length carries the separate
    representation invariant needed by program-counter arithmetic; this is
    not a protocol deployment-size limit. */
type CodeSlice = {
    'off 'len,
    source_valid_range('off, 'len) & code_valid_length('len).
    ByteSliceFields('off, 'len)
}

/*! Reclassifies a byte span after its producer has established the executable
    length invariant. */
val code_slice : forall 'off 'len,
    source_valid_range('off, 'len) & code_valid_length('len).
    ByteSliceFields('off, 'len) -> CodeSlice
function code_slice(bytes) = bytes

/*! Converts a source span whose producer guarantees executable cursor
    headroom. This is vacuous in the canonical model; the optimized splice
    checks the representation invariant if a proof was erased by storage in a
    non-dependent aggregate. */
function validated_code_slice(bytes : ByteSlice) -> CodeSlice =
    code_slice(bytes)

/*! Canonical empty executable code. */
let EMPTY_CODE_SLICE : CodeSlice = code_slice(byte_slice(StatelessInputSource, 0, 0))

/*! Whether an EIP-8024 `DUPN`/`SWAPN` immediate is valid. Invalid
    immediates remain opcode-aligned during JUMPDEST analysis. */
function deep_stack_immediate_valid(immediate : byte) -> bool = {
    let value : opcode = unsigned(immediate);
    value <= 90 | 128 <= value
}

/*! Whether an EIP-8024 `EXCHANGE` immediate is valid. Invalid immediates
    remain opcode-aligned during JUMPDEST analysis. */
function exchange_immediate_valid(immediate : byte) -> bool = {
    let value : opcode = unsigned(immediate);
    value <= 81 | 128 <= value
}

/*! Executable code: its byte span and resolved `JUMPDEST` table. The
    interpreter saves and restores both together; the code hash remains
    the stable code-DB key. */
struct Code = {
    bytes : CodeSlice,
    jumpdests : JumpdestRef,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_CODE_SLICE, jumpdests = EMPTY_JUMPDEST_REF }
