/* ======================================================================== */
/* OPERAND STACK                                                            */
/*                                                                          */
/* The per-frame EVM operand stack (Yellow Paper section 9.1, mu_s): up to  */
/* 1024 words, LIFO. The evm layer (machine.sail push/pop) enforces the     */
/* depth bounds and charges nothing here -- this is pure mechanism. Frames  */
/* form a stack of stacks: stack_enter_frame on a sub-call,                 */
/* stack_leave_frame on return, stack_reset per transaction. The words live */
/* in C (ffi/stack.c) as per-frame flat arrays for O(1) access; proof       */
/* targets see the vals as bodyless axioms over an abstract per-frame       */
/* list-of-words model.                                                     */
/* ======================================================================== */

val stack_reset = impure { c: "stack_reset" } : unit -> unit
val stack_enter_frame = impure { c: "stack_enter_frame" } : unit -> unit
val stack_leave_frame = impure { c: "stack_leave_frame" } : unit -> unit
val stack_depth = impure { c: "stack_depth" } : unit -> operand_stack_height
val stack_push_word = impure { c: "stack_push_word" } : word -> unit
val stack_pop_word = impure { c: "stack_pop_word" } : unit -> word /* pop AND return the top (caller guards underflow) */
val stack_peek_word = impure { c: "stack_peek_word" } : stack_index -> word
val stack_set_word = impure { c: "stack_set_word" } : (stack_index, word) -> unit
