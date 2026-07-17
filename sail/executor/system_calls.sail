
/*md
# Protocol system calls

The block-level system calls the protocol issues around the transactions:
the EIP-4788 / EIP-2935 block-start writes (unchecked — skipped if the
contract is absent) and the EIP-7002 / EIP-7251 block-end request
dequeues (checked — absence or failure invalidates the block). Each is a
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
let SYSTEM_CALL_INPUT_LENGTH : byte_length = WORD_BYTE_LENGTH

let DEPOSIT_EVENT_DATA_LENGTH : byte_length = ByteQuantity(576)
let DEPOSIT_PUBKEY_HEAD : source_pointer = BYTE_ZERO
let DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD : source_pointer = ByteQuantity(32)
let DEPOSIT_AMOUNT_HEAD : source_pointer = ByteQuantity(64)
let DEPOSIT_SIGNATURE_HEAD : source_pointer = ByteQuantity(96)
let DEPOSIT_INDEX_HEAD : source_pointer = ByteQuantity(128)
let DEPOSIT_PUBKEY_LENGTH_WORD : source_pointer = ByteQuantity(160)
let DEPOSIT_PUBKEY_DATA : source_pointer = ByteQuantity(192)
let DEPOSIT_PUBKEY_LENGTH : byte_length = ByteQuantity(48)
let DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD : source_pointer = ByteQuantity(256)
let DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA : source_pointer = ByteQuantity(288)
let DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH : byte_length = WORD_BYTE_LENGTH
let DEPOSIT_AMOUNT_LENGTH_WORD : source_pointer = ByteQuantity(320)
let DEPOSIT_AMOUNT_DATA : source_pointer = ByteQuantity(352)
let DEPOSIT_AMOUNT_LENGTH : byte_length = EIGHT_BYTE_LENGTH
let DEPOSIT_SIGNATURE_LENGTH_WORD : source_pointer = ByteQuantity(384)
let DEPOSIT_SIGNATURE_DATA : source_pointer = ByteQuantity(416)
let DEPOSIT_SIGNATURE_LENGTH : byte_length = ByteQuantity(96)
let DEPOSIT_INDEX_LENGTH_WORD : source_pointer = ByteQuantity(512)
let DEPOSIT_INDEX_DATA : source_pointer = ByteQuantity(544)
let DEPOSIT_INDEX_LENGTH : byte_length = EIGHT_BYTE_LENGTH

/* Enter a top-level protocol system-call frame. The caller has already made a
 * fresh memory frame and, for a word input, frozen the parent-memory span that
 * `input` references. */
/*! Enters a top-level protocol system-call frame. The caller has already
    made a fresh memory frame and, for a word input, frozen the
    parent-memory span that `input` references. */
function enter_system_call_frame(tgt : address, input : ByteSlice) -> StateCheckpoint = {
    let checkpoint = k_state_checkpoint();
    pc = BYTE_ZERO;
    call_depth = 0;
    gas_remaining = Gas(30000000);
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
            is_static = false,
            depth = 0,
        };
    frame_code = code_db_resolve(k_code_key(tgt));
    checkpoint
}

/*! Issues one unchecked block-start system call: a 30M-gas frame from
    `SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
    code, and its output is discarded. */
function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        /* Freeze the word in a parent memory frame. The system call executes
         * above that frame, so its own memory writes cannot mutate calldata. */
        memory_reset();
        memory_expand_to(SYSTEM_CALL_INPUT_LENGTH);
        mem_store_word(BYTE_ZERO, input);
        let input_slice = memory_byte_slice(BYTE_ZERO, SYSTEM_CALL_INPUT_LENGTH);
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

/* ---- EIP-7002 / EIP-7251 block-END system calls (general-purpose requests) ---
 * After the transactions and withdrawals, the protocol dequeues the queued
 * withdrawal (EIP-7002) and consolidation (EIP-7251) requests by a CHECKED
 * system call (SYSTEM_ADDRESS, EMPTY calldata, 30M gas) to each predeploy. The
 * dequeue clears the request queue in the contract's storage, shaping the
 * post-state root. "Checked" means: a missing contract (no code) OR a failed
 * call makes the whole block INVALID (returns false here). */


/*! Issues one checked block-end system call (EIP-7002/EIP-7251): the
    target must exist and the call must succeed — otherwise the block is
    invalid — and the returned dequeued requests are the call's output. */
function system_call_checked(tgt : address) -> option(ByteSlice) = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        None() /* checked: a missing system predeploy invalidates the block */
    } else {
        memory_reset();
        let parent_memory = memory_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, EMPTY_SLICE);
        let output = interpret();
        let ok = frame_succeeded();
        let result : option(ByteSlice) = if ok then {
                let start = scratch_begin();
                scratch_push_slice(output);
                Some(scratch_finish(start))
            } else {
                None()
            };
        memory_frame_leave(parent_memory);
        if not_bool(ok) then {
            k_revert(checkpoint) /* checked: a reverting/erroring system call invalidates the block */
        };
        k_tx_merge(); /* system call is a top-level boundary: merge its storage */
        result
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
    if   (data.len != DEPOSIT_EVENT_DATA_LENGTH)
       | (slice_load(data, DEPOSIT_PUBKEY_HEAD) != word_of_nat(160))
       | (slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD) != word_of_nat(256))
       | (slice_load(data, DEPOSIT_AMOUNT_HEAD) != word_of_nat(320))
       | (slice_load(data, DEPOSIT_SIGNATURE_HEAD) != word_of_nat(384))
       | (slice_load(data, DEPOSIT_INDEX_HEAD) != word_of_nat(512))
       | (slice_load(data, DEPOSIT_PUBKEY_LENGTH_WORD) != word_of_nat(48))
       | (slice_load(data, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD) != word_of_nat(32))
       | (slice_load(data, DEPOSIT_AMOUNT_LENGTH_WORD) != word_of_nat(8))
       | (slice_load(data, DEPOSIT_SIGNATURE_LENGTH_WORD) != word_of_nat(96))
       | (slice_load(data, DEPOSIT_INDEX_LENGTH_WORD) != word_of_nat(8)) then {
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
/*! Collects the EIP-7685 execution requests at block end: deposits from
    logs (EIP-6110), then the withdrawal (EIP-7002) and consolidation
    (EIP-7251) queue dequeues. */
function collect_execution_requests(deposits : ByteSlice) -> (bool, ExecutionRequests) = {
    match system_call_checked(WITHDRAWAL_REQUEST_ADDR) {
        None() => (false, struct { deposits = deposits, withdrawals = EMPTY_SLICE, consolidations = EMPTY_SLICE }),
        Some(withdrawals) => match system_call_checked(CONSOLIDATION_REQUEST_ADDR) {
            None() => (false, struct { deposits = deposits, withdrawals = withdrawals, consolidations = EMPTY_SLICE }),
            Some(consolidations) => (
                true,
                struct { deposits = deposits, withdrawals = withdrawals, consolidations = consolidations },
            ),
        },
    }
}
