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
    nonce_before : u64,
    intrinsic_gas : gas,
    blob_fee : int,
    gas_price : word, /* effective gas price (EIP-1559, from the caps + base fee) */
    priority_fee : word, /* coinbase tip per gas (effective price minus base fee)    */
}

/*! The result of a transaction's top-level frame: success and remaining
    gas. */
struct TxFrameResult = {
    success : bool,
    gas_remaining : gas,
}

/*! The per-frame call message (YP §8, the I tuple): caller, executing
    address, code owner, value, calldata length, static flag, and call
    depth. */
struct Message = {
    caller       : address,  /* CALLER (I_s)                                  */
    address      : address,  /* ADDRESS / storage owner = self (I_a)          */
    code_address : address,  /* account whose code runs (differs under        */
                                 /* DELEGATECALL/CALLCODE and EIP-7702 delegation) */
    value        : word,     /* CALLVALUE (I_v)                               */
    data_len     : int,      /* CALLDATA length (calldata is a ByteSlice)       */
    is_static    : bool,     /* inside a STATICCALL frame (EIP-214)           */
    depth        : int,      /* call depth (YP I_e), bounded by DEPTH_LIMIT   */
}

/*! The zero message; frame registers reset to it between transactions. */
let DEFAULT_MESSAGE : Message = struct {
        caller = ZERO_ADDR,
        address = ZERO_ADDR,
        code_address = ZERO_ADDR,
        value = ZERO_WORD,
        data_len = 0,
        is_static = false,
        depth = 0,
    }
