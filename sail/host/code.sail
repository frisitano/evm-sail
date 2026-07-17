/* Content-addressed code storage and Sail JUMPDEST analysis. */

val code_db_lookup = impure { c: "code_db_lookup" } : hash -> option(Code)
val jumpdest_table_alloc = impure { c: "jumpdest_table_alloc" } : byte_quantity -> JumpdestRef
val jumpdest_table_store_chunk = impure { c: "jumpdest_table_store_chunk" } : (
        JumpdestRef,
        byte_quantity,
        byte_quantity,
        JumpdestChunk,
    ) -> bool
val code_db_store = impure { c: "code_db_store_indexed" } : (ByteSlice, JumpdestRef) -> hash
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (JumpdestRef, byte_quantity, byte_quantity) -> bool
val code_intern_delegation = impure { c: "code_intern_indexed_delegation" } : (address, JumpdestRef) -> hash
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> bits(168)

/* PUSH immediate bytes are data even when they contain 0x5b. Sail constructs
 * each 256-byte bitmap chunk; the host only allocates and stores the table. */
struct CodeAnalysis = {
    chunk : JumpdestChunk,
    chunk_index : code_pointer,
    chunk_offset : range(0, 255),
}

function store_jumpdest_chunk(table : JumpdestRef, code_len : byte_length, analysis : CodeAnalysis) -> unit = {
    if analysis.chunk != EMPTY_JUMPDEST_CHUNK then {
        let stored = jumpdest_table_store_chunk(table, code_len, analysis.chunk_index, analysis.chunk);
        assert(stored, "JUMPDEST chunk store")
    }
}

function jumpdest_bit(index : range(0, 255)) -> JumpdestChunk = {
    var chunk : JumpdestChunk = EMPTY_JUMPDEST_CHUNK;
    chunk[index] = bitone;
    chunk
}

/* The unexamined code suffix strictly decreases; PUSH data is skipped in one step. */
function analyze_code_from(code : ByteSlice, table : JumpdestRef, pc : code_pointer, analysis : CodeAnalysis) -> unit =
    if pc < code.len then {
        var chunk = analysis.chunk;
        let chunk_index : code_pointer = analysis.chunk_index;
        let chunk_offset : range(0, 255) = analysis.chunk_offset;
        let opcode = slice_byte(code, pc);
        if opcode == 0x5b then {
            chunk = or_vec(chunk, jumpdest_bit(chunk_offset))
        };
        let opcode_value = unsigned(opcode);
        let step : range(1, 33) = if (96 <= opcode_value) & (opcode_value <= 127) then {
                opcode_value - 94
            } else {
                1
            };
        let step_quantity = ByteQuantity(step);
        if step_quantity < code.len - pc then {
            let added : code_pointer = pc + step_quantity;
            let progressed : range(1, 288) = chunk_offset + step;
            if progressed < 256 then {
                analyze_code_from(
                    code,
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
                    table,
                    added,
                    struct {
                        chunk = EMPTY_JUMPDEST_CHUNK,
                        chunk_index = chunk_index + BYTE_ONE,
                        chunk_offset = progressed - 256,
                    },
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
termination_measure analyze_code_from(code, table, pc, analysis) =
    let ByteQuantity(code_len) = code.len in
    let ByteQuantity(position) = pc in
    code_len - position

function analyze_code(code : ByteSlice) -> JumpdestRef =
    if code.len == BYTE_ZERO then {
        EMPTY_JUMPDEST_REF
    } else {
        let table = jumpdest_table_alloc(code.len);
        assert(table != EMPTY_JUMPDEST_REF, "JUMPDEST table allocation");
        analyze_code_from(
            code,
            table,
            BYTE_ZERO,
            struct { chunk = EMPTY_JUMPDEST_CHUNK, chunk_index = BYTE_ZERO, chunk_offset = 0 },
        );
        table
    }

function code_db_insert(code : ByteSlice) -> hash =
    code_db_store(code, analyze_code(code))

function code_db_resolve(code_hash : hash) -> Code =
    if code_hash == KECCAK_EMPTY then {
        EMPTY_CODE
    } else {
        match code_db_lookup(code_hash) {
            Some(code) => code,
            None() => throw InvalidBlock(WitnessDeficient),
        }
    }
