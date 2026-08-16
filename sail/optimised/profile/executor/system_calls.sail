/* Optimized C overrides for `sail/executor/system_calls.sail`. */

/*! Profiles one system-call frame. A fatal validation failure terminates the
    guest, so only the returning path closes the scope. */
function profiled_run_system_call_frame(
    tgt : address,
    code : Code,
    input : CalldataSlice,
    memory_base : memory_base,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    cycle_scope_start(50);
    let result = run_system_call_frame(tgt, code, input, memory_base);
    cycle_scope_end(50);
    result
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
    let code_hash = k_code_key(tgt);
    if code_hash == KECCAK_EMPTY then {
        return ()
    };
    let code = code_db_resolve(code_hash);

    let initial_memory_base = MEMORY_BASE_ZERO;
    let initial_memory_height = MEMORY_HEIGHT_ZERO;
    let input_range : MemoryRange = memory_range(0, SYSTEM_CALL_INPUT_LENGTH);
    let expanded_memory = expand_memory(initial_memory_base, initial_memory_height, input_range.len);
    let input_word = hash_to_word(input);
    mem_store(initial_memory_base, input_range.off, input_word);
    let input_slice = active_memory_slice(initial_memory_base, expanded_memory, input_range.off, input_range.len);
    let child_memory_base = memory_absolute(initial_memory_base, expanded_memory);
    let memory_input = evm_memory_slice(input_slice.bytes, input_slice.len);
    let frame_input = MemoryCalldata(memory_input);
    let (_, _, _, _, status, _) = profiled_run_system_call_frame(tgt, code, frame_input, child_memory_base);
    let succeeded = frame_succeeded(status);
    let failed = not_bool(succeeded);
    if failed then {
        k_journal_revert()
    } else {
        k_journal_commit()
    };
    profiled_system_call_merge()
}

function system_call_checked(tgt : address) -> ScratchSlice = {
    let code_hash = k_code_key(tgt);
    if code_hash == KECCAK_EMPTY then {
        fatal_error(ExecutionInvalid)
    } else {
        let code = code_db_resolve(code_hash);
        let (_, _, _, _, status, output) = profiled_run_system_call_frame(tgt, code, EMPTY_CALLDATA, MEMORY_BASE_ZERO);
        let succeeded = frame_succeeded(status);
        if succeeded then {
            let start = scratch_reserve(output.len);
            scratch_push_slice(output);
            let result = scratch_finish(start);
            k_journal_commit();
            profiled_system_call_merge();
            result
        } else {
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
