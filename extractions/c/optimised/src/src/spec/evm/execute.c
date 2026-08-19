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
    (*g) = (uint64_t)GAS_ZERO;
    return Failed(WriteProtection);
  }
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome do_jump(uint32_t *restrict pc_in, uint64_t *restrict g, struct CodeFields frame_code, u256 destination_value)
{
  if (u256_lt_u64(destination_value, (frame_code_len(frame_code)))) {
    uint32_t destination = (uint32_t)u256_to_u64_unchecked(destination_value);
    if (frame_jumpdest_valid(frame_code, destination)) {
      (*pc_in) = destination;
      return Continue(UNIT);
    }
    (*g) = (uint64_t)GAS_ZERO;
    return Failed(InvalidJump);
  }
  (*g) = (uint64_t)GAS_ZERO;
  return Failed(InvalidJump);
}

struct OpcodeOutcome guard_stack(StackPointer carried_sp, uint16_t inputs, uint16_t outputs)
{
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs))) {
    stack_validation = StackOverflowFailure;
  } else {
    stack_validation = StackValid;
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

StackPointer pop_log_topics(uint8_t count, StackPointer sp_in, struct LogTopics *restrict logtopics_8_1374)
{
  StackPointer sp = sp_in;
  if (count == UINT8_C(0)) {
    (*logtopics_8_1374) = LogTopics0(UNIT);
    return sp;
  }
  if (count == UINT8_C(1)) {
    u256 t0 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    (*logtopics_8_1374) = LogTopics1(t0);
    return sp;
  }
  if (count == UINT8_C(2)) {
    u256 t0_3_2902 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    (*logtopics_8_1374) = LogTopics2(((struct tuple_u256_u256){.tup0 = t0_3_2902, .tup1 = t1}));
    return sp;
  }
  if (count == UINT8_C(3)) {
    u256 t0_3_2904 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t1_3_2905 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    u256 t2 = stack_slot_read(sp, UINT16_C(0));
    sp = stack_top_retreat(sp, UINT16_C(1));
    (*logtopics_8_1374) = LogTopics3(((struct tuple_u256_u256_u256){.tup0 = t0_3_2904, .tup1 = t1_3_2905, .tup2 = t2}));
    return sp;
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
    (*logtopics_8_1374) = LogTopics4(((struct tuple_u256_u256_u256_u256){.tup0 = t0_3_2907, .tup1 = t1_3_2908, .tup2 = t2_3_2909, .tup3 = t3}));
    return sp;
  }
  (*logtopics_8_1374) = LogTopics0(UNIT);
  return sp;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_add(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_add(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mul(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_mul(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sub(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_sub(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_div(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_div(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sdiv(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_sdiv(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_mod(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_smod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_smod(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_addmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_mid) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 n = read_stack_word(sp);
  write_stack_word(sp, (alu_addmod(a, b, n)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_mid);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mulmod(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_mid) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 n = read_stack_word(sp);
  write_stack_word(sp, (alu_mulmod(a, b, n)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_mid);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 e = read_stack_word(sp);
  uint64_t gas_cost = exp_gas(e);
  if ((*carried_gas) < gas_cost) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  write_stack_word(sp, (alu_exp(a, e)));
  (*carried_gas) = ((*carried_gas) - gas_cost);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_signextend(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 bi = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  write_stack_word(sp, (alu_signextend(bi, v)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_lt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_lt(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_gt(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_slt(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sgt(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_sgt(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_eq(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_eq(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_iszero(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word((*carried_sp));
  write_stack_word((*carried_sp), (alu_iszero(a)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_and(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_and(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_or(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_or(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_xor(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 b = read_stack_word(sp);
  write_stack_word(sp, (alu_xor(a, b)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_not(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 a = read_stack_word((*carried_sp));
  write_stack_word((*carried_sp), (alu_not(a)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_byte(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 i = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 x = read_stack_word(sp);
  write_stack_word(sp, (alu_byte(i, x)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shl(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  write_stack_word(sp, (alu_shl(s, v)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_shr(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  write_stack_word(sp, (alu_shr(s, v)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sar(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  write_stack_word(sp, (alu_sar(s, v)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_clz(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 x = read_stack_word((*carried_sp));
  write_stack_word((*carried_sp), (alu_clz(x)));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_keccak256(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, keccak_cost.cost);
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  u256 digest = mem_keccak(memory_base, memory, access.range);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, digest);
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_address(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 address_word = address_to_word(carried_address);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, address_word);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_origin(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 origin = k_env(F_Origin);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, origin);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_caller(bytes20 carried_caller, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 caller = address_to_word(carried_caller);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, caller);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_callvalue(u256 carried_value, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, carried_value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gasprice(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 gas_price = k_env(F_GasPrice);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, gas_price);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatasize(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  uint32_t input_length = calldata_slice_length(carried_calldata);
  u256 length_word = word_of_source_byte_count(input_length);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, length_word);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldataload(struct CalldataSlice carried_calldata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = calldata_slice_load_word_offset(carried_calldata, offset_word);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_calldatacopy(struct CalldataSlice carried_calldata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  uint32_t destination = memory_absolute(memory_base, range.off);
  calldata_slice_copy_word_offset(carried_calldata, destination, source_word, range.len);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codesize(struct CodeFields carried_code, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  uint32_t code_length = frame_code_len(carried_code);
  u256 length_word = word_of_source_byte_count(code_length);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, length_word);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_codecopy(struct CodeFields carried_code, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  Bytes bytes = code_bytes(carried_code);
  uint32_t destination = memory_absolute(memory_base, range.off);
  code_slice_copy_word_offset(bytes, destination, source_word, range.len);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_balance(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  if ((*carried_gas) < gas_cost) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  k_account_mark_warm(a);
  u256 balance = k_get_balance(a);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, balance);
  (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_selfbalance(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_low) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 balance = k_get_balance(carried_address);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, balance);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_low);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodesize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  if ((*carried_gas) < ((uint32_t)access_cost + (uint32_t)read_cost)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub_uint64_t_uint32_t_to_uint64_t((*carried_gas), ((uint32_t)access_cost + (uint32_t)read_cost));
  k_account_mark_warm(a);
  uint32_t code_size = k_get_code_size(a);
  u256 size_word = word_of_source_byte_count(code_size);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size_word);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodecopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(4), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint32_t_to_uint64_t(gas, ((uint32_t)access_cost + (uint32_t)read_cost));
  struct GasCharge copy_cost = copy_gas_cost(length_word, gas);
  if (!copy_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t requested_height = memory_requested_height(destination_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  struct MemoryRangeFields range = access.range;
  k_account_mark_warm(a);
  uint32_t destination = memory_absolute(memory_base, range.off);
  k_code_copy(a, destination, source_word, range.len);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_extcodehash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  u256 address_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 a = word_to_address(address_word);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  if ((*carried_gas) < gas_cost) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  k_account_mark_warm(a);
  bytes32 code_hash = k_get_codehash(a);
  u256 hash_word = hash_to_word(code_hash);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, hash_word);
  (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatasize(Bytes carried_returndata, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  uint32_t return_data_size = returndata_size(carried_returndata);
  u256 size_word = word_of_source_byte_count(return_data_size);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size_word);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_returndatacopy(Bytes carried_returndata, uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 destination_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 source_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 length_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t available = returndata_size(carried_returndata);
  if (!u64_lt_u256(available, source_word)) {
    if (!u64_lt_u256((returndata_remaining(available, (uint32_t)u256_to_u64_unchecked(source_word))), length_word)) {
      struct GasCharge copy_cost = copy_gas_cost_u256_uint64_t_to_struct_GasCharge(length_word, gas);
      if (!copy_cost.affordable) {
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_sp) = sp;
        (*carried_memory_height) = memory;
        return Failed(OutOfGas);
      }
      gas = gas_sub(gas, copy_cost.cost);
      uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t_variant_2(destination_word, length_word);
      struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
      if (!expansion_cost.affordable) {
        (*carried_gas) = (uint64_t)GAS_ZERO;
        (*carried_sp) = sp;
        (*carried_memory_height) = memory;
        return Failed(OutOfGas);
      }
      (*carried_gas) = gas_sub(gas, expansion_cost.cost);
      struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields_variant_2(destination_word, length_word);
      (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
      uint32_t destination = memory_absolute(memory_base, access.range.off);
      returndata_copy(carried_returndata, destination, (uint32_t)u256_to_u64_unchecked(source_word), (uint32_t)u256_to_u64_unchecked(length_word));
      (*carried_sp) = sp;
      return Continue(UNIT);
    }
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(InvalidOpcode);
  }
  (*carried_gas) = (uint64_t)GAS_ZERO;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  return Failed(InvalidOpcode);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blockhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < UINT8_C(20)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 block_number = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes32 block_hash = k_blockhash(block_number);
  u256 hash_word = hash_to_word(block_hash);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, hash_word);
  (*carried_gas) = ((*carried_gas) - UINT64_C(20));
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_coinbase(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 coinbase = k_env(F_Coinbase);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, coinbase);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_timestamp(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 timestamp = k_env(F_Timestamp);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, timestamp);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_number(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 block_number = k_env(F_Number);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, block_number);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_slotnum(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 slot_number = k_env(F_SlotNumber);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, slot_number);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_prevrandao(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 prev_randao = k_env(F_PrevRandao);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, prev_randao);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gaslimit(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 gas_limit = k_env(F_GasLimit);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, gas_limit);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_chainid(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 chain_id = k_env(F_ChainId);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, chain_id);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_basefee(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 base_fee = k_env(F_BaseFee);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, base_fee);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobbasefee(u256 blob_fee, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, blob_fee);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_blobhash(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 index = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 blob_hash = k_blobhash(index);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, blob_hash);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pop(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  read_stack_word(sp);
  (*carried_sp) = stack_top_retreat_(sp, UINT8_C(1));
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mload(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  u256 value = mem_load(memory_base, access.range.off);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  mem_store(memory_base, access.range.off, v);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mstore8(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_verylow);
  u256 offset_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint64_t requested_height = memory_requested_height(offset_word, WORD_ONE);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, WORD_ONE);
  (*carried_memory_height) = expand_memory(memory_base, memory, access.requested_height);
  mem_store_byte(memory_base, access.range.off, v);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_msize(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  uint32_t high_water = memory_high_water((*carried_memory_height));
  uint32_t words = memory_word_count_uint32_t_to_uint32_t(high_water);
  u256 size = word_of_nat_byte_count_uint64_t_to_u256(((uint64_t)words * UINT64_C(32)));
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, size);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_mcopy(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(3), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  if (gas < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, copy_cost.cost);
  uint64_t destination_requested_height = memory_requested_height(destination_word, length_word);
  uint64_t source_requested_height = memory_requested_height(source_word, length_word);
  uint64_t requested_height = destination_requested_height < source_requested_height ? source_requested_height : destination_requested_height;
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields destination = memory_access(destination_word, length_word);
  struct MemoryAccessFields source = memory_access(source_word, length_word);
  uint32_t materialized_required_size = destination.requested_height < source.requested_height ? source.requested_height : destination.requested_height;
  (*carried_memory_height) = expand_memory(memory_base, memory, materialized_required_size);
  mem_mcopy(memory_base, destination.range.off, source.range.off, destination.range.len);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  return Continue(UNIT);
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
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  u256 slot = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  uint16_t gas_cost = sload_cost(warm);
  if ((*carried_gas) < gas_cost) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, (bool)(!warm));
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, entry.curr);
  (*carried_gas) = ((*carried_gas) - (uint64_t)gas_cost);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_sstore(struct AccountExecutionContext context, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  uint64_t state_gas = (*carried_state_gas);
  uint32_t state_spill = (*carried_state_spill);
  __int128 refund = (*carried_refund);
  StackPointer sp = (*carried_sp);
  bool halt = false;
  struct OpcodeOutcome status = Continue(UNIT);
  uint64_t g_after_8_1588 = gas;
  struct OpcodeOutcome _8_1372_8_1589 = guard_static(&g_after_8_1588, carried_is_static);
  gas = g_after_8_1588;
  status = _8_1372_8_1589;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return status;
  }
  if ((fork < Amsterdam) && (gas <= G_callstipend)) {
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 slot = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  uint32_t storage_id = storage_resolve_slot(context.storage_begin, context.storage_change_count, context.storage_count, slot);
  bool warm = storage_id_is_warm(storage_id);
  bool cold = (bool)(!warm);
  if (fork >= Amsterdam) {
    if (gas < (sstore_sentry_cost(cold))) {
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Failed(OutOfGas);
    }
  }
  struct StorageValue entry = storage_load_by_id(context.account_id, context.storage_generation, storage_id, cold);
  struct SstoreCosts costs = sstore_costs(entry.orig, entry.curr, value, cold);
  if (costs.state_credit != UINT8_C(0)) {
    uint64_t _8_1365_8_1594;
    uint64_t _8_1366_8_1595;
    uint32_t _8_1367_8_1596 = credit_state_gas_refund(gas, state_gas, state_spill, costs.state_credit, &_8_1365_8_1594, &_8_1366_8_1595);
    gas = _8_1365_8_1594;
    state_gas = _8_1366_8_1595;
    state_spill = _8_1367_8_1596;
  }
  if (gas < costs.execution) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, costs.execution);
  bool _8_1519_8_1590;
  uint64_t _8_1520_8_1591;
  uint64_t _8_1521_8_1592;
  uint32_t _8_1522_8_1593 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas, state_gas, state_spill, costs.state_charge, &_8_1519_8_1590, &_8_1520_8_1591, &_8_1521_8_1592);
  halt = _8_1519_8_1590;
  gas = _8_1520_8_1591;
  state_gas = _8_1521_8_1592;
  state_spill = _8_1522_8_1593;
  if (halt) {
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  if (costs.refund != GAS_REFUND_ZERO) {
    refund = record_refund(refund, costs.refund);
  }
  if (!eq_u256(entry.curr, value)) {
    storage_update_by_id(context.account_id, context.storage_generation, storage_id, value, entry.orig);
  }
  (*carried_gas) = gas;
  (*carried_state_gas) = state_gas;
  (*carried_state_spill) = state_spill;
  (*carried_refund) = refund;
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tload(bytes20 carried_address, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  StackPointer sp = (*carried_sp);
  if ((*carried_gas) < G_warm_access) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 value = k_tload(carried_address, s);
  sp = stack_top_advance_(sp, UINT8_C(1));
  write_stack_word(sp, value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_warm_access);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_tstore(bytes20 carried_address, bool carried_is_static, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  uint64_t g_after_8_1597 = gas;
  struct OpcodeOutcome _8_1372_8_1598 = guard_static(&g_after_8_1597, carried_is_static);
  gas = g_after_8_1597;
  status = _8_1372_8_1598;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_sp) = sp;
    return status;
  }
  if (gas < G_warm_access) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_warm_access);
  u256 s = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 v = read_stack_word(sp);
  (*carried_sp) = stack_top_retreat_(sp, UINT8_C(1));
  k_tstore(carried_address, s, v);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jump(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  uint32_t pc = (*carried_pc);
  struct OpcodeOutcome status = Continue(UNIT);
  if (gas < G_mid) {
    (*carried_pc) = pc;
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_mid);
  u256 dest = read_stack_word((*carried_sp));
  (*carried_sp) = stack_top_retreat_((*carried_sp), UINT8_C(1));
  uint32_t pc_in_after_8_1599 = pc;
  uint64_t g_after_8_1600 = gas;
  struct OpcodeOutcome _8_1373_8_1601 = do_jump(&pc_in_after_8_1599, &g_after_8_1600, carried_code, dest);
  pc = pc_in_after_8_1599;
  gas = g_after_8_1600;
  status = _8_1373_8_1601;
  (*carried_pc) = pc;
  (*carried_gas) = gas;
  return status;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpi(struct CodeFields carried_code, uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  uint32_t pc = (*carried_pc);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  if (gas < G_high) {
    (*carried_pc) = pc;
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    return Failed(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_high);
  u256 dest = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  u256 cond = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  if (word_is_zero(cond)) {
    (*carried_pc) = pc;
    (*carried_gas) = gas;
    (*carried_sp) = sp;
    return status;
  }
  uint32_t pc_in_after_8_1602 = pc;
  uint64_t g_after_8_1603 = gas;
  struct OpcodeOutcome _8_1373_8_1604 = do_jump(&pc_in_after_8_1602, &g_after_8_1603, carried_code, dest);
  pc = pc_in_after_8_1602;
  gas = g_after_8_1603;
  status = _8_1373_8_1604;
  (*carried_pc) = pc;
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  return status;
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_pc(uint32_t *restrict carried_pc, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 next_pc = word_of_source_byte_count((*carried_pc));
  u256 opcode_pc = alu_sub(next_pc, WORD_ONE);
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, opcode_pc);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_base);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_gas(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_base) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  uint64_t gas = ((*carried_gas) - (uint64_t)G_base);
  u256 gas_word = word_of_nat_byte_count_uint64_t_to_u256(gas);
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, gas_word);
  (*carried_gas) = gas;
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_jumpdest(uint64_t *restrict carried_gas)
{
  if ((*carried_gas) < G_jumpdest) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_jumpdest);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_push(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n, u256 v)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_3((*carried_sp), UINT8_C(0), UINT8_C(1));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint16_t cost = n == UINT8_C(0) ? G_base : G_verylow;
  if ((*carried_gas) < cost) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, v);
  (*carried_gas) = ((*carried_gas) - (uint64_t)cost);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dup(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swap(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint8_t n)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 top_value = read_stack_word((*carried_sp));
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), (stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n)));
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_dupn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  if (!(deep_stack_immediate_valid(immediate))) {
    return Failed(InvalidOpcode);
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), n, ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 value = stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  StackPointer sp = stack_top_advance_((*carried_sp), UINT8_C(1));
  write_stack_word(sp, value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  (*carried_sp) = sp;
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_swapn(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  if (!(deep_stack_immediate_valid(immediate))) {
    return Failed(InvalidOpcode);
  }
  uint8_t n = decode_single_stack_index(immediate);
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)), ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)n)));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 top_value = read_stack_word((*carried_sp));
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), UINT8_C(0), (stack_slot_read_StackPointer_uint8_t_to_u256((*carried_sp), n)));
  stack_set_StackPointer_uint8_t_u256_to_unit((*carried_sp), n, top_value);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_exchange(uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint64_t immediate)
{
  if (!(exchange_immediate_valid(immediate))) {
    return Failed(InvalidOpcode);
  }
  uint16_t n;
  uint16_t m = decode_exchange_stack_indices(immediate, &n);
  struct OpcodeOutcome stack_status = guard_stack((*carried_sp), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))), ((uint16_t)((uint32_t)m + (uint32_t)UINT16_C(1))));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  if ((*carried_gas) < G_verylow) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return Failed(OutOfGas);
  }
  u256 first = stack_slot_read_((*carried_sp), n);
  u256 second = stack_slot_read_((*carried_sp), m);
  stack_set((*carried_sp), n, second);
  stack_set((*carried_sp), m, first);
  (*carried_gas) = ((*carried_gas) - (uint64_t)G_verylow);
  return Continue(UNIT);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_log(bytes20 carried_address, bool carried_is_static, uint32_t memory_base, uint8_t n, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome_variant_2((*carried_sp), ((uint8_t)((uint32_t)UINT8_C(2) + (uint32_t)n)), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return stack_status;
  }
  uint64_t gas = (*carried_gas);
  struct OpcodeOutcome status = Continue(UNIT);
  StackPointer sp = (*carried_sp);
  uint32_t memory = (*carried_memory_height);
  struct LogTopics topics = LogTopics0(UNIT);
  uint64_t g_after_8_1605 = gas;
  struct OpcodeOutcome _8_1372_8_1606 = guard_static(&g_after_8_1605, carried_is_static);
  gas = g_after_8_1605;
  status = _8_1372_8_1606;
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
  struct LogTopics _8_1374_8_1607;
  StackPointer _8_1375_8_1608 = pop_log_topics(n, sp, &_8_1374_8_1607);
  topics = _8_1374_8_1607;
  sp = _8_1375_8_1608;
  struct GasCharge log_cost = log_gas_cost(n, length_word, gas);
  if (!log_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  gas = gas_sub(gas, log_cost.cost);
  uint64_t requested_height = memory_requested_height(offset_word, length_word);
  struct GasCharge expansion_cost = memory_expansion_gas_cost(memory, requested_height, gas);
  if (!expansion_cost.affordable) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Failed(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes memory_slice = evm_memory_slice(data.bytes, data.len);
  k_log(carried_address, topics, (LogDataMemory(memory_slice)));
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  return Continue(UNIT);
}

struct FrameStatus execute_stop(void)
{
  return Halted((HaltStop(UNIT)));
}

__attribute__((__always_inline__)) struct FrameStatus execute_return(uint32_t memory_base, uint64_t *restrict carried_gas, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return opcode_frame_status(stack_status);
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Exceptional(OutOfGas);
  }
  (*carried_gas) = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(data);
  struct HaltKind reason = HaltReturn(output);
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  return Halted(reason);
}

__attribute__((__always_inline__)) struct FrameStatus execute_revert(uint64_t carried_state_gas_reservoir, uint32_t memory_base, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, StackPointer *restrict carried_sp, uint32_t *restrict carried_memory_height)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(2), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return opcode_frame_status(stack_status);
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
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_sp) = sp;
    (*carried_memory_height) = memory;
    return Exceptional(OutOfGas);
  }
  gas = gas_sub(gas, expansion_cost.cost);
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  memory = expand_memory(memory_base, memory, access.requested_height);
  if (k_execution_profile.protocol.fork >= Amsterdam) {
    gas = conserved_gas_add_uint64_t_uint32_t_to_uint64_t(gas, state_spill);
    state_gas = carried_state_gas_reservoir;
    state_spill = (uint32_t)STATE_GAS_SPILL_ZERO;
  }
  Bytes data = active_memory_slice(memory_base, memory, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(data);
  struct HaltKind reason = HaltRevert(output);
  (*carried_gas) = gas;
  (*carried_state_gas) = state_gas;
  (*carried_state_spill) = state_spill;
  (*carried_sp) = sp;
  (*carried_memory_height) = memory;
  return Halted(reason);
}

__attribute__((__always_inline__)) struct OpcodeOutcome execute_invalid(uint64_t *restrict carried_gas)
{
  (void)carried_gas;
  return Failed(InvalidOpcode);
}

__attribute__((__always_inline__)) struct FrameStatus execute_selfdestruct(bytes20 carried_address, uint8_t fork, bool carried_is_static, uint64_t *restrict carried_gas, uint64_t *restrict carried_state_gas, uint32_t *restrict carried_state_spill, __int128 *restrict carried_refund, StackPointer *restrict carried_sp)
{
  struct OpcodeOutcome stack_status = guard_stack_StackPointer_uint8_t_uint8_t_to_struct_OpcodeOutcome((*carried_sp), UINT8_C(1), UINT8_C(0));
  if (!(stack_status.kind != Kind_Failed)) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    return opcode_frame_status(stack_status);
  }
  uint64_t gas = (*carried_gas);
  uint64_t state_gas = (*carried_state_gas);
  uint32_t state_spill = (*carried_state_spill);
  __int128 refund = (*carried_refund);
  StackPointer sp = (*carried_sp);
  bool halt = false;
  struct OpcodeOutcome status = Continue(UNIT);
  uint64_t g_after_8_1609 = gas;
  struct OpcodeOutcome _8_1372_8_1610 = guard_static(&g_after_8_1609, carried_is_static);
  gas = g_after_8_1609;
  status = _8_1372_8_1610;
  if (!(status.kind != Kind_Failed)) {
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return opcode_frame_status(status);
  }
  u256 beneficiary_word = read_stack_word(sp);
  sp = stack_top_retreat_(sp, UINT8_C(1));
  bytes20 beneficiary = word_to_address(beneficiary_word);
  struct HaltKind halt_reason = HaltSelfDestruct(UNIT);
  struct FrameStatus halt_status = Halted(halt_reason);
  if (fork >= Amsterdam) {
    bool warm = k_account_is_warm(beneficiary);
    uint16_t cold_access_cost = warm ? G_zero : G_amsterdam_cold_account_access;
    uint32_t access_cost = ((uint32_t)G_selfdestruct + (uint32_t)cold_access_cost);
    if (gas < access_cost) {
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional(OutOfGas);
    }
    k_account_mark_warm(beneficiary);
    u256 bal = k_get_balance(carried_address);
    bool nonzero_balance = word_nonzero(bal);
    bool beneficiary_empty = k_account_is_empty(beneficiary);
    bool creates_account = (bool)(nonzero_balance && beneficiary_empty);
    uint32_t execution_cost = creates_account ? (access_cost + (uint32_t)G_amsterdam_account_write) : access_cost;
    if (gas < execution_cost) {
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional(OutOfGas);
    }
    gas = gas_sub_uint64_t_uint32_t_to_uint64_t(gas, execution_cost);
    if (creates_account) {
      bool _8_1511_8_1611;
      uint64_t _8_1512_8_1612;
      uint64_t _8_1513_8_1613;
      uint32_t _8_1514_8_1614 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas, state_gas, state_spill, G_amsterdam_state_new_account, &_8_1511_8_1611, &_8_1512_8_1612, &_8_1513_8_1613);
      halt = _8_1511_8_1611;
      gas = _8_1512_8_1612;
      state_gas = _8_1513_8_1613;
      state_spill = _8_1514_8_1614;
    }
    if (halt) {
      (*carried_gas) = gas;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional(OutOfGas);
    }
    k_transfer(carried_address, beneficiary, bal);
    if (k_was_created(carried_address)) {
      k_selfdestruct(carried_address);
    }
    (*carried_gas) = gas;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return halt_status;
  }
  u256 bal_3_1830 = k_get_balance(carried_address);
  bool warm_3_1831 = k_account_is_warm(beneficiary);
  if (gas < G_selfdestruct) {
    (*carried_gas) = (uint64_t)GAS_ZERO;
    (*carried_state_gas) = state_gas;
    (*carried_state_spill) = state_spill;
    (*carried_refund) = refund;
    (*carried_sp) = sp;
    return Exceptional(OutOfGas);
  }
  gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_selfdestruct);
  if (!warm_3_1831) {
    if (gas < G_cold_account) {
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional(OutOfGas);
    }
    gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_cold_account);
  }
  k_account_mark_warm(beneficiary);
  bool nonzero_balance_3_1832 = word_nonzero(bal_3_1830);
  bool beneficiary_empty_3_1833 = k_account_is_empty(beneficiary);
  if (nonzero_balance_3_1832 && beneficiary_empty_3_1833) {
    if (gas < G_newaccount) {
      (*carried_gas) = (uint64_t)GAS_ZERO;
      (*carried_state_gas) = state_gas;
      (*carried_state_spill) = state_spill;
      (*carried_refund) = refund;
      (*carried_sp) = sp;
      return Exceptional(OutOfGas);
    }
    gas = gas_sub_uint64_t_uint16_t_to_uint64_t(gas, G_newaccount);
  }
  bool is_selfdestructed = k_is_selfdestructed(carried_address);
  if ((fork < London) && !is_selfdestructed) {
    refund = record_refund___int128_uint16_t_to___int128(refund, R_selfdestruct_pre_london);
  }
  k_transfer(carried_address, beneficiary, bal_3_1830);
  if (fork < Cancun) {
    k_zero_balance(carried_address);
    k_selfdestruct(carried_address);
  } else {
    if (k_was_created(carried_address)) {
      k_zero_balance(carried_address);
      k_selfdestruct(carried_address);
    }
  }
  (*carried_gas) = gas;
  (*carried_state_gas) = state_gas;
  (*carried_state_spill) = state_spill;
  (*carried_refund) = refund;
  (*carried_sp) = sp;
  return halt_status;
}

struct OpcodeOutcome guard_stack_StackPointer_uint16_t_uint8_t_to_struct_OpcodeOutcome(StackPointer carried_sp, uint16_t inputs, uint8_t outputs)
{
  enum StackValidation stack_validation;
  if (carried_sp.height < inputs) {
    stack_validation = StackUnderflowFailure;
  } else if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)inputs)) + (int32_t)(int16_t)outputs))) {
    stack_validation = StackOverflowFailure;
  } else {
    stack_validation = StackValid;
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
  } else if (STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs))) {
    stack_validation = StackOverflowFailure;
  } else {
    stack_validation = StackValid;
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
  } else if (STACK_LIMIT < (uint16_t)((int16_t)((int32_t)(int16_t)((uint16_t)((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs)) + (int32_t)(int16_t)outputs))) {
    stack_validation = StackOverflowFailure;
  } else {
    stack_validation = StackValid;
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
  enum StackValidation stack_validation = STACK_LIMIT < ((uint16_t)(((uint32_t)carried_sp.height - (uint32_t)(uint16_t)inputs) + (uint32_t)(uint16_t)outputs)) ? StackOverflowFailure : StackValid;
  switch (stack_validation) {
  case StackValid:
    return Continue(UNIT);
  case StackUnderflowFailure:
    return Failed(StackUnderflow);
  case StackOverflowFailure:
    return Failed(StackOverflow);
  }
}

