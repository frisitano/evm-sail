/* Optimized C override for `sail/host/code.sail`. */

/*
 * The specification retains the structurally recursive JUMPDEST analysis.
 * Optimized guests scan the same code iteratively so the guest stack does not
 * grow once per opcode for large deployed contracts.
 */
val optimized_analyze_code = impure { c: "code_db_analyze_indexed" } :
    (CodeSlice, bool) -> jump_table_index

function analyze_code(code : CodeSlice, fork : Fork) -> jump_table_index =
    optimized_analyze_code(code, fork >= Amsterdam)
