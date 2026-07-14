/*md
# Code storage

Content-addressed code storage and the Sail-side `JUMPDEST` analysis.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! The stored code for a content hash, or `None` when not witnessed. */
val code_db_lookup = impure { c: "code_db_lookup" } : hash -> option(Code)
/*! Stores code with its precomputed `JUMPDEST` bitmap under its
    KECCAK-256 hash and returns that hash. */
val code_db_store = impure { c: "code_db_store_indexed" } : (ByteSlice, JumpdestBitmap) -> hash
/*! Whether the referenced bitmap marks the given program counter as a
    valid `JUMPDEST`. */
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (JumpdestRef, bits(64), bits(64)) -> bool
/*! Constructs and stores the EIP-7702 delegation designator
    (`0xef0100` followed by the address) with its bitmap, returning its
    code hash. */
val code_intern_delegation = impure { c: "code_intern_indexed_delegation" } : (address, JumpdestBitmap) -> hash
/*! Reads a stored delegation designator: the leading bit of the 168-bit
    result flags a well-formed designator, the low 160 bits are the
    delegate address. */
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> bits(168)

/*! The PUSH-aware `JUMPDEST` analysis (YP §9.4.3): PUSH immediate bytes
    are data even when they contain `0x5b`. The completed bitmap remains a
    first-class Sail value; the host never scans opcodes. */
function analyze_code(code : ByteSlice) -> JumpdestBitmap = {
    let len = unsigned(code.len);
    let word_count = ediv_int(len + 63, 64);
    var words : JumpdestBitmap = [||];
    var word : bits(64) = 0x0000000000000000;
    var word_index : int = 0;
    var pc : int = 0;
    while pc < len do {
        let next_word = ediv_int(pc, 64);
        while word_index < next_word do {
            words = word :: words;
            word = 0x0000000000000000;
            word_index = word_index + 1
        };
        let opcode = unsigned(slice_byte(code, pc));
        if opcode == 91 then {
            word = or_vec(word, sail_shiftleft(0x0000000000000001, emod_int(pc, 64)))
        };
        pc =
            if (opcode >= 96) & (opcode <= 127) then {
                pc + opcode - 94
            } else {
                pc + 1
            }
    };
    while word_index < word_count do {
        words = word :: words;
        word = 0x0000000000000000;
        word_index = word_index + 1
    };
    list_reverse(words)
}

/*! Analyzes and stores code, returning its content hash. */
function code_db_insert(code : ByteSlice) -> hash =
    code_db_store(code, analyze_code(code))

/*! The code for a code hash; `KECCAK_EMPTY` resolves to empty code, and
    an unwitnessed hash is a deficient witness. */
function code_db_resolve(code_hash : hash) -> Code =
    if code_hash == KECCAK_EMPTY then {
        EMPTY_CODE
    } else {
        match code_db_lookup(code_hash) {
            Some(code) => code,
            None() => throw InvalidBlock(WitnessDeficient),
        }
    }
