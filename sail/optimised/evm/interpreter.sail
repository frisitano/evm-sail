/* Optimized C overrides for `sail/evm/interpreter.sail`. */

/*
 * Optimized interpreter refinement. The base model retains the explicit
 * fetch/decode/run loop; optimized C combines those three dispatch layers
 * while continuing to call the generated Sail per-opcode semantic functions.
 * This definition replaces `interpret` from `evm/interpreter.sail` when the
 * optimized C splice is selected.
 */
val interpret_execution = impure { c: "interpret" } : unit -> OutputSlice

function interpret() -> OutputSlice =
    interpret_execution()
