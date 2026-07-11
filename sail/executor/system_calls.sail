/* ======================================================================== */
/* EXECUTOR: PROTOCOL SYSTEM CALLS                                          */
/*                                                                          */
/* The block-level system calls the protocol issues around the              */
/* transactions: the EIP-4788 / EIP-2935 block-start writes (unchecked --   */
/* skipped if the contract is absent) and the EIP-7002 / EIP-7251           */
/* block-end request dequeues (checked -- absence or failure invalidates    */
/* the block). Each is a 30M-gas frame from SYSTEM_ADDRESS whose committed  */
/* storage writes shape the post-state root.                                */
/* ======================================================================== */

/* ---- EIP-4788 / EIP-2935 block-start system calls ---------------------------
 * Before the transactions, the protocol issues unchecked system calls from
 * SYSTEM_ADDRESS to the beacon-roots (EIP-4788) and block-hash-history (EIP-2935)
 * contracts, writing the parent beacon-block root and the parent block hash into
 * their ring buffers. Each takes a 32-byte input, 30M gas, and bumps no nonce /
 * charges no fee / counts toward no block gas; it is skipped if the contract has
 * no code. Their committed storage writes shape the post-state root. */
function SYSTEM_ADDRESS() -> address = 0xfffffffffffffffffffffffffffffffffffffffe
function BEACON_ROOTS_ADDR() -> address = 0x000f3df6d732807ef1319fb7b8bb8522d0beac02
function HISTORY_STORAGE_ADDR() -> address = 0x0000f90827f1c53a10cb7a02335b175320002935

/* Enter a top-level protocol system-call frame.
 *
 * The caller stages txdata before entering this helper.
 * Keeping frame initialization in one place gives the block driver a named
 * boundary: pre-transaction and post-transaction system calls differ in
 * validation policy, not in execution-frame construction.
 */
function enter_system_call_frame(tgt : address, input_len : int) -> unit = {
    k_snapshot();
    pc = 0;
    call_depth = 0;
    gas_remaining = to_gas(30000000);
    stack_clear();
    mem_clear();
    returndata_reset_all();
    memory_size = 0;
    frame_status = Running();
    calldata_bind_tx_input(); /* AFTER mem_clear: it points the base frame's calldata at the slot */
    message =
        struct {
            caller = SYSTEM_ADDRESS(),
            address = tgt,
            code_address = tgt,
            value = ZERO_WORD,
            data_len = input_len,
            is_static = false,
            depth = 0,
        };
    frame_code_addr(tgt)
}

function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        let _ = txdata_set_word(input); /* the call's 32-byte calldata */
        enter_system_call_frame(tgt, 32);
        interpret();
        if frame_succeeded() then {
            k_commit()
        } else {
            k_revert()
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
function WITHDRAWAL_REQUEST_ADDR() -> address = 0x00000961ef480eb55e80d19ad83579a64c007002
function CONSOLIDATION_REQUEST_ADDR() -> address = 0x0000bbddc7ce488642fb579f8b00f3a590007251

function system_call_checked(tgt : address) -> bool = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        false /* checked: a missing system predeploy invalidates the block */
    } else {
        let _ = txdata_set_source(byte_slice_test(WitnessSource, 0, 0)); /* empty calldata: the SYSTEM caller dequeues */
        enter_system_call_frame(tgt, 0);
        interpret();
        let ok = frame_succeeded();
        if ok then {
            k_commit()
        } else {
            k_revert() /* checked: a reverting/erroring system call invalidates the block */
        };
        k_tx_merge(); /* system call is a top-level boundary: merge its storage */
        ok
    }
}
