/* Optimized C overrides for `sail/executor/system_calls.sail`. */

/*! Profiles the interpreter. A fatal validation failure terminates the guest,
    so only the returning path closes the scope. */
function profiled_system_call_interpret() -> OutputSlice = {
    cycle_scope_start(50);
    let output = interpret();
    cycle_scope_end(50);
    output
}

/*! Profiles a system-call transaction merge with the same exception-safe
    scope discipline. */
function profiled_system_call_merge() -> unit = {
    cycle_scope_start(51);
    k_tx_merge();
    cycle_scope_end(51)
}

/*! Profiles one checked request-system call. The wrapper closes its request
    scope before propagating validation failure to the enclosing block scope. */
function profiled_system_call_checked(scope : cycle_scope_id, tgt : address) -> ScratchSlice = {
    cycle_scope_start(scope);
    let output = system_call_checked(tgt);
    cycle_scope_end(scope);
    output
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
        enter_system_call_frame(tgt, MemoryCalldata(evm_memory_slice(input_slice.bytes, input_slice.len)));
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
        fatal_error(ExecutionInvalid)
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
            fatal_error(ExecutionInvalid)
        }
    }
}

/*! Profiles one dequeued request stream and validates it byte for byte
    against the input's committed request bytes. */
function profiled_validate_request_stream(
    scope : cycle_scope_id,
    tgt : address,
    expected : StatelessInputSlice,
) -> (
    unit
) = {
    let dequeued = profiled_system_call_checked(scope, tgt);
    if not_bool(region_slices_equal(dequeued, expected)) then {
        fatal_error(InvalidExecutionRequests)
    }
}

function validate_execution_requests(input_ref : StatelessInputRef) -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    profiled_validate_request_stream(41, WITHDRAWAL_REQUEST_ADDR, input_ref.withdrawal_requests);
    profiled_validate_request_stream(42, CONSOLIDATION_REQUEST_ADDR, input_ref.consolidation_requests);
    if profile.fork >= Amsterdam then {
        profiled_validate_request_stream(43, BUILDER_DEPOSIT_REQUEST_ADDR, input_ref.builder_deposit_requests);
        profiled_validate_request_stream(44, BUILDER_EXIT_REQUEST_ADDR, input_ref.builder_exit_requests)
    } else if (region_slice_length(input_ref.builder_deposit_requests) != 0) |
        (region_slice_length(input_ref.builder_exit_requests) != 0) then {
        fatal_error(InvalidExecutionRequests)
    }
}
