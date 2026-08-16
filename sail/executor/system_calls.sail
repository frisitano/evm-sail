
/*md
# Protocol system calls

The block-level system calls the protocol issues around the transactions:
the EIP-4788 / EIP-2935 block-start writes (unchecked — skipped if the
contract is absent) and the EIP-7002 / EIP-7251 / EIP-8282 block-end
request dequeues (checked — absence or failure invalidates the block). Each is a
30M-gas frame from `SYSTEM_ADDRESS` whose committed storage writes shape
the post-state root.

## Constants

The deposit offsets describe the ABI-encoded `DepositEvent` payload, while
`SYSTEM_CALL_INPUT_LENGTH` is the fixed input length of each block-start
system call.
*/
/*md
## The block-start writes

Before the transactions, the protocol issues unchecked system calls from
`SYSTEM_ADDRESS` to the beacon-roots (EIP-4788) and block-hash-history
(EIP-2935) contracts, writing the parent beacon-block root and the parent
block hash into their ring buffers. Each takes a 32-byte input and 30M
gas, and bumps no nonce, charges no fee, and counts toward no block gas;
it is skipped if the contract has no code.
*/
let SYSTEM_CALL_INPUT_LENGTH : int(32) = WORD_BYTE_LENGTH

let DEPOSIT_EVENT_DATA_LENGTH : int(576) = 576
let DEPOSIT_PUBKEY_HEAD : int(0) = 0
let DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD : int(32) = 32
let DEPOSIT_AMOUNT_HEAD : int(64) = 64
let DEPOSIT_SIGNATURE_HEAD : int(96) = 96
let DEPOSIT_INDEX_HEAD : int(128) = 128
let DEPOSIT_PUBKEY_LENGTH_WORD : int(160) = 160
let DEPOSIT_PUBKEY_DATA : int(192) = 192
let DEPOSIT_PUBKEY_LENGTH : int(48) = 48
let DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD : int(256) = 256
let DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA : int(288) = 288
let DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH : int(32) = WORD_BYTE_LENGTH
let DEPOSIT_AMOUNT_LENGTH_WORD : int(320) = 320
let DEPOSIT_AMOUNT_DATA : int(352) = 352
let DEPOSIT_AMOUNT_LENGTH : int(8) = EIGHT_BYTE_LENGTH
let DEPOSIT_SIGNATURE_LENGTH_WORD : int(384) = 384
let DEPOSIT_SIGNATURE_DATA : int(416) = 416
let DEPOSIT_SIGNATURE_LENGTH : int(96) = 96
let DEPOSIT_INDEX_LENGTH_WORD : int(512) = 512
let DEPOSIT_INDEX_DATA : int(544) = 544
let DEPOSIT_INDEX_LENGTH : int(8) = EIGHT_BYTE_LENGTH
let DEPOSIT_REQUEST_LENGTH : int(192) = 192
let DEPOSIT_REQUEST_PUBKEY : int(0) = 0
let DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS : int(48) = 48
let DEPOSIT_REQUEST_AMOUNT : int(80) = 80
let DEPOSIT_REQUEST_SIGNATURE : int(88) = 88
let DEPOSIT_REQUEST_INDEX : int(184) = 184

/* Enter a top-level protocol system-call frame. The caller has already
 * resolved `code`, made a fresh memory frame and, for a word input, frozen
 * the parent-memory span that `input` references. */
/*! Runs a top-level protocol system-call frame. The caller has already
    resolved `code`, made a fresh memory frame and, for a word input, frozen
    the parent-memory span that `input` references. */
function run_system_call_frame(
    tgt : address,
    code : Code,
    input : CalldataSlice,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    k_journal_checkpoint();
    let stack_top = stack_reset();
    interpret(
        SYSTEM_CALL_GAS_LIMIT,
        STATE_GAS_ZERO,
        STATE_GAS_SPILL_ZERO,
        GAS_REFUND_ZERO,
        stack_top,
        EMPTY_EVM_MEMORY_SLICE,
        SYSTEM_ADDRESS,
        tgt,
        tgt,
        ZERO_WORD,
        STATE_GAS_ZERO,
        false,
        0,
        code,
        input,
    )
}

/*! Issues one unchecked block-start system call: a 30M-gas frame from
    `SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
    code, and its output is discarded. */
function system_call(tgt : address, input : hash) -> unit = {
    let code_hash = k_code_key(tgt);
    if code_hash == KECCAK_EMPTY then {
        return () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    };
    let code = code_db_resolve(code_hash);

    /* Freeze the word in a parent memory frame. The system call executes
     * above that frame, so its own memory writes cannot mutate calldata. */
    let initial_memory = memory_reset();
    let input_range : MemoryRange = memory_range(0, SYSTEM_CALL_INPUT_LENGTH);
    let (expanded_memory, _) = memory_expand_to(initial_memory, input_range.len);
    let input_word = hash_to_word(input);
    mem_store_word(input_range.off, input_word);
    let (accessed_memory, input_slice) = active_memory_slice(expanded_memory, input_range.off, input_range.len);
    let _ = memory_frame_enter();
    let memory_input = evm_memory_slice(input_slice.bytes, input_slice.len);
    let frame_input = MemoryCalldata(memory_input);
    let (_, _, _, _, status, _) = run_system_call_frame(tgt, code, frame_input);
    let _ = memory_frame_leave(accessed_memory);
    let succeeded = frame_succeeded(status);
    let failed = not_bool(succeeded);
    if failed then {
        k_journal_revert()
    } else {
        k_journal_commit()
    };
    k_tx_merge() /* system call is a top-level boundary: merge its storage */
}

/* ---- EIP-7002 / EIP-7251 / EIP-8282 block-END system calls ---
 * After the transactions and withdrawals, the protocol dequeues the queued
 * withdrawal (EIP-7002), consolidation (EIP-7251), and builder (EIP-8282)
 * requests by a CHECKED system call (SYSTEM_ADDRESS, EMPTY calldata, 30M gas)
 * to each predeploy. The
 * dequeue clears the request queue in the contract's storage, shaping the
 * post-state root. "Checked" means: a missing contract (no code) OR a failed
 * call makes the whole block INVALID (returns false here). */


/*! Issues one checked block-end system call (EIP-7002/EIP-7251/EIP-8282): the
    target must exist and the call must succeed. Missing code or frame failure
    throws immediately; a successful call returns the dequeued requests. */
function system_call_checked(tgt : address) -> ScratchSlice = {
    let code_hash = k_code_key(tgt);
    if code_hash == KECCAK_EMPTY then {
        fatal_error(ExecutionInvalid)
    } else {
        let code = code_db_resolve(code_hash);
        let _ = memory_reset();
        let parent_memory = memory_frame_enter();
        let (_, _, _, _, status, output) = run_system_call_frame(tgt, code, EMPTY_CALLDATA);
        let succeeded = frame_succeeded(status);
        if succeeded then {
            let start = scratch_reserve(output.len);
            scratch_push_slice(output);
            let result = scratch_finish(start);
            let _ = memory_frame_leave(parent_memory);
            k_journal_commit();
            k_tx_merge(); /* system call is a top-level boundary: merge its storage */
            result
        } else {
            let _ = memory_frame_leave(parent_memory);
            k_journal_revert();
            k_tx_merge();
            fatal_error(ExecutionInvalid)
        }
    }
}

/*! Whether a retained log is a `DepositEvent` from the deposit contract
    (EIP-6110). */
function deposit_log_matches(index : log_store_index) -> bool =
    let address = log_address(index) in
    if address != DEPOSIT_CONTRACT_ADDR then {
        false
    } else {
        let topic_count = log_topics_count(index);
        if topic_count == 0 then {
            false
        } else {
            let first_topic = log_topic(index, 0);
            first_topic == DEPOSIT_EVENT_TOPIC
        }
    }

/*! Validates one `DepositEvent` ABI payload against the next authenticated
    consensus-layer deposit request and returns the unconsumed request
    suffix (EIP-6110). */
function authenticate_deposit_request(data : LogDataSlice, expected : StatelessInputSlice) -> StatelessInputSlice = {
    let data : LogDataSliceLength(576) =
        if data.len == DEPOSIT_EVENT_DATA_LENGTH then data else fatal_error(InvalidExecutionRequests);
    let pubkey_head = slice_load(data, DEPOSIT_PUBKEY_HEAD);
    let withdrawal_credentials_head = slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD);
    let amount_head = slice_load(data, DEPOSIT_AMOUNT_HEAD);
    let signature_head = slice_load(data, DEPOSIT_SIGNATURE_HEAD);
    let index_head = slice_load(data, DEPOSIT_INDEX_HEAD);
    let pubkey_length = slice_load(data, DEPOSIT_PUBKEY_LENGTH_WORD);
    let withdrawal_credentials_length = slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD);
    let amount_length = slice_load(data, DEPOSIT_AMOUNT_LENGTH_WORD);
    let signature_length = slice_load(data, DEPOSIT_SIGNATURE_LENGTH_WORD);
    let index_length = slice_load(data, DEPOSIT_INDEX_LENGTH_WORD);
    let expected_pubkey_head : word = u256(160);
    let expected_withdrawal_credentials_head : word = u256(256);
    let expected_amount_head : word = u256(320);
    let expected_signature_head : word = u256(384);
    let expected_index_head : word = u256(512);
    let expected_pubkey_length : word = u256(48);
    let expected_withdrawal_credentials_length : word = u256(32);
    let expected_amount_length : word = u256(8);
    let expected_signature_length : word = u256(96);
    let expected_index_length : word = u256(8);
    if   (pubkey_head != expected_pubkey_head)
       | (withdrawal_credentials_head != expected_withdrawal_credentials_head)
       | (amount_head != expected_amount_head)
       | (signature_head != expected_signature_head)
       | (index_head != expected_index_head)
       | (pubkey_length != expected_pubkey_length)
       | (withdrawal_credentials_length != expected_withdrawal_credentials_length)
       | (amount_length != expected_amount_length)
       | (signature_length != expected_signature_length)
       | (index_length != expected_index_length) then {
        fatal_error(InvalidExecutionRequests)
    };
    if DEPOSIT_REQUEST_LENGTH <= expected.len then {
        let log_pubkey = sub_slice(data, DEPOSIT_PUBKEY_DATA, DEPOSIT_PUBKEY_LENGTH);
        let expected_pubkey = sub_slice(expected, DEPOSIT_REQUEST_PUBKEY, DEPOSIT_PUBKEY_LENGTH);
        let pubkey_matches = region_slices_equal(log_pubkey, expected_pubkey);
        let pubkey_mismatch = not_bool(pubkey_matches);
        let log_withdrawal_credentials = sub_slice(
            data,
            DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA,
            DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH,
        );
        let expected_withdrawal_credentials = sub_slice(
            expected,
            DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS,
            DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH,
        );
        let withdrawal_credentials_match = region_slices_equal(
            log_withdrawal_credentials,
            expected_withdrawal_credentials,
        );
        let withdrawal_credentials_mismatch = not_bool(withdrawal_credentials_match);
        let log_amount = sub_slice(data, DEPOSIT_AMOUNT_DATA, DEPOSIT_AMOUNT_LENGTH);
        let expected_amount = sub_slice(expected, DEPOSIT_REQUEST_AMOUNT, DEPOSIT_AMOUNT_LENGTH);
        let amount_matches = region_slices_equal(log_amount, expected_amount);
        let amount_mismatch = not_bool(amount_matches);
        let log_signature = sub_slice(data, DEPOSIT_SIGNATURE_DATA, DEPOSIT_SIGNATURE_LENGTH);
        let expected_signature = sub_slice(expected, DEPOSIT_REQUEST_SIGNATURE, DEPOSIT_SIGNATURE_LENGTH);
        let signature_matches = region_slices_equal(log_signature, expected_signature);
        let signature_mismatch = not_bool(signature_matches);
        let log_index = sub_slice(data, DEPOSIT_INDEX_DATA, DEPOSIT_INDEX_LENGTH);
        let expected_index = sub_slice(expected, DEPOSIT_REQUEST_INDEX, DEPOSIT_INDEX_LENGTH);
        let index_matches = region_slices_equal(log_index, expected_index);
        let index_mismatch = not_bool(index_matches);
        if pubkey_mismatch | withdrawal_credentials_mismatch | amount_mismatch | signature_mismatch | index_mismatch then {
            fatal_error(InvalidExecutionRequests)
        };
        slice_suffix(expected, DEPOSIT_REQUEST_LENGTH)
    } else {
        fatal_error(InvalidExecutionRequests)
    }
}

/*! Authenticates matching deposit logs in emission order and returns the
    unconsumed suffix of the expected request bytes. */
function authenticate_deposit_logs(logs : LogSeriesRef, expected : StatelessInputSlice) -> StatelessInputSlice = {
    var remaining : StatelessInputSlice = expected;
    var offset : log_store_index = 0;
    while offset < logs.count termination_measure(logs.count - offset) do {
        let index = log_store_index_add(logs.start, offset);
        let matches = deposit_log_matches(index);
        if matches then {
            let data = read_log_data(index);
            remaining = authenticate_deposit_request(data, remaining)
        };
        offset = log_store_index_increment(offset)
    };
    remaining
}

/* Request types are processed in ascending order, stopping immediately when
 * a checked system call is missing or fails. */
/*! Dequeues one block-end request stream and validates it byte for byte
    against the input's committed request bytes. */
function validate_request_stream(tgt : address, expected : StatelessInputSlice) -> unit = {
    let dequeued = system_call_checked(tgt);
    let matches = region_slices_equal(dequeued, expected);
    let mismatch = not_bool(matches);
    if mismatch then {
        fatal_error(InvalidExecutionRequests)
    }
}

/*! Validates the EIP-7685 execution requests at block end in request-type
    order against the input's committed request bytes: withdrawal
    (EIP-7002), consolidation (EIP-7251), and, from Amsterdam, builder
    deposit and builder exit (EIP-8282). Before Amsterdam the input must
    commit to empty builder request streams. Deposits (EIP-6110) are
    authenticated inline against the transaction receipt logs. */
function validate_execution_requests(input_ref : StatelessInputRef) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    validate_request_stream(WITHDRAWAL_REQUEST_ADDR, input_ref.withdrawal_requests);
    validate_request_stream(CONSOLIDATION_REQUEST_ADDR, input_ref.consolidation_requests);
    if profile.fork >= Amsterdam then {
        validate_request_stream(BUILDER_DEPOSIT_REQUEST_ADDR, input_ref.builder_deposit_requests);
        validate_request_stream(BUILDER_EXIT_REQUEST_ADDR, input_ref.builder_exit_requests)
    } else {
        let builder_deposit_length = region_slice_length(input_ref.builder_deposit_requests);
        let builder_exit_length = region_slice_length(input_ref.builder_exit_requests);
        if (builder_deposit_length != 0) | (builder_exit_length != 0) then {
            fatal_error(InvalidExecutionRequests)
        }
    }
}
