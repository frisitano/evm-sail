
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

/* Enter a top-level protocol system-call frame. The caller has already made a
 * fresh memory frame and, for a word input, frozen the parent-memory span that
 * `input` references. */
/*! Enters a top-level protocol system-call frame. The caller has already
    made a fresh memory frame and, for a word input, frozen the
    parent-memory span that `input` references. */
function enter_system_call_frame(tgt : address, input : ByteSlice) -> journal_checkpoint = {
    let checkpoint = k_state_checkpoint();
    pc = 0;
    call_depth = 0;
    gas_remaining = SYSTEM_CALL_GAS_LIMIT;
    state_gas_remaining = GAS_ZERO;
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    frame_refund = GAS_REFUND_ZERO;
    stack_reset();
    returndata_clear();
    frame_status = Running();
    calldata = input;
    message =
        struct {
            caller = SYSTEM_ADDRESS,
            address = tgt,
            code_address = tgt,
            value = ZERO_WORD,
            state_gas_reservoir = GAS_ZERO,
            is_static = false,
            depth = 0,
        };
    frame_code = code_db_resolve(k_code_key(tgt));
    checkpoint
}

/*! Issues one unchecked block-start system call: a 30M-gas frame from
    `SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
    code, and its output is discarded. */
function system_call(tgt : address, input : hash) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        /* Freeze the word in a parent memory frame. The system call executes
         * above that frame, so its own memory writes cannot mutate calldata. */
        memory_reset();
        let input_range : MemoryRange = memory_range(0, SYSTEM_CALL_INPUT_LENGTH);
        let _ = memory_expand_to(input_range.len);
        mem_store_word(input_range.off, hash_to_word(input));
        let input_slice = memory_byte_slice(input_range.off, input_range.len);
        let parent_memory = memory_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, input_slice);
        let _ = interpret();
        memory_frame_leave(parent_memory);
        if not_bool(frame_succeeded()) then {
            k_revert(checkpoint)
        };
        k_tx_merge() /* system call is a top-level boundary: merge its storage */
    }
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
function system_call_checked(tgt : address) -> ByteSlice = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        throw InvalidBlock(ExecutionInvalid)
    } else {
        memory_reset();
        let parent_memory = memory_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, EMPTY_SLICE);
        let output = interpret();
        if frame_succeeded() then {
            let start = scratch_begin();
            scratch_push_slice(output);
            let result = scratch_finish(start);
            memory_frame_leave(parent_memory);
            k_tx_merge(); /* system call is a top-level boundary: merge its storage */
            result
        } else {
            memory_frame_leave(parent_memory);
            k_revert(checkpoint);
            k_tx_merge();
            throw InvalidBlock(ExecutionInvalid)
        }
    }
}

/*! Whether a log is a `DepositEvent` from the deposit contract
    (EIP-6110). */
function deposit_log_matches(log : LogEntry) -> bool =
    if log.address != DEPOSIT_CONTRACT_ADDR then {
        false
    } else {
        match log.topics {
            topic :: _ => topic == DEPOSIT_EVENT_TOPIC,
            [||] => false,
        }
    }

/*! Strips the ABI framing from one `DepositEvent` and appends the
    consensus-layer deposit record's fields directly to the requests
    scratch (EIP-6110). */
function append_deposit_request(data : ByteSlice) -> unit = {
    if data.len != DEPOSIT_EVENT_DATA_LENGTH then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    if   (slice_load(data, DEPOSIT_PUBKEY_HEAD) != U256(160))
       | (slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD) != U256(256))
       | (slice_load(data, DEPOSIT_AMOUNT_HEAD) != U256(320))
       | (slice_load(data, DEPOSIT_SIGNATURE_HEAD) != U256(384))
       | (slice_load(data, DEPOSIT_INDEX_HEAD) != U256(512))
       | (slice_load(data, DEPOSIT_PUBKEY_LENGTH_WORD) != U256(48))
       | (slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD) != U256(32))
       | (slice_load(data, DEPOSIT_AMOUNT_LENGTH_WORD) != U256(8))
       | (slice_load(data, DEPOSIT_SIGNATURE_LENGTH_WORD) != U256(96))
       | (slice_load(data, DEPOSIT_INDEX_LENGTH_WORD) != U256(8)) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    scratch_push_slice(sub_slice(data, DEPOSIT_PUBKEY_DATA, DEPOSIT_PUBKEY_LENGTH));
    scratch_push_slice(sub_slice(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH));
    scratch_push_slice(sub_slice(data, DEPOSIT_AMOUNT_DATA, DEPOSIT_AMOUNT_LENGTH));
    scratch_push_slice(sub_slice(data, DEPOSIT_SIGNATURE_DATA, DEPOSIT_SIGNATURE_LENGTH));
    scratch_push_slice(sub_slice(data, DEPOSIT_INDEX_DATA, DEPOSIT_INDEX_LENGTH))
}

/*! Appends every matching deposit log in emission order. */
function append_deposit_logs(logs : list(LogEntry)) -> unit =
    match logs {
        [||] => (),
        log :: rest => {
            if deposit_log_matches(log) then {
                append_deposit_request(log.data)
            };
            append_deposit_logs(rest)
        },
    }

/* Request types are processed in ascending order, stopping immediately when
 * a checked system call is missing or fails. */
/*! Collects the EIP-7685 execution requests at block end in request-type
    order: deposits from logs (EIP-6110), withdrawal (EIP-7002),
    consolidation (EIP-7251), and, from Amsterdam, builder deposit and
    builder exit (EIP-8282). */
function collect_execution_requests(deposits : ByteSlice) -> ExecutionRequests = {
    let withdrawals = system_call_checked(WITHDRAWAL_REQUEST_ADDR);
    let consolidations = system_call_checked(CONSOLIDATION_REQUEST_ADDR);
    let builder_deposits : ByteSlice = if k_fork >= Amsterdam then {
            system_call_checked(BUILDER_DEPOSIT_REQUEST_ADDR)
        } else {
            EMPTY_SLICE
        };
    let builder_exits : ByteSlice = if k_fork >= Amsterdam then {
            system_call_checked(BUILDER_EXIT_REQUEST_ADDR)
        } else {
            EMPTY_SLICE
        };
    struct {
        deposits = deposits,
        withdrawals = withdrawals,
        consolidations = consolidations,
        builder_deposits = builder_deposits,
        builder_exits = builder_exits,
    }
}
