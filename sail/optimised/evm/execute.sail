/* Optimized persistent-storage opcode paths for `sail/evm/execute.sail`.
   The semantic control flow remains in Sail. The optimized host exposes its
   dense storage identifier so each handler resolves a slot once and reuses
   that identifier for warmness, loading, and updating. */
type AccountId = range(0, 2 ^ 32 - 1)
type StorageId = range(0, 2 ^ 32 - 1)
type StorageCount = range(0, 2 ^ 32 - 1)
type StorageGeneration = range(0, 2 ^ 32 - 1)
struct AccountExecutionContext = {
    account_id : AccountId,
    storage_begin : StorageId,
    storage_change_count : StorageCount,
    storage_count : StorageCount,
    storage_generation : StorageGeneration,
}
val account_execution_context_host = impure { c: "account_execution_context" } : address -> AccountExecutionContext

$[c_inline]
function account_execution_context(address : address) -> AccountExecutionContext =
    account_execution_context_host(address)
$[c_inline]
function refresh_account_execution_context(
    context : AccountExecutionContext,
    previous_address : address,
    next_address : address,
) -> AccountExecutionContext =
    if previous_address == next_address then context else account_execution_context_host(next_address)
val storage_resolve_slot_id = impure { c: "storage_resolve_slot" } : (
        StorageId,
        StorageCount,
        StorageCount,
        word,
    ) -> StorageId
val storage_id_is_warm_host = impure { c: "storage_id_is_warm" } : StorageId -> bool
val storage_load_by_id_host = impure { c: "storage_load_by_id" } : (
    AccountId,
    StorageGeneration,
    StorageId,
    bool,
) -> StorageValue
val storage_update_by_id_host = impure { c: "storage_update_by_id" } : (
    AccountId,
    StorageGeneration,
    StorageId,
    word,
    word,
) -> unit
/*! Implements `SLOAD` while retaining the optimized storage identifier
    across the warmness and load operations. */
$[c_inline]
function execute_sload(context : AccountExecutionContext, carried_gas : gas, carried_sp : StackPointer) -> (
    gas,
    StackPointer,
    OpcodeOutcome,
) = {
    let stack_status = guard_stack(carried_sp, 1, 1);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_sp, stack_status)
    };
    let slot = read_stack_word(carried_sp);
    let sp_after_pop = stack_top_retreat(carried_sp, 1);
    let storage_id = storage_resolve_slot_id(
        context.storage_begin,
        context.storage_change_count,
        context.storage_count,
        slot,
    );
    let warm = storage_id_is_warm_host(storage_id);
    let gas_cost = sload_cost(warm);
    let (halt, gas_after_charge) = charge(carried_gas, gas_cost);
    if halt then {
        (gas_after_charge, sp_after_pop, Failed(OutOfGas))
    } else {
        let entry = storage_load_by_id_host(
            context.account_id,
            context.storage_generation,
            storage_id,
            not_bool(warm),
        );
        write_stack_word(carried_sp, entry.curr);
        (gas_after_charge, carried_sp, Continue())
    }
}
/*! Implements `SSTORE` while retaining the optimized storage identifier
    across warmness, load, and update operations. */
$[c_inline]
function execute_sstore(
    context : AccountExecutionContext,
    fork : Fork,
    carried_is_static : bool,
    carried_gas : gas,
    carried_state_gas : state_gas,
    carried_state_spill : state_gas_spill,
    carried_refund : gas_refund,
    carried_sp : StackPointer,
) -> (
    gas,
    state_gas,
    state_gas_spill,
    gas_refund,
    StackPointer,
    OpcodeOutcome,
) = {
    let stack_status = guard_stack(carried_sp, 2, 0);
    if opcode_failed(stack_status) then {
        return (GAS_ZERO, carried_state_gas, carried_state_spill, carried_refund, carried_sp, stack_status)
    };
    let (_, guard_result) = guard_static(carried_gas, carried_is_static);
    if opcode_failed(guard_result) then {
        return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, guard_result)
    };
    if (fork < Amsterdam) & (carried_gas <= G_callstipend) then {
        return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, Failed(OutOfGas))
    };
    let slot = read_stack_word(carried_sp);
    let sp_after_slot = stack_top_retreat(carried_sp, 1);
    let value = read_stack_word(sp_after_slot);
    let sp_after = stack_top_retreat(sp_after_slot, 1);
    let storage_id = storage_resolve_slot_id(
        context.storage_begin,
        context.storage_change_count,
        context.storage_count,
        slot,
    );
    let warm = storage_id_is_warm_host(storage_id);
    let cold = not_bool(warm);
    if fork >= Amsterdam then {
        let sentry_cost = sstore_sentry_cost(cold);
        let (sentry_halt, _) = check_execution_gas(carried_gas, sentry_cost);
        if sentry_halt then {
            return (carried_gas, carried_state_gas, carried_state_spill, carried_refund, sp_after, Failed(OutOfGas))
        }
    };
    let entry = storage_load_by_id_host(
        context.account_id,
        context.storage_generation,
        storage_id,
        cold,
    );
    let costs = sstore_costs(entry.orig, entry.curr, value, cold);
    let (gas_after_refund_credit, state_gas_after_credit, state_spill_after_credit) =
        if costs.state_credit != 0 then
            credit_state_gas_refund(carried_gas, carried_state_gas, carried_state_spill, costs.state_credit)
        else
            (carried_gas, carried_state_gas, carried_state_spill);
    let (execution_halt, gas_after_execution) = charge(gas_after_refund_credit, costs.execution);
    if execution_halt then {
        return (
            gas_after_execution,
            state_gas_after_credit,
            state_spill_after_credit,
            carried_refund,
            sp_after,
            Failed(OutOfGas),
        )
    };
    let (state_halt, gas_after_state_charge, state_gas_after_charge, state_spill_after_charge) =
        charge_state_gas(
            gas_after_execution,
            state_gas_after_credit,
            state_spill_after_credit,
            costs.state_charge,
        );
    if state_halt then {
        return (
            gas_after_state_charge,
            state_gas_after_charge,
            state_spill_after_charge,
            carried_refund,
            sp_after,
            Failed(OutOfGas),
        )
    };
    let refund_after =
        if not_bool(costs.refund == GAS_REFUND_ZERO) then record_refund(carried_refund, costs.refund)
        else carried_refund;
    if entry.curr != value then {
        storage_update_by_id_host(
            context.account_id,
            context.storage_generation,
            storage_id,
            value,
            entry.orig,
        )
    };
    (
        gas_after_state_charge,
        state_gas_after_charge,
        state_spill_after_charge,
        refund_after,
        sp_after,
        Continue(),
    )
}
$[c_inline]
val execute_and : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_or : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_xor : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_lt : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_gt : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_slt : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_sgt : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_byte : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_div : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_mod : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_addmod : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_mulmod : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_shl : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_shr : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_add : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_sub : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_mul : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_eq : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_iszero : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
$[c_inline]
val execute_not : (gas, StackPointer) -> (gas, StackPointer, OpcodeOutcome)
