/* ======================================================================== */
/* EVM USER-SPACE machine: per-frame registers, operand stack, gas counter, */
/* and the saved-frame stack for nested calls                               */
/*                                                                          */
/* This is the user-space half. It holds the gas counter and all per-frame  */
/* compute state (pc, stack, memory, code, message, returndata). World      */
/* state lives in the host (host/kernel.sail) and is reached only via k_*   */
/* calls. World rollback on revert is a kernel snapshot, NOT saved here.    */
/* ======================================================================== */

/* The per-frame call context (YP "message call" I = (I_a, I_o, I_s, ...)).
 * One Message per active frame; sub-calls save/restore it (interpreter.sail). */
/* ----------------------- active-frame registers ------------------------- */

register pc : int = 0
register gas_remaining : gas = 0 /* THE gas counter (user-space) */
register frame_status : FrameStatus = Running()
register message : Message = DEFAULT_MESSAGE
register call_depth : int = 0

/* --------------------------- code length -------------------------------- */
/* The frame's code bytes live in the C buffer (code_db.c), streamed once by
 * set_code; fetch is an O(1) array read and PUSH immediates are assembled by
 * one frame_push_immediate_word call. Only the length is kept on the Sail side. */
register code_len : int = 0

/* Bind the current frame's code to an account's store entry (O(1): the bytes
 * and JUMPDEST bitmap were prepared when the code was written), to the
 * streamed tx input, or to nothing. CREATE initcode uses frame_code_pend_mem
 * BEFORE the frame switch: the child's code is copied from this frame's memory
 * range, so execution code never aliases mutable memory. */
function frame_code_addr(a : address) -> unit = {
    let h : hash = k_code_key(a);
    code_len = unsigned(frame_code_bind_stored(h))
}

/* re-read the (C-side) frame descriptor's length after a child frame pops.
 * NOT a saved Sail local: sail -O's register-effect analysis is per-function
 * body, so a `let saved = code_len ... code_len = saved` round-trip with no
 * direct write in between gets eliminated as a self-assignment. */
function frame_code_resync() -> unit = code_len = unsigned(frame_code_length())
function frame_code_txd() -> unit = code_len = unsigned(frame_code_bind_tx_input())
function frame_code_empty() -> unit = {
    frame_code_clear();
    code_len = 0
}
function frame_code_pend_mem(off : int, len : int) -> int =
    unsigned(frame_code_bind_memory(get_slice_int(64, off, 0), get_slice_int(64, len, 0)))

/* code[i], zero past the end (the EVM's implicit STOP padding) */
function code_byte(i : int) -> byte = frame_code_byte(get_slice_int(64, i, 0))

/* ------------------------------ the stack ------------------------------- */

let STACK_LIMIT : int = 1024 /* YP: 1024-element operand-stack limit */

/* An exceptional halt (OOG, invalid opcode/jump, stack error, write protection,
 * initcode-too-large) consumes ALL remaining gas -- unlike REVERT, which returns
 * it. Route every exceptional halt through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = {
    gas_remaining = 0;
    frame_status = Exceptional(k)
}

function stack_clear() -> unit = stack_reset()
function stack_frame_enter() -> unit = stack_enter_frame()
function stack_frame_leave() -> unit = stack_leave_frame()

function stack_height() -> int = unsigned(stack_depth())

/* nth-from-top operand (n = 0 is the top); zero if out of range */
function peek(n : int) -> word = stack_peek_word(get_slice_int(64, n, 0))

function push(w : word) -> unit =
    if stack_height() >= STACK_LIMIT then {
        exc_halt(StackOverflow)
    } else {
        stack_push_word(w)
    }

function pop() -> word =
    if stack_height() == 0 then {
        exc_halt(StackUnderflow);
        WORD_ZERO
    } else {
        stack_pop_word()
    }

/* overwrite the nth-from-top operand (SWAP) */
function stack_set(n : int, w : word) -> unit = stack_set_word(get_slice_int(64, n, 0), w)

function pop_int() -> int = unsigned(pop())

function is_running() -> bool = match frame_status {
    Running() => true,
    _ => false,
}
