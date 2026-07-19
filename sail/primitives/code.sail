
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

let EMPTY_JUMPDEST_CHUNK : JumpdestChunk = 0x0000000000000000000000000000000000000000000000000000000000000000
let EMPTY_JUMPDEST_REF : JumpdestRef = 0x0000000000000000

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
    bytes : ByteSlice,
    jumpdests : JumpdestRef,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_SLICE, jumpdests = EMPTY_JUMPDEST_REF }
