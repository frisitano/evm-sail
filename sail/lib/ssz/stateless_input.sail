
/*md
# The stateless input decoder

Materialized references into the concrete `SszStatelessInput` wire format
and the stateless guest decoder. The layout constants transcribe the SSZ
schema; the decoder resolves every variable region once and hands
consumers explicit source spans.
*/
/*md
## Constants

The wire-format layout: field offsets transcribing the SSZ schema, grouped
by container.
*/
/*! The input starts with a 2-byte big-endian schema id; the SSZ body
    follows. */
let SSZ_BODY : int(2) = 2

/* SszStatelessInput: 4 variable fields, offset table at body+{0,4,8,12}. */
let IN_NPR_OFF : int(0) = 0
let IN_WITNESS_OFF : int(4) = 4
let IN_CHAIN_CONFIG_OFF : int(8) = 8
let IN_PUBLIC_KEYS_OFF : int(12) = 12

/* SszNewPayloadRequest. */
let NPR_PAYLOAD_OFF : int(0) = 0
let NPR_VHASHES_OFF : int(4) = 4
let NPR_BEACON_ROOT : int(8) = 8
let NPR_REQUESTS_OFF : int(40) = 40

/* SszExecutionRequests. */
let REQ_DEPOSITS_OFF : int(0) = 0
let REQ_WITHDRAWALS_OFF : int(4) = 4
let REQ_CONSOLIDATIONS_OFF : int(8) = 8
let REQ_BUILDER_DEPOSITS_OFF : int(12) = 12
let REQ_BUILDER_EXITS_OFF : int(16) = 16

/* SszExecutionPayload fixed-region field offsets. */
let PL_FEE_RECIPIENT : int(32) = 32
let PL_STATE_ROOT : int(52) = 52
let PL_RECEIPTS_ROOT : int(84) = 84
let PL_LOGS_BLOOM : int(116) = 116
let PL_PREV_RANDAO : int(372) = 372
let PL_BLOCK_NUMBER : int(404) = 404
let PL_GAS_LIMIT : int(412) = 412
let PL_GAS_USED : int(420) = 420
let PL_TIMESTAMP : int(428) = 428
let PL_EXTRA_OFF : int(436) = 436
let PL_BASE_FEE : int(440) = 440
let PL_BLOCK_HASH : int(472) = 472
let PL_TXS_OFF : int(504) = 504
let PL_WDS_OFF : int(508) = 508
let PL_BLOB_GAS_USED : int(512) = 512
let PL_EXCESS_BLOB_GAS : int(520) = 520
let PL_BAL_OFF : int(528) = 528
let PL_SLOT_NUMBER : int(532) = 532

/* SszWithdrawal fixed element layout. */
let WD_SIZE : int(44) = 44
let WD_INDEX : int(0) = 0
let WD_VALIDATOR_INDEX : int(8) = 8
let WD_ADDRESS : int(16) = 16
let WD_AMOUNT : int(36) = 36

/* SszChainConfig and nested fork activation layouts. */
let CC_CHAIN_ID : int(0) = 0
let CC_ACTIVE_FORK_OFF : int(8) = 8
let FC_ACTIVATION_OFF : int(0) = 0
let FA_BLOCK_NUMBER_OFF : int(0) = 0
let FA_TIMESTAMP_OFF : int(4) = 4

/* SszExecutionWitness. */
let WIT_STATE_OFF : int(0) = 0
let WIT_CODES_OFF : int(4) = 4
let WIT_HEADERS_OFF : int(8) = 8

let STATELESS_INPUT_FIXED_LENGTH : int(18) = 18
let STATELESS_INPUT_BODY_FIXED_LENGTH : int(16) = 16
let NEW_PAYLOAD_REQUEST_FIXED_LENGTH : int(44) = 44
let EXECUTION_PAYLOAD_FIXED_LENGTH : int(540) = 540
let EXECUTION_REQUESTS_FIXED_LENGTH : int(20) = 20
let EXECUTION_WITNESS_FIXED_LENGTH : int(12) = 12
let PUBLIC_KEY_LENGTH : int(65) = 65
let CHAIN_CONFIG_HEADER_LENGTH : int(12) = 12
let CHAIN_CONFIG_MIN_LENGTH : int(24) = 24

/* Amsterdam stateless-input SSZ schema bounds. They are protocol/schema
 * limits, not capacities chosen by the C backend. */
let MAX_EXTRA_DATA_LENGTH : range(extra_data_length_bound, extra_data_length_bound) = sizeof(extra_data_length_bound)
let MAX_TRANSACTION_LENGTH : range(transaction_length_bound, transaction_length_bound) = sizeof(
    transaction_length_bound
)
let MAX_TRANSACTIONS_PER_PAYLOAD : range(transaction_count_bound, transaction_count_bound) = sizeof(
    transaction_count_bound
)
let MAX_WITHDRAWALS_PER_PAYLOAD : range(withdrawal_count_bound, withdrawal_count_bound) = sizeof(withdrawal_count_bound)
let MAX_BLOCK_ACCESS_LIST_LENGTH : range(block_access_list_length_bound, block_access_list_length_bound) = sizeof(
    block_access_list_length_bound
)
let MAX_BLOB_COMMITMENTS_PER_BLOCK : range(blob_commitment_count_bound, blob_commitment_count_bound) = sizeof(
    blob_commitment_count_bound
)
let MAX_WITNESS_NODES : range(witness_node_count_bound, witness_node_count_bound) = sizeof(witness_node_count_bound)
let MAX_WITNESS_NODE_LENGTH : range(witness_node_length_bound, witness_node_length_bound) = sizeof(
    witness_node_length_bound
)
let MAX_WITNESS_CODES : range(witness_code_count_bound, witness_code_count_bound) = sizeof(witness_code_count_bound)
let MAX_WITNESS_CODE_LENGTH : range(witness_code_length_bound, witness_code_length_bound) = sizeof(
    witness_code_length_bound
)
let MAX_WITNESS_HEADERS : range(witness_header_count_bound, witness_header_count_bound) = sizeof(
    witness_header_count_bound
)
let MAX_WITNESS_HEADER_LENGTH : range(witness_header_length_bound, witness_header_length_bound) = sizeof(
    witness_header_length_bound
)
let MAX_PUBLIC_KEYS : range(public_key_count_bound, public_key_count_bound) = sizeof(public_key_count_bound)

/*! Returns the byte position of an entry in a `uint32` SSZ offset table. */
function ssz_offset_table_position(index : ssz_offset_index) -> ssz_offset = {
    index * 4
}

/*! A cursor at the first element of a variable-element SSZ list. */
function ssz_list_cursor forall ('maximum : Int), source_valid_length('maximum). (items : BoundedSszListRef('maximum)) -> (
    BoundedSszListCursor('maximum)
) = {
    let bytes = items.bytes;
    let current = if items.count != 0 then {
            ssz_offset_to_source_pointer(ssz_u32(bytes, 0))
        } else {
            bytes.len
        };
    struct { items = items, index = 0, current = current }
}

/*! Whether the cursor has consumed all elements. */
function ssz_list_cursor_empty forall ('maximum : Int), source_valid_length('maximum). (cursor :
    BoundedSszListCursor('maximum)) -> (
    bool
) =
    cursor.items.count <= cursor.index

/*! The next element's span, and the advanced cursor. */
function ssz_list_pop forall ('maximum : Int), source_valid_length('maximum) & 'maximum <= 2 ^ 30 - 1. (cursor :
    BoundedSszListCursor('maximum)) -> (
    (StatelessInputSlice, BoundedSszListCursor('maximum))
) = {
    let count = cursor.items.count;
    let index = cursor.index;
    if count <= index then {
        throw InvalidBlock(InvalidConfig)
    };
    let next_index : range(0, 'maximum) = index + 1;
    let items = cursor.items;
    let bytes = items.bytes;
    let nat = bytes.len;
    let next = if next_index < items.count then {
            ssz_offset_to_source_pointer(ssz_u32_in_slice(bytes, ssz_offset_table_position(next_index)))
        } else {
            nat
        };
    let current_value = cursor.current;
    let next_value = next;
    if current_value <= next_value then {
        if next_value <= nat then {
            let item_length = next_value - current_value;
            if (items.max_item_length != 0) & (items.max_item_length < item_length) then {
                throw InvalidBlock(InvalidConfig)
            };
            let item = sub_slice(bytes, current_value, item_length);
            (item, struct { items = items, index = next_index, current = next })
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Returns a variable-width list item by resolving its adjacent offsets. */
function ssz_list_at forall ('maximum : Int) ('index : Int),
                       source_valid_length('maximum) & 'maximum <= 2 ^ 30 - 1 & 0 <= 'index. (
    items : BoundedSszListRef('maximum),
    index : int('index),
) -> (
    StatelessInputSlice
) = {
    let bytes = items.bytes;
    let count = items.count;
    if count <= index then {
        throw InvalidBlock(InvalidConfig)
    };
    let next_index : range(0, 'maximum) = index + 1;
    let start = ssz_offset_to_source_pointer(ssz_u32_in_slice(bytes, ssz_offset_table_position(index)));
    let stop : source_pointer = if next_index < items.count then {
            ssz_offset_to_source_pointer(ssz_u32_in_slice(bytes, ssz_offset_table_position(next_index)))
        } else {
            bytes.len
        };
    let start_value = start;
    let stop_value = stop;
    let items_length = bytes.len;
    if start_value <= stop_value then {
        if stop_value <= items_length then {
            let item_length = stop_value - start_value;
            if (items.max_item_length != 0) & (items.max_item_length < item_length) then {
                throw InvalidBlock(InvalidConfig)
            };
            sub_slice(bytes, start, item_length)
        } else {
            throw InvalidBlock(InvalidConfig)
        }
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Returns an item from a fixed-stride SSZ list reference. */
val ssz_fixed_list_at : forall 'maximum 'item_size 'index,
                          source_valid_length('maximum) & source_valid_length('item_size) & 0 <= 'index. (
        BoundedSszListRef('maximum),
        int('index),
        int('item_size),
    ) -> StatelessInputSliceLength('item_size)

function ssz_fixed_list_at(items, index, item_size) = {
    let bytes = items.bytes;
    if items.count <= index then {
        throw InvalidBlock(InvalidConfig)
    };
    let width_value = item_size;
    let offset_value = index * width_value;
    let items_length = bytes.len;
    if offset_value + width_value <= items_length then {
        sub_slice(bytes, offset_value, width_value)
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Pops one fixed-size element from a fixed-stride list reference. */
val ssz_fixed_list_pop : forall 'maximum 'item_size, source_valid_length('maximum) & source_valid_length('item_size). (
        BoundedSszListRef('maximum),
        int('item_size),
    ) -> (StatelessInputSliceLength('item_size), BoundedSszListRef('maximum))

function ssz_fixed_list_pop(items, item_size) = {
    let bytes = items.bytes;
    let items_length = bytes.len;
    let width = item_size;
    let count = items.count;
    if count == 0 then {
        throw InvalidBlock(InvalidConfig)
    };
    if items_length < width then {
        throw InvalidBlock(InvalidConfig)
    };
    let item = sub_slice(bytes, 0, item_size);
    let rest : BoundedSszListRef('maximum) = struct {
            bytes = slice_suffix(bytes, width),
            count = count - 1,
            max_item_length = items.max_item_length,
        };
    (item, rest)
}

/*! Every variable region of the input, resolved once before decoding.
    Consumers receive explicit source spans instead of re-reading nested
    SSZ offset tables. */
struct StatelessInputRef = {
    protocol : ProtocolProfile,
    new_payload_request : StatelessInputSlice,
    execution_payload : StatelessInputSliceAtLeast(540),
    versioned_hashes : StatelessInputSlice,
    deposits : StatelessInputSlice,
    withdrawal_requests : StatelessInputSlice,
    consolidation_requests : StatelessInputSlice,
    builder_deposit_requests : StatelessInputSlice,
    builder_exit_requests : StatelessInputSlice,
    extra_data : StatelessInputSliceAtMost(extra_data_length_bound),
    transactions : TransactionListRef,
    withdrawals : WithdrawalListRef,
    block_access_list : StatelessInputSliceAtMost(block_access_list_length_bound),
    witness_state : WitnessNodeListRef,
    witness_codes : WitnessCodeListRef,
    witness_headers : WitnessHeaderListRef,
    chain_config : StatelessInputSlice,
    public_keys : StatelessInputSlice,
}

/*! Retains a successfully checked fixed-region bound in the slice type. */
val ssz_container_bytes : forall 'minimum, 0 <= 'minimum. (StatelessInputSlice, int('minimum)) -> StatelessInputSliceAtLeast(
        'minimum,
    )

function ssz_container_bytes(bytes, minimum) = {
    let fields = bytes;
    if minimum <= fields.len then {
        fields
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! A sequential position within one SSZ container's variable region. The
    container slice carries the region base and limit, so offsets cannot be
    mixed with those of an enclosing container. */
struct SszContainerCursor = {
    bytes : StatelessInputSlice,
    current : source_pointer,
}

/*! Starts a cursor immediately after a container's statically known fixed
    region. The region length is inferred from the validated slice type, so it
    is not carried as a second runtime value. */
val ssz_container_cursor : forall 'minimum, 0 <= 'minimum. (StatelessInputSliceAtLeast('minimum), int('minimum)) -> SszContainerCursor

function ssz_container_cursor(bytes, fixed_length) =
    struct { bytes = bytes, current = fixed_length }

/*! Takes the next variable field, ending at its container-relative SSZ
    offset, and returns the advanced cursor. */
function ssz_take(cursor : SszContainerCursor, stop : ssz_offset) -> (StatelessInputSlice, SszContainerCursor) = {
    let current_value = cursor.current;
    let bytes = cursor.bytes;
    let container_length = bytes.len;
    if (current_value <= stop) & (stop <= container_length) then {
        let stop_pointer : source_pointer = stop;
        let span_length = stop - current_value;
        let span = sub_slice(bytes, current_value, span_length);
        (span, struct { bytes = bytes, current = stop_pointer })
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Takes the remainder of a container after its last offset-delimited
    field. */
function ssz_finish(cursor : SszContainerCursor) -> StatelessInputSlice = {
    let current_value = cursor.current;
    let bytes = cursor.bytes;
    let container_length = bytes.len;
    if current_value <= container_length then {
        let remaining = container_length - current_value;
        sub_slice(bytes, current_value, remaining)
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Constructs a variable-item list with its SSZ count and per-item byte
    limits attached. Item lengths are checked lazily when reached, before a
    consumer can construct any narrower optimized value. */
function ssz_bounded_variable_list_ref forall ('maximum : Int) ('item_length : Int),
                                         source_valid_length('maximum) & source_valid_length('item_length). (
    bytes : StatelessInputSlice,
    maximum_count : int('maximum),
    maximum_item_length : int('item_length),
) -> (
    BoundedSszListRef('maximum)
) = {
    let span = bytes.len;
    let raw_count : ssz_offset_index = if span == 0 then {
            0
        } else {
            if span < SSZ_OFF_BYTES then {
                throw InvalidBlock(InvalidConfig)
            };
            let first_offset = ssz_u32(bytes, 0);
            let count : ssz_offset_index = tdiv_nat(first_offset, 4);
            if (tmod_int(first_offset, 4) != 0) | (count == 0) | first_offset > span then {
                throw InvalidBlock(InvalidConfig)
            };
            count
        };
    if maximum_count < raw_count then {
        throw InvalidBlock(InvalidConfig)
    };
    let count : range(0, 'maximum) = raw_count;
    struct { bytes = bytes, count = count, max_item_length = maximum_item_length }
}

/*! Constructs a schema-bounded fixed-item SSZ list reference. */
function ssz_bounded_fixed_list_ref forall ('maximum : Int) ('item_size : Int),
                                      source_valid_length('maximum) & source_valid_length('item_size). (
    bytes : StatelessInputSlice,
    item_size : int('item_size),
    maximum_count : int('maximum),
) -> (
    BoundedSszListRef('maximum)
) = {
    let width = item_size;
    if width == 0 then {
        throw InvalidBlock(InvalidConfig)
    };
    let span = bytes.len;
    let raw_count = span / width;
    if span != raw_count * width then {
        throw InvalidBlock(InvalidConfig)
    };
    if maximum_count < raw_count then {
        throw InvalidBlock(InvalidConfig)
    };
    let count : range(0, 'maximum) = raw_count;
    struct { bytes = bytes, count = count, max_item_length = item_size }
}

/*! Resolves the input's offset tables into a
    [StatelessInputRef][type-StatelessInputRef], validating the schema id
    and every region bound; a malformed frame is `InvalidConfig`. */
function decode_stateless_input_ref(input : StatelessInputSlice) -> StatelessInputRef = {
    let input_fields = input;
    let input_length = input_fields.len;
    let fixed_length = STATELESS_INPUT_FIXED_LENGTH;
    let body_offset_value = SSZ_BODY;
    if input_length < fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    if slice_byte(input_fields, 1) != 0x01 then {
        throw InvalidBlock(InvalidConfig)
    };
    let schema_fork = slice_byte(input_fields, 0);
    if not_bool(schema_protocol_profile_forwards_matches(schema_fork)) then {
        throw InvalidBlock(InvalidConfig)
    };
    let protocol : ProtocolProfile = schema_protocol_profile(schema_fork);
    let body_bytes = slice_suffix(input_fields, body_offset_value);
    let body = ssz_container_bytes(body_bytes, STATELESS_INPUT_BODY_FIXED_LENGTH);
    let container_start : int(0) = 0;

    let new_payload_request_offset = ssz_u32(body, ssz_field_offset(container_start, IN_NPR_OFF));
    let witness_offset = ssz_u32(body, ssz_field_offset(container_start, IN_WITNESS_OFF));
    let chain_config_offset = ssz_u32(body, ssz_field_offset(container_start, IN_CHAIN_CONFIG_OFF));
    let public_keys_offset = ssz_u32(body, ssz_field_offset(container_start, IN_PUBLIC_KEYS_OFF));
    let body_fixed_length = STATELESS_INPUT_BODY_FIXED_LENGTH;
    if new_payload_request_offset != body_fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let body_cursor = ssz_container_cursor(body, STATELESS_INPUT_BODY_FIXED_LENGTH);
    let (new_payload_request_bytes, body_after_payload_request) = ssz_take(body_cursor, witness_offset);
    let new_payload_request = ssz_container_bytes(new_payload_request_bytes, NEW_PAYLOAD_REQUEST_FIXED_LENGTH);
    let (execution_witness_bytes, body_after_witness) = ssz_take(body_after_payload_request, chain_config_offset);
    let execution_witness = ssz_container_bytes(execution_witness_bytes, EXECUTION_WITNESS_FIXED_LENGTH);
    let (chain_config, body_after_chain_config) = ssz_take(body_after_witness, public_keys_offset);
    let public_keys = ssz_finish(body_after_chain_config);

    let npr_start : int(0) = 0;
    let payload_offset = ssz_u32(new_payload_request, ssz_field_offset(npr_start, NPR_PAYLOAD_OFF));
    let versioned_hashes_offset = ssz_u32(new_payload_request, ssz_field_offset(npr_start, NPR_VHASHES_OFF));
    let requests_offset = ssz_u32(new_payload_request, ssz_field_offset(npr_start, NPR_REQUESTS_OFF));
    let npr_fixed_length = NEW_PAYLOAD_REQUEST_FIXED_LENGTH;
    if payload_offset != npr_fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let npr_cursor = ssz_container_cursor(new_payload_request, NEW_PAYLOAD_REQUEST_FIXED_LENGTH);
    let (execution_payload_bytes, npr_after_payload) = ssz_take(npr_cursor, versioned_hashes_offset);
    let (versioned_hashes, npr_after_versioned_hashes) = ssz_take(npr_after_payload, requests_offset);
    let execution_requests_bytes = ssz_finish(npr_after_versioned_hashes);
    let execution_requests = ssz_container_bytes(execution_requests_bytes, EXECUTION_REQUESTS_FIXED_LENGTH);

    let execution_payload = ssz_container_bytes(execution_payload_bytes, EXECUTION_PAYLOAD_FIXED_LENGTH);
    let payload_start : int(0) = 0;
    let extra_data_offset = ssz_u32(execution_payload, ssz_field_offset(payload_start, PL_EXTRA_OFF));
    let transactions_offset = ssz_u32(execution_payload, ssz_field_offset(payload_start, PL_TXS_OFF));
    let withdrawals_offset = ssz_u32(execution_payload, ssz_field_offset(payload_start, PL_WDS_OFF));
    let block_access_list_offset = ssz_u32(execution_payload, ssz_field_offset(payload_start, PL_BAL_OFF));
    let payload_fixed_length = EXECUTION_PAYLOAD_FIXED_LENGTH;
    if extra_data_offset != payload_fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let payload_cursor = ssz_container_cursor(execution_payload, EXECUTION_PAYLOAD_FIXED_LENGTH);
    let (extra_data_bytes, payload_after_extra_data) = ssz_take(payload_cursor, transactions_offset);
    let (transaction_bytes, payload_after_transactions) = ssz_take(payload_after_extra_data, withdrawals_offset);
    let transactions = ssz_bounded_variable_list_ref(
        transaction_bytes,
        MAX_TRANSACTIONS_PER_PAYLOAD,
        MAX_TRANSACTION_LENGTH,
    );
    let (withdrawal_bytes, payload_after_withdrawals) = ssz_take(payload_after_transactions, block_access_list_offset);
    let withdrawals = ssz_bounded_fixed_list_ref(withdrawal_bytes, WD_SIZE, MAX_WITHDRAWALS_PER_PAYLOAD);
    let block_access_list_bytes = ssz_finish(payload_after_withdrawals);
    let extra_data : StatelessInputSliceAtMost(extra_data_length_bound) =
        if extra_data_bytes.len <= MAX_EXTRA_DATA_LENGTH then {
            extra_data_bytes
        } else {
            throw InvalidBlock(InvalidConfig)
        };
    let block_access_list : StatelessInputSliceAtMost(block_access_list_length_bound) =
        if block_access_list_bytes.len <= MAX_BLOCK_ACCESS_LIST_LENGTH then {
            block_access_list_bytes
        } else {
            throw InvalidBlock(InvalidConfig)
        };

    let requests_start : int(0) = 0;
    let deposits_offset = ssz_u32(execution_requests, ssz_field_offset(requests_start, REQ_DEPOSITS_OFF));
    let withdrawal_requests_offset = ssz_u32(execution_requests, ssz_field_offset(requests_start, REQ_WITHDRAWALS_OFF));
    let consolidation_requests_offset = ssz_u32(
        execution_requests,
        ssz_field_offset(requests_start, REQ_CONSOLIDATIONS_OFF),
    );
    let builder_deposit_requests_offset = ssz_u32(
        execution_requests,
        ssz_field_offset(requests_start, REQ_BUILDER_DEPOSITS_OFF),
    );
    let builder_exit_requests_offset = ssz_u32(
        execution_requests,
        ssz_field_offset(requests_start, REQ_BUILDER_EXITS_OFF),
    );
    let requests_fixed_length = EXECUTION_REQUESTS_FIXED_LENGTH;
    if deposits_offset != requests_fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let requests_cursor = ssz_container_cursor(execution_requests, EXECUTION_REQUESTS_FIXED_LENGTH);
    let (deposits, requests_after_deposits) = ssz_take(requests_cursor, withdrawal_requests_offset);
    let (withdrawal_requests, requests_after_withdrawals) = ssz_take(
        requests_after_deposits,
        consolidation_requests_offset,
    );
    let (consolidation_requests, requests_after_consolidations) = ssz_take(
        requests_after_withdrawals,
        builder_deposit_requests_offset,
    );
    let (builder_deposit_requests, requests_after_builder_deposits) = ssz_take(
        requests_after_consolidations,
        builder_exit_requests_offset,
    );
    let builder_exit_requests = ssz_finish(requests_after_builder_deposits);

    let witness_start : int(0) = 0;
    let witness_state_offset = ssz_u32(execution_witness, ssz_field_offset(witness_start, WIT_STATE_OFF));
    let witness_codes_offset = ssz_u32(execution_witness, ssz_field_offset(witness_start, WIT_CODES_OFF));
    let witness_headers_offset = ssz_u32(execution_witness, ssz_field_offset(witness_start, WIT_HEADERS_OFF));
    let witness_fixed_length = EXECUTION_WITNESS_FIXED_LENGTH;
    if witness_state_offset != witness_fixed_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let witness_cursor = ssz_container_cursor(execution_witness, EXECUTION_WITNESS_FIXED_LENGTH);
    let (witness_state_bytes, witness_after_state) = ssz_take(witness_cursor, witness_codes_offset);
    let witness_state = ssz_bounded_variable_list_ref(witness_state_bytes, MAX_WITNESS_NODES, MAX_WITNESS_NODE_LENGTH);
    let (witness_code_bytes, witness_after_codes) = ssz_take(witness_after_state, witness_headers_offset);
    let witness_codes = ssz_bounded_variable_list_ref(witness_code_bytes, MAX_WITNESS_CODES, MAX_WITNESS_CODE_LENGTH);
    let witness_headers = ssz_bounded_variable_list_ref(
        ssz_finish(witness_after_codes),
        MAX_WITNESS_HEADERS,
        MAX_WITNESS_HEADER_LENGTH,
    );

    let public_key_bytes = public_keys.len;
    let public_key_length = PUBLIC_KEY_LENGTH;
    let public_key_count = public_key_bytes / public_key_length;
    if public_key_bytes != public_key_count * public_key_length then {
        throw InvalidBlock(InvalidConfig)
    };
    if MAX_PUBLIC_KEYS < public_key_count then {
        throw InvalidBlock(InvalidConfig)
    };
    let _ = ssz_bounded_fixed_list_ref(versioned_hashes, WORD_BYTE_LENGTH, MAX_BLOB_COMMITMENTS_PER_BLOCK);

    struct {
        protocol = protocol,
        new_payload_request = new_payload_request,
        execution_payload = execution_payload,
        versioned_hashes = versioned_hashes,
        deposits = deposits,
        withdrawal_requests = withdrawal_requests,
        consolidation_requests = consolidation_requests,
        builder_deposit_requests = builder_deposit_requests,
        builder_exit_requests = builder_exit_requests,
        extra_data = extra_data,
        transactions = transactions,
        withdrawals = withdrawals,
        block_access_list = block_access_list,
        witness_state = witness_state,
        witness_codes = witness_codes,
        witness_headers = witness_headers,
        chain_config = chain_config,
        public_keys = public_keys,
    }
}

/*! The EIP-7685 per-type request digest:
    `sha256(request_type ‖ request_data)`. */
function sha256_request_digest(request_type : byte, s : StatelessInputSlice) -> hash = {
    let mark = scratch_reserve(scratch_length_add(1, s.len));
    scratch_push_byte(request_type);
    scratch_push_slice(s);
    let digest = sha256(scratch_finish(mark));
    scratch_rewind(mark);
    digest
}

/* Index every witness node/code directly from its SSZ list reference. */
/*! Hashes and indexes every remaining witness trie node from its source
    slice. */
function index_witness_nodes_cursor(cursor : WitnessNodeListCursor) -> unit =
    if ssz_list_cursor_empty(cursor) then {
        ()
    } else {
        let (node, next) = ssz_list_pop(cursor);
        nodedb_insert(keccak256(node), node.off, node.len);
        index_witness_nodes_cursor(next)
    }

/* Proof-only: one collection cursor position is consumed per recursive call. */
termination_measure index_witness_nodes_cursor(cursor) =
    cursor.items.count - cursor.index

/*! Indexes every witness trie node into the node-db, keyed by its
    KECCAK-256 digest, directly from the SSZ list reference. */
function index_witness_nodes(nodes : WitnessNodeListRef) -> unit =
    index_witness_nodes_cursor(ssz_list_cursor(nodes))

/*! Analyzes and indexes every remaining witness code body from its source
    slice. */
function index_witness_codes_cursor(cursor : WitnessCodeListCursor) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if ssz_list_cursor_empty(cursor) then {
        ()
    } else {
        let (code, next) = ssz_list_pop(cursor);

        /* The witness envelope permits code blobs up to 2^16 bytes
         * independently of the active fork's deployment limit. */
        let code_length = code.len;
        if MAX_WITNESS_CODE_LENGTH < code_length then {
            throw InvalidBlock(InvalidConfig)
        };
        let executable = code_db_intern_input(code);
        let _ = code_db_insert(executable, profile.fork);
        index_witness_codes_cursor(next)
    }
}

/* Proof-only: one collection cursor position is consumed per recursive call. */
termination_measure index_witness_codes_cursor(cursor) =
    cursor.items.count - cursor.index

/*! Indexes every witness code blob into the content-addressed code
    store. */
function index_witness_codes(codes : WitnessCodeListRef) -> unit =
    index_witness_codes_cursor(ssz_list_cursor(codes))

/*! The parent-header fields required to initialize execution context. */
struct ParentHeaderFields = {
    parent_hash : hash,
    state_root : hash,
    base_fee : word,
    blob_gas_used : blob_gas_used,
    excess_blob_gas : excess_blob_gas,
    have_parent : bool,
    have_state : bool,
    have_base_fee : bool,
    have_blob_gas : bool,
    have_excess_blob_gas : bool,
}

let EMPTY_PARENT_HEADER_FIELDS : ParentHeaderFields = struct {
        parent_hash = ZERO_HASH,
        state_root = ZERO_HASH,
        base_fee = ZERO_WORD,
        blob_gas_used = 0,
        excess_blob_gas = 0,
        have_parent = false,
        have_state = false,
        have_base_fee = false,
        have_blob_gas = false,
        have_excess_blob_gas = false,
    }

/* Field 19 is a sentinel for every header field after the last position used
 * by execution context.  The decoder still consumes and validates the entire
 * RLP list, but its control state does not need an unbounded raw item count. */
/*! A bounded parent-header field position, including the unused-field sentinel. */
type parent_header_field_cursor = range(0, 19)

/*! Advances the parent-header field cursor, saturating at its unused-field
    sentinel. */
function next_parent_header_field(index : parent_header_field_cursor) -> parent_header_field_cursor =
    if index < 19 then {
        index + 1
    } else {
        19
    }

/* The unconsumed RLP content span strictly decreases on every call. */
/*! Extracts the execution-relevant fields while walking one parent header. */
function decode_parent_header_fields forall 'source_off 'source_len, source_valid_range('source_off, 'source_len). (
    cursor : RlpCursor('source_off, 'source_len),
    field_index : parent_header_field_cursor,
    fields : ParentHeaderFields,
) -> (
    ParentHeaderFields
) =
    if cursor.len == 0 then {
        fields
    } else {
        let field = rlp_decode_item(cursor);
        let next = rlp_cursor_advance(cursor, field.source.len);
        var decoded = fields;
        if field_index == 0 then {
            decoded.parent_hash = word_to_hash(rlp_decode_word(field));
            decoded.have_parent = true
        } else {
            if field_index == 3 then {
                decoded.state_root = word_to_hash(rlp_decode_word(field));
                decoded.have_state = true
            } else {
                if field_index == 15 then {
                    decoded.base_fee = rlp_decode_u256(field);
                    decoded.have_base_fee = true
                } else {
                    if field_index == 17 then {
                        let value = rlp_decode_uint64(field);
                        let count = value / sizeof(gas_per_blob_value);
                        let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
                        if
                            (count <= profile.blob_schedule.max)
                            & (value == sizeof(gas_per_blob_value) * count)
                        then {
                            decoded.blob_gas_used =
                                sizeof(gas_per_blob_value) * count;
                            decoded.have_blob_gas = true
                        } else {
                            throw InvalidBlock(RlpDecode)
                        }
                    } else {
                        if field_index == 18 then {
                            let value = rlp_decode_uint64(field);
                            let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
                            let limit = profile.excess_blob_gas_limit;
                            if value <= limit then {
                                decoded.excess_blob_gas = value;
                                decoded.have_excess_blob_gas = true
                            } else {
                                throw InvalidBlock(RlpDecode)
                            }
                        }
                    }
                }
            }
        };
        decode_parent_header_fields(next, next_parent_header_field(field_index), decoded)
    }
termination_measure decode_parent_header_fields(cursor, field_index, fields) =
    cursor.len

/*! The running authentication and execution context of the witness header
    chain. */
struct WitnessHeaderIndex = {
    cursor : WitnessHeaderListCursor,
    previous_hash : hash,
    valid : bool,
    parent_state_root : hash,
    parent_base_fee_per_gas : word,
    parent_blob_gas_used : blob_gas_used,
    parent_excess_blob_gas : excess_blob_gas,
    parent_fields_valid : bool,
}

/*! Authenticates and indexes each remaining ancestor header in chain order. */
function index_witness_header_cursor(state : WitnessHeaderIndex) -> WitnessHeaderIndex = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if ssz_list_cursor_empty(state.cursor) then {
        state
    } else {
        let index = state.cursor.index;
        let (header, next) = ssz_list_pop(state.cursor);
        let is_last = next.index == state.cursor.items.count;
        var result = state;
        result.cursor = next;
        if (index != 0) | is_last then {
            let fields = rlp_node_cursor(header);
            let decoded = decode_parent_header_fields(fields, 0, EMPTY_PARENT_HEADER_FIELDS);
            if (index != 0) & (not_bool(decoded.have_parent) | (decoded.parent_hash != state.previous_hash)) then {
                result.valid = false
            };
            if is_last then {
                result.parent_state_root = decoded.state_root;
                result.parent_base_fee_per_gas = decoded.base_fee;
                result.parent_blob_gas_used = decoded.blob_gas_used;
                result.parent_excess_blob_gas = decoded.excess_blob_gas;
                result.parent_fields_valid =
                      decoded.have_state
                    & ((profile.fork < Cancun) | decoded.have_base_fee)
                    & ((profile.fork < Cancun) | (decoded.have_blob_gas == decoded.have_excess_blob_gas))
            }
        };
        let current_hash = keccak256(header);
        result.previous_hash = current_hash;
        let header_count = state.cursor.items.count;
        let next_index = next.index;
        let distance : range(0, witness_header_count_bound) = if next_index <= header_count then {
                header_count - next_index
            } else {
                throw InvalidBlock(WitnessDeficient)
            };
        if distance < 256 then {
            let ancestor : ancestor_index = distance;
            ancestor_hash_write(ancestor, current_hash)
        };
        index_witness_header_cursor(result)
    }
}

/* Proof-only: one collection cursor position is consumed per recursive call. */
termination_measure index_witness_header_cursor(state) =
    state.cursor.items.count - state.cursor.index

/*! Parses the witness header chain once. Each header is hashed into the
    distance-indexed ancestor table (the `BLOCKHASH` source), adjacent
    parent hashes are checked (a break is `HeaderChainBroken`), and the
    newest header's execution context — parent state root, base fee, blob
    gas — is decoded while its fields are consumed head-to-tail. */
function index_witness_headers(headers : WitnessHeaderListRef) -> WitnessContext = {
    let indexed = index_witness_header_cursor(
        struct {
            cursor = ssz_list_cursor(headers),
            previous_hash = ZERO_HASH,
            valid = headers.count != 0,
            parent_state_root = ZERO_HASH,
            parent_base_fee_per_gas = ZERO_WORD,
            parent_blob_gas_used = 0,
            parent_excess_blob_gas = 0,
            parent_fields_valid = false,
        },
    );
    k_n_headers = headers.count;
    if not_bool(indexed.valid) then {
        throw InvalidBlock(WitnessDeficient)
    };
    if not_bool(indexed.parent_fields_valid) then {
        throw InvalidBlock(RlpDecode)
    };
    struct {
        parent_hash = indexed.previous_hash,
        parent_state_root = indexed.parent_state_root,
        parent_base_fee_per_gas = indexed.parent_base_fee_per_gas,
        parent_blob_gas_used = indexed.parent_blob_gas_used,
        parent_excess_blob_gas = indexed.parent_excess_blob_gas,
    }
}

/*! Decodes the execution-payload header fields from their fixed SSZ
    offsets. */
function decode_payload_blob_gas_used(
    payload : StatelessInputSlice,
    profile : ProtocolProfile,
) -> blob_gas_used = {
    let value = decode_ssz_uint(payload, PL_BLOB_GAS_USED);

    /* EIP-4844 makes this field GAS_PER_BLOB times the number of blobs.
     * The decoded profile supplies the active schedule maximum. */
    let count = value / sizeof(gas_per_blob_value);
    if
        (count <= profile.blob_schedule.max)
        & (value == sizeof(gas_per_blob_value) * count)
    then {
        sizeof(gas_per_blob_value) * count
    } else {
        throw InvalidBlock(InvalidBlobGasUsed)
    }
}

/*! Narrows the SSZ `uint64` excess field to the supported-fork
    reachable-chain invariant. The wider wire value remains explicit in
    `excess_blob_gas_wire_bound`; this check relies on an authenticated,
    previously valid parent chain, not on SSZ alone. */
function decode_payload_excess_blob_gas(
    payload : StatelessInputSlice,
    profile : ProtocolProfile,
) -> excess_blob_gas = {
    let value = decode_ssz_uint(payload, PL_EXCESS_BLOB_GAS);
    let limit = profile.excess_blob_gas_limit;
    if value <= limit then {
        value
    } else {
        throw InvalidBlock(InvalidExcessBlobGas)
    }
}

/*! Decodes the execution-payload header fields from their fixed SSZ
    offsets. */
function decode_block_header_ssz(input_ref : StatelessInputRef) -> BlockHeader = {
    let payload = input_ref.execution_payload;
    let gas_limit_value = decode_ssz_uint(payload, PL_GAS_LIMIT);
    let gas_used_value = decode_ssz_uint(payload, PL_GAS_USED);
    struct {
        number = decode_ssz_uint(payload, PL_BLOCK_NUMBER),
        timestamp = decode_ssz_uint(payload, PL_TIMESTAMP),
        gas_limit = gas_limit_value,
        gas_used = gas_used_value,
        prev_randao = hash_to_word(ssz_bytes32(payload, PL_PREV_RANDAO)),
        base_fee = ssz_u256(payload, PL_BASE_FEE),
        blob_gas_used = decode_payload_blob_gas_used(payload, input_ref.protocol),
        excess_blob_gas = decode_payload_excess_blob_gas(payload, input_ref.protocol),
        state_root = ssz_bytes32(payload, PL_STATE_ROOT),
        receipts_root = ssz_bytes32(payload, PL_RECEIPTS_ROOT),
        logs_bloom = sub_slice(payload, PL_LOGS_BLOOM, 256),
        fee_recipient = ssz_addr(payload, PL_FEE_RECIPIENT),
        parent_hash = ssz_bytes32(payload, 0),
        parent_beacon_block_root = ssz_bytes32(input_ref.new_payload_request, NPR_BEACON_ROOT),
        slot_number = decode_ssz_uint(payload, PL_SLOT_NUMBER),
        extra_data = input_ref.extra_data,
    }
}

/*! Decodes one fixed-layout SSZ withdrawal element. */
function decode_withdrawal(withdrawal : StatelessInputSlice) -> Withdrawal = {
    struct {
        index = decode_ssz_uint(withdrawal, WD_INDEX),
        validator_index = decode_ssz_uint(withdrawal, WD_VALIDATOR_INDEX),
        address = ssz_addr(withdrawal, WD_ADDRESS),
        amount = decode_ssz_uint(withdrawal, WD_AMOUNT),
    }
}

/*! Decodes the `SszChainConfig` at `cc`: the chain id (`u64` at 0) and
    the active Amsterdam `SszForkConfig` activation.
    The activation point (optional block number / timestamp, `List[u64,1]`
    each) must be reached by this payload: at least one bound set, none
    exceeding the payload's — a future activation invalidates the block. */
function decode_chain_config(
    cc : StatelessInputSlice,
    number : block_number,
    timestamp : block_timestamp,
) -> (
    ChainConfig
) = {
    let cc_length = cc.len;
    let header_length = CHAIN_CONFIG_HEADER_LENGTH;
    let minimum_length = CHAIN_CONFIG_MIN_LENGTH;
    if cc_length < header_length then {
        throw InvalidBlock(InvalidConfig)
    };
    let f_offset = ssz_u32(cc, CC_ACTIVE_FORK_OFF);
    if (f_offset != 12) | (cc_length < minimum_length) then {
        throw InvalidBlock(InvalidConfig)
    };
    let f : int(12) = 12;
    let activation_offset = ssz_u32(cc, ssz_field_offset(f, FC_ACTIVATION_OFF));
    if activation_offset != 4 then {
        throw InvalidBlock(InvalidConfig)
    };
    let activation_start : int(16) = 16;
    let activation_fixed_end : int(24) = 24;
    let a : int(16) = 16;
    let block_number_start = activation_start + ssz_u32(cc, ssz_field_offset(a, FA_BLOCK_NUMBER_OFF));
    let timestamp_start = activation_start + ssz_u32(cc, ssz_field_offset(a, FA_TIMESTAMP_OFF));
    if activation_fixed_end != block_number_start then {
        throw InvalidBlock(InvalidConfig)
    };
    if timestamp_start < block_number_start then {
        throw InvalidBlock(InvalidConfig)
    };
    if cc_length < timestamp_start then {
        throw InvalidBlock(InvalidConfig)
    };
    let bn_length = timestamp_start - block_number_start;
    let ts_length = cc_length - timestamp_start;
    if ((bn_length != 0) & (bn_length != SSZ_UINT_BYTES)) | ((ts_length != 0) & (ts_length != SSZ_UINT_BYTES)) then {
        throw InvalidBlock(InvalidConfig)
    };
    if (bn_length == 0) & (ts_length == 0) then {
        throw InvalidBlock(InvalidConfig)
    };
    if bn_length == SSZ_UINT_BYTES then {
        let activation_block : block_number = decode_ssz_uint(cc, block_number_start);
        if number < activation_block then {
            throw InvalidBlock(InvalidConfig)
        }
    };
    if ts_length == SSZ_UINT_BYTES then {
        let activation_timestamp : block_timestamp = decode_ssz_uint(cc, timestamp_start);
        if timestamp < activation_timestamp then {
            throw InvalidBlock(InvalidConfig)
        }
    };
    struct { chain_id = decode_ssz_uint(cc, CC_CHAIN_ID) }
}

/*! Decodes the semantic payload structure — header, chain config, body
    references — without touching an encoded transaction or withdrawal
    element, and installs the header and chain parameters in the kernel. */
function decode_stateless_input(input_ref : StatelessInputRef) -> StatelessInput = {
    let payload = input_ref.execution_payload;
    let header = decode_block_header_ssz(input_ref);
    let chain_config = decode_chain_config(input_ref.chain_config, header.number, header.timestamp);
    k_set_header(header);
    k_chain_id = chain_config.chain_id;
    k_execution_profile = execution_profile_for(input_ref.protocol, header.gas_limit);
    struct {
        payload =
            struct {
                expected_block_hash = ssz_bytes32(input_ref.execution_payload, PL_BLOCK_HASH),
                block =
                    struct {
                        header = header,
                        body =
                            struct {
                                transactions = input_ref.transactions,
                                withdrawals = input_ref.withdrawals,
                                block_access_list = input_ref.block_access_list,
                            },
                    },
            },
        chain_config = chain_config,
    }
}

/*! Indexes the source-backed witness (nodes, codes, headers) and
    installs its authenticated parent-state anchor; no witness list is
    materialized. */
function index_execution_witness(input_ref : StatelessInputRef) -> WitnessContext = {
    nodedb_reset();
    index_witness_nodes(input_ref.witness_state);
    index_witness_codes(input_ref.witness_codes);
    let witness = index_witness_headers(input_ref.witness_headers);
    k_parent_state_root = witness.parent_state_root;
    witness
}

/*! Decodes one transaction envelope with its witnessed public key. */
val decode_transaction : forall 'transaction_off 'transaction_len 'public_key_off,
                           source_valid_range('transaction_off, 'transaction_len) &
                               source_valid_range('public_key_off, 65). (
        StatelessInputSliceFields('transaction_off, 'transaction_len),
        StatelessInputSliceFields('public_key_off, 65),
    ) -> Transaction
function decode_transaction(transaction, public_key) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    if transaction.len <= sizeof(transaction_length_bound) then {
        rlp_decode_tx(
            transaction,
            public_key,
            profile.transaction_blob_limit,
        )
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}
