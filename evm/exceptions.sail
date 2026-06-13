/* ======================================================================== */
/* Halt outcomes and exceptional-halt kinds (Yellow Paper sec. 9.4.2)        */
/*                                                                           */
/* A frame ends either by a normal halt (STOP/RETURN/REVERT/SELFDESTRUCT) or */
/* an exceptional halt. An exceptional halt consumes all remaining gas and   */
/* reverts the frame's state changes (YP: the "exceptional halting state").  */
/* REVERT (EIP-140) is the one failure that REFUNDS remaining gas.           */
/* ======================================================================== */

/* Ordinary frame stops. */
enum HaltKind = {
    HaltStop, /* STOP                                  */
    HaltReturn, /* RETURN: success, returndata exposed   */
    HaltRevert, /* REVERT: failure, state rolled back    */
    HaltSelfDestruct, /* SELFDESTRUCT                          */
}

/* Exceptional halts: consume all remaining gas and revert the frame. */
enum ExceptionKind = {
    StackUnderflow,
    StackOverflow,
    OutOfGas,
    InvalidOpcode,
    InvalidJump,
    StaticViolation, /* EIP-214: state-changing op inside a STATICCALL */
    CallDepthExceeded,
    InsufficientBalance,
    WriteProtection, /* EIP-214 write protection                    */
    InitCodeTooLarge, /* EIP-3860 */
    NonceOverflow,
    AddressCollision, /* EIP-684: CREATE into an occupied account     */
}

/* Per-frame execution status. */
union FrameStatus = {
  Running     : unit,
  Halted      : HaltKind,
  Exceptional : ExceptionKind
}

/* The result handed back to a caller when a sub-frame finishes:
 * success flag + returndata + gas left + whether state must roll back. */
struct CallOutcome = {
  co_success    : bool,
  co_returndata : list(byte),
  co_gas_left   : gas,
  co_reverted   : bool
}
