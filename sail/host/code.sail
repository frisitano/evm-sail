
/*md
# Code storage

Content-addressed code storage and the Sail-side `JUMPDEST` analysis.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The stored code for a content hash, or `None` when not witnessed. */
val code_db_lookup = impure { c: "code_db_lookup" } : hash -> option(Code)
/*! Allocates a JUMPDEST bitmap large enough for a code body. */
val jumpdest_table_alloc = impure { c: "jumpdest_table_alloc" } : code_length -> JumpdestRef
/*! Stores one 256-position chunk in an allocated JUMPDEST bitmap. */
val jumpdest_table_store_chunk = impure { c: "jumpdest_table_store_chunk" } : (
        JumpdestRef,
        code_length,
        code_chunk_index,
        JumpdestChunk,
    ) -> bool
/*! Stores code with its precomputed `JUMPDEST` bitmap under its
    KECCAK-256 hash and returns that hash. */
val code_db_store = impure { c: "code_db_store_indexed" } : (CodeSlice, JumpdestRef) -> hash
/*! Whether the referenced bitmap marks the given program counter as a
    valid `JUMPDEST`. */
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (JumpdestRef, code_length, code_pointer) -> bool
/*! Constructs and stores the EIP-7702 delegation designator
    (`0xef0100` followed by the address) with its bitmap, returning its
    code hash. */
val code_intern_delegation = impure { c: "code_intern_indexed_delegation" } : (address, JumpdestRef) -> hash
/*! Reads a stored delegation designator: the leading bit of the 168-bit
    result flags a well-formed designator and carries the delegate address. */
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> AddressResult

/* PUSH immediate bytes are data even when they contain 0x5b. Sail constructs
 * each 256-byte bitmap chunk; the host only allocates and stores the table. */
/*! The partial JUMPDEST bitmap and its location while code is scanned. */
struct CodeAnalysis = {
    chunk : JumpdestChunk,
    chunk_index : code_chunk_index,
    chunk_offset : range(0, 255),
}

/*! Commits a completed nonempty bitmap chunk to its allocated table. */
function store_jumpdest_chunk(table : JumpdestRef, code_len : code_length, analysis : CodeAnalysis) -> unit = {
    if analysis.chunk != EMPTY_JUMPDEST_CHUNK then {
        let stored = jumpdest_table_store_chunk(table, code_len, analysis.chunk_index, analysis.chunk);
        assert(stored, "JUMPDEST chunk store")
    }
}

/*! Returns the one-hot bitmap value for an offset within a 256-byte chunk. */
function jumpdest_bit(index : range(0, 255)) -> JumpdestChunk = {
    var chunk : JumpdestChunk = EMPTY_JUMPDEST_CHUNK;
    chunk[index] = bitone;
    chunk
}

/* The unexamined code suffix strictly decreases; PUSH data is skipped in one step. */
/*! Scans the remaining code, records opcode-aligned JUMPDEST positions, and
    skips immediate bytes belonging to PUSH instructions. */
val analyze_code_from : forall 'pc, code_valid_length('pc). (CodeSlice, Fork, JumpdestRef, int('pc), CodeAnalysis) -> unit
function analyze_code_from(code, fork, table, pc, analysis) = {
    let position = pc;
    let code_len = code.len;
    if position < code_len then {
        var chunk = analysis.chunk;
        let chunk_index = analysis.chunk_index;
        let chunk_offset : range(0, 255) = analysis.chunk_offset;
        let opcode = slice_byte(code, position);
        if opcode == 0x5b then {
            chunk = or_vec(chunk, jumpdest_bit(chunk_offset))
        };
        let opcode_value = unsigned(opcode);
        let step : range(1, 33) = if (96 <= opcode_value) & (opcode_value <= 127) then {
                opcode_value - 94
            } else {
                if (fork >= Amsterdam) & (opcode_value == 230 | opcode_value == 231) then {
                    if deep_stack_immediate_valid(slice_byte(code, position + 1)) then { 2 } else { 1 }
                } else {
                    if (fork >= Amsterdam) & (opcode_value == 232) then {
                        if exchange_immediate_valid(slice_byte(code, position + 1)) then { 2 } else { 1 }
                    } else {
                        1
                    }
                }
            };
        if step < code_len - position then {
            let added = position + step;
            let progressed : range(1, 288) = chunk_offset + step;
            if progressed < 256 then {
                analyze_code_from(
                    code,
                    fork,
                    table,
                    added,
                    struct { chunk = chunk, chunk_index = chunk_index, chunk_offset = progressed },
                )
            } else {
                store_jumpdest_chunk(
                    table,
                    code.len,
                    struct { chunk = chunk, chunk_index = chunk_index, chunk_offset = chunk_offset },
                );
                analyze_code_from(
                    code,
                    fork,
                    table,
                    added,
                    struct { chunk = EMPTY_JUMPDEST_CHUNK, chunk_index = added / 256, chunk_offset = progressed - 256 },
                )
            }
        } else {
            store_jumpdest_chunk(
                table,
                code.len,
                struct { chunk = chunk, chunk_index = chunk_index, chunk_offset = chunk_offset },
            )
        }
    } else {
        store_jumpdest_chunk(table, code.len, analysis)
    }
}
termination_measure analyze_code_from(code, fork, table, pc, analysis) =
    let code_len = code.len in
    let position = pc in
    code_len - position

/*! The PUSH-aware `JUMPDEST` analysis (YP §9.4.3): PUSH immediate bytes
    are data even when they contain `0x5b`. The completed bitmap remains a
    first-class Sail value; the host never scans opcodes. */
function analyze_code(code : CodeSlice, fork : Fork) -> JumpdestRef =
    if code.len == 0 then {
        EMPTY_JUMPDEST_REF
    } else {
        let length = code.len;
        let table = jumpdest_table_alloc(length);
        assert(table != EMPTY_JUMPDEST_REF, "JUMPDEST table allocation");
        analyze_code_from(
            code,
            fork,
            table,
            0,
            struct { chunk = EMPTY_JUMPDEST_CHUNK, chunk_index = 0, chunk_offset = 0 },
        );
        table
    }

/*! Analyzes and stores code, returning its content hash. */
function code_db_insert(code : CodeSlice, fork : Fork) -> hash =
    code_db_store(code, analyze_code(code, fork))

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
