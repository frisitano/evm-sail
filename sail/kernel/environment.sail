/*md
# The execution environment

The block- and transaction-level environment registers, and the
environment reads the `evm` layer serves opcodes from.
*/

/*! The authenticated parent state root, the anchor of every stateless
    read. */
register k_parent_state_root : hash = ZERO_HASH
/*! The number of witnessed ancestor headers ([k_blockhash][] range). */
register k_n_headers : ancestor_hash_count = 0
/*! The chain id (`CHAINID`, EIP-155 domains). */
register k_chain_id : chain_identifier = 1
/*! The active protocol policy and all gas limits derived from the executing
    header, selected together while decoding the stateless input. */
register k_execution_profile : ExecutionProfile = DEFAULT_EXECUTION_PROFILE

/*! The executing payload's header. */
register k_header : BlockHeader =
    struct {
        number = 0,
        timestamp = 0,
        extra_data = EMPTY_STATELESS_INPUT_SLICE,
        gas_limit = 0,
        gas_used = 0,
        prev_randao = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = 0,
        excess_blob_gas = 0,
        state_root = ZERO_HASH,
        receipts_root = ZERO_HASH,
        logs_bloom = stateless_input_slice(0, 256),
        fee_recipient = ZERO_ADDRESS,
        parent_hash = ZERO_HASH,
        parent_beacon_block_root = ZERO_HASH,
        slot_number = 0,
    }

/*! The per-transaction environment (`ORIGIN`, `GASPRICE`,
    `BLOBHASH`). */
register k_tx : TxEnv =
    struct { origin = ZERO_ADDRESS, gas_price = ZERO_WORD, blob_hashes = EMPTY_BLOB_HASHES } :
        TxEnvFields(blob_schedule_inactive_count)
/*! The current execution epoch: zero for pre-execution effects, transaction
    index plus one during transaction execution, and transaction count plus
    one for post-execution effects. EIP-2929 warmth and EIP-7928 BAL changes
    share this transaction-scoped identity. */
register k_current_transaction_epoch : block_access_index = 0

/*! The closed environment-projection algebra interpreted by [k_env][]. Each
    opcode supplies one constant member, so the shared projection remains
    explicit and first-order for executable and proof backends. */
enum EnvField = {
    /* block number (`NUMBER`) */
    F_Number,
    /* block timestamp (`TIMESTAMP`) */
    F_Timestamp,
    /* beneficiary address (`COINBASE`) */
    F_Coinbase,
    /* block base fee (`BASEFEE`, EIP-3198) */
    F_BaseFee,
    /* chain identifier (`CHAINID`, EIP-1344) */
    F_ChainId,
    /* block gas limit (`GASLIMIT`) */
    F_GasLimit,
    /* randomness beacon (`PREVRANDAO`, EIP-4399) */
    F_PrevRandao,
    /* transaction sender (`ORIGIN`) */
    F_Origin,
    /* effective gas price (`GASPRICE`) */
    F_GasPrice,
    /* consensus slot number (`SLOTNUM`, EIP-7843) */
    F_SlotNumber,
}

/*! An environment field as the word its opcode pushes. */
function k_env(f : EnvField) -> word = {
    let active_tx = k_tx;
    match f {
        F_Number => {
            let number = word_of_block_number(k_header.number);
            u256(number)
        },
        F_Timestamp => {
            let timestamp = word_of_block_timestamp(k_header.timestamp);
            u256(timestamp)
        },
        F_Coinbase => address_to_word(k_header.fee_recipient),
        F_BaseFee => k_header.base_fee,
        F_ChainId => {
            let chain_id = word_of_chain_identifier(k_chain_id);
            u256(chain_id)
        },
        F_GasLimit => u256(k_header.gas_limit),
        F_PrevRandao => k_header.prev_randao,
        F_Origin => address_to_word(active_tx.origin),
        F_GasPrice => active_tx.gas_price,
        F_SlotNumber => {
            let slot_number = word_of_slot_number(k_header.slot_number);
            u256(slot_number)
        },
    }
}

/*! The block's fee recipient (`COINBASE`). */
function k_coinbase() -> address = k_header.fee_recipient

/*! Subtracts ordered EVM words without modular wraparound. */
val blockhash_word_distance : forall 'number 'current, 0 <= 'number & 'number < 'current & 'current < 2 ^ 256. (
        int('current),
        int('number),
    ) -> range(1, 2 ^ 256 - 1)
function blockhash_word_distance(current, number) = current - number

/*! `BLOCKHASH`: the hash of ancestor `number`, zero outside the 256-block
    window; an in-window ancestor missing from the witness is a deficient
    witness. */
function k_blockhash(number_word : word) -> hash = {
    let current = k_header.number;
    let current_number = word_of_block_number(current);
    let current_word : word = u256(current_number);
    if number_word < current_word then {
        let (distance_word as 'distance) = blockhash_word_distance(current_word, number_word);
        if distance_word <= 256 then {
            let distance : range(1, 256) = distance_word;
            if k_n_headers < distance then {
                fatal_error(WitnessDeficient)
            } else {
                let index : ancestor_index = distance - 1;
                ancestor_hash_read(index)
            }
        } else {
            ZERO_HASH
        }
    } else {
        ZERO_HASH
    }
}

/*! `BLOBHASH` (EIP-4844): the `i`-th versioned hash, zero out of
    range. */
function k_blobhash(index_word : word) -> word = {
    let active_tx = k_tx;
    let count = active_tx.blob_hashes.count;
    if index_word < count then {
        let index = index_word;
        let offset : source_pointer = 33 * index + 1;
        slice_load_n(active_tx.blob_hashes.bytes, offset, WORD_BYTE_LENGTH)
    } else {
        ZERO_WORD
    }
}

/*! The `CREATE` address rule, in kernel form. */
function k_create_addr(a : address, nonce : account_nonce) -> address = create_address(a, nonce)

/*! The `CREATE2` address rule, in kernel form. */
function k_create2_addr(a : address, salt : word, inithash : hash) -> address =
    create2_address(a, salt, inithash)
