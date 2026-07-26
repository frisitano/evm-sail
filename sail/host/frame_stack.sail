/*md
# Suspended-frame stack

Nested calls and creates suspend their parent as one
[FrameContinuation][type-FrameContinuation]. This host contract is the
mechanical bounded LIFO behind that semantic operation: the EVM layer enforces
the protocol depth bound, while the backend stores only continuations that are
actually pushed.

The C backend allocates its fixed-capacity arena on the first nested frame and
reuses it thereafter. Popping an empty stack returns [Empty][type-FrameContinuation].
Proof targets model the same contract with an abstract list of continuations.

!!! note "Non-normative"
    This page documents an internal executable-specification contract, not a
    protocol rule.
*/

/*! Removes every suspended-parent continuation. */
val frame_stack_reset = impure { c: "frame_stack_reset" } : unit -> unit
/*! Pushes one suspended-parent continuation. */
val frame_stack_push = impure { c: "frame_stack_push" } : FrameContinuation -> unit
/*! Pops the most recently pushed continuation, or `Empty()` at top level. */
val frame_stack_pop = impure { c: "frame_stack_pop" } : unit -> FrameContinuation
