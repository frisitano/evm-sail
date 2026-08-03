
/* One bit per code byte, with the lowest code offset in the least-significant
 * bit. Analysis writes one completed 256-byte chunk at a time. */
/*md
# Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3).
*/
/*! A 256-byte segment of a PUSH-aware `JUMPDEST` bitmap. Bit `i` marks byte
    position `i`. */
type JumpdestChunk = bits(256)

/* Opaque reference to one preallocated table; zero is empty. The standard C
 * backend interprets the value as a one-based arena index. The optimized C
 * backend may lower the same bounded semantic value to the address of the
 * table's first packed word. */
/*! A reference to a code's completed `JUMPDEST` analysis; zero denotes
    the empty bitmap. */
type jump_table_index = range(0, 2 ^ 64 - 1)

/*md
## Constants

`EMPTY_JUMPDEST_CHUNK` and `EMPTY_JUMP_TABLE` denote empty analysis;
`EMPTY_CODE_SLICE` and `EMPTY_CODE` are the canonical executable-code
placeholders.
*/
let EMPTY_JUMPDEST_CHUNK : JumpdestChunk =
    0x0000000000000000000000000000000000000000000000000000000000000000
let EMPTY_JUMP_TABLE : jump_table_index = 0

/*! A source-backed executable byte span. Its length carries the separate
    representation invariant needed by program-counter arithmetic; this is
    not a protocol deployment-size limit. */
type CodeSlice = {
    'off 'len,
    code_region_valid_range('off, 'len) & code_valid_length('len).
    CodeRegionSliceFields('off, 'len)
}

/*! Reclassifies a byte span after its producer has established the executable
    length invariant. */
val code_slice : forall 'off 'len, code_region_valid_range('off, 'len) & code_valid_length('len). CodeRegionSliceFields(
        'off,
        'len,
    ) -> CodeSlice
function code_slice(bytes) = bytes

/*! Converts a source span whose producer guarantees executable cursor
    headroom. The explicit check re-establishes the proof after the length has
    crossed a non-dependent host boundary. */
function validated_code_slice(bytes : CodeRegionSlice) -> CodeSlice =
    if bytes.len <= sizeof(code_region_bound) - 32 then {
        code_slice(bytes)
    } else {
        assert(false, "executable code cursor headroom");
        code_slice(EMPTY_CODE_REGION_SLICE)
    }

/*! Canonical empty executable code. */
let EMPTY_CODE_SLICE : CodeSlice = code_slice(EMPTY_CODE_REGION_SLICE)

/*! The closed family of Amsterdam opcodes whose instruction encoding carries
    one validity-sensitive immediate byte. Keeping this classification in the
    specification lets instruction fetch and PUSH-aware code analysis share
    one dispatch without introducing a function-valued decoder. */
enum DeepStackOperation = {
    DeepStackDuplicate,
    DeepStackSwap,
    DeepStackExchange,
}

/*! Classifies an opcode as one of Amsterdam's immediate deep-stack
    operations. */
function deep_stack_operation(opcode : opcode) -> option(DeepStackOperation) =
    match opcode {
        230 => Some(DeepStackDuplicate),
        231 => Some(DeepStackSwap),
        232 => Some(DeepStackExchange),
        _ => None(),
    }

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

/*! Applies the immediate-validity rule selected by a decoded deep-stack
    operation. `DUPN` and `SWAPN` share the single-index encoding, while
    `EXCHANGE` uses the pair encoding. */
function deep_stack_operation_immediate_valid(
    operation : DeepStackOperation,
    immediate : byte,
) -> bool = match operation {
    DeepStackDuplicate => deep_stack_immediate_valid(immediate),
    DeepStackSwap => deep_stack_immediate_valid(immediate),
    DeepStackExchange => exchange_immediate_valid(immediate),
}

/*! Executable code: its byte span and resolved `JUMPDEST` table. The
    interpreter saves and restores both together; the code hash remains
    the stable code-DB key. */
struct Code = {
    bytes : CodeSlice,
    jumpdests : jump_table_index,
}

let EMPTY_CODE : Code = struct { bytes = EMPTY_CODE_SLICE, jumpdests = EMPTY_JUMP_TABLE }
