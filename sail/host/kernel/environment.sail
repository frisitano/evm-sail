register k_parent_state_root : hash = sail_zeros(256)
register k_n_headers : item_count = 0
register k_chain_id : chain_identifier = 1
register k_fork : Fork = Amsterdam
register k_blob_schedule : BlobSchedule = struct { target = 14, max = 21, base_fee_update_fraction = 11684671 }

register k_header : BlockHeader =
    struct {
        number = 0,
        timestamp = 0,
        extra_data = EMPTY_SLICE,
        gas_limit = GAS_ZERO,
        gas_used = GAS_ZERO,
        prev_randao = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = 0,
        excess_blob_gas = 0,
        state_root = ZERO_WORD,
        receipts_root = ZERO_WORD,
        logs_bloom = EMPTY_LOGS_BLOOM,
        fee_recipient = ZERO_ADDR,
        parent_hash = ZERO_WORD,
        parent_beacon_block_root = ZERO_WORD,
        slot_number = 0,
    }

register k_tx : TxEnv = struct { origin = ZERO_ADDR, gas_price = ZERO_WORD, blob_hashes = EMPTY_BLOB_HASHES }

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

function k_env(f : EnvField) -> word = {
    match f {
        F_Number => word_of_nat(k_header.number),
        F_Timestamp => word_of_nat(k_header.timestamp),
        F_Coinbase => address_to_word(k_header.fee_recipient),
        F_BaseFee => k_header.base_fee,
        F_ChainId => word_of_nat(k_chain_id),
        F_GasLimit => word_of_gas(k_header.gas_limit),
        F_PrevRandao => k_header.prev_randao,
        F_Origin => address_to_word(k_tx.origin),
        F_GasPrice => k_tx.gas_price,
        F_SlotNumber => word_of_nat(k_header.slot_number),
    }
}

function k_coinbase() -> address = k_header.fee_recipient

function k_blockhash(number_word : word) -> hash = {
    let current = k_header.number;
    let number = unsigned(number_word);
    if number < current then {
        let distance = current - number;
        if distance <= 256 then {
            if k_n_headers < distance then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                let index : ancestor_index = distance - 1;
                ancestor_hash_read(index)
            }
        } else {
            ZERO_WORD
        }
    } else {
        ZERO_WORD
    }
}

function k_blobhash(index_word : word) -> word = {
    let count = k_tx.blob_hashes.count;
    let index = unsigned(index_word);
    if index < count then {
        let offset = 33 * index + 1;
        if offset <= BYTE_QUANTITY_MAX then {
            slice_load_n(k_tx.blob_hashes.bytes, ByteQuantity(offset), WORD_BYTE_LENGTH)
        } else {
            ZERO_WORD
        }
    } else {
        ZERO_WORD
    }
}

function k_create_addr(a : address, nonce : account_nonce) -> address = create_address(a, nonce)

function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)
