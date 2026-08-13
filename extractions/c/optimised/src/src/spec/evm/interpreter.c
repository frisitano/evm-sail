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
  u256 value;
  Bytes code_bytes_result_2_1164 = code_bytes(frame_code);
  value = read_push(code_bytes_result_2_1164, (*immediate_offset), width);
  (*immediate_offset) = ((*immediate_offset) + (uint32_t)width);
  return value;
}

__attribute__((__always_inline__)) uint64_t decode_deep_immediate(struct CodeFields frame_code, uint32_t *restrict immediate_offset, enum DeepStackOperation operation)
{
  uint64_t immediate;
  Bytes code_bytes_result_2_1162 = code_bytes(frame_code);
  immediate = code_slice_byte(code_bytes_result_2_1162, (*immediate_offset));
  uint32_t next_pc;
  bool deep_stack_operation_immediate_valid_result_2_1161 = deep_stack_operation_immediate_valid(operation, immediate);
  next_pc = deep_stack_operation_immediate_valid_result_2_1161 ? ((*immediate_offset) + UINT32_C(1)) : (*immediate_offset);
  (*immediate_offset) = next_pc;
  return immediate;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  if ((UINT8_C(95) <= opcode) && (opcode <= UINT8_C(127))) {
    uint8_t width = ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(95)));
    struct tuple_uint_32_u256 decode_push_immediate_result_2_1158;
    decode_push_immediate_result_2_1158.tup1 = decode_push_immediate(frame_code, immediate_offset, width);
    decode_push_immediate_result_2_1158.tup0 = (*immediate_offset);
    return execute_push(execution_gas, sp, width, decode_push_immediate_result_2_1158.tup1);
    (*immediate_offset) = decode_push_immediate_result_2_1158.tup0;
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

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log_encoded(bytes20 carried_address, bool carried_is_static, uint64_t *restrict execution_gas, StackPointer *restrict sp, Bytes *restrict memory, uint8_t opcode)
{
  if ((UINT8_C(160) <= opcode) && (opcode <= UINT8_C(164))) {
    return execute_log(carried_address, carried_is_static, execution_gas, sp, memory, ((uint8_t)((uint32_t)opcode - (uint32_t)UINT8_C(160))));
  }
  return execute_invalid(execution_gas);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_deep_stack_encoded(struct CodeFields frame_code, uint8_t opcode, uint32_t *restrict immediate_offset, uint64_t *restrict execution_gas, StackPointer *restrict sp)
{
  enum DeepStackOperation operation = deep_stack_operation(opcode);
  struct tuple_uint_32_bits_8 decode_deep_immediate_result_2_1142;
  decode_deep_immediate_result_2_1142.tup1 = decode_deep_immediate(frame_code, immediate_offset, operation);
  decode_deep_immediate_result_2_1142.tup0 = (*immediate_offset);
  uint32_t next_pc = decode_deep_immediate_result_2_1142.tup0;
  uint64_t immediate = decode_deep_immediate_result_2_1142.tup1;
  struct tuple_uint_64_StackPointer_OpcodeOutcome tmp_3_1842;
  switch (operation) {
  case DeepStackDuplicate:
    tmp_3_1842.tup2 = execute_dupn(execution_gas, sp, immediate);
    tmp_3_1842.tup0 = (*execution_gas);
    tmp_3_1842.tup1 = (*sp);
    break;
  case DeepStackSwap:
    tmp_3_1842.tup2 = execute_swapn(execution_gas, sp, immediate);
    tmp_3_1842.tup0 = (*execution_gas);
    tmp_3_1842.tup1 = (*sp);
    break;
  case DeepStackExchange:
    tmp_3_1842.tup2 = execute_exchange(execution_gas, sp, immediate);
    tmp_3_1842.tup0 = (*execution_gas);
    tmp_3_1842.tup1 = (*sp);
    break;
  case NotDeepStackOperation:
  {
    struct tuple_uint_64_OpcodeOutcome execute_invalid_result_2_1143;
    execute_invalid_result_2_1143.tup1 = execute_invalid(execution_gas);
    execute_invalid_result_2_1143.tup0 = (*execution_gas);
    tmp_3_1842 = ((struct tuple_uint_64_StackPointer_OpcodeOutcome){.tup0 = execute_invalid_result_2_1143.tup0, .tup1 = (*sp), .tup2 = execute_invalid_result_2_1143.tup1});
    break;
  }
  }
  (*immediate_offset) = next_pc;
  (*execution_gas) = tmp_3_1842.tup0;
  (*sp) = tmp_3_1842.tup1;
  return tmp_3_1842.tup2;
}

Bytes frame_output(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1905;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltReturn) {
    goto case_1905;
  }
  return frame_status.variants.Halted.variants.HaltReturn;
case_1905: ;
  if (frame_status.kind != Kind_Halted) {
    goto case_1904;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1904;
  }
  return frame_status.variants.Halted.variants.HaltRevert;
case_1904: ;
  return EMPTY_OUTPUT_SLICE;
}

bool frame_succeeded(struct FrameStatus frame_status)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1897;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1897;
  }
  return false;
case_1897: ;
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
    bool neq_anything_result_2_1138 = neq_anything_EPrecompileId__(delegate_precompile, NotPrecompile);
    if (neq_anything_result_2_1138) {
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

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes run_call(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, Bytes carried_memory, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind)
{
  struct OpcodeOutcome result_2_1069;
  uint16_t tmp_3_1817;
  switch (kind) {
  case Call:
  case CallCode:
    tmp_3_1817 = UINT16_C(7);
    break;
  case DelegateCall:
  case StaticCall:
    tmp_3_1817 = UINT16_C(6);
    break;
  }
  result_2_1069 = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1817, UINT8_C(1));
  struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1616;
  switch (result_2_1069.kind) {
  case Kind_Failed:
  {
    struct tuple_uint_64_uint_32_FrameStatus exceptional_state_result_2_1070;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
    if (result_2_2039) {
      struct FrameStatus Exceptional_result_2_2040 = Exceptional(result_2_1069.variants.Failed);
      struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
      /* conversions */
      exceptional_state_result_2_1070.tup0 = tmp_3_3353.tup0;
      exceptional_state_result_2_1070.tup1 = (uint32_t)tmp_3_3353.tup1;
      exceptional_state_result_2_1070.tup2 = tmp_3_3353.tup2;
      /* end conversions */
    } else {
      struct FrameStatus Exceptional_result_2_2041 = Exceptional(result_2_1069.variants.Failed);
      exceptional_state_result_2_1070 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = carried_state_gas, .tup1 = carried_state_spill, .tup2 = Exceptional_result_2_2041});
    }
    struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1780 = ((struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = GAS_ZERO, .tup2 = exceptional_state_result_2_1070.tup0, .tup3 = exceptional_state_result_2_1070.tup1, .tup4 = carried_refund, .tup5 = exceptional_state_result_2_1070.tup2, .tup6 = carried_sp, .tup7 = carried_memory, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = carried_returndata});
    /* conversions */
    tmp_3_1616.tup0 = tmp_3_1780.tup0;
    tmp_3_1616.tup1 = (uint64_t)tmp_3_1780.tup1;
    tmp_3_1616.tup2 = tmp_3_1780.tup2;
    tmp_3_1616.tup3 = tmp_3_1780.tup3;
    tmp_3_1616.tup4 = tmp_3_1780.tup4;
    tmp_3_1616.tup5 = tmp_3_1780.tup5;
    tmp_3_1616.tup6 = tmp_3_1780.tup6;
    tmp_3_1616.tup7 = tmp_3_1780.tup7;
    tmp_3_1616.tup8 = tmp_3_1780.tup8;
    tmp_3_1616.tup9 = tmp_3_1780.tup9;
    tmp_3_1616.tup10 = tmp_3_1780.tup10;
    tmp_3_1616.tup11 = tmp_3_1780.tup11;
    tmp_3_1616.tup12 = tmp_3_1780.tup12;
    tmp_3_1616.tup13 = tmp_3_1780.tup13;
    tmp_3_1616.tup14 = tmp_3_1780.tup14;
    tmp_3_1616.tup15 = tmp_3_1780.tup15;
    tmp_3_1616.tup16 = tmp_3_1780.tup16;
    tmp_3_1616.tup17 = tmp_3_1780.tup17;
    /* end conversions */
    break;
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1783 = carried_state_gas;
    uint32_t state_spill_after_3_1784 = carried_state_spill;
    struct FrameStatus status_after_3_1785 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    Bytes memory_after = carried_memory;
    Bytes returndata_after = carried_returndata;
    struct CallSemantics semantics = call_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_5 = k_execution_profile;
    struct ProtocolProfileFields profile_8_6 = execution_profile_8_5.protocol;
    u256 gas_request = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 target_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    bytes20 target = word_to_address(target_word);
    struct tuple_u256_StackPointer result_2_1073;
    if (semantics.takes_value) {
      u256 value = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1072 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1073 = ((struct tuple_u256_StackPointer){.tup0 = value, .tup1 = stack_top_retreat_result_2_1072});
    } else {
      result_2_1073 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    u256 value_3_1786 = result_2_1073.tup0;
    sp_after = result_2_1073.tup1;
    bool value_nonzero = word_nonzero(value_3_1786);
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
      struct tuple_uint_64_uint_32_FrameStatus result_2_1076;
      struct ExecutionProfileFields execution_profile_8_7 = k_execution_profile;
      bool result_8_9 = (bool)(execution_profile_8_7.protocol.fork >= Amsterdam);
      if (result_8_9) {
        struct FrameStatus Exceptional_result_8_11 = Exceptional(WriteProtection);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_12 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_11});
        /* conversions */
        result_2_1076.tup0 = tmp_8_12.tup0;
        result_2_1076.tup1 = (uint32_t)tmp_8_12.tup1;
        result_2_1076.tup2 = tmp_8_12.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_13 = Exceptional(WriteProtection);
        result_2_1076 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_13});
      }
      state_gas_after_3_1783 = result_2_1076.tup0;
      state_spill_after_3_1784 = result_2_1076.tup1;
      status_after_3_1785 = result_2_1076.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_required;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_required = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      uint64_t tmp_3_3290;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(args_len_word);
        tmp_3_3290 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size));
      } else {
        tmp_3_3290 = UINT64_C(4294967296);
      }
      args_required = tmp_3_3290;
    } else {
      args_required = UINT64_C(4294967296);
    }
    uint64_t ret_required;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_required = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      uint64_t tmp_8_17;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_22 = (uint32_t)u256_to_u64(ret_len_word);
        tmp_8_17 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_22));
      } else {
        tmp_8_17 = UINT64_C(4294967296);
      }
      ret_required = tmp_8_17;
    } else {
      ret_required = UINT64_C(4294967296);
    }
    uint64_t required_size = args_required < ret_required ? ret_required : args_required;
    struct tuple_bool_uint_64 charge_memory_expansion_result_2_1077;
    if (required_size <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)required_size);
      uint32_t old_words;
      uint32_t memory_high_water_result_2_1938 = memory_high_water(memory_after);
      old_words = memory_word_count_uint32_t_to_uint32_t(memory_high_water_result_2_1938);
      if (new_words <= old_words) {
        charge_memory_expansion_result_2_1077 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after});
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_24 = ((uint64_t)G_memory * (uint64_t)new_words);
        if (!((__int128)gas_after < (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_24) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))) {
          charge_memory_expansion_result_2_1077 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_24) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))});
        } else {
          struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
          /* conversions */
          charge_memory_expansion_result_2_1077.tup0 = tmp_3_3313.tup0;
          charge_memory_expansion_result_2_1077.tup1 = (uint64_t)tmp_3_3313.tup1;
          /* end conversions */
        }
      }
    } else {
      struct tuple_bool_uint_8 tmp_3_3283 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_memory_expansion_result_2_1077.tup0 = tmp_3_3283.tup0;
      charge_memory_expansion_result_2_1077.tup1 = (uint64_t)tmp_3_3283.tup1;
      /* end conversions */
    }
    gas_after = charge_memory_expansion_result_2_1077.tup1;
    if (charge_memory_expansion_result_2_1077.tup0) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1078;
      struct ExecutionProfileFields execution_profile_8_27 = k_execution_profile;
      bool result_8_29 = (bool)(execution_profile_8_27.protocol.fork >= Amsterdam);
      if (result_8_29) {
        struct FrameStatus Exceptional_result_8_31 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_32 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_31});
        /* conversions */
        result_2_1078.tup0 = tmp_8_32.tup0;
        result_2_1078.tup1 = (uint32_t)tmp_8_32.tup1;
        result_2_1078.tup2 = tmp_8_32.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_33 = Exceptional(OutOfGas);
        result_2_1078 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_33});
      }
      state_gas_after_3_1783 = result_2_1078.tup0;
      state_spill_after_3_1784 = result_2_1078.tup1;
      status_after_3_1785 = result_2_1078.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    struct tuple_bool_uint_64 charge_result_2_1079;
    bool lteq_int_result_8_35 = (bool)(!((__int128)gas_after < (__int128)static_base));
    if (lteq_int_result_8_35) {
      charge_result_2_1079 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)static_base)});
    } else {
      struct tuple_bool_uint_8 tmp_8_39 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1079.tup0 = tmp_8_39.tup0;
      charge_result_2_1079.tup1 = (uint64_t)tmp_8_39.tup1;
      /* end conversions */
    }
    bool static_base_halt = charge_result_2_1079.tup0;
    gas_after = charge_result_2_1079.tup1;
    if (static_base_halt) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1080;
      struct ExecutionProfileFields execution_profile_8_40 = k_execution_profile;
      bool result_8_42 = (bool)(execution_profile_8_40.protocol.fork >= Amsterdam);
      if (result_8_42) {
        struct FrameStatus Exceptional_result_8_44 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_45 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_44});
        /* conversions */
        result_2_1080.tup0 = tmp_8_45.tup0;
        result_2_1080.tup1 = (uint32_t)tmp_8_45.tup1;
        result_2_1080.tup2 = tmp_8_45.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_46 = Exceptional(OutOfGas);
        result_2_1080 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_46});
      }
      state_gas_after_3_1783 = result_2_1080.tup0;
      state_spill_after_3_1784 = result_2_1080.tup1;
      status_after_3_1785 = result_2_1080.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    k_account_mark_warm(target);
    struct tuple_bool_bytes20 k_deleg_target_result_2_1081 = k_deleg_target(target);
    bool tg_deleg = k_deleg_target_result_2_1081.tup0;
    bytes20 tg_target = k_deleg_target_result_2_1081.tup1;
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool result_2_1136 = (bool)(profile_8_6.fork >= Amsterdam);
    bool tmp_3_1662;
    if (result_2_1136) {
      bool tmp_3_1661 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1662 = tmp_3_1661;
    } else {
      tmp_3_1662 = false;
    }
    uint16_t create_cost;
    bool result_2_1132 = (bool)(profile_8_6.fork < Amsterdam);
    bool tmp_3_1665;
    if (result_2_1132) {
      bool tmp_3_1664 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1665 = tmp_3_1664;
    } else {
      tmp_3_1665 = false;
    }
    create_cost = tmp_3_1665 ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    struct tuple_bool_uint_64 charge_result_2_1082;
    bool lteq_int_result_8_48 = (bool)(!((__int128)gas_after < (__int128)additional_cost));
    if (lteq_int_result_8_48) {
      charge_result_2_1082 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)additional_cost)});
    } else {
      struct tuple_bool_uint_8 tmp_8_52 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1082.tup0 = tmp_8_52.tup0;
      charge_result_2_1082.tup1 = (uint64_t)tmp_8_52.tup1;
      /* end conversions */
    }
    bool additional_cost_halt = charge_result_2_1082.tup0;
    gas_after = charge_result_2_1082.tup1;
    if (additional_cost_halt) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1083;
      struct ExecutionProfileFields execution_profile_8_53 = k_execution_profile;
      bool result_8_55 = (bool)(execution_profile_8_53.protocol.fork >= Amsterdam);
      if (result_8_55) {
        struct FrameStatus Exceptional_result_8_57 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_58 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_57});
        /* conversions */
        result_2_1083.tup0 = tmp_8_58.tup0;
        result_2_1083.tup1 = (uint32_t)tmp_8_58.tup1;
        result_2_1083.tup2 = tmp_8_58.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_59 = Exceptional(OutOfGas);
        result_2_1083 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_59});
      }
      state_gas_after_3_1783 = result_2_1083.tup0;
      state_spill_after_3_1784 = result_2_1083.tup1;
      status_after_3_1785 = result_2_1083.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    bool result_2_1085 = (bool)(profile_8_6.fork >= Amsterdam);
    if (result_2_1085) {
      if (tmp_3_1662) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1086 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1086.tup1;
        state_gas_after_3_1783 = charge_state_gas_result_2_1086.tup2;
        state_spill_after_3_1784 = charge_state_gas_result_2_1086.tup3;
        if (charge_state_gas_result_2_1086.tup0) {
          gas_after = (uint64_t)GAS_ZERO;
          struct tuple_uint_64_uint_32_FrameStatus result_2_1087;
          struct ExecutionProfileFields execution_profile_8_61 = k_execution_profile;
          bool result_8_63 = (bool)(execution_profile_8_61.protocol.fork >= Amsterdam);
          if (result_8_63) {
            struct FrameStatus Exceptional_result_8_65 = Exceptional(OutOfGas);
            struct tuple_uint_64_uint_8_FrameStatus tmp_8_66 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_65});
            /* conversions */
            result_2_1087.tup0 = tmp_8_66.tup0;
            result_2_1087.tup1 = (uint32_t)tmp_8_66.tup1;
            result_2_1087.tup2 = tmp_8_66.tup2;
            /* end conversions */
          } else {
            struct FrameStatus Exceptional_result_8_67 = Exceptional(OutOfGas);
            result_2_1087 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_67});
          }
          state_gas_after_3_1783 = result_2_1087.tup0;
          state_spill_after_3_1784 = result_2_1087.tup1;
          status_after_3_1785 = result_2_1087.tup2;
          return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      struct tuple_bool_uint_64 charge_result_2_1088;
      bool lteq_int_result_8_69 = (bool)(!((__int128)gas_after < (__int128)base_child));
      if (lteq_int_result_8_69) {
        charge_result_2_1088 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)base_child)});
      } else {
        struct tuple_bool_uint_8 tmp_8_73 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
        /* conversions */
        charge_result_2_1088.tup0 = tmp_8_73.tup0;
        charge_result_2_1088.tup1 = (uint64_t)tmp_8_73.tup1;
        /* end conversions */
      }
      gas_after = charge_result_2_1088.tup1;
      if (charge_result_2_1088.tup0) {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1089;
        struct ExecutionProfileFields execution_profile_8_74 = k_execution_profile;
        bool result_8_76 = (bool)(execution_profile_8_74.protocol.fork >= Amsterdam);
        if (result_8_76) {
          struct FrameStatus Exceptional_result_8_78 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_79 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_78});
          /* conversions */
          result_2_1089.tup0 = tmp_8_79.tup0;
          result_2_1089.tup1 = (uint32_t)tmp_8_79.tup1;
          result_2_1089.tup2 = tmp_8_79.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_80 = Exceptional(OutOfGas);
          result_2_1089 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_80});
        }
        state_gas_after_3_1783 = result_2_1089.tup0;
        state_spill_after_3_1784 = result_2_1089.tup1;
        status_after_3_1785 = result_2_1089.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      struct tuple_bool_uint_64 charge_result_2_1090;
      bool lteq_int_result_8_82 = (bool)(!((__int128)gas_after < (__int128)base_child));
      if (lteq_int_result_8_82) {
        charge_result_2_1090 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)base_child)});
      } else {
        struct tuple_bool_uint_8 tmp_8_86 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
        /* conversions */
        charge_result_2_1090.tup0 = tmp_8_86.tup0;
        charge_result_2_1090.tup1 = (uint64_t)tmp_8_86.tup1;
        /* end conversions */
      }
      gas_after = charge_result_2_1090.tup1;
      if (charge_result_2_1090.tup0) {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1091;
        struct ExecutionProfileFields execution_profile_8_87 = k_execution_profile;
        bool result_8_89 = (bool)(execution_profile_8_87.protocol.fork >= Amsterdam);
        if (result_8_89) {
          struct FrameStatus Exceptional_result_8_91 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_92 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_91});
          /* conversions */
          result_2_1091.tup0 = tmp_8_92.tup0;
          result_2_1091.tup1 = (uint32_t)tmp_8_92.tup1;
          result_2_1091.tup2 = tmp_8_92.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_93 = Exceptional(OutOfGas);
          result_2_1091 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_93});
        }
        state_gas_after_3_1783 = result_2_1091.tup0;
        state_spill_after_3_1784 = result_2_1091.tup1;
        status_after_3_1785 = result_2_1091.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
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
    bool eq_int_result_2_1939 = u256_eq_u64(args_len_word, UINT8_C(0));
    if (eq_int_result_2_1939) {
      args_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      struct MemoryAccessFields tmp_3_3285;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size_8_96 = (uint32_t)u256_to_u64(args_len_word);
        struct MemoryRangeFields memory_range_result_2_1943 = memory_range((uint32_t)u256_to_u64_unchecked(args_off_word), bounded_size_8_96);
        uint32_t add_atom_result_2_1944 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size_8_96));
        tmp_3_3285 = ((struct MemoryAccessFields){.range = memory_range_result_2_1943, .required_size = add_atom_result_2_1944});
      } else {
        fatal_error(ExecutionInvalid);
      }
      args_access = tmp_3_3285;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryAccessFields ret_access;
    bool eq_int_result_8_97 = u256_eq_u64(ret_len_word, UINT8_C(0));
    if (eq_int_result_8_97) {
      ret_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      struct MemoryAccessFields tmp_8_99;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_104 = (uint32_t)u256_to_u64(ret_len_word);
        struct MemoryRangeFields memory_range_result_8_106 = memory_range((uint32_t)u256_to_u64_unchecked(ret_off_word), bounded_size_8_104);
        uint32_t add_atom_result_8_107 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_104));
        tmp_8_99 = ((struct MemoryAccessFields){.range = memory_range_result_8_106, .required_size = add_atom_result_8_107});
      } else {
        fatal_error(ExecutionInvalid);
      }
      ret_access = tmp_8_99;
    } else {
      fatal_error(ExecutionInvalid);
    }
    uint32_t materialized_required_size = args_access.required_size < ret_access.required_size ? ret_access.required_size : args_access.required_size;
    struct tuple_Bytes_Bytes memory_expand_to_result_2_1933 = memory_expand_to(memory_after, materialized_required_size);
    Bytes tmp_3_3281 = memory_expand_to_result_2_1933.tup1;
    struct MemoryRangeFields args = args_access.range;
    struct MemoryRangeFields ret = ret_access.range;
    uint64_t child_gas = conserved_gas_add(base_child, stipend);
    k_aload_(target);
    struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1718;
    bool insufficient_balance;
    if (semantics.takes_value && value_nonzero) {
      u256 caller_balance = k_get_balance(carried_address);
      bool transfer_affordable = word_ule(value_3_1786, caller_balance);
      insufficient_balance = (bool)(!transfer_affordable);
    } else {
      insufficient_balance = false;
    }
    bool condition_2_1095 = (bool)(insufficient_balance || (carried_depth == UINT16_C(1024)));
    if (condition_2_1095) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (tmp_3_1662) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1096 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
        gas_after = result_2_1096.tup0;
        state_gas_after_3_1783 = result_2_1096.tup1;
        state_spill_after_3_1784 = result_2_1096.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = tmp_3_3281;
      tmp_3_1718 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    } else {
      enum PrecompileId selected_precompile = precompile_id_for_address(target);
      bool neq_anything_result_2_1097 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
      if (neq_anything_result_2_1097) {
        struct tuple_Bytes_Bytes result_2_1100 = active_memory_slice(tmp_3_3281, args.off, args.len);
        struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1731;
        struct CalldataSlice input = MemoryCalldata(result_2_1100.tup0);
        struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
        if (precompile_charge.affordable) {
          struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
          if (result.success) {
            returndata_after = result.output;
            if (semantics.transfers_value && value_nonzero) {
              k_transfer(carried_address, target, value_3_1786);
            }
            returndata_copy_prefix(returndata_after, ret.off, ret.len);
            uint64_t unused = gas_sub(child_gas, precompile_charge.cost);
            gas_after = refund_gas(gas_after, unused);
            sp_after = stack_top_advance(sp_after, UINT16_C(1));
            stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
            memory_after = result_2_1100.tup1;
            tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          } else {
            returndata_after = returndata_clear();
            if (tmp_3_1662) {
              struct tuple_uint_64_uint_64_uint_32 result_2_1107 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
              gas_after = result_2_1107.tup0;
              state_gas_after_3_1783 = result_2_1107.tup1;
              state_spill_after_3_1784 = result_2_1107.tup2;
            }
            sp_after = stack_top_advance(sp_after, UINT16_C(1));
            stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
            memory_after = result_2_1100.tup1;
            tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          }
        } else {
          returndata_after = returndata_clear();
          if (tmp_3_1662) {
            struct tuple_uint_64_uint_64_uint_32 result_2_1108 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
            gas_after = result_2_1108.tup0;
            state_gas_after_3_1783 = result_2_1108.tup1;
            state_spill_after_3_1784 = result_2_1108.tup2;
          }
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
          memory_after = result_2_1100.tup1;
          tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        }
        tmp_3_1718 = tmp_3_1731;
      } else {
        struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
        bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
        bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
        u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1786;
        bool child_static = (bool)(semantics.enters_static_context || carried_is_static);
        struct tuple_Bytes_Bytes result_2_1111 = active_memory_slice(tmp_3_3281, args.off, args.len);
        Bytes child_memory = evm_memory_slice(result_2_1111.tup0.bytes, result_2_1111.tup0.len);
        struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
        struct tuple_FrameCheckpoint_StackPointer_Bytes result_2_1113;
        struct FrameStatus Running_result_2_1112 = Running(UNIT);
        result_2_1113 = suspend_frame(carried_pc, gas_after, sp_after, result_2_1111.tup1, STATE_GAS_ZERO, state_spill_after_3_1784, carried_refund, Running_result_2_1112, ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), carried_code, carried_calldata);
        struct FrameContinuation continuation = ResumeCall(((struct CallContinuation){.checkpoint = result_2_1113.tup0, .new_account_charged = tmp_3_1662, .return_length = ret.len, .return_offset = ret.off}));
        frame_stack_push(continuation);
        if (semantics.transfers_value && value_nonzero) {
          k_transfer(carried_address, target, value_3_1786);
        }
        Bytes child_returndata = returndata_clear();
        struct FrameStatus Running_result_2_1116 = Running(UNIT);
        struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1728 = ((struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = UINT8_C(0), .tup1 = child_gas, .tup2 = state_gas_after_3_1783, .tup3 = STATE_GAS_SPILL_ZERO, .tup4 = GAS_REFUND_ZERO, .tup5 = Running_result_2_1116, .tup6 = result_2_1113.tup1, .tup7 = result_2_1113.tup2, .tup8 = child_caller, .tup9 = child_addr, .tup10 = target, .tup11 = child_value, .tup12 = state_gas_after_3_1783, .tup13 = child_static, .tup14 = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .tup15 = child_code, .tup16 = child_calldata, .tup17 = child_returndata});
        /* conversions */
        tmp_3_1718.tup0 = (uint32_t)tmp_3_1728.tup0;
        tmp_3_1718.tup1 = tmp_3_1728.tup1;
        tmp_3_1718.tup2 = tmp_3_1728.tup2;
        tmp_3_1718.tup3 = (uint32_t)tmp_3_1728.tup3;
        tmp_3_1718.tup4 = tmp_3_1728.tup4;
        tmp_3_1718.tup5 = tmp_3_1728.tup5;
        tmp_3_1718.tup6 = tmp_3_1728.tup6;
        tmp_3_1718.tup7 = tmp_3_1728.tup7;
        tmp_3_1718.tup8 = tmp_3_1728.tup8;
        tmp_3_1718.tup9 = tmp_3_1728.tup9;
        tmp_3_1718.tup10 = tmp_3_1728.tup10;
        tmp_3_1718.tup11 = tmp_3_1728.tup11;
        tmp_3_1718.tup12 = tmp_3_1728.tup12;
        tmp_3_1718.tup13 = tmp_3_1728.tup13;
        tmp_3_1718.tup14 = tmp_3_1728.tup14;
        tmp_3_1718.tup15 = tmp_3_1728.tup15;
        tmp_3_1718.tup16 = tmp_3_1728.tup16;
        tmp_3_1718.tup17 = tmp_3_1728.tup17;
        /* end conversions */
      }
    }
    tmp_3_1616 = tmp_3_1718;
    break;
  }
  }
  return tmp_3_1616;
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

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes run_create(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, Bytes carried_memory, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind)
{
  struct OpcodeOutcome result_2_1026;
  uint16_t tmp_3_1608;
  switch (kind) {
  case CreateByNonce:
    tmp_3_1608 = UINT16_C(3);
    break;
  case CreateBySalt:
    tmp_3_1608 = UINT16_C(4);
    break;
  }
  result_2_1026 = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1608, UINT8_C(1));
  struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1450;
  switch (result_2_1026.kind) {
  case Kind_Failed:
  {
    struct tuple_uint_64_uint_32_FrameStatus exceptional_state_result_2_1027;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
    if (result_2_2039) {
      struct FrameStatus Exceptional_result_2_2040 = Exceptional(result_2_1026.variants.Failed);
      struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
      /* conversions */
      exceptional_state_result_2_1027.tup0 = tmp_3_3353.tup0;
      exceptional_state_result_2_1027.tup1 = (uint32_t)tmp_3_3353.tup1;
      exceptional_state_result_2_1027.tup2 = tmp_3_3353.tup2;
      /* end conversions */
    } else {
      struct FrameStatus Exceptional_result_2_2041 = Exceptional(result_2_1026.variants.Failed);
      exceptional_state_result_2_1027 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = carried_state_gas, .tup1 = carried_state_spill, .tup2 = Exceptional_result_2_2041});
    }
    struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1578 = ((struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = GAS_ZERO, .tup2 = exceptional_state_result_2_1027.tup0, .tup3 = exceptional_state_result_2_1027.tup1, .tup4 = carried_refund, .tup5 = exceptional_state_result_2_1027.tup2, .tup6 = carried_sp, .tup7 = carried_memory, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = carried_returndata});
    /* conversions */
    tmp_3_1450.tup0 = tmp_3_1578.tup0;
    tmp_3_1450.tup1 = (uint64_t)tmp_3_1578.tup1;
    tmp_3_1450.tup2 = tmp_3_1578.tup2;
    tmp_3_1450.tup3 = tmp_3_1578.tup3;
    tmp_3_1450.tup4 = tmp_3_1578.tup4;
    tmp_3_1450.tup5 = tmp_3_1578.tup5;
    tmp_3_1450.tup6 = tmp_3_1578.tup6;
    tmp_3_1450.tup7 = tmp_3_1578.tup7;
    tmp_3_1450.tup8 = tmp_3_1578.tup8;
    tmp_3_1450.tup9 = tmp_3_1578.tup9;
    tmp_3_1450.tup10 = tmp_3_1578.tup10;
    tmp_3_1450.tup11 = tmp_3_1578.tup11;
    tmp_3_1450.tup12 = tmp_3_1578.tup12;
    tmp_3_1450.tup13 = tmp_3_1578.tup13;
    tmp_3_1450.tup14 = tmp_3_1578.tup14;
    tmp_3_1450.tup15 = tmp_3_1578.tup15;
    tmp_3_1450.tup16 = tmp_3_1578.tup16;
    tmp_3_1450.tup17 = tmp_3_1578.tup17;
    /* end conversions */
    break;
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1581 = carried_state_gas;
    uint32_t state_spill_after_3_1582 = carried_state_spill;
    struct FrameStatus status_after_3_1583 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    Bytes memory_after = carried_memory;
    Bytes returndata_after = carried_returndata;
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_110 = k_execution_profile;
    struct ProtocolProfileFields profile_8_111 = execution_profile_8_110.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    struct tuple_u256_StackPointer result_2_1030;
    if (semantics.uses_salt) {
      u256 salt = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1029 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1030 = ((struct tuple_u256_StackPointer){.tup0 = salt, .tup1 = stack_top_retreat_result_2_1029});
    } else {
      result_2_1030 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    sp_after = result_2_1030.tup1;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1031;
      struct ExecutionProfileFields execution_profile_8_112 = k_execution_profile;
      bool result_8_114 = (bool)(execution_profile_8_112.protocol.fork >= Amsterdam);
      if (result_8_114) {
        struct FrameStatus Exceptional_result_8_116 = Exceptional(WriteProtection);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_117 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_116});
        /* conversions */
        result_2_1031.tup0 = tmp_8_117.tup0;
        result_2_1031.tup1 = (uint32_t)tmp_8_117.tup1;
        result_2_1031.tup2 = tmp_8_117.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_118 = Exceptional(WriteProtection);
        result_2_1031 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_118});
      }
      state_gas_after_3_1581 = result_2_1031.tup0;
      state_spill_after_3_1582 = result_2_1031.tup1;
      status_after_3_1583 = result_2_1031.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t required_size;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      required_size = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      uint64_t tmp_3_3290;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(len_word);
        tmp_3_3290 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size));
      } else {
        tmp_3_3290 = UINT64_C(4294967296);
      }
      required_size = tmp_3_3290;
    } else {
      required_size = UINT64_C(4294967296);
    }
    struct tuple_bool_uint_64 charge_memory_expansion_result_2_1032;
    if (required_size <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)required_size);
      uint32_t old_words;
      uint32_t memory_high_water_result_2_1938 = memory_high_water(memory_after);
      old_words = memory_word_count_uint32_t_to_uint32_t(memory_high_water_result_2_1938);
      if (new_words <= old_words) {
        charge_memory_expansion_result_2_1032 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after});
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_120 = ((uint64_t)G_memory * (uint64_t)new_words);
        if (!((__int128)gas_after < (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_120) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))) {
          charge_memory_expansion_result_2_1032 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_120) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))});
        } else {
          struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
          /* conversions */
          charge_memory_expansion_result_2_1032.tup0 = tmp_3_3313.tup0;
          charge_memory_expansion_result_2_1032.tup1 = (uint64_t)tmp_3_3313.tup1;
          /* end conversions */
        }
      }
    } else {
      struct tuple_bool_uint_8 tmp_3_3283 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_memory_expansion_result_2_1032.tup0 = tmp_3_3283.tup0;
      charge_memory_expansion_result_2_1032.tup1 = (uint64_t)tmp_3_3283.tup1;
      /* end conversions */
    }
    gas_after = charge_memory_expansion_result_2_1032.tup1;
    if (charge_memory_expansion_result_2_1032.tup0) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1033;
      struct ExecutionProfileFields execution_profile_8_123 = k_execution_profile;
      bool result_8_125 = (bool)(execution_profile_8_123.protocol.fork >= Amsterdam);
      if (result_8_125) {
        struct FrameStatus Exceptional_result_8_127 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_128 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_127});
        /* conversions */
        result_2_1033.tup0 = tmp_8_128.tup0;
        result_2_1033.tup1 = (uint32_t)tmp_8_128.tup1;
        result_2_1033.tup2 = tmp_8_128.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_129 = Exceptional(OutOfGas);
        result_2_1033 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_129});
      }
      state_gas_after_3_1581 = result_2_1033.tup0;
      state_spill_after_3_1582 = result_2_1033.tup1;
      status_after_3_1583 = result_2_1033.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    struct MemoryAccessFields initcode_access;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      initcode_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      struct MemoryAccessFields tmp_3_3285;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size_8_132 = (uint32_t)u256_to_u64(len_word);
        struct MemoryRangeFields memory_range_result_2_1943 = memory_range((uint32_t)u256_to_u64_unchecked(off_word), bounded_size_8_132);
        uint32_t add_atom_result_2_1944 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size_8_132));
        tmp_3_3285 = ((struct MemoryAccessFields){.range = memory_range_result_2_1943, .required_size = add_atom_result_2_1944});
      } else {
        fatal_error(ExecutionInvalid);
      }
      initcode_access = tmp_3_3285;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct tuple_Bytes_Bytes memory_expand_to_result_2_1933 = memory_expand_to(memory_after, initcode_access.required_size);
    Bytes tmp_3_3281 = memory_expand_to_result_2_1933.tup1;
    struct MemoryRangeFields initcode = initcode_access.range;
    uint16_t access_cost = create_access_cost();
    struct tuple_bool_uint_64 charge_result_2_1034;
    bool lteq_int_result_8_133 = (bool)(access_cost <= gas_after);
    if (lteq_int_result_8_133) {
      charge_result_2_1034 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (gas_after - (uint64_t)access_cost)});
    } else {
      struct tuple_bool_uint_8 tmp_8_136 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1034.tup0 = tmp_8_136.tup0;
      charge_result_2_1034.tup1 = (uint64_t)tmp_8_136.tup1;
      /* end conversions */
    }
    struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1477;
    gas_after = charge_result_2_1034.tup1;
    if (charge_result_2_1034.tup0) {
      memory_after = tmp_3_3281;
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1035;
      struct ExecutionProfileFields execution_profile_8_137 = k_execution_profile;
      bool result_8_139 = (bool)(execution_profile_8_137.protocol.fork >= Amsterdam);
      if (result_8_139) {
        struct FrameStatus Exceptional_result_8_141 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_142 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_141});
        /* conversions */
        result_2_1035.tup0 = tmp_8_142.tup0;
        result_2_1035.tup1 = (uint32_t)tmp_8_142.tup1;
        result_2_1035.tup2 = tmp_8_142.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_143 = Exceptional(OutOfGas);
        result_2_1035 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_143});
      }
      state_gas_after_3_1581 = result_2_1035.tup0;
      state_spill_after_3_1582 = result_2_1035.tup1;
      status_after_3_1583 = result_2_1035.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    u256 remainder = word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    bool eq_int_result_2_1949 = eq_u256(remainder, WORD_ZERO);
    if (eq_int_result_2_1949) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    bool result_2_1037 = (bool)(profile_8_111.fork >= Shanghai);
    if (result_2_1037) {
      struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1038 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64_variant_2(gas_after, G_initcode_word, initcode_word_count);
      gas_after = charge_word_scaled_gas_result_2_1038.tup1;
      if (charge_word_scaled_gas_result_2_1038.tup0) {
        memory_after = tmp_3_3281;
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1039;
        struct ExecutionProfileFields execution_profile_8_145 = k_execution_profile;
        bool result_8_147 = (bool)(execution_profile_8_145.protocol.fork >= Amsterdam);
        if (result_8_147) {
          struct FrameStatus Exceptional_result_8_149 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_150 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_149});
          /* conversions */
          result_2_1039.tup0 = tmp_8_150.tup0;
          result_2_1039.tup1 = (uint32_t)tmp_8_150.tup1;
          result_2_1039.tup2 = tmp_8_150.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_151 = Exceptional(OutOfGas);
          result_2_1039 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_151});
        }
        state_gas_after_3_1581 = result_2_1039.tup0;
        state_spill_after_3_1582 = result_2_1039.tup1;
        status_after_3_1583 = result_2_1039.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    }
    if (semantics.uses_salt) {
      struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1041 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(gas_after, G_keccak_word, initcode_word_count);
      gas_after = charge_word_scaled_gas_result_2_1041.tup1;
      if (charge_word_scaled_gas_result_2_1041.tup0) {
        memory_after = tmp_3_3281;
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1042;
        struct ExecutionProfileFields execution_profile_8_153 = k_execution_profile;
        bool result_8_155 = (bool)(execution_profile_8_153.protocol.fork >= Amsterdam);
        if (result_8_155) {
          struct FrameStatus Exceptional_result_8_157 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_158 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_157});
          /* conversions */
          result_2_1042.tup0 = tmp_8_158.tup0;
          result_2_1042.tup1 = (uint32_t)tmp_8_158.tup1;
          result_2_1042.tup2 = tmp_8_158.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_159 = Exceptional(OutOfGas);
          result_2_1042 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_159});
        }
        state_gas_after_3_1581 = result_2_1042.tup0;
        state_spill_after_3_1582 = result_2_1042.tup1;
        status_after_3_1583 = result_2_1042.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    }
    bool valid_initcode_size = initcode_size_allowed(initcode.len);
    if (valid_initcode_size) {
      uint64_t nonce = k_get_nonce(carried_address);
      Bytes mem2 = tmp_3_3281;
      bytes20 new_addr;
      if (semantics.uses_salt) {
        struct tuple_u256_Bytes mem_keccak_result_2_1065 = mem_keccak(tmp_3_3281, initcode);
        mem2 = mem_keccak_result_2_1065.tup1;
        bytes32 initcode_digest = word_to_hash(mem_keccak_result_2_1065.tup0);
        new_addr = k_create2_addr(carried_address, result_2_1030.tup0, initcode_digest);
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      bool result_2_1045 = (bool)(profile_8_111.fork < Amsterdam);
      if (result_2_1045) {
        uint64_t avail = gas_after;
        uint64_t retained_gas = (avail >> 6);
        child_gas = gas_sub(avail, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      bool tmp_3_1519 = (bool)(!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024))));
      if (tmp_3_1519) {
        returndata_after = returndata_clear();
        bool result_2_1049 = (bool)(profile_8_111.fork < Amsterdam);
        if (result_2_1049) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem2;
        tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      } else {
        k_account_mark_warm(new_addr);
        bool result_2_1063 = (bool)(profile_8_111.fork >= Amsterdam);
        bool tmp_3_1521;
        if (result_2_1063) {
          tmp_3_1521 = k_account_is_empty(new_addr);
        } else {
          tmp_3_1521 = false;
        }
        if (tmp_3_1521) {
          struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1050 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1581, state_spill_after_3_1582, G_amsterdam_state_new_account);
          gas_after = charge_state_gas_result_2_1050.tup1;
          state_gas_after_3_1581 = charge_state_gas_result_2_1050.tup2;
          state_spill_after_3_1582 = charge_state_gas_result_2_1050.tup3;
          if (charge_state_gas_result_2_1050.tup0) {
            memory_after = mem2;
            gas_after = (uint64_t)GAS_ZERO;
            struct tuple_uint_64_uint_32_FrameStatus result_2_1051;
            struct ExecutionProfileFields execution_profile_8_169 = k_execution_profile;
            bool result_8_171 = (bool)(execution_profile_8_169.protocol.fork >= Amsterdam);
            if (result_8_171) {
              struct FrameStatus Exceptional_result_8_173 = Exceptional(OutOfGas);
              struct tuple_uint_64_uint_8_FrameStatus tmp_8_174 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_173});
              /* conversions */
              result_2_1051.tup0 = tmp_8_174.tup0;
              result_2_1051.tup1 = (uint32_t)tmp_8_174.tup1;
              result_2_1051.tup2 = tmp_8_174.tup2;
              /* end conversions */
            } else {
              struct FrameStatus Exceptional_result_8_175 = Exceptional(OutOfGas);
              result_2_1051 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_175});
            }
            state_gas_after_3_1581 = result_2_1051.tup0;
            state_spill_after_3_1582 = result_2_1051.tup1;
            status_after_3_1583 = result_2_1051.tup2;
            return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          }
        }
        bool result_2_1053 = (bool)(profile_8_111.fork >= Amsterdam);
        if (result_2_1053) {
          uint64_t avail_3_1603 = gas_after;
          uint64_t retained_gas_3_1604 = (avail_3_1603 >> 6);
          child_gas = gas_sub(avail_3_1603, retained_gas_3_1604);
          gas_after = retained_gas_3_1604;
        }
        bool occupied = k_account_occupied(new_addr);
        returndata_after = returndata_clear();
        k_bump_nonce(carried_address);
        if (occupied) {
          if (tmp_3_1521) {
            struct tuple_uint_64_uint_64_uint_32 result_2_1054 = credit_state_gas_refund(gas_after, state_gas_after_3_1581, state_spill_after_3_1582, G_amsterdam_state_new_account);
            gas_after = result_2_1054.tup0;
            state_gas_after_3_1581 = result_2_1054.tup1;
            state_spill_after_3_1582 = result_2_1054.tup2;
          }
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
          memory_after = mem2;
          tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        } else {
          struct tuple_Bytes_Bytes_1 result_2_1057 = memory_code_slice(mem2, initcode.off, initcode.len);
          bytes32 child_code_id = code_db_insert(result_2_1057.tup0, profile_8_111.fork);
          struct CodeFields child_code = code_db_resolve(child_code_id);
          struct tuple_FrameCheckpoint_StackPointer_Bytes result_2_1059;
          struct FrameStatus Running_result_2_1058 = Running(UNIT);
          result_2_1059 = suspend_frame(carried_pc, gas_after, sp_after, result_2_1057.tup1, STATE_GAS_ZERO, state_spill_after_3_1582, carried_refund, Running_result_2_1058, ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), carried_code, carried_calldata);
          struct FrameContinuation continuation = ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = result_2_1059.tup0, .new_account_charged = tmp_3_1521}));
          frame_stack_push(continuation);
          k_mark_created(new_addr);
          k_clear_storage(new_addr);
          k_bump_nonce(new_addr);
          k_transfer(carried_address, new_addr, value);
          Bytes child_returndata = returndata_clear();
          struct FrameStatus Running_result_2_1060 = Running(UNIT);
          struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1548 = ((struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = UINT8_C(0), .tup1 = child_gas, .tup2 = state_gas_after_3_1581, .tup3 = STATE_GAS_SPILL_ZERO, .tup4 = GAS_REFUND_ZERO, .tup5 = Running_result_2_1060, .tup6 = result_2_1059.tup1, .tup7 = result_2_1059.tup2, .tup8 = carried_address, .tup9 = new_addr, .tup10 = new_addr, .tup11 = value, .tup12 = state_gas_after_3_1581, .tup13 = carried_is_static, .tup14 = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .tup15 = child_code, .tup16 = EMPTY_CALLDATA, .tup17 = child_returndata});
          /* conversions */
          tmp_3_1477.tup0 = (uint32_t)tmp_3_1548.tup0;
          tmp_3_1477.tup1 = tmp_3_1548.tup1;
          tmp_3_1477.tup2 = tmp_3_1548.tup2;
          tmp_3_1477.tup3 = (uint32_t)tmp_3_1548.tup3;
          tmp_3_1477.tup4 = tmp_3_1548.tup4;
          tmp_3_1477.tup5 = tmp_3_1548.tup5;
          tmp_3_1477.tup6 = tmp_3_1548.tup6;
          tmp_3_1477.tup7 = tmp_3_1548.tup7;
          tmp_3_1477.tup8 = tmp_3_1548.tup8;
          tmp_3_1477.tup9 = tmp_3_1548.tup9;
          tmp_3_1477.tup10 = tmp_3_1548.tup10;
          tmp_3_1477.tup11 = tmp_3_1548.tup11;
          tmp_3_1477.tup12 = tmp_3_1548.tup12;
          tmp_3_1477.tup13 = tmp_3_1548.tup13;
          tmp_3_1477.tup14 = tmp_3_1548.tup14;
          tmp_3_1477.tup15 = tmp_3_1548.tup15;
          tmp_3_1477.tup16 = tmp_3_1548.tup16;
          tmp_3_1477.tup17 = tmp_3_1548.tup17;
          /* end conversions */
        }
      }
    } else {
      memory_after = tmp_3_3281;
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1043;
      struct ExecutionProfileFields execution_profile_8_161 = k_execution_profile;
      bool result_8_163 = (bool)(execution_profile_8_161.protocol.fork >= Amsterdam);
      if (result_8_163) {
        struct FrameStatus Exceptional_result_8_165 = Exceptional(InitCodeTooLarge);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_166 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_165});
        /* conversions */
        result_2_1043.tup0 = tmp_8_166.tup0;
        result_2_1043.tup1 = (uint32_t)tmp_8_166.tup1;
        result_2_1043.tup2 = tmp_8_166.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_167 = Exceptional(InitCodeTooLarge);
        result_2_1043 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_167});
      }
      state_gas_after_3_1581 = result_2_1043.tup0;
      state_spill_after_3_1582 = result_2_1043.tup1;
      status_after_3_1583 = result_2_1043.tup2;
      tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    tmp_3_1450 = tmp_3_1477;
    break;
  }
  }
  return tmp_3_1450;
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes resume_call(struct CallContinuation continuation, Bytes output, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status)
{
  bool succeeded = frame_succeeded(child_status);
  struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice restore_frame_result_2_1012 = restore_frame(continuation.checkpoint);
  uint64_t parent_gas = refund_gas(restore_frame_result_2_1012.tup1, child_gas);
  uint64_t parent_state_gas = restore_frame_result_2_1012.tup4;
  uint32_t parent_state_spill = restore_frame_result_2_1012.tup5;
  struct tuple_uint_64_uint_32 result_2_1013 = return_child_state_gas(parent_state_gas, parent_state_spill, child_state_gas, child_state_spill);
  parent_state_gas = result_2_1013.tup0;
  parent_state_spill = result_2_1013.tup1;
  __int128 parent_refund = restore_frame_result_2_1012.tup6;
  StackPointer parent_sp = restore_frame_result_2_1012.tup2;
  returndata_copy_prefix(output, continuation.return_offset, continuation.return_length);
  if (succeeded) {
    parent_refund = record_refund(parent_refund, child_refund);
    k_journal_commit();
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), WORD_ONE);
  } else {
    k_journal_revert();
    if (continuation.new_account_charged) {
      struct tuple_uint_64_uint_64_uint_32 result_2_1017 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account);
      parent_gas = result_2_1017.tup0;
      parent_state_gas = result_2_1017.tup1;
      parent_state_spill = result_2_1017.tup2;
    }
    parent_sp = stack_top_advance(parent_sp, UINT16_C(1));
    stack_slot_write(parent_sp, UINT16_C(0), WORD_ZERO);
  }
  return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = restore_frame_result_2_1012.tup0, .tup1 = parent_gas, .tup2 = parent_state_gas, .tup3 = parent_state_spill, .tup4 = parent_refund, .tup5 = restore_frame_result_2_1012.tup7, .tup6 = parent_sp, .tup7 = restore_frame_result_2_1012.tup3, .tup8 = restore_frame_result_2_1012.tup8.caller, .tup9 = restore_frame_result_2_1012.tup8.address, .tup10 = restore_frame_result_2_1012.tup8.code_address, .tup11 = restore_frame_result_2_1012.tup8.value, .tup12 = restore_frame_result_2_1012.tup8.state_gas_reservoir, .tup13 = restore_frame_result_2_1012.tup8.is_static, .tup14 = restore_frame_result_2_1012.tup8.depth, .tup15 = restore_frame_result_2_1012.tup9, .tup16 = restore_frame_result_2_1012.tup10, .tup17 = output});
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes resume_create(struct CreateContinuation continuation, Bytes output, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
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
    bool tmp_3_1376;
    if (deployed_size_allowed) {
      bool result_2_989 = (bool)(execution_profile.protocol.fork >= London);
      tmp_3_1376 = (bool)(result_2_989 && prohibited_prefix);
    } else {
      tmp_3_1376 = true;
    }
    if (tmp_3_1376) {
      settled_child_gas = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_991;
      struct ExecutionProfileFields execution_profile_8_177 = k_execution_profile;
      bool result_2_2039 = (bool)(execution_profile_8_177.protocol.fork >= Amsterdam);
      if (result_2_2039) {
        struct FrameStatus Exceptional_result_2_2040 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = child_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
        /* conversions */
        result_2_991.tup0 = tmp_3_3353.tup0;
        result_2_991.tup1 = (uint32_t)tmp_3_3353.tup1;
        result_2_991.tup2 = tmp_3_3353.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_2_2041 = Exceptional(OutOfGas);
        result_2_991 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = settled_child_state_gas, .tup1 = settled_child_state_spill, .tup2 = Exceptional_result_2_2041});
      }
      settled_child_state_gas = result_2_991.tup0;
      settled_child_state_spill = result_2_991.tup1;
      settled_child_status = result_2_991.tup2;
    } else {
      struct GasCharge deployment_charge = code_deployment_execution_cost(deployed_length, settled_child_gas);
      if (deployment_charge.affordable) {
        settled_child_gas = gas_sub(settled_child_gas, deployment_charge.cost);
        uint64_t state_deposit = code_deployment_state_cost(deployed_length);
        struct tuple_bool_uint_64_uint_64_uint_32 charge_deployment_state_gas_result_2_993 = charge_deployment_state_gas(settled_child_gas, settled_child_state_gas, settled_child_state_spill, state_deposit);
        settled_child_gas = charge_deployment_state_gas_result_2_993.tup1;
        settled_child_state_gas = charge_deployment_state_gas_result_2_993.tup2;
        settled_child_state_spill = charge_deployment_state_gas_result_2_993.tup3;
        if (charge_deployment_state_gas_result_2_993.tup0) {
          settled_child_gas = (uint64_t)GAS_ZERO;
          struct tuple_uint_64_uint_32_FrameStatus result_2_994;
          struct ExecutionProfileFields execution_profile_8_179 = k_execution_profile;
          bool result_8_181 = (bool)(execution_profile_8_179.protocol.fork >= Amsterdam);
          if (result_8_181) {
            struct FrameStatus Exceptional_result_8_183 = Exceptional(OutOfGas);
            struct tuple_uint_64_uint_8_FrameStatus tmp_8_184 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = child_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_183});
            /* conversions */
            result_2_994.tup0 = tmp_8_184.tup0;
            result_2_994.tup1 = (uint32_t)tmp_8_184.tup1;
            result_2_994.tup2 = tmp_8_184.tup2;
            /* end conversions */
          } else {
            struct FrameStatus Exceptional_result_8_185 = Exceptional(OutOfGas);
            result_2_994 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = settled_child_state_gas, .tup1 = settled_child_state_spill, .tup2 = Exceptional_result_8_185});
          }
          settled_child_state_gas = result_2_994.tup0;
          settled_child_state_spill = result_2_994.tup1;
          settled_child_status = result_2_994.tup2;
        }
      } else {
        settled_child_gas = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_997;
        struct ExecutionProfileFields execution_profile_8_187 = k_execution_profile;
        bool result_8_189 = (bool)(execution_profile_8_187.protocol.fork >= Amsterdam);
        if (result_8_189) {
          struct FrameStatus Exceptional_result_8_191 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_192 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = child_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_191});
          /* conversions */
          result_2_997.tup0 = tmp_8_192.tup0;
          result_2_997.tup1 = (uint32_t)tmp_8_192.tup1;
          result_2_997.tup2 = tmp_8_192.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_193 = Exceptional(OutOfGas);
          result_2_997 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = settled_child_state_gas, .tup1 = settled_child_state_spill, .tup2 = Exceptional_result_8_193});
        }
        settled_child_state_gas = result_2_997.tup0;
        settled_child_state_spill = result_2_997.tup1;
        settled_child_status = result_2_997.tup2;
      }
    }
  }
  bool tmp_3_1398;
  if (initcode_succeeded) {
    tmp_3_1398 = frame_succeeded(settled_child_status);
  } else {
    tmp_3_1398 = false;
  }
  struct tuple_uint_32_uint_64_StackPointer_Bytes_uint_64_uint_32_int_128_FrameStatus_Message_CodeFields_CalldataSlice restore_frame_result_2_999 = restore_frame(continuation.checkpoint);
  uint64_t parent_gas = refund_gas(restore_frame_result_2_999.tup1, settled_child_gas);
  uint64_t parent_state_gas = restore_frame_result_2_999.tup4;
  uint32_t parent_state_spill = restore_frame_result_2_999.tup5;
  struct tuple_uint_64_uint_32 result_2_1000 = return_child_state_gas(parent_state_gas, parent_state_spill, settled_child_state_gas, settled_child_state_spill);
  parent_state_gas = result_2_1000.tup0;
  parent_state_spill = result_2_1000.tup1;
  __int128 parent_refund = restore_frame_result_2_999.tup6;
  StackPointer parent_sp = restore_frame_result_2_999.tup2;
  if (tmp_3_1398) {
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
      struct tuple_uint_64_uint_64_uint_32 result_2_1004 = credit_state_gas_refund(parent_gas, parent_state_gas, parent_state_spill, G_amsterdam_state_new_account);
      parent_gas = result_2_1004.tup0;
      parent_state_gas = result_2_1004.tup1;
      parent_state_spill = result_2_1004.tup2;
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
  return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = restore_frame_result_2_999.tup0, .tup1 = parent_gas, .tup2 = parent_state_gas, .tup3 = parent_state_spill, .tup4 = parent_refund, .tup5 = restore_frame_result_2_999.tup7, .tup6 = parent_sp, .tup7 = restore_frame_result_2_999.tup3, .tup8 = restore_frame_result_2_999.tup8.caller, .tup9 = restore_frame_result_2_999.tup8.address, .tup10 = restore_frame_result_2_999.tup8.code_address, .tup11 = restore_frame_result_2_999.tup8.value, .tup12 = restore_frame_result_2_999.tup8.state_gas_reservoir, .tup13 = restore_frame_result_2_999.tup8.is_static, .tup14 = restore_frame_result_2_999.tup8.depth, .tup15 = restore_frame_result_2_999.tup9, .tup16 = restore_frame_result_2_999.tup10, .tup17 = parent_returndata});
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes resume_frame(struct FrameContinuation continuation, Bytes output, uint64_t child_gas, uint64_t child_state_gas, uint32_t child_state_spill, __int128 child_refund, struct FrameStatus child_status, uint64_t child_state_gas_reservoir)
{
  switch (continuation.kind) {
  case Kind_Empty:
    fatal_error(ExecutionInvalid);
  case Kind_ResumeCall:
    return resume_call(continuation.variants.ResumeCall, output, child_gas, child_state_gas, child_state_spill, child_refund, child_status);
  case Kind_ResumeCreate:
    return resume_create(continuation.variants.ResumeCreate, output, child_gas, child_state_gas, child_state_spill, child_refund, child_status, child_state_gas_reservoir);
  }
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes run_frame_entry_encoded(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, Bytes carried_memory, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, uint8_t opcode)
{
  if (opcode == UINT8_C(240)) {
    return run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CreateByNonce);
  }
  if (opcode == UINT8_C(241)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, Call);
  }
  if (opcode == UINT8_C(242)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CallCode);
  }
  if (opcode == UINT8_C(244)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, DelegateCall);
  }
  if (opcode == UINT8_C(245)) {
    return run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, CreateBySalt);
  }
  if (opcode == UINT8_C(250)) {
    return run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory, carried_caller, carried_address, carried_code_address, carried_value, carried_state_gas_reservoir, carried_is_static, carried_depth, carried_code, carried_calldata, carried_returndata, StaticCall);
  }
  struct tuple_uint_64_uint_32_FrameStatus exceptional_state_result_2_987;
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_2039) {
    struct FrameStatus Exceptional_result_2_2040 = Exceptional(InvalidOpcode);
    struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
    /* conversions */
    exceptional_state_result_2_987.tup0 = tmp_3_3353.tup0;
    exceptional_state_result_2_987.tup1 = (uint32_t)tmp_3_3353.tup1;
    exceptional_state_result_2_987.tup2 = tmp_3_3353.tup2;
    /* end conversions */
  } else {
    struct FrameStatus Exceptional_result_2_2041 = Exceptional(InvalidOpcode);
    exceptional_state_result_2_987 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = carried_state_gas, .tup1 = carried_state_spill, .tup2 = Exceptional_result_2_2041});
  }
  struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1355;
  struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1356 = ((struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = GAS_ZERO, .tup2 = exceptional_state_result_2_987.tup0, .tup3 = exceptional_state_result_2_987.tup1, .tup4 = carried_refund, .tup5 = exceptional_state_result_2_987.tup2, .tup6 = carried_sp, .tup7 = carried_memory, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = carried_returndata});
  /* conversions */
  tmp_3_1355.tup0 = tmp_3_1356.tup0;
  tmp_3_1355.tup1 = (uint64_t)tmp_3_1356.tup1;
  tmp_3_1355.tup2 = tmp_3_1356.tup2;
  tmp_3_1355.tup3 = tmp_3_1356.tup3;
  tmp_3_1355.tup4 = tmp_3_1356.tup4;
  tmp_3_1355.tup5 = tmp_3_1356.tup5;
  tmp_3_1355.tup6 = tmp_3_1356.tup6;
  tmp_3_1355.tup7 = tmp_3_1356.tup7;
  tmp_3_1355.tup8 = tmp_3_1356.tup8;
  tmp_3_1355.tup9 = tmp_3_1356.tup9;
  tmp_3_1355.tup10 = tmp_3_1356.tup10;
  tmp_3_1355.tup11 = tmp_3_1356.tup11;
  tmp_3_1355.tup12 = tmp_3_1356.tup12;
  tmp_3_1355.tup13 = tmp_3_1356.tup13;
  tmp_3_1355.tup14 = tmp_3_1356.tup14;
  tmp_3_1355.tup15 = tmp_3_1356.tup15;
  tmp_3_1355.tup16 = tmp_3_1356.tup16;
  tmp_3_1355.tup17 = tmp_3_1356.tup17;
  /* end conversions */
  return tmp_3_1355;
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint32_t initial_gas, uint8_t initial_state_gas, uint8_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, Bytes initial_memory, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint8_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee;
  uint32_t result_2_1141 = k_header.excess_blob_gas;
  blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, result_2_1141);
  return threaded_interpret((uint64_t)initial_gas, (uint64_t)initial_state_gas, (uint32_t)initial_state_spill, initial_refund, initial_sp, initial_memory, initial_caller, initial_address, initial_code_address, initial_value, (uint64_t)initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(uint64_t initial_gas, uint64_t initial_state_gas, uint32_t initial_state_spill, __int128 initial_refund, StackPointer initial_sp, Bytes initial_memory, bytes20 initial_caller, bytes20 initial_address, bytes20 initial_code_address, u256 initial_value, uint64_t initial_state_gas_reservoir, bool initial_is_static, uint8_t initial_depth, struct CodeFields initial_code, struct CalldataSlice initial_calldata)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint8_t fork = profile.fork;
  u256 blob_fee;
  uint32_t result_2_1141 = k_header.excess_blob_gas;
  blob_fee = blob_base_fee(fork, profile.blob_schedule, profile.excess_blob_gas_limit, result_2_1141);
  return threaded_interpret(initial_gas, initial_state_gas, initial_state_spill, initial_refund, initial_sp, initial_memory, initial_caller, initial_address, initial_code_address, initial_value, initial_state_gas_reservoir, initial_is_static, (uint16_t)initial_depth, initial_code, initial_calldata, fork, blob_fee);
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes run_call_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CallKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, Bytes carried_memory, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CallKind kind)
{
  struct OpcodeOutcome result_2_1069;
  uint16_t tmp_3_1817;
  switch (kind) {
  case Call:
  case CallCode:
    tmp_3_1817 = UINT16_C(7);
    break;
  case DelegateCall:
  case StaticCall:
    tmp_3_1817 = UINT16_C(6);
    break;
  }
  result_2_1069 = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1817, UINT8_C(1));
  struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1616;
  switch (result_2_1069.kind) {
  case Kind_Failed:
  {
    struct tuple_uint_64_uint_32_FrameStatus exceptional_state_result_2_1070;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
    if (result_2_2039) {
      struct FrameStatus Exceptional_result_2_2040 = Exceptional(result_2_1069.variants.Failed);
      struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
      /* conversions */
      exceptional_state_result_2_1070.tup0 = tmp_3_3353.tup0;
      exceptional_state_result_2_1070.tup1 = (uint32_t)tmp_3_3353.tup1;
      exceptional_state_result_2_1070.tup2 = tmp_3_3353.tup2;
      /* end conversions */
    } else {
      struct FrameStatus Exceptional_result_2_2041 = Exceptional(result_2_1069.variants.Failed);
      exceptional_state_result_2_1070 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = carried_state_gas, .tup1 = carried_state_spill, .tup2 = Exceptional_result_2_2041});
    }
    struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1780 = ((struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = GAS_ZERO, .tup2 = exceptional_state_result_2_1070.tup0, .tup3 = exceptional_state_result_2_1070.tup1, .tup4 = carried_refund, .tup5 = exceptional_state_result_2_1070.tup2, .tup6 = carried_sp, .tup7 = carried_memory, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = carried_returndata});
    /* conversions */
    tmp_3_1616.tup0 = tmp_3_1780.tup0;
    tmp_3_1616.tup1 = (uint64_t)tmp_3_1780.tup1;
    tmp_3_1616.tup2 = tmp_3_1780.tup2;
    tmp_3_1616.tup3 = tmp_3_1780.tup3;
    tmp_3_1616.tup4 = tmp_3_1780.tup4;
    tmp_3_1616.tup5 = tmp_3_1780.tup5;
    tmp_3_1616.tup6 = tmp_3_1780.tup6;
    tmp_3_1616.tup7 = tmp_3_1780.tup7;
    tmp_3_1616.tup8 = tmp_3_1780.tup8;
    tmp_3_1616.tup9 = tmp_3_1780.tup9;
    tmp_3_1616.tup10 = tmp_3_1780.tup10;
    tmp_3_1616.tup11 = tmp_3_1780.tup11;
    tmp_3_1616.tup12 = tmp_3_1780.tup12;
    tmp_3_1616.tup13 = tmp_3_1780.tup13;
    tmp_3_1616.tup14 = tmp_3_1780.tup14;
    tmp_3_1616.tup15 = tmp_3_1780.tup15;
    tmp_3_1616.tup16 = tmp_3_1780.tup16;
    tmp_3_1616.tup17 = tmp_3_1780.tup17;
    /* end conversions */
    break;
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1783 = carried_state_gas;
    uint32_t state_spill_after_3_1784 = carried_state_spill;
    struct FrameStatus status_after_3_1785 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    Bytes memory_after = carried_memory;
    Bytes returndata_after = carried_returndata;
    struct CallSemantics semantics = call_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_245 = k_execution_profile;
    struct ProtocolProfileFields profile_8_246 = execution_profile_8_245.protocol;
    u256 gas_request = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 target_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    bytes20 target = word_to_address(target_word);
    struct tuple_u256_StackPointer result_2_1073;
    if (semantics.takes_value) {
      u256 value = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1072 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1073 = ((struct tuple_u256_StackPointer){.tup0 = value, .tup1 = stack_top_retreat_result_2_1072});
    } else {
      result_2_1073 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    u256 value_3_1786 = result_2_1073.tup0;
    sp_after = result_2_1073.tup1;
    bool value_nonzero = word_nonzero(value_3_1786);
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
      struct tuple_uint_64_uint_32_FrameStatus result_2_1076;
      struct ExecutionProfileFields execution_profile_8_247 = k_execution_profile;
      bool result_8_249 = (bool)(execution_profile_8_247.protocol.fork >= Amsterdam);
      if (result_8_249) {
        struct FrameStatus Exceptional_result_8_251 = Exceptional(WriteProtection);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_252 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_251});
        /* conversions */
        result_2_1076.tup0 = tmp_8_252.tup0;
        result_2_1076.tup1 = (uint32_t)tmp_8_252.tup1;
        result_2_1076.tup2 = tmp_8_252.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_253 = Exceptional(WriteProtection);
        result_2_1076 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_253});
      }
      state_gas_after_3_1783 = result_2_1076.tup0;
      state_spill_after_3_1784 = result_2_1076.tup1;
      status_after_3_1785 = result_2_1076.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    bool warm = k_account_is_warm(target);
    uint16_t target_cost = account_cost(warm);
    uint16_t transfer_cost;
    if (value_nonzero) {
      transfer_cost = call_value_cost();
    } else {
      transfer_cost = GAS_CONSTANT_ZERO;
    }
    uint64_t args_required;
    if (u256_eq_u64(args_len_word, UINT8_C(0))) {
      args_required = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      uint64_t tmp_3_3290;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(args_len_word);
        tmp_3_3290 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size));
      } else {
        tmp_3_3290 = UINT64_C(4294967296);
      }
      args_required = tmp_3_3290;
    } else {
      args_required = UINT64_C(4294967296);
    }
    uint64_t ret_required;
    if (u256_eq_u64(ret_len_word, UINT8_C(0))) {
      ret_required = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      uint64_t tmp_8_257;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_262 = (uint32_t)u256_to_u64(ret_len_word);
        tmp_8_257 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_262));
      } else {
        tmp_8_257 = UINT64_C(4294967296);
      }
      ret_required = tmp_8_257;
    } else {
      ret_required = UINT64_C(4294967296);
    }
    uint64_t required_size = args_required < ret_required ? ret_required : args_required;
    struct tuple_bool_uint_64 charge_memory_expansion_result_2_1077;
    if (required_size <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)required_size);
      uint32_t old_words;
      uint32_t memory_high_water_result_2_1938 = memory_high_water(memory_after);
      old_words = memory_word_count_uint32_t_to_uint32_t(memory_high_water_result_2_1938);
      if (new_words <= old_words) {
        charge_memory_expansion_result_2_1077 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after});
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_264 = ((uint64_t)G_memory * (uint64_t)new_words);
        if (!((__int128)gas_after < (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_264) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))) {
          charge_memory_expansion_result_2_1077 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_264) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))});
        } else {
          struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
          /* conversions */
          charge_memory_expansion_result_2_1077.tup0 = tmp_3_3313.tup0;
          charge_memory_expansion_result_2_1077.tup1 = (uint64_t)tmp_3_3313.tup1;
          /* end conversions */
        }
      }
    } else {
      struct tuple_bool_uint_8 tmp_3_3283 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_memory_expansion_result_2_1077.tup0 = tmp_3_3283.tup0;
      charge_memory_expansion_result_2_1077.tup1 = (uint64_t)tmp_3_3283.tup1;
      /* end conversions */
    }
    gas_after = charge_memory_expansion_result_2_1077.tup1;
    if (charge_memory_expansion_result_2_1077.tup0) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1078;
      struct ExecutionProfileFields execution_profile_8_267 = k_execution_profile;
      bool result_8_269 = (bool)(execution_profile_8_267.protocol.fork >= Amsterdam);
      if (result_8_269) {
        struct FrameStatus Exceptional_result_8_271 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_272 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_271});
        /* conversions */
        result_2_1078.tup0 = tmp_8_272.tup0;
        result_2_1078.tup1 = (uint32_t)tmp_8_272.tup1;
        result_2_1078.tup2 = tmp_8_272.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_273 = Exceptional(OutOfGas);
        result_2_1078 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_273});
      }
      state_gas_after_3_1783 = result_2_1078.tup0;
      state_spill_after_3_1784 = result_2_1078.tup1;
      status_after_3_1785 = result_2_1078.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t static_base = ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost);
    struct tuple_bool_uint_64 charge_result_2_1079;
    bool lteq_int_result_8_275 = (bool)(!((__int128)gas_after < (__int128)static_base));
    if (lteq_int_result_8_275) {
      charge_result_2_1079 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)static_base)});
    } else {
      struct tuple_bool_uint_8 tmp_8_279 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1079.tup0 = tmp_8_279.tup0;
      charge_result_2_1079.tup1 = (uint64_t)tmp_8_279.tup1;
      /* end conversions */
    }
    bool static_base_halt = charge_result_2_1079.tup0;
    gas_after = charge_result_2_1079.tup1;
    if (static_base_halt) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1080;
      struct ExecutionProfileFields execution_profile_8_280 = k_execution_profile;
      bool result_8_282 = (bool)(execution_profile_8_280.protocol.fork >= Amsterdam);
      if (result_8_282) {
        struct FrameStatus Exceptional_result_8_284 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_285 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_284});
        /* conversions */
        result_2_1080.tup0 = tmp_8_285.tup0;
        result_2_1080.tup1 = (uint32_t)tmp_8_285.tup1;
        result_2_1080.tup2 = tmp_8_285.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_286 = Exceptional(OutOfGas);
        result_2_1080 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_286});
      }
      state_gas_after_3_1783 = result_2_1080.tup0;
      state_spill_after_3_1784 = result_2_1080.tup1;
      status_after_3_1785 = result_2_1080.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    k_account_mark_warm(target);
    struct tuple_bool_bytes20 k_deleg_target_result_2_1081 = k_deleg_target(target);
    bool tg_deleg = k_deleg_target_result_2_1081.tup0;
    bytes20 tg_target = k_deleg_target_result_2_1081.tup1;
    uint16_t delegation_cost;
    if (tg_deleg) {
      bool dw = k_account_is_warm(tg_target);
      delegation_cost = account_cost(dw);
    } else {
      delegation_cost = GAS_CONSTANT_ZERO;
    }
    bool target_empty = k_account_is_empty(target);
    bool result_2_1136 = (bool)(profile_8_246.fork >= Amsterdam);
    bool tmp_3_1662;
    if (result_2_1136) {
      bool tmp_3_1661 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1662 = tmp_3_1661;
    } else {
      tmp_3_1662 = false;
    }
    uint16_t create_cost;
    bool result_2_1132 = (bool)(profile_8_246.fork < Amsterdam);
    bool tmp_3_1665;
    if (result_2_1132) {
      bool tmp_3_1664 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
      tmp_3_1665 = tmp_3_1664;
    } else {
      tmp_3_1665 = false;
    }
    create_cost = tmp_3_1665 ? G_newaccount : GAS_CONSTANT_ZERO;
    uint64_t additional_cost = ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost);
    struct tuple_bool_uint_64 charge_result_2_1082;
    bool lteq_int_result_8_288 = (bool)(!((__int128)gas_after < (__int128)additional_cost));
    if (lteq_int_result_8_288) {
      charge_result_2_1082 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)additional_cost)});
    } else {
      struct tuple_bool_uint_8 tmp_8_292 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1082.tup0 = tmp_8_292.tup0;
      charge_result_2_1082.tup1 = (uint64_t)tmp_8_292.tup1;
      /* end conversions */
    }
    bool additional_cost_halt = charge_result_2_1082.tup0;
    gas_after = charge_result_2_1082.tup1;
    if (additional_cost_halt) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1083;
      struct ExecutionProfileFields execution_profile_8_293 = k_execution_profile;
      bool result_8_295 = (bool)(execution_profile_8_293.protocol.fork >= Amsterdam);
      if (result_8_295) {
        struct FrameStatus Exceptional_result_8_297 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_298 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_297});
        /* conversions */
        result_2_1083.tup0 = tmp_8_298.tup0;
        result_2_1083.tup1 = (uint32_t)tmp_8_298.tup1;
        result_2_1083.tup2 = tmp_8_298.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_299 = Exceptional(OutOfGas);
        result_2_1083 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_299});
      }
      state_gas_after_3_1783 = result_2_1083.tup0;
      state_spill_after_3_1784 = result_2_1083.tup1;
      status_after_3_1785 = result_2_1083.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t stipend = value_nonzero ? G_callstipend : (uint64_t)GAS_ZERO;
    uint64_t base_child = (uint64_t)GAS_ZERO;
    bool result_2_1085 = (bool)(profile_8_246.fork >= Amsterdam);
    if (result_2_1085) {
      if (tmp_3_1662) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1086 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_1086.tup1;
        state_gas_after_3_1783 = charge_state_gas_result_2_1086.tup2;
        state_spill_after_3_1784 = charge_state_gas_result_2_1086.tup3;
        if (charge_state_gas_result_2_1086.tup0) {
          gas_after = (uint64_t)GAS_ZERO;
          struct tuple_uint_64_uint_32_FrameStatus result_2_1087;
          struct ExecutionProfileFields execution_profile_8_301 = k_execution_profile;
          bool result_8_303 = (bool)(execution_profile_8_301.protocol.fork >= Amsterdam);
          if (result_8_303) {
            struct FrameStatus Exceptional_result_8_305 = Exceptional(OutOfGas);
            struct tuple_uint_64_uint_8_FrameStatus tmp_8_306 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_305});
            /* conversions */
            result_2_1087.tup0 = tmp_8_306.tup0;
            result_2_1087.tup1 = (uint32_t)tmp_8_306.tup1;
            result_2_1087.tup2 = tmp_8_306.tup2;
            /* end conversions */
          } else {
            struct FrameStatus Exceptional_result_8_307 = Exceptional(OutOfGas);
            result_2_1087 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_307});
          }
          state_gas_after_3_1783 = result_2_1087.tup0;
          state_spill_after_3_1784 = result_2_1087.tup1;
          status_after_3_1785 = result_2_1087.tup2;
          return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        }
      }
      base_child = call_gas_cap_word(gas_after, gas_request);
      struct tuple_bool_uint_64 charge_result_2_1088;
      bool lteq_int_result_8_309 = (bool)(!((__int128)gas_after < (__int128)base_child));
      if (lteq_int_result_8_309) {
        charge_result_2_1088 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)base_child)});
      } else {
        struct tuple_bool_uint_8 tmp_8_313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
        /* conversions */
        charge_result_2_1088.tup0 = tmp_8_313.tup0;
        charge_result_2_1088.tup1 = (uint64_t)tmp_8_313.tup1;
        /* end conversions */
      }
      gas_after = charge_result_2_1088.tup1;
      if (charge_result_2_1088.tup0) {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1089;
        struct ExecutionProfileFields execution_profile_8_314 = k_execution_profile;
        bool result_8_316 = (bool)(execution_profile_8_314.protocol.fork >= Amsterdam);
        if (result_8_316) {
          struct FrameStatus Exceptional_result_8_318 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_319 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_318});
          /* conversions */
          result_2_1089.tup0 = tmp_8_319.tup0;
          result_2_1089.tup1 = (uint32_t)tmp_8_319.tup1;
          result_2_1089.tup2 = tmp_8_319.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_320 = Exceptional(OutOfGas);
          result_2_1089 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_320});
        }
        state_gas_after_3_1783 = result_2_1089.tup0;
        state_spill_after_3_1784 = result_2_1089.tup1;
        status_after_3_1785 = result_2_1089.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    } else {
      base_child = call_gas_cap_word(gas_after, gas_request);
      struct tuple_bool_uint_64 charge_result_2_1090;
      bool lteq_int_result_8_322 = (bool)(!((__int128)gas_after < (__int128)base_child));
      if (lteq_int_result_8_322) {
        charge_result_2_1090 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (__int128)base_child)});
      } else {
        struct tuple_bool_uint_8 tmp_8_326 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
        /* conversions */
        charge_result_2_1090.tup0 = tmp_8_326.tup0;
        charge_result_2_1090.tup1 = (uint64_t)tmp_8_326.tup1;
        /* end conversions */
      }
      gas_after = charge_result_2_1090.tup1;
      if (charge_result_2_1090.tup0) {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1091;
        struct ExecutionProfileFields execution_profile_8_327 = k_execution_profile;
        bool result_8_329 = (bool)(execution_profile_8_327.protocol.fork >= Amsterdam);
        if (result_8_329) {
          struct FrameStatus Exceptional_result_8_331 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_332 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_331});
          /* conversions */
          result_2_1091.tup0 = tmp_8_332.tup0;
          result_2_1091.tup1 = (uint32_t)tmp_8_332.tup1;
          result_2_1091.tup2 = tmp_8_332.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_333 = Exceptional(OutOfGas);
          result_2_1091 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1783, .tup1 = state_spill_after_3_1784, .tup2 = Exceptional_result_8_333});
        }
        state_gas_after_3_1783 = result_2_1091.tup0;
        state_spill_after_3_1784 = result_2_1091.tup1;
        status_after_3_1785 = result_2_1091.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
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
    bool eq_int_result_2_1939 = u256_eq_u64(args_len_word, UINT8_C(0));
    if (eq_int_result_2_1939) {
      args_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), args_off_word)) {
      struct MemoryAccessFields tmp_3_3285;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(args_off_word)), args_len_word)) {
        uint32_t bounded_size_8_336 = (uint32_t)u256_to_u64(args_len_word);
        struct MemoryRangeFields memory_range_result_2_1943 = memory_range((uint32_t)u256_to_u64_unchecked(args_off_word), bounded_size_8_336);
        uint32_t add_atom_result_2_1944 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(args_off_word)), bounded_size_8_336));
        tmp_3_3285 = ((struct MemoryAccessFields){.range = memory_range_result_2_1943, .required_size = add_atom_result_2_1944});
      } else {
        fatal_error(ExecutionInvalid);
      }
      args_access = tmp_3_3285;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct MemoryAccessFields ret_access;
    bool eq_int_result_8_337 = u256_eq_u64(ret_len_word, UINT8_C(0));
    if (eq_int_result_8_337) {
      ret_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), ret_off_word)) {
      struct MemoryAccessFields tmp_8_339;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(ret_off_word)), ret_len_word)) {
        uint32_t bounded_size_8_344 = (uint32_t)u256_to_u64(ret_len_word);
        struct MemoryRangeFields memory_range_result_8_346 = memory_range((uint32_t)u256_to_u64_unchecked(ret_off_word), bounded_size_8_344);
        uint32_t add_atom_result_8_347 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(ret_off_word)), bounded_size_8_344));
        tmp_8_339 = ((struct MemoryAccessFields){.range = memory_range_result_8_346, .required_size = add_atom_result_8_347});
      } else {
        fatal_error(ExecutionInvalid);
      }
      ret_access = tmp_8_339;
    } else {
      fatal_error(ExecutionInvalid);
    }
    uint32_t materialized_required_size = args_access.required_size < ret_access.required_size ? ret_access.required_size : args_access.required_size;
    struct tuple_Bytes_Bytes memory_expand_to_result_2_1933 = memory_expand_to(memory_after, materialized_required_size);
    Bytes tmp_3_3281 = memory_expand_to_result_2_1933.tup1;
    struct MemoryRangeFields args = args_access.range;
    struct MemoryRangeFields ret = ret_access.range;
    uint64_t child_gas = conserved_gas_add(base_child, stipend);
    k_aload_(target);
    struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1718;
    bool insufficient_balance;
    if (semantics.takes_value && value_nonzero) {
      u256 caller_balance = k_get_balance(carried_address);
      bool transfer_affordable = word_ule(value_3_1786, caller_balance);
      insufficient_balance = (bool)(!transfer_affordable);
    } else {
      insufficient_balance = false;
    }
    bool condition_2_1095 = (bool)(insufficient_balance || (carried_depth == UINT16_C(1024)));
    if (condition_2_1095) {
      returndata_after = returndata_clear();
      gas_after = refund_gas(gas_after, child_gas);
      if (tmp_3_1662) {
        struct tuple_uint_64_uint_64_uint_32 result_2_1096 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
        gas_after = result_2_1096.tup0;
        state_gas_after_3_1783 = result_2_1096.tup1;
        state_spill_after_3_1784 = result_2_1096.tup2;
      }
      sp_after = stack_top_advance(sp_after, UINT16_C(1));
      stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
      memory_after = tmp_3_3281;
      tmp_3_1718 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    } else {
      enum PrecompileId selected_precompile = precompile_id_for_address(target);
      bool neq_anything_result_2_1097 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
      if (neq_anything_result_2_1097) {
        struct tuple_Bytes_Bytes result_2_1100 = active_memory_slice(tmp_3_3281, args.off, args.len);
        struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1731;
        struct CalldataSlice input = MemoryCalldata(result_2_1100.tup0);
        struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
        if (precompile_charge.affordable) {
          struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
          if (result.success) {
            returndata_after = result.output;
            if (semantics.transfers_value && value_nonzero) {
              k_transfer(carried_address, target, value_3_1786);
            }
            returndata_copy_prefix(returndata_after, ret.off, ret.len);
            uint64_t unused = gas_sub(child_gas, precompile_charge.cost);
            gas_after = refund_gas(gas_after, unused);
            sp_after = stack_top_advance(sp_after, UINT16_C(1));
            stack_slot_write(sp_after, UINT16_C(0), WORD_ONE);
            memory_after = result_2_1100.tup1;
            tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          } else {
            returndata_after = returndata_clear();
            if (tmp_3_1662) {
              struct tuple_uint_64_uint_64_uint_32 result_2_1107 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
              gas_after = result_2_1107.tup0;
              state_gas_after_3_1783 = result_2_1107.tup1;
              state_spill_after_3_1784 = result_2_1107.tup2;
            }
            sp_after = stack_top_advance(sp_after, UINT16_C(1));
            stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
            memory_after = result_2_1100.tup1;
            tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          }
        } else {
          returndata_after = returndata_clear();
          if (tmp_3_1662) {
            struct tuple_uint_64_uint_64_uint_32 result_2_1108 = credit_state_gas_refund(gas_after, state_gas_after_3_1783, state_spill_after_3_1784, G_amsterdam_state_new_account);
            gas_after = result_2_1108.tup0;
            state_gas_after_3_1783 = result_2_1108.tup1;
            state_spill_after_3_1784 = result_2_1108.tup2;
          }
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
          memory_after = result_2_1100.tup1;
          tmp_3_1731 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1783, .tup3 = state_spill_after_3_1784, .tup4 = carried_refund, .tup5 = status_after_3_1785, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        }
        tmp_3_1718 = tmp_3_1731;
      } else {
        struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
        bytes20 child_addr = semantics.uses_target_address ? target : carried_address;
        bytes20 child_caller = semantics.inherits_caller_and_value ? carried_caller : carried_address;
        u256 child_value = semantics.inherits_caller_and_value ? carried_value : value_3_1786;
        bool child_static = (bool)(semantics.enters_static_context || carried_is_static);
        struct tuple_Bytes_Bytes result_2_1111 = active_memory_slice(tmp_3_3281, args.off, args.len);
        Bytes child_memory = evm_memory_slice(result_2_1111.tup0.bytes, result_2_1111.tup0.len);
        struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
        struct tuple_FrameCheckpoint_StackPointer_Bytes result_2_1113;
        struct FrameStatus Running_result_2_1112 = Running(UNIT);
        result_2_1113 = suspend_frame(carried_pc, gas_after, sp_after, result_2_1111.tup1, STATE_GAS_ZERO, state_spill_after_3_1784, carried_refund, Running_result_2_1112, ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), carried_code, carried_calldata);
        struct FrameContinuation continuation = ResumeCall(((struct CallContinuation){.checkpoint = result_2_1113.tup0, .new_account_charged = tmp_3_1662, .return_length = ret.len, .return_offset = ret.off}));
        frame_stack_push(continuation);
        if (semantics.transfers_value && value_nonzero) {
          k_transfer(carried_address, target, value_3_1786);
        }
        Bytes child_returndata = returndata_clear();
        struct FrameStatus Running_result_2_1116 = Running(UNIT);
        struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1728 = ((struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = UINT8_C(0), .tup1 = child_gas, .tup2 = state_gas_after_3_1783, .tup3 = STATE_GAS_SPILL_ZERO, .tup4 = GAS_REFUND_ZERO, .tup5 = Running_result_2_1116, .tup6 = result_2_1113.tup1, .tup7 = result_2_1113.tup2, .tup8 = child_caller, .tup9 = child_addr, .tup10 = target, .tup11 = child_value, .tup12 = state_gas_after_3_1783, .tup13 = child_static, .tup14 = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .tup15 = child_code, .tup16 = child_calldata, .tup17 = child_returndata});
        /* conversions */
        tmp_3_1718.tup0 = (uint32_t)tmp_3_1728.tup0;
        tmp_3_1718.tup1 = tmp_3_1728.tup1;
        tmp_3_1718.tup2 = tmp_3_1728.tup2;
        tmp_3_1718.tup3 = (uint32_t)tmp_3_1728.tup3;
        tmp_3_1718.tup4 = tmp_3_1728.tup4;
        tmp_3_1718.tup5 = tmp_3_1728.tup5;
        tmp_3_1718.tup6 = tmp_3_1728.tup6;
        tmp_3_1718.tup7 = tmp_3_1728.tup7;
        tmp_3_1718.tup8 = tmp_3_1728.tup8;
        tmp_3_1718.tup9 = tmp_3_1728.tup9;
        tmp_3_1718.tup10 = tmp_3_1728.tup10;
        tmp_3_1718.tup11 = tmp_3_1728.tup11;
        tmp_3_1718.tup12 = tmp_3_1728.tup12;
        tmp_3_1718.tup13 = tmp_3_1728.tup13;
        tmp_3_1718.tup14 = tmp_3_1728.tup14;
        tmp_3_1718.tup15 = tmp_3_1728.tup15;
        tmp_3_1718.tup16 = tmp_3_1728.tup16;
        tmp_3_1718.tup17 = tmp_3_1728.tup17;
        /* end conversions */
      }
    }
    tmp_3_1616 = tmp_3_1718;
    break;
  }
  }
  return tmp_3_1616;
}

struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes run_create_uint32_t_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint16_t_struct_CodeFields_struct_CalldataSlice_Bytes_enum_CreateKind_to_struct_tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes(uint32_t carried_pc, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_sp, Bytes carried_memory, bytes20 carried_caller, bytes20 carried_address, bytes20 carried_code_address, u256 carried_value, uint64_t carried_state_gas_reservoir, bool carried_is_static, uint16_t carried_depth, struct CodeFields carried_code, struct CalldataSlice carried_calldata, Bytes carried_returndata, enum CreateKind kind)
{
  struct OpcodeOutcome result_2_1026;
  uint16_t tmp_3_1608;
  switch (kind) {
  case CreateByNonce:
    tmp_3_1608 = UINT16_C(3);
    break;
  case CreateBySalt:
    tmp_3_1608 = UINT16_C(4);
    break;
  }
  result_2_1026 = guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(carried_sp, tmp_3_1608, UINT8_C(1));
  struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1450;
  switch (result_2_1026.kind) {
  case Kind_Failed:
  {
    struct tuple_uint_64_uint_32_FrameStatus exceptional_state_result_2_1027;
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    bool result_2_2039 = (bool)(execution_profile.protocol.fork >= Amsterdam);
    if (result_2_2039) {
      struct FrameStatus Exceptional_result_2_2040 = Exceptional(result_2_1026.variants.Failed);
      struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
      /* conversions */
      exceptional_state_result_2_1027.tup0 = tmp_3_3353.tup0;
      exceptional_state_result_2_1027.tup1 = (uint32_t)tmp_3_3353.tup1;
      exceptional_state_result_2_1027.tup2 = tmp_3_3353.tup2;
      /* end conversions */
    } else {
      struct FrameStatus Exceptional_result_2_2041 = Exceptional(result_2_1026.variants.Failed);
      exceptional_state_result_2_1027 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = carried_state_gas, .tup1 = carried_state_spill, .tup2 = Exceptional_result_2_2041});
    }
    struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1578 = ((struct tuple_uint_32_uint_8_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = GAS_ZERO, .tup2 = exceptional_state_result_2_1027.tup0, .tup3 = exceptional_state_result_2_1027.tup1, .tup4 = carried_refund, .tup5 = exceptional_state_result_2_1027.tup2, .tup6 = carried_sp, .tup7 = carried_memory, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = carried_returndata});
    /* conversions */
    tmp_3_1450.tup0 = tmp_3_1578.tup0;
    tmp_3_1450.tup1 = (uint64_t)tmp_3_1578.tup1;
    tmp_3_1450.tup2 = tmp_3_1578.tup2;
    tmp_3_1450.tup3 = tmp_3_1578.tup3;
    tmp_3_1450.tup4 = tmp_3_1578.tup4;
    tmp_3_1450.tup5 = tmp_3_1578.tup5;
    tmp_3_1450.tup6 = tmp_3_1578.tup6;
    tmp_3_1450.tup7 = tmp_3_1578.tup7;
    tmp_3_1450.tup8 = tmp_3_1578.tup8;
    tmp_3_1450.tup9 = tmp_3_1578.tup9;
    tmp_3_1450.tup10 = tmp_3_1578.tup10;
    tmp_3_1450.tup11 = tmp_3_1578.tup11;
    tmp_3_1450.tup12 = tmp_3_1578.tup12;
    tmp_3_1450.tup13 = tmp_3_1578.tup13;
    tmp_3_1450.tup14 = tmp_3_1578.tup14;
    tmp_3_1450.tup15 = tmp_3_1578.tup15;
    tmp_3_1450.tup16 = tmp_3_1578.tup16;
    tmp_3_1450.tup17 = tmp_3_1578.tup17;
    /* end conversions */
    break;
  }
  case Kind_Continue:
  {
    uint64_t gas_after = carried_gas;
    uint64_t state_gas_after_3_1581 = carried_state_gas;
    uint32_t state_spill_after_3_1582 = carried_state_spill;
    struct FrameStatus status_after_3_1583 = Running(UNIT);
    StackPointer sp_after = carried_sp;
    Bytes memory_after = carried_memory;
    Bytes returndata_after = carried_returndata;
    struct CreateSemantics semantics = create_semantics(kind);
    struct ExecutionProfileFields execution_profile_8_350 = k_execution_profile;
    struct ProtocolProfileFields profile_8_351 = execution_profile_8_350.protocol;
    u256 value = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 off_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    u256 len_word = stack_slot_read(sp_after, UINT16_C(0));
    sp_after = stack_top_retreat(sp_after, UINT16_C(1));
    struct tuple_u256_StackPointer result_2_1030;
    if (semantics.uses_salt) {
      u256 salt = stack_slot_read(sp_after, UINT16_C(0));
      StackPointer stack_top_retreat_result_2_1029 = stack_top_retreat(sp_after, UINT16_C(1));
      result_2_1030 = ((struct tuple_u256_StackPointer){.tup0 = salt, .tup1 = stack_top_retreat_result_2_1029});
    } else {
      result_2_1030 = ((struct tuple_u256_StackPointer){.tup0 = WORD_ZERO, .tup1 = sp_after});
    }
    sp_after = result_2_1030.tup1;
    if (carried_is_static) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1031;
      struct ExecutionProfileFields execution_profile_8_352 = k_execution_profile;
      bool result_8_354 = (bool)(execution_profile_8_352.protocol.fork >= Amsterdam);
      if (result_8_354) {
        struct FrameStatus Exceptional_result_8_356 = Exceptional(WriteProtection);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_357 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_356});
        /* conversions */
        result_2_1031.tup0 = tmp_8_357.tup0;
        result_2_1031.tup1 = (uint32_t)tmp_8_357.tup1;
        result_2_1031.tup2 = tmp_8_357.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_358 = Exceptional(WriteProtection);
        result_2_1031 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_358});
      }
      state_gas_after_3_1581 = result_2_1031.tup0;
      state_spill_after_3_1582 = result_2_1031.tup1;
      status_after_3_1583 = result_2_1031.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    uint64_t required_size;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      required_size = UINT64_C(0);
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      uint64_t tmp_3_3290;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size = (uint32_t)u256_to_u64(len_word);
        tmp_3_3290 = u256_to_u64_unchecked(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size));
      } else {
        tmp_3_3290 = UINT64_C(4294967296);
      }
      required_size = tmp_3_3290;
    } else {
      required_size = UINT64_C(4294967296);
    }
    struct tuple_bool_uint_64 charge_memory_expansion_result_2_1032;
    if (required_size <= UINT32_C(4294967295)) {
      uint32_t new_words = memory_word_count_uint32_t_to_uint32_t((uint32_t)required_size);
      uint32_t old_words;
      uint32_t memory_high_water_result_2_1938 = memory_high_water(memory_after);
      old_words = memory_word_count_uint32_t_to_uint32_t(memory_high_water_result_2_1938);
      if (new_words <= old_words) {
        charge_memory_expansion_result_2_1032 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after});
      } else {
        uint64_t linear = ((uint64_t)G_memory * (uint64_t)old_words);
        uint64_t linear_8_360 = ((uint64_t)G_memory * (uint64_t)new_words);
        if (!((__int128)gas_after < (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_360) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))) {
          charge_memory_expansion_result_2_1032 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (uint64_t)((__int128)gas_after - (((__int128)(((uint64_t)new_words * (uint64_t)new_words) >> 9) + (__int128)linear_8_360) - ((__int128)(((uint64_t)old_words * (uint64_t)old_words) >> 9) + (__int128)linear)))});
        } else {
          struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
          /* conversions */
          charge_memory_expansion_result_2_1032.tup0 = tmp_3_3313.tup0;
          charge_memory_expansion_result_2_1032.tup1 = (uint64_t)tmp_3_3313.tup1;
          /* end conversions */
        }
      }
    } else {
      struct tuple_bool_uint_8 tmp_3_3283 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_memory_expansion_result_2_1032.tup0 = tmp_3_3283.tup0;
      charge_memory_expansion_result_2_1032.tup1 = (uint64_t)tmp_3_3283.tup1;
      /* end conversions */
    }
    gas_after = charge_memory_expansion_result_2_1032.tup1;
    if (charge_memory_expansion_result_2_1032.tup0) {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1033;
      struct ExecutionProfileFields execution_profile_8_363 = k_execution_profile;
      bool result_8_365 = (bool)(execution_profile_8_363.protocol.fork >= Amsterdam);
      if (result_8_365) {
        struct FrameStatus Exceptional_result_8_367 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_368 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_367});
        /* conversions */
        result_2_1033.tup0 = tmp_8_368.tup0;
        result_2_1033.tup1 = (uint32_t)tmp_8_368.tup1;
        result_2_1033.tup2 = tmp_8_368.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_369 = Exceptional(OutOfGas);
        result_2_1033 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_369});
      }
      state_gas_after_3_1581 = result_2_1033.tup0;
      state_spill_after_3_1582 = result_2_1033.tup1;
      status_after_3_1583 = result_2_1033.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    struct MemoryAccessFields initcode_access;
    if (u256_eq_u64(len_word, UINT8_C(0))) {
      initcode_access = EMPTY_MEMORY_ACCESS;
    } else if (!u64_lt_u256(UINT32_C(4294967295), off_word)) {
      struct MemoryAccessFields tmp_3_3285;
      if (!u64_lt_u256((UINT32_C(4294967295) - (uint32_t)u256_to_u64_unchecked(off_word)), len_word)) {
        uint32_t bounded_size_8_372 = (uint32_t)u256_to_u64(len_word);
        struct MemoryRangeFields memory_range_result_2_1943 = memory_range((uint32_t)u256_to_u64_unchecked(off_word), bounded_size_8_372);
        uint32_t add_atom_result_2_1944 = (uint32_t)u256_to_u64(u256_add_u64(u256_of_fbits((uint32_t)u256_to_u64_unchecked(off_word)), bounded_size_8_372));
        tmp_3_3285 = ((struct MemoryAccessFields){.range = memory_range_result_2_1943, .required_size = add_atom_result_2_1944});
      } else {
        fatal_error(ExecutionInvalid);
      }
      initcode_access = tmp_3_3285;
    } else {
      fatal_error(ExecutionInvalid);
    }
    struct tuple_Bytes_Bytes memory_expand_to_result_2_1933 = memory_expand_to(memory_after, initcode_access.required_size);
    Bytes tmp_3_3281 = memory_expand_to_result_2_1933.tup1;
    struct MemoryRangeFields initcode = initcode_access.range;
    uint16_t access_cost = create_access_cost();
    struct tuple_bool_uint_64 charge_result_2_1034;
    bool lteq_int_result_8_373 = (bool)(access_cost <= gas_after);
    if (lteq_int_result_8_373) {
      charge_result_2_1034 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (gas_after - (uint64_t)access_cost)});
    } else {
      struct tuple_bool_uint_8 tmp_8_376 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_1034.tup0 = tmp_8_376.tup0;
      charge_result_2_1034.tup1 = (uint64_t)tmp_8_376.tup1;
      /* end conversions */
    }
    struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1477;
    gas_after = charge_result_2_1034.tup1;
    if (charge_result_2_1034.tup0) {
      memory_after = tmp_3_3281;
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1035;
      struct ExecutionProfileFields execution_profile_8_377 = k_execution_profile;
      bool result_8_379 = (bool)(execution_profile_8_377.protocol.fork >= Amsterdam);
      if (result_8_379) {
        struct FrameStatus Exceptional_result_8_381 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_382 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_381});
        /* conversions */
        result_2_1035.tup0 = tmp_8_382.tup0;
        result_2_1035.tup1 = (uint32_t)tmp_8_382.tup1;
        result_2_1035.tup2 = tmp_8_382.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_383 = Exceptional(OutOfGas);
        result_2_1035 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_383});
      }
      state_gas_after_3_1581 = result_2_1035.tup0;
      state_spill_after_3_1582 = result_2_1035.tup1;
      status_after_3_1583 = result_2_1035.tup2;
      return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    u256 initcode_word_count;
    u256 quotient = word_div_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    u256 remainder = word_mod_word_u256_u256_to_u256(len_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
    bool eq_int_result_2_1949 = eq_u256(remainder, WORD_ZERO);
    if (eq_int_result_2_1949) {
      initcode_word_count = quotient;
    } else {
      initcode_word_count = word_add_word(quotient, WORD_ONE);
    }
    bool result_2_1037 = (bool)(profile_8_351.fork >= Shanghai);
    if (result_2_1037) {
      struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1038 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64_variant_2(gas_after, G_initcode_word, initcode_word_count);
      gas_after = charge_word_scaled_gas_result_2_1038.tup1;
      if (charge_word_scaled_gas_result_2_1038.tup0) {
        memory_after = tmp_3_3281;
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1039;
        struct ExecutionProfileFields execution_profile_8_385 = k_execution_profile;
        bool result_8_387 = (bool)(execution_profile_8_385.protocol.fork >= Amsterdam);
        if (result_8_387) {
          struct FrameStatus Exceptional_result_8_389 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_390 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_389});
          /* conversions */
          result_2_1039.tup0 = tmp_8_390.tup0;
          result_2_1039.tup1 = (uint32_t)tmp_8_390.tup1;
          result_2_1039.tup2 = tmp_8_390.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_391 = Exceptional(OutOfGas);
          result_2_1039 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_391});
        }
        state_gas_after_3_1581 = result_2_1039.tup0;
        state_spill_after_3_1582 = result_2_1039.tup1;
        status_after_3_1583 = result_2_1039.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    }
    if (semantics.uses_salt) {
      struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1041 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(gas_after, G_keccak_word, initcode_word_count);
      gas_after = charge_word_scaled_gas_result_2_1041.tup1;
      if (charge_word_scaled_gas_result_2_1041.tup0) {
        memory_after = tmp_3_3281;
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_1042;
        struct ExecutionProfileFields execution_profile_8_393 = k_execution_profile;
        bool result_8_395 = (bool)(execution_profile_8_393.protocol.fork >= Amsterdam);
        if (result_8_395) {
          struct FrameStatus Exceptional_result_8_397 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_398 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_397});
          /* conversions */
          result_2_1042.tup0 = tmp_8_398.tup0;
          result_2_1042.tup1 = (uint32_t)tmp_8_398.tup1;
          result_2_1042.tup2 = tmp_8_398.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_399 = Exceptional(OutOfGas);
          result_2_1042 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_399});
        }
        state_gas_after_3_1581 = result_2_1042.tup0;
        state_spill_after_3_1582 = result_2_1042.tup1;
        status_after_3_1583 = result_2_1042.tup2;
        return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      }
    }
    bool valid_initcode_size = initcode_size_allowed(initcode.len);
    if (valid_initcode_size) {
      uint64_t nonce = k_get_nonce(carried_address);
      Bytes mem2 = tmp_3_3281;
      bytes20 new_addr;
      if (semantics.uses_salt) {
        struct tuple_u256_Bytes mem_keccak_result_2_1065 = mem_keccak(tmp_3_3281, initcode);
        mem2 = mem_keccak_result_2_1065.tup1;
        bytes32 initcode_digest = word_to_hash(mem_keccak_result_2_1065.tup0);
        new_addr = k_create2_addr(carried_address, result_2_1030.tup0, initcode_digest);
      } else {
        new_addr = k_create_addr(carried_address, nonce);
      }
      uint64_t child_gas = (uint64_t)GAS_ZERO;
      bool result_2_1045 = (bool)(profile_8_351.fork < Amsterdam);
      if (result_2_1045) {
        uint64_t avail = gas_after;
        uint64_t retained_gas = (avail >> 6);
        child_gas = gas_sub(avail, retained_gas);
        gas_after = retained_gas;
      }
      u256 creator_balance = k_get_balance(carried_address);
      bool endowment_affordable = word_ule(value, creator_balance);
      bool tmp_3_1519 = (bool)(!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (carried_depth == UINT16_C(1024))));
      if (tmp_3_1519) {
        returndata_after = returndata_clear();
        bool result_2_1049 = (bool)(profile_8_351.fork < Amsterdam);
        if (result_2_1049) {
          gas_after = refund_gas(gas_after, child_gas);
        } else {
          gas_after = gas_after;
        }
        sp_after = stack_top_advance(sp_after, UINT16_C(1));
        stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
        memory_after = mem2;
        tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
      } else {
        k_account_mark_warm(new_addr);
        bool result_2_1063 = (bool)(profile_8_351.fork >= Amsterdam);
        bool tmp_3_1521;
        if (result_2_1063) {
          tmp_3_1521 = k_account_is_empty(new_addr);
        } else {
          tmp_3_1521 = false;
        }
        if (tmp_3_1521) {
          struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_1050 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after_3_1581, state_spill_after_3_1582, G_amsterdam_state_new_account);
          gas_after = charge_state_gas_result_2_1050.tup1;
          state_gas_after_3_1581 = charge_state_gas_result_2_1050.tup2;
          state_spill_after_3_1582 = charge_state_gas_result_2_1050.tup3;
          if (charge_state_gas_result_2_1050.tup0) {
            memory_after = mem2;
            gas_after = (uint64_t)GAS_ZERO;
            struct tuple_uint_64_uint_32_FrameStatus result_2_1051;
            struct ExecutionProfileFields execution_profile_8_409 = k_execution_profile;
            bool result_8_411 = (bool)(execution_profile_8_409.protocol.fork >= Amsterdam);
            if (result_8_411) {
              struct FrameStatus Exceptional_result_8_413 = Exceptional(OutOfGas);
              struct tuple_uint_64_uint_8_FrameStatus tmp_8_414 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_413});
              /* conversions */
              result_2_1051.tup0 = tmp_8_414.tup0;
              result_2_1051.tup1 = (uint32_t)tmp_8_414.tup1;
              result_2_1051.tup2 = tmp_8_414.tup2;
              /* end conversions */
            } else {
              struct FrameStatus Exceptional_result_8_415 = Exceptional(OutOfGas);
              result_2_1051 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_415});
            }
            state_gas_after_3_1581 = result_2_1051.tup0;
            state_spill_after_3_1582 = result_2_1051.tup1;
            status_after_3_1583 = result_2_1051.tup2;
            return ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
          }
        }
        bool result_2_1053 = (bool)(profile_8_351.fork >= Amsterdam);
        if (result_2_1053) {
          uint64_t avail_3_1603 = gas_after;
          uint64_t retained_gas_3_1604 = (avail_3_1603 >> 6);
          child_gas = gas_sub(avail_3_1603, retained_gas_3_1604);
          gas_after = retained_gas_3_1604;
        }
        bool occupied = k_account_occupied(new_addr);
        returndata_after = returndata_clear();
        k_bump_nonce(carried_address);
        if (occupied) {
          if (tmp_3_1521) {
            struct tuple_uint_64_uint_64_uint_32 result_2_1054 = credit_state_gas_refund(gas_after, state_gas_after_3_1581, state_spill_after_3_1582, G_amsterdam_state_new_account);
            gas_after = result_2_1054.tup0;
            state_gas_after_3_1581 = result_2_1054.tup1;
            state_spill_after_3_1582 = result_2_1054.tup2;
          }
          sp_after = stack_top_advance(sp_after, UINT16_C(1));
          stack_slot_write(sp_after, UINT16_C(0), WORD_ZERO);
          memory_after = mem2;
          tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
        } else {
          struct tuple_Bytes_Bytes_1 result_2_1057 = memory_code_slice(mem2, initcode.off, initcode.len);
          bytes32 child_code_id = code_db_insert(result_2_1057.tup0, profile_8_351.fork);
          struct CodeFields child_code = code_db_resolve(child_code_id);
          struct tuple_FrameCheckpoint_StackPointer_Bytes result_2_1059;
          struct FrameStatus Running_result_2_1058 = Running(UNIT);
          result_2_1059 = suspend_frame(carried_pc, gas_after, sp_after, result_2_1057.tup1, STATE_GAS_ZERO, state_spill_after_3_1582, carried_refund, Running_result_2_1058, ((struct Message){.address = carried_address, .caller = carried_caller, .code_address = carried_code_address, .depth = carried_depth, .is_static = carried_is_static, .state_gas_reservoir = carried_state_gas_reservoir, .value = carried_value}), carried_code, carried_calldata);
          struct FrameContinuation continuation = ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = result_2_1059.tup0, .new_account_charged = tmp_3_1521}));
          frame_stack_push(continuation);
          k_mark_created(new_addr);
          k_clear_storage(new_addr);
          k_bump_nonce(new_addr);
          k_transfer(carried_address, new_addr, value);
          Bytes child_returndata = returndata_clear();
          struct FrameStatus Running_result_2_1060 = Running(UNIT);
          struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes tmp_3_1548 = ((struct tuple_uint_8_uint_64_uint_64_uint_8_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = UINT8_C(0), .tup1 = child_gas, .tup2 = state_gas_after_3_1581, .tup3 = STATE_GAS_SPILL_ZERO, .tup4 = GAS_REFUND_ZERO, .tup5 = Running_result_2_1060, .tup6 = result_2_1059.tup1, .tup7 = result_2_1059.tup2, .tup8 = carried_address, .tup9 = new_addr, .tup10 = new_addr, .tup11 = value, .tup12 = state_gas_after_3_1581, .tup13 = carried_is_static, .tup14 = ((uint16_t)((uint32_t)carried_depth + (uint32_t)UINT16_C(1))), .tup15 = child_code, .tup16 = EMPTY_CALLDATA, .tup17 = child_returndata});
          /* conversions */
          tmp_3_1477.tup0 = (uint32_t)tmp_3_1548.tup0;
          tmp_3_1477.tup1 = tmp_3_1548.tup1;
          tmp_3_1477.tup2 = tmp_3_1548.tup2;
          tmp_3_1477.tup3 = (uint32_t)tmp_3_1548.tup3;
          tmp_3_1477.tup4 = tmp_3_1548.tup4;
          tmp_3_1477.tup5 = tmp_3_1548.tup5;
          tmp_3_1477.tup6 = tmp_3_1548.tup6;
          tmp_3_1477.tup7 = tmp_3_1548.tup7;
          tmp_3_1477.tup8 = tmp_3_1548.tup8;
          tmp_3_1477.tup9 = tmp_3_1548.tup9;
          tmp_3_1477.tup10 = tmp_3_1548.tup10;
          tmp_3_1477.tup11 = tmp_3_1548.tup11;
          tmp_3_1477.tup12 = tmp_3_1548.tup12;
          tmp_3_1477.tup13 = tmp_3_1548.tup13;
          tmp_3_1477.tup14 = tmp_3_1548.tup14;
          tmp_3_1477.tup15 = tmp_3_1548.tup15;
          tmp_3_1477.tup16 = tmp_3_1548.tup16;
          tmp_3_1477.tup17 = tmp_3_1548.tup17;
          /* end conversions */
        }
      }
    } else {
      memory_after = tmp_3_3281;
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_1043;
      struct ExecutionProfileFields execution_profile_8_401 = k_execution_profile;
      bool result_8_403 = (bool)(execution_profile_8_401.protocol.fork >= Amsterdam);
      if (result_8_403) {
        struct FrameStatus Exceptional_result_8_405 = Exceptional(InitCodeTooLarge);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_406 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = carried_state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_405});
        /* conversions */
        result_2_1043.tup0 = tmp_8_406.tup0;
        result_2_1043.tup1 = (uint32_t)tmp_8_406.tup1;
        result_2_1043.tup2 = tmp_8_406.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_407 = Exceptional(InitCodeTooLarge);
        result_2_1043 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after_3_1581, .tup1 = state_spill_after_3_1582, .tup2 = Exceptional_result_8_407});
      }
      state_gas_after_3_1581 = result_2_1043.tup0;
      state_spill_after_3_1582 = result_2_1043.tup1;
      status_after_3_1583 = result_2_1043.tup2;
      tmp_3_1477 = ((struct tuple_uint_32_uint_64_uint_64_uint_32_int_128_FrameStatus_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint_64_bool_uint_16_CodeFields_CalldataSlice_Bytes){.tup0 = carried_pc, .tup1 = gas_after, .tup2 = state_gas_after_3_1581, .tup3 = state_spill_after_3_1582, .tup4 = carried_refund, .tup5 = status_after_3_1583, .tup6 = sp_after, .tup7 = memory_after, .tup8 = carried_caller, .tup9 = carried_address, .tup10 = carried_code_address, .tup11 = carried_value, .tup12 = carried_state_gas_reservoir, .tup13 = carried_is_static, .tup14 = carried_depth, .tup15 = carried_code, .tup16 = carried_calldata, .tup17 = returndata_after});
    }
    tmp_3_1450 = tmp_3_1477;
    break;
  }
  }
  return tmp_3_1450;
}

