#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

u256 read_push(Bytes code, uint32_t offset, uint8_t n)
{
  return code_slice_load_n(code, offset, n);
}

__attribute__((__always_inline__)) bool opcode_available(uint8_t opcode, uint8_t fork)
{
  if (opcode == UINT8_C(30)) {
    return (bool)(fork >= Osaka);
  }
  if (opcode == UINT8_C(72)) {
    return (bool)(fork >= London);
  }
  if (opcode == UINT8_C(73)) {
    return (bool)(fork >= Cancun);
  }
  if (opcode == UINT8_C(74)) {
    return (bool)(fork >= Cancun);
  }
  if (opcode == UINT8_C(75)) {
    return (bool)(fork >= Amsterdam);
  }
  if (opcode == UINT8_C(92)) {
    return (bool)(fork >= Cancun);
  }
  if (opcode == UINT8_C(93)) {
    return (bool)(fork >= Cancun);
  }
  if (opcode == UINT8_C(94)) {
    return (bool)(fork >= Cancun);
  }
  if (opcode == UINT8_C(95)) {
    return (bool)(fork >= Shanghai);
  }
  if (opcode == UINT8_C(230)) {
    return (bool)(fork >= Amsterdam);
  }
  if (opcode == UINT8_C(231)) {
    return (bool)(fork >= Amsterdam);
  }
  if (opcode == UINT8_C(232)) {
    return (bool)(fork >= Amsterdam);
  }
  return true;
}

__attribute__((__always_inline__)) u256 decode_push_immediate(struct CodeFields frame_code, uint32_t *restrict immediate_offset, uint8_t width)
{
  Bytes bytes = code_bytes(frame_code);
  u256 value = read_push(bytes, (*immediate_offset), width);
  (*immediate_offset) = ((*immediate_offset) + (uint32_t)width);
  return value;
}

__attribute__((__always_inline__)) uint64_t decode_deep_immediate(struct CodeFields frame_code, uint32_t *restrict immediate_offset, enum DeepStackOperation operation)
{
  Bytes bytes = code_bytes(frame_code);
  uint64_t immediate = code_slice_byte(bytes, (*immediate_offset));
  bool immediate_valid = deep_stack_operation_immediate_valid(operation, immediate);
  uint32_t next_pc = immediate_valid ? ((*immediate_offset) + UINT32_C(1)) : (*immediate_offset);
  (*immediate_offset) = next_pc;
  return immediate;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(95) <= opcode) && (opcode <= UINT8_C(127))) {
    uint8_t width = ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(95)));
    u256 _8_1457_8_1615 = decode_push_immediate(frame_code, immediate_offset, width);
    uint64_t carried_gas_8_1569 = (*execution_gas);
    StackPointer carried_sp_8_1570 = (*sp);
    struct OpcodeOutcome _8_1446_8_1571 = execute_push(&carried_gas_8_1569, &carried_sp_8_1570, width, _8_1457_8_1615);
    (*execution_gas) = carried_gas_8_1569;
    (*sp) = carried_sp_8_1570;
    return _8_1446_8_1571;
  }
  uint64_t carried_gas_8_1567 = (*execution_gas);
  struct OpcodeOutcome _8_1455_8_1568 = execute_invalid(&carried_gas_8_1567);
  (*execution_gas) = carried_gas_8_1567;
  return _8_1455_8_1568;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(128) <= opcode) && (opcode <= UINT8_C(143))) {
    return execute_dup(execution_gas, sp, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(127))));
  }
  uint64_t carried_gas_8_1572 = (*execution_gas);
  struct OpcodeOutcome _8_1455_8_1573 = execute_invalid(&carried_gas_8_1572);
  (*execution_gas) = carried_gas_8_1572;
  return _8_1455_8_1573;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(144) <= opcode) && (opcode <= UINT8_C(159))) {
    return execute_swap(execution_gas, sp, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(143))));
  }
  uint64_t carried_gas_8_1574 = (*execution_gas);
  struct OpcodeOutcome _8_1455_8_1575 = execute_invalid(&carried_gas_8_1574);
  (*execution_gas) = carried_gas_8_1574;
  return _8_1455_8_1575;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log_encoded(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp, uint32_t *restrict memory)
{
  if ((UINT8_C(160) <= opcode) && (opcode <= UINT8_C(164))) {
    return execute_log(carried_address, carried_is_static, memory_base, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(160))), execution_gas, sp, memory);
  }
  uint64_t carried_gas_8_1576 = (*execution_gas);
  struct OpcodeOutcome _8_1455_8_1577 = execute_invalid(&carried_gas_8_1576);
  (*execution_gas) = carried_gas_8_1576;
  return _8_1455_8_1577;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_deep_stack_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  enum DeepStackOperation operation = deep_stack_operation(opcode);
  uint64_t _8_1458_8_1616 = decode_deep_immediate(frame_code, immediate_offset, operation);
  uint64_t field_0_8_1790;
  StackPointer stackpointer_8_1791;
  struct OpcodeOutcome opcodeoutcome_8_1792;
  switch (operation) {
  case DeepStackDuplicate:
    opcodeoutcome_8_1792 = execute_dupn(execution_gas, sp, _8_1458_8_1616);
    field_0_8_1790 = (*execution_gas);
    stackpointer_8_1791 = (*sp);
    break;
  case DeepStackSwap:
    opcodeoutcome_8_1792 = execute_swapn(execution_gas, sp, _8_1458_8_1616);
    field_0_8_1790 = (*execution_gas);
    stackpointer_8_1791 = (*sp);
    break;
  case DeepStackExchange:
    opcodeoutcome_8_1792 = execute_exchange(execution_gas, sp, _8_1458_8_1616);
    field_0_8_1790 = (*execution_gas);
    stackpointer_8_1791 = (*sp);
    break;
  case NotDeepStackOperation:
  {
    struct OpcodeOutcome _8_1455_8_1617 = execute_invalid(execution_gas);
    field_0_8_1790 = (*execution_gas);
    stackpointer_8_1791 = (*sp);
    opcodeoutcome_8_1792 = _8_1455_8_1617;
    break;
  }
  }
  (*execution_gas) = field_0_8_1790;
  (*sp) = stackpointer_8_1791;
  return opcodeoutcome_8_1792;
}

Bytes frame_output(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1799;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltReturn) {
    goto case_1799;
  }
  return frame_status.variants.Halted.variants.HaltReturn;
case_1799: ;
  if (frame_status.kind != Kind_Halted) {
    goto case_1798;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1798;
  }
  return frame_status.variants.Halted.variants.HaltRevert;
case_1798: ;
  return EMPTY_OUTPUT_SLICE;
}

bool frame_succeeded(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1791;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1791;
  }
  return false;
case_1791: ;
  switch (frame_status.kind) {
  case Kind_Halted:
  case Kind_Running:
    return true;
  case Kind_Exceptional:
    return false;
  }
}

struct CodeFields executable_code(bytes20 target, bool dele, bytes20 dtgt)
{
  if (dele) {
    bytes32 delegate_key = k_code_key(dtgt);
    struct CodeFields delegate_code = code_db_resolve(delegate_key);
    enum PrecompileId delegate_precompile = precompile_id_for_address(dtgt);
    if (neq_anything_EPrecompileId__(delegate_precompile, NotPrecompile)) {
      delegate_code = EMPTY_CODE;
    }
    return delegate_code;
  }
  bytes32 target_key = k_code_key(target);
  return code_db_resolve(target_key);
}

struct CallSemantics call_semantics(enum CallKind kind)
{
  switch (kind) {
  case Call:
    return ((struct CallSemantics){.enters_static_context = false, .inherits_caller_and_value = false, .takes_value = true, .transfers_value = true, .uses_target_address = true});
  case CallCode:
    return ((struct CallSemantics){.enters_static_context = false, .inherits_caller_and_value = false, .takes_value = true, .transfers_value = false, .uses_target_address = false});
  case DelegateCall:
    return ((struct CallSemantics){.enters_static_context = false, .inherits_caller_and_value = true, .takes_value = false, .transfers_value = false, .uses_target_address = false});
  case StaticCall:
    return ((struct CallSemantics){.enters_static_context = true, .inherits_caller_and_value = false, .takes_value = false, .transfers_value = false, .uses_target_address = true});
  }
}

uint16_t call_stack_inputs(enum CallKind kind)
{
  switch (kind) {
  case Call:
  case CallCode:
    return UINT16_C(7);
  case DelegateCall:
  case StaticCall:
    return UINT16_C(6);
  }
}

struct FrameTransition run_call(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind)
{
  uint16_t stack_inputs = call_stack_inputs(kind);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, stack_inputs, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(stack_status.variants.Failed))});
    } else {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = (Exceptional(stack_status.variants.Failed))});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1670 = carried_state_gas;
    uint32_t state_spill_after_3_1671 = carried_state_spill;
    struct FrameStatus status_after_3_1672 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CallSemantics semantics = call_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_5 = k_execution_profile;
    struct ProtocolProfileFields profile_8_6 = execution_profile_8_5.protocol;
    u256 gas_request = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 target_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    bytes20 target = word_to_address(target_word);
    u256 __sail_c_repr_u256_8_1793;
    StackPointer stackpointer_8_1794;
    if (semantics.takes_value) {
      __sail_c_repr_u256_8_1793 = stack_slot_read(sp_after, UINT16_C(0));
      stackpointer_8_1794 = stack_top_retreat(sp_after, UINT16_C(1));
    } else {
      __sail_c_repr_u256_8_1793 = WORD_ZERO;
      stackpointer_8_1794 = sp_after;
    }
    u256 value_3_1673 = __sail_c_repr_u256_8_1793;
    sp_after = stackpointer_8_1794;
    bool value_nonzero = word_nonzero(value_3_1673);
    u256 args_off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 args_len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 ret_off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 ret_len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    if (semantics.transfers_value && (value_nonzero && carried_is_static)) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1674;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(WriteProtection))});
      } else {
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(WriteProtection))});
      }
      state_gas_after_3_1670 = exceptional_3_1674.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1674.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1674.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_requested_height = memory_requested_height(args_off_word, args_len_word);
    uint64_t ret_requested_height = memory_requested_height(ret_off_word, ret_len_word);
    uint64_t requested_height = args_requested_height < ret_requested_height ? ret_requested_height : args_requested_height;
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))) <= gas_after) {
        expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))));
      } else {
        expansion_cost = GAS_CHARGE_UNAFFORDABLE;
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1675;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1675.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1675.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1675.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    if (gas_after < static_base) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1676;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1676.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1676.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1676.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, static_base);
    k_account_mark_warm(target);
    bool tg_deleg;
    bytes20 tg_target = k_deleg_target(target, &tg_deleg);
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool new_account_charged = (bool)((profile_8_6.fork >= Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)));
    uint16_t create_cost = (profile_8_6.fork < Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)) ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    if (gas_after < additional_cost) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1677;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1677.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1677.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1677.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, additional_cost);
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    if (profile_8_6.fork >= Amsterdam) {
      if (new_account_charged) {
        bool _8_1511_8_1631;
        uint64_t _8_1512_8_1632;
        uint64_t _8_1513_8_1633;
        uint32_t _8_1514_8_1634 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1511_8_1631, &_8_1512_8_1632, &_8_1513_8_1633);
        gas_after = _8_1512_8_1632;
        state_gas_after_3_1670 = _8_1513_8_1633;
        state_spill_after_3_1671 = _8_1514_8_1634;
        if (_8_1511_8_1631) {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1678;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
          }
          state_gas_after_3_1670 = exceptional_3_1678.state_gas_remaining;
          state_spill_after_3_1671 = exceptional_3_1678.state_gas_spilled;
          status_after_3_1672 = exceptional_3_1678.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1679;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1670 = exceptional_3_1679.state_gas_remaining;
        state_spill_after_3_1671 = exceptional_3_1679.state_gas_spilled;
        status_after_3_1672 = exceptional_3_1679.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      gas_after = gas_sub(gas_after, base_child);
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1680;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1680 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1680 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1670 = exceptional_3_1680.state_gas_remaining;
        state_spill_after_3_1671 = exceptional_3_1680.state_gas_spilled;
        status_after_3_1672 = exceptional_3_1680.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      gas_after = gas_sub(gas_after, base_child);
    }
    if (tg_deleg) {
      k_account_mark_warm(tg_target);
    }
    if (tg_deleg) {
      bytes32 delegate_key = k_code_key(tg_target);
      code_db_resolve(delegate_key);
      k_aload_(tg_target);
    }
    struct MemoryAccessFields args_access = memory_access(args_off_word, args_len_word);
    struct MemoryAccessFields ret_access = memory_access(ret_off_word, ret_len_word);
    uint32_t materialized_required_size = args_access.requested_height < ret_access.requested_height ? ret_access.requested_height : args_access.requested_height;
    uint32_t mem1;
    if (materialized_required_size <= (UINT32_C(4294967295) - carried_memory_base)) {
      if (memory_after < materialized_required_size) {
        mem_expand(carried_memory_base, memory_after, materialized_required_size);
        mem1 = materialized_required_size;
      } else {
        mem1 = memory_after;
      }
    } else {
      fatal_error(ExecutionInvalid);
    }
    uint64_t child_gas = conserved_gas_add(base_child, stipend);
    k_aload_(target);
    bool insufficient_balance;
    if (semantics.takes_value && value_nonzero) {
      u256 caller_balance = k_get_balance(carried_address);
      insufficient_balance = (bool)(!(word_ule(value_3_1673, caller_balance)));
    } else {
      insufficient_balance = false;
    }
    if (insufficient_balance || (carried_depth == UINT16_C(1024))) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (new_account_charged) {
        uint64_t _8_1365_8_1628;
        uint64_t _8_1366_8_1629;
        uint32_t _8_1367_8_1630 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1628, &_8_1366_8_1629);
        gas_after = _8_1365_8_1628;
        state_gas_after_3_1670 = _8_1366_8_1629;
        state_spill_after_3_1671 = _8_1367_8_1630;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    enum PrecompileId selected_precompile = precompile_id_for_address(target);
    if (neq_anything_EPrecompileId__(selected_precompile, NotPrecompile)) {
      Bytes input_memory = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
      struct CalldataSlice input = MemoryCalldata(input_memory);
      struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
      if (precompile_charge.affordable) {
        struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
        if (result.success) {
          returndata_after = result.output;
          if (semantics.transfers_value && value_nonzero) {
            k_transfer(carried_address, target, value_3_1673);
          }
          uint32_t return_destination = memory_absolute(carried_memory_base, ret_access.range.off);
          returndata_copy_prefix(returndata_after, return_destination, ret_access.range.len);
          gas_after = refund_gas(gas_after, (gas_sub(child_gas, precompile_charge.cost)));
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
          memory_after = mem1;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
        }
        returndata_after = returndata_clear();
        if (new_account_charged) {
          uint64_t _8_1365_8_1625;
          uint64_t _8_1366_8_1626;
          uint32_t _8_1367_8_1627 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1625, &_8_1366_8_1626);
          gas_after = _8_1365_8_1625;
          state_gas_after_3_1670 = _8_1366_8_1626;
          state_spill_after_3_1671 = _8_1367_8_1627;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      returndata_after = returndata_clear();
      if (new_account_charged) {
        uint64_t _8_1365_8_1622;
        uint64_t _8_1366_8_1623;
        uint32_t _8_1367_8_1624 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1622, &_8_1366_8_1623);
        gas_after = _8_1365_8_1622;
        state_gas_after_3_1670 = _8_1366_8_1623;
        state_spill_after_3_1671 = _8_1367_8_1624;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
    bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
    bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
    u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1673;
    Bytes bytes = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
    Bytes child_memory = evm_memory_slice(bytes.bytes, bytes.len);
    struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
    struct FrameStatus running = Running(UNIT);
    struct FrameCheckpoint _8_1553_8_1618;
    StackPointer _8_1554_8_1619;
    uint32_t _8_1555_8_1620;
    uint32_t _8_1556_8_1621 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1671, carried_refund, running, parent_message, carried_code, carried_calldata, &_8_1553_8_1618, &_8_1554_8_1619, &_8_1555_8_1620);
    frame_stack_push((ResumeCall(((struct CallContinuation){.checkpoint = _8_1553_8_1618, .new_account_charged = new_account_charged, .return_length = ret_access.range.len, .return_offset = ret_access.range.off}))));
    if (semantics.transfers_value && value_nonzero) {
      k_transfer(carried_address, target, value_3_1673);
    }
    Bytes child_returndata = returndata_clear();
    return ((struct FrameTransition){.calldata = child_calldata, .code = child_code, .gas_remaining = child_gas, .memory_base = _8_1555_8_1620, .memory_height = _8_1556_8_1621, .message = ((struct Message){.address = child_addr, .caller = child_caller, .code_address = target, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = (bool)(semantics.enters_static_context || carried_is_static), .state_gas_reservoir = state_gas_after_3_1670, .value = child_value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = _8_1554_8_1619, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
  }
  }
}

struct CreateSemantics create_semantics(enum CreateKind kind)
{
  switch (kind) {
  case CreateByNonce:
    return ((struct CreateSemantics){.uses_salt = false});
  case CreateBySalt:
    return ((struct CreateSemantics){.uses_salt = true});
  }
}

uint16_t create_stack_inputs(enum CreateKind kind)
{
  switch (kind) {
  case CreateByNonce:
    return UINT16_C(3);
  case CreateBySalt:
    return UINT16_C(4);
  }
}

struct FrameTransition run_create(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind)
{
  uint16_t stack_inputs = create_stack_inputs(kind);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, stack_inputs, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(stack_status.variants.Failed))});
    } else {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = (Exceptional(stack_status.variants.Failed))});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1510 = carried_state_gas;
    uint32_t state_spill_after_3_1511 = carried_state_spill;
    struct FrameStatus status_after_3_1512 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_66 = k_execution_profile;
    struct ProtocolProfileFields profile_8_67 = execution_profile_8_66.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 __sail_c_repr_u256_8_1795;
    StackPointer stackpointer_8_1796;
    if (semantics.uses_salt) {
      __sail_c_repr_u256_8_1795 = stack_slot_read(sp_after, UINT16_C(0));
      stackpointer_8_1796 = stack_top_retreat(sp_after, UINT16_C(1));
    } else {
      __sail_c_repr_u256_8_1795 = WORD_ZERO;
      stackpointer_8_1796 = sp_after;
    }
    sp_after = stackpointer_8_1796;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1514;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(WriteProtection))});
      } else {
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(WriteProtection))});
      }
      state_gas_after_3_1510 = exceptional_3_1514.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1514.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1514.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    uint64_t requested_height = memory_requested_height(off_word, len_word);
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))) <= gas_after) {
        expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))));
      } else {
        expansion_cost = GAS_CHARGE_UNAFFORDABLE;
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1515;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1510 = exceptional_3_1515.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1515.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1515.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    struct MemoryAccessFields initcode_access = memory_access(off_word, len_word);
    uint32_t mem1;
    if (initcode_access.requested_height <= (UINT32_C(4294967295) - carried_memory_base)) {
      if (memory_after < initcode_access.requested_height) {
        mem_expand(carried_memory_base, memory_after, initcode_access.requested_height);
        mem1 = initcode_access.requested_height;
      } else {
        mem1 = memory_after;
      }
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryRangeFields initcode = initcode_access.range;
    uint16_t access_cost = create_access_cost();
    if (gas_after < access_cost) {
      memory_after = mem1;
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1516;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1510 = exceptional_3_1516.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1516.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1516.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    if (eq_u256((word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}})), WORD_ZERO)) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    if (profile_8_67.fork >= Shanghai) {
      struct GasCharge initcode_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_initcode_word, initcode_word_count, gas_after);
      if (!initcode_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1517;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1510 = exceptional_3_1517.state_gas_remaining;
        state_spill_after_3_1511 = exceptional_3_1517.state_gas_spilled;
        status_after_3_1512 = exceptional_3_1517.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      gas_after = gas_sub(gas_after, initcode_cost.cost);
    }
    if (semantics.uses_salt) {
      struct GasCharge hashing_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_keccak_word, initcode_word_count, gas_after);
      if (!hashing_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1518;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1510 = exceptional_3_1518.state_gas_remaining;
        state_spill_after_3_1511 = exceptional_3_1518.state_gas_spilled;
        status_after_3_1512 = exceptional_3_1518.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      gas_after = gas_sub(gas_after, hashing_cost.cost);
    }
    if (initcode_size_allowed(initcode.len)) {
      uint64_t nonce = k_get_nonce(carried_address);
      bytes20 new_addr;
      if (semantics.uses_salt) {
        u256 initcode_digest_word = mem_keccak(carried_memory_base, mem1, initcode);
        new_addr = k_create2_addr(carried_address, __sail_c_repr_u256_8_1795, (word_to_hash(initcode_digest_word)));
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      if (profile_8_67.fork < Amsterdam) {
        uint64_t retained_gas = (gas_after / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(gas_after, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      if (!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024)))) {
        returndata_after = returndata_clear();
        if (profile_8_67.fork < Amsterdam) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      k_account_mark_warm(new_addr);
      bool new_account_charged = false;
      if (profile_8_67.fork >= Amsterdam) {
        new_account_charged = k_account_is_empty(new_addr);
      }
      if (new_account_charged) {
        bool _8_1511_8_1642;
        uint64_t _8_1512_8_1643;
        uint64_t _8_1513_8_1644;
        uint32_t _8_1514_8_1645 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1510, state_spill_after_3_1511, G_amsterdam_state_new_account, &_8_1511_8_1642, &_8_1512_8_1643, &_8_1513_8_1644);
        gas_after = _8_1512_8_1643;
        state_gas_after_3_1510 = _8_1513_8_1644;
        state_spill_after_3_1511 = _8_1514_8_1645;
        if (_8_1511_8_1642) {
          memory_after = mem1;
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1520;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1520 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1520 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
          }
          state_gas_after_3_1510 = exceptional_3_1520.state_gas_remaining;
          state_spill_after_3_1511 = exceptional_3_1520.state_gas_spilled;
          status_after_3_1512 = exceptional_3_1520.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
        }
      }
      if (profile_8_67.fork >= Amsterdam) {
        uint64_t retained_gas_3_1522 = (gas_after / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(gas_after, retained_gas_3_1522);
        gas_after = retained_gas_3_1522;
      }
      bool occupied = k_account_occupied(new_addr);
      returndata_after = returndata_clear();
      k_bump_nonce(carried_address);
      if (occupied) {
        if (new_account_charged) {
          uint64_t _8_1365_8_1639;
          uint64_t _8_1366_8_1640;
          uint32_t _8_1367_8_1641 = credit_state_gas_refund(gas_after, state_gas_after_3_1510, state_spill_after_3_1511, G_amsterdam_state_new_account, &_8_1365_8_1639, &_8_1366_8_1640);
          gas_after = _8_1365_8_1639;
          state_gas_after_3_1510 = _8_1366_8_1640;
          state_spill_after_3_1511 = _8_1367_8_1641;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      Bytes initcode_bytes = memory_code_slice(carried_memory_base, mem1, initcode.off, initcode.len);
      bytes32 child_code_id = code_db_insert(initcode_bytes, profile_8_67.fork);
      struct CodeFields child_code = code_db_resolve(child_code_id);
      struct FrameStatus running = Running(UNIT);
      struct FrameCheckpoint _8_1553_8_1635;
      StackPointer _8_1554_8_1636;
      uint32_t _8_1555_8_1637;
      uint32_t _8_1556_8_1638 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1511, carried_refund, running, parent_message, carried_code, carried_calldata, &_8_1553_8_1635, &_8_1554_8_1636, &_8_1555_8_1637);
      frame_stack_push((ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = _8_1553_8_1635, .new_account_charged = new_account_charged}))));
      k_mark_created(new_addr);
      k_clear_storage(new_addr);
      k_bump_nonce(new_addr);
      k_transfer(carried_address, new_addr, value);
      return ((struct FrameTransition){.calldata = EMPTY_CALLDATA, .code = child_code, .gas_remaining = child_gas, .memory_base = _8_1555_8_1637, .memory_height = _8_1556_8_1638, .message = ((struct Message){.address = new_addr, .caller = carried_address, .code_address = new_addr, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = carried_is_static, .state_gas_reservoir = state_gas_after_3_1510, .value = value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = (returndata_clear()), .stack_top = _8_1554_8_1636, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
    }
    memory_after = mem1;
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional_3_1519;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(InitCodeTooLarge))});
    } else {
      exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(InitCodeTooLarge))});
    }
    state_gas_after_3_1510 = exceptional_3_1519.state_gas_remaining;
    state_spill_after_3_1511 = exceptional_3_1519.state_gas_spilled;
    status_after_3_1512 = exceptional_3_1519.status;
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
  }
  }
}

struct FrameTransition resume_call(struct CallContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status)
{
  struct FrameCheckpoint checkpoint = continuation.checkpoint;
  bool succeeded = frame_succeeded(child_status);
  operand_stack_pop_frame();
  uint32_t parent_memory_base = memory_parent_base(child_memory_base, checkpoint.memory_height);
  uint64_t parent_gas = refund_gas(checkpoint.gas_remaining, child_gas);
  uint64_t parent_state_gas = checkpoint.state_gas_remaining;
  uint32_t parent_state_spill = checkpoint.state_gas_spilled;
  uint64_t _8_1368_8_1646;
  uint32_t _8_1369_8_1647 = return_child_state_gas(parent_state_gas, parent_state_spill, child_state_gas, child_state_spill, &_8_1368_8_1646);
  parent_state_gas = _8_1368_8_1646;
  parent_state_spill = _8_1369_8_1647;
  __int128 parent_refund = checkpoint.refund;
  StackPointer parent_sp = checkpoint.stack_top;
  uint32_t return_destination = memory_absolute(parent_memory_base, continuation.return_offset);
  returndata_copy_prefix(output, return_destination, continuation.return_length);
  if (succeeded) {
    parent_refund = record_refund(parent_refund, child_refund);
    k_journal_commit();
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), WORD_ONE);
  } else {
    k_journal_revert();
    if (continuation.new_account_charged) {
      uint64_t _8_1365_8_1648;
      uint64_t _8_1366_8_1649;
      uint32_t _8_1367_8_1650 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account, &_8_1365_8_1648, &_8_1366_8_1649);
      parent_gas = _8_1365_8_1648;
      parent_state_gas = _8_1366_8_1649;
      parent_state_spill = _8_1367_8_1650;
    }
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), WORD_ZERO);
  }
  return ((struct FrameTransition){.calldata = checkpoint.calldata, .code = checkpoint.code, .gas_remaining = parent_gas, .memory_base = parent_memory_base, .memory_height = checkpoint.memory_height, .message = checkpoint.message, .pc = checkpoint.pc, .refund = parent_refund, .returndata = output, .stack_top = parent_sp, .state_gas_remaining = parent_state_gas, .state_gas_spilled = parent_state_spill, .status = checkpoint.status});
}

struct FrameTransition resume_create(struct CreateContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct FrameCheckpoint checkpoint = continuation.checkpoint;
  bool initcode_succeeded = frame_succeeded(child_status);
  uint32_t deployed_length = returndata_size(output);
  uint64_t settled_child_gas = child_gas;
  uint64_t settled_child_state_gas = child_state_gas;
  uint32_t settled_child_state_spill = child_state_spill;
  struct FrameStatus settled_child_status = child_status;
  if (initcode_succeeded) {
    bool deployed_size_allowed = deployed_code_size_allowed(deployed_length);
    bool prohibited_prefix;
    if (deployed_length != UINT8_C(0)) {
      prohibited_prefix = (bool)((output_byte(output, UINT8_C(0))) == UINT64_C(0xEF));
    } else {
      prohibited_prefix = false;
    }
    if (!deployed_size_allowed || ((execution_profile.protocol.fork >= London) && prohibited_prefix)) {
      settled_child_gas = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = (Exceptional(OutOfGas))});
      }
      settled_child_state_gas = exceptional.state_gas_remaining;
      settled_child_state_spill = exceptional.state_gas_spilled;
      settled_child_status = exceptional.status;
    } else {
      struct GasCharge deployment_charge = code_deployment_execution_cost(deployed_length, settled_child_gas);
      if (deployment_charge.affordable) {
        settled_child_gas = gas_sub(settled_child_gas, deployment_charge.cost);
        uint64_t state_deposit = code_deployment_state_cost(deployed_length);
        bool deployment_halt = false;
        bool _8_1519_8_1656;
        uint64_t _8_1520_8_1657;
        uint64_t _8_1521_8_1658;
        uint32_t _8_1522_8_1659 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(settled_child_gas, settled_child_state_gas, settled_child_state_spill, state_deposit, &_8_1519_8_1656, &_8_1520_8_1657, &_8_1521_8_1658);
        deployment_halt = _8_1519_8_1656;
        settled_child_gas = _8_1520_8_1657;
        settled_child_state_gas = _8_1521_8_1658;
        settled_child_state_spill = _8_1522_8_1659;
        if (deployment_halt) {
          settled_child_gas = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1379;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1379 = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1379 = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = (Exceptional(OutOfGas))});
          }
          settled_child_state_gas = exceptional_3_1379.state_gas_remaining;
          settled_child_state_spill = exceptional_3_1379.state_gas_spilled;
          settled_child_status = exceptional_3_1379.status;
        }
      } else {
        settled_child_gas = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1380;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1380 = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1380 = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = (Exceptional(OutOfGas))});
        }
        settled_child_state_gas = exceptional_3_1380.state_gas_remaining;
        settled_child_state_spill = exceptional_3_1380.state_gas_spilled;
        settled_child_status = exceptional_3_1380.status;
      }
    }
  }
  bool deploy_succeeds = false;
  if (initcode_succeeded) {
    deploy_succeeds = frame_succeeded(settled_child_status);
  }
  operand_stack_pop_frame();
  uint32_t parent_memory_base = memory_parent_base(child_memory_base, checkpoint.memory_height);
  uint64_t parent_gas = refund_gas(checkpoint.gas_remaining, settled_child_gas);
  uint64_t parent_state_gas = checkpoint.state_gas_remaining;
  uint32_t parent_state_spill = checkpoint.state_gas_spilled;
  uint64_t _8_1368_8_1651;
  uint32_t _8_1369_8_1652 = return_child_state_gas(parent_state_gas, parent_state_spill, settled_child_state_gas, settled_child_state_spill, &_8_1368_8_1651);
  parent_state_gas = _8_1368_8_1651;
  parent_state_spill = _8_1369_8_1652;
  __int128 parent_refund = checkpoint.refund;
  StackPointer parent_sp = checkpoint.stack_top;
  if (deploy_succeeds) {
    parent_refund = record_refund(parent_refund, child_refund);
    Bytes deployed_code = code_db_intern_output(output);
    k_deploy_code(continuation.address, deployed_code);
    k_journal_commit();
    u256 deployed_address = address_to_word(continuation.address);
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), deployed_address);
  } else {
    k_journal_revert();
    if (continuation.new_account_charged) {
      uint64_t _8_1365_8_1653;
      uint64_t _8_1366_8_1654;
      uint32_t _8_1367_8_1655 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account, &_8_1365_8_1653, &_8_1366_8_1654);
      parent_gas = _8_1365_8_1653;
      parent_state_gas = _8_1366_8_1654;
      parent_state_spill = _8_1367_8_1655;
    }
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), WORD_ZERO);
  }
  Bytes parent_returndata;
  if (initcode_succeeded) {
    parent_returndata = returndata_clear();
  } else {
    parent_returndata = output;
  }
  return ((struct FrameTransition){.calldata = checkpoint.calldata, .code = checkpoint.code, .gas_remaining = parent_gas, .memory_base = parent_memory_base, .memory_height = checkpoint.memory_height, .message = checkpoint.message, .pc = checkpoint.pc, .refund = parent_refund, .returndata = parent_returndata, .stack_top = parent_sp, .state_gas_remaining = parent_state_gas, .state_gas_spilled = parent_state_spill, .status = checkpoint.status});
}

struct FrameTransition resume_frame(struct FrameContinuation continuation, Bytes output, uint32_t child_memory_base, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir)
{
  switch (continuation.kind) {
  case Kind_Empty:
    fatal_error(ExecutionInvalid);
  case Kind_ResumeCall:
    return resume_call(continuation.variants.ResumeCall, output, child_memory_base, child_gas, child_state_gas, child_state_spill, child_refund, child_status);
  case Kind_ResumeCreate:
    return resume_create(continuation.variants.ResumeCreate, output, child_memory_base, child_gas, child_state_gas, child_state_spill, child_refund, child_status, child_state_gas_reservoir);
  }
}

struct FrameTransition run_frame_entry_encoded(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, uint8_t opcode)
{
  if (opcode == UINT8_C(240)) {
    return run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CreateByNonce);
  }
  if (opcode == UINT8_C(241)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, Call);
  }
  if (opcode == UINT8_C(242)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CallCode);
  }
  if (opcode == UINT8_C(244)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, DelegateCall);
  }
  if (opcode == UINT8_C(245)) {
    return run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CreateBySalt);
  }
  if (opcode == UINT8_C(250)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_base, carried_memory_height, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, StaticCall);
  }
  struct ExceptionalStateTransition exceptional;
  if (k_execution_profile.protocol.fork >= Amsterdam) {
    exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(InvalidOpcode))});
  } else {
    exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = (Exceptional(InvalidOpcode))});
  }
  return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
}

Bytes interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint32_t initial_gas, uint8_t initial_state_gas, uint8_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint8_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata, uint64_t *restrict field_0_8_1523, uint64_t *restrict field_1_8_1524, uint32_t *restrict field_2_8_1525, __int128 *restrict field_3_8_1526, struct FrameStatus *restrict framestatus_8_1527)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, (k_header.excess_blob_gas));
  struct InterpreterCompletion completion = threaded_interpret((uint64_t)initial_gas, (uint64_t)initial_state_gas, (uint32_t)initial_state_spill, initial_refund, initial_sp, initial_memory_base, initial_memory_height, initial_caller, initial_address, initial_code_address, initial_value, (uint64_t)initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
  (*field_0_8_1523) = completion.gas_remaining;
  (*field_1_8_1524) = completion.state_gas_remaining;
  (*field_2_8_1525) = completion.state_gas_spilled;
  (*field_3_8_1526) = completion.refund;
  (*framestatus_8_1527) = completion.status;
  return completion.output;
}

Bytes interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint64_t initial_gas, uint64_t initial_state_gas, uint32_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint64_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata, uint64_t *restrict field_0_8_1529, uint64_t *restrict field_1_8_1530, uint32_t *restrict field_2_8_1531, __int128 *restrict field_3_8_1532, struct FrameStatus *restrict framestatus_8_1533)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, (k_header.excess_blob_gas));
  struct InterpreterCompletion completion = threaded_interpret(initial_gas, initial_state_gas, initial_state_spill, initial_refund, initial_sp, initial_memory_base, initial_memory_height, initial_caller, initial_address, initial_code_address, initial_value, initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
  (*field_0_8_1529) = completion.gas_remaining;
  (*field_1_8_1530) = completion.state_gas_remaining;
  (*field_2_8_1531) = completion.state_gas_spilled;
  (*field_3_8_1532) = completion.refund;
  (*framestatus_8_1533) = completion.status;
  return completion.output;
}

struct FrameTransition run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind)
{
  uint16_t stack_inputs = call_stack_inputs(kind);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, stack_inputs, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(stack_status.variants.Failed))});
    } else {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = (Exceptional(stack_status.variants.Failed))});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1670 = carried_state_gas;
    uint32_t state_spill_after_3_1671 = carried_state_spill;
    struct FrameStatus status_after_3_1672 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CallSemantics semantics = call_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_191 = k_execution_profile;
    struct ProtocolProfileFields profile_8_192 = execution_profile_8_191.protocol;
    u256 gas_request = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 target_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    bytes20 target = word_to_address(target_word);
    u256 __sail_c_repr_u256_8_1815;
    StackPointer stackpointer_8_1816;
    if (semantics.takes_value) {
      __sail_c_repr_u256_8_1815 = stack_slot_read(sp_after, UINT16_C(0));
      stackpointer_8_1816 = stack_top_retreat(sp_after, UINT16_C(1));
    } else {
      __sail_c_repr_u256_8_1815 = WORD_ZERO;
      stackpointer_8_1816 = sp_after;
    }
    u256 value_3_1673 = __sail_c_repr_u256_8_1815;
    sp_after = stackpointer_8_1816;
    bool value_nonzero = word_nonzero(value_3_1673);
    u256 args_off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 args_len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 ret_off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 ret_len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    if (semantics.transfers_value && (value_nonzero && carried_is_static)) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1674;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(WriteProtection))});
      } else {
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(WriteProtection))});
      }
      state_gas_after_3_1670 = exceptional_3_1674.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1674.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1674.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_requested_height = memory_requested_height(args_off_word, args_len_word);
    uint64_t ret_requested_height = memory_requested_height(ret_off_word, ret_len_word);
    uint64_t requested_height = args_requested_height < ret_requested_height ? ret_requested_height : args_requested_height;
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))) <= gas_after) {
        expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))));
      } else {
        expansion_cost = GAS_CHARGE_UNAFFORDABLE;
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1675;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1675.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1675.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1675.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    if (gas_after < static_base) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1676;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1676.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1676.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1676.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, static_base);
    k_account_mark_warm(target);
    bool tg_deleg;
    bytes20 tg_target = k_deleg_target(target, &tg_deleg);
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool new_account_charged = (bool)((profile_8_192.fork >= Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)));
    uint16_t create_cost = (profile_8_192.fork < Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)) ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    if (gas_after < additional_cost) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1677;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1670 = exceptional_3_1677.state_gas_remaining;
      state_spill_after_3_1671 = exceptional_3_1677.state_gas_spilled;
      status_after_3_1672 = exceptional_3_1677.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    gas_after = gas_sub(gas_after, additional_cost);
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    if (profile_8_192.fork >= Amsterdam) {
      if (new_account_charged) {
        bool _8_1511_8_1771;
        uint64_t _8_1512_8_1772;
        uint64_t _8_1513_8_1773;
        uint32_t _8_1514_8_1774 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1511_8_1771, &_8_1512_8_1772, &_8_1513_8_1773);
        gas_after = _8_1512_8_1772;
        state_gas_after_3_1670 = _8_1513_8_1773;
        state_spill_after_3_1671 = _8_1514_8_1774;
        if (_8_1511_8_1771) {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1678;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
          }
          state_gas_after_3_1670 = exceptional_3_1678.state_gas_remaining;
          state_spill_after_3_1671 = exceptional_3_1678.state_gas_spilled;
          status_after_3_1672 = exceptional_3_1678.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1679;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1670 = exceptional_3_1679.state_gas_remaining;
        state_spill_after_3_1671 = exceptional_3_1679.state_gas_spilled;
        status_after_3_1672 = exceptional_3_1679.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      gas_after = gas_sub(gas_after, base_child);
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1680;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1680 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1680 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1670 = exceptional_3_1680.state_gas_remaining;
        state_spill_after_3_1671 = exceptional_3_1680.state_gas_spilled;
        status_after_3_1672 = exceptional_3_1680.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      gas_after = gas_sub(gas_after, base_child);
    }
    if (tg_deleg) {
      k_account_mark_warm(tg_target);
    }
    if (tg_deleg) {
      bytes32 delegate_key = k_code_key(tg_target);
      code_db_resolve(delegate_key);
      k_aload_(tg_target);
    }
    struct MemoryAccessFields args_access = memory_access(args_off_word, args_len_word);
    struct MemoryAccessFields ret_access = memory_access(ret_off_word, ret_len_word);
    uint32_t materialized_required_size = args_access.requested_height < ret_access.requested_height ? ret_access.requested_height : args_access.requested_height;
    uint32_t mem1;
    if (materialized_required_size <= (UINT32_C(4294967295) - carried_memory_base)) {
      if (memory_after < materialized_required_size) {
        mem_expand(carried_memory_base, memory_after, materialized_required_size);
        mem1 = materialized_required_size;
      } else {
        mem1 = memory_after;
      }
    } else {
      fatal_error(ExecutionInvalid);
    }
    uint64_t child_gas = conserved_gas_add(base_child, stipend);
    k_aload_(target);
    bool insufficient_balance;
    if (semantics.takes_value && value_nonzero) {
      u256 caller_balance = k_get_balance(carried_address);
      insufficient_balance = (bool)(!(word_ule(value_3_1673, caller_balance)));
    } else {
      insufficient_balance = false;
    }
    if (insufficient_balance || (carried_depth == UINT16_C(1024))) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (new_account_charged) {
        uint64_t _8_1365_8_1768;
        uint64_t _8_1366_8_1769;
        uint32_t _8_1367_8_1770 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1768, &_8_1366_8_1769);
        gas_after = _8_1365_8_1768;
        state_gas_after_3_1670 = _8_1366_8_1769;
        state_spill_after_3_1671 = _8_1367_8_1770;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    enum PrecompileId selected_precompile = precompile_id_for_address(target);
    if (neq_anything_EPrecompileId__(selected_precompile, NotPrecompile)) {
      Bytes input_memory = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
      struct CalldataSlice input = MemoryCalldata(input_memory);
      struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
      if (precompile_charge.affordable) {
        struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
        if (result.success) {
          returndata_after = result.output;
          if (semantics.transfers_value && value_nonzero) {
            k_transfer(carried_address, target, value_3_1673);
          }
          uint32_t return_destination = memory_absolute(carried_memory_base, ret_access.range.off);
          returndata_copy_prefix(returndata_after, return_destination, ret_access.range.len);
          gas_after = refund_gas(gas_after, (gas_sub(child_gas, precompile_charge.cost)));
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
          memory_after = mem1;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
        }
        returndata_after = returndata_clear();
        if (new_account_charged) {
          uint64_t _8_1365_8_1765;
          uint64_t _8_1366_8_1766;
          uint32_t _8_1367_8_1767 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1765, &_8_1366_8_1766);
          gas_after = _8_1365_8_1765;
          state_gas_after_3_1670 = _8_1366_8_1766;
          state_spill_after_3_1671 = _8_1367_8_1767;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
      }
      returndata_after = returndata_clear();
      if (new_account_charged) {
        uint64_t _8_1365_8_1762;
        uint64_t _8_1366_8_1763;
        uint32_t _8_1367_8_1764 = credit_state_gas_refund(gas_after, state_gas_after_3_1670, state_spill_after_3_1671, G_amsterdam_state_new_account, &_8_1365_8_1762, &_8_1366_8_1763);
        gas_after = _8_1365_8_1762;
        state_gas_after_3_1670 = _8_1366_8_1763;
        state_spill_after_3_1671 = _8_1367_8_1764;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = state_spill_after_3_1671, .status = status_after_3_1672});
    }
    struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
    bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
    bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
    u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1673;
    Bytes bytes = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
    Bytes child_memory = evm_memory_slice(bytes.bytes, bytes.len);
    struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
    struct FrameStatus running = Running(UNIT);
    struct FrameCheckpoint _8_1553_8_1758;
    StackPointer _8_1554_8_1759;
    uint32_t _8_1555_8_1760;
    uint32_t _8_1556_8_1761 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1671, carried_refund, running, parent_message, carried_code, carried_calldata, &_8_1553_8_1758, &_8_1554_8_1759, &_8_1555_8_1760);
    frame_stack_push((ResumeCall(((struct CallContinuation){.checkpoint = _8_1553_8_1758, .new_account_charged = new_account_charged, .return_length = ret_access.range.len, .return_offset = ret_access.range.off}))));
    if (semantics.transfers_value && value_nonzero) {
      k_transfer(carried_address, target, value_3_1673);
    }
    Bytes child_returndata = returndata_clear();
    return ((struct FrameTransition){.calldata = child_calldata, .code = child_code, .gas_remaining = child_gas, .memory_base = _8_1555_8_1760, .memory_height = _8_1556_8_1761, .message = ((struct Message){.address = child_addr, .caller = child_caller, .code_address = target, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = (bool)(semantics.enters_static_context || carried_is_static), .state_gas_reservoir = state_gas_after_3_1670, .value = child_value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = _8_1554_8_1759, .state_gas_remaining = state_gas_after_3_1670, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
  }
  }
}

struct FrameTransition run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind)
{
  uint16_t stack_inputs = create_stack_inputs(kind);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, stack_inputs, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(stack_status.variants.Failed))});
    } else {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = (Exceptional(stack_status.variants.Failed))});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1510 = carried_state_gas;
    uint32_t state_spill_after_3_1511 = carried_state_spill;
    struct FrameStatus status_after_3_1512 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_252 = k_execution_profile;
    struct ProtocolProfileFields profile_8_253 = execution_profile_8_252.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 __sail_c_repr_u256_8_1817;
    StackPointer stackpointer_8_1818;
    if (semantics.uses_salt) {
      __sail_c_repr_u256_8_1817 = stack_slot_read(sp_after, UINT16_C(0));
      stackpointer_8_1818 = stack_top_retreat(sp_after, UINT16_C(1));
    } else {
      __sail_c_repr_u256_8_1817 = WORD_ZERO;
      stackpointer_8_1818 = sp_after;
    }
    sp_after = stackpointer_8_1818;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1514;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(WriteProtection))});
      } else {
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(WriteProtection))});
      }
      state_gas_after_3_1510 = exceptional_3_1514.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1514.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1514.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    uint64_t requested_height = memory_requested_height(off_word, len_word);
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))) <= gas_after) {
        expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + ((uint64_t)G_memory * (uint64_t)new_words)) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + ((uint64_t)G_memory * (uint64_t)old_words))));
      } else {
        expansion_cost = GAS_CHARGE_UNAFFORDABLE;
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1515;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1510 = exceptional_3_1515.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1515.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1515.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    struct MemoryAccessFields initcode_access = memory_access(off_word, len_word);
    uint32_t mem1;
    if (initcode_access.requested_height <= (UINT32_C(4294967295) - carried_memory_base)) {
      if (memory_after < initcode_access.requested_height) {
        mem_expand(carried_memory_base, memory_after, initcode_access.requested_height);
        mem1 = initcode_access.requested_height;
      } else {
        mem1 = memory_after;
      }
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryRangeFields initcode = initcode_access.range;
    uint16_t access_cost = create_access_cost();
    if (gas_after < access_cost) {
      memory_after = mem1;
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1516;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after_3_1510 = exceptional_3_1516.state_gas_remaining;
      state_spill_after_3_1511 = exceptional_3_1516.state_gas_spilled;
      status_after_3_1512 = exceptional_3_1516.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
    }
    gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    if (eq_u256((word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}})), WORD_ZERO)) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    if (profile_8_253.fork >= Shanghai) {
      struct GasCharge initcode_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_initcode_word, initcode_word_count, gas_after);
      if (!initcode_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1517;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1510 = exceptional_3_1517.state_gas_remaining;
        state_spill_after_3_1511 = exceptional_3_1517.state_gas_spilled;
        status_after_3_1512 = exceptional_3_1517.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      gas_after = gas_sub(gas_after, initcode_cost.cost);
    }
    if (semantics.uses_salt) {
      struct GasCharge hashing_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_keccak_word, initcode_word_count, gas_after);
      if (!hashing_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1518;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after_3_1510 = exceptional_3_1518.state_gas_remaining;
        state_spill_after_3_1511 = exceptional_3_1518.state_gas_spilled;
        status_after_3_1512 = exceptional_3_1518.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      gas_after = gas_sub(gas_after, hashing_cost.cost);
    }
    if (initcode_size_allowed(initcode.len)) {
      uint64_t nonce = k_get_nonce(carried_address);
      bytes20 new_addr;
      if (semantics.uses_salt) {
        u256 initcode_digest_word = mem_keccak(carried_memory_base, mem1, initcode);
        new_addr = k_create2_addr(carried_address, __sail_c_repr_u256_8_1817, (word_to_hash(initcode_digest_word)));
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      if (profile_8_253.fork < Amsterdam) {
        uint64_t retained_gas = (gas_after / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(gas_after, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      if (!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024)))) {
        returndata_after = returndata_clear();
        if (profile_8_253.fork < Amsterdam) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      k_account_mark_warm(new_addr);
      bool new_account_charged = false;
      if (profile_8_253.fork >= Amsterdam) {
        new_account_charged = k_account_is_empty(new_addr);
      }
      if (new_account_charged) {
        bool _8_1511_8_1782;
        uint64_t _8_1512_8_1783;
        uint64_t _8_1513_8_1784;
        uint32_t _8_1514_8_1785 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1510, state_spill_after_3_1511, G_amsterdam_state_new_account, &_8_1511_8_1782, &_8_1512_8_1783, &_8_1513_8_1784);
        gas_after = _8_1512_8_1783;
        state_gas_after_3_1510 = _8_1513_8_1784;
        state_spill_after_3_1511 = _8_1514_8_1785;
        if (_8_1511_8_1782) {
          memory_after = mem1;
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1520;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1520 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1520 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(OutOfGas))});
          }
          state_gas_after_3_1510 = exceptional_3_1520.state_gas_remaining;
          state_spill_after_3_1511 = exceptional_3_1520.state_gas_spilled;
          status_after_3_1512 = exceptional_3_1520.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
        }
      }
      if (profile_8_253.fork >= Amsterdam) {
        uint64_t retained_gas_3_1522 = (gas_after / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(gas_after, retained_gas_3_1522);
        gas_after = retained_gas_3_1522;
      }
      bool occupied = k_account_occupied(new_addr);
      returndata_after = returndata_clear();
      k_bump_nonce(carried_address);
      if (occupied) {
        if (new_account_charged) {
          uint64_t _8_1365_8_1779;
          uint64_t _8_1366_8_1780;
          uint32_t _8_1367_8_1781 = credit_state_gas_refund(gas_after, state_gas_after_3_1510, state_spill_after_3_1511, G_amsterdam_state_new_account, &_8_1365_8_1779, &_8_1366_8_1780);
          gas_after = _8_1365_8_1779;
          state_gas_after_3_1510 = _8_1366_8_1780;
          state_spill_after_3_1511 = _8_1367_8_1781;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
      }
      Bytes initcode_bytes = memory_code_slice(carried_memory_base, mem1, initcode.off, initcode.len);
      bytes32 child_code_id = code_db_insert(initcode_bytes, profile_8_253.fork);
      struct CodeFields child_code = code_db_resolve(child_code_id);
      struct FrameStatus running = Running(UNIT);
      struct FrameCheckpoint _8_1553_8_1775;
      StackPointer _8_1554_8_1776;
      uint32_t _8_1555_8_1777;
      uint32_t _8_1556_8_1778 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1511, carried_refund, running, parent_message, carried_code, carried_calldata, &_8_1553_8_1775, &_8_1554_8_1776, &_8_1555_8_1777);
      frame_stack_push((ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = _8_1553_8_1775, .new_account_charged = new_account_charged}))));
      k_mark_created(new_addr);
      k_clear_storage(new_addr);
      k_bump_nonce(new_addr);
      k_transfer(carried_address, new_addr, value);
      return ((struct FrameTransition){.calldata = EMPTY_CALLDATA, .code = child_code, .gas_remaining = child_gas, .memory_base = _8_1555_8_1777, .memory_height = _8_1556_8_1778, .message = ((struct Message){.address = new_addr, .caller = carried_address, .code_address = new_addr, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = carried_is_static, .state_gas_reservoir = state_gas_after_3_1510, .value = value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = (returndata_clear()), .stack_top = _8_1554_8_1776, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
    }
    memory_after = mem1;
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional_3_1519;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(InitCodeTooLarge))});
    } else {
      exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = (Exceptional(InitCodeTooLarge))});
    }
    state_gas_after_3_1510 = exceptional_3_1519.state_gas_remaining;
    state_spill_after_3_1511 = exceptional_3_1519.state_gas_spilled;
    status_after_3_1512 = exceptional_3_1519.status;
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1510, .state_gas_spilled = state_spill_after_3_1511, .status = status_after_3_1512});
  }
  }
}

