
/*md
# EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs.
*/
/*! The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
    plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. */
struct TxEnvFields('blob_limit : Int), transaction_blob_limit_value('blob_limit) = {
    origin : address,
    gas_price : word,
    blob_hashes : BlobHashesFields('blob_limit),
}

/*! A transaction environment packing its fork-selected blob-count limit
    existentially. */
type TxEnv = {
    'blob_limit,
    transaction_blob_limit_value('blob_limit).
    TxEnvFields('blob_limit)
}

/*! Constructs a transaction environment without hiding the decoder-proven
    blob-count limit before it reaches the environment register. */
val tx_env : forall 'blob_limit, transaction_blob_limit_value('blob_limit). (
        address,
        word,
        BlobHashesFields('blob_limit),
    ) -> TxEnvFields('blob_limit)
function tx_env(origin, gas_price, blob_hashes) =
    struct { origin = origin, gas_price = gas_price, blob_hashes = blob_hashes }

/*! The header/profile-bounded gas allowance attached to one decoded
    transaction before stateful validation. The total allowance funds both
    gas dimensions; the regular allowance is the portion available to
    ordinary EVM execution. */
struct TransactionGasAllowanceFields(
    'total : Int,
    'regular : Int,
), 0 <= 'regular & 'regular <= 'total & 'total <= block_gas_limit_bound = {
    total : int('total),
    regular : int('regular),
}

/*! A transaction gas allowance with existentially hidden total and regular
    budgets. */
type TransactionGasAllowance = {
    'total 'regular,
    0 <= 'regular & 'regular <= 'total & 'total <= block_gas_limit_bound.
    TransactionGasAllowanceFields('total, 'regular)
}

/*! The post-intrinsic gas state admitted for a transaction. Keeping the
    admitted limit, the regular-gas ceiling, both intrinsic charges, and the
    two live reservoirs in one dependent value preserves the conservation
    equation established by transaction validation. */
type transaction_initial_gas_relation(
    'total : Int,
    'regular : Int,
    'intrinsic_execution : Int,
    'intrinsic_state : Int,
    'calldata_floor : Int,
    'execution : Int,
    'state : Int,
) -> Bool =
       0 <= 'intrinsic_execution
    &  0 <= 'intrinsic_state
    &  0 <= 'calldata_floor
    &  'intrinsic_execution <= 'regular
    &  'calldata_floor <= 'regular
    &  'regular <= 'total
    &  'total <= block_gas_limit_bound
    &  0 <= 'execution
    &  0 <= 'state
    &  'execution + 'state + 'intrinsic_execution + 'intrinsic_state == 'total
    &  'execution <= 'regular - 'intrinsic_execution

/*! The concrete post-intrinsic gas record whose indices retain the
    transaction conservation relation. */
struct TransactionInitialGasFields(
    'total : Int,
    'regular : Int,
    'intrinsic_execution : Int,
    'intrinsic_state : Int,
    'calldata_floor : Int,
    'execution : Int,
    'state : Int,
), transaction_initial_gas_relation(
    'total,
    'regular,
    'intrinsic_execution,
    'intrinsic_state,
    'calldata_floor,
    'execution,
    'state,
) = {
    admitted_limit : int('total),
    regular_limit : int('regular),
    intrinsic_execution : int('intrinsic_execution),
    intrinsic_state : int('intrinsic_state),
    calldata_floor : int('calldata_floor),
    execution_remaining : int('execution),
    state_remaining : int('state),
}

/*! The initial gas state with every conserved quantity hidden
    existentially. */
type TransactionInitialGas = {
    'total
    'regular
    'intrinsic_execution
    'intrinsic_state
    'calldata_floor
    'execution
    'state,
    transaction_initial_gas_relation(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ).
    TransactionInitialGasFields(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
}

/*! The initial gas state for one concrete admitted transaction limit. This
    view hides the internal split while retaining the limit that the block
    transaction loop has already checked against its remaining budgets. */
type TransactionInitialGasFor('total : Int) = {
    'regular
    'intrinsic_execution
    'intrinsic_state
    'calldata_floor
    'execution
    'state,
    transaction_initial_gas_relation(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ).
    TransactionInitialGasFields(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
}

/*! The initial gas state retaining both the complete and regular execution
    allowances checked by the block loop. */
type TransactionInitialGasForLimits(
    'total : Int,
    'regular : Int,
) = {
    'intrinsic_execution
    'intrinsic_state
    'calldata_floor
    'execution
    'state,
    transaction_initial_gas_relation(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ).
    TransactionInitialGasFields(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
}

/*! Constructs the dependent initial-gas record once transaction validation
    has established its conservation relation. */
val transaction_initial_gas_fields : forall 'total 'regular 'intrinsic_execution 'intrinsic_state 'calldata_floor 'execution 'state,
                                       transaction_initial_gas_relation(
                                           'total,
                                           'regular,
                                           'intrinsic_execution,
                                           'intrinsic_state,
                                           'calldata_floor,
                                           'execution,
                                           'state,
                                       ). (
        int('total),
        int('regular),
        int('intrinsic_execution),
        int('intrinsic_state),
        int('calldata_floor),
        int('execution),
        int('state),
    ) -> TransactionInitialGasFields(
        'total,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
function transaction_initial_gas_fields(
    total,
    regular,
    intrinsic_execution,
    intrinsic_state,
    calldata_floor,
    execution,
    state,
) =
    struct {
        admitted_limit = total,
        regular_limit = regular,
        intrinsic_execution = intrinsic_execution,
        intrinsic_state = intrinsic_state,
        calldata_floor = calldata_floor,
        execution_remaining = execution,
        state_remaining = state,
    }

/*! The values established by successful up-front transaction validation
    (YP §6.2): sender, pre-state nonce, intrinsic gas, blob fee, and the
    EIP-1559 effective gas and priority prices. Invalid transactions terminate
    before this value is constructed. */
struct TxValidityFields(
    'limit : Int,
    'regular : Int,
    'intrinsic_execution : Int,
    'intrinsic_state : Int,
    'calldata_floor : Int,
    'execution : Int,
    'state : Int,
), transaction_initial_gas_relation(
    'limit,
    'regular,
    'intrinsic_execution,
    'intrinsic_state,
    'calldata_floor,
    'execution,
    'state,
) = {
    sender : address,
    nonce_before : account_nonce,
    gas : TransactionInitialGasFields(
        'limit,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ),
    blob_fee : word,
    /* effective gas price (EIP-1559, from the caps + base fee) */
    gas_price : word,
    /* coinbase tip per gas (effective price minus base fee) */
    priority_fee : word,
}

/*! Constructs the validity record once up-front transaction validation has
    established the initial-gas conservation relation. */
val tx_validity_fields : forall 'limit 'regular 'intrinsic_execution 'intrinsic_state 'calldata_floor 'execution 'state,
                           transaction_initial_gas_relation(
                               'limit,
                               'regular,
                               'intrinsic_execution,
                               'intrinsic_state,
                               'calldata_floor,
                               'execution,
                               'state,
                           ). (
        address,
        account_nonce,
        TransactionInitialGasFields(
            'limit,
            'regular,
            'intrinsic_execution,
            'intrinsic_state,
            'calldata_floor,
            'execution,
            'state,
        ),
        word,
        word,
        word,
    ) -> TxValidityFields('limit, 'regular, 'intrinsic_execution, 'intrinsic_state, 'calldata_floor, 'execution, 'state)
function tx_validity_fields(sender, nonce_before, gas, blob_fee, gas_price, priority_fee) =
    struct {
        sender = sender,
        nonce_before = nonce_before,
        gas = gas,
        blob_fee = blob_fee,
        gas_price = gas_price,
        priority_fee = priority_fee,
    }

/*! A validity result retaining the transaction's admitted total and regular
    gas limits while hiding the internal initial split. */
type TxValidityForLimits(
    'limit : Int,
    'regular : Int,
) = {
    'intrinsic_execution
    'intrinsic_state
    'calldata_floor
    'execution
    'state,
    transaction_initial_gas_relation(
        'limit,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ).
    TxValidityFields(
        'limit,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
}

/*! A validity result whose limit is not needed by the consumer. */
type TxValidity = {
    'limit
    'regular
    'intrinsic_execution
    'intrinsic_state
    'calldata_floor
    'execution
    'state,
    transaction_initial_gas_relation(
        'limit,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    ).
    TxValidityFields(
        'limit,
        'regular,
        'intrinsic_execution,
        'intrinsic_state,
        'calldata_floor,
        'execution,
        'state,
    )
}

/*! The transaction gas conserved at the top-level frame boundary. The
    snapshot retains both admitted budgets and the exact split consumed by
    ordinary execution and state access. Settlement can therefore apply
    refunds and the calldata floor without reconstructing or revalidating
    either contribution. */
type tx_frame_gas_snapshot_relation(
    'limit : Int,
    'regular : Int,
    'calldata_floor : Int,
    'remaining : Int,
    'state_used : Int,
) -> Bool =
       0 <= 'regular
    &  'regular <= 'limit
    &  'limit <= block_gas_limit_bound
    &  0 <= 'calldata_floor
    &  'calldata_floor <= 'regular
    &  0 <= 'remaining
    &  0 <= 'state_used
    &  'state_used <= 'limit
    &  'remaining + 'state_used <= 'limit
    &  'limit - 'remaining - 'state_used <= 'regular

/*! The concrete top-level frame gas snapshot whose indices retain the
    transaction admission and conservation proof. */
struct TxFrameGasSnapshotFields(
    'limit : Int,
    'regular : Int,
    'calldata_floor : Int,
    'remaining : Int,
    'state_used : Int,
), tx_frame_gas_snapshot_relation(
    'limit,
    'regular,
    'calldata_floor,
    'remaining,
    'state_used,
) = {
    admitted_limit : int('limit),
    regular_limit : int('regular),
    calldata_floor : int('calldata_floor),
    remaining : int('remaining),
    state_used : int('state_used),
}

/*! Constructs a frame snapshot with each conserved gas quantity tied to its
    own index. Keeping this boundary explicit also lets proof backends retain
    the five distinct existential witnesses when the snapshot is packed. */
val tx_frame_gas_snapshot_fields : forall 'limit 'regular 'calldata_floor 'remaining 'state_used,
                                     tx_frame_gas_snapshot_relation(
                                         'limit,
                                         'regular,
                                         'calldata_floor,
                                         'remaining,
                                         'state_used,
                                     ). (
        int('limit),
        int('regular),
        int('calldata_floor),
        int('remaining),
        int('state_used),
    ) -> TxFrameGasSnapshotFields('limit, 'regular, 'calldata_floor, 'remaining, 'state_used)
function tx_frame_gas_snapshot_fields(limit, regular, calldata_floor, remaining, state_used) =
    struct {
        admitted_limit = limit,
        regular_limit = regular,
        calldata_floor = calldata_floor,
        remaining = remaining,
        state_used = state_used,
    }

/*! A top-level frame gas snapshot with existentially hidden gas totals. */
type TxFrameGasSnapshot = {
    'limit 'regular 'calldata_floor 'remaining 'state_used,
    tx_frame_gas_snapshot_relation(
        'limit,
        'regular,
        'calldata_floor,
        'remaining,
        'state_used,
    ).
    TxFrameGasSnapshotFields(
        'limit,
        'regular,
        'calldata_floor,
        'remaining,
        'state_used,
    )
}

/*! A completed top-level frame tied to both admitted transaction limits. */
type TxFrameGasSnapshotForLimits(
    'limit : Int,
    'regular : Int,
) = {
    'calldata_floor 'remaining 'state_used,
    tx_frame_gas_snapshot_relation(
        'limit,
        'regular,
        'calldata_floor,
        'remaining,
        'state_used,
    ).
    TxFrameGasSnapshotFields(
        'limit,
        'regular,
        'calldata_floor,
        'remaining,
        'state_used,
    )
}

/*! The result of a transaction's top-level frame: success, the bounded gas
    snapshot consumed by settlement, and refunds. */
struct TxFrameResultFields(
    'limit : Int,
    'regular : Int,
), 0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound = {
    success : bool,
    gas : TxFrameGasSnapshotForLimits('limit, 'regular),
    refund : gas_refund,
}

/*! A completed top-level frame with existentially hidden admitted limits. */
type TxFrameResult = {
    'limit 'regular,
    0 <= 'regular & 'regular <= 'limit & 'limit <= block_gas_limit_bound.
    TxFrameResultFields('limit, 'regular)
}

/*! The four CALL-family execution modes. `Call` is an ordinary call;
    `CallCode` combines the caller's storage with the target's code;
    `DelegateCall` additionally inherits the caller and value; and
    `StaticCall` enters a read-only frame. */
enum CallKind = { Call, CallCode, DelegateCall, StaticCall }

/*! The two contract-creation address schemes. `CreateByNonce` is ordinary
    `CREATE`; `CreateBySalt` is EIP-1014 `CREATE2`. Keeping this as a closed
    semantic tag prevents callers from encoding an execution mode in an
    otherwise unexplained boolean. */
enum CreateKind = { CreateByNonce, CreateBySalt }

/*! The per-frame call message (YP §8, the I tuple): caller, executing
    address, code owner, value, calldata length, static flag, and call
    depth. */
struct Message = {
    /* CALLER (I_s) */
    caller : address,
    /* account whose code runs (differs under DELEGATECALL/CALLCODE and
       EIP-7702 delegation) */
    code_address : address,
    /* ADDRESS / storage owner = self (I_a) */
    address : address,
    /* CALLVALUE (I_v) */
    value : word,
    /* State-gas reservoir available when the frame was entered. */
    state_gas_reservoir : state_gas,
    /* inside a STATICCALL frame (EIP-214) */
    is_static : bool,
    /* call depth (YP I_e), bounded by call_depth_limit */
    depth : frame_depth,
}

/*! Lightweight result of one opcode handler. */
union OpcodeOutcome = {
    /*! Continue executing the active frame. */
    Continue : unit,
    /*! Stop the active opcode path with the given exceptional halt. */
    Failed : ExceptionKind,
}

/* The complete suspended parent frame at a nested execution boundary. The
 * kernel checkpoint owns mutable world state; stack frames remain in the
 * host's stack-of-stacks; returndata intentionally becomes the child output. */
/*! The suspended parent-frame state restored after nested execution. */
struct FrameCheckpoint = {
    pc : code_pointer,
    gas_remaining : gas,
    stack_top : StackPointer,
    state_gas_remaining : state_gas,
    state_gas_spilled : state_gas_spill,
    refund : gas_refund,
    status : FrameStatus,
    message : Message,
    code : Code,
    calldata : CalldataSlice,
    memory_height : memory_height,
}

/*! The suspended parent information needed after a message call returns. */
struct CallContinuation = {
    checkpoint : FrameCheckpoint,
    return_offset : memory_base,
    return_length : memory_length,
    /* Amsterdam NEW_ACCOUNT state gas paid for the child target. */
    new_account_charged : bool,
}

/*! The suspended parent information needed after initcode returns. */
struct CreateContinuation = {
    checkpoint : FrameCheckpoint,
    address : address,
    /* Amsterdam NEW_ACCOUNT state gas paid for the created address. */
    new_account_charged : bool,
}

/*! The pending action performed when a child frame finishes. */
union FrameContinuation = {
    /*! No suspended parent remains; the completed frame was top-level. */
    Empty : unit,
    /*! Resume a suspended message-call parent. */
    ResumeCall : CallContinuation,
    /*! Resume a suspended contract-creation parent. */
    ResumeCreate : CreateContinuation
}

/*! The zero message; frame registers reset to it between transactions. */
let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDRESS,
        address = ZERO_ADDRESS,
        code_address = ZERO_ADDRESS,
        value = ZERO_WORD,
        state_gas_reservoir = STATE_GAS_ZERO,
        is_static = false,
        depth = 0,
    }
