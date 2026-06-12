/* ======================================================================== */
/* Halt outcomes and exceptional-halt kinds                            */
/* ======================================================================== */

/* Ordinary frame stops. */
enum HaltKind = {
  HaltStop,         /* STOP                                  */
  HaltReturn,       /* RETURN: success, returndata exposed   */
  HaltRevert,       /* REVERT: failure, state rolled back    */
  HaltSelfDestruct  /* SELFDESTRUCT                          */
}

/* Exceptional halts: consume all remaining gas and revert the frame. */
enum ExceptionKind = {
  StackUnderflow,
  StackOverflow,
  OutOfGas,
  InvalidOpcode,
  InvalidJump,
  StaticViolation,    /* state-changing op inside a STATICCALL frame */
  CallDepthExceeded,
  InsufficientBalance,
  WriteProtection,
  InitCodeTooLarge,   /* EIP-3860 */
  NonceOverflow,
  AddressCollision    /* CREATE into an existing non-empty account   */
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
