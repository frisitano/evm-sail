
/*md
# Code storage

Content-addressed code storage and the Sail-side `JUMPDEST` analysis.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! The stored code for a content hash, or [EMPTY_CODE][] when not witnessed.
    Nonempty hashes cannot identify empty code, so length zero is an
    unambiguous miss sentinel at this boundary. */
val code_db_lookup = impure { c: "code_db_lookup" } : hash -> Code
/*! Copies immutable input bytes into the executable-code arena and returns
    their stable arena span. */
val code_region_from_input = impure { c: "code_region_from_input" } : StatelessInputSlice -> CodeRegionSlice
/*! Copies active-frame memory into the executable-code arena. */
val code_region_from_memory = impure { c: "code_region_from_memory" } : EvmMemorySlice -> CodeRegionSlice
/*! Copies frozen initcode output into the executable-code arena. */
val code_region_from_output = impure { c: "code_region_from_output" } : OutputSlice -> CodeRegionSlice
/*! Materializes an EIP-7702 delegation designator in the executable-code
    arena so it follows the ordinary analysis and content-addressing path. */
val code_region_from_delegation = impure { c: "code_region_from_delegation" } : address -> CodeRegionSlice
/*! Allocates a JUMPDEST bitmap large enough for a code body. */
val jumpdest_table_alloc = impure { c: "jumpdest_table_alloc" } : CodeSlice -> jump_table_index
/*! Marks one opcode-aligned program counter in an allocated JUMPDEST table. */
val jumpdest_table_mark = impure { c: "jumpdest_table_mark" } : (jump_table_index, code_length, code_pointer) -> bool
/*! Stores code with its precomputed `JUMPDEST` bitmap under its
    KECCAK-256 hash and returns that hash. */
val code_db_store = impure { c: "code_db_store_indexed" } : Code -> hash
/*! Whether the referenced bitmap marks the given program counter as a
    valid `JUMPDEST`. */
val jumpdest_ref_contains = impure { c: "jumpdest_ref_contains" } : (jump_table_index, code_length, code_pointer) -> bool
/*! Reads a stored delegation designator: the leading bit of the 168-bit
    result flags a well-formed designator and carries the delegate address. */
val code_db_read_delegation = impure { c: "code_db_read_delegation" } : hash -> AddressResult

/* The unexamined code suffix strictly decreases; PUSH data is skipped in one step. */
/*! Scans the remaining code, records opcode-aligned JUMPDEST positions, and
    skips immediate bytes belonging to PUSH instructions. The scan is a loop
    rather than a self-call: one step per instruction over a whole contract is
    far beyond the call-stack depth an extraction target can be expected to
    provide. */
val analyze_code_from : forall 'pc, code_valid_length('pc). (CodeSlice, Fork, jump_table_index, int('pc)) -> unit
function analyze_code_from(code, fork, table, pc) = {
    let code_len = code.len;
    var scanning : bool = true;
    var position : code_scan_position = pc;
    while scanning & position < code_len termination_measure(code_len - position) do {
        let current = position;
        let opcode = slice_byte(code, current);
        if opcode == 0x5b then {
            let marked = jumpdest_table_mark(table, code_len, current);
            assert(marked, "JUMPDEST mark")
        };
        let opcode_value = unsigned(opcode);
        let step : range(1, 33) =
            if (96 <= opcode_value) & (opcode_value <= 127)
            then opcode_value - 94
            else if fork >= Amsterdam then {
                let operation = deep_stack_operation(opcode_value);
                let immediate = slice_byte(code, current + 1);
                let immediate_valid = deep_stack_operation_immediate_valid(operation, immediate);
                if immediate_valid then {
                    2
                } else {
                    1
                }
            } else {
                1
            };

        if step < code_len - current then {
            position = current + step
        } else {
            scanning = false
        }
    }
}

/*! The PUSH-aware `JUMPDEST` analysis (YP §9.4.3): PUSH immediate bytes
    are data even when they contain `0x5b`. The completed bitmap remains a
    first-class Sail value; the host never scans opcodes. */
function analyze_code(code : CodeSlice, fork : Fork) -> jump_table_index =
    if code.len == 0 then {
        EMPTY_JUMP_TABLE
    } else {
        let table = jumpdest_table_alloc(code);
        assert(table != EMPTY_JUMP_TABLE, "JUMPDEST table allocation");
        analyze_code_from(code, fork, table, 0);
        table
    }

/*! Analyzes and stores code, returning its content hash. */
function code_db_insert(code : CodeSlice, fork : Fork) -> hash = {
    let jumpdest_table = analyze_code(code, fork);
    let analyzed = analyzed_code(code, jumpdest_table);
    code_db_store(analyzed)
}

/*! Normalizes stateless-input code into the code arena before analysis. */
function code_db_intern_input(bytes : StatelessInputSlice) -> CodeSlice = {
    let region = code_region_from_input(bytes);
    validated_code_slice(region)
}

/*! Normalizes memory-backed initcode into the code arena before analysis. */
function code_db_intern_memory(bytes : EvmMemorySlice) -> CodeSlice = {
    let region = code_region_from_memory(bytes);
    validated_code_slice(region)
}

/*! Normalizes frozen creation output into the code arena before deployment. */
function code_db_intern_output(bytes : OutputSlice) -> CodeSlice = {
    let region = code_region_from_output(bytes);
    validated_code_slice(region)
}

/*! The code for a code hash; `KECCAK_EMPTY` resolves to empty code, and
    an unwitnessed hash is a deficient witness. */
function code_db_resolve(code_hash : hash) -> Code =
    if code_hash == KECCAK_EMPTY then {
        EMPTY_CODE
    } else {
        let code = code_db_lookup(code_hash);
        if code.len == 0 then {
            fatal_error(WitnessDeficient)
        } else {
            code
        }
    }
