/*md
# Payload commitments

The header-level commitments recomputed from the payload: the
transactions and withdrawals tries, the EIP-7685 requests hash, and the
block header hash itself.
*/

/*! The RLP of one withdrawal (EIP-4895), assembled in the scratch
    arena. */
function withdrawal_rlp(withdrawal : ByteSlice) -> ByteSlice = {
    let base = unsigned(withdrawal.off);
    let index = rlp_int(ssz_u64(base + WD_INDEX));
    let validator_index = rlp_int(ssz_u64(base + WD_VALIDATOR_INDEX));
    let amount = rlp_int(ssz_u64(base + WD_AMOUNT));
    let payload_len = list_len(index) + list_len(validator_index) + 21 + list_len(amount);
    let start = scratch_begin();
    scratch_push_bytes(rlp_list_prefix(payload_len));
    scratch_push_bytes(index);
    scratch_push_bytes(validator_index);
    scratch_push_bytes([|0x94|]);
    scratch_push_slice(sub_slice(withdrawal, WD_ADDRESS, 20));
    scratch_push_bytes(amount);
    scratch_finish(start)
}

/*! The block header hash: `keccak256(rlp(header))` with the recomputed
    body roots spliced in (YP §4.4; post-merge constants for ommers,
    difficulty, and nonce). */
function block_header_hash(
    h : BlockHeader,
    transactions_root : hash,
    withdrawals_root : hash,
    requests_hash : hash,
    bal_hash : hash,
) -> (
    hash
) = {
    var content = rlp_builder_encoded(rlp_word(h.parent_hash));
    content = rlp_builder_append_encoded(content, rlp_word(EMPTY_OMMER_HASH));
    content = rlp_builder_append_encoded(content, rlp_addr(h.fee_recipient));
    content = rlp_builder_append_encoded(content, rlp_word(h.state_root));
    content = rlp_builder_append_encoded(content, rlp_word(transactions_root));
    content = rlp_builder_append_encoded(content, rlp_word(h.receipts_root));
    content = rlp_builder_append_encoded(content, rlp_bytes(logs_bloom_bytes(h.logs_bloom)));
    content = rlp_builder_append_encoded(content, rlp_int(0));
    content = rlp_builder_append_encoded(content, rlp_int(h.number));
    content = rlp_builder_append_encoded(content, rlp_int(h.gas_limit));
    content = rlp_builder_append_encoded(content, rlp_int(h.gas_used));
    content = rlp_builder_append_encoded(content, rlp_int(h.timestamp));
    content = rlp_builder_append(content, rlp_builder_slice(h.extra_data));
    content = rlp_builder_append_encoded(content, rlp_word(h.prev_randao));
    content = rlp_builder_append_encoded(content, rlp_bytes(zero_bytes(8)));
    if k_fork >= London then {
        content = rlp_builder_append_encoded(content, rlp_int_word(h.base_fee))
    };
    if k_fork >= Shanghai then {
        content = rlp_builder_append_encoded(content, rlp_word(withdrawals_root))
    };
    if k_fork >= Cancun then {
        content = rlp_builder_append_encoded(content, rlp_int(unsigned(h.blob_gas_used)));
        content = rlp_builder_append_encoded(content, rlp_int(unsigned(h.excess_blob_gas)));
        content = rlp_builder_append_encoded(content, rlp_word(h.parent_beacon_block_root))
    };
    if k_fork >= Prague then {
        content = rlp_builder_append_encoded(content, rlp_word(requests_hash))
    };
    if k_fork >= Amsterdam then {
        content = rlp_builder_append_encoded(content, rlp_word(bal_hash));
        content = rlp_builder_append_encoded(content, rlp_int(unsigned(h.slot_number)))
    };
    keccak256(rlp_builder_segments(rlp_builder_list(content)))
}

function trie_index_key(i : int) -> TriePath =
    path_from_bytes(rlp_int(i))

/* RLP(index) keys sort as 1..127, 0, 128.. . Input bodies arrive in numeric
 * order, so move the first update past the single-byte index range in one
 * linear pass instead of inserting every update through an existing list. */
function place_zero_update(zero : TrieUpdate, rest : list(TrieUpdate), remaining : int) -> list(TrieUpdate) =
    match rest {
        next :: tail => if remaining > 0 then {
            next :: place_zero_update(zero, tail, remaining - 1)
        } else {
            zero :: rest
        },
        [||] => [|zero|],
    }

function order_trie_index_updates(updates : list(TrieUpdate)) -> list(TrieUpdate) =
    match updates {
        zero :: rest => place_zero_update(zero, rest, 127),
        [||] => [||],
    }

function transaction_trie_root(txs : SszListRef) -> hash = {
    var updates_rev : list(TrieUpdate) = [||];
    var cursor = ssz_list_cursor(txs);
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let i = cursor.index;
        let (tx, next) = ssz_list_pop(cursor);
        cursor = next;
        updates_rev = struct { key = trie_index_key(i), change = TriePut(BytesSlice(tx)) } :: updates_rev
    };
    trie_root(EMPTY_TRIE_ROOT, order_trie_index_updates(list_reverse(updates_rev)))
}

function withdrawals_trie_root(wds : SszListRef) -> hash = {
    let scratch_mark = scratch_begin();
    var updates_rev : list(TrieUpdate) = [||];
    var rest = wds;
    var i : int = 0;
    while rest.count > 0 do {
        let (withdrawal_ref, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        updates_rev =
            struct { key = trie_index_key(i), change = TriePut(BytesSlice(withdrawal_rlp(withdrawal_ref))) } ::
                updates_rev;
        i = i + 1
    };
    let root = trie_root(EMPTY_TRIE_ROOT, order_trie_index_updates(list_reverse(updates_rev)));
    scratch_rewind(scratch_mark);
    root
}

function expected_payload_excess_blob_gas(witness : WitnessContext) -> int =
    next_excess_blob_gas(witness.parent_excess_blob_gas, witness.parent_blob_gas_used, witness.parent_base_fee_per_gas)

/* EIP-7685 commitment over the present request-type digests, in request-type
 * order. The request bodies remain source-backed through the hash calls. */
function execution_requests_hash(input_ref : StatelessInputRef) -> hash = {
    let l0 = unsigned(input_ref.deposits.len);
    let l1 = unsigned(input_ref.withdrawal_requests.len);
    let l2 = unsigned(input_ref.consolidation_requests.len);
    let d0 : hash = if l0 != 0 then {
            sha256_request_digest(0, input_ref.deposits)
        } else {
            sail_zeros(256)
        };
    let d1 : hash = if l1 != 0 then {
            sha256_request_digest(1, input_ref.withdrawal_requests)
        } else {
            sail_zeros(256)
        };
    let d2 : hash = if l2 != 0 then {
            sha256_request_digest(2, input_ref.consolidation_requests)
        } else {
            sail_zeros(256)
        };
    var segs : list(Bytes) = [||];
    if l2 != 0 then {
        segs = BytesList(word_to_bytes32(d2)) :: segs
    };
    if l1 != 0 then {
        segs = BytesList(word_to_bytes32(d1)) :: segs
    };
    if l0 != 0 then {
        segs = BytesList(word_to_bytes32(d0)) :: segs
    };
    sha256(segs)
}

/* Validate commitments that can be checked before transaction decoding. The
 * supplied BAL is hashed once for the header; post-execution validation
 * compares its bytes directly with the canonical model encoding. */
function validate_execution_payload(
    input : StatelessInput,
    input_ref : StatelessInputRef,
    witness : WitnessContext,
) -> (
    unit
) = {
    let payload = input.payload;
    let block = payload.block;
    let header = block.header;
    let body = block.body;
    if not_bool(
          (input.chain_config.fork_index >= 9)
        & (input.chain_config.fork_index <= 20)
        & input.chain_config.activation_active
        & chain_config_blob_schedule_valid(input.chain_config),
    ) then {
        throw InvalidBlock(InvalidConfig)
    };
    if witness.parent_hash != header.parent_hash then {
        throw InvalidBlock(InvalidParentHash)
    };
    if (k_fork >= Cancun) & (unsigned(header.excess_blob_gas) != expected_payload_excess_blob_gas(witness)) then {
        throw InvalidBlock(InvalidExcessBlobGas)
    };
    if k_fork >= Paris then {
        let transactions_root = transaction_trie_root(body.transactions);
        let withdrawals_root = if k_fork >= Shanghai then {
                withdrawals_trie_root(body.withdrawals)
            } else {
                EMPTY_TRIE_ROOT
            };
        let requests_hash = if k_fork >= Prague then {
                execution_requests_hash(input_ref)
            } else {
                ZERO_WORD
            };
        let supplied_block_access_list_hash = if k_fork >= Amsterdam then {
                keccak256(body.block_access_list)
            } else {
                ZERO_WORD
            };
        let computed_block_hash = block_header_hash(
            header,
            transactions_root,
            withdrawals_root,
            requests_hash,
            supplied_block_access_list_hash,
        );
        if computed_block_hash != payload.expected_block_hash then {
            throw InvalidBlock(InvalidBlockHash)
        }
    }
}
