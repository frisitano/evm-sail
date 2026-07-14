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
let SSZ_BODY : int = 2

/* SszStatelessInput: 4 variable fields, offset table at body+{0,4,8,12}. */
let IN_NPR_OFF : int = 0
let IN_WITNESS_OFF : int = 4
let IN_CHAIN_CONFIG_OFF : int = 8
let IN_PUBLIC_KEYS_OFF : int = 12

/* SszNewPayloadRequest. */
let NPR_PAYLOAD_OFF : int = 0
let NPR_VHASHES_OFF : int = 4
let NPR_BEACON_ROOT : int = 8
let NPR_REQUESTS_OFF : int = 40

/* SszExecutionRequests. */
let REQ_DEPOSITS_OFF : int = 0
let REQ_WITHDRAWALS_OFF : int = 4
let REQ_CONSOLIDATIONS_OFF : int = 8

/* SszExecutionPayload fixed-region field offsets. */
let PL_FEE_RECIPIENT : int = 32
let PL_STATE_ROOT : int = 52
let PL_RECEIPTS_ROOT : int = 84
let PL_LOGS_BLOOM : int = 116
let PL_PREV_RANDAO : int = 372
let PL_BLOCK_NUMBER : int = 404
let PL_GAS_LIMIT : int = 412
let PL_GAS_USED : int = 420
let PL_TIMESTAMP : int = 428
let PL_EXTRA_OFF : int = 436
let PL_BASE_FEE : int = 440
let PL_BLOCK_HASH : int = 472
let PL_TXS_OFF : int = 504
let PL_WDS_OFF : int = 508
let PL_BLOB_GAS_USED : int = 512
let PL_EXCESS_BLOB_GAS : int = 520
let PL_BAL_OFF : int = 528
let PL_SLOT_NUMBER : int = 532

/* SszWithdrawal fixed element layout. */
let WD_SIZE : int = 44
let WD_INDEX : int = 0
let WD_VALIDATOR_INDEX : int = 8
let WD_ADDRESS : int = 16
let WD_AMOUNT : int = 36

/* SszChainConfig and nested fork activation layouts. */
let CC_CHAIN_ID : int = 0
let CC_ACTIVE_FORK_OFF : int = 8
let FC_FORK : int = 0
let FC_ACTIVATION_OFF : int = 8
let FC_BLOBSCHED_OFF : int = 12
let FA_BLOCK_NUMBER_OFF : int = 0
let FA_TIMESTAMP_OFF : int = 4

/* SszExecutionWitness. */
let WIT_STATE_OFF : int = 0
let WIT_CODES_OFF : int = 4
let WIT_HEADERS_OFF : int = 8

/*! A cursor at the first element of a variable-element SSZ list. */
function ssz_list_cursor(items : SszListRef) -> SszListCursor = {
    let start = unsigned(items.bytes.off);
    let current = if items.count > 0 then {
            start + ssz_u32(start)
        } else {
            start + unsigned(items.bytes.len)
        };
    struct { items = items, index = 0, current = current }
}

/*! Whether the cursor has consumed all elements. */
function ssz_list_cursor_empty(cursor : SszListCursor) -> bool = cursor.index >= cursor.items.count

/*! The next element's span, and the advanced cursor. */
function ssz_list_pop(cursor : SszListCursor) -> (ByteSlice, SszListCursor) = {
    let next_index = cursor.index + 1;
    let start = unsigned(cursor.items.bytes.off);
    let stop = start + unsigned(cursor.items.bytes.len);
    let next = if next_index < cursor.items.count then {
            start + ssz_u32(start + next_index * SSZ_OFF_BYTES)
        } else {
            stop
        };
    let item = byte_slice(cursor.items.bytes.source, cursor.current, next - cursor.current);
    (item, struct { items = cursor.items, index = next_index, current = next })
}

/*! Pops one fixed-size element from a fixed-stride list reference. */
function ssz_fixed_list_pop(items : SszListRef, item_size : int) -> (ByteSlice, SszListRef) = {
    let remaining = unsigned(items.bytes.len) - item_size;
    let item = sub_slice(items.bytes, 0, item_size);
    let rest : SszListRef = struct { bytes = sub_slice(items.bytes, item_size, remaining), count = items.count - 1 };
    (item, rest)
}

/*! Every variable region of the input, resolved once before decoding.
    Consumers receive explicit source spans instead of re-reading nested
    SSZ offset tables. */
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

/*! Resolves the input's offset tables into a
    [StatelessInputRef][type-StatelessInputRef], validating the schema id
    and every region bound; a malformed frame is `InvalidConfig`. */
function decode_stateless_input_ref() -> StatelessInputRef = {
    let input_end = unsigned(ssz_src_len());
    if input_end < 18 then {
        throw InvalidBlock(InvalidConfig)
    };
    if unsigned(ssz_src_byte(0)) * 256 + unsigned(ssz_src_byte(1)) != 1 then {
        throw InvalidBlock(InvalidConfig)
    };

    let npr_off = ssz_u32(SSZ_BODY + IN_NPR_OFF);
    let witness_off = ssz_u32(SSZ_BODY + IN_WITNESS_OFF);
    let cc_off = ssz_u32(SSZ_BODY + IN_CHAIN_CONFIG_OFF);
    let public_keys_off = ssz_u32(SSZ_BODY + IN_PUBLIC_KEYS_OFF);
    if   (npr_off != 16)
       | (npr_off > witness_off)
       | (witness_off > cc_off)
       | (cc_off > public_keys_off)
       | (SSZ_BODY + public_keys_off > input_end)
       | (emod_int(input_end - (SSZ_BODY + public_keys_off), 65) != 0) then {
        throw InvalidBlock(InvalidConfig)
    };

    let npr = SSZ_BODY + npr_off;
    let witness = SSZ_BODY + witness_off;
    let cc = SSZ_BODY + cc_off;
    let public_keys = SSZ_BODY + public_keys_off;

    let payload = npr + ssz_u32(npr + NPR_PAYLOAD_OFF);
    let versioned_hashes = npr + ssz_u32(npr + NPR_VHASHES_OFF);
    let requests = npr + ssz_u32(npr + NPR_REQUESTS_OFF);
    let deposits = requests + ssz_u32(requests + REQ_DEPOSITS_OFF);
    let withdrawal_requests = requests + ssz_u32(requests + REQ_WITHDRAWALS_OFF);
    let consolidation_requests = requests + ssz_u32(requests + REQ_CONSOLIDATIONS_OFF);

    let extra_data = payload + ssz_u32(payload + PL_EXTRA_OFF);
    let transactions = payload + ssz_u32(payload + PL_TXS_OFF);
    let withdrawals = payload + ssz_u32(payload + PL_WDS_OFF);
    let block_access_list = payload + ssz_u32(payload + PL_BAL_OFF);
    let n_transactions = if transactions == withdrawals then {
            0
        } else {
            ediv_int(ssz_u32(transactions), SSZ_OFF_BYTES)
        };
    let n_withdrawals = ediv_int(block_access_list - withdrawals, WD_SIZE);

    let witness_state = witness + ssz_u32(witness + WIT_STATE_OFF);
    let witness_codes = witness + ssz_u32(witness + WIT_CODES_OFF);
    let witness_headers = witness + ssz_u32(witness + WIT_HEADERS_OFF);
    let n_witness_state = if witness_state == witness_codes then {
            0
        } else {
            ediv_int(ssz_u32(witness_state), SSZ_OFF_BYTES)
        };
    let n_witness_codes = if witness_codes == witness_headers then {
            0
        } else {
            ediv_int(ssz_u32(witness_codes), SSZ_OFF_BYTES)
        };
    let n_witness_headers = if witness_headers == cc then {
            0
        } else {
            ediv_int(ssz_u32(witness_headers), SSZ_OFF_BYTES)
        };

    struct {
        new_payload_request = stateless_input_byte_slice(npr, witness - npr),
        execution_payload = stateless_input_byte_slice(payload, versioned_hashes - payload),
        versioned_hashes = stateless_input_byte_slice(versioned_hashes, requests - versioned_hashes),
        deposits = stateless_input_byte_slice(deposits, withdrawal_requests - deposits),
        withdrawal_requests =
            stateless_input_byte_slice(withdrawal_requests, consolidation_requests - withdrawal_requests),
        consolidation_requests = stateless_input_byte_slice(consolidation_requests, witness - consolidation_requests),
        extra_data = stateless_input_byte_slice(extra_data, transactions - extra_data),
        transactions =
            struct {
                bytes = stateless_input_byte_slice(transactions, withdrawals - transactions),
                count = n_transactions,
            },
        withdrawals =
            struct {
                bytes = stateless_input_byte_slice(withdrawals, block_access_list - withdrawals),
                count = n_withdrawals,
            },
        block_access_list = stateless_input_byte_slice(block_access_list, versioned_hashes - block_access_list),
        witness_state =
            struct {
                bytes = stateless_input_byte_slice(witness_state, witness_codes - witness_state),
                count = n_witness_state,
            },
        witness_codes =
            struct {
                bytes = stateless_input_byte_slice(witness_codes, witness_headers - witness_codes),
                count = n_witness_codes,
            },
        witness_headers =
            struct {
                bytes = stateless_input_byte_slice(witness_headers, cc - witness_headers),
                count = n_witness_headers,
            },
        chain_config = stateless_input_byte_slice(cc, public_keys - cc),
        public_keys = stateless_input_byte_slice(public_keys, input_end - public_keys),
    }
}

/*! The EIP-7685 per-type request digest:
    `sha256(request_type ‖ request_data)`. */
function sha256_request_digest(request_type : int, s : ByteSlice) -> bits(256) =
    sha256([|BytesList([|get_slice_int(8, request_type, 0)|]), BytesSlice(s)|])

/*! Indexes every witness trie node into the node-db, keyed by its
    KECCAK-256 digest, directly from the SSZ list reference. */
function index_witness_nodes(nodes : SszListRef) -> unit = {
    var cursor = ssz_list_cursor(nodes);
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let (node, next) = ssz_list_pop(cursor);
        cursor = next;
        nodedb_insert(keccak256(node), node.off, node.len)
    }
}

/*! Indexes every witness code blob into the content-addressed code
    store. */
function index_witness_codes(codes : SszListRef) -> unit = {
    var cursor = ssz_list_cursor(codes);
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let (code, next) = ssz_list_pop(cursor);
        cursor = next;
        let _ = code_db_insert(code)
    }
}

/*! Parses the witness header chain once. Each header is hashed into the
    distance-indexed ancestor table (the `BLOCKHASH` source), adjacent
    parent hashes are checked (a break is `HeaderChainBroken`), and the
    newest header's execution context — parent state root, base fee, blob
    gas — is decoded while its fields are consumed head-to-tail. */
function index_witness_headers(headers : SszListRef) -> WitnessContext = {
    let n = headers.count;
    var prev : hash = ZERO_WORD;
    var ok : bool = n >= 1;
    var parent_state : hash = ZERO_WORD;
    var parent_base_fee : int = 0;
    var parent_blob_gas : int = 0;
    var parent_excess_blob_gas : int = 0;
    var parent_fields_ok : bool = false;
    var cursor = ssz_list_cursor(headers);
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let i = cursor.index;
        let (header, next) = ssz_list_pop(cursor);
        cursor = next;
        if (i > 0) | (i == n - 1) then {
            let fields = rlp_node_refs(BytesSlice(header));
            if i > 0 then {
                match fields {
                    parent :: _ => if rlp_ref_word(parent) != prev then {
                        ok = false
                    },
                    [||] => ok = false,
                }
            };
            if i == n - 1 then {
                var rest = fields;
                var field_index : int = 0;
                var have_state : bool = false;
                var have_base_fee : bool = false;
                var have_blob_gas : bool = false;
                var have_excess_blob_gas : bool = false;
                var more : bool = true;
                while more do {
                    match rest {
                        field :: tail => {
                            if field_index == 3 then {
                                parent_state = rlp_ref_word(field);
                                have_state = true
                            } else {
                                if field_index == 15 then {
                                    parent_base_fee = rlp_ref_int(field);
                                    have_base_fee = true
                                } else {
                                    if field_index == 17 then {
                                        parent_blob_gas = rlp_ref_int(field);
                                        have_blob_gas = true
                                    } else {
                                        if field_index == 18 then {
                                            parent_excess_blob_gas = rlp_ref_int(field);
                                            have_excess_blob_gas = true
                                        }
                                    }
                                }
                            };
                            field_index = field_index + 1;
                            rest = tail
                        },
                        [||] => more = false,
                    }
                };
                parent_fields_ok =
                      have_state
                    & ((k_fork < Cancun) | have_base_fee)
                    & ((k_fork < Cancun) | (have_blob_gas == have_excess_blob_gas))
            }
        };
        prev = keccak256(header);
        let j = n - 1 - i; /* distance-1 from the block under validation */
        if j < 256 then {
            ancestor_hash_write(get_slice_int(8, j, 0), prev)
        }
    };
    k_n_headers = n;
    if not_bool(ok) then {
        throw InvalidBlock(WitnessDeficient)
    };
    if not_bool(parent_fields_ok) then {
        throw InvalidBlock(RlpDecode)
    };
    struct {
        parent_hash = prev,
        parent_state_root = parent_state,
        parent_base_fee_per_gas = parent_base_fee,
        parent_blob_gas_used = parent_blob_gas,
        parent_excess_blob_gas = parent_excess_blob_gas,
    }
}

/*! Decodes the execution-payload header fields from their fixed SSZ
    offsets. */
function decode_block_header_ssz(input_ref : StatelessInputRef) -> BlockHeader = {
    let p = unsigned(input_ref.execution_payload.off);
    let npr = unsigned(input_ref.new_payload_request.off);
    struct {
        number = ssz_u64(p + PL_BLOCK_NUMBER),
        timestamp = ssz_u64(p + PL_TIMESTAMP),
        gas_limit = ssz_u64(p + PL_GAS_LIMIT),
        gas_used = ssz_u64(p + PL_GAS_USED),
        prev_randao = ssz_bytes32(p + PL_PREV_RANDAO),
        base_fee = ssz_u256(p + PL_BASE_FEE),
        blob_gas_used = ssz_src_le(p + PL_BLOB_GAS_USED, 8),
        excess_blob_gas = ssz_src_le(p + PL_EXCESS_BLOB_GAS, 8),
        state_root = ssz_bytes32(p + PL_STATE_ROOT),
        receipts_root = ssz_bytes32(p + PL_RECEIPTS_ROOT),
        logs_bloom = ssz_logs_bloom(p + PL_LOGS_BLOOM),
        fee_recipient = ssz_addr(p + PL_FEE_RECIPIENT),
        parent_hash = ssz_bytes32(p + 0),
        parent_beacon_block_root = ssz_bytes32(npr + NPR_BEACON_ROOT),
        slot_number = ssz_src_le(p + PL_SLOT_NUMBER, 8),
        extra_data = input_ref.extra_data,
    }
}

/*! Decodes one fixed-layout SSZ withdrawal element. */
function decode_withdrawal(withdrawal : ByteSlice) -> Withdrawal = {
    let b = unsigned(withdrawal.off);
    struct {
        index = ssz_u64(b + WD_INDEX),
        validator_index = ssz_u64(b + WD_VALIDATOR_INDEX),
        address = ssz_addr(b + WD_ADDRESS),
        amount = ssz_u64(b + WD_AMOUNT),
    }
}

/*! Decodes the `SszChainConfig` at `cc`: the chain id (`u64` at 0) and
    the active `SszForkConfig` (fork index, activation, blob schedule).
    The activation point (optional block number / timestamp, `List[u64,1]`
    each) must be reached by this payload: at least one bound set, none
    exceeding the
    payload's — a future activation invalidates the block. */
function decode_chain_config(cc : int, cc_end : int, number : int, timestamp : int) -> ChainConfig = {
    let f = cc + ssz_u32(cc + CC_ACTIVE_FORK_OFF);
    let fidx = ssz_u64(f + FC_FORK);
    let a = f + ssz_u32(f + FC_ACTIVATION_OFF);
    let bs_start = f + ssz_u32(f + FC_BLOBSCHED_OFF);
    let bn_start = a + ssz_u32(a + FA_BLOCK_NUMBER_OFF);
    let ts_start = a + ssz_u32(a + FA_TIMESTAMP_OFF);
    let bn_present = (ts_start - bn_start) >= 8; /* the optional list is non-empty */
    let ts_present = (bs_start - ts_start) >= 8;
    let bn_ok = not_bool(bn_present) | (number >= ssz_u64(bn_start));
    let ts_ok = not_bool(ts_present) | (timestamp >= ssz_u64(ts_start));
    let blob_schedule_shape_valid = (bs_start == cc_end) | (bs_start + 24 == cc_end);
    let blob_schedule : option(BlobSchedule) = if bs_start + 24 == cc_end then {
            Some(
                struct {
                    target = ssz_u64(bs_start),
                    max = ssz_u64(bs_start + 8),
                    base_fee_update_fraction = ssz_u64(bs_start + 16),
                },
            )
        } else {
            None()
        };
    struct {
        chain_id = ssz_u64(cc + CC_CHAIN_ID),
        fork_index = fidx,
        fork = fork_of_protocol_index(fidx),
        activation_active = (bn_present | ts_present) & bn_ok & ts_ok,
        blob_schedule = blob_schedule,
        blob_schedule_shape_valid = blob_schedule_shape_valid,
    }
}

/*! Decodes the semantic payload structure — header, chain config, body
    references — without touching an encoded transaction or withdrawal
    element, and installs the header and chain parameters in the kernel. */
function decode_stateless_input(input_ref : StatelessInputRef) -> StatelessInput = {
    let p = unsigned(input_ref.execution_payload.off);
    let cc = unsigned(input_ref.chain_config.off);
    let header = decode_block_header_ssz(input_ref);
    k_set_header(header);
    let chain_config = decode_chain_config(
        cc,
        cc + unsigned(input_ref.chain_config.len),
        header.number,
        header.timestamp,
    );
    k_chain_id = chain_config.chain_id;
    k_blob_schedule = match chain_config.blob_schedule {
        Some(schedule) => schedule,
        None() => struct { target = 0, max = 0, base_fee_update_fraction = 1 },
    };
    struct {
        payload =
            struct {
                expected_block_hash = ssz_bytes32(p + PL_BLOCK_HASH),
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
function decode_transaction(transaction : ByteSlice, public_key : ByteSlice) -> Transaction =
    rlp_decode_tx(transaction, public_key)
