/* Optimized C overrides for `sail/executor/system_calls.sail`. */

/*! Profiles the interpreter while preserving balanced scope tags when a block
    validation exception leaves the system call early. */
function profiled_system_call_interpret() -> OutputSlice = {
    cycle_scope_start(50);
    try {
        let output = interpret();
        cycle_scope_end(50);
        output
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(50);
            throw InvalidBlock(reason)
        },
        Overflow() => {
            cycle_scope_end(50);
            throw Overflow()
        },
    }
}

/*! Profiles a system-call transaction merge with the same exception-safe
    scope discipline. */
function profiled_system_call_merge() -> unit = {
    cycle_scope_start(51);
    try {
        k_tx_merge();
        cycle_scope_end(51)
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(51);
            throw InvalidBlock(reason)
        },
        Overflow() => {
            cycle_scope_end(51);
            throw Overflow()
        },
    }
}

/*! Profiles one checked request-system call. The wrapper closes its request
    scope before propagating validation failure to the enclosing block scope. */
function profiled_system_call_checked(scope : cycle_scope_id, tgt : address) -> ScratchSlice = {
    cycle_scope_start(scope);
    try {
        let output = system_call_checked(tgt);
        cycle_scope_end(scope);
        output
    } catch {
        InvalidBlock(reason) => {
            cycle_scope_end(scope);
            throw InvalidBlock(reason)
        },
        Overflow() => {
            cycle_scope_end(scope);
            throw Overflow()
        },
    }
}

function system_call(tgt : address, input : hash) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        ()
    } else {
        memory_reset();
        let input_range : MemoryRange = memory_range(0, SYSTEM_CALL_INPUT_LENGTH);
        let _ = memory_expand_to(input_range.len);
        mem_store_word(input_range.off, hash_to_word(input));
        let input_slice = active_memory_slice(input_range.off, input_range.len);
        let parent_memory = memory_frame_enter();
        enter_system_call_frame(tgt, MemoryCalldata(evm_memory_slice(input_slice.off, input_slice.len)));
        let _ = profiled_system_call_interpret();
        memory_frame_leave(parent_memory);
        if not_bool(frame_succeeded()) then {
            k_journal_revert()
        } else {
            k_journal_commit()
        };
        profiled_system_call_merge()
    }
}

function system_call_checked(tgt : address) -> ScratchSlice = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        throw InvalidBlock(ExecutionInvalid)
    } else {
        memory_reset();
        let parent_memory = memory_frame_enter();
        enter_system_call_frame(tgt, EMPTY_CALLDATA);
        let output = profiled_system_call_interpret();
        if frame_succeeded() then {
            let start = scratch_reserve(output.len);
            scratch_push_slice(output);
            let result = scratch_finish(start);
            memory_frame_leave(parent_memory);
            k_journal_commit();
            profiled_system_call_merge();
            result
        } else {
            memory_frame_leave(parent_memory);
            k_journal_revert();
            profiled_system_call_merge();
            throw InvalidBlock(ExecutionInvalid)
        }
    }
}

function collect_execution_requests(deposits : StatelessInputSlice) -> ExecutionRequests = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let withdrawals = profiled_system_call_checked(41, WITHDRAWAL_REQUEST_ADDR);
    let consolidations = profiled_system_call_checked(42, CONSOLIDATION_REQUEST_ADDR);
    let builder_deposits : ScratchSlice = if profile.fork >= Amsterdam then {
            profiled_system_call_checked(43, BUILDER_DEPOSIT_REQUEST_ADDR)
        } else {
            EMPTY_SCRATCH_SLICE
        };
    let builder_exits : ScratchSlice = if profile.fork >= Amsterdam then {
            profiled_system_call_checked(44, BUILDER_EXIT_REQUEST_ADDR)
        } else {
            EMPTY_SCRATCH_SLICE
        };
    struct {
        deposits = deposits,
        withdrawals = withdrawals,
        consolidations = consolidations,
        builder_deposits = builder_deposits,
        builder_exits = builder_exits,
    }
}
