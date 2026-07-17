
/*md
# EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs.
*/
/*! The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
    plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : BlobHashes }

/*! The outcome of up-front transaction validation (YP §6.2): validity,
    recovered sender, pre-state nonce, intrinsic gas, blob fee, and the
    EIP-1559 effective gas and priority prices. */
struct TxValidity = {
    valid : bool,
    sender : address,
    nonce_before : account_nonce,
    intrinsic_gas : gas_cost,
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
    gas_remaining : gas,
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
    state : StateCheckpoint,
    pc : code_pointer,
    gas_remaining : gas,
    refund : gas_refund,
    status : FrameStatus,
    message : Message,
    call_depth : frame_depth,
    code : Code,
    calldata : ByteSlice,
    memory : ByteSlice,
}

/*! The zero message; frame registers reset to it between transactions. */
let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDR,
        address = ZERO_ADDR,
        code_address = ZERO_ADDR,
        value = ZERO_WORD,
        is_static = false,
        depth = 0,
    }
