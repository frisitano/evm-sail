/* ======================================================================== */
/* SSZ: SszStatelessInput REFS AND DECODER                                  */
/*                                                                          */
/* Materialized references into the concrete SszStatelessInput wire format  */
/* and the stateless guest decoder.                                         */
/* ======================================================================== */

/* The input starts with a 2-byte big-endian schema id; the SSZ body follows. */
let SSZ_BODY : source_pointer = ByteQuantity(2)

/* SszStatelessInput: 4 variable fields, offset table at body+{0,4,8,12}. */
let IN_NPR_OFF : source_pointer = BYTE_ZERO
let IN_WITNESS_OFF : source_pointer = ByteQuantity(4)
let IN_CHAIN_CONFIG_OFF : source_pointer = ByteQuantity(8)
let IN_PUBLIC_KEYS_OFF : source_pointer = ByteQuantity(12)

/* SszNewPayloadRequest. */
let NPR_PAYLOAD_OFF : source_pointer = BYTE_ZERO
let NPR_VHASHES_OFF : source_pointer = ByteQuantity(4)
let NPR_BEACON_ROOT : source_pointer = ByteQuantity(8)
let NPR_REQUESTS_OFF : source_pointer = ByteQuantity(40)

/* SszExecutionRequests. */
let REQ_DEPOSITS_OFF : source_pointer = BYTE_ZERO
let REQ_WITHDRAWALS_OFF : source_pointer = ByteQuantity(4)
let REQ_CONSOLIDATIONS_OFF : source_pointer = ByteQuantity(8)

/* SszExecutionPayload fixed-region field offsets. */
let PL_FEE_RECIPIENT : source_pointer = ByteQuantity(32)
let PL_STATE_ROOT : source_pointer = ByteQuantity(52)
let PL_RECEIPTS_ROOT : source_pointer = ByteQuantity(84)
let PL_LOGS_BLOOM : source_pointer = ByteQuantity(116)
let PL_PREV_RANDAO : source_pointer = ByteQuantity(372)
let PL_BLOCK_NUMBER : source_pointer = ByteQuantity(404)
let PL_GAS_LIMIT : source_pointer = ByteQuantity(412)
let PL_GAS_USED : source_pointer = ByteQuantity(420)
let PL_TIMESTAMP : source_pointer = ByteQuantity(428)
let PL_EXTRA_OFF : source_pointer = ByteQuantity(436)
let PL_BASE_FEE : source_pointer = ByteQuantity(440)
let PL_BLOCK_HASH : source_pointer = ByteQuantity(472)
let PL_TXS_OFF : source_pointer = ByteQuantity(504)
let PL_WDS_OFF : source_pointer = ByteQuantity(508)
let PL_BLOB_GAS_USED : source_pointer = ByteQuantity(512)
let PL_EXCESS_BLOB_GAS : source_pointer = ByteQuantity(520)
let PL_BAL_OFF : source_pointer = ByteQuantity(528)
let PL_SLOT_NUMBER : source_pointer = ByteQuantity(532)

/* SszWithdrawal fixed element layout. */
let WD_SIZE : byte_length = ByteQuantity(44)
let WD_INDEX : source_pointer = BYTE_ZERO
let WD_VALIDATOR_INDEX : source_pointer = ByteQuantity(8)
let WD_ADDRESS : source_pointer = ByteQuantity(16)
let WD_AMOUNT : source_pointer = ByteQuantity(36)

/* SszChainConfig and nested fork activation layouts. */
let CC_CHAIN_ID : source_pointer = BYTE_ZERO
let CC_ACTIVE_FORK_OFF : source_pointer = ByteQuantity(8)
let FC_FORK : source_pointer = BYTE_ZERO
let FC_ACTIVATION_OFF : source_pointer = ByteQuantity(8)
let FC_BLOBSCHED_OFF : source_pointer = ByteQuantity(12)
let FA_BLOCK_NUMBER_OFF : source_pointer = BYTE_ZERO
let FA_TIMESTAMP_OFF : source_pointer = ByteQuantity(4)

/* SszExecutionWitness. */
let WIT_STATE_OFF : source_pointer = BYTE_ZERO
let WIT_CODES_OFF : source_pointer = ByteQuantity(4)
let WIT_HEADERS_OFF : source_pointer = ByteQuantity(8)

let STATELESS_INPUT_FIXED_LENGTH : byte_length = ByteQuantity(18)
let PUBLIC_KEY_LENGTH : byte_length = ByteQuantity(65)
let CHAIN_CONFIG_HEADER_LENGTH : byte_length = ByteQuantity(12)
let CHAIN_CONFIG_MIN_LENGTH : byte_length = ByteQuantity(28)
let BLOB_SCHEDULE_LENGTH : byte_length = ByteQuantity(24)

function ssz_offset_table_position(index : item_index) -> source_pointer = {
    let position = index * 4;
    if position <= BYTE_QUANTITY_MAX then {
        ByteQuantity(position)
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

function ssz_list_cursor(items : SszListRef) -> SszListCursor = {
    let current = if items.count != 0 then {
            ByteQuantity(ssz_u32(items.bytes, BYTE_ZERO))
        } else {
            items.bytes.len
        };
    struct { items = items, index = 0, current = current }
}

function ssz_list_cursor_empty(cursor : SszListCursor) -> bool = cursor.items.count <= cursor.index

function ssz_list_pop(cursor : SszListCursor) -> (ByteSlice, SszListCursor) = {
    if ssz_list_cursor_empty(cursor) then {
        throw InvalidBlock(InvalidConfig)
    };
    let next_index = protocol_quantity_increment(cursor.index);
    let stop = cursor.items.bytes.len;
    let next = if next_index < cursor.items.count then {
            ByteQuantity(ssz_u32(cursor.items.bytes, ssz_offset_table_position(next_index)))
        } else {
            stop
        };
    if cursor.current <= next & next <= stop then {
        let item = sub_slice(cursor.items.bytes, cursor.current, next - cursor.current);
        (item, struct { items = cursor.items, index = next_index, current = next })
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

function ssz_list_at(items : SszListRef, index : item_index) -> ByteSlice = {
    if items.count <= index then {
        throw InvalidBlock(InvalidConfig)
    };
    let next_index = protocol_quantity_increment(index);
    let start = ByteQuantity(ssz_u32_at(items.bytes, ssz_offset_table_position(index)));
    let stop = if next_index < items.count then {
            ByteQuantity(ssz_u32_at(items.bytes, ssz_offset_table_position(next_index)))
        } else {
            items.bytes.len
        };
    if start <= stop & stop <= items.bytes.len then {
        sub_slice(items.bytes, start, stop - start)
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

function ssz_fixed_list_at(items : SszListRef, index : item_index, item_size : byte_length) -> ByteSlice = {
    if items.count <= index then {
        throw InvalidBlock(InvalidConfig)
    };
    let width = item_size;
    let ByteQuantity(width_value) = width;
    let offset_value = index * width_value;
    if BYTE_QUANTITY_MAX < offset_value then {
        throw InvalidBlock(InvalidConfig)
    };
    let offset : source_pointer = ByteQuantity(offset_value);
    if items.bytes.len < offset then {
        throw InvalidBlock(InvalidConfig)
    };
    if items.bytes.len - offset < width then {
        throw InvalidBlock(InvalidConfig)
    };
    sub_slice(items.bytes, offset, width)
}

function ssz_fixed_list_pop(items : SszListRef, item_size : byte_length) -> (ByteSlice, SszListRef) = {
    let width = item_size;
    if (items.count == 0) | items.bytes.len < width then {
        throw InvalidBlock(InvalidConfig)
    };
    let remaining = items.bytes.len - width;
    let item = sub_slice(items.bytes, BYTE_ZERO, width);
    let rest : SszListRef = struct {
            bytes = sub_slice(items.bytes, width, remaining),
            count = protocol_quantity_decrement(items.count),
        };
    (item, rest)
}

/* Every variable region is resolved once before decoding. Consumers receive
 * explicit source spans instead of re-reading nested SSZ offset tables. */
struct StatelessInputRef = {
    new_payload_request : ByteSlice,
    execution_payload : ByteSlice,
    versioned_hashes : ByteSlice,
    deposits : ByteSlice,
    withdrawal_requests : ByteSlice,
    consolidation_requests : ByteSlice,
    extra_data : ByteSlice,
    transactions : SszListRef,
    withdrawals : SszListRef,
    block_access_list : ByteSlice,
    witness_state : SszListRef,
    witness_codes : SszListRef,
    witness_headers : SszListRef,
    chain_config : ByteSlice,
    public_keys : ByteSlice,
}

function ssz_checked_offset(
    base : source_pointer,
    relative : protocol_quantity,
    limit : source_pointer,
) -> (
    source_pointer
) = {
    let ByteQuantity(base_value) = base;
    let resolved_value = base_value + relative;
    if BYTE_QUANTITY_MAX < resolved_value then {
        throw InvalidBlock(InvalidConfig)
    };
    let resolved : source_pointer = ByteQuantity(resolved_value);
    if resolved > limit then {
        throw InvalidBlock(InvalidConfig)
    };
    resolved
}

function ssz_span(input : ByteSlice, start : source_pointer, stop : source_pointer) -> ByteSlice = {
    if start > stop | stop > input.len then {
        throw InvalidBlock(InvalidConfig)
    };
    sub_slice(input, start, stop - start)
}

function ssz_variable_list_ref(input : ByteSlice, start : source_pointer, stop : source_pointer) -> SszListRef = {
    let bytes = ssz_span(input, start, stop);
    if start == stop then {
        struct { bytes = bytes, count = 0 }
    } else {
        let span = stop - start;
        if span < SSZ_OFF_BYTES then {
            throw InvalidBlock(InvalidConfig)
        };
        let first_offset = ssz_u32(input, start);
        let count = protocol_quantity_quotient(first_offset, 4);
        let first = ByteQuantity(first_offset);
        if (tmod_int(first_offset, 4) != 0) | (count == 0) | first > span then {
            throw InvalidBlock(InvalidConfig)
        };
        struct { bytes = bytes, count = count }
    }
}

function ssz_fixed_list_ref(
    input : ByteSlice,
    start : source_pointer,
    stop : source_pointer,
    item_size : byte_length,
) -> (
    SszListRef
) = {
    let bytes = ssz_span(input, start, stop);
    if item_size == BYTE_ZERO then {
        throw InvalidBlock(InvalidConfig)
    };
    let span = stop - start;
    let count = span / item_size;
    if span != count * item_size then {
        throw InvalidBlock(InvalidConfig)
    };
    let ByteQuantity(count_value) = count;
    if 2 ^ 64 - 1 < count_value then {
        throw InvalidBlock(InvalidConfig)
    };
    struct { bytes = bytes, count = count_value }
}

function decode_stateless_input_ref(input : ByteSlice) -> StatelessInputRef = {
    let input_end = input.len;
    if input_end < STATELESS_INPUT_FIXED_LENGTH then {
        throw InvalidBlock(InvalidConfig)
    };
    if (slice_byte(input, BYTE_ZERO) != 0x00) | (slice_byte(input, BYTE_ONE) != 0x01) then {
        throw InvalidBlock(InvalidConfig)
    };

    let npr_off = ssz_u32(input, SSZ_BODY + IN_NPR_OFF);
    let witness_off = ssz_u32(input, SSZ_BODY + IN_WITNESS_OFF);
    let cc_off = ssz_u32(input, SSZ_BODY + IN_CHAIN_CONFIG_OFF);
    let public_keys_off = ssz_u32(input, SSZ_BODY + IN_PUBLIC_KEYS_OFF);
    if (npr_off != 16) | (witness_off < npr_off) | (cc_off < witness_off) | (public_keys_off < cc_off) then {
        throw InvalidBlock(InvalidConfig)
    };

    let npr = ssz_checked_offset(SSZ_BODY, npr_off, input_end);
    let witness = ssz_checked_offset(SSZ_BODY, witness_off, input_end);
    let cc = ssz_checked_offset(SSZ_BODY, cc_off, input_end);
    let public_keys = ssz_checked_offset(SSZ_BODY, public_keys_off, input_end);
    let public_key_bytes = input_end - public_keys;
    let public_key_count = public_key_bytes / PUBLIC_KEY_LENGTH;
    if public_key_bytes != public_key_count * PUBLIC_KEY_LENGTH then {
        throw InvalidBlock(InvalidConfig)
    };

    let npr_fixed_end = ssz_checked_offset(npr, 44, witness);
    let payload = ssz_checked_offset(npr, ssz_u32(input, npr + NPR_PAYLOAD_OFF), witness);
    let versioned_hashes = ssz_checked_offset(npr, ssz_u32(input, npr + NPR_VHASHES_OFF), witness);
    let requests = ssz_checked_offset(npr, ssz_u32(input, npr + NPR_REQUESTS_OFF), witness);
    if (npr_fixed_end != payload) | (payload > versioned_hashes) | (versioned_hashes > requests) then {
        throw InvalidBlock(InvalidConfig)
    };

    let requests_fixed_end = ssz_checked_offset(requests, 12, witness);
    let deposits = ssz_checked_offset(requests, ssz_u32(input, requests + REQ_DEPOSITS_OFF), witness);
    let withdrawal_requests = ssz_checked_offset(requests, ssz_u32(input, requests + REQ_WITHDRAWALS_OFF), witness);
    let consolidation_requests = ssz_checked_offset(
        requests,
        ssz_u32(input, requests + REQ_CONSOLIDATIONS_OFF),
        witness,
    );
    if   (requests_fixed_end != deposits)
       | (deposits > withdrawal_requests)
       | (withdrawal_requests > consolidation_requests) then {
        throw InvalidBlock(InvalidConfig)
    };

    let payload_fixed_end = ssz_checked_offset(payload, 540, versioned_hashes);
    let extra_data = ssz_checked_offset(payload, ssz_u32(input, payload + PL_EXTRA_OFF), versioned_hashes);
    let transactions = ssz_checked_offset(payload, ssz_u32(input, payload + PL_TXS_OFF), versioned_hashes);
    let withdrawals = ssz_checked_offset(payload, ssz_u32(input, payload + PL_WDS_OFF), versioned_hashes);
    let block_access_list = ssz_checked_offset(payload, ssz_u32(input, payload + PL_BAL_OFF), versioned_hashes);
    if   (payload_fixed_end != extra_data)
       | (extra_data > transactions)
       | (transactions > withdrawals)
       | (withdrawals > block_access_list) then {
        throw InvalidBlock(InvalidConfig)
    };

    let witness_fixed_end = ssz_checked_offset(witness, 12, cc);
    let witness_state = ssz_checked_offset(witness, ssz_u32(input, witness + WIT_STATE_OFF), cc);
    let witness_codes = ssz_checked_offset(witness, ssz_u32(input, witness + WIT_CODES_OFF), cc);
    let witness_headers = ssz_checked_offset(witness, ssz_u32(input, witness + WIT_HEADERS_OFF), cc);
    if (witness_fixed_end != witness_state) | (witness_state > witness_codes) | (witness_codes > witness_headers) then {
        throw InvalidBlock(InvalidConfig)
    };

    struct {
        new_payload_request = ssz_span(input, npr, witness),
        execution_payload = ssz_span(input, payload, versioned_hashes),
        versioned_hashes = ssz_span(input, versioned_hashes, requests),
        deposits = ssz_span(input, deposits, withdrawal_requests),
        withdrawal_requests = ssz_span(input, withdrawal_requests, consolidation_requests),
        consolidation_requests = ssz_span(input, consolidation_requests, witness),
        extra_data = ssz_span(input, extra_data, transactions),
        transactions = ssz_variable_list_ref(input, transactions, withdrawals),
        withdrawals = ssz_fixed_list_ref(input, withdrawals, block_access_list, WD_SIZE),
        block_access_list = ssz_span(input, block_access_list, versioned_hashes),
        witness_state = ssz_variable_list_ref(input, witness_state, witness_codes),
        witness_codes = ssz_variable_list_ref(input, witness_codes, witness_headers),
        witness_headers = ssz_variable_list_ref(input, witness_headers, cc),
        chain_config = ssz_span(input, cc, public_keys),
        public_keys = ssz_span(input, public_keys, input_end),
    }
}

function sha256_request_digest(request_type : byte, s : ByteSlice) -> bits(256) =
    sha256([|bytes_list([|request_type|], BYTE_ONE), BytesSlice(s)|])

/* Index every witness node/code directly from its SSZ list reference. */
function index_witness_nodes_cursor(cursor : SszListCursor) -> unit =
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

function index_witness_nodes(nodes : SszListRef) -> unit =
    index_witness_nodes_cursor(ssz_list_cursor(nodes))

function index_witness_codes_cursor(cursor : SszListCursor) -> unit =
    if ssz_list_cursor_empty(cursor) then {
        ()
    } else {
        let (code, next) = ssz_list_pop(cursor);
        let _ = code_db_insert(code);
        index_witness_codes_cursor(next)
    }

/* Proof-only: one collection cursor position is consumed per recursive call. */
termination_measure index_witness_codes_cursor(cursor) =
    cursor.items.count - cursor.index

function index_witness_codes(codes : SszListRef) -> unit =
    index_witness_codes_cursor(ssz_list_cursor(codes))

struct ParentHeaderFields = {
    parent_hash : hash,
    state_root : hash,
    base_fee : word,
    blob_gas_used : blob_gas,
    excess_blob_gas : blob_gas,
    have_parent : bool,
    have_state : bool,
    have_base_fee : bool,
    have_blob_gas : bool,
    have_excess_blob_gas : bool,
}

let EMPTY_PARENT_HEADER_FIELDS : ParentHeaderFields = struct {
        parent_hash = ZERO_WORD,
        state_root = ZERO_WORD,
        base_fee = ZERO_WORD,
        blob_gas_used = 0,
        excess_blob_gas = 0,
        have_parent = false,
        have_state = false,
        have_base_fee = false,
        have_blob_gas = false,
        have_excess_blob_gas = false,
    }

/* The unconsumed RLP content span strictly decreases on every call. */
function decode_parent_header_fields(
    cursor : RlpCursor,
    field_index : item_index,
    fields : ParentHeaderFields,
) -> (
    ParentHeaderFields
) =
    if rlp_cursor_empty(cursor) then {
        fields
    } else {
        let (field, next) = rlp_cursor_pop(cursor);
        var decoded = fields;
        if field_index == 0 then {
            decoded.parent_hash = rlp_ref_word(field);
            decoded.have_parent = true
        } else {
            if field_index == 3 then {
                decoded.state_root = rlp_ref_word(field);
                decoded.have_state = true
            } else {
                if field_index == 15 then {
                    decoded.base_fee = rlp_ref_uint_word(field);
                    decoded.have_base_fee = true
                } else {
                    if field_index == 17 then {
                        decoded.blob_gas_used = rlp_ref_uint(field);
                        decoded.have_blob_gas = true
                    } else {
                        if field_index == 18 then {
                            decoded.excess_blob_gas = rlp_ref_uint(field);
                            decoded.have_excess_blob_gas = true
                        }
                    }
                }
            }
        };
        decode_parent_header_fields(next, protocol_quantity_increment(field_index), decoded)
    }
termination_measure decode_parent_header_fields(cursor, field_index, fields) =
    {
        let ByteQuantity(stop) = cursor.stop;
        let ByteQuantity(current) = cursor.current;
        stop - current
    }

struct WitnessHeaderIndex = {
    cursor : SszListCursor,
    previous_hash : hash,
    valid : bool,
    parent_state_root : hash,
    parent_base_fee_per_gas : word,
    parent_blob_gas_used : blob_gas,
    parent_excess_blob_gas : blob_gas,
    parent_fields_valid : bool,
}

function index_witness_header_cursor(state : WitnessHeaderIndex) -> WitnessHeaderIndex =
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
            if fields.valid then {
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
                        & ((k_fork < Cancun) | decoded.have_base_fee)
                        & ((k_fork < Cancun) | (decoded.have_blob_gas == decoded.have_excess_blob_gas))
                }
            } else {
                if index != 0 then {
                    result.valid = false
                }
            }
        };
        let current_hash = keccak256(header);
        result.previous_hash = current_hash;
        let distance = sub_nat(state.cursor.items.count, next.index);
        if distance < 256 then {
            let ancestor : ancestor_index = distance;
            ancestor_hash_write(ancestor, current_hash)
        };
        index_witness_header_cursor(result)
    }

/* Proof-only: one collection cursor position is consumed per recursive call. */
termination_measure index_witness_header_cursor(state) =
    state.cursor.items.count - state.cursor.index

/* Parse the witness header chain once. Each header is hashed into the
 * distance-indexed ancestor table, adjacent parent hashes are checked, and
 * the newest header's execution context is decoded while its fields are
 * consumed head-to-tail. */
function index_witness_headers(headers : SszListRef) -> WitnessContext = {
    let indexed = index_witness_header_cursor(
        struct {
            cursor = ssz_list_cursor(headers),
            previous_hash = ZERO_WORD,
            valid = headers.count != 0,
            parent_state_root = ZERO_WORD,
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

function decode_block_header_ssz(input_ref : StatelessInputRef) -> BlockHeader = {
    let payload = input_ref.execution_payload;
    let gas_limit_value = ssz_uint(payload, PL_GAS_LIMIT);
    let gas_used_value = ssz_uint(payload, PL_GAS_USED);
    if GAS_MAX_VALUE < gas_limit_value then {
        throw InvalidBlock(InvalidGasLimit)
    };
    if GAS_MAX_VALUE < gas_used_value then {
        throw InvalidBlock(InvalidGasUsed)
    };
    struct {
        number = ssz_uint(payload, PL_BLOCK_NUMBER),
        timestamp = ssz_uint(payload, PL_TIMESTAMP),
        gas_limit = nat_to_gas(gas_limit_value),
        gas_used = nat_to_gas(gas_used_value),
        prev_randao = ssz_bytes32(payload, PL_PREV_RANDAO),
        base_fee = ssz_u256(payload, PL_BASE_FEE),
        blob_gas_used = ssz_uint(payload, PL_BLOB_GAS_USED),
        excess_blob_gas = ssz_uint(payload, PL_EXCESS_BLOB_GAS),
        state_root = ssz_bytes32(payload, PL_STATE_ROOT),
        receipts_root = ssz_bytes32(payload, PL_RECEIPTS_ROOT),
        logs_bloom = ssz_logs_bloom(payload, PL_LOGS_BLOOM),
        fee_recipient = ssz_addr(payload, PL_FEE_RECIPIENT),
        parent_hash = ssz_bytes32(payload, BYTE_ZERO),
        parent_beacon_block_root = ssz_bytes32(input_ref.new_payload_request, NPR_BEACON_ROOT),
        slot_number = ssz_uint(payload, PL_SLOT_NUMBER),
        extra_data = input_ref.extra_data,
    }
}

function decode_withdrawal(withdrawal : ByteSlice) -> Withdrawal = {
    struct {
        index = ssz_uint(withdrawal, WD_INDEX),
        validator_index = ssz_uint(withdrawal, WD_VALIDATOR_INDEX),
        address = ssz_addr(withdrawal, WD_ADDRESS),
        amount = ssz_uint(withdrawal, WD_AMOUNT),
    }
}

/* Decode the SszChainConfig at cc: chain_id (64-bit unsigned @0) and the active
 * SszForkConfig (offset @8: fork index unsigned @0, SszForkActivation offset @8,
 * blob_schedule offset @12). The activation point (optional block_number /
 * timestamp, one-element unsigned lists) must be REACHED by this payload (EELS
 * validate_chain_config -> _is_activation_active): at least one bound set,
 * none exceeding the payload's -- a future activation invalidates the block. */
function decode_chain_config(cc : ByteSlice, number : block_number, timestamp : block_timestamp) -> ChainConfig = {
    let cc_end = cc.len;
    if cc_end < CHAIN_CONFIG_HEADER_LENGTH then {
        throw InvalidBlock(InvalidConfig)
    };
    let f_offset = ssz_u32(cc, CC_ACTIVE_FORK_OFF);
    if (f_offset != 12) | cc_end < CHAIN_CONFIG_MIN_LENGTH then {
        throw InvalidBlock(InvalidConfig)
    };
    let f : source_pointer = ByteQuantity(f_offset);
    let fidx = ssz_uint(cc, f + FC_FORK);
    let a = ssz_checked_offset(f, ssz_u32(cc, f + FC_ACTIVATION_OFF), cc_end);
    let bs_start = ssz_checked_offset(f, ssz_u32(cc, f + FC_BLOBSCHED_OFF), cc_end);
    if a > bs_start then {
        throw InvalidBlock(InvalidConfig)
    };
    let activation_fixed_end = ssz_checked_offset(a, 8, bs_start);
    let bn_start = ssz_checked_offset(a, ssz_u32(cc, a + FA_BLOCK_NUMBER_OFF), bs_start);
    let ts_start = ssz_checked_offset(a, ssz_u32(cc, a + FA_TIMESTAMP_OFF), bs_start);
    if (activation_fixed_end != bn_start) | (bn_start > ts_start) then {
        throw InvalidBlock(InvalidConfig)
    };
    let bn_present = SSZ_UINT_BYTES <= ts_start - bn_start;
    let ts_present = SSZ_UINT_BYTES <= bs_start - ts_start;
    let bn_ok = not_bool(bn_present) | (ssz_uint(cc, bn_start) <= number);
    let ts_ok = not_bool(ts_present) | (ssz_uint(cc, ts_start) <= timestamp);
    let have_schedule = bs_start + BLOB_SCHEDULE_LENGTH == cc_end;
    let blob_schedule_shape_valid = (bs_start == cc_end) | have_schedule;
    let blob_schedule : option(BlobSchedule) = if have_schedule then {
            Some(
                struct {
                    target = ssz_uint(cc, bs_start),
                    max = ssz_uint(cc, bs_start + SSZ_UINT_BYTES),
                    base_fee_update_fraction = ssz_uint(cc, bs_start + SSZ_UINT_BYTES + SSZ_UINT_BYTES),
                },
            )
        } else {
            None()
        };
    struct {
        chain_id = ssz_uint(cc, CC_CHAIN_ID),
        fork_index = fidx,
        fork = fork_of_protocol_index(fidx),
        activation_active = (bn_present | ts_present) & bn_ok & ts_ok,
        blob_schedule = blob_schedule,
        blob_schedule_shape_valid = blob_schedule_shape_valid,
    }
}

/* Decode the semantic payload structure without touching an encoded
 * transaction or withdrawal element. */
function decode_stateless_input(input_ref : StatelessInputRef) -> StatelessInput = {
    let payload = input_ref.execution_payload;
    let header = decode_block_header_ssz(input_ref);
    let chain_config = decode_chain_config(input_ref.chain_config, header.number, header.timestamp);
    k_set_header(header);
    k_chain_id = chain_config.chain_id;
    k_blob_schedule = match chain_config.blob_schedule {
        Some(schedule) => schedule,
        None() => struct { target = 0, max = 0, base_fee_update_fraction = 1 },
    };
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

/* Index the source-backed witness and install its authenticated parent-state
 * anchor. No witness list is materialized. */
function index_execution_witness(input_ref : StatelessInputRef) -> WitnessContext = {
    nodedb_reset();
    index_witness_nodes(input_ref.witness_state);
    index_witness_codes(input_ref.witness_codes);
    let witness = index_witness_headers(input_ref.witness_headers);
    k_parent_state_root = witness.parent_state_root;
    witness
}

function decode_transaction(transaction : ByteSlice, public_key : ByteSlice) -> Transaction =
    rlp_decode_tx(transaction, public_key, k_fork)
