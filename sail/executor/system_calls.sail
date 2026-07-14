/*md
# Protocol system calls

The block-level system calls the protocol issues around the transactions:
the EIP-4788 / EIP-2935 block-start writes (unchecked — skipped if the
contract is absent) and the EIP-7002 / EIP-7251 block-end request
dequeues (checked — absence or failure invalidates the block). Each is a
30M-gas frame from `SYSTEM_ADDRESS` whose committed storage writes shape
the post-state root.
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

/*! Enters a top-level protocol system-call frame. The caller has already
    made a fresh memory frame and, for a word input, frozen the
    parent-memory span that `input` references. */
function enter_system_call_frame(tgt : address, input : ByteSlice) -> StateCheckpoint = {
    let checkpoint = k_snapshot();
    pc = 0;
    call_depth = 0;
    gas_remaining = to_gas(30000000);
    stack_reset();
    returndata_clear();
    memory_size = 0;
    frame_status = Running();
    calldata = input;
    message =
        struct {
            caller = SYSTEM_ADDRESS,
            address = tgt,
            code_address = tgt,
            value = ZERO_WORD,
            data_len = unsigned(input.len),
            is_static = false,
            depth = 0,
        };
    frame_code = code_db_resolve(k_code_key(tgt));
    checkpoint
}

function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        /* Freeze the word in a parent memory frame. The system call executes
         * above that frame, so its own memory writes cannot mutate calldata. */
        mem_clear();
        mem_store_word(0x0000000000000000, input);
        let input_slice = mem_slice(0, 32);
        mem_frame_enter();
        let checkpoint = enter_system_call_frame(tgt, input_slice);
        let _ = interpret();
        mem_frame_leave();
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


function system_call_checked(tgt : address) -> option(ByteSlice) = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        None() /* checked: a missing system predeploy invalidates the block */
    } else {
        mem_clear();
        mem_frame_enter();
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
        mem_frame_leave();
        if not_bool(ok) then {
            k_revert(checkpoint) /* checked: a reverting/erroring system call invalidates the block */
        };
        k_tx_merge(); /* system call is a top-level boundary: merge its storage */
        result
    }
}

function deposit_log_matches(log : LogEntry) -> bool =
    if log.address != DEPOSIT_CONTRACT_ADDR then {
        false
    } else {
        match log.topics {
            topic :: _ => topic == DEPOSIT_EVENT_TOPIC,
            [||] => false,
        }
    }

/* Strip the ABI framing from one DepositEvent and append the consensus-layer
 * request fields directly to the scratch arena. */
function append_deposit_request(data : ByteSlice) -> unit = {
    if   (unsigned(data.len) != 576)
       | (slice_load(data, 0) != word_of_int(160))
       | (slice_load(data, 32) != word_of_int(256))
       | (slice_load(data, 64) != word_of_int(320))
       | (slice_load(data, 96) != word_of_int(384))
       | (slice_load(data, 128) != word_of_int(512))
       | (slice_load(data, 160) != word_of_int(48))
       | (slice_load(data, 256) != word_of_int(32))
       | (slice_load(data, 320) != word_of_int(8))
       | (slice_load(data, 384) != word_of_int(96))
       | (slice_load(data, 512) != word_of_int(8)) then {
        throw InvalidBlock(InvalidExecutionRequests)
    };
    scratch_push_slice(sub_slice(data, 192, 48));
    scratch_push_slice(sub_slice(data, 288, 32));
    scratch_push_slice(sub_slice(data, 352, 8));
    scratch_push_slice(sub_slice(data, 416, 96));
    scratch_push_slice(sub_slice(data, 544, 8))
}

function collect_deposit_requests(receipts : list(Receipt)) -> ByteSlice = {
    let start = scratch_begin();
    var remaining_receipts : list(Receipt) = receipts;
    var more_receipts : bool = true;
    while more_receipts do {
        match remaining_receipts {
            receipt :: receipt_tail => {
                var remaining_logs : list(LogEntry) = receipt.logs;
                var more_logs : bool = true;
                while more_logs do {
                    match remaining_logs {
                        log :: log_tail => {
                            if deposit_log_matches(log) then {
                                append_deposit_request(log.data)
                            };
                            remaining_logs = log_tail
                        },
                        [||] => more_logs = false,
                    }
                };
                remaining_receipts = receipt_tail
            },
            [||] => more_receipts = false,
        }
    };
    scratch_finish(start)
}

/* Request types are processed in ascending order, stopping immediately when
 * a checked system call is missing or fails. */
function collect_execution_requests(receipts : list(Receipt)) -> (bool, ExecutionRequests) = {
    let deposits = collect_deposit_requests(receipts);
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
