/* Attribute-only refinements keep raw-byte decoding and fork deployment in
 * the canonical specification while exposing their bodies to the threaded
 * interpreter's fork-gated destination labels. */

$[c_inline]
val opcode_available : (opcode, Fork) -> bool

$[c_inline]
val decode_push_immediate : (Code, code_scan_position, push_width) -> (code_pointer, word)

$[c_inline]
val decode_deep_immediate : (Code, code_scan_position, DeepStackOperation) -> (code_pointer, byte)

$[c_inline]
val execute_push_encoded : (Code, opcode, code_scan_position, gas, StackPointer) -> (
        code_pointer,
        gas,
        StackPointer,
        OpcodeOutcome,
    )

$[c_inline]
val execute_dup_encoded : (opcode, gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)

$[c_inline]
val execute_swap_encoded : (opcode, gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)

$[c_inline]
val execute_log_encoded : (address, bool, memory_base, opcode, gas, StackPointer, memory_height) -> (
        gas,
        StackPointer,
        memory_height,
        OpcodeOutcome,
    )

$[c_inline]
val execute_deep_stack_encoded : (Code, opcode, code_scan_position, gas, StackPointer) -> (
        code_pointer,
        gas,
        StackPointer,
        OpcodeOutcome,
    )

/* The stack arities remain small compile-time selectors used inside the
 * canonical frame-entry operations. */
$[c_inline]
val call_stack_inputs : CallKind -> operand_stack_height

$[c_inline]
val create_stack_inputs : CreateKind -> operand_stack_height

/* Frame entry and resumption are deliberately ordinary generated Sail calls.
 * They are reached only when control crosses a frame boundary and pull in
 * large state, journal, code, and precompile graphs. Forcing them into the
 * threaded loop creates one enormous LTO partition without removing work
 * from the per-opcode dispatch path. */

$[c_inline]
val opcode_frame_status : OpcodeOutcome -> FrameStatus

$[c_inline]
val exceptional_state : (state_gas, state_gas_spill, state_gas, ExceptionKind) -> ExceptionalStateTransition

/* The optimized build refines only the interpreter control-flow boundary.
 * Opcode semantics, frame entry, and frame resumption remain generated from
 * the canonical Sail bodies and are called by the threaded C dispatcher. */
val threaded_interpret_host = impure { c: "threaded_interpret" } : (
        gas,
        state_gas,
        state_gas_spill,
        gas_refund,
        StackPointer,
        memory_base,
        memory_height,
        address,
        address,
        address,
        word,
        state_gas,
        bool,
        frame_depth,
        Code,
        CalldataSlice,
        Fork,
        word,
    ) -> (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)

function interpret(
    initial_gas : gas,
    initial_state_gas : state_gas,
    initial_state_spill : state_gas_spill,
    initial_refund : gas_refund,
    initial_sp : StackPointer,
    initial_memory_base : memory_base,
    initial_memory_height : memory_height,
    initial_caller : address,
    initial_address : address,
    initial_code_address : address,
    initial_value : word,
    initial_state_gas_reservoir : state_gas,
    initial_is_static : bool,
    initial_depth : frame_depth,
    initial_code : Code,
    initial_calldata : CalldataSlice,
) -> (
    (gas, state_gas, state_gas_spill, gas_refund, FrameStatus, OutputSlice)
) = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let fork = profile.fork;
    let blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, k_header.excess_blob_gas);
    threaded_interpret_host(
        initial_gas,
        initial_state_gas,
        initial_state_spill,
        initial_refund,
        initial_sp,
        initial_memory_base,
        initial_memory_height,
        initial_caller,
        initial_address,
        initial_code_address,
        initial_value,
        initial_state_gas_reservoir,
        initial_is_static,
        initial_depth,
        initial_code,
        initial_calldata,
        fork,
        blob_fee,
    )
}
