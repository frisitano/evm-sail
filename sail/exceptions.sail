/* ======================================================================== */
/* HALT KINDS AND THE EXCEPTIONAL HALTING STATE (Yellow Paper sec. 9.4.2)   */
/*                                                                          */
/* How an execution frame can end. A frame either halts normally (one of    */
/* HaltKind: STOP / RETURN / REVERT / SELFDESTRUCT) or hits an exceptional  */
/* halt (one of ExceptionKind). An exceptional halt is the YP "exceptional  */
/* halting state": it consumes ALL remaining gas and reverts the frame's    */
/* state changes. REVERT (EIP-140) is the sole failure that instead REFUNDS */
/* the remaining gas while still rolling state back.                        */
/*                                                                          */
/* FrameStatus is the running/halted/exceptional tag; CallOutcome is what a */
/* finished sub-frame hands back to its caller.                             */
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
