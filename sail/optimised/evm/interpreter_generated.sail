/* Alternate optimized splice for `sail/evm/interpreter.sail`, selected by
 * `EVM_GENERATED_INTERP=on` in place of `evm/interpreter.sail` (which
 * replaces `interpret` with the hand-written C loop). This variant keeps
 * the GENERATED Sail interpreter loop and attaches `$[c_inline]` so that,
 * with the optimized compiler's --c-inline-attr (EVM_INLINE_ATTR=on), the
 * fetch/decode dispatch layers are inlined into `interpret` alongside the
 * `execute` layers annotated in `evm/execute.sail`. It is deliberately NOT
 * listed in `sail/optimised/manifest`.
 *
 * The `interpret` body below textually repeats the canonical run loop from
 * `sail/evm/interpreter.sail` MINUS the proof-only `call_tree_steps_remaining`
 * termination measure: that counter is an unbounded mathematical int that
 * exists only to supply a decreasing measure to total extraction targets,
 * and --c-require-bounded-int (implied by --c-optimized-model) rejects it.
 * Keep this loop in sync with the canonical definition. */

$[c_inline]
val fetch : unit -> ast

$[c_inline]
val decode_simple : opcode -> ast

function interpret() -> OutputSlice = {
    frame_stack_reset();
    var interpreting : bool = true;
    var result : OutputSlice = EMPTY_OUTPUT_SLICE;
    while interpreting do {
        let running = is_running();
        if running then {
            let instruction = fetch();
            execute(instruction)
        } else {
            let output = frame_output();
            let continuation = frame_stack_pop();
            match continuation {
                Empty() => {
                    result = output;
                    interpreting = false
                },
                continuation => resume_frame(continuation, output),
            }
        }
    };
    result
}
