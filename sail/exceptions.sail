/*md
# Exceptions

The failures that interrupt EVM execution or terminate payload validation. An
[ExceptionKind][type-ExceptionKind] exceptionally halts the current frame,
consumes its remaining gas, and reverts its state changes. A
[FatalError][type-FatalError] rejects the entire payload and terminates the
validator immediately.
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

Every terminal validator failure maps to one variant. The validator calls
`fatal_error(reason)` at the point of detection. That operation does not
return, so fatal validation failures never enter Sail's recoverable control
flow. A valid block reaches the normal success output.
*/
/*! The reason a block fails validation; one variant per violated
    block-validity rule. */
enum FatalError = {
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
    /* an exact protocol integer exceeds its bounded execution representation */
    NumericOverflow,
}

/*! Terminal validation failure. C binds this operation to the guest failure
    boundary; semantic extraction retains the explicit non-returning body. */
val fatal_error = impure { c: "fatal_error" } :
  FatalError -> unit

function fatal_error(_reason) = exit(())

/*! Stable identifier for the validation stage that raised a block error.
    This diagnostic metadata is a bounded integer, not a protocol bitvector. */
type validation_stage = range(0, 255)
