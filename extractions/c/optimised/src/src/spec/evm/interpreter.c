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
    struct tuple_uint_32_u256 decode_push_immediate_result_2_1117;
    decode_push_immediate_result_2_1117.tup1 = decode_push_immediate(frame_code, immediate_offset, width);
    decode_push_immediate_result_2_1117.tup0 = (*immediate_offset);
    return execute_push(execution_gas, sp, width, decode_push_immediate_result_2_1117.tup1);
    (*immediate_offset) = decode_push_immediate_result_2_1117.tup0;
  }
  return execute_invalid(execution_gas);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(128) <= opcode) && (opcode <= UINT8_C(143))) {
    return execute_dup(execution_gas, sp, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(127))));
  }
  return execute_invalid(execution_gas);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap_encoded(uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(144) <= opcode) && (opcode <= UINT8_C(159))) {
    return execute_swap(execution_gas, sp, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(143))));
  }
  return execute_invalid(execution_gas);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log_encoded(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t opcode, uint64_t *restrict execution_gas, StackPointer *restrict sp, uint32_t *restrict memory)
{
  if ((UINT8_C(160) <= opcode) && (opcode <= UINT8_C(164))) {
    return execute_log(carried_address, carried_is_static, memory_base, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(160))), execution_gas, sp, memory);
  }
  return execute_invalid(execution_gas);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_deep_stack_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  enum DeepStackOperation operation = deep_stack_operation(opcode);
  struct tuple_uint_32_bits_8 decode_deep_immediate_result_2_1101;
  decode_deep_immediate_result_2_1101.tup1 = decode_deep_immediate(frame_code, immediate_offset, operation);
  decode_deep_immediate_result_2_1101.tup0 = (*immediate_offset);
  struct tuple_uint_64_StackPointer_OpcodeOutcome tmp_3_1711;
  switch (operation) {
  case DeepStackDuplicate:
    tmp_3_1711.tup2 = execute_dupn(execution_gas, sp, decode_deep_immediate_result_2_1101.tup1);
    tmp_3_1711.tup0 = (*execution_gas);
    tmp_3_1711.tup1 = (*sp);
    break;
  case DeepStackSwap:
    tmp_3_1711.tup2 = execute_swapn(execution_gas, sp, decode_deep_immediate_result_2_1101.tup1);
    tmp_3_1711.tup0 = (*execution_gas);
    tmp_3_1711.tup1 = (*sp);
    break;
  case DeepStackExchange:
    tmp_3_1711.tup2 = execute_exchange(execution_gas, sp, decode_deep_immediate_result_2_1101.tup1);
    tmp_3_1711.tup0 = (*execution_gas);
    tmp_3_1711.tup1 = (*sp);
    break;
  case NotDeepStackOperation:
  {
    struct tuple_uint_64_OpcodeOutcome execute_invalid_result_2_1102;
    execute_invalid_result_2_1102.tup1 = execute_invalid(execution_gas);
    execute_invalid_result_2_1102.tup0 = (*execution_gas);
    tmp_3_1711 = ((struct tuple_uint_64_StackPointer_OpcodeOutcome){.tup0 = execute_invalid_result_2_1102.tup0, .tup1 = (*sp), .tup2 = execute_invalid_result_2_1102.tup1});
    break;
  }
  }
  (*immediate_offset) = decode_deep_immediate_result_2_1101.tup0;
  (*execution_gas) = tmp_3_1711.tup0;
  (*sp) = tmp_3_1711.tup1;
  return tmp_3_1711.tup2;
}

Bytes frame_output(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1800;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltReturn) {
    goto case_1800;
  }
  return frame_status.variants.Halted.variants.HaltReturn;
case_1800: ;
  if (frame_status.kind != Kind_Halted) {
    goto case_1799;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1799;
  }
  return frame_status.variants.Halted.variants.HaltRevert;
case_1799: ;
  return EMPTY_OUTPUT_SLICE;
}

bool frame_succeeded(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1792;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1792;
  }
  return false;
case_1792: ;
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
    bool neq_anything_result_2_1097 = neq_anything_EPrecompileId__(delegate_precompile, NotPrecompile);
    if (neq_anything_result_2_1097) {
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

__attribute__((__always_inline__)) uint16_t call_stack_inputs(enum CallKind kind)
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
  uint16_t tmp_3_1686;
  switch (kind) {
  case Call:
  case CallCode:
    tmp_3_1686 = UINT16_C(7);
    break;
  case DelegateCall:
  case StaticCall:
    tmp_3_1686 = UINT16_C(6);
    break;
  }
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1686, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    if (execution_profile.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_2_2021 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
    } else {
      struct FrameStatus Exceptional_result_2_2022 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = Exceptional_result_2_2022});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1669 = carried_state_gas;
    uint32_t state_spill_after_3_1670 = carried_state_spill;
    struct FrameStatus status_after_3_1671 = Running(UNIT);
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
    struct tuple_u256_StackPointer result_2_1041;
    if (semantics.takes_value) {
      u256 value = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1040 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1041 = ((struct tuple_u256_StackPointer){.tup0 = value, .tup1 = stack_top_retreat_result_2_1040});
    } else {
      result_2_1041 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    u256 value_3_1672 = result_2_1041.tup0;
    sp_after = result_2_1041.tup1;
    bool value_nonzero = word_nonzero(value_3_1672);
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
      struct ExceptionalStateTransition exceptional_3_1673;
      struct ExecutionProfileFields execution_profile_8_7 = k_execution_profile;
      if (execution_profile_8_7.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_11 = Exceptional(WriteProtection);
        exceptional_3_1673 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_11});
      } else {
        struct FrameStatus Exceptional_result_8_13 = Exceptional(WriteProtection);
        exceptional_3_1673 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_13});
      }
      state_gas_after_3_1669 = exceptional_3_1673.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1673.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1673.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_requested_height;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      uint64_t tmp_3_3061;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(args_len_word);
        tmp_3_3061 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size));
      } else {
        tmp_3_3061 = UINT64_C(4294967296);
      }
      args_requested_height = tmp_3_3061;
    } else {
      args_requested_height = UINT64_C(4294967296);
    }
    uint64_t ret_requested_height;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      uint64_t tmp_8_17;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_22 = (uint32_t)u256_to_u64(ret_len_word);
        tmp_8_17 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_22));
      } else {
        tmp_8_17 = UINT64_C(4294967296);
      }
      ret_requested_height = tmp_8_17;
    } else {
      ret_requested_height = UINT64_C(4294967296);
    }
    uint64_t requested_height = args_requested_height < ret_requested_height ? ret_requested_height : args_requested_height;
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_24 = ((uint64_t)G_memory * (uint64_t)new_words);
        if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_24) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)) <= gas_after) {
          expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_24) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)));
        } else {
          expansion_cost = GAS_CHARGE_UNAFFORDABLE;
        }
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1674;
      struct ExecutionProfileFields execution_profile_8_27 = k_execution_profile;
      if (execution_profile_8_27.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_31 = Exceptional(OutOfGas);
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_31});
      } else {
        struct FrameStatus Exceptional_result_8_33 = Exceptional(OutOfGas);
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_33});
      }
      state_gas_after_3_1669 = exceptional_3_1674.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1674.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1674.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    if (gas_after < static_base) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1675;
      struct ExecutionProfileFields execution_profile_8_35 = k_execution_profile;
      if (execution_profile_8_35.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_39 = Exceptional(OutOfGas);
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_39});
      } else {
        struct FrameStatus Exceptional_result_8_41 = Exceptional(OutOfGas);
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_41});
      }
      state_gas_after_3_1669 = exceptional_3_1675.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1675.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1675.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, static_base);
    k_account_mark_warm(target);
    struct tuple_bool_bytes20 k_deleg_target_result_2_1048 = k_deleg_target(target);
    bool tg_deleg = k_deleg_target_result_2_1048.tup0;
    bytes20 tg_target = k_deleg_target_result_2_1048.tup1;
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool result_2_1095 = (bool)(profile_8_6.fork >= Amsterdam);
    bool tmp_3_1568;
    if (result_2_1095) {
      bool tmp_3_1567 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1568 = tmp_3_1567;
    } else {
      tmp_3_1568 = false;
    }
    uint16_t create_cost;
    bool result_2_1091 = (bool)(profile_8_6.fork < Amsterdam);
    bool tmp_3_1571;
    if (result_2_1091) {
      bool tmp_3_1570 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1571 = tmp_3_1570;
    } else {
      tmp_3_1571 = false;
    }
    create_cost = tmp_3_1571 ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    if (gas_after < additional_cost) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1676;
      struct ExecutionProfileFields execution_profile_8_43 = k_execution_profile;
      if (execution_profile_8_43.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_47 = Exceptional(OutOfGas);
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_47});
      } else {
        struct FrameStatus Exceptional_result_8_49 = Exceptional(OutOfGas);
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_49});
      }
      state_gas_after_3_1669 = exceptional_3_1676.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1676.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1676.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, additional_cost);
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    if (profile_8_6.fork >= Amsterdam) {
      if (tmp_3_1568) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1052 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1052.tup1;
        state_gas_after_3_1669 = charge_state_gas_result_2_1052.tup2;
        state_spill_after_3_1670 = charge_state_gas_result_2_1052.tup3;
        if (charge_state_gas_result_2_1052.tup0) {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1677;
          struct ExecutionProfileFields execution_profile_8_51 = k_execution_profile;
          if (execution_profile_8_51.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_55 = Exceptional(OutOfGas);
            exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_55});
          } else {
            struct FrameStatus Exceptional_result_8_57 = Exceptional(OutOfGas);
            exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_57});
          }
          state_gas_after_3_1669 = exceptional_3_1677.state_gas_remaining;
          state_spill_after_3_1670 = exceptional_3_1677.state_gas_spilled;
          status_after_3_1671 = exceptional_3_1677.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1678;
        struct ExecutionProfileFields execution_profile_8_59 = k_execution_profile;
        if (execution_profile_8_59.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_63 = Exceptional(OutOfGas);
          exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_63});
        } else {
          struct FrameStatus Exceptional_result_8_65 = Exceptional(OutOfGas);
          exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_65});
        }
        state_gas_after_3_1669 = exceptional_3_1678.state_gas_remaining;
        state_spill_after_3_1670 = exceptional_3_1678.state_gas_spilled;
        status_after_3_1671 = exceptional_3_1678.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
      }
      gas_after = gas_sub(gas_after, base_child);
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1679;
        struct ExecutionProfileFields execution_profile_8_67 = k_execution_profile;
        if (execution_profile_8_67.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_71 = Exceptional(OutOfGas);
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_71});
        } else {
          struct FrameStatus Exceptional_result_8_73 = Exceptional(OutOfGas);
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_73});
        }
        state_gas_after_3_1669 = exceptional_3_1679.state_gas_remaining;
        state_spill_after_3_1670 = exceptional_3_1679.state_gas_spilled;
        status_after_3_1671 = exceptional_3_1679.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
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
    struct MemoryAccessFields args_access;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      struct MemoryAccessFields tmp_3_3056;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size_8_76 = (uint32_t)u256_to_u64(args_len_word);
        struct MemoryRangeFields memory_range_result_2_1935 = memory_range((uint32_t)u256_to_u64_unchecked(args_off_word), bounded_size_8_76);
        uint32_t add_atom_result_2_1936 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size_8_76));
        tmp_3_3056 = ((struct MemoryAccessFields){.range = memory_range_result_2_1935, .requested_height = add_atom_result_2_1936});
      } else {
        fatal_error(ExecutionInvalid);
      }
      args_access = tmp_3_3056;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryAccessFields ret_access;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      struct MemoryAccessFields tmp_8_79;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_84 = (uint32_t)u256_to_u64(ret_len_word);
        struct MemoryRangeFields memory_range_result_8_86 = memory_range((uint32_t)u256_to_u64_unchecked(ret_off_word), bounded_size_8_84);
        uint32_t add_atom_result_8_87 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_84));
        tmp_8_79 = ((struct MemoryAccessFields){.range = memory_range_result_8_86, .requested_height = add_atom_result_8_87});
      } else {
        fatal_error(ExecutionInvalid);
      }
      ret_access = tmp_8_79;
    } else {
      fatal_error(ExecutionInvalid);
    }
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
      bool transfer_affordable = word_ule(value_3_1672, caller_balance);
      insufficient_balance = (bool)(!transfer_affordable);
    } else {
      insufficient_balance = false;
    }
    if (insufficient_balance || (carried_depth == UINT16_C(1024))) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (tmp_3_1568) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1059 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = result_2_1059.tup0;
        state_gas_after_3_1669 = result_2_1059.tup1;
        state_spill_after_3_1670 = result_2_1059.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    enum PrecompileId selected_precompile = precompile_id_for_address(target);
    bool neq_anything_result_2_1060 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
    if (neq_anything_result_2_1060) {
      Bytes input_memory = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
      struct CalldataSlice input = MemoryCalldata(input_memory);
      struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
      if (precompile_charge.affordable) {
        struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
        if (result.success) {
          returndata_after = result.output;
          if (semantics.transfers_value && value_nonzero) {
            k_transfer(carried_address, target, value_3_1672);
          }
          uint32_t return_destination = memory_absolute(carried_memory_base, ret_access.range.off);
          returndata_copy_prefix(returndata_after, return_destination, ret_access.range.len);
          uint64_t unused = gas_sub(child_gas, precompile_charge.cost);
          gas_after = refund_gas(gas_after, unused);
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
          memory_after = mem1;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
        }
        returndata_after = returndata_clear();
        if (tmp_3_1568) {
          struct tuple_uint_64_uint_64_uint_32 result_2_1067 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
          gas_after = result_2_1067.tup0;
          state_gas_after_3_1669 = result_2_1067.tup1;
          state_spill_after_3_1670 = result_2_1067.tup2;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
      }
      returndata_after = returndata_clear();
      if (tmp_3_1568) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1068 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = result_2_1068.tup0;
        state_gas_after_3_1669 = result_2_1068.tup1;
        state_spill_after_3_1670 = result_2_1068.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
    bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
    bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
    u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1672;
    bool child_static = (bool)(semantics.enters_static_context || carried_is_static);
    Bytes bytes = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
    Bytes child_memory = evm_memory_slice(bytes.bytes, bytes.len);
    struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
    struct FrameStatus running = Running(UNIT);
    struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 suspend_frame_result_2_1071 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1670, carried_refund, running, parent_message, carried_code, carried_calldata);
    struct FrameContinuation continuation = ResumeCall(((struct CallContinuation){.checkpoint = suspend_frame_result_2_1071.tup0, .new_account_charged = tmp_3_1568, .return_length = ret_access.range.len, .return_offset = ret_access.range.off}));
    frame_stack_push(continuation);
    if (semantics.transfers_value && value_nonzero) {
      k_transfer(carried_address, target, value_3_1672);
    }
    Bytes child_returndata = returndata_clear();
    return ((struct FrameTransition){.calldata = child_calldata, .code = child_code, .gas_remaining = child_gas, .memory_base = suspend_frame_result_2_1071.tup2, .memory_height = suspend_frame_result_2_1071.tup3, .message = ((struct Message){.address = child_addr, .caller = child_caller, .code_address = target, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = child_static, .state_gas_reservoir = state_gas_after_3_1669, .value = child_value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = suspend_frame_result_2_1071.tup1, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
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

__attribute__((__always_inline__)) uint16_t create_stack_inputs(enum CreateKind kind)
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
  uint16_t tmp_3_1522;
  switch (kind) {
  case CreateByNonce:
    tmp_3_1522 = UINT16_C(3);
    break;
  case CreateBySalt:
    tmp_3_1522 = UINT16_C(4);
    break;
  }
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1522, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    if (execution_profile.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_2_2021 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
    } else {
      struct FrameStatus Exceptional_result_2_2022 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = Exceptional_result_2_2022});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1509 = carried_state_gas;
    uint32_t state_spill_after_3_1510 = carried_state_spill;
    struct FrameStatus status_after_3_1511 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_90 = k_execution_profile;
    struct ProtocolProfileFields profile_8_91 = execution_profile_8_90.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    struct tuple_u256_StackPointer result_2_1006;
    if (semantics.uses_salt) {
      u256 salt = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1005 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1006 = ((struct tuple_u256_StackPointer){.tup0 = salt, .tup1 = stack_top_retreat_result_2_1005});
    } else {
      result_2_1006 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    sp_after = result_2_1006.tup1;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1513;
      struct ExecutionProfileFields execution_profile_8_92 = k_execution_profile;
      if (execution_profile_8_92.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_96 = Exceptional(WriteProtection);
        exceptional_3_1513 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_96});
      } else {
        struct FrameStatus Exceptional_result_8_98 = Exceptional(WriteProtection);
        exceptional_3_1513 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_98});
      }
      state_gas_after_3_1509 = exceptional_3_1513.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1513.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1513.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    uint64_t requested_height;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      uint64_t tmp_3_3061;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(len_word);
        tmp_3_3061 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size));
      } else {
        tmp_3_3061 = UINT64_C(4294967296);
      }
      requested_height = tmp_3_3061;
    } else {
      requested_height = UINT64_C(4294967296);
    }
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_100 = ((uint64_t)G_memory * (uint64_t)new_words);
        if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_100) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)) <= gas_after) {
          expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_100) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)));
        } else {
          expansion_cost = GAS_CHARGE_UNAFFORDABLE;
        }
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1514;
      struct ExecutionProfileFields execution_profile_8_103 = k_execution_profile;
      if (execution_profile_8_103.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_107 = Exceptional(OutOfGas);
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_107});
      } else {
        struct FrameStatus Exceptional_result_8_109 = Exceptional(OutOfGas);
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_109});
      }
      state_gas_after_3_1509 = exceptional_3_1514.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1514.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1514.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    struct MemoryAccessFields initcode_access;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      initcode_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      struct MemoryAccessFields tmp_3_3056;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size_8_112 = (uint32_t)u256_to_u64(len_word);
        struct MemoryRangeFields memory_range_result_2_1935 = memory_range((uint32_t)u256_to_u64_unchecked(off_word), bounded_size_8_112);
        uint32_t add_atom_result_2_1936 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size_8_112));
        tmp_3_3056 = ((struct MemoryAccessFields){.range = memory_range_result_2_1935, .requested_height = add_atom_result_2_1936});
      } else {
        fatal_error(ExecutionInvalid);
      }
      initcode_access = tmp_3_3056;
    } else {
      fatal_error(ExecutionInvalid);
    }
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
      struct ExceptionalStateTransition exceptional_3_1515;
      struct ExecutionProfileFields execution_profile_8_113 = k_execution_profile;
      if (execution_profile_8_113.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_117 = Exceptional(OutOfGas);
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_117});
      } else {
        struct FrameStatus Exceptional_result_8_119 = Exceptional(OutOfGas);
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_119});
      }
      state_gas_after_3_1509 = exceptional_3_1515.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1515.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1515.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    u256 remainder = word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    if (eq_u256(remainder, WORD_ZERO)) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    if (profile_8_91.fork >= Shanghai) {
      struct GasCharge initcode_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_initcode_word, initcode_word_count, gas_after);
      if (!initcode_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1516;
        struct ExecutionProfileFields execution_profile_8_121 = k_execution_profile;
        if (execution_profile_8_121.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_125 = Exceptional(OutOfGas);
          exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_125});
        } else {
          struct FrameStatus Exceptional_result_8_127 = Exceptional(OutOfGas);
          exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_127});
        }
        state_gas_after_3_1509 = exceptional_3_1516.state_gas_remaining;
        state_spill_after_3_1510 = exceptional_3_1516.state_gas_spilled;
        status_after_3_1511 = exceptional_3_1516.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      gas_after = gas_sub(gas_after, initcode_cost.cost);
    }
    if (semantics.uses_salt) {
      struct GasCharge hashing_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_keccak_word, initcode_word_count, gas_after);
      if (!hashing_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1517;
        struct ExecutionProfileFields execution_profile_8_129 = k_execution_profile;
        if (execution_profile_8_129.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_133 = Exceptional(OutOfGas);
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_133});
        } else {
          struct FrameStatus Exceptional_result_8_135 = Exceptional(OutOfGas);
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_135});
        }
        state_gas_after_3_1509 = exceptional_3_1517.state_gas_remaining;
        state_spill_after_3_1510 = exceptional_3_1517.state_gas_spilled;
        status_after_3_1511 = exceptional_3_1517.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      gas_after = gas_sub(gas_after, hashing_cost.cost);
    }
    bool valid_initcode_size = initcode_size_allowed(initcode.len);
    if (valid_initcode_size) {
      uint64_t nonce = k_get_nonce(carried_address);
      bytes20 new_addr;
      if (semantics.uses_salt) {
        u256 initcode_digest_word = mem_keccak(carried_memory_base, mem1, initcode);
        bytes32 initcode_digest = word_to_hash(initcode_digest_word);
        new_addr = k_create2_addr(carried_address, result_2_1006.tup0, initcode_digest);
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      if (profile_8_91.fork < Amsterdam) {
        uint64_t avail = gas_after;
        uint64_t retained_gas = (avail / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(avail, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      if (!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024)))) {
        returndata_after = returndata_clear();
        if (profile_8_91.fork < Amsterdam) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      k_account_mark_warm(new_addr);
      bool result_2_1035 = (bool)(profile_8_91.fork >= Amsterdam);
      bool tmp_3_1457;
      if (result_2_1035) {
        tmp_3_1457 = k_account_is_empty(new_addr);
      } else {
        tmp_3_1457 = false;
      }
      if (tmp_3_1457) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1026 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1509, state_spill_after_3_1510, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1026.tup1;
        state_gas_after_3_1509 = charge_state_gas_result_2_1026.tup2;
        state_spill_after_3_1510 = charge_state_gas_result_2_1026.tup3;
        if (charge_state_gas_result_2_1026.tup0) {
          memory_after = mem1;
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1519;
          struct ExecutionProfileFields execution_profile_8_145 = k_execution_profile;
          if (execution_profile_8_145.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_149 = Exceptional(OutOfGas);
            exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_149});
          } else {
            struct FrameStatus Exceptional_result_8_151 = Exceptional(OutOfGas);
            exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_151});
          }
          state_gas_after_3_1509 = exceptional_3_1519.state_gas_remaining;
          state_spill_after_3_1510 = exceptional_3_1519.state_gas_spilled;
          status_after_3_1511 = exceptional_3_1519.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
        }
      }
      if (profile_8_91.fork >= Amsterdam) {
        uint64_t avail_3_1520 = gas_after;
        uint64_t retained_gas_3_1521 = (avail_3_1520 / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(avail_3_1520, retained_gas_3_1521);
        gas_after = retained_gas_3_1521;
      }
      bool occupied = k_account_occupied(new_addr);
      returndata_after = returndata_clear();
      k_bump_nonce(carried_address);
      if (occupied) {
        if (tmp_3_1457) {
          struct tuple_uint_64_uint_64_uint_32 result_2_1029 = credit_state_gas_refund(gas_after, state_gas_after_3_1509, state_spill_after_3_1510, G_amsterdam_state_new_account);
          gas_after = result_2_1029.tup0;
          state_gas_after_3_1509 = result_2_1029.tup1;
          state_spill_after_3_1510 = result_2_1029.tup2;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      Bytes initcode_bytes = memory_code_slice(carried_memory_base, mem1, initcode.off, initcode.len);
      bytes32 child_code_id = code_db_insert(initcode_bytes, profile_8_91.fork);
      struct CodeFields child_code = code_db_resolve(child_code_id);
      struct FrameStatus running = Running(UNIT);
      struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 suspend_frame_result_2_1030 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1510, carried_refund, running, parent_message, carried_code, carried_calldata);
      struct FrameContinuation continuation = ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = suspend_frame_result_2_1030.tup0, .new_account_charged = tmp_3_1457}));
      frame_stack_push(continuation);
      k_mark_created(new_addr);
      k_clear_storage(new_addr);
      k_bump_nonce(new_addr);
      k_transfer(carried_address, new_addr, value);
      Bytes child_returndata = returndata_clear();
      return ((struct FrameTransition){.calldata = EMPTY_CALLDATA, .code = child_code, .gas_remaining = child_gas, .memory_base = suspend_frame_result_2_1030.tup2, .memory_height = suspend_frame_result_2_1030.tup3, .message = ((struct Message){.address = new_addr, .caller = carried_address, .code_address = new_addr, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = carried_is_static, .state_gas_reservoir = state_gas_after_3_1509, .value = value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = suspend_frame_result_2_1030.tup1, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
    }
    memory_after = mem1;
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional_3_1518;
    struct ExecutionProfileFields execution_profile_8_137 = k_execution_profile;
    if (execution_profile_8_137.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_8_141 = Exceptional(InitCodeTooLarge);
      exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_141});
    } else {
      struct FrameStatus Exceptional_result_8_143 = Exceptional(InitCodeTooLarge);
      exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_143});
    }
    state_gas_after_3_1509 = exceptional_3_1518.state_gas_remaining;
    state_spill_after_3_1510 = exceptional_3_1518.state_gas_spilled;
    status_after_3_1511 = exceptional_3_1518.status;
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
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
  struct tuple_uint_64_uint_32 result_2_991 = return_child_state_gas(parent_state_gas, parent_state_spill, child_state_gas, child_state_spill);
  parent_state_gas = result_2_991.tup0;
  parent_state_spill = result_2_991.tup1;
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
      struct tuple_uint_64_uint_64_uint_32 result_2_994 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account);
      parent_gas = result_2_994.tup0;
      parent_state_gas = result_2_994.tup1;
      parent_state_spill = result_2_994.tup2;
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
      uint64_t first_byte = output_byte(output, UINT8_C(0));
      prohibited_prefix = (bool)(first_byte == UINT64_C(0xEF));
    } else {
      prohibited_prefix = false;
    }
    bool tmp_3_1342;
    if (deployed_size_allowed) {
      bool result_2_971 = (bool)(execution_profile.protocol.fork >= London);
      tmp_3_1342 = (bool)(result_2_971 && prohibited_prefix);
    } else {
      tmp_3_1342 = true;
    }
    if (tmp_3_1342) {
      settled_child_gas = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional;
      struct ExecutionProfileFields execution_profile_8_153 = k_execution_profile;
      if (execution_profile_8_153.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_2_2021 = Exceptional(OutOfGas);
        exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
      } else {
        struct FrameStatus Exceptional_result_2_2022 = Exceptional(OutOfGas);
        exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = Exceptional_result_2_2022});
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
        struct tuple_bool_uint_64_uint_64_uint_32 result_2_974 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(settled_child_gas, settled_child_state_gas, settled_child_state_spill, state_deposit);
        deployment_halt = result_2_974.tup0;
        settled_child_gas = result_2_974.tup1;
        settled_child_state_gas = result_2_974.tup2;
        settled_child_state_spill = result_2_974.tup3;
        if (deployment_halt) {
          settled_child_gas = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1378;
          struct ExecutionProfileFields execution_profile_8_155 = k_execution_profile;
          if (execution_profile_8_155.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_159 = Exceptional(OutOfGas);
            exceptional_3_1378 = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_159});
          } else {
            struct FrameStatus Exceptional_result_8_161 = Exceptional(OutOfGas);
            exceptional_3_1378 = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = Exceptional_result_8_161});
          }
          settled_child_state_gas = exceptional_3_1378.state_gas_remaining;
          settled_child_state_spill = exceptional_3_1378.state_gas_spilled;
          settled_child_status = exceptional_3_1378.status;
        }
      } else {
        settled_child_gas = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1379;
        struct ExecutionProfileFields execution_profile_8_163 = k_execution_profile;
        if (execution_profile_8_163.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_167 = Exceptional(OutOfGas);
          exceptional_3_1379 = ((struct ExceptionalStateTransition){.state_gas_remaining = child_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_167});
        } else {
          struct FrameStatus Exceptional_result_8_169 = Exceptional(OutOfGas);
          exceptional_3_1379 = ((struct ExceptionalStateTransition){.state_gas_remaining = settled_child_state_gas, .state_gas_spilled = settled_child_state_spill, .status = Exceptional_result_8_169});
        }
        settled_child_state_gas = exceptional_3_1379.state_gas_remaining;
        settled_child_state_spill = exceptional_3_1379.state_gas_spilled;
        settled_child_status = exceptional_3_1379.status;
      }
    }
  }
  bool tmp_3_1359;
  if (initcode_succeeded) {
    tmp_3_1359 = frame_succeeded(settled_child_status);
  } else {
    tmp_3_1359 = false;
  }
  operand_stack_pop_frame();
  uint32_t parent_memory_base = memory_parent_base(child_memory_base, checkpoint.memory_height);
  uint64_t parent_gas = refund_gas(checkpoint.gas_remaining, settled_child_gas);
  uint64_t parent_state_gas = checkpoint.state_gas_remaining;
  uint32_t parent_state_spill = checkpoint.state_gas_spilled;
  struct tuple_uint_64_uint_32 result_2_978 = return_child_state_gas(parent_state_gas, parent_state_spill, settled_child_state_gas, settled_child_state_spill);
  parent_state_gas = result_2_978.tup0;
  parent_state_spill = result_2_978.tup1;
  __int128 parent_refund = checkpoint.refund;
  StackPointer parent_sp = checkpoint.stack_top;
  if (tmp_3_1359) {
    parent_refund = record_refund(parent_refund, child_refund);
    Bytes deployed_bytes = output;
    Bytes deployed_code = code_db_intern_output(deployed_bytes);
    k_deploy_code(continuation.address, deployed_code);
    k_journal_commit();
    u256 deployed_address = address_to_word(continuation.address);
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), deployed_address);
  } else {
    k_journal_revert();
    if (continuation.new_account_charged) {
      struct tuple_uint_64_uint_64_uint_32 result_2_982 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account);
      parent_gas = result_2_982.tup0;
      parent_state_gas = result_2_982.tup1;
      parent_state_spill = result_2_982.tup2;
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
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork >= Amsterdam) {
    struct FrameStatus Exceptional_result_2_2021 = Exceptional(InvalidOpcode);
    exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
  } else {
    struct FrameStatus Exceptional_result_2_2022 = Exceptional(InvalidOpcode);
    exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = Exceptional_result_2_2022});
  }
  return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint32_t initial_gas, uint8_t initial_state_gas, uint8_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint8_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee;
  uint32_t result_2_1100 = k_header.excess_blob_gas;
  blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, result_2_1100);
  return threaded_interpret((uint64_t)initial_gas, (uint64_t)initial_state_gas, (uint32_t)initial_state_spill, initial_refund, initial_sp, initial_memory_base, initial_memory_height, initial_caller, initial_address, initial_code_address, initial_value, (uint64_t)initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint64_t initial_gas, uint64_t initial_state_gas, uint32_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, uint32_t initial_memory_base, uint32_t initial_memory_height, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint64_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee;
  uint32_t result_2_1100 = k_header.excess_blob_gas;
  blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, result_2_1100);
  return threaded_interpret(initial_gas, initial_state_gas, initial_state_spill, initial_refund, initial_sp, initial_memory_base, initial_memory_height, initial_caller, initial_address, initial_code_address, initial_value, initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
}

struct FrameTransition run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind)
{
  uint16_t tmp_3_1686;
  switch (kind) {
  case Call:
  case CallCode:
    tmp_3_1686 = UINT16_C(7);
    break;
  case DelegateCall:
  case StaticCall:
    tmp_3_1686 = UINT16_C(6);
    break;
  }
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1686, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    if (execution_profile.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_2_2021 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
    } else {
      struct FrameStatus Exceptional_result_2_2022 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = Exceptional_result_2_2022});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1669 = carried_state_gas;
    uint32_t state_spill_after_3_1670 = carried_state_spill;
    struct FrameStatus status_after_3_1671 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CallSemantics semantics = call_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_217 = k_execution_profile;
    struct ProtocolProfileFields profile_8_218 = execution_profile_8_217.protocol;
    u256 gas_request = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 target_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    bytes20 target = word_to_address(target_word);
    struct tuple_u256_StackPointer result_2_1041;
    if (semantics.takes_value) {
      u256 value = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1040 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1041 = ((struct tuple_u256_StackPointer){.tup0 = value, .tup1 = stack_top_retreat_result_2_1040});
    } else {
      result_2_1041 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    u256 value_3_1672 = result_2_1041.tup0;
    sp_after = result_2_1041.tup1;
    bool value_nonzero = word_nonzero(value_3_1672);
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
      struct ExceptionalStateTransition exceptional_3_1673;
      struct ExecutionProfileFields execution_profile_8_219 = k_execution_profile;
      if (execution_profile_8_219.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_223 = Exceptional(WriteProtection);
        exceptional_3_1673 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_223});
      } else {
        struct FrameStatus Exceptional_result_8_225 = Exceptional(WriteProtection);
        exceptional_3_1673 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_225});
      }
      state_gas_after_3_1669 = exceptional_3_1673.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1673.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1673.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_requested_height;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      uint64_t tmp_3_3061;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(args_len_word);
        tmp_3_3061 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size));
      } else {
        tmp_3_3061 = UINT64_C(4294967296);
      }
      args_requested_height = tmp_3_3061;
    } else {
      args_requested_height = UINT64_C(4294967296);
    }
    uint64_t ret_requested_height;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      uint64_t tmp_8_229;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_234 = (uint32_t)u256_to_u64(ret_len_word);
        tmp_8_229 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_234));
      } else {
        tmp_8_229 = UINT64_C(4294967296);
      }
      ret_requested_height = tmp_8_229;
    } else {
      ret_requested_height = UINT64_C(4294967296);
    }
    uint64_t requested_height = args_requested_height < ret_requested_height ? ret_requested_height : args_requested_height;
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_236 = ((uint64_t)G_memory * (uint64_t)new_words);
        if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_236) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)) <= gas_after) {
          expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_236) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)));
        } else {
          expansion_cost = GAS_CHARGE_UNAFFORDABLE;
        }
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1674;
      struct ExecutionProfileFields execution_profile_8_239 = k_execution_profile;
      if (execution_profile_8_239.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_243 = Exceptional(OutOfGas);
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_243});
      } else {
        struct FrameStatus Exceptional_result_8_245 = Exceptional(OutOfGas);
        exceptional_3_1674 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_245});
      }
      state_gas_after_3_1669 = exceptional_3_1674.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1674.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1674.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    if (gas_after < static_base) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1675;
      struct ExecutionProfileFields execution_profile_8_247 = k_execution_profile;
      if (execution_profile_8_247.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_251 = Exceptional(OutOfGas);
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_251});
      } else {
        struct FrameStatus Exceptional_result_8_253 = Exceptional(OutOfGas);
        exceptional_3_1675 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_253});
      }
      state_gas_after_3_1669 = exceptional_3_1675.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1675.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1675.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, static_base);
    k_account_mark_warm(target);
    struct tuple_bool_bytes20 k_deleg_target_result_2_1048 = k_deleg_target(target);
    bool tg_deleg = k_deleg_target_result_2_1048.tup0;
    bytes20 tg_target = k_deleg_target_result_2_1048.tup1;
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool result_2_1095 = (bool)(profile_8_218.fork >= Amsterdam);
    bool tmp_3_1568;
    if (result_2_1095) {
      bool tmp_3_1567 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1568 = tmp_3_1567;
    } else {
      tmp_3_1568 = false;
    }
    uint16_t create_cost;
    bool result_2_1091 = (bool)(profile_8_218.fork < Amsterdam);
    bool tmp_3_1571;
    if (result_2_1091) {
      bool tmp_3_1570 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1571 = tmp_3_1570;
    } else {
      tmp_3_1571 = false;
    }
    create_cost = tmp_3_1571 ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    if (gas_after < additional_cost) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1676;
      struct ExecutionProfileFields execution_profile_8_255 = k_execution_profile;
      if (execution_profile_8_255.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_259 = Exceptional(OutOfGas);
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_259});
      } else {
        struct FrameStatus Exceptional_result_8_261 = Exceptional(OutOfGas);
        exceptional_3_1676 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_261});
      }
      state_gas_after_3_1669 = exceptional_3_1676.state_gas_remaining;
      state_spill_after_3_1670 = exceptional_3_1676.state_gas_spilled;
      status_after_3_1671 = exceptional_3_1676.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    gas_after = gas_sub(gas_after, additional_cost);
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    if (profile_8_218.fork >= Amsterdam) {
      if (tmp_3_1568) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1052 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1052.tup1;
        state_gas_after_3_1669 = charge_state_gas_result_2_1052.tup2;
        state_spill_after_3_1670 = charge_state_gas_result_2_1052.tup3;
        if (charge_state_gas_result_2_1052.tup0) {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1677;
          struct ExecutionProfileFields execution_profile_8_263 = k_execution_profile;
          if (execution_profile_8_263.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_267 = Exceptional(OutOfGas);
            exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_267});
          } else {
            struct FrameStatus Exceptional_result_8_269 = Exceptional(OutOfGas);
            exceptional_3_1677 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_269});
          }
          state_gas_after_3_1669 = exceptional_3_1677.state_gas_remaining;
          state_spill_after_3_1670 = exceptional_3_1677.state_gas_spilled;
          status_after_3_1671 = exceptional_3_1677.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1678;
        struct ExecutionProfileFields execution_profile_8_271 = k_execution_profile;
        if (execution_profile_8_271.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_275 = Exceptional(OutOfGas);
          exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_275});
        } else {
          struct FrameStatus Exceptional_result_8_277 = Exceptional(OutOfGas);
          exceptional_3_1678 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_277});
        }
        state_gas_after_3_1669 = exceptional_3_1678.state_gas_remaining;
        state_spill_after_3_1670 = exceptional_3_1678.state_gas_spilled;
        status_after_3_1671 = exceptional_3_1678.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
      }
      gas_after = gas_sub(gas_after, base_child);
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      if (gas_after < base_child) {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1679;
        struct ExecutionProfileFields execution_profile_8_279 = k_execution_profile;
        if (execution_profile_8_279.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_283 = Exceptional(OutOfGas);
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_283});
        } else {
          struct FrameStatus Exceptional_result_8_285 = Exceptional(OutOfGas);
          exceptional_3_1679 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = Exceptional_result_8_285});
        }
        state_gas_after_3_1669 = exceptional_3_1679.state_gas_remaining;
        state_spill_after_3_1670 = exceptional_3_1679.state_gas_spilled;
        status_after_3_1671 = exceptional_3_1679.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
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
    struct MemoryAccessFields args_access;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      struct MemoryAccessFields tmp_3_3056;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size_8_288 = (uint32_t)u256_to_u64(args_len_word);
        struct MemoryRangeFields memory_range_result_2_1935 = memory_range((uint32_t)u256_to_u64_unchecked(args_off_word), bounded_size_8_288);
        uint32_t add_atom_result_2_1936 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size_8_288));
        tmp_3_3056 = ((struct MemoryAccessFields){.range = memory_range_result_2_1935, .requested_height = add_atom_result_2_1936});
      } else {
        fatal_error(ExecutionInvalid);
      }
      args_access = tmp_3_3056;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryAccessFields ret_access;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      struct MemoryAccessFields tmp_8_291;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_296 = (uint32_t)u256_to_u64(ret_len_word);
        struct MemoryRangeFields memory_range_result_8_298 = memory_range((uint32_t)u256_to_u64_unchecked(ret_off_word), bounded_size_8_296);
        uint32_t add_atom_result_8_299 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_296));
        tmp_8_291 = ((struct MemoryAccessFields){.range = memory_range_result_8_298, .requested_height = add_atom_result_8_299});
      } else {
        fatal_error(ExecutionInvalid);
      }
      ret_access = tmp_8_291;
    } else {
      fatal_error(ExecutionInvalid);
    }
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
      bool transfer_affordable = word_ule(value_3_1672, caller_balance);
      insufficient_balance = (bool)(!transfer_affordable);
    } else {
      insufficient_balance = false;
    }
    if (insufficient_balance || (carried_depth == UINT16_C(1024))) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (tmp_3_1568) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1059 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = result_2_1059.tup0;
        state_gas_after_3_1669 = result_2_1059.tup1;
        state_spill_after_3_1670 = result_2_1059.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    enum PrecompileId selected_precompile = precompile_id_for_address(target);
    bool neq_anything_result_2_1060 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
    if (neq_anything_result_2_1060) {
      Bytes input_memory = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
      struct CalldataSlice input = MemoryCalldata(input_memory);
      struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
      if (precompile_charge.affordable) {
        struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
        if (result.success) {
          returndata_after = result.output;
          if (semantics.transfers_value && value_nonzero) {
            k_transfer(carried_address, target, value_3_1672);
          }
          uint32_t return_destination = memory_absolute(carried_memory_base, ret_access.range.off);
          returndata_copy_prefix(returndata_after, return_destination, ret_access.range.len);
          uint64_t unused = gas_sub(child_gas, precompile_charge.cost);
          gas_after = refund_gas(gas_after, unused);
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
          memory_after = mem1;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
        }
        returndata_after = returndata_clear();
        if (tmp_3_1568) {
          struct tuple_uint_64_uint_64_uint_32 result_2_1067 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
          gas_after = result_2_1067.tup0;
          state_gas_after_3_1669 = result_2_1067.tup1;
          state_spill_after_3_1670 = result_2_1067.tup2;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
      }
      returndata_after = returndata_clear();
      if (tmp_3_1568) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1068 = credit_state_gas_refund(gas_after, state_gas_after_3_1669, state_spill_after_3_1670, G_amsterdam_state_new_account);
        gas_after = result_2_1068.tup0;
        state_gas_after_3_1669 = result_2_1068.tup1;
        state_spill_after_3_1670 = result_2_1068.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = mem1;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = state_spill_after_3_1670, .status = status_after_3_1671});
    }
    struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
    bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
    bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
    u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1672;
    bool child_static = (bool)(semantics.enters_static_context || carried_is_static);
    Bytes bytes = active_memory_slice(carried_memory_base, mem1, args_access.range.off, args_access.range.len);
    Bytes child_memory = evm_memory_slice(bytes.bytes, bytes.len);
    struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
    struct FrameStatus running = Running(UNIT);
    struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 suspend_frame_result_2_1071 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1670, carried_refund, running, parent_message, carried_code, carried_calldata);
    struct FrameContinuation continuation = ResumeCall(((struct CallContinuation){.checkpoint = suspend_frame_result_2_1071.tup0, .new_account_charged = tmp_3_1568, .return_length = ret_access.range.len, .return_offset = ret_access.range.off}));
    frame_stack_push(continuation);
    if (semantics.transfers_value && value_nonzero) {
      k_transfer(carried_address, target, value_3_1672);
    }
    Bytes child_returndata = returndata_clear();
    return ((struct FrameTransition){.calldata = child_calldata, .code = child_code, .gas_remaining = child_gas, .memory_base = suspend_frame_result_2_1071.tup2, .memory_height = suspend_frame_result_2_1071.tup3, .message = ((struct Message){.address = child_addr, .caller = child_caller, .code_address = target, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = child_static, .state_gas_reservoir = state_gas_after_3_1669, .value = child_value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = suspend_frame_result_2_1071.tup1, .state_gas_remaining = state_gas_after_3_1669, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
  }
  }
}

struct FrameTransition run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_FrameTransition(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind)
{
  uint16_t tmp_3_1522;
  switch (kind) {
  case CreateByNonce:
    tmp_3_1522 = UINT16_C(3);
    break;
  case CreateBySalt:
    tmp_3_1522 = UINT16_C(4);
    break;
  }
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1522, UINT8_C(1));
  switch (stack_status.kind) {
  case Kind_Failed:
  {
    struct ExceptionalStateTransition exceptional;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    if (execution_profile.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_2_2021 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
    } else {
      struct FrameStatus Exceptional_result_2_2022 = Exceptional(stack_status.variants.Failed);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas, .state_gas_spilled = carried_state_spill, .status = Exceptional_result_2_2022});
    }
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = (uint64_t)GAS_ZERO, .memory_base = carried_memory_base, .memory_height = carried_memory_height, .message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), .pc = carried_pc, .refund = carried_refund, .returndata = carried_returndata, .stack_top = carried_sp, .state_gas_remaining = exceptional.state_gas_remaining, .state_gas_spilled = exceptional.state_gas_spilled, .status = exceptional.status});
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1509 = carried_state_gas;
    uint32_t state_spill_after_3_1510 = carried_state_spill;
    struct FrameStatus status_after_3_1511 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    uint32_t memory_after = carried_memory_height;
    Bytes returndata_after = carried_returndata;
    struct Message parent_message = ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value});
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_302 = k_execution_profile;
    struct ProtocolProfileFields profile_8_303 = execution_profile_8_302.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    struct tuple_u256_StackPointer result_2_1006;
    if (semantics.uses_salt) {
      u256 salt = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1005 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1006 = ((struct tuple_u256_StackPointer){.tup0 = salt, .tup1 = stack_top_retreat_result_2_1005});
    } else {
      result_2_1006 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    sp_after = result_2_1006.tup1;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1513;
      struct ExecutionProfileFields execution_profile_8_304 = k_execution_profile;
      if (execution_profile_8_304.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_308 = Exceptional(WriteProtection);
        exceptional_3_1513 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_308});
      } else {
        struct FrameStatus Exceptional_result_8_310 = Exceptional(WriteProtection);
        exceptional_3_1513 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_310});
      }
      state_gas_after_3_1509 = exceptional_3_1513.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1513.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1513.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    uint64_t requested_height;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      requested_height = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      uint64_t tmp_3_3061;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(len_word);
        tmp_3_3061 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size));
      } else {
        tmp_3_3061 = UINT64_C(4294967296);
      }
      requested_height = tmp_3_3061;
    } else {
      requested_height = UINT64_C(4294967296);
    }
    struct GasCharge expansion_cost;
    if (requested_height <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)requested_height);
      uint32_t old_size = memory_high_water(memory_after);
      uint32_t old_words = memory_word_count_uint32_t_to_uint32_t(old_size);
      if (new_words <= old_words) {
        expansion_cost = gas_charge(GAS_COST_ZERO);
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_312 = ((uint64_t)G_memory * (uint64_t)new_words);
        if ((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_312) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)) <= gas_after) {
          expansion_cost = gas_charge((((((uint64_t)new_words * (uint64_t)new_words) >> 9) + linear_8_312) - ((((uint64_t)old_words * (uint64_t)old_words) >> 9) + linear)));
        } else {
          expansion_cost = GAS_CHARGE_UNAFFORDABLE;
        }
      }
    } else {
      expansion_cost = GAS_CHARGE_UNAFFORDABLE;
    }
    if (!expansion_cost.affordable) {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1514;
      struct ExecutionProfileFields execution_profile_8_315 = k_execution_profile;
      if (execution_profile_8_315.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_319 = Exceptional(OutOfGas);
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_319});
      } else {
        struct FrameStatus Exceptional_result_8_321 = Exceptional(OutOfGas);
        exceptional_3_1514 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_321});
      }
      state_gas_after_3_1509 = exceptional_3_1514.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1514.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1514.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    gas_after = gas_sub(gas_after, expansion_cost.cost);
    struct MemoryAccessFields initcode_access;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      initcode_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      struct MemoryAccessFields tmp_3_3056;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size_8_324 = (uint32_t)u256_to_u64(len_word);
        struct MemoryRangeFields memory_range_result_2_1935 = memory_range((uint32_t)u256_to_u64_unchecked(off_word), bounded_size_8_324);
        uint32_t add_atom_result_2_1936 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size_8_324));
        tmp_3_3056 = ((struct MemoryAccessFields){.range = memory_range_result_2_1935, .requested_height = add_atom_result_2_1936});
      } else {
        fatal_error(ExecutionInvalid);
      }
      initcode_access = tmp_3_3056;
    } else {
      fatal_error(ExecutionInvalid);
    }
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
      struct ExceptionalStateTransition exceptional_3_1515;
      struct ExecutionProfileFields execution_profile_8_325 = k_execution_profile;
      if (execution_profile_8_325.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_329 = Exceptional(OutOfGas);
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_329});
      } else {
        struct FrameStatus Exceptional_result_8_331 = Exceptional(OutOfGas);
        exceptional_3_1515 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_331});
      }
      state_gas_after_3_1509 = exceptional_3_1515.state_gas_remaining;
      state_spill_after_3_1510 = exceptional_3_1515.state_gas_spilled;
      status_after_3_1511 = exceptional_3_1515.status;
      return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
    }
    gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    u256 remainder = word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    if (eq_u256(remainder, WORD_ZERO)) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    if (profile_8_303.fork >= Shanghai) {
      struct GasCharge initcode_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_initcode_word, initcode_word_count, gas_after);
      if (!initcode_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1516;
        struct ExecutionProfileFields execution_profile_8_333 = k_execution_profile;
        if (execution_profile_8_333.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_337 = Exceptional(OutOfGas);
          exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_337});
        } else {
          struct FrameStatus Exceptional_result_8_339 = Exceptional(OutOfGas);
          exceptional_3_1516 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_339});
        }
        state_gas_after_3_1509 = exceptional_3_1516.state_gas_remaining;
        state_spill_after_3_1510 = exceptional_3_1516.state_gas_spilled;
        status_after_3_1511 = exceptional_3_1516.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      gas_after = gas_sub(gas_after, initcode_cost.cost);
    }
    if (semantics.uses_salt) {
      struct GasCharge hashing_cost = word_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_GasCharge(G_keccak_word, initcode_word_count, gas_after);
      if (!hashing_cost.affordable) {
        memory_after = mem1;
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1517;
        struct ExecutionProfileFields execution_profile_8_341 = k_execution_profile;
        if (execution_profile_8_341.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_345 = Exceptional(OutOfGas);
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_345});
        } else {
          struct FrameStatus Exceptional_result_8_347 = Exceptional(OutOfGas);
          exceptional_3_1517 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_347});
        }
        state_gas_after_3_1509 = exceptional_3_1517.state_gas_remaining;
        state_spill_after_3_1510 = exceptional_3_1517.state_gas_spilled;
        status_after_3_1511 = exceptional_3_1517.status;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      gas_after = gas_sub(gas_after, hashing_cost.cost);
    }
    bool valid_initcode_size = initcode_size_allowed(initcode.len);
    if (valid_initcode_size) {
      uint64_t nonce = k_get_nonce(carried_address);
      bytes20 new_addr;
      if (semantics.uses_salt) {
        u256 initcode_digest_word = mem_keccak(carried_memory_base, mem1, initcode);
        bytes32 initcode_digest = word_to_hash(initcode_digest_word);
        new_addr = k_create2_addr(carried_address, result_2_1006.tup0, initcode_digest);
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      if (profile_8_303.fork < Amsterdam) {
        uint64_t avail = gas_after;
        uint64_t retained_gas = (avail / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(avail, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      if (!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024)))) {
        returndata_after = returndata_clear();
        if (profile_8_303.fork < Amsterdam) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      k_account_mark_warm(new_addr);
      bool result_2_1035 = (bool)(profile_8_303.fork >= Amsterdam);
      bool tmp_3_1457;
      if (result_2_1035) {
        tmp_3_1457 = k_account_is_empty(new_addr);
      } else {
        tmp_3_1457 = false;
      }
      if (tmp_3_1457) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1026 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1509, state_spill_after_3_1510, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1026.tup1;
        state_gas_after_3_1509 = charge_state_gas_result_2_1026.tup2;
        state_spill_after_3_1510 = charge_state_gas_result_2_1026.tup3;
        if (charge_state_gas_result_2_1026.tup0) {
          memory_after = mem1;
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1519;
          struct ExecutionProfileFields execution_profile_8_357 = k_execution_profile;
          if (execution_profile_8_357.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_361 = Exceptional(OutOfGas);
            exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_361});
          } else {
            struct FrameStatus Exceptional_result_8_363 = Exceptional(OutOfGas);
            exceptional_3_1519 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_363});
          }
          state_gas_after_3_1509 = exceptional_3_1519.state_gas_remaining;
          state_spill_after_3_1510 = exceptional_3_1519.state_gas_spilled;
          status_after_3_1511 = exceptional_3_1519.status;
          return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
        }
      }
      if (profile_8_303.fork >= Amsterdam) {
        uint64_t avail_3_1520 = gas_after;
        uint64_t retained_gas_3_1521 = (avail_3_1520 / (uint64_t)UINT8_C(64));
        child_gas = gas_sub(avail_3_1520, retained_gas_3_1521);
        gas_after = retained_gas_3_1521;
      }
      bool occupied = k_account_occupied(new_addr);
      returndata_after = returndata_clear();
      k_bump_nonce(carried_address);
      if (occupied) {
        if (tmp_3_1457) {
          struct tuple_uint_64_uint_64_uint_32 result_2_1029 = credit_state_gas_refund(gas_after, state_gas_after_3_1509, state_spill_after_3_1510, G_amsterdam_state_new_account);
          gas_after = result_2_1029.tup0;
          state_gas_after_3_1509 = result_2_1029.tup1;
          state_spill_after_3_1510 = result_2_1029.tup2;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem1;
        return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
      }
      Bytes initcode_bytes = memory_code_slice(carried_memory_base, mem1, initcode.off, initcode.len);
      bytes32 child_code_id = code_db_insert(initcode_bytes, profile_8_303.fork);
      struct CodeFields child_code = code_db_resolve(child_code_id);
      struct FrameStatus running = Running(UNIT);
      struct tuple_FrameCheckpoint_StackPointer_uint_32_uint_32 suspend_frame_result_2_1030 = suspend_frame(carried_pc, gas_after, sp_after, carried_memory_base, mem1, STATE_GAS_ZERO, state_spill_after_3_1510, carried_refund, running, parent_message, carried_code, carried_calldata);
      struct FrameContinuation continuation = ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = suspend_frame_result_2_1030.tup0, .new_account_charged = tmp_3_1457}));
      frame_stack_push(continuation);
      k_mark_created(new_addr);
      k_clear_storage(new_addr);
      k_bump_nonce(new_addr);
      k_transfer(carried_address, new_addr, value);
      Bytes child_returndata = returndata_clear();
      return ((struct FrameTransition){.calldata = EMPTY_CALLDATA, .code = child_code, .gas_remaining = child_gas, .memory_base = suspend_frame_result_2_1030.tup2, .memory_height = suspend_frame_result_2_1030.tup3, .message = ((struct Message){.address = new_addr, .caller = carried_address, .code_address = new_addr, .depth = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .is_static = carried_is_static, .state_gas_reservoir = state_gas_after_3_1509, .value = value}), .pc = UINT32_C(0), .refund = GAS_REFUND_ZERO, .returndata = child_returndata, .stack_top = suspend_frame_result_2_1030.tup1, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = running});
    }
    memory_after = mem1;
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional_3_1518;
    struct ExecutionProfileFields execution_profile_8_349 = k_execution_profile;
    if (execution_profile_8_349.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_8_353 = Exceptional(InitCodeTooLarge);
      exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = carried_state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_353});
    } else {
      struct FrameStatus Exceptional_result_8_355 = Exceptional(InitCodeTooLarge);
      exceptional_3_1518 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = Exceptional_result_8_355});
    }
    state_gas_after_3_1509 = exceptional_3_1518.state_gas_remaining;
    state_spill_after_3_1510 = exceptional_3_1518.state_gas_spilled;
    status_after_3_1511 = exceptional_3_1518.status;
    return ((struct FrameTransition){.calldata = carried_calldata, .code = carried_code, .gas_remaining = gas_after, .memory_base = carried_memory_base, .memory_height = memory_after, .message = parent_message, .pc = carried_pc, .refund = carried_refund, .returndata = returndata_after, .stack_top = sp_after, .state_gas_remaining = state_gas_after_3_1509, .state_gas_spilled = state_spill_after_3_1510, .status = status_after_3_1511});
  }
  }
}

