/* ======================================================================== */
/* HOST INTERFACE CONTRACT: OPERAND STACK                                   */
/*                                                                          */
/* Abstract `val` declarations for the C-backed EVM operand stack (part of  */
/* the "Memory, calldata, returndata, code, stack" contract group of        */
/* proof/extern-boundary.md). Frame push/pop is LIFO and the hs_*           */
/* operations refine an abstract per-frame list-of-words model: build       */
/* implementations must refine those materialized list-model semantics.     */
/* ======================================================================== */

/* C-backed (ffi/stack.c): per-call-frame flat word arrays, O(1)
 * push/pop/peek/set, zero steady-state allocation (a Sail list(word) paid a
 * heap cons per PUSH). The frame lifecycle pairs with calls: stack_frame_enter
 * on a sub-call, stack_frame_leave on return, stack_clear per transaction. */

val hs_reset : unit -> unit
val hs_enter : unit -> unit
val hs_leave : unit -> unit
val hs_depth : unit -> bits(64)
val hs_push_w : word -> unit
val hs_pop : unit -> unit
val hs_peek_w : bits(64) -> word
val hs_set_w : (bits(64), word) -> unit
