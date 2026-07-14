/*md
# Kernel environment

The block- and transaction-level environment registers, and the
environment reads the `evm` layer serves opcodes from.
*/

/*! The EIP-3529 gas-refund counter — the model's one signed quantity. */
register k_refund : i64 = 0
/*! The authenticated parent state root, the anchor of every stateless
    read. */
register k_parent_state_root : hash = sail_zeros(256)
/*! The number of witnessed ancestor headers ([k_blockhash][] range). */
register k_n_headers : int = 0
/*! The chain id (`CHAINID`, EIP-155 domains). */
register k_chain_id : int = 1
/*! The active fork; every fork-gated rule compares against it. */
register k_fork : Fork = Amsterdam
/*! The active blob schedule (EIP-4844/EIP-7691). */
register k_blob_schedule : BlobSchedule = struct { target = 14, max = 21, base_fee_update_fraction = 11684671 }

/*! The executing payload's header. */
register k_header : BlockHeader =
    struct {
        number = 0,
        timestamp = 0,
        extra_data = EMPTY_SLICE,
        gas_limit = 0,
        gas_used = 0,
        prev_randao = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = sail_zeros(64),
        excess_blob_gas = sail_zeros(64),
        state_root = ZERO_WORD,
        receipts_root = ZERO_WORD,
        logs_bloom = EMPTY_LOGS_BLOOM,
        fee_recipient = ZERO_ADDR,
        parent_hash = ZERO_WORD,
        parent_beacon_block_root = ZERO_WORD,
        slot_number = sail_zeros(64),
    }

/*! The per-transaction environment (`ORIGIN`, `GASPRICE`,
    `BLOBHASH`). */
register k_tx : TxEnv = struct { origin = ZERO_ADDR, gas_price = ZERO_WORD, blob_hashes = EMPTY_BLOB_HASHES }

/*! The environment fields opcodes read through [k_env][]. */
enum EnvField = {
    F_Number,
    F_Timestamp,
    F_Coinbase,
    F_BaseFee,
    F_ChainId,
    F_GasLimit,
    F_PrevRandao,
    F_Origin,
    F_GasPrice,
    F_SlotNumber,
}

/*! An environment field as the word its opcode pushes. */
function k_env(f : EnvField) -> word = {
    match f {
        F_Number => word_of_int(k_header.number),
        F_Timestamp => word_of_int(k_header.timestamp),
        F_Coinbase => address_to_word(k_header.fee_recipient),
        F_BaseFee => k_header.base_fee,
        F_ChainId => word_of_int(k_chain_id),
        F_GasLimit => word_of_int(k_header.gas_limit),
        F_PrevRandao => k_header.prev_randao,
        F_Origin => address_to_word(k_tx.origin),
        F_GasPrice => k_tx.gas_price,
        F_SlotNumber => sail_zero_extend(k_header.slot_number, 256),
    }
}

/*! The block's fee recipient (`COINBASE`). */
function k_coinbase() -> address = k_header.fee_recipient

/*! `BLOCKHASH`: the hash of ancestor `number`, zero outside the 256-block
    window; an in-window ancestor missing from the witness is a deficient
    witness. */
function k_blockhash(number : int) -> hash = {
    let offset = k_header.number - number;
    if offset < 1 | offset > 256 then {
        word_of_int(0)
    } else {
        if offset > k_n_headers then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            ancestor_hash_read(get_slice_int(8, offset - 1, 0))
        }
    }
}

/*! `BLOBHASH` (EIP-4844): the `i`-th versioned hash, zero out of
    range. */
function k_blobhash(i : int) -> word = {
    if i < 0 | i >= k_tx.blob_hashes.count then {
        ZERO_WORD
    } else {
        slice_load_n(k_tx.blob_hashes.bytes, i * 33 + 1, 32)
    }
}

/*! The `CREATE` address rule, in kernel form. */
function k_create_addr(a : address, nonce : u64) -> address = create_address(a, nonce)

/*! The `CREATE2` address rule, in kernel form. */
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)
