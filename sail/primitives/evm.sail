/* ======================================================================== */
/* PRIMITIVES: EVM EXECUTION TYPES                                          */
/*                                                                          */
/* The per-tx environment, the tx validity / frame-result records, and the  */
/* per-frame call Message. Pure data -- no registers, no externs.           */
/* ======================================================================== */

/* Per-transaction environment: ORIGIN/GASPRICE (YP I_o, I_p) + the EIP-4844
 * blob versioned hashes the BLOBHASH opcode reads. */
struct TxEnv = { origin : address, gas_price : word, blob_hashes : list(word) }

struct TxValidity = {
    valid : bool,
    sender : address,
    nonce_before : u64,
    intrinsic_gas : gas,
    blob_fee : int,
    input_len : int,
    gas_price : word, /* effective gas price (EIP-1559, from the caps + base fee) */
    priority_fee : word, /* coinbase tip per gas (effective price minus base fee)    */
}

struct TxFrameResult = {
    success : bool,
    gas_remaining : gas,
}

struct Message = {
    caller       : address,  /* CALLER (I_s)                                  */
    address      : address,  /* ADDRESS / storage owner = self (I_a)          */
    code_address : address,  /* account whose code runs (differs under        */
                                 /* DELEGATECALL/CALLCODE and EIP-7702 delegation) */
    value        : word,     /* CALLVALUE (I_v)                               */
    data_len     : int,      /* CALLDATA length (bytes are C-backed, host_mem) */
    is_static    : bool,     /* inside a STATICCALL frame (EIP-214)           */
    depth        : int,      /* call depth (YP I_e), bounded by DEPTH_LIMIT   */
}

let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDR,
        address = ZERO_ADDR,
        code_address = ZERO_ADDR,
        value = ZERO_WORD,
        data_len = 0,
        is_static = false,
        depth = 0,
    }
