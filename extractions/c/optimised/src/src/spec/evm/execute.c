#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bool opcode_failed(struct OpcodeOutcome result)
{
  switch (result.kind) {
  case Kind_Continue:
    return false;
  case Kind_Failed:
    return true;
  }
}

__attribute__((__always_inline__)) struct FrameStatus opcode_frame_status(struct OpcodeOutcome result)
{
  switch (result.kind) {
  case Kind_Continue:
    return Running(UNIT);
  case Kind_Failed:
    return Exceptional(result.variants.Failed);
  }
}

__attribute__((__always_inline__)) struct OpcodeOutcome guard_static(uint64_t *restrict g, bool is_static)
{
  if (is_static) {
    struct OpcodeOutcome Failed_result_2_1785 = Failed(WriteProtection);
    (*g) = (uint64_t)GAS_ZERO;
    return Failed_result_2_1785;
  } else {
    struct OpcodeOutcome Continue_result_2_1786 = Continue(UNIT);
    return Continue_result_2_1786;
  }
}

__attribute__((__always_inline__)) struct OpcodeOutcome do_jump(uint32_t *restrict pc_in, uint64_t *restrict g, struct CodeFields frame_code, u256 destination_value)
{
  uint32_t code_length = frame_code_len(frame_code);
  if (u256_lt_u64(destination_value, code_length)) {
    uint32_t destination = (uint32_t)u256_to_u64_unchecked(destination_value);
    bool valid_destination = frame_jumpdest_valid(frame_code, destination);
    if (valid_destination) {
      struct OpcodeOutcome Continue_result_2_1782 = Continue(UNIT);
      (*pc_in) = destination;
      return Continue_result_2_1782;
    } else {
      struct OpcodeOutcome Failed_result_2_1783 = Failed(InvalidJump);
      (*g) = (uint64_t)GAS_ZERO;
      return Failed_result_2_1783;
    }
  } else {
    struct OpcodeOutcome Failed_result_2_1784 = Failed(InvalidJump);
    (*g) = (uint64_t)GAS_ZERO;
    return Failed_result_2_1784;
  }
}

struct OpcodeOutcome guard_stack(StackPointer carried_sp, uint16_t inputs, uint16_t outputs)
{
  enum StackValidation validate_stack_result_2_1780;
  if (carried_sp.height < inputs) {
    validate_stack_result_2_1780 = StackUnderflowFailure;
  } else {
    bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
    validate_stack_result_2_1780 = result_2_2037 ? StackOverflowFailure : StackValid;
  }
  switch (validate_stack_result_2_1780) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

struct tuple_LogTopics_StackPointer pop_log_topics(uint8_t count, StackPointer sp_in)
{
  StackPointer sp = sp_in;
  if (count == UINT8_C(0)) {
    struct LogTopics LogTopics0_result_2_1774 = LogTopics0(UNIT);
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics0_result_2_1774, .tup1 = sp});
  }
  if (count == UINT8_C(1)) {
    u256 t0 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics1_result_2_1775 = LogTopics1(t0);
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics1_result_2_1775, .tup1 = sp});
  }
  if (count == UINT8_C(2)) {
    u256 t0_3_3112 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics2_result_2_1776 = LogTopics2(((struct tuple_u256_u256){.tup0 = t0_3_3112, .tup1 = t1}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics2_result_2_1776, .tup1 = sp});
  }
  if (count == UINT8_C(3)) {
    u256 t0_3_3114 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1_3_3115 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t2 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics3_result_2_1777 = LogTopics3(((struct tuple_u256_u256_u256){.tup0 = t0_3_3114, .tup1 = t1_3_3115, .tup2 = t2}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics3_result_2_1777, .tup1 = sp});
  }
  if (count == UINT8_C(4)) {
    u256 t0_3_3117 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1_3_3118 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t2_3_3119 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t3 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics4_result_2_1778 = LogTopics4(((struct tuple_u256_u256_u256_u256){.tup0 = t0_3_3117, .tup1 = t1_3_3118, .tup2 = t2_3_3119, .tup3 = t3}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics4_result_2_1778, .tup1 = sp});
  }
  struct LogTopics LogTopics0_result_2_1779 = LogTopics0(UNIT);
  return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics0_result_2_1779, .tup1 = sp});
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_add(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1475;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1769 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1769) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1475 = stack_status;
    goto end_function_2743;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1770 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1770.tup1;
  if (charge_result_2_1770.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1771 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1771;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_add(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1773 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1475 = Continue_result_2_1773;
end_function_2743: ;
  return opcodeoutcome_8_1475;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mul(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1476;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1764 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1764) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1476 = stack_status;
    goto end_function_2735;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1765 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1765.tup1;
  if (charge_result_2_1765.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1766 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1766;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_mul(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1768 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1476 = Continue_result_2_1768;
end_function_2735: ;
  return opcodeoutcome_8_1476;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sub(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1477;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1759 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1759) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1477 = stack_status;
    goto end_function_2727;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1760 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1760.tup1;
  if (charge_result_2_1760.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1761 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1761;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_sub(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1763 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1477 = Continue_result_2_1763;
end_function_2727: ;
  return opcodeoutcome_8_1477;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_div(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1478;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1754 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1754) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1478 = stack_status;
    goto end_function_2719;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1755 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1755.tup1;
  if (charge_result_2_1755.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1756 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1756;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_div(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1758 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1478 = Continue_result_2_1758;
end_function_2719: ;
  return opcodeoutcome_8_1478;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sdiv(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1479;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1749 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1749) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1479 = stack_status;
    goto end_function_2711;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1750 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1750.tup1;
  if (charge_result_2_1750.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1751 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1751;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_sdiv(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1753 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1479 = Continue_result_2_1753;
end_function_2711: ;
  return opcodeoutcome_8_1479;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1480;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1744 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1744) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1480 = stack_status;
    goto end_function_2703;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1745 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1745.tup1;
  if (charge_result_2_1745.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1746 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1746;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_mod(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1748 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1480 = Continue_result_2_1748;
end_function_2703: ;
  return opcodeoutcome_8_1480;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_smod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1481;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1739 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1739) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1481 = stack_status;
    goto end_function_2695;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1740 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1740.tup1;
  if (charge_result_2_1740.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1741 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1741;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_smod(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1743 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1481 = Continue_result_2_1743;
end_function_2695: ;
  return opcodeoutcome_8_1481;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_addmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1482;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  bool opcode_failed_result_2_1734 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1734) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1482 = stack_status;
    goto end_function_2687;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1735 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_mid);
  uint64_t gas_after_charge = charge_result_2_1735.tup1;
  if (charge_result_2_1735.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1736 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1736;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 n = read_stack_word(sp_after);
  u256 result = alu_addmod(a, b, n);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1738 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1482 = Continue_result_2_1738;
end_function_2687: ;
  return opcodeoutcome_8_1482;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mulmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1483;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  bool opcode_failed_result_2_1729 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1729) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1483 = stack_status;
    goto end_function_2679;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1730 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_mid);
  uint64_t gas_after_charge = charge_result_2_1730.tup1;
  if (charge_result_2_1730.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1731 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1731;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 n = read_stack_word(sp_after);
  u256 result = alu_mulmod(a, b, n);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1733 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1483 = Continue_result_2_1733;
end_function_2679: ;
  return opcodeoutcome_8_1483;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1484;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1724 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1724) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1484 = stack_status;
    goto end_function_2671;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 e = read_stack_word(sp_after);
  uint64_t gas_cost = exp_gas(e);
  struct tuple_bool_uint_64 charge_result_2_1725 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(gas_after, gas_cost);
  uint64_t gas_after_charge = charge_result_2_1725.tup1;
  if (charge_result_2_1725.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1726 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1726;
  }
  u256 result = alu_exp(a, e);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1728 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1484 = Continue_result_2_1728;
end_function_2671: ;
  return opcodeoutcome_8_1484;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_signextend(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1485;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1719 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1719) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1485 = stack_status;
    goto end_function_2663;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1720 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1720.tup1;
  if (charge_result_2_1720.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1721 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1721;
  }
  u256 bi = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  u256 result = alu_signextend(bi, v);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1723 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1485 = Continue_result_2_1723;
end_function_2663: ;
  return opcodeoutcome_8_1485;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_lt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1486;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1714 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1714) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1486 = stack_status;
    goto end_function_2655;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1715 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1715.tup1;
  if (charge_result_2_1715.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1716 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1716;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_lt(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1718 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1486 = Continue_result_2_1718;
end_function_2655: ;
  return opcodeoutcome_8_1486;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1487;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1709 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1709) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1487 = stack_status;
    goto end_function_2647;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1710 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1710.tup1;
  if (charge_result_2_1710.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1711 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1711;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_gt(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1713 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1487 = Continue_result_2_1713;
end_function_2647: ;
  return opcodeoutcome_8_1487;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1488;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1704 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1704) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1488 = stack_status;
    goto end_function_2639;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1705 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1705.tup1;
  if (charge_result_2_1705.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1706 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1706;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_slt(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1708 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1488 = Continue_result_2_1708;
end_function_2639: ;
  return opcodeoutcome_8_1488;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sgt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1489;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1699 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1699) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1489 = stack_status;
    goto end_function_2631;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1700 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1700.tup1;
  if (charge_result_2_1700.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1701 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1701;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_sgt(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1703 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1489 = Continue_result_2_1703;
end_function_2631: ;
  return opcodeoutcome_8_1489;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_eq(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1490;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1694 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1694) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1490 = stack_status;
    goto end_function_2623;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1695 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1695.tup1;
  if (charge_result_2_1695.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1696 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1696;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_eq(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1698 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1490 = Continue_result_2_1698;
end_function_2623: ;
  return opcodeoutcome_8_1490;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_iszero(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1491;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1689 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1689) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1491 = stack_status;
    goto end_function_2615;
  }
  uint64_t gas_after = (*carried_gas);
  struct tuple_bool_uint_64 charge_result_2_1690 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1690.tup1;
  if (charge_result_2_1690.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1691 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1691;
  }
  u256 a = read_stack_word((*carried_sp));
  u256 result = alu_iszero(a);
  write_stack_word((*carried_sp), result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1693 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1491 = Continue_result_2_1693;
end_function_2615: ;
  return opcodeoutcome_8_1491;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_and(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1492;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1684 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1684) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1492 = stack_status;
    goto end_function_2607;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1685 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1685.tup1;
  if (charge_result_2_1685.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1686 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1686;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_and(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1688 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1492 = Continue_result_2_1688;
end_function_2607: ;
  return opcodeoutcome_8_1492;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_or(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1493;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1679 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1679) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1493 = stack_status;
    goto end_function_2599;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1680 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1680.tup1;
  if (charge_result_2_1680.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1681 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1681;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_or(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1683 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1493 = Continue_result_2_1683;
end_function_2599: ;
  return opcodeoutcome_8_1493;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_xor(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1494;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1674 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1674) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1494 = stack_status;
    goto end_function_2591;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1675 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1675.tup1;
  if (charge_result_2_1675.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1676 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1676;
  }
  u256 a = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 b = read_stack_word(sp_after);
  u256 result = alu_xor(a, b);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1678 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1494 = Continue_result_2_1678;
end_function_2591: ;
  return opcodeoutcome_8_1494;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_not(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1495;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1669 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1669) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1495 = stack_status;
    goto end_function_2583;
  }
  uint64_t gas_after = (*carried_gas);
  struct tuple_bool_uint_64 charge_result_2_1670 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1670.tup1;
  if (charge_result_2_1670.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1671 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1671;
  }
  u256 a = read_stack_word((*carried_sp));
  u256 result = alu_not(a);
  write_stack_word((*carried_sp), result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1673 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1495 = Continue_result_2_1673;
end_function_2583: ;
  return opcodeoutcome_8_1495;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_byte(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1496;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1664 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1664) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1496 = stack_status;
    goto end_function_2575;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1665 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1665.tup1;
  if (charge_result_2_1665.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1666 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1666;
  }
  u256 i = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 x = read_stack_word(sp_after);
  u256 result = alu_byte(i, x);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1668 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1496 = Continue_result_2_1668;
end_function_2575: ;
  return opcodeoutcome_8_1496;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shl(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1497;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1659 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1659) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1497 = stack_status;
    goto end_function_2567;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1660 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1660.tup1;
  if (charge_result_2_1660.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1661 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1661;
  }
  u256 s = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  u256 result = alu_shl(s, v);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1663 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1497 = Continue_result_2_1663;
end_function_2567: ;
  return opcodeoutcome_8_1497;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shr(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1498;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1654 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1654) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1498 = stack_status;
    goto end_function_2559;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1655 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1655.tup1;
  if (charge_result_2_1655.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1656 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1656;
  }
  u256 s = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  u256 result = alu_shr(s, v);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1658 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1498 = Continue_result_2_1658;
end_function_2559: ;
  return opcodeoutcome_8_1498;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sar(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1499;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1649 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1649) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1499 = stack_status;
    goto end_function_2551;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1650 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1650.tup1;
  if (charge_result_2_1650.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1651 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1651;
  }
  u256 s = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  u256 result = alu_sar(s, v);
  write_stack_word(sp_after, result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1653 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1499 = Continue_result_2_1653;
end_function_2551: ;
  return opcodeoutcome_8_1499;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_clz(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1500;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1644 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1644) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1500 = stack_status;
    goto end_function_2543;
  }
  uint64_t gas_after = (*carried_gas);
  struct tuple_bool_uint_64 charge_result_2_1645 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1645.tup1;
  if (charge_result_2_1645.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1646 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1646;
  }
  u256 x = read_stack_word((*carried_sp));
  u256 result = alu_clz(x);
  write_stack_word((*carried_sp), result);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1648 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1500 = Continue_result_2_1648;
end_function_2543: ;
  return opcodeoutcome_8_1500;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_keccak256(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1501;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  bool opcode_failed_result_2_1633 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1633) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1501 = stack_status;
    goto end_function_2533;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_bool_uint_64 charge_keccak_gas_result_2_1634 = charge_keccak_gas(gas_after, length_word);
  uint64_t gas_after_hashing = charge_keccak_gas_result_2_1634.tup1;
  if (charge_keccak_gas_result_2_1634.tup0) {
    gas_after = gas_after_hashing;
    struct OpcodeOutcome Failed_result_2_1635 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1635;
  }
  uint64_t required_size = memory_required_size(offset_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1637 = charge_memory_expansion(gas_after_hashing, memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1637.tup1;
  if (charge_memory_expansion_result_2_1637.tup0) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1638 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1638;
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  struct tuple_u256_Bytes result_2_1641 = mem_keccak(mem1, access.range);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, result_2_1641.tup0);
  memory_after = result_2_1641.tup1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1642 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1501 = Continue_result_2_1642;
end_function_2533: ;
  return opcodeoutcome_8_1501;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_address(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1502;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1628 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1628) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1502 = stack_status;
    goto end_function_2521;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1629 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1629.tup1;
  if (charge_result_2_1629.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1630 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1630;
  }
  u256 address_word = address_to_word(carried_address);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, address_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1632 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1502 = Continue_result_2_1632;
end_function_2521: ;
  return opcodeoutcome_8_1502;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_origin(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1503;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1623 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1623) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1503 = stack_status;
    goto end_function_2513;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1624 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1624.tup1;
  if (charge_result_2_1624.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1625 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1625;
  }
  u256 origin = k_env(F_Origin);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, origin);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1627 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1503 = Continue_result_2_1627;
end_function_2513: ;
  return opcodeoutcome_8_1503;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_caller(bytes20 carried_caller, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1504;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1618 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1618) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1504 = stack_status;
    goto end_function_2505;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1619 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1619.tup1;
  if (charge_result_2_1619.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1620 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1620;
  }
  u256 caller = address_to_word(carried_caller);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, caller);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1622 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1504 = Continue_result_2_1622;
end_function_2505: ;
  return opcodeoutcome_8_1504;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_callvalue(u256 carried_value, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1505;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1613 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1613) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1505 = stack_status;
    goto end_function_2497;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1614 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1614.tup1;
  if (charge_result_2_1614.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1615 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1615;
  }
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, carried_value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1617 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1505 = Continue_result_2_1617;
end_function_2497: ;
  return opcodeoutcome_8_1505;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gasprice(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1506;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1608 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1608) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1506 = stack_status;
    goto end_function_2489;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1609 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1609.tup1;
  if (charge_result_2_1609.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1610 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1610;
  }
  u256 gas_price = k_env(F_GasPrice);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, gas_price);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1612 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1506 = Continue_result_2_1612;
end_function_2489: ;
  return opcodeoutcome_8_1506;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatasize(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1507;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1603 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1603) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1507 = stack_status;
    goto end_function_2481;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1604 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1604.tup1;
  if (charge_result_2_1604.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1605 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1605;
  }
  uint32_t input_length = calldata_slice_length(carried_calldata);
  u256 length_word = word_of_source_byte_count(input_length);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, length_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1607 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1507 = Continue_result_2_1607;
end_function_2481: ;
  return opcodeoutcome_8_1507;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldataload(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1508;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1598 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1598) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1508 = stack_status;
    goto end_function_2473;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1599 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1599.tup1;
  if (charge_result_2_1599.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1600 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1600;
  }
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 value = calldata_slice_load_word_offset(carried_calldata, offset_word);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1602 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1508 = Continue_result_2_1602;
end_function_2473: ;
  return opcodeoutcome_8_1508;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatacopy(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1509;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  bool opcode_failed_result_2_1584 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1584) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1509 = stack_status;
    goto end_function_2461;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1585 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1585.tup1;
  if (charge_result_2_1585.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1586 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1586;
  }
  u256 destination_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 source_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1588 = charge_copy_gas(gas_after_base, length_word);
  uint64_t gas_after_copy = charge_copy_gas_result_2_1588.tup1;
  if (charge_copy_gas_result_2_1588.tup0) {
    gas_after = gas_after_copy;
    struct OpcodeOutcome Failed_result_2_1589 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1589;
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1591 = charge_memory_expansion(gas_after_copy, memory_after, required_size);
  bool expansion_halt = charge_memory_expansion_result_2_1591.tup0;
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1591.tup1;
  if (expansion_halt) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1592 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1592;
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  calldata_slice_copy_word_offset(carried_calldata, access.range.off, source_word, access.range.len);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1596 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1509 = Continue_result_2_1596;
end_function_2461: ;
  return opcodeoutcome_8_1509;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codesize(struct CodeFields carried_code, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1510;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1579 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1579) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1510 = stack_status;
    goto end_function_2449;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1580 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1580.tup1;
  if (charge_result_2_1580.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1581 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1581;
  }
  uint32_t code_length = frame_code_len(carried_code);
  u256 length_word = word_of_source_byte_count(code_length);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, length_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1583 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1510 = Continue_result_2_1583;
end_function_2449: ;
  return opcodeoutcome_8_1510;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codecopy(struct CodeFields carried_code, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1511;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  bool opcode_failed_result_2_1565 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1565) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1511 = stack_status;
    goto end_function_2437;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1566 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1566.tup1;
  if (charge_result_2_1566.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1567 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1567;
  }
  u256 destination_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 source_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1569 = charge_copy_gas(gas_after_base, length_word);
  uint64_t gas_after_copy = charge_copy_gas_result_2_1569.tup1;
  if (charge_copy_gas_result_2_1569.tup0) {
    gas_after = gas_after_copy;
    struct OpcodeOutcome Failed_result_2_1570 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1570;
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1572 = charge_memory_expansion(gas_after_copy, memory_after, required_size);
  bool expansion_halt = charge_memory_expansion_result_2_1572.tup0;
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1572.tup1;
  if (expansion_halt) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1573 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1573;
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  Bytes bytes = code_bytes(carried_code);
  code_slice_copy_word_offset(bytes, access.range.off, source_word, access.range.len);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1577 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1511 = Continue_result_2_1577;
end_function_2437: ;
  return opcodeoutcome_8_1511;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_balance(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1512;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1560 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1560) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1512 = stack_status;
    goto end_function_2425;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  u256 address_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  struct tuple_bool_uint_64 charge_result_2_1561 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, gas_cost);
  uint64_t gas_after_charge = charge_result_2_1561.tup1;
  if (charge_result_2_1561.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1562 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1562;
  }
  k_account_mark_warm(a);
  u256 balance = k_get_balance(a);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, balance);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1564 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1512 = Continue_result_2_1564;
end_function_2425: ;
  return opcodeoutcome_8_1512;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_selfbalance(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1513;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1555 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1555) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1513 = stack_status;
    goto end_function_2417;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1556 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_low);
  uint64_t gas_after_charge = charge_result_2_1556.tup1;
  if (charge_result_2_1556.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1557 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1557;
  }
  u256 balance = k_get_balance(carried_address);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, balance);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1559 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1513 = Continue_result_2_1559;
end_function_2417: ;
  return opcodeoutcome_8_1513;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodesize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1514;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1549 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1549) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1514 = stack_status;
    goto end_function_2409;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  u256 address_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  struct tuple_bool_uint_64 result_2_1551 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(gas_after, ((uint32_t)access_cost + (uint32_t)read_cost));
  uint64_t gas_after_charge = result_2_1551.tup1;
  if (result_2_1551.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1552 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1552;
  }
  k_account_mark_warm(a);
  uint32_t code_size = k_get_code_size(a);
  u256 size_word = word_of_source_byte_count(code_size);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, size_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1554 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1514 = Continue_result_2_1554;
end_function_2409: ;
  return opcodeoutcome_8_1514;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodecopy(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1515;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(4), UINT8_C(0));
  bool opcode_failed_result_2_1534 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1534) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1515 = stack_status;
    goto end_function_2397;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  u256 address_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  u256 destination_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 source_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  struct tuple_bool_uint_64 result_2_1536 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(gas_after, ((uint32_t)access_cost + (uint32_t)read_cost));
  uint64_t gas_after_access = result_2_1536.tup1;
  if (result_2_1536.tup0) {
    gas_after = gas_after_access;
    struct OpcodeOutcome Failed_result_2_1537 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1537;
  }
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1539 = charge_copy_gas(gas_after_access, length_word);
  uint64_t gas_after_copy = charge_copy_gas_result_2_1539.tup1;
  if (charge_copy_gas_result_2_1539.tup0) {
    gas_after = gas_after_copy;
    struct OpcodeOutcome Failed_result_2_1540 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1540;
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1542 = charge_memory_expansion(gas_after_copy, memory_after, required_size);
  bool expansion_halt = charge_memory_expansion_result_2_1542.tup0;
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1542.tup1;
  if (expansion_halt) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1543 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1543;
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  k_account_mark_warm(a);
  k_code_copy(a, access.range.off, source_word, access.range.len);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1547 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1515 = Continue_result_2_1547;
end_function_2397: ;
  return opcodeoutcome_8_1515;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodehash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1516;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1529 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1529) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1516 = stack_status;
    goto end_function_2385;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  u256 address_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  struct tuple_bool_uint_64 charge_result_2_1530 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, gas_cost);
  uint64_t gas_after_charge = charge_result_2_1530.tup1;
  if (charge_result_2_1530.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1531 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1531;
  }
  k_account_mark_warm(a);
  bytes32 code_hash = k_get_codehash(a);
  u256 hash_word = hash_to_word(code_hash);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, hash_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1533 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1516 = Continue_result_2_1533;
end_function_2385: ;
  return opcodeoutcome_8_1516;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatasize(Bytes carried_returndata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1517;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1524 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1524) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1517 = stack_status;
    goto end_function_2377;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1525 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1525.tup1;
  if (charge_result_2_1525.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1526 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1526;
  }
  uint32_t return_data_size = returndata_size(carried_returndata);
  u256 size_word = word_of_source_byte_count(return_data_size);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, size_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1528 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1517 = Continue_result_2_1528;
end_function_2377: ;
  return opcodeoutcome_8_1517;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatacopy(Bytes carried_returndata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1518;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  bool opcode_failed_result_2_1505 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1505) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1518 = stack_status;
    goto end_function_2365;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1506 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1506.tup1;
  if (charge_result_2_1506.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1507 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1507;
  }
  u256 destination_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 source_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint32_t returndata_size_result_2_1509 = returndata_size(carried_returndata);
  struct tuple_uint_64_StackPointer_Bytes_OpcodeOutcome tmp_3_2492;
  if (!u64_lt_u256(returndata_size_result_2_1509, source_word)) {
    uint32_t remaining;
    uint32_t tmp_3_4022 = (uint32_t)u256_to_u64(source_word);
    remaining = returndata_remaining(returndata_size_result_2_1509, tmp_3_4022);
    if (!u64_lt_u256(remaining, length_word)) {
      struct tuple_bool_uint_64 charge_copy_gas_result_2_1512 = charge_copy_gas_uint64_t_u256_to_struct_tuple_bool_uint_64(gas_after_base, length_word);
      uint64_t gas_after_copy = charge_copy_gas_result_2_1512.tup1;
      if (charge_copy_gas_result_2_1512.tup0) {
        gas_after = gas_after_copy;
        struct OpcodeOutcome Failed_result_2_1513 = Failed(OutOfGas);
        (*carried_gas) = gas_after;
        (*carried_sp) = sp_after;
        (*carried_memory) = memory_after;
        return Failed_result_2_1513;
      }
      uint64_t required_size = memory_required_size_u256_u256_to_uint64_t_variant_2(destination_word, length_word);
      struct tuple_bool_uint_64 charge_memory_expansion_result_2_1515 = charge_memory_expansion(gas_after_copy, memory_after, required_size);
      uint64_t gas_after_expansion = charge_memory_expansion_result_2_1515.tup1;
      if (charge_memory_expansion_result_2_1515.tup0) {
        gas_after = gas_after_expansion;
        struct OpcodeOutcome Failed_result_2_1516 = Failed(OutOfGas);
        (*carried_gas) = gas_after;
        (*carried_sp) = sp_after;
        (*carried_memory) = memory_after;
        return Failed_result_2_1516;
      }
      struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields_variant_2(destination_word, length_word);
      Bytes mem1 = expand_memory(memory_after, access.required_size);
      returndata_copy(carried_returndata, access.range.off, (uint32_t)u256_to_u64_unchecked(source_word), (uint32_t)u256_to_u64_unchecked(length_word));
      memory_after = mem1;
      gas_after = gas_after_expansion;
      struct OpcodeOutcome Continue_result_2_1520 = Continue(UNIT);
      tmp_3_2492 = ((struct tuple_uint_64_StackPointer_Bytes_OpcodeOutcome){.tup0 = gas_after, .tup1 = sp_after, .tup2 = memory_after, .tup3 = Continue_result_2_1520});
    } else {
      struct OpcodeOutcome Failed_result_2_1522 = Failed(InvalidOpcode);
      struct tuple_uint_8_StackPointer_Bytes_OpcodeOutcome tmp_3_2495 = ((struct tuple_uint_8_StackPointer_Bytes_OpcodeOutcome){.tup0 = GAS_ZERO, .tup1 = sp_after, .tup2 = memory_after, .tup3 = Failed_result_2_1522});
      /* conversions */
      tmp_3_2492.tup0 = (uint64_t)tmp_3_2495.tup0;
      tmp_3_2492.tup1 = tmp_3_2495.tup1;
      tmp_3_2492.tup2 = tmp_3_2495.tup2;
      tmp_3_2492.tup3 = tmp_3_2495.tup3;
      /* end conversions */
    }
  } else {
    struct OpcodeOutcome Failed_result_2_1523 = Failed(InvalidOpcode);
    struct tuple_uint_8_StackPointer_Bytes_OpcodeOutcome tmp_3_2494 = ((struct tuple_uint_8_StackPointer_Bytes_OpcodeOutcome){.tup0 = GAS_ZERO, .tup1 = sp_after, .tup2 = memory_after, .tup3 = Failed_result_2_1523});
    /* conversions */
    tmp_3_2492.tup0 = (uint64_t)tmp_3_2494.tup0;
    tmp_3_2492.tup1 = tmp_3_2494.tup1;
    tmp_3_2492.tup2 = tmp_3_2494.tup2;
    tmp_3_2492.tup3 = tmp_3_2494.tup3;
    /* end conversions */
  }
  (*carried_gas) = tmp_3_2492.tup0;
  (*carried_sp) = tmp_3_2492.tup1;
  (*carried_memory) = tmp_3_2492.tup2;
  opcodeoutcome_8_1518 = tmp_3_2492.tup3;
end_function_2365: ;
  return opcodeoutcome_8_1518;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blockhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1519;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1500 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1500) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1519 = stack_status;
    goto end_function_2351;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1501 = charge_uint64_t_uint8_t_to_struct_tuple_bool_uint_64(gas_after, UINT8_C(20));
  uint64_t gas_after_charge = charge_result_2_1501.tup1;
  if (charge_result_2_1501.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1502 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1502;
  }
  u256 block_number = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bytes32 block_hash = k_blockhash(block_number);
  u256 hash_word = hash_to_word(block_hash);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, hash_word);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1504 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1519 = Continue_result_2_1504;
end_function_2351: ;
  return opcodeoutcome_8_1519;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_coinbase(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1520;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1495 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1495) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1520 = stack_status;
    goto end_function_2343;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1496 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1496.tup1;
  if (charge_result_2_1496.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1497 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1497;
  }
  u256 coinbase = k_env(F_Coinbase);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, coinbase);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1499 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1520 = Continue_result_2_1499;
end_function_2343: ;
  return opcodeoutcome_8_1520;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_timestamp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1521;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1490 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1490) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1521 = stack_status;
    goto end_function_2335;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1491 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1491.tup1;
  if (charge_result_2_1491.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1492 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1492;
  }
  u256 timestamp = k_env(F_Timestamp);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, timestamp);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1494 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1521 = Continue_result_2_1494;
end_function_2335: ;
  return opcodeoutcome_8_1521;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_number(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1522;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1485 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1485) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1522 = stack_status;
    goto end_function_2327;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1486 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1486.tup1;
  if (charge_result_2_1486.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1487 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1487;
  }
  u256 block_number = k_env(F_Number);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, block_number);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1489 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1522 = Continue_result_2_1489;
end_function_2327: ;
  return opcodeoutcome_8_1522;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slotnum(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1523;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1480 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1480) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1523 = stack_status;
    goto end_function_2319;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1481 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1481.tup1;
  if (charge_result_2_1481.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1482 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1482;
  }
  u256 slot_number = k_env(F_SlotNumber);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, slot_number);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1484 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1523 = Continue_result_2_1484;
end_function_2319: ;
  return opcodeoutcome_8_1523;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_prevrandao(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1524;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1475 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1475) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1524 = stack_status;
    goto end_function_2311;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1476 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1476.tup1;
  if (charge_result_2_1476.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1477 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1477;
  }
  u256 prev_randao = k_env(F_PrevRandao);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, prev_randao);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1479 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1524 = Continue_result_2_1479;
end_function_2311: ;
  return opcodeoutcome_8_1524;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gaslimit(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1525;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1470 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1470) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1525 = stack_status;
    goto end_function_2303;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1471 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1471.tup1;
  if (charge_result_2_1471.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1472 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1472;
  }
  u256 gas_limit = k_env(F_GasLimit);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, gas_limit);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1474 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1525 = Continue_result_2_1474;
end_function_2303: ;
  return opcodeoutcome_8_1525;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_chainid(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1526;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1465 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1465) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1526 = stack_status;
    goto end_function_2295;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1466 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1466.tup1;
  if (charge_result_2_1466.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1467 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1467;
  }
  u256 chain_id = k_env(F_ChainId);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, chain_id);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1469 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1526 = Continue_result_2_1469;
end_function_2295: ;
  return opcodeoutcome_8_1526;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_basefee(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1527;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1460 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1460) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1527 = stack_status;
    goto end_function_2287;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1461 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1461.tup1;
  if (charge_result_2_1461.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1462 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1462;
  }
  u256 base_fee = k_env(F_BaseFee);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, base_fee);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1464 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1527 = Continue_result_2_1464;
end_function_2287: ;
  return opcodeoutcome_8_1527;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobbasefee(u256 blob_fee, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1528;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1455 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1455) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1528 = stack_status;
    goto end_function_2279;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1456 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1456.tup1;
  if (charge_result_2_1456.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1457 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1457;
  }
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, blob_fee);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1459 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1528 = Continue_result_2_1459;
end_function_2279: ;
  return opcodeoutcome_8_1528;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1529;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1450 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1450) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1529 = stack_status;
    goto end_function_2271;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1451 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1451.tup1;
  if (charge_result_2_1451.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1452 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1452;
  }
  u256 index = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 blob_hash = k_blobhash(index);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, blob_hash);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1454 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1529 = Continue_result_2_1454;
end_function_2271: ;
  return opcodeoutcome_8_1529;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pop(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1530;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  bool opcode_failed_result_2_1444 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1444) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1530 = stack_status;
    goto end_function_2263;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1445 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1445.tup1;
  if (charge_result_2_1445.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1446 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1446;
  }
  read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1449 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1530 = Continue_result_2_1449;
end_function_2263: ;
  return opcodeoutcome_8_1530;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mload(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1531;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1433 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1433) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1531 = stack_status;
    goto end_function_2251;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1434 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1434.tup1;
  if (charge_result_2_1434.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1435 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1435;
  }
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint64_t required_size = memory_required_size_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1437 = charge_memory_expansion(gas_after_base, memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1437.tup1;
  if (charge_memory_expansion_result_2_1437.tup0) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1438 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1438;
  }
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  u256 value = mem_load(access.range.off);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, value);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1440 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1531 = Continue_result_2_1440;
end_function_2251: ;
  return opcodeoutcome_8_1531;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1532;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1422 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1422) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1532 = stack_status;
    goto end_function_2239;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1423 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1423.tup1;
  if (charge_result_2_1423.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1424 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1424;
  }
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint64_t required_size = memory_required_size_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1426 = charge_memory_expansion(gas_after_base, memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1426.tup1;
  if (charge_memory_expansion_result_2_1426.tup0) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1427 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1427;
  }
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  mem_store(access.range.off, v);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1431 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1532 = Continue_result_2_1431;
end_function_2239: ;
  return opcodeoutcome_8_1532;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore8(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1533;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1411 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1411) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1533 = stack_status;
    goto end_function_2227;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1412 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1412.tup1;
  if (charge_result_2_1412.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1413 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1413;
  }
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint64_t required_size = memory_required_size(offset_word, WORD_ONE);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1415 = charge_memory_expansion(gas_after_base, memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1415.tup1;
  if (charge_memory_expansion_result_2_1415.tup0) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1416 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1416;
  }
  struct MemoryAccessFields access = memory_access(offset_word, WORD_ONE);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  mem_store_byte(access.range.off, v);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1420 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1533 = Continue_result_2_1420;
end_function_2227: ;
  return opcodeoutcome_8_1533;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_msize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1534;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1405 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1405) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1534 = stack_status;
    goto end_function_2217;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1406 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1406.tup1;
  if (charge_result_2_1406.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1407 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1407;
  }
  uint32_t high_water = memory_high_water((*carried_memory));
  uint32_t words = memory_word_count_uint32_t_to_uint32_t(high_water);
  u256 size = word_of_nat_byte_count_uint64_t_to_u256(((uint64_t)words * UINT64_C(32)));
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, size);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1409 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1534 = Continue_result_2_1409;
end_function_2217: ;
  return opcodeoutcome_8_1534;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mcopy(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct OpcodeOutcome opcodeoutcome_8_1535;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  bool opcode_failed_result_2_1384 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1384) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1535 = stack_status;
    goto end_function_2205;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  struct tuple_bool_uint_64 charge_result_2_1385 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_base = charge_result_2_1385.tup1;
  if (charge_result_2_1385.tup0) {
    gas_after = gas_after_base;
    struct OpcodeOutcome Failed_result_2_1386 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1386;
  }
  u256 destination_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 source_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1388 = charge_copy_gas(gas_after_base, length_word);
  uint64_t gas_after_copy = charge_copy_gas_result_2_1388.tup1;
  if (charge_copy_gas_result_2_1388.tup0) {
    gas_after = gas_after_copy;
    struct OpcodeOutcome Failed_result_2_1389 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1389;
  }
  uint64_t destination_required = memory_required_size(destination_word, length_word);
  uint64_t source_required = memory_required_size(source_word, length_word);
  uint64_t required_size = destination_required < source_required ? source_required : destination_required;
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1391 = charge_memory_expansion(gas_after_copy, memory_after, required_size);
  bool expansion_halt = charge_memory_expansion_result_2_1391.tup0;
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1391.tup1;
  if (expansion_halt) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1392 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1392;
  }
  struct MemoryAccessFields destination = memory_access(destination_word, length_word);
  struct MemoryAccessFields source = memory_access(source_word, length_word);
  uint32_t materialized_required_size = destination.required_size < source.required_size ? source.required_size : destination.required_size;
  Bytes mem1 = expand_memory(memory_after, materialized_required_size);
  mem_mcopy(destination.range.off, source.range.off, destination.range.len);
  memory_after = mem1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1400 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1535 = Continue_result_2_1400;
end_function_2205: ;
  return opcodeoutcome_8_1535;
}

__attribute__((__always_inline__)) struct AccountExecutionContext account_execution_context_(bytes20 address)
{
  return account_execution_context(address);
}

__attribute__((__always_inline__)) struct AccountExecutionContext refresh_account_execution_context(struct AccountExecutionContext context, bytes20 previous_address, bytes20 next_address)
{
  if (eq_bytes20(previous_address, next_address)) {
    return context;
  }
  return account_execution_context(next_address);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sload(struct AccountExecutionContext context, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1536;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1372 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1372) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1536 = stack_status;
    goto end_function_2191;
  }
  u256 slot = read_stack_word((*carried_sp));
  StackPointer sp_after_pop = stack_top_retreat_((*carried_sp), UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  uint16_t gas_cost = sload_cost(warm);
  struct tuple_bool_uint_64 charge_result_2_1373 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64((*carried_gas), gas_cost);
  struct tuple_uint_64_StackPointer_OpcodeOutcome tmp_3_2236;
  if (charge_result_2_1373.tup0) {
    struct OpcodeOutcome Failed_result_2_1374 = Failed(OutOfGas);
    tmp_3_2236 = ((struct tuple_uint_64_StackPointer_OpcodeOutcome){.tup0 = charge_result_2_1373.tup1, .tup1 = sp_after_pop, .tup2 = Failed_result_2_1374});
  } else {
    struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, (bool)(!warm));
    write_stack_word((*carried_sp), entry.curr);
    struct OpcodeOutcome Continue_result_2_1376 = Continue(UNIT);
    tmp_3_2236 = ((struct tuple_uint_64_StackPointer_OpcodeOutcome){.tup0 = charge_result_2_1373.tup1, .tup1 = (*carried_sp), .tup2 = Continue_result_2_1376});
  }
  (*carried_gas) = tmp_3_2236.tup0;
  (*carried_sp) = tmp_3_2236.tup1;
  opcodeoutcome_8_1536 = tmp_3_2236.tup2;
end_function_2191: ;
  return opcodeoutcome_8_1536;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sstore(struct AccountExecutionContext context, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1537;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1332 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1332) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1537 = stack_status;
    goto end_function_2175;
  }
  uint64_t g_after_8_1563;
  struct OpcodeOutcome guard_result;
  g_after_8_1563 = (*carried_gas);
  guard_result = guard_static(&g_after_8_1563, carried_is_static);
  bool opcode_failed_result_2_1334 = opcode_failed(guard_result);
  if (opcode_failed_result_2_1334) {
    (*carried_gas) = g_after_8_1563;
    return guard_result;
  }
  bool result_2_1336;
  bool lt_int_result_2_1335 = (bool)(fork < Amsterdam);
  result_2_1336 = (bool)(lt_int_result_2_1335 && ((*carried_gas) <= G_callstipend));
  if (result_2_1336) {
    struct OpcodeOutcome Failed_result_2_1337 = Failed(OutOfGas);
    return Failed_result_2_1337;
  }
  u256 slot = read_stack_word((*carried_sp));
  StackPointer sp_after_slot = stack_top_retreat_((*carried_sp), UINT8_C(1));
  u256 value = read_stack_word(sp_after_slot);
  StackPointer sp_after = stack_top_retreat_(sp_after_slot, UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  bool cold = (bool)(!warm);
  bool gteq_int_result_2_1339 = (bool)(fork >= Amsterdam);
  if (gteq_int_result_2_1339) {
    uint64_t sentry_cost = sstore_sentry_cost(cold);
    struct tuple_bool_uint_64 check_execution_gas_result_2_1340 = check_execution_gas_uint64_t_uint64_t_to_struct_tuple_bool_uint_64((*carried_gas), sentry_cost);
    if (check_execution_gas_result_2_1340.tup0) {
      struct OpcodeOutcome Failed_result_2_1341 = Failed(OutOfGas);
      (*carried_sp) = sp_after;
      return Failed_result_2_1341;
    }
  }
  struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, cold);
  struct SstoreCosts costs = sstore_costs(entry.orig, entry.curr, value, cold);
  struct tuple_uint_64_uint_64_uint_32 result_2_1346;
  if (costs.state_credit != UINT8_C(0)) {
    result_2_1346 = credit_state_gas_refund((*carried_gas), (*carried_state_gas), (*carried_state_spill), costs.state_credit);
  } else {
    result_2_1346 = ((struct tuple_uint_64_uint_64_uint_32){.tup0 = (*carried_gas), .tup1 = (*carried_state_gas), .tup2 = (*carried_state_spill)});
  }
  uint64_t state_gas_after_credit = result_2_1346.tup1;
  uint32_t state_spill_after_credit = result_2_1346.tup2;
  struct tuple_bool_uint_64 result_2_1348 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(result_2_1346.tup0, costs.execution);
  bool execution_halt = result_2_1348.tup0;
  uint64_t gas_after_execution = result_2_1348.tup1;
  if (execution_halt) {
    struct OpcodeOutcome Failed_result_2_1349 = Failed(OutOfGas);
    (*carried_gas) = gas_after_execution;
    (*carried_state_gas) = state_gas_after_credit;
    (*carried_state_spill) = state_spill_after_credit;
    (*carried_sp) = sp_after;
    return Failed_result_2_1349;
  }
  struct tuple_bool_uint_64_uint_64_uint_32 result_2_1352 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after_execution, state_gas_after_credit, state_spill_after_credit, costs.state_charge);
  bool state_halt = result_2_1352.tup0;
  uint64_t gas_after_state_charge = result_2_1352.tup1;
  uint64_t state_gas_after_charge = result_2_1352.tup2;
  uint32_t state_spill_after_charge = result_2_1352.tup3;
  if (state_halt) {
    struct OpcodeOutcome Failed_result_2_1353 = Failed(OutOfGas);
    (*carried_gas) = gas_after_state_charge;
    (*carried_state_gas) = state_gas_after_charge;
    (*carried_state_spill) = state_spill_after_charge;
    (*carried_sp) = sp_after;
    return Failed_result_2_1353;
  }
  __int128 refund_after;
  bool result_2_1362 = (bool)(costs.refund == GAS_REFUND_ZERO);
  if (result_2_1362) {
    refund_after = (*carried_refund);
  } else {
    refund_after = record_refund((*carried_refund), costs.refund);
  }
  if (!eq_u256(entry.curr, value)) {
    storage_update_by_id(context.account_id, context.storage_generation, storage_id, value, entry.orig);
  }
  struct OpcodeOutcome Continue_result_2_1360 = Continue(UNIT);
  (*carried_gas) = gas_after_state_charge;
  (*carried_state_gas) = state_gas_after_charge;
  (*carried_state_spill) = state_spill_after_charge;
  (*carried_refund) = refund_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1537 = Continue_result_2_1360;
end_function_2175: ;
  return opcodeoutcome_8_1537;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tload(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1538;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  bool opcode_failed_result_2_1327 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1327) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1538 = stack_status;
    goto end_function_2159;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1328 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_warm_access);
  uint64_t gas_after_charge = charge_result_2_1328.tup1;
  if (charge_result_2_1328.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1329 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1329;
  }
  u256 s = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 value = k_tload(carried_address, s);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1331 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1538 = Continue_result_2_1331;
end_function_2159: ;
  return opcodeoutcome_8_1538;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tstore(bytes20 carried_address, bool carried_is_static, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1539;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1320 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1320) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1539 = stack_status;
    goto end_function_2149;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  uint64_t gas_after_static_guard;
  struct OpcodeOutcome status_after_guard;
  gas_after_static_guard = gas_after;
  status_after_guard = guard_static(&gas_after_static_guard, carried_is_static);
  bool opcode_failed_result_2_1322 = opcode_failed(status_after_guard);
  if (opcode_failed_result_2_1322) {
    gas_after = gas_after_static_guard;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return status_after_guard;
  }
  struct tuple_bool_uint_64 charge_result_2_1323 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after_static_guard, G_warm_access);
  uint64_t gas_after_charge = charge_result_2_1323.tup1;
  if (charge_result_2_1323.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1324 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1324;
  }
  u256 s = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 v = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  k_tstore(carried_address, s, v);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1326 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1539 = Continue_result_2_1326;
end_function_2149: ;
  return opcodeoutcome_8_1539;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jump(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1540;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  bool opcode_failed_result_2_1315 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1315) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1540 = stack_status;
    goto end_function_2139;
  }
  uint32_t pc_after = (*carried_pc);
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1316 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_mid);
  uint64_t gas_after_charge = charge_result_2_1316.tup1;
  if (charge_result_2_1316.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1317 = Failed(OutOfGas);
    (*carried_pc) = pc_after;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1317;
  }
  u256 dest = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_uint_32_uint_64_OpcodeOutcome do_jump_result_2_1319;
  do_jump_result_2_1319.tup0 = pc_after;
  do_jump_result_2_1319.tup2 = do_jump(&((&do_jump_result_2_1319)->tup0), &gas_after_charge, carried_code, dest);
  do_jump_result_2_1319.tup1 = gas_after_charge;
  pc_after = do_jump_result_2_1319.tup0;
  gas_after = do_jump_result_2_1319.tup1;
  (*carried_pc) = pc_after;
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1540 = do_jump_result_2_1319.tup2;
end_function_2139: ;
  return opcodeoutcome_8_1540;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpi(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1541;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1308 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1308) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1541 = stack_status;
    goto end_function_2127;
  }
  uint32_t pc_after = (*carried_pc);
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1309 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_high);
  uint64_t gas_after_charge = charge_result_2_1309.tup1;
  if (charge_result_2_1309.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1310 = Failed(OutOfGas);
    (*carried_pc) = pc_after;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1310;
  }
  u256 dest = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 cond = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  bool condition_is_zero = word_is_zero(cond);
  if (condition_is_zero) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Continue_result_2_1312 = Continue(UNIT);
    (*carried_pc) = pc_after;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Continue_result_2_1312;
  }
  struct tuple_uint_32_uint_64_OpcodeOutcome do_jump_result_2_1314;
  do_jump_result_2_1314.tup0 = pc_after;
  do_jump_result_2_1314.tup2 = do_jump(&((&do_jump_result_2_1314)->tup0), &gas_after_charge, carried_code, dest);
  do_jump_result_2_1314.tup1 = gas_after_charge;
  pc_after = do_jump_result_2_1314.tup0;
  gas_after = do_jump_result_2_1314.tup1;
  (*carried_pc) = pc_after;
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1541 = do_jump_result_2_1314.tup2;
end_function_2127: ;
  return opcodeoutcome_8_1541;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pc(uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1542;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1303 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1303) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1542 = stack_status;
    goto end_function_2117;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1304 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1304.tup1;
  if (charge_result_2_1304.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1305 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1305;
  }
  u256 next_pc = word_of_source_byte_count((*carried_pc));
  u256 opcode_pc = alu_sub(next_pc, WORD_ONE);
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, opcode_pc);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1307 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1542 = Continue_result_2_1307;
end_function_2117: ;
  return opcodeoutcome_8_1542;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gas(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1543;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1297 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1297) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1543 = stack_status;
    goto end_function_2109;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1298 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
  uint64_t gas_after_charge = charge_result_2_1298.tup1;
  if (charge_result_2_1298.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1299 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1299;
  }
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  u256 word_of_nat_byte_count_result_2_1301 = word_of_nat_byte_count_uint64_t_to_u256(gas_after_charge);
  write_stack_word(sp_after, word_of_nat_byte_count_result_2_1301);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1302 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1543 = Continue_result_2_1302;
end_function_2109: ;
  return opcodeoutcome_8_1543;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpdest(uint64_t *restrict carried_gas)
{
  uint64_t gas_after = (*carried_gas);
  struct tuple_bool_uint_64 charge_result_2_1293 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_jumpdest);
  uint64_t next_gas = charge_result_2_1293.tup1;
  if (charge_result_2_1293.tup0) {
    gas_after = next_gas;
    struct OpcodeOutcome Failed_result_2_1294 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1294;
  }
  gas_after = next_gas;
  struct OpcodeOutcome Continue_result_2_1296 = Continue(UNIT);
  (*carried_gas) = gas_after;
  return Continue_result_2_1296;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n, u256 v)
{
  struct OpcodeOutcome opcodeoutcome_8_1545;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  bool opcode_failed_result_2_1283 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1283) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1545 = stack_status;
    goto end_function_2093;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  if (n == UINT8_C(0)) {
    struct tuple_bool_uint_64 charge_result_2_1285 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_base);
    uint64_t gas_after_charge = charge_result_2_1285.tup1;
    if (charge_result_2_1285.tup0) {
      gas_after = gas_after_charge;
      struct OpcodeOutcome Failed_result_2_1286 = Failed(OutOfGas);
      (*carried_gas) = gas_after;
      (*carried_sp) = sp_after;
      return Failed_result_2_1286;
    }
    sp_after = stack_top_advance_(sp_after, UINT8_C(1));
    write_stack_word(sp_after, v);
    gas_after = gas_after_charge;
    struct OpcodeOutcome Continue_result_2_1288 = Continue(UNIT);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    opcodeoutcome_8_1545 = Continue_result_2_1288;
  } else {
    struct tuple_bool_uint_64 charge_result_2_1289 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
    uint64_t gas_after_charge_3_2111 = charge_result_2_1289.tup1;
    if (charge_result_2_1289.tup0) {
      gas_after = gas_after_charge_3_2111;
      struct OpcodeOutcome Failed_result_2_1290 = Failed(OutOfGas);
      (*carried_gas) = gas_after;
      (*carried_sp) = sp_after;
      return Failed_result_2_1290;
    }
    sp_after = stack_top_advance_(sp_after, UINT8_C(1));
    write_stack_word(sp_after, v);
    gas_after = gas_after_charge_3_2111;
    struct OpcodeOutcome Continue_result_2_1292 = Continue(UNIT);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    opcodeoutcome_8_1545 = Continue_result_2_1292;
  }
end_function_2093: ;
  return opcodeoutcome_8_1545;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome opcodeoutcome_8_1546;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  bool opcode_failed_result_2_1276 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1276) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1546 = stack_status;
    goto end_function_2083;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  struct tuple_bool_uint_64 charge_result_2_1277 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1277.tup1;
  if (charge_result_2_1277.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1278 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1278;
  }
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256(sp_after, ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1280 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1546 = Continue_result_2_1280;
end_function_2083: ;
  return opcodeoutcome_8_1546;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome opcodeoutcome_8_1547;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  bool opcode_failed_result_2_1269 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1269) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1547 = stack_status;
    goto end_function_2075;
  }
  uint64_t gas_after = (*carried_gas);
  struct tuple_bool_uint_64 charge_result_2_1270 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1270.tup1;
  if (charge_result_2_1270.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1271 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1271;
  }
  u256 top_value = read_stack_word((*carried_sp));
  u256 other = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), other);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1273 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1547 = Continue_result_2_1273;
end_function_2075: ;
  return opcodeoutcome_8_1547;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dupn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1548;
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1260 = Failed(InvalidOpcode);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1260;
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  bool opcode_failed_result_2_1262 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1262) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1548 = stack_status;
    goto end_function_2065;
  }
  struct tuple_bool_uint_64 charge_result_2_1263 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1263.tup1;
  if (charge_result_2_1263.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1264 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    return Failed_result_2_1264;
  }
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256(sp_after, ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  sp_after = stack_top_advance_(sp_after, UINT8_C(1));
  write_stack_word(sp_after, value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1266 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  opcodeoutcome_8_1548 = Continue_result_2_1266;
end_function_2065: ;
  return opcodeoutcome_8_1548;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swapn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1549;
  uint64_t gas_after = (*carried_gas);
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1250 = Failed(InvalidOpcode);
    (*carried_gas) = gas_after;
    return Failed_result_2_1250;
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  bool opcode_failed_result_2_1252 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1252) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1549 = stack_status;
    goto end_function_2055;
  }
  struct tuple_bool_uint_64 charge_result_2_1253 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1253.tup1;
  if (charge_result_2_1253.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1254 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1254;
  }
  u256 top_value = read_stack_word((*carried_sp));
  u256 other = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), other);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1256 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1549 = Continue_result_2_1256;
end_function_2055: ;
  return opcodeoutcome_8_1549;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exchange(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1550;
  uint64_t gas_after = (*carried_gas);
  bool valid_immediate = exchange_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1239 = Failed(InvalidOpcode);
    (*carried_gas) = gas_after;
    return Failed_result_2_1239;
  }
  struct tuple_uint_16_uint_16 decode_exchange_stack_indices_result_2_1241 = decode_exchange_stack_indices(immediate);
  uint16_t n = decode_exchange_stack_indices_result_2_1241.tup0;
  uint16_t m = decode_exchange_stack_indices_result_2_1241.tup1;
  struct OpcodeOutcome stack_status = guard_stack((*carried_sp), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))));
  bool opcode_failed_result_2_1242 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1242) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1550 = stack_status;
    goto end_function_2045;
  }
  struct tuple_bool_uint_64 charge_result_2_1243 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after, G_verylow);
  uint64_t gas_after_charge = charge_result_2_1243.tup1;
  if (charge_result_2_1243.tup0) {
    gas_after = gas_after_charge;
    struct OpcodeOutcome Failed_result_2_1244 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    return Failed_result_2_1244;
  }
  u256 first = stack_slot_read_((*carried_sp), n);
  u256 second = stack_slot_read_((*carried_sp), m);
  stack_set((*carried_sp), n, second);
  stack_set((*carried_sp), m, first);
  gas_after = gas_after_charge;
  struct OpcodeOutcome Continue_result_2_1246 = Continue(UNIT);
  (*carried_gas) = gas_after;
  opcodeoutcome_8_1550 = Continue_result_2_1246;
end_function_2045: ;
  return opcodeoutcome_8_1550;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log(bytes20 carried_address, bool carried_is_static, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory, uint8_t n)
{
  struct OpcodeOutcome opcodeoutcome_8_1551;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(2) + (uint32_t)n)), UINT8_C(0));
  bool opcode_failed_result_2_1220 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1220) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1551 = stack_status;
    goto end_function_2031;
  }
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  uint64_t gas_after_static_guard;
  struct OpcodeOutcome status_after_guard;
  gas_after_static_guard = gas_after;
  status_after_guard = guard_static(&gas_after_static_guard, carried_is_static);
  bool opcode_failed_result_2_1222 = opcode_failed(status_after_guard);
  if (opcode_failed_result_2_1222) {
    gas_after = gas_after_static_guard;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return status_after_guard;
  }
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  struct tuple_LogTopics_StackPointer pop_log_topics_result_2_1223 = pop_log_topics(n, sp_after);
  sp_after = pop_log_topics_result_2_1223.tup1;
  struct tuple_bool_uint_64 charge_log_gas_result_2_1224 = charge_log_gas(gas_after_static_guard, n, length_word);
  bool log_halt = charge_log_gas_result_2_1224.tup0;
  uint64_t gas_after_log = charge_log_gas_result_2_1224.tup1;
  if (log_halt) {
    gas_after = gas_after_log;
    struct OpcodeOutcome Failed_result_2_1225 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1225;
  }
  uint64_t required_size = memory_required_size(offset_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1227 = charge_memory_expansion(gas_after_log, memory_after, required_size);
  bool expansion_halt = charge_memory_expansion_result_2_1227.tup0;
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1227.tup1;
  if (expansion_halt) {
    gas_after = gas_after_expansion;
    struct OpcodeOutcome Failed_result_2_1228 = Failed(OutOfGas);
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return Failed_result_2_1228;
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  struct tuple_Bytes_Bytes result_2_1232 = active_memory_slice(mem1, access.range.off, access.range.len);
  Bytes memory_slice = evm_memory_slice(result_2_1232.tup0.bytes, result_2_1232.tup0.len);
  struct LogData log_data = LogDataMemory(memory_slice);
  k_log(carried_address, pop_log_topics_result_2_1223.tup0, log_data);
  memory_after = result_2_1232.tup1;
  gas_after = gas_after_expansion;
  struct OpcodeOutcome Continue_result_2_1233 = Continue(UNIT);
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  opcodeoutcome_8_1551 = Continue_result_2_1233;
end_function_2031: ;
  return opcodeoutcome_8_1551;
}

struct FrameStatus execute_stop(void)
{
  struct HaltKind reason = HaltStop(UNIT);
  return Halted(reason);
}

__attribute__((__always_inline__)) struct FrameStatus execute_return(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct FrameStatus framestatus_8_1552;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1212 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1212) {
    struct FrameStatus opcode_frame_status_result_2_1213 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1552 = opcode_frame_status_result_2_1213;
    goto end_function_2013;
  }
  struct FrameStatus status_after = Running(UNIT);
  uint64_t gas_after = (*carried_gas);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint64_t required_size = memory_required_size(offset_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1215 = charge_memory_expansion(gas_after, memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1215.tup1;
  if (charge_memory_expansion_result_2_1215.tup0) {
    status_after = Exceptional(OutOfGas);
    gas_after = gas_after_expansion;
    (*carried_gas) = gas_after;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return status_after;
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  struct tuple_Bytes_Bytes result_2_1218 = active_memory_slice(mem1, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(result_2_1218.tup0);
  struct HaltKind reason = HaltReturn(output);
  status_after = Halted(reason);
  memory_after = result_2_1218.tup1;
  gas_after = gas_after_expansion;
  (*carried_gas) = gas_after;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  framestatus_8_1552 = status_after;
end_function_2013: ;
  return framestatus_8_1552;
}

__attribute__((__always_inline__)) struct FrameStatus execute_revert(uint64_t carried_state_gas_reservoir, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, StackPointer *restrict carried_sp, Bytes *restrict carried_memory)
{
  struct FrameStatus framestatus_8_1553;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  bool opcode_failed_result_2_1203 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1203) {
    struct FrameStatus opcode_frame_status_result_2_1204 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1553 = opcode_frame_status_result_2_1204;
    goto end_function_2003;
  }
  struct FrameStatus status_after = Running(UNIT);
  StackPointer sp_after = (*carried_sp);
  Bytes memory_after = (*carried_memory);
  u256 offset_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  u256 length_word = read_stack_word(sp_after);
  sp_after = stack_top_retreat_(sp_after, UINT8_C(1));
  uint64_t required_size = memory_required_size(offset_word, length_word);
  struct tuple_bool_uint_64 charge_memory_expansion_result_2_1206 = charge_memory_expansion((*carried_gas), memory_after, required_size);
  uint64_t gas_after_expansion = charge_memory_expansion_result_2_1206.tup1;
  if (charge_memory_expansion_result_2_1206.tup0) {
    status_after = Exceptional(OutOfGas);
    (*carried_gas) = gas_after_expansion;
    (*carried_sp) = sp_after;
    (*carried_memory) = memory_after;
    return status_after;
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(memory_after, access.required_size);
  struct tuple_uint_64_uint_64_uint_32 refill_frame_state_gas_result_2_1207 = refill_frame_state_gas(gas_after_expansion, (*carried_state_gas), (*carried_state_spill), carried_state_gas_reservoir);
  struct tuple_Bytes_Bytes result_2_1210 = active_memory_slice(mem1, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(result_2_1210.tup0);
  struct HaltKind reason = HaltRevert(output);
  status_after = Halted(reason);
  memory_after = result_2_1210.tup1;
  (*carried_gas) = refill_frame_state_gas_result_2_1207.tup0;
  (*carried_state_gas) = refill_frame_state_gas_result_2_1207.tup1;
  (*carried_state_spill) = refill_frame_state_gas_result_2_1207.tup2;
  (*carried_sp) = sp_after;
  (*carried_memory) = memory_after;
  framestatus_8_1553 = status_after;
end_function_2003: ;
  return framestatus_8_1553;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_invalid(uint64_t *restrict carried_gas)
{
  (void)carried_gas;
  struct OpcodeOutcome Failed_result_2_1202 = Failed(InvalidOpcode);
  return Failed_result_2_1202;
}

__attribute__((__always_inline__)) struct FrameStatus execute_selfdestruct(bytes20 carried_address, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct FrameStatus framestatus_8_1555;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  bool opcode_failed_result_2_1165 = opcode_failed(stack_status);
  if (opcode_failed_result_2_1165) {
    struct FrameStatus opcode_frame_status_result_2_1166 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1555 = opcode_frame_status_result_2_1166;
    goto end_function_1977;
  }
  uint64_t g_after_8_1564;
  struct OpcodeOutcome guard_result;
  g_after_8_1564 = (*carried_gas);
  guard_result = guard_static(&g_after_8_1564, carried_is_static);
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_FrameStatus tmp_3_1908;
  bool opcode_failed_result_2_1169 = opcode_failed(guard_result);
  if (opcode_failed_result_2_1169) {
    struct FrameStatus opcode_frame_status_result_2_1170 = opcode_frame_status(guard_result);
    (*carried_gas) = g_after_8_1564;
    return opcode_frame_status_result_2_1170;
  }
  u256 beneficiary_word = read_stack_word((*carried_sp));
  StackPointer sp_after = stack_top_retreat_((*carried_sp), UINT8_C(1));
  bytes20 beneficiary = word_to_address(beneficiary_word);
  bool gteq_int_result_2_1172 = (bool)(fork >= Amsterdam);
  if (gteq_int_result_2_1172) {
    bool warm = k_account_is_warm(beneficiary);
    uint32_t access_cost;
    uint16_t add_atom_result_2_1184 = ((uint16_t)((uint32_t)G_selfdestruct + (uint32_t)UINT16_C(0)));
    uint16_t result_2_1185 = warm ? G_zero : G_amsterdam_cold_account_access;
    access_cost = ((uint32_t)add_atom_result_2_1184 + (uint32_t)result_2_1185);
    struct tuple_bool_uint_64 check_execution_gas_result_2_1173 = check_execution_gas_uint64_t_uint32_t_to_struct_tuple_bool_uint_64((*carried_gas), access_cost);
    if (check_execution_gas_result_2_1173.tup0) {
      struct FrameStatus Exceptional_result_2_1174 = Exceptional(OutOfGas);
      (*carried_gas) = check_execution_gas_result_2_1173.tup1;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1174;
    }
    k_account_mark_warm(beneficiary);
    u256 bal = k_get_balance(carried_address);
    bool nonzero_balance = word_nonzero(bal);
    bool beneficiary_empty = k_account_is_empty(beneficiary);
    bool creates_account = (bool)(nonzero_balance && beneficiary_empty);
    uint32_t execution_cost = creates_account ? (access_cost + (uint32_t)G_amsterdam_account_write) : access_cost;
    struct tuple_bool_uint_64 charge_result_2_1176 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64((*carried_gas), execution_cost);
    uint64_t gas_after_execution = charge_result_2_1176.tup1;
    if (charge_result_2_1176.tup0) {
      struct FrameStatus Exceptional_result_2_1177 = Exceptional(OutOfGas);
      (*carried_gas) = gas_after_execution;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1177;
    }
    struct tuple_bool_uint_64_uint_64_uint_32 result_2_1179;
    if (creates_account) {
      result_2_1179 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after_execution, (*carried_state_gas), (*carried_state_spill), G_amsterdam_state_new_account);
    } else {
      result_2_1179 = ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after_execution, .tup2 = (*carried_state_gas), .tup3 = (*carried_state_spill)});
    }
    bool state_halt = result_2_1179.tup0;
    uint64_t gas_after_all_charges = result_2_1179.tup1;
    uint64_t state_gas_after = result_2_1179.tup2;
    uint32_t state_spill_after = result_2_1179.tup3;
    if (state_halt) {
      struct FrameStatus Exceptional_result_2_1180 = Exceptional(OutOfGas);
      (*carried_gas) = gas_after_all_charges;
      (*carried_state_gas) = state_gas_after;
      (*carried_state_spill) = state_spill_after;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1180;
    }
    k_transfer(carried_address, beneficiary, bal);
    bool created = k_was_created(carried_address);
    if (created) {
      k_selfdestruct(carried_address);
    }
    struct FrameStatus result_2_1183;
    struct HaltKind HaltSelfDestruct_result_2_1182 = HaltSelfDestruct(UNIT);
    result_2_1183 = Halted(HaltSelfDestruct_result_2_1182);
    tmp_3_1908 = ((struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_FrameStatus){.tup0 = gas_after_all_charges, .tup1 = state_gas_after, .tup2 = state_spill_after, .tup3 = (*carried_refund), .tup4 = sp_after, .tup5 = result_2_1183});
  } else {
    u256 bal_3_1952 = k_get_balance(carried_address);
    bool warm_3_1953 = k_account_is_warm(beneficiary);
    struct tuple_bool_uint_64 charge_result_2_1186 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64((*carried_gas), G_selfdestruct);
    uint64_t gas_after_execution_3_1954 = charge_result_2_1186.tup1;
    if (charge_result_2_1186.tup0) {
      struct FrameStatus Exceptional_result_2_1187 = Exceptional(OutOfGas);
      (*carried_gas) = gas_after_execution_3_1954;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1187;
    }
    struct tuple_bool_uint_64 result_2_1190;
    if (warm_3_1953) {
      result_2_1190 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after_execution_3_1954});
    } else {
      result_2_1190 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after_execution_3_1954, G_cold_account);
    }
    uint64_t gas_after_access = result_2_1190.tup1;
    if (result_2_1190.tup0) {
      struct FrameStatus Exceptional_result_2_1191 = Exceptional(OutOfGas);
      (*carried_gas) = gas_after_access;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1191;
    }
    k_account_mark_warm(beneficiary);
    bool nonzero_balance_3_1955 = word_nonzero(bal_3_1952);
    bool beneficiary_empty_3_1956 = k_account_is_empty(beneficiary);
    struct tuple_bool_uint_64 result_2_1194;
    if (nonzero_balance_3_1955 && beneficiary_empty_3_1956) {
      result_2_1194 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_after_access, G_newaccount);
    } else {
      result_2_1194 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = gas_after_access});
    }
    bool new_account_halt = result_2_1194.tup0;
    uint64_t gas_after_all_charges_3_1957 = result_2_1194.tup1;
    if (new_account_halt) {
      struct FrameStatus Exceptional_result_2_1195 = Exceptional(OutOfGas);
      (*carried_gas) = gas_after_all_charges_3_1957;
      (*carried_sp) = sp_after;
      return Exceptional_result_2_1195;
    }
    bool is_selfdestructed = k_is_selfdestructed(carried_address);
    __int128 refund_after;
    bool lt_int_result_2_1200 = (bool)(fork < London);
    if (lt_int_result_2_1200 && !is_selfdestructed) {
      refund_after = record_refund___int128_uint16_t_to___int128((*carried_refund), R_selfdestruct_pre_london);
    } else {
      refund_after = (*carried_refund);
    }
    k_transfer(carried_address, beneficiary, bal_3_1952);
    bool lt_int_result_2_1197 = (bool)(fork < Cancun);
    if (lt_int_result_2_1197) {
      k_zero_balance(carried_address);
      k_selfdestruct(carried_address);
    } else {
      bool created_3_1958 = k_was_created(carried_address);
      if (created_3_1958) {
        k_zero_balance(carried_address);
        k_selfdestruct(carried_address);
      }
    }
    struct FrameStatus result_2_1199;
    struct HaltKind HaltSelfDestruct_result_2_1198 = HaltSelfDestruct(UNIT);
    result_2_1199 = Halted(HaltSelfDestruct_result_2_1198);
    tmp_3_1908 = ((struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_FrameStatus){.tup0 = gas_after_all_charges_3_1957, .tup1 = (*carried_state_gas), .tup2 = (*carried_state_spill), .tup3 = refund_after, .tup4 = sp_after, .tup5 = result_2_1199});
  }
  (*carried_gas) = tmp_3_1908.tup0;
  (*carried_state_gas) = tmp_3_1908.tup1;
  (*carried_state_spill) = tmp_3_1908.tup2;
  (*carried_refund) = tmp_3_1908.tup3;
  (*carried_sp) = tmp_3_1908.tup4;
  framestatus_8_1555 = tmp_3_1908.tup5;
end_function_1977: ;
  return framestatus_8_1555;
}

struct OpcodeOutcome guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint16_t inputs, uint8_t outputs)
{
  enum StackValidation validate_stack_result_2_1780;
  if (carried_sp.height < inputs) {
    validate_stack_result_2_1780 = StackUnderflowFailure;
  } else {
    bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
    validate_stack_result_2_1780 = result_2_2037 ? StackOverflowFailure : StackValid;
  }
  switch (validate_stack_result_2_1780) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint8_t inputs, uint8_t outputs)
{
  enum StackValidation validate_stack_result_2_1780;
  if (carried_sp.height < inputs) {
    validate_stack_result_2_1780 = StackUnderflowFailure;
  } else {
    bool result_2_2037 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
    validate_stack_result_2_1780 = result_2_2037 ? StackOverflowFailure : StackValid;
  }
  switch (validate_stack_result_2_1780) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2(StackPointer carried_sp, uint8_t inputs, uint8_t outputs)
{
  enum StackValidation validate_stack_result_2_1780;
  if (carried_sp.height < inputs) {
    validate_stack_result_2_1780 = StackUnderflowFailure;
  } else {
    bool result_2_2037 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs)) + (int32_t)(int16_t)outputs)));
    validate_stack_result_2_1780 = result_2_2037 ? StackOverflowFailure : StackValid;
  }
  switch (validate_stack_result_2_1780) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

struct OpcodeOutcome guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3(StackPointer carried_sp, uint8_t inputs, uint8_t outputs)
{
  enum StackValidation validate_stack_result_2_1780;
  bool result_2_2037 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
  validate_stack_result_2_1780 = result_2_2037 ? StackOverflowFailure : StackValid;
  switch (validate_stack_result_2_1780) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

