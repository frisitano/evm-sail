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
  HaltStop         : unit,
  HaltReturn       : ByteSlice,
  HaltRevert       : ByteSlice,
  HaltSelfDestruct : unit
}

/*! Exceptional halts (YP §9.4.2): each consumes all remaining gas and
    reverts the frame's state changes. */
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

/*! Per-frame execution status: running, halted normally, or exceptionally
    halted. */
union FrameStatus = {
  Running     : unit,
  Halted      : HaltKind,
  Exceptional : ExceptionKind
}

/*md
## Block validation failures

The `InvalidBlock` hierarchy, as data. Mirrors EELS `InvalidBlock`
(execution-specs `amsterdam/fork.py`, `state_transition`): every
`raise InvalidBlock` site maps to one variant. Failures are modeled as a
typed enum, not a thrown exception — the same data-flow discipline as
[ExceptionKind][type-ExceptionKind] — keeping validation total. The guest
reports the first failure it finds ([block_validation_error][]); a valid
block yields `None`. Only the pass/fail boolean is emitted, so the reason is
diagnostic (and a hook for future structured output).
*/

/*! The reason a block fails validation. Each variant corresponds to an
    EELS `raise InvalidBlock` site in `state_transition`. */
enum BlockError = {
    InvalidConfig, /* chain config: wrong fork / inactive activation      */
    HeaderChainBroken, /* witness ancestor headers not contiguous             */
    RlpDecode, /* a transaction failed to RLP-decode                  */
    InvalidSignature, /* a tx signature did not authenticate its sender      */
    GasUsedExceedsLimit, /* EIP-7778: a tx exceeds the block's remaining gas    */
    BlobGasLimitExceeded, /* a tx exceeds the block's remaining blob gas         */
    ExecutionInvalid, /* an invalid tx or a failed block-end system call     */
    InvalidGasUsed, /* recomputed cumulative gas != header.gas_used        */
    InvalidBlobGasUsed, /* recomputed blob gas != header.blob_gas_used         */
    InvalidExcessBlobGas, /* header.excess_blob_gas != expected                  */
    InvalidStateRoot, /* recomputed post-state root != header.state_root     */
    InvalidReceiptsRoot, /* recomputed receipts root != header.receipts_root    */
    InvalidLogsBloom, /* recomputed logs bloom != header.logs_bloom          */
    InvalidBlockHash, /* recomputed block hash != payload expected hash     */
    InvalidParentHash, /* header.parent_hash != authenticated parent          */
    BlockAccessListTooLarge, /* EIP-7928: BAL item count > gas_limit / 2000         */
    InvalidBlockAccessList, /* reconstructed EIP-7928 BAL bytes mismatch            */
    InvalidExecutionRequests, /* reconstructed EIP-7685 request bytes mismatch       */
    WitnessDeficient, /* a missing/inconsistent proof node (thrown at use)   */
}

/*! The single Sail exception type. Block validation throws
    `InvalidBlock(reason)` at the failure point (mirroring EELS
    `raise InvalidBlock`), and `main` catches it once — any failure becomes
    `successful_validation = false`. This is exactly the EELS
    `verify_stateless_new_payload` try/except structure. Sail's C backend
    unwinds via `have_exception` return-checks (no `longjmp`), so it is
    zkVM-guest safe. */
union exception = {
    InvalidBlock : BlockError,
}
