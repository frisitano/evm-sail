#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes20 self_addr(void)
{
  return message.address;
}

struct tuple_bool_uint_64 guard_static(uint64_t g)
{
  bool result_2_1579 = message.is_static;
  if (result_2_1579) {
    uint64_t exc_halt_result_2_1580 = exc_halt(g, WriteProtection);
    return ((struct tuple_bool_uint_64){.tup0 = true, .tup1 = exc_halt_result_2_1580});
  }
  return ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = g});
}

struct tuple_uint_32_uint_64 do_jump(uint32_t pc_in, uint64_t g, u256 destination_value)
{
  uint32_t code_length = frame_code_len();
  if (u256_lt_u64(destination_value, code_length)) {
    uint32_t destination = (uint32_t)u256_to_u64_unchecked(destination_value);
    bool valid_destination = frame_jumpdest_valid(destination);
    if (valid_destination) {
      return ((struct tuple_uint_32_uint_64){.tup0 = destination, .tup1 = g});
    }
    uint64_t exc_halt_result_2_1577 = exc_halt(g, InvalidJump);
    return ((struct tuple_uint_32_uint_64){.tup0 = pc_in, .tup1 = exc_halt_result_2_1577});
  }
  uint64_t exc_halt_result_2_1578 = exc_halt(g, InvalidJump);
  return ((struct tuple_uint_32_uint_64){.tup0 = pc_in, .tup1 = exc_halt_result_2_1578});
}

struct tuple_LogTopics_bits_64 pop_log_topics(uint8_t count, uint64_t top)
{
  if (count == UINT8_C(0)) {
    struct LogTopics LogTopics0_result_2_1560 = LogTopics0(UNIT);
    return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics0_result_2_1560, .tup1 = top});
  }
  if (count == UINT8_C(1)) {
    struct tuple_u256_bits_64 pop_result_2_1561 = pop(top);
    struct LogTopics LogTopics1_result_2_1562 = LogTopics1(pop_result_2_1561.tup0);
    return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics1_result_2_1562, .tup1 = pop_result_2_1561.tup1});
  }
  if (count == UINT8_C(2)) {
    struct tuple_u256_bits_64 pop_result_2_1563 = pop(top);
    struct tuple_u256_bits_64 pop_result_2_1564 = pop(pop_result_2_1563.tup1);
    struct LogTopics LogTopics2_result_2_1565 = LogTopics2(((struct tuple_u256_u256){.tup0 = pop_result_2_1563.tup0, .tup1 = pop_result_2_1564.tup0}));
    return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics2_result_2_1565, .tup1 = pop_result_2_1564.tup1});
  }
  if (count == UINT8_C(3)) {
    struct tuple_u256_bits_64 pop_result_2_1566 = pop(top);
    struct tuple_u256_bits_64 pop_result_2_1567 = pop(pop_result_2_1566.tup1);
    struct tuple_u256_bits_64 pop_result_2_1568 = pop(pop_result_2_1567.tup1);
    struct LogTopics LogTopics3_result_2_1569 = LogTopics3(((struct tuple_u256_u256_u256){.tup0 = pop_result_2_1566.tup0, .tup1 = pop_result_2_1567.tup0, .tup2 = pop_result_2_1568.tup0}));
    return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics3_result_2_1569, .tup1 = pop_result_2_1568.tup1});
  }
  if (count == UINT8_C(4)) {
    struct tuple_u256_bits_64 pop_result_2_1570 = pop(top);
    struct tuple_u256_bits_64 pop_result_2_1571 = pop(pop_result_2_1570.tup1);
    struct tuple_u256_bits_64 pop_result_2_1572 = pop(pop_result_2_1571.tup1);
    struct tuple_u256_bits_64 pop_result_2_1573 = pop(pop_result_2_1572.tup1);
    struct LogTopics LogTopics4_result_2_1574 = LogTopics4(((struct tuple_u256_u256_u256_u256){.tup0 = pop_result_2_1570.tup0, .tup1 = pop_result_2_1571.tup0, .tup2 = pop_result_2_1572.tup0, .tup3 = pop_result_2_1573.tup0}));
    return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics4_result_2_1574, .tup1 = pop_result_2_1573.tup1});
  }
  struct LogTopics LogTopics0_result_2_1575 = LogTopics0(UNIT);
  return ((struct tuple_LogTopics_bits_64){.tup0 = LogTopics0_result_2_1575, .tup1 = top});
}

struct tuple_bits_64_uint_64 execute_add(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1557 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1557.tup1;
  if (!charge_result_2_1557.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_add_result_2_1559 = alu_add(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_add_result_2_1559);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_mul(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1554 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1554.tup1;
  if (!charge_result_2_1554.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_mul_result_2_1556 = alu_mul(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_mul_result_2_1556);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_sub(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1551 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1551.tup1;
  if (!charge_result_2_1551.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_sub_result_2_1553 = alu_sub(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_sub_result_2_1553);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_div(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1548 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1548.tup1;
  if (!charge_result_2_1548.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_div_result_2_1550 = alu_div(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_div_result_2_1550);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_sdiv(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1545 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1545.tup1;
  if (!charge_result_2_1545.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_sdiv_result_2_1547 = alu_sdiv(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_sdiv_result_2_1547);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_mod(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1542 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1542.tup1;
  if (!charge_result_2_1542.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_mod_result_2_1544 = alu_mod(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_mod_result_2_1544);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_smod(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1539 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1539.tup1;
  if (!charge_result_2_1539.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_smod_result_2_1541 = alu_smod(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_smod_result_2_1541);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_addmod(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1536 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_mid);
  uint64_t g1 = charge_result_2_1536.tup1;
  if (!charge_result_2_1536.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  u256 n = stack_slot_read(top, UINT16_C(2));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(2));
  u256 alu_addmod_result_2_1538 = alu_addmod(a, b, n);
  stack_slot_write(top1, UINT16_C(0), alu_addmod_result_2_1538);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_mulmod(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1533 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_mid);
  uint64_t g1 = charge_result_2_1533.tup1;
  if (!charge_result_2_1533.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  u256 n = stack_slot_read(top, UINT16_C(2));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(2));
  u256 alu_mulmod_result_2_1535 = alu_mulmod(a, b, n);
  stack_slot_write(top1, UINT16_C(0), alu_mulmod_result_2_1535);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_exp(uint64_t top, uint64_t g)
{
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 e = stack_slot_read(top, UINT16_C(1));
  uint64_t gas_cost = exp_gas(e);
  struct tuple_bool_uint_64 charge_result_2_1528 = charge_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g, gas_cost);
  uint64_t g1 = charge_result_2_1528.tup1;
  if (!charge_result_2_1528.tup0) {
    uint64_t stack_top_retreat_result_2_1530 = stack_top_retreat(top, UINT16_C(2));
    return ((struct tuple_bits_64_uint_64){.tup0 = stack_top_retreat_result_2_1530, .tup1 = g1});
  }
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_exp_result_2_1532 = alu_exp(a, e);
  stack_slot_write(top1, UINT16_C(0), alu_exp_result_2_1532);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_signextend(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1525 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1525.tup1;
  if (!charge_result_2_1525.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 bi = stack_slot_read(top, UINT16_C(0));
  u256 v = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_signextend_result_2_1527 = alu_signextend(bi, v);
  stack_slot_write(top1, UINT16_C(0), alu_signextend_result_2_1527);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_lt(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1522 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1522.tup1;
  if (!charge_result_2_1522.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_lt_result_2_1524 = alu_lt(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_lt_result_2_1524);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_gt(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1519 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1519.tup1;
  if (!charge_result_2_1519.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_gt_result_2_1521 = alu_gt(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_gt_result_2_1521);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_slt(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1516 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1516.tup1;
  if (!charge_result_2_1516.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_slt_result_2_1518 = alu_slt(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_slt_result_2_1518);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_sgt(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1513 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1513.tup1;
  if (!charge_result_2_1513.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_sgt_result_2_1515 = alu_sgt(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_sgt_result_2_1515);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_eq(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1510 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1510.tup1;
  if (!charge_result_2_1510.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_eq_result_2_1512 = alu_eq(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_eq_result_2_1512);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_iszero(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1507 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1507.tup1;
  if (!charge_result_2_1507.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 alu_iszero_result_2_1509 = alu_iszero(a);
  stack_slot_write(top, UINT16_C(0), alu_iszero_result_2_1509);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_and(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1504 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1504.tup1;
  if (!charge_result_2_1504.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_and_result_2_1506 = alu_and(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_and_result_2_1506);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_or(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1501 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1501.tup1;
  if (!charge_result_2_1501.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_or_result_2_1503 = alu_or(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_or_result_2_1503);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_xor(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1498 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1498.tup1;
  if (!charge_result_2_1498.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 b = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_xor_result_2_1500 = alu_xor(a, b);
  stack_slot_write(top1, UINT16_C(0), alu_xor_result_2_1500);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_not(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1495 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1495.tup1;
  if (!charge_result_2_1495.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 a = stack_slot_read(top, UINT16_C(0));
  u256 alu_not_result_2_1497 = alu_not(a);
  stack_slot_write(top, UINT16_C(0), alu_not_result_2_1497);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_byte(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1492 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1492.tup1;
  if (!charge_result_2_1492.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 i = stack_slot_read(top, UINT16_C(0));
  u256 x = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_byte_result_2_1494 = alu_byte(i, x);
  stack_slot_write(top1, UINT16_C(0), alu_byte_result_2_1494);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_shl(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1489 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1489.tup1;
  if (!charge_result_2_1489.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 s = stack_slot_read(top, UINT16_C(0));
  u256 v = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_shl_result_2_1491 = alu_shl(s, v);
  stack_slot_write(top1, UINT16_C(0), alu_shl_result_2_1491);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_shr(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1486 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1486.tup1;
  if (!charge_result_2_1486.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 s = stack_slot_read(top, UINT16_C(0));
  u256 v = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_shr_result_2_1488 = alu_shr(s, v);
  stack_slot_write(top1, UINT16_C(0), alu_shr_result_2_1488);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_sar(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1483 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1483.tup1;
  if (!charge_result_2_1483.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 s = stack_slot_read(top, UINT16_C(0));
  u256 v = stack_slot_read(top, UINT16_C(1));
  uint64_t top1 = stack_top_retreat(top, UINT16_C(1));
  u256 alu_sar_result_2_1485 = alu_sar(s, v);
  stack_slot_write(top1, UINT16_C(0), alu_sar_result_2_1485);
  return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_clz(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1480 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1480.tup1;
  if (!charge_result_2_1480.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 x = stack_slot_read(top, UINT16_C(0));
  u256 alu_clz_result_2_1482 = alu_clz(x);
  stack_slot_write(top, UINT16_C(0), alu_clz_result_2_1482);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_keccak256(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1470 = pop(top);
  u256 offset_word = pop_result_2_1470.tup0;
  struct tuple_u256_bits_64 pop_result_2_1471 = pop(pop_result_2_1470.tup1);
  u256 length_word = pop_result_2_1471.tup0;
  uint64_t top2 = pop_result_2_1471.tup1;
  struct tuple_bool_uint_64 charge_keccak_gas_result_2_1472 = charge_keccak_gas(g, length_word);
  uint64_t g1 = charge_keccak_gas_result_2_1472.tup1;
  bool not_bool_result_2_1473 = (bool)(!charge_keccak_gas_result_2_1472.tup0);
  if (not_bool_result_2_1473) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g1});
  }
  uint64_t required_size = memory_required_size(offset_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1474 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g1, expansion_cost);
  uint64_t g2 = charge_result_2_1474.tup1;
  bool not_bool_result_2_1475 = (bool)(!charge_result_2_1474.tup0);
  if (not_bool_result_2_1475) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g2});
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  struct tuple_u256_Bytes result_2_1477 = mem_keccak(mem1, access.range);
  uint64_t push_word_result_2_1478 = push_word(top2, result_2_1477.tup0);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = push_word_result_2_1478, .tup1 = result_2_1477.tup1, .tup2 = g2});
}

struct tuple_bits_64_uint_64 execute_address(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1467 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1467.tup1;
  if (!charge_result_2_1467.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  bytes20 address = self_addr();
  u256 address_word = address_to_word(address);
  uint64_t push_word_result_2_1469 = push_word(top, address_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1469, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_origin(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1464 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1464.tup1;
  if (!charge_result_2_1464.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 origin = k_env(F_Origin);
  uint64_t push_word_result_2_1466 = push_word(top, origin);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1466, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_caller(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1460 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1460.tup1;
  if (!charge_result_2_1460.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 caller;
  bytes20 result_2_1463 = message.caller;
  caller = address_to_word(result_2_1463);
  uint64_t push_word_result_2_1462 = push_word(top, caller);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1462, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_callvalue(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1456 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1456.tup1;
  if (!charge_result_2_1456.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  uint64_t result_2_1459;
  u256 result_2_1458 = message.value;
  result_2_1459 = push_word(top, result_2_1458);
  return ((struct tuple_bits_64_uint_64){.tup0 = result_2_1459, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_gasprice(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1453 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1453.tup1;
  if (!charge_result_2_1453.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 gas_price = k_env(F_GasPrice);
  uint64_t push_word_result_2_1455 = push_word(top, gas_price);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1455, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_calldatasize(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1450 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1450.tup1;
  if (!charge_result_2_1450.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct CalldataSlice input = calldata;
  uint32_t input_length = calldata_slice_length(input);
  u256 length_word = word_of_source_byte_count(input_length);
  uint64_t push_word_result_2_1452 = push_word(top, length_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1452, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_calldataload(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1446 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1446.tup1;
  if (!charge_result_2_1446.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1448 = pop(top);
  u256 value = calldata_slice_load_word_offset(calldata, pop_result_2_1448.tup0);
  uint64_t push_word_result_2_1449 = push_word(pop_result_2_1448.tup1, value);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1449, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_calldatacopy(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1434 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1434.tup1;
  if (!charge_result_2_1434.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1436 = pop(top);
  u256 destination_word = pop_result_2_1436.tup0;
  struct tuple_u256_bits_64 pop_result_2_1437 = pop(pop_result_2_1436.tup1);
  u256 source_word = pop_result_2_1437.tup0;
  struct tuple_u256_bits_64 pop_result_2_1438 = pop(pop_result_2_1437.tup1);
  u256 length_word = pop_result_2_1438.tup0;
  uint64_t top3 = pop_result_2_1438.tup1;
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1439 = charge_copy_gas(g1, length_word);
  uint64_t g2 = charge_copy_gas_result_2_1439.tup1;
  bool not_bool_result_2_1440 = (bool)(!charge_copy_gas_result_2_1439.tup0);
  if (not_bool_result_2_1440) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g2});
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1441 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g2, expansion_cost);
  uint64_t g3 = charge_result_2_1441.tup1;
  bool not_bool_result_2_1442 = (bool)(!charge_result_2_1441.tup0);
  if (not_bool_result_2_1442) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g3});
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  calldata_slice_copy_word_offset(calldata, access.range.off, source_word, access.range.len);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem1, .tup2 = g3});
}

struct tuple_bits_64_uint_64 execute_codesize(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1431 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1431.tup1;
  if (!charge_result_2_1431.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  uint32_t code_length = frame_code_len();
  u256 length_word = word_of_source_byte_count(code_length);
  uint64_t push_word_result_2_1433 = push_word(top, length_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1433, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_codecopy(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1419 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1419.tup1;
  if (!charge_result_2_1419.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1421 = pop(top);
  u256 destination_word = pop_result_2_1421.tup0;
  struct tuple_u256_bits_64 pop_result_2_1422 = pop(pop_result_2_1421.tup1);
  u256 source_word = pop_result_2_1422.tup0;
  struct tuple_u256_bits_64 pop_result_2_1423 = pop(pop_result_2_1422.tup1);
  u256 length_word = pop_result_2_1423.tup0;
  uint64_t top3 = pop_result_2_1423.tup1;
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1424 = charge_copy_gas(g1, length_word);
  uint64_t g2 = charge_copy_gas_result_2_1424.tup1;
  bool not_bool_result_2_1425 = (bool)(!charge_copy_gas_result_2_1424.tup0);
  if (not_bool_result_2_1425) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g2});
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1426 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g2, expansion_cost);
  uint64_t g3 = charge_result_2_1426.tup1;
  bool not_bool_result_2_1427 = (bool)(!charge_result_2_1426.tup0);
  if (not_bool_result_2_1427) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g3});
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  struct CodeFields code = frame_code;
  Bytes bytes = code_bytes(code);
  code_slice_copy_word_offset(bytes, access.range.off, source_word, access.range.len);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem1, .tup2 = g3});
}

struct tuple_bits_64_uint_64 execute_balance(uint64_t top, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1415 = pop(top);
  uint64_t top1 = pop_result_2_1415.tup1;
  bytes20 a = word_to_address(pop_result_2_1415.tup0);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  struct tuple_bool_uint_64 charge_result_2_1416 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, gas_cost);
  uint64_t g1 = charge_result_2_1416.tup1;
  if (!charge_result_2_1416.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
  }
  k_account_mark_warm(a);
  u256 balance = k_get_balance(a);
  uint64_t push_word_result_2_1418 = push_word(top1, balance);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1418, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_selfbalance(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1412 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_low);
  uint64_t g1 = charge_result_2_1412.tup1;
  if (!charge_result_2_1412.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  bytes20 address = self_addr();
  u256 balance = k_get_balance(address);
  uint64_t push_word_result_2_1414 = push_word(top, balance);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1414, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_extcodesize(uint64_t top, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1407 = pop(top);
  uint64_t top1 = pop_result_2_1407.tup1;
  bytes20 a = word_to_address(pop_result_2_1407.tup0);
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  struct tuple_bool_uint_64 result_2_1409 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(g, ((uint32_t)access_cost + (uint32_t)read_cost));
  uint64_t g1 = result_2_1409.tup1;
  if (!result_2_1409.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
  }
  k_account_mark_warm(a);
  uint32_t code_size = k_get_code_size(a);
  u256 size_word = word_of_source_byte_count(code_size);
  uint64_t push_word_result_2_1411 = push_word(top1, size_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1411, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_extcodecopy(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1393 = pop(top);
  bytes20 a = word_to_address(pop_result_2_1393.tup0);
  struct tuple_u256_bits_64 pop_result_2_1394 = pop(pop_result_2_1393.tup1);
  u256 destination_word = pop_result_2_1394.tup0;
  struct tuple_u256_bits_64 pop_result_2_1395 = pop(pop_result_2_1394.tup1);
  u256 source_word = pop_result_2_1395.tup0;
  struct tuple_u256_bits_64 pop_result_2_1396 = pop(pop_result_2_1395.tup1);
  u256 length_word = pop_result_2_1396.tup0;
  uint64_t top4 = pop_result_2_1396.tup1;
  bool warm = k_account_is_warm(a);
  uint16_t access_cost = account_cost(warm);
  uint16_t read_cost = external_code_read_cost();
  struct tuple_bool_uint_64 result_2_1398 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(g, ((uint32_t)access_cost + (uint32_t)read_cost));
  uint64_t g1 = result_2_1398.tup1;
  bool not_bool_result_2_1399 = (bool)(!result_2_1398.tup0);
  if (not_bool_result_2_1399) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top4, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1400 = charge_copy_gas(g1, length_word);
  uint64_t g2 = charge_copy_gas_result_2_1400.tup1;
  bool not_bool_result_2_1401 = (bool)(!charge_copy_gas_result_2_1400.tup0);
  if (not_bool_result_2_1401) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top4, .tup1 = mem, .tup2 = g2});
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1402 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g2, expansion_cost);
  uint64_t g3 = charge_result_2_1402.tup1;
  bool not_bool_result_2_1403 = (bool)(!charge_result_2_1402.tup0);
  if (not_bool_result_2_1403) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top4, .tup1 = mem, .tup2 = g3});
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  k_account_mark_warm(a);
  k_code_copy(a, access.range.off, source_word, access.range.len);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top4, .tup1 = mem1, .tup2 = g3});
}

struct tuple_bits_64_uint_64 execute_extcodehash(uint64_t top, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1389 = pop(top);
  uint64_t top1 = pop_result_2_1389.tup1;
  bytes20 a = word_to_address(pop_result_2_1389.tup0);
  bool warm = k_account_is_warm(a);
  uint16_t gas_cost = account_cost(warm);
  struct tuple_bool_uint_64 charge_result_2_1390 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, gas_cost);
  uint64_t g1 = charge_result_2_1390.tup1;
  if (!charge_result_2_1390.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top1, .tup1 = g1});
  }
  k_account_mark_warm(a);
  bytes32 code_hash = k_get_codehash(a);
  u256 hash_word = hash_to_word(code_hash);
  uint64_t push_word_result_2_1392 = push_word(top1, hash_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1392, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_returndatasize(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1386 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1386.tup1;
  if (!charge_result_2_1386.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  uint32_t return_data_size = returndata_size();
  u256 size_word = word_of_source_byte_count(return_data_size);
  uint64_t push_word_result_2_1388 = push_word(top, size_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1388, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_returndatacopy(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1375 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1375.tup1;
  if (!charge_result_2_1375.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1377 = pop(top);
  u256 destination_word = pop_result_2_1377.tup0;
  struct tuple_u256_bits_64 pop_result_2_1378 = pop(pop_result_2_1377.tup1);
  u256 source_word = pop_result_2_1378.tup0;
  struct tuple_u256_bits_64 pop_result_2_1379 = pop(pop_result_2_1378.tup1);
  u256 length_word = pop_result_2_1379.tup0;
  uint64_t top3 = pop_result_2_1379.tup1;
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1380 = charge_copy_gas(g1, length_word);
  uint64_t g2 = charge_copy_gas_result_2_1380.tup1;
  bool not_bool_result_2_1381 = (bool)(!charge_copy_gas_result_2_1380.tup0);
  if (not_bool_result_2_1381) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g2});
  }
  uint64_t required_size = memory_required_size(destination_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1382 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g2, expansion_cost);
  uint64_t g3 = charge_result_2_1382.tup1;
  bool not_bool_result_2_1383 = (bool)(!charge_result_2_1382.tup0);
  if (not_bool_result_2_1383) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g3});
  }
  struct MemoryAccessFields access = memory_access(destination_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  uint64_t g4 = returndata_copy_words(g3, access.range.off, source_word, length_word);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem1, .tup2 = g4});
}

struct tuple_bits_64_uint_64 execute_blockhash(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1371 = charge_uint64_t_uint8_t_to_struct_tuple_bool_uint_64(g, UINT8_C(20));
  uint64_t g1 = charge_result_2_1371.tup1;
  if (!charge_result_2_1371.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1373 = pop(top);
  bytes32 block_hash = k_blockhash(pop_result_2_1373.tup0);
  u256 hash_word = hash_to_word(block_hash);
  uint64_t push_word_result_2_1374 = push_word(pop_result_2_1373.tup1, hash_word);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1374, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_coinbase(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1368 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1368.tup1;
  if (!charge_result_2_1368.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 coinbase = k_env(F_Coinbase);
  uint64_t push_word_result_2_1370 = push_word(top, coinbase);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1370, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_timestamp(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1365 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1365.tup1;
  if (!charge_result_2_1365.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 timestamp = k_env(F_Timestamp);
  uint64_t push_word_result_2_1367 = push_word(top, timestamp);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1367, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_number(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1362 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1362.tup1;
  if (!charge_result_2_1362.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 block_number = k_env(F_Number);
  uint64_t push_word_result_2_1364 = push_word(top, block_number);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1364, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_slotnum(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1359 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1359.tup1;
  if (!charge_result_2_1359.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 slot_number = k_env(F_SlotNumber);
  uint64_t push_word_result_2_1361 = push_word(top, slot_number);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1361, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_prevrandao(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1356 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1356.tup1;
  if (!charge_result_2_1356.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 prev_randao = k_env(F_PrevRandao);
  uint64_t push_word_result_2_1358 = push_word(top, prev_randao);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1358, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_gaslimit(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1353 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1353.tup1;
  if (!charge_result_2_1353.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 gas_limit = k_env(F_GasLimit);
  uint64_t push_word_result_2_1355 = push_word(top, gas_limit);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1355, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_chainid(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1350 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1350.tup1;
  if (!charge_result_2_1350.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 chain_id = k_env(F_ChainId);
  uint64_t push_word_result_2_1352 = push_word(top, chain_id);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1352, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_basefee(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1347 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1347.tup1;
  if (!charge_result_2_1347.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 base_fee = k_env(F_BaseFee);
  uint64_t push_word_result_2_1349 = push_word(top, base_fee);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1349, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_blobbasefee(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1342 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1342.tup1;
  if (!charge_result_2_1342.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  u256 base_fee;
  uint32_t result_2_1346 = k_header.excess_blob_gas;
  base_fee = blob_base_fee(execution_profile.protocol, result_2_1346);
  uint64_t push_word_result_2_1344 = push_word(top, base_fee);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1344, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_blobhash(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1338 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1338.tup1;
  if (!charge_result_2_1338.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1340 = pop(top);
  u256 blob_hash = k_blobhash(pop_result_2_1340.tup0);
  uint64_t push_word_result_2_1341 = push_word(pop_result_2_1340.tup1, blob_hash);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1341, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_pop(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1335 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1335.tup1;
  if (!charge_result_2_1335.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1337 = pop(top);
  return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1337.tup1, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_mload(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1326 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1326.tup1;
  if (!charge_result_2_1326.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1328 = pop(top);
  u256 offset_word = pop_result_2_1328.tup0;
  uint64_t top1 = pop_result_2_1328.tup1;
  uint64_t required_size = memory_required_size_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1329 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g1, expansion_cost);
  uint64_t g2 = charge_result_2_1329.tup1;
  bool not_bool_result_2_1330 = (bool)(!charge_result_2_1329.tup0);
  if (not_bool_result_2_1330) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top1, .tup1 = mem, .tup2 = g2});
  }
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  Bytes mem1 = expand_memory(mem, access.required_size);
  u256 value = mem_load(access.range.off);
  uint64_t push_word_result_2_1331 = push_word(top1, value);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = push_word_result_2_1331, .tup1 = mem1, .tup2 = g2});
}

struct tuple_bits_64_Bytes_uint_64 execute_mstore(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1317 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1317.tup1;
  if (!charge_result_2_1317.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1319 = pop(top);
  u256 offset_word = pop_result_2_1319.tup0;
  struct tuple_u256_bits_64 pop_result_2_1320 = pop(pop_result_2_1319.tup1);
  u256 v = pop_result_2_1320.tup0;
  uint64_t top2 = pop_result_2_1320.tup1;
  uint64_t required_size = memory_required_size_u256_u256_to_uint64_t(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1321 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g1, expansion_cost);
  uint64_t g2 = charge_result_2_1321.tup1;
  bool not_bool_result_2_1322 = (bool)(!charge_result_2_1321.tup0);
  if (not_bool_result_2_1322) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g2});
  }
  struct MemoryAccessFields access = memory_access_u256_u256_to_struct_MemoryAccessFields(offset_word, (u256){{UINT64_C(32), UINT64_C(0), UINT64_C(0), UINT64_C(0)}});
  Bytes mem1 = expand_memory(mem, access.required_size);
  mem_store(access.range.off, v);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem1, .tup2 = g2});
}

struct tuple_bits_64_Bytes_uint_64 execute_mstore8(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1308 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1308.tup1;
  if (!charge_result_2_1308.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1310 = pop(top);
  u256 offset_word = pop_result_2_1310.tup0;
  struct tuple_u256_bits_64 pop_result_2_1311 = pop(pop_result_2_1310.tup1);
  u256 v = pop_result_2_1311.tup0;
  uint64_t top2 = pop_result_2_1311.tup1;
  uint64_t required_size = memory_required_size(offset_word, WORD_ONE);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1312 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g1, expansion_cost);
  uint64_t g2 = charge_result_2_1312.tup1;
  bool not_bool_result_2_1313 = (bool)(!charge_result_2_1312.tup0);
  if (not_bool_result_2_1313) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g2});
  }
  struct MemoryAccessFields access = memory_access(offset_word, WORD_ONE);
  Bytes mem1 = expand_memory(mem, access.required_size);
  mem_store_byte(access.range.off, v);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem1, .tup2 = g2});
}

struct tuple_bits_64_Bytes_uint_64 execute_msize(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1304 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1304.tup1;
  if (!charge_result_2_1304.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  uint32_t high_water = memory_high_water(mem);
  uint32_t words = memory_word_count_uint32_t_to_uint32_t(high_water);
  u256 size = word_of_nat_byte_count_uint64_t_to_u256(((uint64_t)words * UINT64_C(32)));
  uint64_t push_word_result_2_1306 = push_word(top, size);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = push_word_result_2_1306, .tup1 = mem, .tup2 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_mcopy(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1285 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1285.tup1;
  if (!charge_result_2_1285.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1287 = pop(top);
  u256 destination_word = pop_result_2_1287.tup0;
  struct tuple_u256_bits_64 pop_result_2_1288 = pop(pop_result_2_1287.tup1);
  u256 source_word = pop_result_2_1288.tup0;
  struct tuple_u256_bits_64 pop_result_2_1289 = pop(pop_result_2_1288.tup1);
  u256 length_word = pop_result_2_1289.tup0;
  uint64_t top3 = pop_result_2_1289.tup1;
  struct tuple_bool_uint_64 charge_copy_gas_result_2_1290 = charge_copy_gas(g1, length_word);
  uint64_t g2 = charge_copy_gas_result_2_1290.tup1;
  bool not_bool_result_2_1291 = (bool)(!charge_copy_gas_result_2_1290.tup0);
  if (not_bool_result_2_1291) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g2});
  }
  uint64_t destination_required = memory_required_size(destination_word, length_word);
  uint64_t source_required = memory_required_size(source_word, length_word);
  uint64_t required_size = destination_required < source_required ? source_required : destination_required;
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1292 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g2, expansion_cost);
  uint64_t g3 = charge_result_2_1292.tup1;
  bool not_bool_result_2_1293 = (bool)(!charge_result_2_1292.tup0);
  if (not_bool_result_2_1293) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g3});
  }
  struct MemoryAccessFields destination = memory_access(destination_word, length_word);
  struct MemoryAccessFields source = memory_access(source_word, length_word);
  uint32_t materialized_required_size = destination.required_size < source.required_size ? source.required_size : destination.required_size;
  Bytes mem1 = expand_memory(mem, materialized_required_size);
  mem_mcopy(destination.range.off, source.range.off, destination.range.len);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem1, .tup2 = g3});
}

struct tuple_bits_64_uint_64 execute_sload_(uint64_t top, uint64_t g)
{
  uint64_t charged = execute_sload(top, g);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = charged});
}

struct tuple_bits_64_uint_64 execute_sstore_(uint64_t top, uint64_t g)
{
  uint64_t charged = execute_sstore(top, g);
  uint64_t stack_top_retreat_result_2_1284 = stack_top_retreat(top, UINT16_C(2));
  return ((struct tuple_bits_64_uint_64){.tup0 = stack_top_retreat_result_2_1284, .tup1 = charged});
}

struct tuple_bits_64_uint_64 execute_tload(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1280 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_warm_access);
  uint64_t g1 = charge_result_2_1280.tup1;
  if (!charge_result_2_1280.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1282 = pop(top);
  bytes20 address = self_addr();
  u256 value = k_tload(address, pop_result_2_1282.tup0);
  uint64_t push_word_result_2_1283 = push_word(pop_result_2_1282.tup1, value);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1283, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_tstore(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 guard_static_result_2_1275 = guard_static(g);
  uint64_t g0 = guard_static_result_2_1275.tup1;
  if (guard_static_result_2_1275.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g0});
  }
  struct tuple_bool_uint_64 charge_result_2_1276 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g0, G_warm_access);
  uint64_t g1 = charge_result_2_1276.tup1;
  if (!charge_result_2_1276.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1278 = pop(top);
  struct tuple_u256_bits_64 pop_result_2_1279 = pop(pop_result_2_1278.tup1);
  bytes20 address = self_addr();
  k_tstore(address, pop_result_2_1278.tup0, pop_result_2_1279.tup0);
  return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1279.tup1, .tup1 = g1});
}

struct tuple_uint_32_bits_64_uint_64 execute_jump(uint32_t pc_in, uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1271 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_mid);
  uint64_t g1 = charge_result_2_1271.tup1;
  if (!charge_result_2_1271.tup0) {
    return ((struct tuple_uint_32_bits_64_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1273 = pop(top);
  struct tuple_uint_32_uint_64 do_jump_result_2_1274 = do_jump(pc_in, g1, pop_result_2_1273.tup0);
  return ((struct tuple_uint_32_bits_64_uint_64){.tup0 = do_jump_result_2_1274.tup0, .tup1 = pop_result_2_1273.tup1, .tup2 = do_jump_result_2_1274.tup1});
}

struct tuple_uint_32_bits_64_uint_64 execute_jumpi(uint32_t pc_in, uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1266 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_high);
  uint64_t g1 = charge_result_2_1266.tup1;
  if (!charge_result_2_1266.tup0) {
    return ((struct tuple_uint_32_bits_64_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = g1});
  }
  struct tuple_u256_bits_64 pop_result_2_1268 = pop(top);
  struct tuple_u256_bits_64 pop_result_2_1269 = pop(pop_result_2_1268.tup1);
  uint64_t top2 = pop_result_2_1269.tup1;
  bool condition_is_zero = word_is_zero(pop_result_2_1269.tup0);
  if (condition_is_zero) {
    return ((struct tuple_uint_32_bits_64_uint_64){.tup0 = pc_in, .tup1 = top2, .tup2 = g1});
  }
  struct tuple_uint_32_uint_64 do_jump_result_2_1270 = do_jump(pc_in, g1, pop_result_2_1268.tup0);
  return ((struct tuple_uint_32_bits_64_uint_64){.tup0 = do_jump_result_2_1270.tup0, .tup1 = top2, .tup2 = do_jump_result_2_1270.tup1});
}

struct tuple_bits_64_uint_64 execute_pc(uint32_t pc_in, uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1263 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1263.tup1;
  if (!charge_result_2_1263.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 next_pc = word_of_source_byte_count(pc_in);
  u256 opcode_pc = alu_sub(next_pc, WORD_ONE);
  uint64_t push_word_result_2_1265 = push_word(top, opcode_pc);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1265, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_gas(uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1260 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
  uint64_t g1 = charge_result_2_1260.tup1;
  if (!charge_result_2_1260.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  uint64_t push_gas_result_2_1262 = push_gas(top, g1);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_gas_result_2_1262, .tup1 = g1});
}

uint64_t execute_jumpdest(uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1259 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_jumpdest);
  return charge_result_2_1259.tup1;
}

struct tuple_bits_64_uint_64 execute_push(uint8_t n, u256 v, uint64_t top, uint64_t g)
{
  if (n == UINT8_C(0)) {
    struct tuple_bool_uint_64 charge_result_2_1253 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_base);
    uint64_t g1 = charge_result_2_1253.tup1;
    if (!charge_result_2_1253.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
    }
    uint64_t push_word_result_2_1255 = push_word(top, v);
    return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1255, .tup1 = g1});
  }
  struct tuple_bool_uint_64 charge_result_2_1256 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1_3_2347 = charge_result_2_1256.tup1;
  if (!charge_result_2_1256.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1_3_2347});
  }
  uint64_t push_word_result_2_1258 = push_word(top, v);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1258, .tup1 = g1_3_2347});
}

struct tuple_bits_64_uint_64 execute_swap(uint8_t n, uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1246 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1246.tup1;
  if (!charge_result_2_1246.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 top_value = peek_uint64_t_uint8_t_to_u256(top, UINT8_C(0));
  u256 other = peek_uint64_t_uint8_t_to_u256(top, n);
  stack_set_uint64_t_uint8_t_u256_to_unit(top, UINT8_C(0), other);
  stack_set_uint64_t_uint8_t_u256_to_unit(top, n, top_value);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
}

struct tuple_bits_64_uint_64 execute_dupn(uint64_t immediate, uint64_t top, uint64_t g)
{
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (valid_immediate) {
    struct tuple_bool_uint_64 charge_result_2_1242 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
    uint64_t g1 = charge_result_2_1242.tup1;
    if (!charge_result_2_1242.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
    }
    uint8_t n = decode_single_stack_index(immediate);
    u256 value = peek_uint64_t_uint8_t_to_u256(top, ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
    uint64_t push_word_result_2_1244 = push_word(top, value);
    return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1244, .tup1 = g1});
  }
  uint64_t exc_halt_result_2_1241 = exc_halt(g, InvalidOpcode);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = exc_halt_result_2_1241});
}

struct tuple_bits_64_uint_64 execute_swapn(uint64_t immediate, uint64_t top, uint64_t g)
{
  bool valid_immediate = deep_stack_immediate_valid(immediate);
  if (valid_immediate) {
    struct tuple_bool_uint_64 charge_result_2_1238 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
    uint64_t g1 = charge_result_2_1238.tup1;
    if (!charge_result_2_1238.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
    }
    uint8_t n = decode_single_stack_index(immediate);
    u256 top_value = peek_uint64_t_uint8_t_to_u256(top, UINT8_C(0));
    u256 other = peek_uint64_t_uint8_t_to_u256(top, n);
    stack_set_uint64_t_uint8_t_u256_to_unit(top, UINT8_C(0), other);
    stack_set_uint64_t_uint8_t_u256_to_unit(top, n, top_value);
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  uint64_t exc_halt_result_2_1237 = exc_halt(g, InvalidOpcode);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = exc_halt_result_2_1237});
}

struct tuple_bits_64_uint_64 execute_exchange(uint64_t immediate, uint64_t top, uint64_t g)
{
  bool valid_immediate = exchange_immediate_valid(immediate);
  if (!valid_immediate) {
    uint64_t exc_halt_result_2_1231 = exc_halt(g, InvalidOpcode);
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = exc_halt_result_2_1231});
  }
  struct tuple_bool_uint_64 charge_result_2_1233 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1233.tup1;
  if (!charge_result_2_1233.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  struct tuple_uint_16_uint_16 decode_exchange_stack_indices_result_2_1235 = decode_exchange_stack_indices(immediate);
  uint16_t n = decode_exchange_stack_indices_result_2_1235.tup0;
  uint16_t m = decode_exchange_stack_indices_result_2_1235.tup1;
  u256 first = peek(top, n);
  u256 second = peek(top, m);
  stack_set(top, n, second);
  stack_set(top, m, first);
  return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_log(uint8_t n, uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_bool_uint_64 guard_static_result_2_1216 = guard_static(g);
  uint64_t g0 = guard_static_result_2_1216.tup1;
  if (guard_static_result_2_1216.tup0) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top, .tup1 = mem, .tup2 = g0});
  }
  struct tuple_u256_bits_64 pop_result_2_1217 = pop(top);
  u256 offset_word = pop_result_2_1217.tup0;
  struct tuple_u256_bits_64 pop_result_2_1218 = pop(pop_result_2_1217.tup1);
  u256 length_word = pop_result_2_1218.tup0;
  struct tuple_LogTopics_bits_64 pop_log_topics_result_2_1219 = pop_log_topics(n, pop_result_2_1218.tup1);
  struct LogTopics topics = pop_log_topics_result_2_1219.tup0;
  uint64_t top3 = pop_log_topics_result_2_1219.tup1;
  struct tuple_bool_uint_64 charge_log_gas_result_2_1220 = charge_log_gas(g0, n, length_word);
  uint64_t g1 = charge_log_gas_result_2_1220.tup1;
  bool not_bool_result_2_1221 = (bool)(!charge_log_gas_result_2_1220.tup0);
  if (not_bool_result_2_1221) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g1});
  }
  uint64_t required_size = memory_required_size(offset_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1222 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g1, expansion_cost);
  uint64_t g2 = charge_result_2_1222.tup1;
  bool not_bool_result_2_1223 = (bool)(!charge_result_2_1222.tup0);
  if (not_bool_result_2_1223) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = mem, .tup2 = g2});
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  struct tuple_Bytes_Bytes result_2_1226 = active_memory_slice(mem1, access.range.off, access.range.len);
  bytes20 address = self_addr();
  Bytes memory_slice = evm_memory_slice(result_2_1226.tup0.bytes, result_2_1226.tup0.len);
  struct LogData log_data = LogDataMemory(memory_slice);
  k_log(address, topics, log_data);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top3, .tup1 = result_2_1226.tup1, .tup2 = g2});
}

void execute_stop(void)
{
  struct HaltKind reason = HaltStop(UNIT);
  frame_status = Halted(reason);
}

struct tuple_bits_64_Bytes_uint_64 execute_return(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1208 = pop(top);
  u256 offset_word = pop_result_2_1208.tup0;
  struct tuple_u256_bits_64 pop_result_2_1209 = pop(pop_result_2_1208.tup1);
  u256 length_word = pop_result_2_1209.tup0;
  uint64_t top2 = pop_result_2_1209.tup1;
  uint64_t required_size = memory_required_size(offset_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1210 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g, expansion_cost);
  uint64_t g1 = charge_result_2_1210.tup1;
  bool not_bool_result_2_1211 = (bool)(!charge_result_2_1210.tup0);
  if (not_bool_result_2_1211) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g1});
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  struct tuple_Bytes_Bytes result_2_1214 = active_memory_slice(mem1, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(result_2_1214.tup0);
  struct HaltKind reason = HaltReturn(output);
  frame_status = Halted(reason);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = result_2_1214.tup1, .tup2 = g1});
}

struct tuple_bits_64_Bytes_uint_64 execute_revert(uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_u256_bits_64 pop_result_2_1200 = pop(top);
  u256 offset_word = pop_result_2_1200.tup0;
  struct tuple_u256_bits_64 pop_result_2_1201 = pop(pop_result_2_1200.tup1);
  u256 length_word = pop_result_2_1201.tup0;
  uint64_t top2 = pop_result_2_1201.tup1;
  uint64_t required_size = memory_required_size(offset_word, length_word);
  u128 expansion_cost = memory_expansion_cost(mem, required_size);
  struct tuple_bool_uint_64 charge_result_2_1202 = charge_uint64_t_u128_to_struct_tuple_bool_uint_64(g, expansion_cost);
  uint64_t g1 = charge_result_2_1202.tup1;
  bool not_bool_result_2_1203 = (bool)(!charge_result_2_1202.tup0);
  if (not_bool_result_2_1203) {
    return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = mem, .tup2 = g1});
  }
  struct MemoryAccessFields access = memory_access(offset_word, length_word);
  Bytes mem1 = expand_memory(mem, access.required_size);
  uint64_t g2 = refill_frame_state_gas(g1);
  struct tuple_Bytes_Bytes result_2_1206 = active_memory_slice(mem1, access.range.off, access.range.len);
  Bytes output = freeze_memory_output(result_2_1206.tup0);
  struct HaltKind reason = HaltRevert(output);
  frame_status = Halted(reason);
  return ((struct tuple_bits_64_Bytes_uint_64){.tup0 = top2, .tup1 = result_2_1206.tup1, .tup2 = g2});
}

uint64_t execute_invalid(uint64_t g)
{
  uint64_t consumed = gas_sub(g, g);
  return exc_halt(consumed, InvalidOpcode);
}

struct tuple_bits_64_uint_64 execute_selfdestruct(uint64_t top, uint64_t g)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct tuple_bool_uint_64 guard_static_result_2_1175 = guard_static(g);
  uint64_t g0 = guard_static_result_2_1175.tup1;
  if (guard_static_result_2_1175.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g0});
  }
  struct tuple_u256_bits_64 pop_result_2_1176 = pop(top);
  bytes20 beneficiary = word_to_address(pop_result_2_1176.tup0);
  bytes20 address = self_addr();
  bool result_2_1178 = (bool)(profile.fork >= Amsterdam);
  if (result_2_1178) {
    bool warm = k_account_is_warm(beneficiary);
    uint32_t access_cost;
    uint16_t add_atom_result_2_1185 = ((uint16_t)((uint32_t)G_selfdestruct + (uint32_t)UINT16_C(0)));
    uint16_t result_2_1186 = warm ? G_zero : G_amsterdam_cold_account_access;
    access_cost = ((uint32_t)add_atom_result_2_1185 + (uint32_t)result_2_1186);
    struct tuple_bool_uint_64 check_execution_gas_result_2_1179 = check_execution_gas(g0, access_cost);
    if (!check_execution_gas_result_2_1179.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = check_execution_gas_result_2_1179.tup1});
    }
    k_account_mark_warm(beneficiary);
    u256 bal = k_get_balance(address);
    bool nonzero_balance = word_nonzero(bal);
    bool beneficiary_empty = k_account_is_empty(beneficiary);
    bool creates_account = (bool)(nonzero_balance && beneficiary_empty);
    uint32_t execution_cost = creates_account ? (access_cost + (uint32_t)G_amsterdam_account_write) : access_cost;
    struct tuple_bool_uint_64 charge_result_2_1181 = charge_uint64_t_uint32_t_to_struct_tuple_bool_uint_64(g0, execution_cost);
    uint64_t g1 = charge_result_2_1181.tup1;
    if (!charge_result_2_1181.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = g1});
    }
    uint64_t g2 = g1;
    if (creates_account) {
      struct tuple_bool_uint_64 charge_state_gas_result_2_1183 = charge_state_gas(g1, G_amsterdam_state_new_account);
      uint64_t state_gas = charge_state_gas_result_2_1183.tup1;
      g2 = state_gas;
      if (!charge_state_gas_result_2_1183.tup0) {
        return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = state_gas});
      }
    }
    k_transfer(address, beneficiary, bal);
    bool created = k_was_created(address);
    if (created) {
      k_selfdestruct(address);
    }
    struct HaltKind reason = HaltSelfDestruct(UNIT);
    frame_status = Halted(reason);
    return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = g2});
  }
  u256 bal_3_2244 = k_get_balance(address);
  bool warm_3_2245 = k_account_is_warm(beneficiary);
  struct tuple_bool_uint_64 charge_result_2_1187 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g0, G_selfdestruct);
  uint64_t g1_3_2246 = charge_result_2_1187.tup1;
  if (!charge_result_2_1187.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = g1_3_2246});
  }
  uint64_t g2_3_2247 = g1_3_2246;
  if (!warm_3_2245) {
    struct tuple_bool_uint_64 charge_result_2_1190 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g1_3_2246, G_cold_account);
    uint64_t cold_gas = charge_result_2_1190.tup1;
    g2_3_2247 = cold_gas;
    if (!charge_result_2_1190.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = cold_gas});
    }
  }
  k_account_mark_warm(beneficiary);
  bool nonzero_balance_3_2248 = word_nonzero(bal_3_2244);
  bool beneficiary_empty_3_2249 = k_account_is_empty(beneficiary);
  uint64_t g3 = g2_3_2247;
  if (nonzero_balance_3_2248 && beneficiary_empty_3_2249) {
    struct tuple_bool_uint_64 charge_result_2_1193 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g2_3_2247, G_newaccount);
    uint64_t new_account_gas = charge_result_2_1193.tup1;
    g3 = new_account_gas;
    if (!charge_result_2_1193.tup0) {
      return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = new_account_gas});
    }
  }
  bool is_selfdestructed = k_is_selfdestructed(address);
  bool result_2_1196 = (bool)(profile.fork < London);
  if (result_2_1196 && !is_selfdestructed) {
    record_refund_uint16_t_to_unit(R_selfdestruct_pre_london);
  }
  k_transfer(address, beneficiary, bal_3_2244);
  bool result_2_1199 = (bool)(profile.fork < Cancun);
  if (result_2_1199) {
    k_zero_balance(address);
    k_selfdestruct(address);
  } else {
    bool created_3_2250 = k_was_created(address);
    if (created_3_2250) {
      k_zero_balance(address);
      k_selfdestruct(address);
    }
  }
  struct HaltKind reason_3_2251 = HaltSelfDestruct(UNIT);
  frame_status = Halted(reason_3_2251);
  return ((struct tuple_bits_64_uint_64){.tup0 = pop_result_2_1176.tup1, .tup1 = g3});
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_create(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_create(CreateByNonce, pc_in, top, mem, g);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_create2(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_create(CreateBySalt, pc_in, top, mem, g);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_call(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_call(Call, pc_in, top, mem, g);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_callcode(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_call(CallCode, pc_in, top, mem, g);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_delegatecall(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_call(DelegateCall, pc_in, top, mem, g);
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_staticcall(uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  return run_call(StaticCall, pc_in, top, mem, g);
}

struct tuple_uint_16_uint_16 opcode_stack_effect(struct ast op)
{
  switch (op.kind) {
  case Kind_STOP:
  case Kind_JUMPDEST:
  case Kind_INVALID:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(0), .tup1 = UINT16_C(0)});
  case Kind_ADD:
  case Kind_MUL:
  case Kind_SUB:
  case Kind_DIV:
  case Kind_SDIV:
  case Kind_MOD:
  case Kind_SMOD:
  case Kind_EXP:
  case Kind_SIGNEXTEND:
  case Kind_LT:
  case Kind_GT:
  case Kind_SLT:
  case Kind_SGT:
  case Kind_EQ:
  case Kind_AND:
  case Kind_OR:
  case Kind_XOR:
  case Kind_BYTE:
  case Kind_SHL:
  case Kind_SHR:
  case Kind_SAR:
  case Kind_KECCAK256:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(2), .tup1 = UINT16_C(1)});
  case Kind_ADDMOD:
  case Kind_MULMOD:
  case Kind_opcode_CREATE:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(3), .tup1 = UINT16_C(1)});
  case Kind_ISZERO:
  case Kind_NOT:
  case Kind_CLZ:
  case Kind_BALANCE:
  case Kind_CALLDATALOAD:
  case Kind_EXTCODESIZE:
  case Kind_EXTCODEHASH:
  case Kind_BLOCKHASH:
  case Kind_BLOBHASH:
  case Kind_MLOAD:
  case Kind_SLOAD:
  case Kind_TLOAD:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(1), .tup1 = UINT16_C(1)});
  case Kind_ADDRESS:
  case Kind_ORIGIN:
  case Kind_CALLER:
  case Kind_CALLVALUE:
  case Kind_CALLDATASIZE:
  case Kind_CODESIZE:
  case Kind_GASPRICE:
  case Kind_RETURNDATASIZE:
  case Kind_COINBASE:
  case Kind_TIMESTAMP:
  case Kind_NUMBER:
  case Kind_PREVRANDAO:
  case Kind_GASLIMIT:
  case Kind_CHAINID:
  case Kind_SELFBALANCE:
  case Kind_BASEFEE:
  case Kind_BLOBBASEFEE:
  case Kind_SLOTNUM:
  case Kind_PC:
  case Kind_MSIZE:
  case Kind_GAS:
  case Kind_PUSH:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(0), .tup1 = UINT16_C(1)});
  case Kind_CALLDATACOPY:
  case Kind_CODECOPY:
  case Kind_RETURNDATACOPY:
  case Kind_MCOPY:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(3), .tup1 = UINT16_C(0)});
  case Kind_EXTCODECOPY:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(4), .tup1 = UINT16_C(0)});
  case Kind_POP:
  case Kind_JUMP:
  case Kind_SELFDESTRUCT:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(1), .tup1 = UINT16_C(0)});
  case Kind_MSTORE:
  case Kind_MSTORE8:
  case Kind_SSTORE:
  case Kind_JUMPI:
  case Kind_TSTORE:
  case Kind_RETURN:
  case Kind_REVERT:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(2), .tup1 = UINT16_C(0)});
  case Kind_DUP:
    return ((struct tuple_uint_16_uint_16){.tup0 = (uint16_t)op.variants.DUP, .tup1 = ((uint16_t)UINT8_C(1) + (uint16_t)op.variants.DUP)});
  case Kind_SWAP:
    return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)UINT8_C(1) + (uint16_t)op.variants.SWAP), .tup1 = ((uint16_t)UINT8_C(1) + (uint16_t)op.variants.SWAP)});
  case Kind_DUPN:
  {
    uint64_t immediate = op.variants.DUPN;
    bool valid_immediate = deep_stack_immediate_valid(immediate);
    if (valid_immediate) {
      uint8_t n_3_1980 = decode_single_stack_index(immediate);
      return ((struct tuple_uint_16_uint_16){.tup0 = (uint16_t)n_3_1980, .tup1 = ((uint16_t)UINT8_C(1) + (uint16_t)n_3_1980)});
    }
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(0), .tup1 = UINT16_C(0)});
  }
  case Kind_SWAPN:
  {
    uint64_t immediate_3_1981 = op.variants.SWAPN;
    bool valid_immediate_3_1982 = deep_stack_immediate_valid(immediate_3_1981);
    if (valid_immediate_3_1982) {
      uint8_t n_3_1983 = decode_single_stack_index(immediate_3_1981);
      return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)UINT8_C(1) + (uint16_t)n_3_1983), .tup1 = ((uint16_t)UINT8_C(1) + (uint16_t)n_3_1983)});
    }
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(0), .tup1 = UINT16_C(0)});
  }
  case Kind_EXCHANGE:
  {
    uint64_t immediate_3_1984 = op.variants.EXCHANGE;
    bool valid_immediate_3_1985 = exchange_immediate_valid(immediate_3_1984);
    if (valid_immediate_3_1985) {
      struct tuple_uint_16_uint_16 decode_exchange_stack_indices_result_2_1063 = decode_exchange_stack_indices(immediate_3_1984);
      return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)((uint32_t)decode_exchange_stack_indices_result_2_1063.tup1 + (uint32_t)UINT16_C(1))), .tup1 = ((uint16_t)((uint32_t)decode_exchange_stack_indices_result_2_1063.tup1 + (uint32_t)UINT16_C(1)))});
    }
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(0), .tup1 = UINT16_C(0)});
  }
  case Kind_LOG:
    return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)UINT8_C(2) + (uint16_t)op.variants.LOG), .tup1 = UINT16_C(0)});
  case Kind_CALL:
  case Kind_CALLCODE:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(7), .tup1 = UINT16_C(1)});
  case Kind_DELEGATECALL:
  case Kind_STATICCALL:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(6), .tup1 = UINT16_C(1)});
  case Kind_CREATE2:
    return ((struct tuple_uint_16_uint_16){.tup0 = UINT16_C(4), .tup1 = UINT16_C(1)});
  }
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute_opcode(struct ast op, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  switch (op.kind) {
  case Kind_STOP:
    execute_stop();
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = mem, .tup3 = g});
  case Kind_ADD:
  {
    struct tuple_bits_64_uint_64 execute_add_result_2_976 = execute_add(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_add_result_2_976.tup0, .tup2 = mem, .tup3 = execute_add_result_2_976.tup1});
  }
  case Kind_MUL:
  {
    struct tuple_bits_64_uint_64 execute_mul_result_2_977 = execute_mul(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mul_result_2_977.tup0, .tup2 = mem, .tup3 = execute_mul_result_2_977.tup1});
  }
  case Kind_SUB:
  {
    struct tuple_bits_64_uint_64 execute_sub_result_2_978 = execute_sub(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sub_result_2_978.tup0, .tup2 = mem, .tup3 = execute_sub_result_2_978.tup1});
  }
  case Kind_DIV:
  {
    struct tuple_bits_64_uint_64 execute_div_result_2_979 = execute_div(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_div_result_2_979.tup0, .tup2 = mem, .tup3 = execute_div_result_2_979.tup1});
  }
  case Kind_SDIV:
  {
    struct tuple_bits_64_uint_64 execute_sdiv_result_2_980 = execute_sdiv(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sdiv_result_2_980.tup0, .tup2 = mem, .tup3 = execute_sdiv_result_2_980.tup1});
  }
  case Kind_MOD:
  {
    struct tuple_bits_64_uint_64 execute_mod_result_2_981 = execute_mod(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mod_result_2_981.tup0, .tup2 = mem, .tup3 = execute_mod_result_2_981.tup1});
  }
  case Kind_SMOD:
  {
    struct tuple_bits_64_uint_64 execute_smod_result_2_982 = execute_smod(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_smod_result_2_982.tup0, .tup2 = mem, .tup3 = execute_smod_result_2_982.tup1});
  }
  case Kind_ADDMOD:
  {
    struct tuple_bits_64_uint_64 execute_addmod_result_2_983 = execute_addmod(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_addmod_result_2_983.tup0, .tup2 = mem, .tup3 = execute_addmod_result_2_983.tup1});
  }
  case Kind_MULMOD:
  {
    struct tuple_bits_64_uint_64 execute_mulmod_result_2_984 = execute_mulmod(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mulmod_result_2_984.tup0, .tup2 = mem, .tup3 = execute_mulmod_result_2_984.tup1});
  }
  case Kind_EXP:
  {
    struct tuple_bits_64_uint_64 execute_exp_result_2_985 = execute_exp(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_exp_result_2_985.tup0, .tup2 = mem, .tup3 = execute_exp_result_2_985.tup1});
  }
  case Kind_SIGNEXTEND:
  {
    struct tuple_bits_64_uint_64 execute_signextend_result_2_986 = execute_signextend(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_signextend_result_2_986.tup0, .tup2 = mem, .tup3 = execute_signextend_result_2_986.tup1});
  }
  case Kind_LT:
  {
    struct tuple_bits_64_uint_64 execute_lt_result_2_987 = execute_lt(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_lt_result_2_987.tup0, .tup2 = mem, .tup3 = execute_lt_result_2_987.tup1});
  }
  case Kind_GT:
  {
    struct tuple_bits_64_uint_64 execute_gt_result_2_988 = execute_gt(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_gt_result_2_988.tup0, .tup2 = mem, .tup3 = execute_gt_result_2_988.tup1});
  }
  case Kind_SLT:
  {
    struct tuple_bits_64_uint_64 execute_slt_result_2_989 = execute_slt(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_slt_result_2_989.tup0, .tup2 = mem, .tup3 = execute_slt_result_2_989.tup1});
  }
  case Kind_SGT:
  {
    struct tuple_bits_64_uint_64 execute_sgt_result_2_990 = execute_sgt(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sgt_result_2_990.tup0, .tup2 = mem, .tup3 = execute_sgt_result_2_990.tup1});
  }
  case Kind_EQ:
  {
    struct tuple_bits_64_uint_64 execute_eq_result_2_991 = execute_eq(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_eq_result_2_991.tup0, .tup2 = mem, .tup3 = execute_eq_result_2_991.tup1});
  }
  case Kind_ISZERO:
  {
    struct tuple_bits_64_uint_64 execute_iszero_result_2_992 = execute_iszero(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_iszero_result_2_992.tup0, .tup2 = mem, .tup3 = execute_iszero_result_2_992.tup1});
  }
  case Kind_AND:
  {
    struct tuple_bits_64_uint_64 execute_and_result_2_993 = execute_and(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_and_result_2_993.tup0, .tup2 = mem, .tup3 = execute_and_result_2_993.tup1});
  }
  case Kind_OR:
  {
    struct tuple_bits_64_uint_64 execute_or_result_2_994 = execute_or(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_or_result_2_994.tup0, .tup2 = mem, .tup3 = execute_or_result_2_994.tup1});
  }
  case Kind_XOR:
  {
    struct tuple_bits_64_uint_64 execute_xor_result_2_995 = execute_xor(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_xor_result_2_995.tup0, .tup2 = mem, .tup3 = execute_xor_result_2_995.tup1});
  }
  case Kind_NOT:
  {
    struct tuple_bits_64_uint_64 execute_not_result_2_996 = execute_not(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_not_result_2_996.tup0, .tup2 = mem, .tup3 = execute_not_result_2_996.tup1});
  }
  case Kind_BYTE:
  {
    struct tuple_bits_64_uint_64 execute_byte_result_2_997 = execute_byte(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_byte_result_2_997.tup0, .tup2 = mem, .tup3 = execute_byte_result_2_997.tup1});
  }
  case Kind_SHL:
  {
    struct tuple_bits_64_uint_64 execute_shl_result_2_998 = execute_shl(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_shl_result_2_998.tup0, .tup2 = mem, .tup3 = execute_shl_result_2_998.tup1});
  }
  case Kind_SHR:
  {
    struct tuple_bits_64_uint_64 execute_shr_result_2_999 = execute_shr(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_shr_result_2_999.tup0, .tup2 = mem, .tup3 = execute_shr_result_2_999.tup1});
  }
  case Kind_SAR:
  {
    struct tuple_bits_64_uint_64 execute_sar_result_2_1000 = execute_sar(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sar_result_2_1000.tup0, .tup2 = mem, .tup3 = execute_sar_result_2_1000.tup1});
  }
  case Kind_CLZ:
  {
    struct tuple_bits_64_uint_64 execute_clz_result_2_1001 = execute_clz(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_clz_result_2_1001.tup0, .tup2 = mem, .tup3 = execute_clz_result_2_1001.tup1});
  }
  case Kind_KECCAK256:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_keccak256_result_2_1002 = execute_keccak256(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_keccak256_result_2_1002.tup0, .tup2 = execute_keccak256_result_2_1002.tup1, .tup3 = execute_keccak256_result_2_1002.tup2});
  }
  case Kind_ADDRESS:
  {
    struct tuple_bits_64_uint_64 execute_address_result_2_1003 = execute_address(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_address_result_2_1003.tup0, .tup2 = mem, .tup3 = execute_address_result_2_1003.tup1});
  }
  case Kind_ORIGIN:
  {
    struct tuple_bits_64_uint_64 execute_origin_result_2_1004 = execute_origin(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_origin_result_2_1004.tup0, .tup2 = mem, .tup3 = execute_origin_result_2_1004.tup1});
  }
  case Kind_CALLER:
  {
    struct tuple_bits_64_uint_64 execute_caller_result_2_1005 = execute_caller(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_caller_result_2_1005.tup0, .tup2 = mem, .tup3 = execute_caller_result_2_1005.tup1});
  }
  case Kind_CALLVALUE:
  {
    struct tuple_bits_64_uint_64 execute_callvalue_result_2_1006 = execute_callvalue(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_callvalue_result_2_1006.tup0, .tup2 = mem, .tup3 = execute_callvalue_result_2_1006.tup1});
  }
  case Kind_GASPRICE:
  {
    struct tuple_bits_64_uint_64 execute_gasprice_result_2_1007 = execute_gasprice(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_gasprice_result_2_1007.tup0, .tup2 = mem, .tup3 = execute_gasprice_result_2_1007.tup1});
  }
  case Kind_CALLDATASIZE:
  {
    struct tuple_bits_64_uint_64 execute_calldatasize_result_2_1008 = execute_calldatasize(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_calldatasize_result_2_1008.tup0, .tup2 = mem, .tup3 = execute_calldatasize_result_2_1008.tup1});
  }
  case Kind_CALLDATALOAD:
  {
    struct tuple_bits_64_uint_64 execute_calldataload_result_2_1009 = execute_calldataload(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_calldataload_result_2_1009.tup0, .tup2 = mem, .tup3 = execute_calldataload_result_2_1009.tup1});
  }
  case Kind_CALLDATACOPY:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_calldatacopy_result_2_1010 = execute_calldatacopy(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_calldatacopy_result_2_1010.tup0, .tup2 = execute_calldatacopy_result_2_1010.tup1, .tup3 = execute_calldatacopy_result_2_1010.tup2});
  }
  case Kind_CODESIZE:
  {
    struct tuple_bits_64_uint_64 execute_codesize_result_2_1011 = execute_codesize(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_codesize_result_2_1011.tup0, .tup2 = mem, .tup3 = execute_codesize_result_2_1011.tup1});
  }
  case Kind_CODECOPY:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_codecopy_result_2_1012 = execute_codecopy(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_codecopy_result_2_1012.tup0, .tup2 = execute_codecopy_result_2_1012.tup1, .tup3 = execute_codecopy_result_2_1012.tup2});
  }
  case Kind_BALANCE:
  {
    struct tuple_bits_64_uint_64 execute_balance_result_2_1013 = execute_balance(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_balance_result_2_1013.tup0, .tup2 = mem, .tup3 = execute_balance_result_2_1013.tup1});
  }
  case Kind_SELFBALANCE:
  {
    struct tuple_bits_64_uint_64 execute_selfbalance_result_2_1014 = execute_selfbalance(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_selfbalance_result_2_1014.tup0, .tup2 = mem, .tup3 = execute_selfbalance_result_2_1014.tup1});
  }
  case Kind_EXTCODESIZE:
  {
    struct tuple_bits_64_uint_64 execute_extcodesize_result_2_1015 = execute_extcodesize(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_extcodesize_result_2_1015.tup0, .tup2 = mem, .tup3 = execute_extcodesize_result_2_1015.tup1});
  }
  case Kind_EXTCODECOPY:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_extcodecopy_result_2_1016 = execute_extcodecopy(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_extcodecopy_result_2_1016.tup0, .tup2 = execute_extcodecopy_result_2_1016.tup1, .tup3 = execute_extcodecopy_result_2_1016.tup2});
  }
  case Kind_EXTCODEHASH:
  {
    struct tuple_bits_64_uint_64 execute_extcodehash_result_2_1017 = execute_extcodehash(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_extcodehash_result_2_1017.tup0, .tup2 = mem, .tup3 = execute_extcodehash_result_2_1017.tup1});
  }
  case Kind_RETURNDATASIZE:
  {
    struct tuple_bits_64_uint_64 execute_returndatasize_result_2_1018 = execute_returndatasize(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_returndatasize_result_2_1018.tup0, .tup2 = mem, .tup3 = execute_returndatasize_result_2_1018.tup1});
  }
  case Kind_RETURNDATACOPY:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_returndatacopy_result_2_1019 = execute_returndatacopy(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_returndatacopy_result_2_1019.tup0, .tup2 = execute_returndatacopy_result_2_1019.tup1, .tup3 = execute_returndatacopy_result_2_1019.tup2});
  }
  case Kind_BLOCKHASH:
  {
    struct tuple_bits_64_uint_64 execute_blockhash_result_2_1020 = execute_blockhash(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_blockhash_result_2_1020.tup0, .tup2 = mem, .tup3 = execute_blockhash_result_2_1020.tup1});
  }
  case Kind_COINBASE:
  {
    struct tuple_bits_64_uint_64 execute_coinbase_result_2_1021 = execute_coinbase(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_coinbase_result_2_1021.tup0, .tup2 = mem, .tup3 = execute_coinbase_result_2_1021.tup1});
  }
  case Kind_TIMESTAMP:
  {
    struct tuple_bits_64_uint_64 execute_timestamp_result_2_1022 = execute_timestamp(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_timestamp_result_2_1022.tup0, .tup2 = mem, .tup3 = execute_timestamp_result_2_1022.tup1});
  }
  case Kind_NUMBER:
  {
    struct tuple_bits_64_uint_64 execute_number_result_2_1023 = execute_number(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_number_result_2_1023.tup0, .tup2 = mem, .tup3 = execute_number_result_2_1023.tup1});
  }
  case Kind_SLOTNUM:
  {
    struct tuple_bits_64_uint_64 execute_slotnum_result_2_1024 = execute_slotnum(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_slotnum_result_2_1024.tup0, .tup2 = mem, .tup3 = execute_slotnum_result_2_1024.tup1});
  }
  case Kind_PREVRANDAO:
  {
    struct tuple_bits_64_uint_64 execute_prevrandao_result_2_1025 = execute_prevrandao(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_prevrandao_result_2_1025.tup0, .tup2 = mem, .tup3 = execute_prevrandao_result_2_1025.tup1});
  }
  case Kind_GASLIMIT:
  {
    struct tuple_bits_64_uint_64 execute_gaslimit_result_2_1026 = execute_gaslimit(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_gaslimit_result_2_1026.tup0, .tup2 = mem, .tup3 = execute_gaslimit_result_2_1026.tup1});
  }
  case Kind_CHAINID:
  {
    struct tuple_bits_64_uint_64 execute_chainid_result_2_1027 = execute_chainid(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_chainid_result_2_1027.tup0, .tup2 = mem, .tup3 = execute_chainid_result_2_1027.tup1});
  }
  case Kind_BASEFEE:
  {
    struct tuple_bits_64_uint_64 execute_basefee_result_2_1028 = execute_basefee(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_basefee_result_2_1028.tup0, .tup2 = mem, .tup3 = execute_basefee_result_2_1028.tup1});
  }
  case Kind_BLOBBASEFEE:
  {
    struct tuple_bits_64_uint_64 execute_blobbasefee_result_2_1029 = execute_blobbasefee(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_blobbasefee_result_2_1029.tup0, .tup2 = mem, .tup3 = execute_blobbasefee_result_2_1029.tup1});
  }
  case Kind_BLOBHASH:
  {
    struct tuple_bits_64_uint_64 execute_blobhash_result_2_1030 = execute_blobhash(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_blobhash_result_2_1030.tup0, .tup2 = mem, .tup3 = execute_blobhash_result_2_1030.tup1});
  }
  case Kind_POP:
  {
    struct tuple_bits_64_uint_64 execute_pop_result_2_1031 = execute_pop(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_pop_result_2_1031.tup0, .tup2 = mem, .tup3 = execute_pop_result_2_1031.tup1});
  }
  case Kind_MLOAD:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_mload_result_2_1032 = execute_mload(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mload_result_2_1032.tup0, .tup2 = execute_mload_result_2_1032.tup1, .tup3 = execute_mload_result_2_1032.tup2});
  }
  case Kind_MSTORE:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_mstore_result_2_1033 = execute_mstore(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mstore_result_2_1033.tup0, .tup2 = execute_mstore_result_2_1033.tup1, .tup3 = execute_mstore_result_2_1033.tup2});
  }
  case Kind_MSTORE8:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_mstore8_result_2_1034 = execute_mstore8(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mstore8_result_2_1034.tup0, .tup2 = execute_mstore8_result_2_1034.tup1, .tup3 = execute_mstore8_result_2_1034.tup2});
  }
  case Kind_MSIZE:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_msize_result_2_1035 = execute_msize(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_msize_result_2_1035.tup0, .tup2 = execute_msize_result_2_1035.tup1, .tup3 = execute_msize_result_2_1035.tup2});
  }
  case Kind_MCOPY:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_mcopy_result_2_1036 = execute_mcopy(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_mcopy_result_2_1036.tup0, .tup2 = execute_mcopy_result_2_1036.tup1, .tup3 = execute_mcopy_result_2_1036.tup2});
  }
  case Kind_SLOAD:
  {
    struct tuple_bits_64_uint_64 execute_sload_result_2_1037 = execute_sload_(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sload_result_2_1037.tup0, .tup2 = mem, .tup3 = execute_sload_result_2_1037.tup1});
  }
  case Kind_SSTORE:
  {
    struct tuple_bits_64_uint_64 execute_sstore_result_2_1038 = execute_sstore_(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_sstore_result_2_1038.tup0, .tup2 = mem, .tup3 = execute_sstore_result_2_1038.tup1});
  }
  case Kind_TLOAD:
  {
    struct tuple_bits_64_uint_64 execute_tload_result_2_1039 = execute_tload(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_tload_result_2_1039.tup0, .tup2 = mem, .tup3 = execute_tload_result_2_1039.tup1});
  }
  case Kind_TSTORE:
  {
    struct tuple_bits_64_uint_64 execute_tstore_result_2_1040 = execute_tstore(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_tstore_result_2_1040.tup0, .tup2 = mem, .tup3 = execute_tstore_result_2_1040.tup1});
  }
  case Kind_JUMP:
  {
    struct tuple_uint_32_bits_64_uint_64 execute_jump_result_2_1041 = execute_jump(pc_in, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = execute_jump_result_2_1041.tup0, .tup1 = execute_jump_result_2_1041.tup1, .tup2 = mem, .tup3 = execute_jump_result_2_1041.tup2});
  }
  case Kind_JUMPI:
  {
    struct tuple_uint_32_bits_64_uint_64 execute_jumpi_result_2_1042 = execute_jumpi(pc_in, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = execute_jumpi_result_2_1042.tup0, .tup1 = execute_jumpi_result_2_1042.tup1, .tup2 = mem, .tup3 = execute_jumpi_result_2_1042.tup2});
  }
  case Kind_PC:
  {
    struct tuple_bits_64_uint_64 execute_pc_result_2_1043 = execute_pc(pc_in, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_pc_result_2_1043.tup0, .tup2 = mem, .tup3 = execute_pc_result_2_1043.tup1});
  }
  case Kind_GAS:
  {
    struct tuple_bits_64_uint_64 execute_gas_result_2_1044 = execute_gas(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_gas_result_2_1044.tup0, .tup2 = mem, .tup3 = execute_gas_result_2_1044.tup1});
  }
  case Kind_JUMPDEST:
  {
    uint64_t execute_jumpdest_result_2_1045 = execute_jumpdest(g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = mem, .tup3 = execute_jumpdest_result_2_1045});
  }
  case Kind_PUSH:
  {
    struct tuple_bits_64_uint_64 execute_push_result_2_1046 = execute_push(op.variants.PUSH.tup0, op.variants.PUSH.tup1, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_push_result_2_1046.tup0, .tup2 = mem, .tup3 = execute_push_result_2_1046.tup1});
  }
  case Kind_DUP:
  {
    struct tuple_bits_64_uint_64 execute_dup_result_2_1047 = execute_dup(op.variants.DUP, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_dup_result_2_1047.tup0, .tup2 = mem, .tup3 = execute_dup_result_2_1047.tup1});
  }
  case Kind_SWAP:
  {
    struct tuple_bits_64_uint_64 execute_swap_result_2_1048 = execute_swap(op.variants.SWAP, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_swap_result_2_1048.tup0, .tup2 = mem, .tup3 = execute_swap_result_2_1048.tup1});
  }
  case Kind_DUPN:
  {
    struct tuple_bits_64_uint_64 execute_dupn_result_2_1049 = execute_dupn(op.variants.DUPN, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_dupn_result_2_1049.tup0, .tup2 = mem, .tup3 = execute_dupn_result_2_1049.tup1});
  }
  case Kind_SWAPN:
  {
    struct tuple_bits_64_uint_64 execute_swapn_result_2_1050 = execute_swapn(op.variants.SWAPN, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_swapn_result_2_1050.tup0, .tup2 = mem, .tup3 = execute_swapn_result_2_1050.tup1});
  }
  case Kind_EXCHANGE:
  {
    struct tuple_bits_64_uint_64 execute_exchange_result_2_1051 = execute_exchange(op.variants.EXCHANGE, top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_exchange_result_2_1051.tup0, .tup2 = mem, .tup3 = execute_exchange_result_2_1051.tup1});
  }
  case Kind_LOG:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_log_result_2_1052 = execute_log(op.variants.LOG, top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_log_result_2_1052.tup0, .tup2 = execute_log_result_2_1052.tup1, .tup3 = execute_log_result_2_1052.tup2});
  }
  case Kind_opcode_CREATE:
    return execute_create(pc_in, top, mem, g);
  case Kind_CREATE2:
    return execute_create2(pc_in, top, mem, g);
  case Kind_CALL:
    return execute_call(pc_in, top, mem, g);
  case Kind_CALLCODE:
    return execute_callcode(pc_in, top, mem, g);
  case Kind_DELEGATECALL:
    return execute_delegatecall(pc_in, top, mem, g);
  case Kind_STATICCALL:
    return execute_staticcall(pc_in, top, mem, g);
  case Kind_RETURN:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_return_result_2_1053 = execute_return(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_return_result_2_1053.tup0, .tup2 = execute_return_result_2_1053.tup1, .tup3 = execute_return_result_2_1053.tup2});
  }
  case Kind_REVERT:
  {
    struct tuple_bits_64_Bytes_uint_64 execute_revert_result_2_1054 = execute_revert(top, mem, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_revert_result_2_1054.tup0, .tup2 = execute_revert_result_2_1054.tup1, .tup3 = execute_revert_result_2_1054.tup2});
  }
  case Kind_INVALID:
  {
    uint64_t execute_invalid_result_2_1055 = execute_invalid(g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = mem, .tup3 = execute_invalid_result_2_1055});
  }
  case Kind_SELFDESTRUCT:
  {
    struct tuple_bits_64_uint_64 execute_selfdestruct_result_2_1056 = execute_selfdestruct(top, g);
    return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = execute_selfdestruct_result_2_1056.tup0, .tup2 = mem, .tup3 = execute_selfdestruct_result_2_1056.tup1});
  }
  }
}

struct tuple_uint_32_bits_64_Bytes_uint_64 execute(struct ast op, uint32_t pc_in, uint64_t top, Bytes mem, uint64_t g)
{
  struct tuple_uint_16_uint_16 opcode_stack_effect_result_2_974 = opcode_stack_effect(op);
  struct tuple_bool_uint_64 validate_stack_result_2_975 = validate_stack(g, top, opcode_stack_effect_result_2_974.tup0, opcode_stack_effect_result_2_974.tup1);
  if (validate_stack_result_2_975.tup0) {
    return execute_opcode(op, pc_in, top, mem, validate_stack_result_2_975.tup1);
  }
  return ((struct tuple_uint_32_bits_64_Bytes_uint_64){.tup0 = pc_in, .tup1 = top, .tup2 = mem, .tup3 = validate_stack_result_2_975.tup1});
}

struct tuple_bits_64_uint_64 execute_dup(uint8_t n, uint64_t top, uint64_t g)
{
  struct tuple_bool_uint_64 charge_result_2_1248 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(g, G_verylow);
  uint64_t g1 = charge_result_2_1248.tup1;
  if (!charge_result_2_1248.tup0) {
    return ((struct tuple_bits_64_uint_64){.tup0 = top, .tup1 = g1});
  }
  u256 value = peek_uint64_t_uint8_t_to_u256(top, ((uint8_t)((uint32_t)n - (uint32_t)UINT8_C(1))));
  uint64_t push_word_result_2_1250 = push_word(top, value);
  return ((struct tuple_bits_64_uint_64){.tup0 = push_word_result_2_1250, .tup1 = g1});
}

