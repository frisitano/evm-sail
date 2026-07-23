
/*md
# EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs.
*/
/*! The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
    plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : BlobHashes }

/*! The values established by successful up-front transaction validation
    (YP §6.2): sender, pre-state nonce, intrinsic gas, blob fee, and the
    EIP-1559 effective gas and priority prices. Invalid transactions throw
    before this value is constructed. */
struct TxValidity = {
    sender : address,
    nonce_before : account_nonce,
    gas_limit : block_gas_limit,
    intrinsic_execution_gas : gas_cost,
    intrinsic_state_gas : gas_cost,
    calldata_floor : gas_cost,
    blob_fee : word,
    /* effective gas price (EIP-1559, from the caps + base fee) */
    gas_price : word,
    /* coinbase tip per gas (effective price minus base fee) */
    priority_fee : word,
}

/*! The result of a transaction's top-level frame: success and remaining
    gas. */
struct TxFrameResult = {
    success : bool,
    execution_gas_remaining : gas,
    state_gas_remaining : gas,
    state_gas_used : state_gas_delta,
    refund : gas_refund,
}

/*! The four CALL-family execution modes. `Call` is an ordinary call;
    `CallCode` combines the caller's storage with the target's code;
    `DelegateCall` additionally inherits the caller and value; and
    `StaticCall` enters a read-only frame. */
enum CallKind = { Call, CallCode, DelegateCall, StaticCall }

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
    state_gas_reservoir : gas,
    /* inside a STATICCALL frame (EIP-214) */
    is_static : bool,
    /* call depth (YP I_e), bounded by DEPTH_LIMIT */
    depth : frame_depth,
}

/* The complete suspended parent frame at a nested execution boundary. The
 * kernel checkpoint owns mutable world state; stack frames remain in the
 * host's stack-of-stacks; returndata intentionally becomes the child output. */
/*! The suspended parent-frame state restored after nested execution. */
struct FrameCheckpoint = {
    state : journal_checkpoint,
    pc : code_pointer,
    gas_remaining : gas,
    state_gas_remaining : gas,
    state_gas_spilled : state_gas_spill,
    refund : gas_refund,
    status : FrameStatus,
    message : Message,
    call_depth : frame_depth,
    code : Code,
    calldata : ByteSlice,
    memory : ByteSlice,
}

/*! The suspended parent information needed after a message call returns. */
struct CallContinuation = {
    checkpoint : FrameCheckpoint,
    return_offset : memory_pointer,
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
    /*! Resume a suspended message-call parent. */
    ResumeCall : CallContinuation,
    /*! Resume a suspended contract-creation parent. */
    ResumeCreate : CreateContinuation
}

/*! The protocol-bounded stack of suspended parent continuations. */
type FrameStack = vector(1024, dec, FrameContinuation)

/*! The zero message; frame registers reset to it between transactions. */
let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDRESS,
        address = ZERO_ADDRESS,
        code_address = ZERO_ADDRESS,
        value = ZERO_WORD,
        state_gas_reservoir = GAS_ZERO,
        is_static = false,
        depth = 0,
    }

let DEFAULT_FRAME_CHECKPOINT : FrameCheckpoint = struct {
        state = 0,
        pc = 0,
        gas_remaining = GAS_ZERO,
        state_gas_remaining = GAS_ZERO,
        state_gas_spilled = STATE_GAS_SPILL_ZERO,
        refund = GAS_REFUND_ZERO,
        status = Running(),
        message = DEFAULT_MESSAGE,
        call_depth = 0,
        code = EMPTY_CODE,
        calldata = EMPTY_SLICE,
        memory = EMPTY_SLICE,
    }

let DEFAULT_FRAME_CONTINUATION : FrameContinuation = ResumeCall(
    struct { checkpoint = DEFAULT_FRAME_CHECKPOINT, return_offset = 0, return_length = 0, new_account_charged = false },
)
