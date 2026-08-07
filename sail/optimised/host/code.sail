/* Optimized C override for `sail/host/code.sail`. */

/*
 * The specification retains the structurally recursive JUMPDEST analysis.
 * Optimized guests scan the same code iteratively so the guest stack does not
 * grow once per opcode for large deployed contracts.
 */
val analyze_code_indexed = impure { c: "code_db_analyze_indexed" } : (CodeSlice, bool) -> jump_table_index

val code_db_resolve_indexed = impure { c: "code_db_resolve_indexed" } : hash -> Code

function analyze_code(code : CodeSlice, fork : Fork) -> jump_table_index =
    analyze_code_indexed(code, fork >= Amsterdam)

function code_db_resolve(code_hash : hash) -> Code =
    code_db_resolve_indexed(code_hash)
