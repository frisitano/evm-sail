/* ======================================================================== */
/* EVM USER-SPACE machine: per-frame registers, operand stack, gas counter,  */
/* and the saved-frame stack for nested calls                                */
/*                                                                           */
/* This is the user-space half. It holds the gas counter and all per-frame   */
/* compute state (pc, stack, memory, code, message, returndata). World state */
/* lives in the kernel (el_kernel.sail) and is reached only via k_* syscalls. */
/* World rollback on revert is a kernel snapshot, NOT saved here.             */
/* ======================================================================== */

/* Per-frame message / call context (user-space). */
struct Message = {
  msg_caller       : address,
  msg_address      : address,    /* ADDRESS / storage owner (self)          */
  msg_code_address : address,    /* account whose code executes             */
  msg_value        : word,       /* CALLVALUE                               */
  msg_data         : list(byte), /* CALLDATA                                */
  msg_is_static    : bool,
  msg_depth        : int
}

let DEFAULT_MESSAGE : Message = struct {
  msg_caller = ZERO_ADDR, msg_address = ZERO_ADDR, msg_code_address = ZERO_ADDR,
  msg_value = ZERO_WORD, msg_data = [||], msg_is_static = false, msg_depth = 0
}

/* ----------------------- active-frame registers ------------------------- */

register pc            : int        = 0
register gas_remaining : gas        = 0      /* THE gas counter (user-space) */
register frame_status  : FrameStatus = Running()
register code          : list(byte) = [||]
register message       : Message    = DEFAULT_MESSAGE
register call_depth    : int        = 0

/* --------------------------- code length -------------------------------- */
/* The frame's code bytes live in the C buffer (host_code.c), streamed once by
 * set_code; fetch is an O(1) array read and PUSH immediates are assembled by
 * one hc_word call. Only the length is kept on the Sail side. */
register code_len      : int        = 0

val hj_begin = impure "hj_begin" : unit -> unit
val hj_byte  = impure "hj_byte"  : bits(8) -> unit
val hj_valid = impure "hj_valid" : bits(64) -> bool
val hc_byte  = impure "hc_byte"  : bits(64) -> bits(8)
val hc_word  = impure "hc_word"  : (bits(64), bits(64)) -> word

/* The single setter for the `code` register: one walk measures the length AND
 * streams the bytes into the C JUMPDEST bitmap (PUSH immediates skipped), so
 * jump validity is an O(1) bit test instead of a per-jump O(n^2) code scan. */
function set_code(c : list(byte)) -> unit = {
  code = c;
  hj_begin();
  var n : int = 0;
  var rest : list(byte) = c;
  var more : bool = true;
  while more do {
    match rest { b :: t => { hj_byte(b); n = n + 1; rest = t }, [||] => more = false }
  };
  code_len = n
}

/* code[i], zero past the end (the EVM's implicit STOP padding) */
function code_byte(i : int) -> byte = hc_byte(get_slice_int(64, i, 0))

/* ------------------------------ the stack ------------------------------- */
/* C-backed (ffi/host_stack.c): per-call-frame flat word arrays, O(1)
 * push/pop/peek/set, zero steady-state allocation (a Sail list(word) paid a
 * heap cons per PUSH). The frame lifecycle pairs with calls: stack_frame_enter
 * on a sub-call, stack_frame_leave on return, stack_clear per transaction. */

val hs_reset = impure "hs_reset" : unit -> unit
val hs_enter = impure "hs_enter" : unit -> unit
val hs_leave = impure "hs_leave" : unit -> unit
val hs_depth = impure "hs_depth" : unit -> bits(64)
val hs_push_w = impure "hs_push_w" : word -> unit
val hs_pop    = impure "hs_pop"    : unit -> unit
val hs_peek_w = impure "hs_peek_w" : bits(64) -> word
val hs_set_w  = impure "hs_set_w"  : (bits(64), word) -> unit

let STACK_LIMIT : int = 1024

/* An exceptional halt (OOG, invalid opcode/jump, stack error, write protection,
 * initcode-too-large) consumes ALL remaining gas -- unlike REVERT, which returns
 * it. Route every exceptional halt through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = { gas_remaining = 0; frame_status = Exceptional(k) }

function stack_clear()       -> unit = hs_reset()
function stack_frame_enter() -> unit = hs_enter()
function stack_frame_leave() -> unit = hs_leave()

function stack_height() -> int = unsigned(hs_depth())

/* nth-from-top operand (n = 0 is the top); zero if out of range */
function peek(n : int) -> word = hs_peek_w(get_slice_int(64, n, 0))

function push(w : word) -> unit =
  if stack_height() >= STACK_LIMIT then exc_halt(StackOverflow)
  else hs_push_w(w)

function pop() -> word =
  if stack_height() == 0 then { exc_halt(StackUnderflow); WORD_ZERO }
  else { let w = peek(0); hs_pop(); w }

/* overwrite the nth-from-top operand (SWAP) */
function stack_set(n : int, w : word) -> unit = hs_set_w(get_slice_int(64, n, 0), w)

function pop_int() -> int = unsigned(pop())

function is_running() -> bool = match frame_status { Running() => true, _ => false }
