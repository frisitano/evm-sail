/*md
# Exceptions

The failures that interrupt EVM execution or invalidate a block. An
[ExceptionKind][type-ExceptionKind] exceptionally halts the current frame,
consumes its remaining gas, and reverts its state changes. A
[BlockError][type-BlockError] rejects the entire payload and is carried by
the model's single `exception` type.
*/

/*! Exceptional halts (YP §9.4.2): each consumes all remaining gas and
    reverts the frame's state changes. */
enum ExceptionKind = {
    /* an opcode pops more items than the stack holds */
    StackUnderflow,
    /* a push would exceed the 1024-item stack limit */
    StackOverflow,
    /* the operation's cost exceeds the remaining gas */
    OutOfGas,
    /* an unassigned or fork-inactive opcode, or INVALID (0xfe) */
    InvalidOpcode,
    /* a jump target that is not a valid JUMPDEST */
    InvalidJump,
    /* EIP-214: state-changing op inside a STATICCALL */
    StaticViolation,
    /* a call or create beyond depth 1024 */
    CallDepthExceeded,
    /* a value transfer exceeding the sender's balance */
    InsufficientBalance,
    /* EIP-214 write protection */
    WriteProtection,
    /* EIP-3860 */
    InitCodeTooLarge,
    /* a nonce at its maximum cannot be bumped (EIP-2681) */
    NonceOverflow,
    /* EIP-684: CREATE into an occupied account */
    AddressCollision,
}

/*md
## The block-validation failures

Every rule whose violation invalidates a block maps to one variant. The
validator throws `InvalidBlock(reason)` at the point of detection and the
guest catches it once at its boundary. A valid block yields no failure. Only
the pass/fail boolean is emitted, so the reason is diagnostic (and a hook for
future structured output).
*/
/*! The reason a block fails validation; one variant per violated
    block-validity rule. */
enum BlockError = {
    /* chain config: wrong fork / inactive activation */
    InvalidConfig,
    /* witness ancestor headers not contiguous */
    HeaderChainBroken,
    /* a transaction failed to RLP-decode */
    RlpDecode,
    /* a tx signature did not authenticate its sender */
    InvalidSignature,
    /* header.gas_limit is outside the consensus domain */
    InvalidGasLimit,
    /* EIP-7778: a tx exceeds the block's remaining gas */
    GasUsedExceedsLimit,
    /* a tx exceeds the block's remaining blob gas */
    BlobGasLimitExceeded,
    /* an invalid tx or a failed block-end system call */
    ExecutionInvalid,
    /* recomputed cumulative gas != header.gas_used */
    InvalidGasUsed,
    /* recomputed blob gas != header.blob_gas_used */
    InvalidBlobGasUsed,
    /* header.excess_blob_gas != expected */
    InvalidExcessBlobGas,
    /* recomputed post-state root != header.state_root */
    InvalidStateRoot,
    /* recomputed receipts root != header.receipts_root */
    InvalidReceiptsRoot,
    /* recomputed logs bloom != header.logs_bloom */
    InvalidLogsBloom,
    /* recomputed block hash != payload expected hash */
    InvalidBlockHash,
    /* header.parent_hash != authenticated parent */
    InvalidParentHash,
    /* EIP-7928: BAL item count > gas_limit / 2000 */
    BlockAccessListTooLarge,
    /* reconstructed EIP-7928 BAL bytes mismatch */
    InvalidBlockAccessList,
    /* reconstructed EIP-7685 request bytes mismatch */
    InvalidExecutionRequests,
    /* a missing/inconsistent proof node (thrown at use) */
    WitnessDeficient,
}

/*! The single Sail exception type. Block validation throws
    `InvalidBlock(reason)` at the failure point, and `main` catches it
    once — any failure yields an unsuccessful validation. Sail's C backend
    unwinds via `have_exception` return-checks (no `longjmp`), so it is
    zkVM-guest safe. */
union exception = {
    /* the block is invalid, with the failing rule */
    InvalidBlock : BlockError,
}
