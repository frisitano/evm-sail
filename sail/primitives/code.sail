
/*md
# Code

Executable code and its PUSH-aware `JUMPDEST` analysis (YP §9.4.3).
*/
/* Opaque reference to one preallocated table; zero is empty. The standard C
 * backend interprets the value as a one-based arena index. The optimized C
 * backend may lower the same bounded semantic value to the address of the
 * table's first byte. */
/*! A reference to a code's completed `JUMPDEST` analysis; zero denotes
    the empty bitmap. */
type jump_table_index = range(0, 2 ^ 64 - 1)

/*md
## Constants

`EMPTY_JUMP_TABLE` denotes empty analysis; `EMPTY_CODE_SLICE` and `EMPTY_CODE`
are the canonical executable-code placeholders.
*/
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
    NotDeepStackOperation,
}

/*! Classifies an opcode against Amsterdam's immediate deep-stack
    operations; every other opcode maps to the non-member. */
function deep_stack_operation(opcode : opcode) -> DeepStackOperation =
    match opcode {
        230 => DeepStackDuplicate,
        231 => DeepStackSwap,
        232 => DeepStackExchange,
        _ => NotDeepStackOperation,
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
    NotDeepStackOperation => false,
}

/*! Executable code: its byte address, length, and resolved `JUMPDEST` table.
    The flat dependent record keeps the byte address and length relationship
    explicit while allowing optimized C to represent both addresses directly
    as pointers. The interpreter saves and restores all three together; the
    code hash remains the stable code-DB key. */
struct CodeFields('off : Int, 'len : Int),
    code_region_valid_range('off, 'len) & code_valid_length('len) = {
    bytes : int('off),
    len : int('len),
    jumpdests : jump_table_index,
}

/*! Existential executable-code value whose concrete byte address and length
    remain correlated inside `CodeFields`. */
type Code = {
    'off 'len,
    code_region_valid_range('off, 'len) & code_valid_length('len).
    CodeFields('off, 'len)
}

/*! Combines an executable byte span with its completed analysis without
    nesting one slice record inside another. */
val analyzed_code : forall 'off 'len,
    code_region_valid_range('off, 'len) & code_valid_length('len).
    (CodeRegionSliceFields('off, 'len), jump_table_index) -> CodeFields('off, 'len)
function analyzed_code(bytes, jumpdests) =
    struct { bytes = bytes.bytes, len = bytes.len, jumpdests = jumpdests }

/*! Reconstructs the nominal byte-span view used by generic region readers.
    Optimized C preserves the pointer representation through this projection. */
val code_bytes : forall 'off 'len,
    code_region_valid_range('off, 'len) & code_valid_length('len).
    CodeFields('off, 'len) -> CodeRegionSliceFields('off, 'len)
function code_bytes(code) = struct { bytes = code.bytes, len = code.len }

let EMPTY_CODE : Code = analyzed_code(EMPTY_CODE_SLICE, EMPTY_JUMP_TABLE)
