#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

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
    struct OpcodeOutcome Failed_result_2_1773 = Failed(WriteProtection);
    (*g) = (uint64_t)GAS_ZERO;
    return Failed_result_2_1773;
  } else {
    struct OpcodeOutcome Continue_result_2_1774 = Continue(UNIT);
    return Continue_result_2_1774;
  }
}

__attribute__((__always_inline__)) struct OpcodeOutcome do_jump(uint32_t *restrict pc_in, uint64_t *restrict g, struct CodeFields frame_code, u256 destination_value)
{
  uint32_t code_length = frame_code_len(frame_code);
  if (u256_lt_u64(destination_value, code_length)) {
    uint32_t destination = (uint32_t)u256_to_u64_unchecked(destination_value);
    bool valid_destination = frame_jumpdest_valid(frame_code, destination);
    if (valid_destination) {
      struct OpcodeOutcome Continue_result_2_1770 = Continue(UNIT);
      (*pc_in) = destination;
      return Continue_result_2_1770;
    } else {
      struct OpcodeOutcome Failed_result_2_1771 = Failed(InvalidJump);
      (*g) = (uint64_t)GAS_ZERO;
      return Failed_result_2_1771;
    }
  } else {
    struct OpcodeOutcome Failed_result_2_1772 = Failed(InvalidJump);
    (*g) = (uint64_t)GAS_ZERO;
    return Failed_result_2_1772;
  }
}

struct OpcodeOutcome guard_stack(StackPointer carried_sp, uint16_t inputs, uint16_t outputs)
{
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else {
    bool result_2_2018 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
    stack_validation = result_2_2018 ? StackOverflowFailure : StackValid;
  }
  switch (stack_validation) {
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
    struct LogTopics LogTopics0_result_2_1763 = LogTopics0(UNIT);
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics0_result_2_1763, .tup1 = sp});
  }
  if (count == UINT8_C(1)) {
    u256 t0 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics1_result_2_1764 = LogTopics1(t0);
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics1_result_2_1764, .tup1 = sp});
  }
  if (count == UINT8_C(2)) {
    u256 t0_3_2902 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics2_result_2_1765 = LogTopics2(((struct tuple_u256_u256){.tup0 = t0_3_2902, .tup1 = t1}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics2_result_2_1765, .tup1 = sp});
  }
  if (count == UINT8_C(3)) {
    u256 t0_3_2904 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1_3_2905 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t2 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics3_result_2_1766 = LogTopics3(((struct tuple_u256_u256_u256){.tup0 = t0_3_2904, .tup1 = t1_3_2905, .tup2 = t2}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics3_result_2_1766, .tup1 = sp});
  }
  if (count == UINT8_C(4)) {
    u256 t0_3_2907 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1_3_2908 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t2_3_2909 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t3 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    struct LogTopics LogTopics4_result_2_1767 = LogTopics4(((struct tuple_u256_u256_u256_u256){.tup0 = t0_3_2907, .tup1 = t1_3_2908, .tup2 = t2_3_2909, .tup3 = t3}));
    return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics4_result_2_1767, .tup1 = sp});
  }
  struct LogTopics LogTopics0_result_2_1768 = LogTopics0(UNIT);
  return ((struct tuple_LogTopics_StackPointer){.tup0 = LogTopics0_result_2_1768, .tup1 = sp});
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_add(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1410;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1410 = stack_status;
    goto end_function_2673;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1760 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1410 = Failed_result_2_1760;
    goto end_function_2673;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_add(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1762 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1410 = Continue_result_2_1762;
end_function_2673: ;
  return opcodeoutcome_8_1410;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mul(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1411;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1411 = stack_status;
    goto end_function_2664;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1755 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1411 = Failed_result_2_1755;
    goto end_function_2664;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_mul(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1757 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1411 = Continue_result_2_1757;
end_function_2664: ;
  return opcodeoutcome_8_1411;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sub(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1412;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1412 = stack_status;
    goto end_function_2655;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1750 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1412 = Failed_result_2_1750;
    goto end_function_2655;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_sub(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1752 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1412 = Continue_result_2_1752;
end_function_2655: ;
  return opcodeoutcome_8_1412;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_div(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1413;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1413 = stack_status;
    goto end_function_2646;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1745 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1413 = Failed_result_2_1745;
    goto end_function_2646;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_div(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1747 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1413 = Continue_result_2_1747;
end_function_2646: ;
  return opcodeoutcome_8_1413;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sdiv(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1414;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1414 = stack_status;
    goto end_function_2637;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1740 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1414 = Failed_result_2_1740;
    goto end_function_2637;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_sdiv(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1742 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1414 = Continue_result_2_1742;
end_function_2637: ;
  return opcodeoutcome_8_1414;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1415;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1415 = stack_status;
    goto end_function_2628;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1735 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1415 = Failed_result_2_1735;
    goto end_function_2628;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_mod(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1737 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1415 = Continue_result_2_1737;
end_function_2628: ;
  return opcodeoutcome_8_1415;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_smod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1416;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1416 = stack_status;
    goto end_function_2619;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1730 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1416 = Failed_result_2_1730;
    goto end_function_2619;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_smod(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1732 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1416 = Continue_result_2_1732;
end_function_2619: ;
  return opcodeoutcome_8_1416;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_addmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1417;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1417 = stack_status;
    goto end_function_2610;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_mid) {
    struct OpcodeOutcome Failed_result_2_1725 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1417 = Failed_result_2_1725;
    goto end_function_2610;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_mid);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 n = read_stack_word(sp);
  u256 result = alu_addmod(a, b, n);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1727 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1417 = Continue_result_2_1727;
end_function_2610: ;
  return opcodeoutcome_8_1417;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mulmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1418;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1418 = stack_status;
    goto end_function_2601;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_mid) {
    struct OpcodeOutcome Failed_result_2_1720 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1418 = Failed_result_2_1720;
    goto end_function_2601;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_mid);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 n = read_stack_word(sp);
  u256 result = alu_mulmod(a, b, n);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1722 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1418 = Continue_result_2_1722;
end_function_2601: ;
  return opcodeoutcome_8_1418;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1419;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1419 = stack_status;
    goto end_function_2592;
  }
  StackPointer sp = (*carried_sp);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 e = read_stack_word(sp);
  uint64_t gas_cost = exp_gas(e);
  if ((*carried_gas) < gas_cost) {
    struct OpcodeOutcome Failed_result_2_1715 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1419 = Failed_result_2_1715;
    goto end_function_2592;
  }
  u256 result = alu_exp(a, e);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1717 = Continue(UNIT);
  (*carried_gas) = ((*carried_gas) - gas_cost);
  (*carried_sp) = sp;
  opcodeoutcome_8_1419 = Continue_result_2_1717;
end_function_2592: ;
  return opcodeoutcome_8_1419;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_signextend(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1420;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1420 = stack_status;
    goto end_function_2583;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1710 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1420 = Failed_result_2_1710;
    goto end_function_2583;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 bi = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  u256 result = alu_signextend(bi, v);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1712 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1420 = Continue_result_2_1712;
end_function_2583: ;
  return opcodeoutcome_8_1420;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_lt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1421;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1421 = stack_status;
    goto end_function_2574;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1705 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1421 = Failed_result_2_1705;
    goto end_function_2574;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_lt(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1707 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1421 = Continue_result_2_1707;
end_function_2574: ;
  return opcodeoutcome_8_1421;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1422;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1422 = stack_status;
    goto end_function_2565;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1700 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1422 = Failed_result_2_1700;
    goto end_function_2565;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_gt(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1702 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1422 = Continue_result_2_1702;
end_function_2565: ;
  return opcodeoutcome_8_1422;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1423;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1423 = stack_status;
    goto end_function_2556;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1695 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1423 = Failed_result_2_1695;
    goto end_function_2556;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_slt(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1697 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1423 = Continue_result_2_1697;
end_function_2556: ;
  return opcodeoutcome_8_1423;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sgt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1424;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1424 = stack_status;
    goto end_function_2547;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1690 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1424 = Failed_result_2_1690;
    goto end_function_2547;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_sgt(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1692 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1424 = Continue_result_2_1692;
end_function_2547: ;
  return opcodeoutcome_8_1424;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_eq(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1425;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1425 = stack_status;
    goto end_function_2538;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1685 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1425 = Failed_result_2_1685;
    goto end_function_2538;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_eq(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1687 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1425 = Continue_result_2_1687;
end_function_2538: ;
  return opcodeoutcome_8_1425;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_iszero(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1426;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1426 = stack_status;
    goto end_function_2529;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1680 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1426 = Failed_result_2_1680;
    goto end_function_2529;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word((*carried_sp));
  u256 result = alu_iszero(a);
  write_stack_word((*carried_sp), result);
  struct OpcodeOutcome Continue_result_2_1682 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1426 = Continue_result_2_1682;
end_function_2529: ;
  return opcodeoutcome_8_1426;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_and(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1427;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1427 = stack_status;
    goto end_function_2520;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1675 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1427 = Failed_result_2_1675;
    goto end_function_2520;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_and(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1677 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1427 = Continue_result_2_1677;
end_function_2520: ;
  return opcodeoutcome_8_1427;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_or(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1428;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1428 = stack_status;
    goto end_function_2511;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1670 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1428 = Failed_result_2_1670;
    goto end_function_2511;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_or(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1672 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1428 = Continue_result_2_1672;
end_function_2511: ;
  return opcodeoutcome_8_1428;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_xor(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1429;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1429 = stack_status;
    goto end_function_2502;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1665 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1429 = Failed_result_2_1665;
    goto end_function_2502;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  u256 result = alu_xor(a, b);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1667 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1429 = Continue_result_2_1667;
end_function_2502: ;
  return opcodeoutcome_8_1429;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_not(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1430;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1430 = stack_status;
    goto end_function_2493;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1660 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1430 = Failed_result_2_1660;
    goto end_function_2493;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 a = read_stack_word((*carried_sp));
  u256 result = alu_not(a);
  write_stack_word((*carried_sp), result);
  struct OpcodeOutcome Continue_result_2_1662 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1430 = Continue_result_2_1662;
end_function_2493: ;
  return opcodeoutcome_8_1430;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_byte(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1431;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1431 = stack_status;
    goto end_function_2484;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1655 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1431 = Failed_result_2_1655;
    goto end_function_2484;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 i = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 x = read_stack_word(sp);
  u256 result = alu_byte(i, x);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1657 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1431 = Continue_result_2_1657;
end_function_2484: ;
  return opcodeoutcome_8_1431;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shl(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1432;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1432 = stack_status;
    goto end_function_2475;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1650 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1432 = Failed_result_2_1650;
    goto end_function_2475;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  u256 result = alu_shl(s, v);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1652 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1432 = Continue_result_2_1652;
end_function_2475: ;
  return opcodeoutcome_8_1432;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shr(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1433;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1433 = stack_status;
    goto end_function_2466;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1645 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1433 = Failed_result_2_1645;
    goto end_function_2466;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  u256 result = alu_shr(s, v);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1647 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1433 = Continue_result_2_1647;
end_function_2466: ;
  return opcodeoutcome_8_1433;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sar(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1434;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1434 = stack_status;
    goto end_function_2457;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1640 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1434 = Failed_result_2_1640;
    goto end_function_2457;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  u256 result = alu_sar(s, v);
  write_stack_word(sp, result);
  struct OpcodeOutcome Continue_result_2_1642 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1434 = Continue_result_2_1642;
end_function_2457: ;
  return opcodeoutcome_8_1434;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_clz(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1435;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1435 = stack_status;
    goto end_function_2448;
  }
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1635 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1435 = Failed_result_2_1635;
    goto end_function_2448;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 x = read_stack_word((*carried_sp));
  u256 result = alu_clz(x);
  write_stack_word((*carried_sp), result);
  struct OpcodeOutcome Continue_result_2_1637 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1435 = Continue_result_2_1637;
end_function_2448: ;
  return opcodeoutcome_8_1435;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_keccak256(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1436;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1436 = stack_status;
    goto end_function_2437;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct GasCharge keccak_cost = keccak_gas_cost(length_word, gas);
  if (!keccak_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1622 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1436 = Failed_result_2_1622;
    goto end_function_2437;
  }
  gas = gas_sub(gas, keccak_cost.cost);
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1627 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1436 = Failed_result_2_1627;
    goto end_function_2437;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  u256 digest = mem_keccak(memory_base, memory, access.range);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, digest);
  struct OpcodeOutcome Continue_result_2_1631 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1436 = Continue_result_2_1631;
end_function_2437: ;
  return opcodeoutcome_8_1436;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_address(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1437;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1437 = stack_status;
    goto end_function_2428;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1616 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1437 = Failed_result_2_1616;
    goto end_function_2428;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 address_word = address_to_word(carried_address);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, address_word);
  struct OpcodeOutcome Continue_result_2_1618 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1437 = Continue_result_2_1618;
end_function_2428: ;
  return opcodeoutcome_8_1437;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_origin(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1438;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1438 = stack_status;
    goto end_function_2419;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1611 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1438 = Failed_result_2_1611;
    goto end_function_2419;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 origin = k_env(F_Origin);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, origin);
  struct OpcodeOutcome Continue_result_2_1613 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1438 = Continue_result_2_1613;
end_function_2419: ;
  return opcodeoutcome_8_1438;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_caller(bytes20 carried_caller, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1439;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1439 = stack_status;
    goto end_function_2410;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1606 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1439 = Failed_result_2_1606;
    goto end_function_2410;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 caller = address_to_word(carried_caller);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, caller);
  struct OpcodeOutcome Continue_result_2_1608 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1439 = Continue_result_2_1608;
end_function_2410: ;
  return opcodeoutcome_8_1439;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_callvalue(u256 carried_value, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1440;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1440 = stack_status;
    goto end_function_2401;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1601 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1440 = Failed_result_2_1601;
    goto end_function_2401;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, carried_value);
  struct OpcodeOutcome Continue_result_2_1603 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1440 = Continue_result_2_1603;
end_function_2401: ;
  return opcodeoutcome_8_1440;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gasprice(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1441;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1441 = stack_status;
    goto end_function_2392;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1596 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1441 = Failed_result_2_1596;
    goto end_function_2392;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 gas_price = k_env(F_GasPrice);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, gas_price);
  struct OpcodeOutcome Continue_result_2_1598 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1441 = Continue_result_2_1598;
end_function_2392: ;
  return opcodeoutcome_8_1441;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatasize(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1442;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1442 = stack_status;
    goto end_function_2383;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1591 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1442 = Failed_result_2_1591;
    goto end_function_2383;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  uint32_t input_length = calldata_slice_length(carried_calldata);
  u256 length_word = word_of_source_byte_count(input_length);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, length_word);
  struct OpcodeOutcome Continue_result_2_1593 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1442 = Continue_result_2_1593;
end_function_2383: ;
  return opcodeoutcome_8_1442;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldataload(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1443;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1443 = stack_status;
    goto end_function_2374;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1586 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1443 = Failed_result_2_1586;
    goto end_function_2374;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = calldata_slice_load_word_offset(carried_calldata, offset_word);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  struct OpcodeOutcome Continue_result_2_1588 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1443 = Continue_result_2_1588;
end_function_2374: ;
  return opcodeoutcome_8_1443;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatacopy(struct CalldataSlice carried_calldata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1444;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1444 = stack_status;
    goto end_function_2361;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1568 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1444 = Failed_result_2_1568;
    goto end_function_2361;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct GasCharge copy_cost = copy_gas_cost(length_word, gas);
  if (!copy_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1572 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1444 = Failed_result_2_1572;
    goto end_function_2361;
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1577 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1444 = Failed_result_2_1577;
    goto end_function_2361;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  uint32_t destination = memory_absolute(memory_base, range.off);
  calldata_slice_copy_word_offset(carried_calldata, destination, source_word, range.len);
  struct OpcodeOutcome Continue_result_2_1582 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1444 = Continue_result_2_1582;
end_function_2361: ;
  return opcodeoutcome_8_1444;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codesize(struct CodeFields carried_code, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1445;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1445 = stack_status;
    goto end_function_2352;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1563 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1445 = Failed_result_2_1563;
    goto end_function_2352;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  uint32_t code_length = frame_code_len(carried_code);
  u256 length_word = word_of_source_byte_count(code_length);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, length_word);
  struct OpcodeOutcome Continue_result_2_1565 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1445 = Continue_result_2_1565;
end_function_2352: ;
  return opcodeoutcome_8_1445;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codecopy(struct CodeFields carried_code, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1446;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1446 = stack_status;
    goto end_function_2339;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1545 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1446 = Failed_result_2_1545;
    goto end_function_2339;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct GasCharge copy_cost = copy_gas_cost(length_word, gas);
  if (!copy_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1549 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1446 = Failed_result_2_1549;
    goto end_function_2339;
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1554 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1446 = Failed_result_2_1554;
    goto end_function_2339;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  Bytes bytes = code_bytes(carried_code);
  uint32_t destination = memory_absolute(memory_base, range.off);
  code_slice_copy_word_offset(bytes, destination, source_word, range.len);
  struct OpcodeOutcome Continue_result_2_1559 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1446 = Continue_result_2_1559;
end_function_2339: ;
  return opcodeoutcome_8_1446;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_balance(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1447;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1447 = stack_status;
    goto end_function_2330;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  if ((*carried_gas) < gas_cost) {
    struct OpcodeOutcome Failed_result_2_1540 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1447 = Failed_result_2_1540;
    goto end_function_2330;
  }
  k_account_mark_warm(a);
  u256 balance = k_get_balance(a);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, balance);
  struct OpcodeOutcome Continue_result_2_1542 = Continue(UNIT);
  (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
  (*carried_sp) = sp;
  opcodeoutcome_8_1447 = Continue_result_2_1542;
end_function_2330: ;
  return opcodeoutcome_8_1447;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_selfbalance(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1448;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1448 = stack_status;
    goto end_function_2321;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    struct OpcodeOutcome Failed_result_2_1535 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1448 = Failed_result_2_1535;
    goto end_function_2321;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_low);
  u256 balance = k_get_balance(carried_address);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, balance);
  struct OpcodeOutcome Continue_result_2_1537 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1448 = Continue_result_2_1537;
end_function_2321: ;
  return opcodeoutcome_8_1448;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodesize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1449;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1449 = stack_status;
    goto end_function_2312;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  if ((*carried_gas) < ((uint32_t)access_cost + (uint32_t)read_cost)) {
    struct OpcodeOutcome Failed_result_2_1529 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1449 = Failed_result_2_1529;
    goto end_function_2312;
  }
  uint64_t gas = gas_sub_uint64_t_uint32_t_to_uint64_t((*carried_gas), ((uint32_t)access_cost + (uint32_t)read_cost));
  k_account_mark_warm(a);
  uint32_t code_size = k_get_code_size(a);
  u256 size_word = word_of_source_byte_count(code_size);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size_word);
  struct OpcodeOutcome Continue_result_2_1531 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1449 = Continue_result_2_1531;
end_function_2312: ;
  return opcodeoutcome_8_1449;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodecopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1450;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(4), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1450 = stack_status;
    goto end_function_2299;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  if (gas < ((uint32_t)access_cost + (uint32_t)read_cost)) {
    struct OpcodeOutcome Failed_result_2_1509 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1450 = Failed_result_2_1509;
    goto end_function_2299;
  }
  gas = gas_sub_uint64_t_uint32_t_to_uint64_t(gas, ((uint32_t)access_cost + (uint32_t)read_cost));
  struct GasCharge copy_cost = copy_gas_cost(length_word, gas);
  if (!copy_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1514 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1450 = Failed_result_2_1514;
    goto end_function_2299;
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1519 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1450 = Failed_result_2_1519;
    goto end_function_2299;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  k_account_mark_warm(a);
  uint32_t destination = memory_absolute(memory_base, range.off);
  k_code_copy(a, destination, source_word, range.len);
  struct OpcodeOutcome Continue_result_2_1524 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1450 = Continue_result_2_1524;
end_function_2299: ;
  return opcodeoutcome_8_1450;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodehash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1451;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1451 = stack_status;
    goto end_function_2290;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  if ((*carried_gas) < gas_cost) {
    struct OpcodeOutcome Failed_result_2_1503 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1451 = Failed_result_2_1503;
    goto end_function_2290;
  }
  k_account_mark_warm(a);
  bytes32 code_hash = k_get_codehash(a);
  u256 hash_word = hash_to_word(code_hash);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, hash_word);
  struct OpcodeOutcome Continue_result_2_1505 = Continue(UNIT);
  (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
  (*carried_sp) = sp;
  opcodeoutcome_8_1451 = Continue_result_2_1505;
end_function_2290: ;
  return opcodeoutcome_8_1451;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatasize(Bytes carried_returndata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1452;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1452 = stack_status;
    goto end_function_2281;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1498 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1452 = Failed_result_2_1498;
    goto end_function_2281;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  uint32_t return_data_size = returndata_size(carried_returndata);
  u256 size_word = word_of_source_byte_count(return_data_size);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size_word);
  struct OpcodeOutcome Continue_result_2_1500 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1452 = Continue_result_2_1500;
end_function_2281: ;
  return opcodeoutcome_8_1452;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatacopy(Bytes carried_returndata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1453;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1453 = stack_status;
    goto end_function_2268;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1475 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1453 = Failed_result_2_1475;
    goto end_function_2268;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t returndata_size_result_2_1477 = returndata_size(carried_returndata);
  struct tuple_uint_64_StackPointer_uint_32_OpcodeOutcome tmp_3_2343;
  if (!u64_lt_u256(returndata_size_result_2_1477, source_word)) {
    uint32_t remaining;
    uint32_t tmp_3_3760 = (uint32_t)u256_to_u64(source_word);
    remaining = returndata_remaining(returndata_size_result_2_1477, tmp_3_3760);
    if (!u64_lt_u256(remaining, length_word)) {
      struct GasCharge copy_cost = copy_gas_cost_u256_uint64_t_to_struct_GasCharge(length_word, gas);
      if (!copy_cost.affordable) {
        struct OpcodeOutcome Failed_result_2_1482 = Failed(OutOfGas);
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_sp) = sp;
        (*carried_memory_height) = memory;
        opcodeoutcome_8_1453 = Failed_result_2_1482;
        goto end_function_2268;
      }
      gas = gas_sub(gas, copy_cost.cost);
      uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t_variant_2(destination_word, length_word);
      struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
      if (!expansion_cost.affordable) {
        struct OpcodeOutcome Failed_result_2_1487 = Failed(OutOfGas);
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_sp) = sp;
        (*carried_memory_height) = memory;
        opcodeoutcome_8_1453 = Failed_result_2_1487;
        goto end_function_2268;
      }
      gas = gas_sub(gas, expansion_cost.cost);
      struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields_variant_2(destination_word, length_word);
      memory = expand_memory(memory_base, memory, access.requested_height);
      uint32_t destination = memory_absolute(memory_base, access.range.off);
      returndata_copy(carried_returndata, destination, (uint32_t)u256_to_u64_unchecked(source_word), (uint32_t)u256_to_u64_unchecked(length_word));
      struct OpcodeOutcome Continue_result_2_1491 = Continue(UNIT);
      tmp_3_2343 = ((struct tuple_uint_64_StackPointer_uint_32_OpcodeOutcome){.tup0 = gas, .tup1 = sp, .tup2 = memory, .tup3 = Continue_result_2_1491});
    } else {
      struct OpcodeOutcome Failed_result_2_1494 = Failed(InvalidOpcode);
      struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome tmp_3_2346 = ((struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome){.tup0 = GAS_ZERO, .tup1 = sp, .tup2 = memory, .tup3 = Failed_result_2_1494});
      /* conversions */
      tmp_3_2343.tup0 = (uint64_t)tmp_3_2346.tup0;
      tmp_3_2343.tup1 = tmp_3_2346.tup1;
      tmp_3_2343.tup2 = tmp_3_2346.tup2;
      tmp_3_2343.tup3 = tmp_3_2346.tup3;
      /* end conversions */
    }
  } else {
    struct OpcodeOutcome Failed_result_2_1495 = Failed(InvalidOpcode);
    struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome tmp_3_2345 = ((struct tuple_uint_8_StackPointer_uint_32_OpcodeOutcome){.tup0 = GAS_ZERO, .tup1 = sp, .tup2 = memory, .tup3 = Failed_result_2_1495});
    /* conversions */
    tmp_3_2343.tup0 = (uint64_t)tmp_3_2345.tup0;
    tmp_3_2343.tup1 = tmp_3_2345.tup1;
    tmp_3_2343.tup2 = tmp_3_2345.tup2;
    tmp_3_2343.tup3 = tmp_3_2345.tup3;
    /* end conversions */
  }
  (*carried_gas) = tmp_3_2343.tup0;
  (*carried_sp) = tmp_3_2343.tup1;
  (*carried_memory_height) = tmp_3_2343.tup2;
  opcodeoutcome_8_1453 = tmp_3_2343.tup3;
end_function_2268: ;
  return opcodeoutcome_8_1453;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blockhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1454;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1454 = stack_status;
    goto end_function_2257;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < UINT8_C(20)) {
    struct OpcodeOutcome Failed_result_2_1470 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1454 = Failed_result_2_1470;
    goto end_function_2257;
  }
  u256 block_number = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes32 block_hash = k_blockhash(block_number);
  u256 hash_word = hash_to_word(block_hash);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, hash_word);
  struct OpcodeOutcome Continue_result_2_1472 = Continue(UNIT);
  (*carried_gas) = ((*carried_gas) - UINT64_C(20));
  (*carried_sp) = sp;
  opcodeoutcome_8_1454 = Continue_result_2_1472;
end_function_2257: ;
  return opcodeoutcome_8_1454;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_coinbase(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1455;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1455 = stack_status;
    goto end_function_2248;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1465 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1455 = Failed_result_2_1465;
    goto end_function_2248;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 coinbase = k_env(F_Coinbase);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, coinbase);
  struct OpcodeOutcome Continue_result_2_1467 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1455 = Continue_result_2_1467;
end_function_2248: ;
  return opcodeoutcome_8_1455;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_timestamp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1456;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1456 = stack_status;
    goto end_function_2239;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1460 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1456 = Failed_result_2_1460;
    goto end_function_2239;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 timestamp = k_env(F_Timestamp);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, timestamp);
  struct OpcodeOutcome Continue_result_2_1462 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1456 = Continue_result_2_1462;
end_function_2239: ;
  return opcodeoutcome_8_1456;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_number(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1457;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1457 = stack_status;
    goto end_function_2230;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1455 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1457 = Failed_result_2_1455;
    goto end_function_2230;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 block_number = k_env(F_Number);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, block_number);
  struct OpcodeOutcome Continue_result_2_1457 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1457 = Continue_result_2_1457;
end_function_2230: ;
  return opcodeoutcome_8_1457;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slotnum(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1458;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1458 = stack_status;
    goto end_function_2221;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1450 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1458 = Failed_result_2_1450;
    goto end_function_2221;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 slot_number = k_env(F_SlotNumber);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, slot_number);
  struct OpcodeOutcome Continue_result_2_1452 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1458 = Continue_result_2_1452;
end_function_2221: ;
  return opcodeoutcome_8_1458;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_prevrandao(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1459;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1459 = stack_status;
    goto end_function_2212;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1445 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1459 = Failed_result_2_1445;
    goto end_function_2212;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 prev_randao = k_env(F_PrevRandao);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, prev_randao);
  struct OpcodeOutcome Continue_result_2_1447 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1459 = Continue_result_2_1447;
end_function_2212: ;
  return opcodeoutcome_8_1459;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gaslimit(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1460;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1460 = stack_status;
    goto end_function_2203;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1440 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1460 = Failed_result_2_1440;
    goto end_function_2203;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 gas_limit = k_env(F_GasLimit);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, gas_limit);
  struct OpcodeOutcome Continue_result_2_1442 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1460 = Continue_result_2_1442;
end_function_2203: ;
  return opcodeoutcome_8_1460;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_chainid(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1461;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1461 = stack_status;
    goto end_function_2194;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1435 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1461 = Failed_result_2_1435;
    goto end_function_2194;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 chain_id = k_env(F_ChainId);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, chain_id);
  struct OpcodeOutcome Continue_result_2_1437 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1461 = Continue_result_2_1437;
end_function_2194: ;
  return opcodeoutcome_8_1461;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_basefee(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1462;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1462 = stack_status;
    goto end_function_2185;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1430 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1462 = Failed_result_2_1430;
    goto end_function_2185;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 base_fee = k_env(F_BaseFee);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, base_fee);
  struct OpcodeOutcome Continue_result_2_1432 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1462 = Continue_result_2_1432;
end_function_2185: ;
  return opcodeoutcome_8_1462;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobbasefee(u256 blob_fee, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1463;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1463 = stack_status;
    goto end_function_2176;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1425 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1463 = Failed_result_2_1425;
    goto end_function_2176;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, blob_fee);
  struct OpcodeOutcome Continue_result_2_1427 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1463 = Continue_result_2_1427;
end_function_2176: ;
  return opcodeoutcome_8_1463;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1464;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1464 = stack_status;
    goto end_function_2167;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1420 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1464 = Failed_result_2_1420;
    goto end_function_2167;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 index = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 blob_hash = k_blobhash(index);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, blob_hash);
  struct OpcodeOutcome Continue_result_2_1422 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1464 = Continue_result_2_1422;
end_function_2167: ;
  return opcodeoutcome_8_1464;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pop(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1465;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1465 = stack_status;
    goto end_function_2158;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1414 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1465 = Failed_result_2_1414;
    goto end_function_2158;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct OpcodeOutcome Continue_result_2_1417 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1465 = Continue_result_2_1417;
end_function_2158: ;
  return opcodeoutcome_8_1465;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mload(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1466;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1466 = stack_status;
    goto end_function_2145;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1401 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1466 = Failed_result_2_1401;
    goto end_function_2145;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1405 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1466 = Failed_result_2_1405;
    goto end_function_2145;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  memory = expand_memory(memory_base, memory, access.requested_height);
  u256 value = mem_load(memory_base, access.range.off);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  struct OpcodeOutcome Continue_result_2_1409 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1466 = Continue_result_2_1409;
end_function_2145: ;
  return opcodeoutcome_8_1466;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1467;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1467 = stack_status;
    goto end_function_2134;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1388 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1467 = Failed_result_2_1388;
    goto end_function_2134;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1392 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1467 = Failed_result_2_1392;
    goto end_function_2134;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  memory = expand_memory(memory_base, memory, access.requested_height);
  mem_store(memory_base, access.range.off, v);
  struct OpcodeOutcome Continue_result_2_1398 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1467 = Continue_result_2_1398;
end_function_2134: ;
  return opcodeoutcome_8_1467;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore8(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1468;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1468 = stack_status;
    goto end_function_2123;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1375 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1468 = Failed_result_2_1375;
    goto end_function_2123;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height(offset_word, WORD_ONE);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1379 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1468 = Failed_result_2_1379;
    goto end_function_2123;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, WORD_ONE);
  memory = expand_memory(memory_base, memory, access.requested_height);
  mem_store_byte(memory_base, access.range.off, v);
  struct OpcodeOutcome Continue_result_2_1385 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1468 = Continue_result_2_1385;
end_function_2123: ;
  return opcodeoutcome_8_1468;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_msize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1469;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1469 = stack_status;
    goto end_function_2114;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1369 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1469 = Failed_result_2_1369;
    goto end_function_2114;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  uint32_t high_water = memory_high_water((*carried_memory_height));
  uint32_t words = memory_word_count_uint32_t_to_uint32_t(high_water);
  u256 size = word_of_nat_byte_count_uint64_t_to_u256(((uint64_t)words * UINT64_C(32)));
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size);
  struct OpcodeOutcome Continue_result_2_1371 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1469 = Continue_result_2_1371;
end_function_2114: ;
  return opcodeoutcome_8_1469;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mcopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1470;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1470 = stack_status;
    goto end_function_2101;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1344 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1470 = Failed_result_2_1344;
    goto end_function_2101;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct GasCharge copy_cost = copy_gas_cost(length_word, gas);
  if (!copy_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1348 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1470 = Failed_result_2_1348;
    goto end_function_2101;
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t destination_requested_height = memory_requested_height(destination_word, length_word);
  uint64_t source_requested_height = memory_requested_height(source_word, length_word);
  uint64_t requested_height = destination_requested_height < source_requested_height ? source_requested_height : destination_requested_height;
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1353 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1470 = Failed_result_2_1353;
    goto end_function_2101;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields destination = memory_access(destination_word, length_word);
  struct MemoryAccessFields source = memory_access(source_word, length_word);
  uint32_t materialized_required_size = destination.requested_height < source.requested_height ? source.requested_height : destination.requested_height;
  memory = expand_memory(memory_base, memory, materialized_required_size);
  mem_mcopy(memory_base, destination.range.off, source.range.off, destination.range.len);
  struct OpcodeOutcome Continue_result_2_1362 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1470 = Continue_result_2_1362;
end_function_2101: ;
  return opcodeoutcome_8_1470;
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
  struct OpcodeOutcome opcodeoutcome_8_1471;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1471 = stack_status;
    goto end_function_2090;
  }
  StackPointer sp = (*carried_sp);
  u256 slot = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  uint16_t gas_cost = sload_cost(warm);
  if ((*carried_gas) < gas_cost) {
    struct OpcodeOutcome Failed_result_2_1332 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1471 = Failed_result_2_1332;
  } else {
    struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, (bool)(!warm));
    sp = stack_top_advance_(sp, UINT8_C(1));
    write_stack_word(sp, entry.curr);
    struct OpcodeOutcome Continue_result_2_1334 = Continue(UNIT);
    (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
    (*carried_sp) = sp;
    opcodeoutcome_8_1471 = Continue_result_2_1334;
  }
end_function_2090: ;
  return opcodeoutcome_8_1471;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sstore(struct AccountExecutionContext context, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1472;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1472 = stack_status;
    goto end_function_2073;
  }
  uint64_t gas = (*carried_gas);
  uint64_t state_gas = (*carried_state_gas);
  uint32_t state_spill = (*carried_state_spill);
  __int128 refund = (*carried_refund);
  StackPointer sp = (*carried_sp);
  bool halt = false;
  struct OpcodeOutcome status = Continue(UNIT);
  uint64_t g_after_8_1498;
  struct OpcodeOutcome _8_1408_8_1499;
  g_after_8_1498 = gas;
  _8_1408_8_1499 = guard_static(&g_after_8_1498, carried_is_static);
  gas = g_after_8_1498;
  status = _8_1408_8_1499;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return status;
  }
  bool result_2_1293;
  bool lt_int_result_2_1292 = (bool)(fork < Amsterdam);
  result_2_1293 = (bool)(lt_int_result_2_1292 && (gas <= G_callstipend));
  if (result_2_1293) {
    struct OpcodeOutcome Failed_result_2_1294 = Failed(OutOfGas);
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Failed_result_2_1294;
  }
  u256 slot = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  bool cold = (bool)(!warm);
  if (fork >= Amsterdam) {
    uint64_t sentry_cost = sstore_sentry_cost(cold);
    if (gas < sentry_cost) {
      struct OpcodeOutcome Failed_result_2_1298 = Failed(OutOfGas);
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      opcodeoutcome_8_1472 = Failed_result_2_1298;
      goto end_function_2073;
    }
  }
  struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, cold);
  struct SstoreCosts costs = sstore_costs(entry.orig, entry.curr, value, cold);
  if (costs.state_credit != UINT8_C(0)) {
    struct tuple_uint_64_uint_64_uint_32 result_2_1303 = credit_state_gas_refund(gas, state_gas, state_spill, costs.state_credit);
    gas = result_2_1303.tup0;
    state_gas = result_2_1303.tup1;
    state_spill = result_2_1303.tup2;
  }
  if (gas < costs.execution) {
    struct OpcodeOutcome Failed_result_2_1306 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    opcodeoutcome_8_1472 = Failed_result_2_1306;
    goto end_function_2073;
  }
  gas = gas_sub(gas, costs.execution);
  struct tuple_bool_uint_64_uint_64_uint_32 result_2_1310 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas, state_gas, state_spill, costs.state_charge);
  halt = result_2_1310.tup0;
  gas = result_2_1310.tup1;
  state_gas = result_2_1310.tup2;
  state_spill = result_2_1310.tup3;
  if (halt) {
    struct OpcodeOutcome Failed_result_2_1311 = Failed(OutOfGas);
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Failed_result_2_1311;
  }
  if (costs.refund != GAS_REFUND_ZERO) {
    refund = record_refund(refund, costs.refund);
  }
  if (!eq_u256(entry.curr, value)) {
    storage_update_by_id(context.account_id, context.storage_generation, storage_id, value, entry.orig);
  }
  struct OpcodeOutcome Continue_result_2_1322 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_state_gas) = state_gas;
  (*carried_state_spill) = state_spill;
  (*carried_refund) = refund;
  (*carried_sp) = sp;
  opcodeoutcome_8_1472 = Continue_result_2_1322;
end_function_2073: ;
  return opcodeoutcome_8_1472;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tload(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1473;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1473 = stack_status;
    goto end_function_2055;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_warm_access) {
    struct OpcodeOutcome Failed_result_2_1286 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1473 = Failed_result_2_1286;
    goto end_function_2055;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_warm_access);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = k_tload(carried_address, s);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  struct OpcodeOutcome Continue_result_2_1288 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1473 = Continue_result_2_1288;
end_function_2055: ;
  return opcodeoutcome_8_1473;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tstore(bytes20 carried_address, bool carried_is_static, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1474;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1474 = stack_status;
    goto end_function_2044;
  }
  uint64_t gas = (*carried_gas);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  uint64_t g_after_8_1500;
  struct OpcodeOutcome _8_1408_8_1501;
  g_after_8_1500 = gas;
  _8_1408_8_1501 = guard_static(&g_after_8_1500, carried_is_static);
  gas = g_after_8_1500;
  status = _8_1408_8_1501;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_sp) = sp;
    return status;
  }
  if (gas < G_warm_access) {
    struct OpcodeOutcome Failed_result_2_1281 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1474 = Failed_result_2_1281;
    goto end_function_2044;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_warm_access);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  k_tstore(carried_address, s, v);
  struct OpcodeOutcome Continue_result_2_1283 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1474 = Continue_result_2_1283;
end_function_2044: ;
  return opcodeoutcome_8_1474;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jump(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1475;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1475 = stack_status;
    goto end_function_2030;
  }
  uint64_t gas = (*carried_gas);
  uint32_t pc = (*carried_pc);
  struct OpcodeOutcome status = Continue(UNIT);
  if (gas < G_mid) {
    struct OpcodeOutcome Failed_result_2_1274 = Failed(OutOfGas);
    (*carried_pc) = pc;
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1475 = Failed_result_2_1274;
    goto end_function_2030;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_mid);
  u256 dest = read_stack_word((*carried_sp));
  StackPointer sp = stack_top_retreat_((*carried_sp), UINT8_C(1));
  uint32_t pc_in_after_8_1502;
  uint64_t g_after_8_1503;
  struct OpcodeOutcome _8_1409_8_1504;
  pc_in_after_8_1502 = pc;
  g_after_8_1503 = gas;
  _8_1409_8_1504 = do_jump(&pc_in_after_8_1502, &g_after_8_1503, carried_code, dest);
  pc = pc_in_after_8_1502;
  gas = g_after_8_1503;
  status = _8_1409_8_1504;
  (*carried_pc) = pc;
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1475 = status;
end_function_2030: ;
  return opcodeoutcome_8_1475;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpi(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1476;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1476 = stack_status;
    goto end_function_2017;
  }
  uint64_t gas = (*carried_gas);
  uint32_t pc = (*carried_pc);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  if (gas < G_high) {
    struct OpcodeOutcome Failed_result_2_1269 = Failed(OutOfGas);
    (*carried_pc) = pc;
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    opcodeoutcome_8_1476 = Failed_result_2_1269;
    goto end_function_2017;
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_high);
  u256 dest = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 cond = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bool condition_is_zero = word_is_zero(cond);
  if (condition_is_zero) {
    (*carried_pc) = pc;
    (*carried_gas) = gas;
    (*carried_sp) = sp;
    return status;
  }
  uint32_t pc_in_after_8_1505;
  uint64_t g_after_8_1506;
  struct OpcodeOutcome _8_1409_8_1507;
  pc_in_after_8_1505 = pc;
  g_after_8_1506 = gas;
  _8_1409_8_1507 = do_jump(&pc_in_after_8_1505, &g_after_8_1506, carried_code, dest);
  pc = pc_in_after_8_1505;
  gas = g_after_8_1506;
  status = _8_1409_8_1507;
  (*carried_pc) = pc;
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1476 = status;
end_function_2017: ;
  return opcodeoutcome_8_1476;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pc(uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1477;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1477 = stack_status;
    goto end_function_2006;
  }
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1264 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1477 = Failed_result_2_1264;
    goto end_function_2006;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 next_pc = word_of_source_byte_count((*carried_pc));
  u256 opcode_pc = alu_sub(next_pc, WORD_ONE);
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, opcode_pc);
  struct OpcodeOutcome Continue_result_2_1266 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1477 = Continue_result_2_1266;
end_function_2006: ;
  return opcodeoutcome_8_1477;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gas(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome opcodeoutcome_8_1478;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1478 = stack_status;
    goto end_function_1997;
  }
  if ((*carried_gas) < G_base) {
    struct OpcodeOutcome Failed_result_2_1259 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1478 = Failed_result_2_1259;
    goto end_function_1997;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 gas_word = word_of_nat_byte_count_uint64_t_to_u256(gas);
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, gas_word);
  struct OpcodeOutcome Continue_result_2_1261 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1478 = Continue_result_2_1261;
end_function_1997: ;
  return opcodeoutcome_8_1478;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpdest(uint64_t *restrict carried_gas)
{
  struct OpcodeOutcome opcodeoutcome_8_1479;
  if ((*carried_gas) < G_jumpdest) {
    struct OpcodeOutcome Failed_result_2_1254 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1479 = Failed_result_2_1254;
    goto end_function_1990;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_jumpdest);
  struct OpcodeOutcome Continue_result_2_1256 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1479 = Continue_result_2_1256;
end_function_1990: ;
  return opcodeoutcome_8_1479;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n, u256 v)
{
  struct OpcodeOutcome opcodeoutcome_8_1480;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1480 = stack_status;
    goto end_function_1984;
  }
  uint16_t cost = n == UINT8_C(0) ? G_base : G_verylow;
  if ((*carried_gas) < cost) {
    struct OpcodeOutcome Failed_result_2_1249 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1480 = Failed_result_2_1249;
    goto end_function_1984;
  }
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, v);
  struct OpcodeOutcome Continue_result_2_1251 = Continue(UNIT);
  (*carried_gas) = ((*carried_gas) - (uint64_t)cost);
  (*carried_sp) = sp;
  opcodeoutcome_8_1480 = Continue_result_2_1251;
end_function_1984: ;
  return opcodeoutcome_8_1480;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome opcodeoutcome_8_1481;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1481 = stack_status;
    goto end_function_1975;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1242 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1481 = Failed_result_2_1242;
    goto end_function_1975;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, value);
  struct OpcodeOutcome Continue_result_2_1244 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1481 = Continue_result_2_1244;
end_function_1975: ;
  return opcodeoutcome_8_1481;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome opcodeoutcome_8_1482;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1482 = stack_status;
    goto end_function_1966;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1235 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1482 = Failed_result_2_1235;
    goto end_function_1966;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 top_value = read_stack_word((*carried_sp));
  u256 other = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), other);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  struct OpcodeOutcome Continue_result_2_1237 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1482 = Continue_result_2_1237;
end_function_1966: ;
  return opcodeoutcome_8_1482;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dupn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1483;
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1224 = Failed(InvalidOpcode);
    return Failed_result_2_1224;
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1483 = stack_status;
    goto end_function_1955;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1228 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1483 = Failed_result_2_1228;
    goto end_function_1955;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, value);
  struct OpcodeOutcome Continue_result_2_1230 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  opcodeoutcome_8_1483 = Continue_result_2_1230;
end_function_1955: ;
  return opcodeoutcome_8_1483;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swapn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1484;
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1214 = Failed(InvalidOpcode);
    return Failed_result_2_1214;
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1484 = stack_status;
    goto end_function_1944;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1218 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1484 = Failed_result_2_1218;
    goto end_function_1944;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 top_value = read_stack_word((*carried_sp));
  u256 other = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), other);
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  struct OpcodeOutcome Continue_result_2_1220 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1484 = Continue_result_2_1220;
end_function_1944: ;
  return opcodeoutcome_8_1484;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exchange(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  struct OpcodeOutcome opcodeoutcome_8_1485;
  bool valid_immediate = exchange_immediate_valid(immediate);
  if (!valid_immediate) {
    struct OpcodeOutcome Failed_result_2_1203 = Failed(InvalidOpcode);
    return Failed_result_2_1203;
  }
  struct tuple_uint_16_uint_16 decode_exchange_stack_indices_result_2_1205 = decode_exchange_stack_indices(immediate);
  uint16_t n = decode_exchange_stack_indices_result_2_1205.tup0;
  uint16_t m = decode_exchange_stack_indices_result_2_1205.tup1;
  struct OpcodeOutcome stack_status = guard_stack((*carried_sp), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1485 = stack_status;
    goto end_function_1933;
  }
  if ((*carried_gas) < G_verylow) {
    struct OpcodeOutcome Failed_result_2_1208 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1485 = Failed_result_2_1208;
    goto end_function_1933;
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_verylow);
  u256 first = stack_slot_read_((*carried_sp), n);
  u256 second = stack_slot_read_((*carried_sp), m);
  stack_set((*carried_sp), n, second);
  stack_set((*carried_sp), m, first);
  struct OpcodeOutcome Continue_result_2_1210 = Continue(UNIT);
  (*carried_gas) = gas;
  opcodeoutcome_8_1485 = Continue_result_2_1210;
end_function_1933: ;
  return opcodeoutcome_8_1485;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t n, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome opcodeoutcome_8_1486;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(2) + (uint32_t)n)), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    opcodeoutcome_8_1486 = stack_status;
    goto end_function_1918;
  }
  uint64_t gas = (*carried_gas);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  struct LogTopics topics = LogTopics0(UNIT);
  uint64_t g_after_8_1508;
  struct OpcodeOutcome _8_1408_8_1509;
  g_after_8_1508 = gas;
  _8_1408_8_1509 = guard_static(&g_after_8_1508, carried_is_static);
  gas = g_after_8_1508;
  status = _8_1408_8_1509;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return status;
  }
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  struct tuple_LogTopics_StackPointer result_2_1184 = pop_log_topics(n, sp);
  topics = result_2_1184.tup0;
  sp = result_2_1184.tup1;
  struct GasCharge log_cost = log_gas_cost(n, length_word, gas);
  if (!log_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1187 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1486 = Failed_result_2_1187;
    goto end_function_1918;
  }
  gas = gas_sub(gas, log_cost.cost);
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct OpcodeOutcome Failed_result_2_1192 = Failed(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    opcodeoutcome_8_1486 = Failed_result_2_1192;
    goto end_function_1918;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes memory_slice = evm_memory_slice(data.bytes, data.len);
  struct LogData log_data = LogDataMemory(memory_slice);
  k_log(carried_address, topics, log_data);
  struct OpcodeOutcome Continue_result_2_1196 = Continue(UNIT);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  opcodeoutcome_8_1486 = Continue_result_2_1196;
end_function_1918: ;
  return opcodeoutcome_8_1486;
}

struct FrameStatus execute_stop(void)
{
  struct HaltKind reason = HaltStop(UNIT);
  return Halted(reason);
}

__attribute__((__always_inline__)) struct FrameStatus execute_return(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct FrameStatus framestatus_8_1487;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    struct FrameStatus opcode_frame_status_result_2_1170 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1487 = opcode_frame_status_result_2_1170;
    goto end_function_1900;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct FrameStatus Exceptional_result_2_1174 = Exceptional(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    framestatus_8_1487 = Exceptional_result_2_1174;
    goto end_function_1900;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(data);
  struct HaltKind reason = HaltReturn(output);
  struct FrameStatus Halted_result_2_1178 = Halted(reason);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  framestatus_8_1487 = Halted_result_2_1178;
end_function_1900: ;
  return framestatus_8_1487;
}

__attribute__((__always_inline__)) struct FrameStatus execute_revert(uint64_t carried_state_gas_reservoir, uint32_t memory_base, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct FrameStatus framestatus_8_1488;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    struct FrameStatus opcode_frame_status_result_2_1156 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1488 = opcode_frame_status_result_2_1156;
    goto end_function_1891;
  }
  uint64_t gas = (*carried_gas);
  uint64_t state_gas = (*carried_state_gas);
  uint32_t state_spill = (*carried_state_spill);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    struct FrameStatus Exceptional_result_2_1160 = Exceptional(OutOfGas);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    framestatus_8_1488 = Exceptional_result_2_1160;
    goto end_function_1891;
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork >= Amsterdam) {
    gas = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(gas, state_spill);
    state_gas = carried_state_gas_reservoir;
    state_spill = (uint32_t)STATE_GAS_SPILL_ZERO;
  }
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(data);
  struct HaltKind reason = HaltRevert(output);
  struct FrameStatus Halted_result_2_1166 = Halted(reason);
  (*carried_gas) = gas;
  (*carried_state_gas) = state_gas;
  (*carried_state_spill) = state_spill;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  framestatus_8_1488 = Halted_result_2_1166;
end_function_1891: ;
  return framestatus_8_1488;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_invalid(uint64_t *restrict carried_gas)
{
  (void)carried_gas;
  struct OpcodeOutcome Failed_result_2_1154 = Failed(InvalidOpcode);
  return Failed_result_2_1154;
}

__attribute__((__always_inline__)) struct FrameStatus execute_selfdestruct(bytes20 carried_address, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct FrameStatus framestatus_8_1490;
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    struct FrameStatus opcode_frame_status_result_2_1122 = opcode_frame_status(stack_status);
    (*carried_gas) = (uint64_t)GAS_ZERO;
    framestatus_8_1490 = opcode_frame_status_result_2_1122;
    goto end_function_1868;
  }
  uint64_t gas = (*carried_gas);
  uint64_t state_gas = (*carried_state_gas);
  uint32_t state_spill = (*carried_state_spill);
  __int128 refund = (*carried_refund);
  StackPointer sp = (*carried_sp);
  bool halt = false;
  struct OpcodeOutcome status = Continue(UNIT);
  uint64_t g_after_8_1510;
  struct OpcodeOutcome _8_1408_8_1511;
  g_after_8_1510 = gas;
  _8_1408_8_1511 = guard_static(&g_after_8_1510, carried_is_static);
  gas = g_after_8_1510;
  status = _8_1408_8_1511;
  if (!(status.kind != Kind_Failed)) {
    struct FrameStatus opcode_frame_status_result_2_1126 = opcode_frame_status(status);
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return opcode_frame_status_result_2_1126;
  }
  u256 beneficiary_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 beneficiary = word_to_address(beneficiary_word);
  struct HaltKind halt_reason = HaltSelfDestruct(UNIT);
  struct FrameStatus halt_status = Halted(halt_reason);
  if (fork >= Amsterdam) {
    bool warm = k_account_is_warm(beneficiary);
    uint32_t access_cost;
    uint16_t add_atom_result_2_1138 = ((uint16_t)((uint32_t)G_selfdestruct + (uint32_t)UINT16_C(0)));
    uint16_t result_2_1139 = warm ? G_zero : G_amsterdam_cold_account_access;
    access_cost = ((uint32_t)add_atom_result_2_1138 + (uint32_t)result_2_1139);
    if (gas < access_cost) {
      struct FrameStatus Exceptional_result_2_1130 = Exceptional(OutOfGas);
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      framestatus_8_1490 = Exceptional_result_2_1130;
      goto end_function_1868;
    }
    k_account_mark_warm(beneficiary);
    u256 bal = k_get_balance(carried_address);
    bool nonzero_balance = word_nonzero(bal);
    bool beneficiary_empty = k_account_is_empty(beneficiary);
    bool creates_account = (bool)(nonzero_balance && beneficiary_empty);
    uint32_t execution_cost = creates_account ? (access_cost + (uint32_t)G_amsterdam_account_write) : access_cost;
    if (gas < execution_cost) {
      struct FrameStatus Exceptional_result_2_1133 = Exceptional(OutOfGas);
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      framestatus_8_1490 = Exceptional_result_2_1133;
      goto end_function_1868;
    }
    gas = gas_sub_uint64_t_uint32_t_to_uint64_t(gas, execution_cost);
    if (creates_account) {
      struct tuple_bool_uint_64_uint_64_uint_32 result_2_1135 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas, state_gas, state_spill, G_amsterdam_state_new_account);
      halt = result_2_1135.tup0;
      gas = result_2_1135.tup1;
      state_gas = result_2_1135.tup2;
      state_spill = result_2_1135.tup3;
    }
    if (halt) {
      struct FrameStatus Exceptional_result_2_1136 = Exceptional(OutOfGas);
      (*carried_gas) = gas;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional_result_2_1136;
    }
    k_transfer(carried_address, beneficiary, bal);
    bool created = k_was_created(carried_address);
    if (created) {
      k_selfdestruct(carried_address);
    }
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    framestatus_8_1490 = halt_status;
  } else {
    u256 bal_3_1828 = k_get_balance(carried_address);
    bool warm_3_1829 = k_account_is_warm(beneficiary);
    if (gas < G_selfdestruct) {
      struct FrameStatus Exceptional_result_2_1141 = Exceptional(OutOfGas);
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      framestatus_8_1490 = Exceptional_result_2_1141;
      goto end_function_1868;
    }
    gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_selfdestruct);
    if (!warm_3_1829) {
      if (gas < G_cold_account) {
        struct FrameStatus Exceptional_result_2_1145 = Exceptional(OutOfGas);
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_state_gas) = state_gas;
        (*carried_state_spill) = state_spill;
        (*carried_refund) = refund;
        (*carried_sp) = sp;
        framestatus_8_1490 = Exceptional_result_2_1145;
        goto end_function_1868;
      }
      gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_cold_account);
    }
    k_account_mark_warm(beneficiary);
    bool nonzero_balance_3_1830 = word_nonzero(bal_3_1828);
    bool beneficiary_empty_3_1831 = k_account_is_empty(beneficiary);
    if (nonzero_balance_3_1830 && beneficiary_empty_3_1831) {
      if (gas < G_newaccount) {
        struct FrameStatus Exceptional_result_2_1149 = Exceptional(OutOfGas);
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_state_gas) = state_gas;
        (*carried_state_spill) = state_spill;
        (*carried_refund) = refund;
        (*carried_sp) = sp;
        framestatus_8_1490 = Exceptional_result_2_1149;
        goto end_function_1868;
      }
      gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_newaccount);
    }
    bool is_selfdestructed = k_is_selfdestructed(carried_address);
    bool lt_int_result_2_1151 = (bool)(fork < London);
    if (lt_int_result_2_1151 && !is_selfdestructed) {
      refund = record_refund___int128_uint16_t_to___int128(refund, R_selfdestruct_pre_london);
    }
    k_transfer(carried_address, beneficiary, bal_3_1828);
    if (fork < Cancun) {
      k_zero_balance(carried_address);
      k_selfdestruct(carried_address);
    } else {
      bool created_3_1832 = k_was_created(carried_address);
      if (created_3_1832) {
        k_zero_balance(carried_address);
        k_selfdestruct(carried_address);
      }
    }
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    framestatus_8_1490 = halt_status;
  }
end_function_1868: ;
  return framestatus_8_1490;
}

struct OpcodeOutcome guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint16_t inputs, uint8_t outputs)
{
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else {
    bool result_2_2018 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs)));
    stack_validation = result_2_2018 ? StackOverflowFailure : StackValid;
  }
  switch (stack_validation) {
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
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else {
    bool result_2_2018 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
    stack_validation = result_2_2018 ? StackOverflowFailure : StackValid;
  }
  switch (stack_validation) {
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
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else {
    bool result_2_2018 = (bool)(STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs)) + (int32_t)(int16_t)outputs)));
    stack_validation = result_2_2018 ? StackOverflowFailure : StackValid;
  }
  switch (stack_validation) {
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
  enum StackValidation stack_validation;
  bool result_2_2018 = (bool)(STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)));
  stack_validation = result_2_2018 ? StackOverflowFailure : StackValid;
  switch (stack_validation) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

