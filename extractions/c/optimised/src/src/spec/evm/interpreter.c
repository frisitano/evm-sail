#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct CreateSemantics create_semantics(enum CreateKind kind)
{
  switch (kind) {
  case CreateByNonce:
    return ((struct CreateSemantics){.uses_salt = false});
  case CreateBySalt:
    return ((struct CreateSemantics){.uses_salt = true});
  }
}

struct tuple_uint_32_bits_64_Bytes_uint_64 run_create(enum CreateKind kind, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  struct CreateSemantics semantics = create_semantics(kind);
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint16_t current_depth = call_depth;
  bytes20 creator = self_addr();
  struct tuple_u256_bits_64 pop_result_2_1134 = pop(top);
  struct tuple_u256_bits_64 pop_result_2_1135 = pop(pop_result_2_1134.tup1);
  u256 off_word = pop_result_2_1135.tup0;
  struct tuple_u256_bits_64 pop_result_2_1136 = pop(pop_result_2_1135.tup1);
  u256 len_word = pop_result_2_1136.tup0;
  struct tuple_u256_bits_64 result_2_1138;
  if (semantics.uses_salt) {
    result_2_1138 = pop(pop_result_2_1136.tup1);
  } else {
    result_2_1138 = ((struct tuple_u256_bits_64){.tup0 = WORD_ZERO, .tup1 = pop_result_2_1136.tup1});
  }
  u256 salt = result_2_1138.tup0;
  uint64_t top4 = result_2_1138.tup1;
  struct tuple_bool_uint_64 guard_static_result_2_1139 = guard_static(g);
  bool static_violation = guard_static_result_2_1139.tup0;
  uint64_t g0 = guard_static_result_2_1139.tup1;
  if (static_violation) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem, .tup3 = g0});
  }
  uint64_t required_size = memory_required_size(off_word, len_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1140 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g0, expansion_cost);
  uint64_t g1 = charge_result_2_1140.tup1;
  bool expansion_out_of_gas = (bool)(!charge_result_2_1140.tup0);
  if (expansion_out_of_gas) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem, .tup3 = g1});
  }
  struct MemoryAccessFields initcode_access = memory_access(off_word, len_word);
  Bytes mem1 = expand_memory(mem, initcode_access.required_size);
  struct MemoryRangeFields initcode = initcode_access.range;
  uint16_t access_cost = create_access_cost();
  struct tuple_bool_uint_64 charge_result_2_1141 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g1, access_cost);
  struct tuple_uint_32_bits_64_Bytes_uint_64 tmp_3_2117;
  uint64_t g2 = charge_result_2_1141.tup1;
  bool access_out_of_gas = (bool)(!charge_result_2_1141.tup0);
  if (access_out_of_gas) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem1, .tup3 = g2});
  }
  uint64_t g3 = g2;
  u256 initcode_word_count = memory_word_count_word(len_word);
  bool result_2_1143 = (bool)(profile.fork >= Shanghai);
  if (result_2_1143) {
    struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1144 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(g3, G_initcode_word, initcode_word_count);
    uint64_t initcode_gas = charge_word_scaled_gas_result_2_1144.tup1;
    g3 = initcode_gas;
    if (!charge_word_scaled_gas_result_2_1144.tup0) {
      return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem1, .tup3 = initcode_gas});
    }
  }
  bool result_2_1145 = semantics.uses_salt;
  if (result_2_1145) {
    struct tuple_bool_uint_64 charge_word_scaled_gas_result_2_1146 = charge_word_scaled_gas_uint64_t_uint8_t_u256_to_struct_tuple_bool_uint_64(g3, G_keccak_word, initcode_word_count);
    uint64_t hashing_gas = charge_word_scaled_gas_result_2_1146.tup1;
    g3 = hashing_gas;
    if (!charge_word_scaled_gas_result_2_1146.tup0) {
      return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem1, .tup3 = hashing_gas});
    }
  }
  bool valid_initcode_size = initcode_size_allowed(initcode.len);
  bool invalid_initcode_size = (bool)(!valid_initcode_size);
  if (invalid_initcode_size) {
    uint64_t exc_halt_result_2_1147 = exc_halt(g3, InitCodeTooLarge);
    tmp_3_2117 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem1, .tup3 = exc_halt_result_2_1147});
  } else {
    uint64_t nonce = k_get_nonce(creator);
    Bytes mem2 = mem1;
    bytes20 new_addr;
    if (semantics.uses_salt) {
      struct tuple_u256_Bytes mem_keccak_result_2_1172 = mem_keccak(mem1, initcode);
      mem2 = mem_keccak_result_2_1172.tup1;
      bytes32 initcode_digest = word_to_hash(mem_keccak_result_2_1172.tup0);
      new_addr = k_create2_addr(creator, salt, initcode_digest);
    } else {
      new_addr = k_create_addr(creator, nonce);
    }
    uint64_t g4 = g3;
    uint64_t child_gas = GAS_ZERO;
    bool result_2_1149 = (bool)(profile.fork < Amsterdam);
    if (result_2_1149) {
      uint64_t avail = g4;
      uint64_t retained_gas = (avail >> 6);
      child_gas = gas_sub(avail, retained_gas);
      g4 = retained_gas;
    }
    bool result_2_1150 = message.is_static;
    if (result_2_1150) {
      uint64_t exc_halt_result_2_1151 = exc_halt(g4, WriteProtection);
      return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem2, .tup3 = exc_halt_result_2_1151});
    }
    u256 creator_balance = k_get_balance(creator);
    bool endowment_affordable = word_ule(pop_result_2_1134.tup0, creator_balance);
    bool tmp_3_2139 = (bool)(!endowment_affordable || ((nonce == UINT64_C(18446744073709551615)) || (current_depth == UINT16_C(1024))));
    if (tmp_3_2139) {
      returndata_clear();
      uint64_t g5;
      bool result_2_1157 = (bool)(profile.fork < Amsterdam);
      if (result_2_1157) {
        g5 = refund_gas(g4, child_gas);
      } else {
        g5 = g4;
      }
      uint64_t push_word_result_2_1155 = push_word(top4, WORD_ZERO);
      tmp_3_2117 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1155, .tup2 = mem2, .tup3 = g5});
    } else {
      k_account_mark_warm(new_addr);
      bool result_2_1170 = (bool)(profile.fork >= Amsterdam);
      bool tmp_3_2141;
      if (result_2_1170) {
        tmp_3_2141 = k_account_is_empty(new_addr);
      } else {
        tmp_3_2141 = false;
      }
      if (tmp_3_2141) {
        struct tuple_bool_uint_64 charge_state_gas_result_2_1158 = charge_state_gas(g4, G_amsterdam_state_new_account);
        uint64_t state_gas = charge_state_gas_result_2_1158.tup1;
        g4 = state_gas;
        if (!charge_state_gas_result_2_1158.tup0) {
          return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top4, .tup2 = mem2, .tup3 = state_gas});
        }
      }
      bool result_2_1160 = (bool)(profile.fork >= Amsterdam);
      if (result_2_1160) {
        uint64_t avail_3_2187 = g4;
        uint64_t retained_gas_3_2188 = (avail_3_2187 >> 6);
        child_gas = gas_sub(avail_3_2187, retained_gas_3_2188);
        g4 = retained_gas_3_2188;
      }
      bool occupied = k_account_occupied(new_addr);
      returndata_clear();
      k_bump_nonce(creator);
      if (occupied) {
        uint64_t g5_3_2189;
        if (tmp_3_2141) {
          g5_3_2189 = credit_state_gas_refund(g4, G_amsterdam_state_new_account);
        } else {
          g5_3_2189 = g4;
        }
        uint64_t push_word_result_2_1161 = push_word(top4, WORD_ZERO);
        tmp_3_2117 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1161, .tup2 = mem2, .tup3 = g5_3_2189});
      } else {
        struct tuple_Bytes_Bytes_1 result_2_1164 = memory_code_slice(mem2, initcode.off, initcode.len);
        bytes32 child_code_id = code_db_insert(result_2_1164.tup0, profile.fork);
        struct CodeFields child_code = code_db_resolve(child_code_id);
        pc = pc_in;
        gas_remaining = g4;
        stack_top = top4;
        evm_memory = result_2_1164.tup1;
        struct FrameCheckpoint suspend_frame_result_2_1167 = suspend_frame();
        struct FrameCheckpoint tmp_3_2156 = suspend_frame_result_2_1167;
        tmp_3_2156.state_gas_remaining = GAS_ZERO;
        struct FrameContinuation continuation = ResumeCreate(((struct CreateContinuation){.address = new_addr, .checkpoint = tmp_3_2156, .new_account_charged = tmp_3_2141}));
        frame_stack_push(continuation);
        k_mark_created(new_addr);
        k_clear_storage(new_addr);
        k_bump_nonce(new_addr);
        k_transfer(creator, new_addr, pop_result_2_1134.tup0);
        message = ((struct Message){.address = new_addr, .caller = creator, .code_address = new_addr, .depth = ((uint16_t)((uint32_t)current_depth + (uint32_t)UINT16_C(1))), .is_static = tmp_3_2156.message.is_static, .state_gas_reservoir = state_gas_remaining, .value = pop_result_2_1134.tup0});
        calldata_install(EMPTY_CALLDATA);
        pc = UINT32_C(0);
        gas_remaining = child_gas;
        state_gas_spilled = STATE_GAS_SPILL_ZERO;
        frame_status = Running(UNIT);
        returndata_clear();
        frame_code = child_code;
        call_depth = ((uint16_t)((uint32_t)current_depth + (uint32_t)UINT16_C(1)));
        frame_refund = GAS_REFUND_ZERO;
        struct tuple_uint_8_bits_64_Bytes_uint_64 tmp_3_2175 = ((struct tuple_uint_8_bits_64_Bytes_uint_64){.tup0 = UINT8_C(0), .tup1 = stack_top, .tup2 = evm_memory, .tup3 = child_gas});
        /* conversions */
        tmp_3_2117.tup0 = (uint32_t)tmp_3_2175.tup0;
        tmp_3_2117.tup1 = tmp_3_2175.tup1;
        tmp_3_2117.tup2 = tmp_3_2175.tup2;
        tmp_3_2117.tup3 = tmp_3_2175.tup3;
        /* end conversions */
      }
    }
  }
  return tmp_3_2117;
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

struct CodeFields executable_code(bytes20 target, bool dele, bytes20 dtgt)
{
  if (dele) {
    bytes32 delegate_key = k_code_key(dtgt);
    struct CodeFields delegate_code = code_db_resolve(delegate_key);
    enum PrecompileId delegate_precompile = precompile_id_for_address(dtgt);
    bool neq_anything_result_2_1133 = neq_anything_EPrecompileId__(delegate_precompile, NotPrecompile);
    if (neq_anything_result_2_1133) {
      delegate_code = EMPTY_CODE;
    }
    return delegate_code;
  }
  bytes32 target_key = k_code_key(target);
  return code_db_resolve(target_key);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 run_call(enum CallKind kind, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  struct CallSemantics semantics = call_semantics(kind);
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  uint16_t current_depth = call_depth;
  bytes20 caller = self_addr();
  struct tuple_u256_bits_64 pop_result_2_1067 = pop(top);
  struct tuple_u256_bits_64 pop_result_2_1068 = pop(pop_result_2_1067.tup1);
  bytes20 target = word_to_address(pop_result_2_1068.tup0);
  struct tuple_u256_bits_64 result_2_1070;
  if (semantics.takes_value) {
    result_2_1070 = pop(pop_result_2_1068.tup1);
  } else {
    result_2_1070 = ((struct tuple_u256_bits_64){.tup0 = WORD_ZERO, .tup1 = pop_result_2_1068.tup1});
  }
  u256 value = result_2_1070.tup0;
  bool value_nonzero = word_nonzero(value);
  struct tuple_u256_bits_64 pop_result_2_1071 = pop(result_2_1070.tup1);
  u256 args_off_word = pop_result_2_1071.tup0;
  struct tuple_u256_bits_64 pop_result_2_1072 = pop(pop_result_2_1071.tup1);
  u256 args_len_word = pop_result_2_1072.tup0;
  struct tuple_u256_bits_64 pop_result_2_1073 = pop(pop_result_2_1072.tup1);
  u256 ret_off_word = pop_result_2_1073.tup0;
  struct tuple_u256_bits_64 pop_result_2_1074 = pop(pop_result_2_1073.tup1);
  u256 ret_len_word = pop_result_2_1074.tup0;
  uint64_t top7 = pop_result_2_1074.tup1;
  bool result_2_1076 = (bool)(semantics.transfers_value && (value_nonzero && message.is_static));
  if (result_2_1076) {
    uint64_t exc_halt_result_2_1077 = exc_halt(g, WriteProtection);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = exc_halt_result_2_1077});
  }
  bool warm = k_account_is_warm(target);
  uint16_t target_cost = account_cost(warm);
  uint16_t transfer_cost;
  if (value_nonzero) {
    transfer_cost = call_value_cost();
  } else {
    transfer_cost = GAS_CONSTANT_ZERO;
  }
  uint64_t args_required = memory_required_size(args_off_word, args_len_word);
  uint64_t ret_required = memory_required_size(ret_off_word, ret_len_word);
  uint64_t required_size = args_required < ret_required ? ret_required : args_required;
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1079 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g, expansion_cost);
  uint64_t g1 = charge_result_2_1079.tup1;
  bool expansion_out_of_gas = (bool)(!charge_result_2_1079.tup0);
  if (expansion_out_of_gas) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = g1});
  }
  struct tuple_bool_uint_64 charge_result_2_1080 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g1, ((uint64_t)(uint32_t)target_cost + (uint64_t)(uint32_t)transfer_cost));
  uint64_t g2 = charge_result_2_1080.tup1;
  bool static_base_out_of_gas = (bool)(!charge_result_2_1080.tup0);
  if (static_base_out_of_gas) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = g2});
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
  bool result_2_1131 = (bool)(profile.fork >= Amsterdam);
  bool tmp_3_2009;
  if (result_2_1131) {
    bool tmp_3_2008 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
    tmp_3_2009 = tmp_3_2008;
  } else {
    tmp_3_2009 = false;
  }
  uint16_t create_cost;
  bool result_2_1127 = (bool)(profile.fork < Amsterdam);
  bool tmp_3_2012;
  if (result_2_1127) {
    bool tmp_3_2011 = (bool)(value_nonzero && (semantics.transfers_value && target_empty));
    tmp_3_2012 = tmp_3_2011;
  } else {
    tmp_3_2012 = false;
  }
  create_cost = tmp_3_2012 ? G_newaccount : GAS_CONSTANT_ZERO;
  struct tuple_bool_uint_64 charge_result_2_1082 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g2, ((uint64_t)(uint32_t)delegation_cost + (uint64_t)(uint32_t)create_cost));
  uint64_t g3 = charge_result_2_1082.tup1;
  bool additional_cost_out_of_gas = (bool)(!charge_result_2_1082.tup0);
  if (additional_cost_out_of_gas) {
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = g3});
  }
  uint64_t stipend = value_nonzero ? G_callstipend : GAS_ZERO;
  uint64_t g4 = g3;
  uint64_t base_child = GAS_ZERO;
  bool result_2_1084 = (bool)(profile.fork >= Amsterdam);
  if (result_2_1084) {
    if (tmp_3_2009) {
      struct tuple_bool_uint_64 charge_state_gas_result_2_1085 = charge_state_gas(g3, G_amsterdam_state_new_account);
      uint64_t state_gas = charge_state_gas_result_2_1085.tup1;
      g4 = state_gas;
      if (!charge_state_gas_result_2_1085.tup0) {
        return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = state_gas});
      }
    }
    base_child = call_gas_cap_word(g4, pop_result_2_1067.tup0);
    struct tuple_bool_uint_64 charge_result_2_1086 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g4, base_child);
    uint64_t child_charged_gas = charge_result_2_1086.tup1;
    g4 = child_charged_gas;
    if (!charge_result_2_1086.tup0) {
      return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = child_charged_gas});
    }
  } else {
    base_child = call_gas_cap_word(g4, pop_result_2_1067.tup0);
    struct tuple_bool_uint_64 charge_result_2_1087 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g4, base_child);
    uint64_t child_charged_gas_3_2091 = charge_result_2_1087.tup1;
    g4 = child_charged_gas_3_2091;
    if (!charge_result_2_1087.tup0) {
      return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top7, .tup2 = mem, .tup3 = child_charged_gas_3_2091});
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
  struct MemoryAccessFields args_access = memory_access(args_off_word, args_len_word);
  struct MemoryAccessFields ret_access = memory_access(ret_off_word, ret_len_word);
  uint32_t materialized_required_size = args_access.required_size < ret_access.required_size ? ret_access.required_size : args_access.required_size;
  Bytes mem1 = expand_memory(mem, materialized_required_size);
  struct MemoryRangeFields args = args_access.range;
  struct MemoryRangeFields ret = ret_access.range;
  uint64_t child_gas = conserved_gas_add(base_child, stipend);
  k_aload_(target);
  struct tuple_uint_32_bits_64_Bytes_uint_64 tmp_3_2038;
  bool insufficient_balance;
  if (semantics.takes_value && value_nonzero) {
    u256 caller_balance = k_get_balance(caller);
    bool transfer_affordable = word_ule(value, caller_balance);
    insufficient_balance = (bool)(!transfer_affordable);
  } else {
    insufficient_balance = false;
  }
  bool condition_2_1091 = (bool)(insufficient_balance || (current_depth == UINT16_C(1024)));
  if (condition_2_1091) {
    returndata_clear();
    uint64_t g5 = refund_gas(g4, child_gas);
    uint64_t g6;
    if (tmp_3_2009) {
      g6 = credit_state_gas_refund(g5, G_amsterdam_state_new_account);
    } else {
      g6 = g5;
    }
    uint64_t push_word_result_2_1092 = push_word(top7, WORD_ZERO);
    tmp_3_2038 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1092, .tup2 = mem1, .tup3 = g6});
  } else {
    enum PrecompileId selected_precompile = precompile_id_for_address(target);
    bool neq_anything_result_2_1093 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
    if (neq_anything_result_2_1093) {
      struct tuple_Bytes_Bytes result_2_1096 = active_memory_slice(mem1, args.off, args.len);
      struct tuple_uint_32_bits_64_Bytes_uint_64 tmp_3_2066;
      struct CalldataSlice input = MemoryCalldata(result_2_1096.tup0);
      struct GasCharge precompile_charge = precompile_gas(selected_precompile, input, child_gas);
      if (precompile_charge.affordable) {
        struct PrecompileResult result = run_precompile_slice(selected_precompile, input);
        if (result.success) {
          returndata = result.output;
          if (semantics.transfers_value && value_nonzero) {
            k_transfer(caller, target, value);
          }
          returndata_copy_prefix(ret.off, ret.len);
          uint64_t unused = gas_sub(child_gas, precompile_charge.cost);
          uint64_t g5_3_2093 = refund_gas(g4, unused);
          uint64_t push_word_result_2_1103 = push_word(top7, WORD_ONE);
          tmp_3_2066 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1103, .tup2 = result_2_1096.tup1, .tup3 = g5_3_2093});
        } else {
          returndata_clear();
          uint64_t g5_3_2094;
          if (tmp_3_2009) {
            g5_3_2094 = credit_state_gas_refund(g4, G_amsterdam_state_new_account);
          } else {
            g5_3_2094 = g4;
          }
          uint64_t push_word_result_2_1104 = push_word(top7, WORD_ZERO);
          tmp_3_2066 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1104, .tup2 = result_2_1096.tup1, .tup3 = g5_3_2094});
        }
      } else {
        returndata_clear();
        uint64_t g5_3_2095;
        if (tmp_3_2009) {
          g5_3_2095 = credit_state_gas_refund(g4, G_amsterdam_state_new_account);
        } else {
          g5_3_2095 = g4;
        }
        uint64_t push_word_result_2_1105 = push_word(top7, WORD_ZERO);
        tmp_3_2066 = ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = push_word_result_2_1105, .tup2 = result_2_1096.tup1, .tup3 = g5_3_2095});
      }
      tmp_3_2038 = tmp_3_2066;
    } else {
      uint16_t child_depth = ((uint16_t)((uint32_t)current_depth + (uint32_t)UINT16_C(1)));
      struct CodeFields child_code = executable_code(target, tg_deleg, tg_target);
      bytes20 child_addr = semantics.uses_target_address ? target : caller;
      bytes20 child_caller = semantics.inherits_caller_and_value ? message.caller : caller;
      u256 child_value = semantics.inherits_caller_and_value ? message.value : value;
      bool child_static = (bool)(semantics.enters_static_context || message.is_static);
      struct tuple_Bytes_Bytes result_2_1108 = active_memory_slice(mem1, args.off, args.len);
      Bytes child_memory = evm_memory_slice(result_2_1108.tup0.bytes, result_2_1108.tup0.len);
      struct CalldataSlice child_calldata = MemoryCalldata(child_memory);
      pc = pc_in;
      gas_remaining = g4;
      stack_top = top7;
      evm_memory = result_2_1108.tup1;
      struct FrameCheckpoint suspend_frame_result_2_1113 = suspend_frame();
      struct FrameCheckpoint tmp_3_2047 = suspend_frame_result_2_1113;
      tmp_3_2047.state_gas_remaining = GAS_ZERO;
      struct FrameContinuation continuation = ResumeCall(((struct CallContinuation){.checkpoint = tmp_3_2047, .new_account_charged = tmp_3_2009, .return_length = ret.len, .return_offset = ret.off}));
      frame_stack_push(continuation);
      if (semantics.transfers_value && value_nonzero) {
        k_transfer(caller, target, value);
      }
      message = ((struct Message){.address = child_addr, .caller = child_caller, .code_address = target, .depth = child_depth, .is_static = child_static, .state_gas_reservoir = state_gas_remaining, .value = child_value});
      calldata_install(child_calldata);
      pc = UINT32_C(0);
      gas_remaining = child_gas;
      state_gas_spilled = STATE_GAS_SPILL_ZERO;
      frame_status = Running(UNIT);
      returndata_clear();
      frame_code = child_code;
      call_depth = child_depth;
      frame_refund = GAS_REFUND_ZERO;
      struct tuple_uint_8_bits_64_Bytes_uint_64 tmp_3_2064 = ((struct tuple_uint_8_bits_64_Bytes_uint_64){.tup0 = UINT8_C(0), .tup1 = stack_top, .tup2 = evm_memory, .tup3 = child_gas});
      /* conversions */
      tmp_3_2038.tup0 = (uint32_t)tmp_3_2064.tup0;
      tmp_3_2038.tup1 = tmp_3_2064.tup1;
      tmp_3_2038.tup2 = tmp_3_2064.tup2;
      tmp_3_2038.tup3 = tmp_3_2064.tup3;
      /* end conversions */
    }
  }
  return tmp_3_2038;
}

Bytes interpret_(void)
{
  return interpret();
}

bool frame_succeeded(void)
{
  if (frame_status.kind != Kind_Halted) {
    goto case_1664;
  }
  if (frame_status.variants.Halted.kind != Kind_HaltRevert) {
    goto case_1664;
  }
  return false;
case_1664: ;
  switch (frame_status.kind) {
  case Kind_Halted:
  case Kind_Running:
    return true;
  case Kind_Exceptional:
    return false;
  }
}

void resume_call(struct CallContinuation continuation, Bytes output)
{
  returndata = output;
  bool succeeded = frame_succeeded();
  uint64_t child_left = gas_remaining;
  uint64_t child_state_left = state_gas_remaining;
  uint32_t child_state_spill = state_gas_spilled;
  __int128 child_refund = frame_refund;
  restore_frame(continuation.checkpoint);
  gas_remaining = refund_gas(gas_remaining, child_left);
  return_child_state_gas(child_state_left, child_state_spill);
  returndata_copy_prefix(continuation.return_offset, continuation.return_length);
  if (succeeded) {
    record_refund(child_refund);
    k_journal_commit();
    stack_top = push_word(stack_top, WORD_ONE);
    return;
  }
  k_journal_revert();
  if (continuation.new_account_charged) {
    gas_remaining = credit_state_gas_refund(gas_remaining, G_amsterdam_state_new_account);
  }
  stack_top = push_word(stack_top, WORD_ZERO);
}

void resume_create(struct CreateContinuation continuation, Bytes output)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  returndata = output;
  bool initcode_succeeded = frame_succeeded();
  uint32_t deployed_length = returndata_size();
  if (initcode_succeeded) {
    bool deployed_size_allowed = deployed_code_size_allowed(deployed_length);
    bool prohibited_prefix;
    if (deployed_length != UINT8_C(0)) {
      uint64_t first_byte = output_byte(returndata, UINT8_C(0));
      prohibited_prefix = (bool)(first_byte == UINT64_C(0xEF));
    } else {
      prohibited_prefix = false;
    }
    bool tmp_3_1258;
    if (deployed_size_allowed) {
      bool result_2_961 = (bool)(execution_profile.protocol.fork >= London);
      tmp_3_1258 = (bool)(result_2_961 && prohibited_prefix);
    } else {
      tmp_3_1258 = true;
    }
    if (tmp_3_1258) {
      gas_remaining = exc_halt(gas_remaining, OutOfGas);
    } else {
      struct GasCharge deployment_charge = code_deployment_execution_cost(deployed_length, gas_remaining);
      if (deployment_charge.affordable) {
        gas_remaining = gas_sub(gas_remaining, deployment_charge.cost);
        uint64_t state_deposit = code_deployment_state_cost(deployed_length);
        struct tuple_bool_uint_64 charge_deployment_state_gas_result_2_964 = charge_deployment_state_gas(gas_remaining, state_deposit);
        gas_remaining = charge_deployment_state_gas_result_2_964.tup1;
      } else {
        gas_remaining = exc_halt(gas_remaining, OutOfGas);
      }
    }
  }
  bool tmp_3_1262;
  if (initcode_succeeded) {
    tmp_3_1262 = frame_succeeded();
  } else {
    tmp_3_1262 = false;
  }
  uint64_t child_left = gas_remaining;
  uint64_t child_state_left = state_gas_remaining;
  uint32_t child_state_spill = state_gas_spilled;
  __int128 child_refund = frame_refund;
  restore_frame(continuation.checkpoint);
  gas_remaining = refund_gas(gas_remaining, child_left);
  return_child_state_gas(child_state_left, child_state_spill);
  if (tmp_3_1262) {
    record_refund(child_refund);
    Bytes deployed_bytes = returndata;
    Bytes deployed_code = code_db_intern_output(deployed_bytes);
    k_deploy_code(continuation.address, deployed_code);
    k_journal_commit();
    u256 deployed_address = address_to_word(continuation.address);
    stack_top = push_word(stack_top, deployed_address);
  } else {
    k_journal_revert();
    if (continuation.new_account_charged) {
      gas_remaining = credit_state_gas_refund(gas_remaining, G_amsterdam_state_new_account);
    }
    stack_top = push_word(stack_top, WORD_ZERO);
  }
  if (initcode_succeeded) {
    returndata_clear();
    return;
  }
}

void resume_frame(struct FrameContinuation continuation, Bytes output)
{
  switch (continuation.kind) {
  case Kind_Empty:
    fatal_error(ExecutionInvalid);
  case Kind_ResumeCall:
    resume_call(continuation.variants.ResumeCall, output);
    return;
  case Kind_ResumeCreate:
    resume_create(continuation.variants.ResumeCreate, output);
    return;
  }
}

