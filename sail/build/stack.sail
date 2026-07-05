/* Build backend: C FFI bindings for the per-call-frame operand stack
 * (ffi/stack.c). */

val hs_reset_ffi = impure { _: "hs_reset" } : unit -> unit
function hs_reset() -> unit = hs_reset_ffi()

val hs_enter_ffi = impure { _: "hs_enter" } : unit -> unit
function hs_enter() -> unit = hs_enter_ffi()

val hs_leave_ffi = impure { _: "hs_leave" } : unit -> unit
function hs_leave() -> unit = hs_leave_ffi()

val hs_depth_ffi = impure { _: "hs_depth" } : unit -> bits(64)
function hs_depth() -> bits(64) = hs_depth_ffi()

val hs_push_w_ffi = impure { _: "hs_push_w" } : word -> unit
function hs_push_w(x0 : word) -> unit = hs_push_w_ffi(x0)

val hs_pop_ffi = impure { _: "hs_pop" } : unit -> unit
function hs_pop() -> unit = hs_pop_ffi()

val hs_peek_w_ffi = impure { _: "hs_peek_w" } : bits(64) -> word
function hs_peek_w(x0 : bits(64)) -> word = hs_peek_w_ffi(x0)

val hs_set_w_ffi = impure { _: "hs_set_w" } : (bits(64), word) -> unit
function hs_set_w(x0 : bits(64), x1 : word) -> unit = hs_set_w_ffi(x0, x1)
