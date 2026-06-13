/* ======================================================================== */
/* EVM USER-SPACE machine: per-frame registers, operand stack, gas counter,  */
/* and the saved-frame stack for nested calls                                */
/*                                                                           */
/* This is the user-space half. It holds the gas counter and all per-frame   */
/* compute state (pc, stack, memory, code, message, returndata). World state */
/* lives in the host (host/kernel.sail) and is reached only via k_* calls.    */
/* World rollback on revert is a kernel snapshot, NOT saved here.             */
/* ======================================================================== */

/* The per-frame call context (YP "message call" I = (I_a, I_o, I_s, ...)).
 * One Message per active frame; sub-calls save/restore it (interpreter.sail). */
struct Message = {
    msg_caller       : address,  /* CALLER (I_s)                                  */
    msg_address      : address,  /* ADDRESS / storage owner = self (I_a)          */
    msg_code_address : address,  /* account whose code runs (differs under        */
                                 /* DELEGATECALL/CALLCODE and EIP-7702 delegation) */
    msg_value        : word,     /* CALLVALUE (I_v)                               */
    msg_data_len     : int,      /* CALLDATA length (bytes are C-backed, host_mem) */
    msg_is_static    : bool,     /* inside a STATICCALL frame (EIP-214)           */
    msg_depth        : int,      /* call depth (YP I_e), bounded by DEPTH_LIMIT   */
}

let DEFAULT_MESSAGE : Message = struct {
        msg_caller = ZERO_ADDR,
        msg_address = ZERO_ADDR,
        msg_code_address = ZERO_ADDR,
        msg_value = ZERO_WORD,
        msg_data_len = 0,
        msg_is_static = false,
        msg_depth = 0,
    }

/* ----------------------- active-frame registers ------------------------- */

register pc : int = 0
register gas_remaining : gas = 0 /* THE gas counter (user-space) */
register frame_status : FrameStatus = Running()
register message : Message = DEFAULT_MESSAGE
register call_depth : int = 0

/* --------------------------- code length -------------------------------- */
/* The frame's code bytes live in the C buffer (host_code.c), streamed once by
 * set_code; fetch is an O(1) array read and PUSH immediates are assembled by
 * one hc_word call. Only the length is kept on the Sail side. */
register code_len : int = 0

val hj_valid = impure { _: "hj_valid" } : bits(64) -> bool
val hc_byte = impure { _: "hc_byte" } : bits(64) -> bits(8)
val hc_word = impure { _: "hc_word" } : (bits(64), bits(64)) -> word
val fc_set_hash = impure { _: "fc_set_hash" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val fc_pend_mem = impure { _: "fc_pend_mem" } : (bits(64), bits(64)) -> bits(64)
val fc_set_txd = impure { _: "fc_set_txd" } : unit -> bits(64)
val fc_set_empty = impure { _: "fc_set_empty" } : unit -> unit
val hc_len = impure { _: "hc_len" } : unit -> bits(64)

/* Bind the current frame's code to an account's store entry (O(1): the bytes
 * and JUMPDEST bitmap were prepared when the code was written), to the
 * streamed tx input, or to nothing. CREATE initcode uses frame_code_pend_mem
 * BEFORE the frame switch: the child's code is an alias of this frame's
 * memory range (the parent is frozen while the child runs). */
function frame_code_addr(a : address) -> unit = {
    let h : hash = k_code_key(a);
    code_len = unsigned(fc_set_hash(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]))
}

/* re-read the (C-side) frame descriptor's length after a child frame pops.
 * NOT a saved Sail local: sail -O's register-effect analysis is per-function
 * body, so a `let saved = code_len ... code_len = saved` round-trip with no
 * direct write in between gets eliminated as a self-assignment. */
function frame_code_resync() -> unit = code_len = unsigned(hc_len())
function frame_code_txd() -> unit = code_len = unsigned(fc_set_txd())
function frame_code_empty() -> unit = {
    fc_set_empty();
    code_len = 0
}
function frame_code_pend_mem(off : int, len : int) -> int =
    unsigned(fc_pend_mem(get_slice_int(64, off, 0), get_slice_int(64, len, 0)))

/* code[i], zero past the end (the EVM's implicit STOP padding) */
function code_byte(i : int) -> byte = hc_byte(get_slice_int(64, i, 0))

/* ------------------------------ the stack ------------------------------- */
/* C-backed (ffi/host_stack.c): per-call-frame flat word arrays, O(1)
 * push/pop/peek/set, zero steady-state allocation (a Sail list(word) paid a
 * heap cons per PUSH). The frame lifecycle pairs with calls: stack_frame_enter
 * on a sub-call, stack_frame_leave on return, stack_clear per transaction. */

val hs_reset = impure { _: "hs_reset" } : unit -> unit
val hs_enter = impure { _: "hs_enter" } : unit -> unit
val hs_leave = impure { _: "hs_leave" } : unit -> unit
val hs_depth = impure { _: "hs_depth" } : unit -> bits(64)
val hs_push_w = impure { _: "hs_push_w" } : word -> unit
val hs_pop = impure { _: "hs_pop" } : unit -> unit
val hs_peek_w = impure { _: "hs_peek_w" } : bits(64) -> word
val hs_set_w = impure { _: "hs_set_w" } : (bits(64), word) -> unit

let STACK_LIMIT : int = 1024 /* YP: 1024-element operand-stack limit */

/* An exceptional halt (OOG, invalid opcode/jump, stack error, write protection,
 * initcode-too-large) consumes ALL remaining gas -- unlike REVERT, which returns
 * it. Route every exceptional halt through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = {
    gas_remaining = 0;
    frame_status = Exceptional(k)
}

function stack_clear() -> unit = hs_reset()
function stack_frame_enter() -> unit = hs_enter()
function stack_frame_leave() -> unit = hs_leave()

function stack_height() -> int = unsigned(hs_depth())

/* nth-from-top operand (n = 0 is the top); zero if out of range */
function peek(n : int) -> word = hs_peek_w(get_slice_int(64, n, 0))

function push(w : word) -> unit =
    if stack_height() >= STACK_LIMIT then {
        exc_halt(StackOverflow)
    } else {
        hs_push_w(w)
    }

function pop() -> word =
    if stack_height() == 0 then {
        exc_halt(StackUnderflow);
        WORD_ZERO
    } else {
        let w = peek(0);
        hs_pop();
        w
    }

/* overwrite the nth-from-top operand (SWAP) */
function stack_set(n : int, w : word) -> unit = hs_set_w(get_slice_int(64, n, 0), w)

function pop_int() -> int = unsigned(pop())

function is_running() -> bool = match frame_status {
    Running() => true,
    _ => false,
}
