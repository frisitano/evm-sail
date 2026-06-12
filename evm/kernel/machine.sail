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
register stack         : list(word) = [||]
register frame_status  : FrameStatus = Running()
register code          : list(byte) = [||]
register returndata    : list(byte) = [||]
register frame_output  : list(byte) = [||]
register message       : Message    = DEFAULT_MESSAGE
register call_depth    : int        = 0

/* ----------------------- code length + sequential cursor ----------------- */
/* `code` is a list(byte), so list_len(code) is O(n) and list_nth(code, pc) is   */
/* O(pc). A long loop (run-until-OOG, deep reentrancy) therefore fetches in      */
/* O(n^2). We cache the code length, and keep a sequential cursor: a register    */
/* holding the suffix of `code` starting at index `code_cur_pc`. Reading byte i  */
/* advances the cursor forward (O(1) amortized for the sequential PC walk and    */
/* PUSH-operand reads, which only ever move forward); a backward read (a JUMP to  */
/* an earlier dest, or a re-entry at pc=0) re-seeks from `code` once. ALL `code`  */
/* assignments must go through set_code so the length + cursor stay coherent.    */
register code_len      : int        = 0
register code_cur      : list(byte) = [||]   /* suffix of code at index code_cur_pc */
register code_cur_pc   : int        = 0

/* Re-seek the cursor to the head of `code` (used on a fresh frame / backward
 * read). O(n) once, amortized away by the sequential reads that follow. */
function code_reseek() -> unit = { code_cur = code; code_cur_pc = 0 }

/* The single setter for the `code` register: keep code_len + cursor coherent. */
function set_code(c : list(byte)) -> unit = {
  code = c;
  code_len = list_len(c);
  code_cur = c;
  code_cur_pc = 0
}

/* code[i], zero past the end. O(1) amortized for a forward (sequential) walk;
 * O(i) on a backward seek. Drives the cursor to index i, then reads its head. */
function code_byte(i : int) -> byte = {
  if i < code_cur_pc then code_reseek() else ();
  /* advance the cursor head forward to index i */
  while code_cur_pc < i do {
    match code_cur { _ :: t => code_cur = t, [||] => () };
    code_cur_pc = code_cur_pc + 1
  };
  match code_cur { h :: _ => h, [||] => 0x00 }
}

/* ------------------------------ the stack ------------------------------- */

let STACK_LIMIT : int = 1024

/* An exceptional halt (OOG, invalid opcode/jump, stack error, write protection,
 * initcode-too-large) consumes ALL remaining gas -- unlike REVERT, which returns
 * it. Route every exceptional halt through here so the gas is always zeroed. */
function exc_halt(k : ExceptionKind) -> unit = { gas_remaining = 0; frame_status = Exceptional(k) }

function stack_height() -> int = list_len(stack)

function push(w : word) -> unit =
  if stack_height() >= STACK_LIMIT then exc_halt(StackOverflow)
  else stack = w :: stack

function pop() -> word =
  match stack {
    [||]      => { exc_halt(StackUnderflow); word_of_int(0) },
    w :: rest => { stack = rest; w }
  }

function peek(n : int) -> word = list_nth(stack, n, word_of_int(0))
function pop_int() -> int = unsigned(pop())

function is_running() -> bool = match frame_status { Running() => true, _ => false }

/* --------------------- saved frame (nested call) ------------------------ */
/* Saves USER-SPACE registers only. The world is snapshotted in the kernel.   */
struct SavedFrame = {
  sf_pc : int, sf_gas : gas, sf_stack : list(word), sf_code : list(byte),
  sf_returndata : list(byte), sf_frame_output : list(byte), sf_message : Message,
  sf_status : FrameStatus, sf_memory : list(MemByte), sf_memory_size : int,
  sf_ret_off : int, sf_ret_len : int
}
register frames : list(SavedFrame) = [||]
