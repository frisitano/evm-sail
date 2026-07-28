/*md
# Halting

How an execution frame ends (Yellow Paper §9.4.2). A frame either halts
normally — one of [HaltKind][type-HaltKind]: `STOP`, `RETURN`, `REVERT`, or
`SELFDESTRUCT` — or enters the exceptional halting state, one of
[ExceptionKind][type-ExceptionKind]. An exceptional halt consumes all
remaining gas and reverts the frame's state changes. `REVERT` (EIP-140) is
the sole failure mode that instead refunds the remaining gas while still
rolling state back.

[FrameStatus][type-FrameStatus] is the running/halted/exceptional tag.
`RETURN` and `REVERT` carry the frame output directly; [interpret][] returns
that value.
*/

/*! Ordinary frame stops. Only `RETURN` and `REVERT` produce output bytes. */
union HaltKind = {
  /* STOP: success, empty return data */
  HaltStop : unit,
  /* RETURN: success with output data */
  HaltReturn : OutputSlice,
  /* REVERT: state unwound, output kept (EIP-140) */
  HaltRevert : OutputSlice,
  /* SELFDESTRUCT: success, empty return data */
  HaltSelfDestruct : unit
}

/*! Per-frame execution status: running, halted normally, or exceptionally
    halted. */
union FrameStatus = {
  /* mid-execution */
  Running : unit,
  /* halted normally (YP §9.4.4) */
  Halted : HaltKind,
  /* halted exceptionally: all frame gas consumed, effects void */
  Exceptional : ExceptionKind
}
