/* ======================================================================== */
/* SPEC BACKEND: PURE REFERENCE MODELS (STATE)                              */
/*                                                                          */
/* Pure Sail reference implementations of the C-backed host mechanisms,     */
/* over registers holding materialized data. These give the impure          */
/* interface (sail/iface/) definitional semantics in the spec backend: the  */
/* Coq export gets state-monad definitions instead of unconstrained         */
/* parameters, and the build backend's pointer implementations must refine  */
/* exactly this behavior. The registers exist ONLY in spec builds; the      */
/* build backend keeps its state opaquely in C.                             */
/* ======================================================================== */

/* ---- operand stack (iface/stack.sail; refines ffi/stack.c) --------------
 * A stack of call frames, each a list of words with the head as the top of
 * the stack. Observable laws (matching the C): reset -> one empty frame;
 * enter pushes an empty frame; leave drops the top frame but never the
 * bottom one; peek(n) is the nth-from-top word and reads zero past the
 * height; pop/set out of range are no-ops. The EVM's 1024-word limit is
 * enforced by the interpreter before push, not here. */

register spec_hs_frames : list(list(word)) = [|[||]|]

function hs_reset() -> unit = spec_hs_frames = [|[||]|]

function hs_enter() -> unit = spec_hs_frames = [||] :: spec_hs_frames

function hs_leave() -> unit =
    match spec_hs_frames {
        _ :: (f :: rest) => spec_hs_frames = f :: rest,
        _ => (),
    }

function hs_depth() -> bits(64) =
    match spec_hs_frames {
        f :: _ => get_slice_int(64, list_len(f), 0),
        [||] => get_slice_int(64, 0, 0),
    }

function hs_push_w(w : word) -> unit =
    match spec_hs_frames {
        f :: rest => spec_hs_frames = (w :: f) :: rest,
        [||] => (),
    }

function hs_pop() -> unit =
    match spec_hs_frames {
        (_ :: f) :: rest => spec_hs_frames = f :: rest,
        _ => (),
    }

function hs_peek_w(n : bits(64)) -> word =
    match spec_hs_frames {
        f :: _ => list_nth(f, unsigned(n), sail_zeros(256)),
        [||] => sail_zeros(256),
    }

/* replace element `i` of a list (identity when i is out of range) */
val spec_list_set_word : (list(word), int, word) -> list(word)
function spec_list_set_word(xs, i, w) =
    match xs {
        [||] => [||],
        x :: t => if i == 0 then {
            w :: t
        } else {
            x :: spec_list_set_word(t, i - 1, w)
        },
    }

function hs_set_w(n : bits(64), w : word) -> unit =
    match spec_hs_frames {
        f :: rest => spec_hs_frames = spec_list_set_word(f, unsigned(n), w) :: rest,
        [||] => (),
    }
