/* ======================================================================== */
/* PRIMITIVES: EVM EXECUTION TYPES                                          */
/*                                                                          */
/* The per-tx environment, the tx validity / frame-result records, and the  */
/* per-frame call Message. Pure data -- no registers, no externs.           */
/* ======================================================================== */

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : BlobHashes }

struct TxValidity = {
    valid : bool,
    sender : address,
    nonce_before : account_nonce,
    intrinsic_gas : gas_cost,
    calldata_floor : gas_cost,
    blob_fee : word,
    gas_price : word, /* effective gas price (EIP-1559, from the caps + base fee) */
    priority_fee : word, /* coinbase tip per gas (effective price minus base fee)    */
}

struct TxFrameResult = {
    success : bool,
    gas_remaining : gas,
    refund : gas_refund,
}

enum CallKind = { Call, CallCode, DelegateCall, StaticCall }

struct Message = {
    caller       : address,  /* CALLER (I_s)                                  */
    code_address : address,  /* account whose code runs (differs under        */
                                 /* DELEGATECALL/CALLCODE and EIP-7702 delegation) */
    address      : address,  /* ADDRESS / storage owner = self (I_a)          */
    value        : word,     /* CALLVALUE (I_v)                               */
    is_static    : bool,     /* inside a STATICCALL frame (EIP-214)           */
    depth        : frame_depth, /* call depth (YP I_e), bounded by DEPTH_LIMIT */
}

/* The complete suspended parent frame at a nested execution boundary. The
 * kernel checkpoint owns mutable world state; stack frames remain in the
 * host's stack-of-stacks; returndata intentionally becomes the child output. */
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

let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDR,
        address = ZERO_ADDR,
        code_address = ZERO_ADDR,
        value = ZERO_WORD,
        is_static = false,
        depth = 0,
    }
