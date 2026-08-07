/*
 * Optimized fetch/decode/run loop.
 *
 * The readable Sail decoder and opcode bodies remain the standard
 * implementation. This optimized override executes stack-local opcodes
 * against the C stack window and calls their generated Sail ALU functions
 * directly. Stateful, control-flow, call, and create opcodes continue to use
 * the generated Sail handlers below.
 */
#include "evmsail/prelude.h"

#include "evmsail/primitives/chain_config.h"

#include "evmsail/host/region_access.h"
#include "evmsail/host/frame_stack.h"
#include "evmsail/spec/primitives/code.h"
#include "evmsail/spec/evm/machine.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/internal.h"
#include "evmsail/spec/evm/gas.h"
#include "evmsail/spec/prelude.h"
#include "evmsail/spec/evm/execute.h"
#include "evmsail/spec/evm/halt.h"
#include "evmsail/spec/primitives/evm.h"
#include "evmsail/spec/evm/interpreter.h"
#include "host/stack_ops.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static Bytes empty_slice(void)
{
  return (Bytes){
      .len = 0,
      .bytes = NULL,
  };
}

static u256 read_push(const struct CodeFields *code, uint32_t offset, uint8_t width)
{
  u256 value = {{0, 0, 0, 0}};
  uint32_t available = offset < code->len ? code->len - offset : 0;
  if (available > width) {
    available = width;
  }

  /* EVM immediates are big-endian; u256 stores least-significant limbs
   * first. Bytes beyond the end of code remain zero as required by PUSH. */
  for (uint32_t i = 0; i < available; i++) {
    uint32_t byte_from_low = width - 1U - i;
    value.limbs[byte_from_low / 8] |= (uint64_t)code->bytes[offset + i]
                                      << (8 * (byte_from_low % 8));
  }
  return value;
}

#define EXECUTE(name) ((void)execute_##name())

enum {
  GAS_BASE = 2,
  GAS_VERYLOW = 3,
  GAS_LOW = 5,
  GAS_MID = 8,
};

/* Reached only from the interpret() Kind_Running dispatch, with no halting
 * operation between that test and the charge. */
static bool charge_opcode(uint64_t amount)
{
  if (amount == 0) {
    return true;
  }
  if (amount <= gas_remaining) {
    gas_remaining -= amount;
    return true;
  }
  exc_halt(OutOfGas);
  return false;
}

static bool stack_operation_succeeded(enum stack_rewrite_status status)
{
  if (status == EVMSAIL_STACK_REWRITE_OK) {
    return true;
  }
  exc_halt(status == EVMSAIL_STACK_REWRITE_UNDERFLOW ? StackUnderflow : StackOverflow);
  return false;
}

#define EXECUTE_STACK(name, inputs, outputs)                                                       \
  do {                                                                                             \
    if (stack_operation_succeeded(stack_validate(inputs, outputs)))                                \
      EXECUTE(name);                                                                               \
  } while (0)

#define EXECUTE_STACK_CALL(call, inputs, outputs)                                                  \
  do {                                                                                             \
    if (stack_operation_succeeded(stack_validate(inputs, outputs)))                                \
      (call);                                                                                      \
  } while (0)

/* CALL- and CREATE-family handlers install a child frame only on their
 * successful frame-entry path. Update the optimized host's private account
 * context exactly at that semantic boundary; failed calls leave call_depth
 * and the parent context unchanged. */
#define EXECUTE_STACK_FRAME_ENTRY(name, inputs, outputs)                                           \
  do {                                                                                             \
    uint16_t parent_depth = call_depth;                                                            \
    EXECUTE_STACK(name, inputs, outputs);                                                          \
    if (call_depth != parent_depth)                                                                \
      current_account_context_enter(message.address);                                              \
  } while (0)

/* The opcode set is closed. Expand the three stack shapes at each call site so
 * the selected ALU operation remains a direct call and can be inlined. */
#define EXECUTE_UNARY(gas, operation)                                                              \
  do {                                                                                             \
    if (!charge_opcode(gas))                                                                       \
      break;                                                                                       \
    u256 *rows = NULL;                                                                             \
    if (stack_operation_succeeded(stack_rewrite(1, 1, &rows)))                                     \
      rows[0] = operation(rows[0]);                                                                \
  } while (0)

#define EXECUTE_BINARY(gas, operation)                                                             \
  do {                                                                                             \
    if (!charge_opcode(gas))                                                                       \
      break;                                                                                       \
    u256 *rows = NULL;                                                                             \
    if (stack_operation_succeeded(stack_rewrite(2, 1, &rows)))                                     \
      rows[0] = operation(rows[1], rows[0]);                                                       \
  } while (0)

#define EXECUTE_TERNARY(gas, operation)                                                            \
  do {                                                                                             \
    if (!charge_opcode(gas))                                                                       \
      break;                                                                                       \
    u256 *rows = NULL;                                                                             \
    if (stack_operation_succeeded(stack_rewrite(3, 1, &rows)))                                     \
      rows[0] = operation(rows[2], rows[1], rows[0]);                                              \
  } while (0)

static void interpreter_execute_exp(void)
{
  u256 *rows = NULL;
  if (!stack_operation_succeeded(stack_rewrite(2, 1, &rows))) {
    return;
  }
  if (!charge_opcode(exp_gas(rows[0]))) {
    return;
  }
  rows[0] = alu_exp(rows[1], rows[0]);
}

static void interpreter_execute_push(uint8_t width, u256 value)
{
  if (!charge_opcode(width == 0 ? GAS_BASE : GAS_VERYLOW)) {
    return;
  }
  u256 *rows = NULL;
  if (!stack_operation_succeeded(stack_rewrite(0, 1, &rows))) {
    return;
  }
  rows[0] = value;
}

static void interpreter_execute_dup(uint64_t depth)
{
  if (!charge_opcode(GAS_VERYLOW)) {
    return;
  }
  (void)stack_operation_succeeded(stack_dup((uint32_t)depth));
}

static void interpreter_execute_swap(uint64_t depth)
{
  if (!charge_opcode(GAS_VERYLOW)) {
    return;
  }
  (void)stack_operation_succeeded(stack_swap((uint32_t)depth));
}

static void interpreter_execute_pop(void)
{
  if (!charge_opcode(GAS_BASE)) {
    return;
  }
  u256 *rows = NULL;
  (void)stack_operation_succeeded(stack_rewrite(1, 0, &rows));
}

static uint32_t interpreter_deep_stack_index(uint8_t immediate)
{
  return immediate <= 90 ? (uint32_t)immediate + 145 : (uint32_t)immediate - 111;
}

/* interpret() computes each deep-stack immediate's validity once and passes
 * the verdict down, matching the single Sail computation. */
static void interpreter_execute_dupn(uint8_t immediate, bool valid)
{
  if (!valid) {
    exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) {
    return;
  }
  (void)stack_operation_succeeded(stack_dup(interpreter_deep_stack_index(immediate)));
}

static void interpreter_execute_swapn(uint8_t immediate, bool valid)
{
  if (!valid) {
    exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) {
    return;
  }
  (void)stack_operation_succeeded(stack_swap(interpreter_deep_stack_index(immediate)));
}

static void interpreter_execute_exchange(uint8_t immediate, bool valid)
{
  if (!valid) {
    exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) {
    return;
  }

  const uint8_t shifted = immediate ^ 0x8f;
  const uint32_t quotient = shifted >> 4;
  const uint32_t remainder = shifted & 0x0f;
  const uint32_t left = quotient < remainder ? quotient + 1 : remainder + 1;
  const uint32_t right = quotient < remainder ? remainder + 1 : 29 - quotient;
  (void)stack_operation_succeeded(stack_exchange(left, right));
}

static void interpreter_execute_log(uint8_t topics)
{
  EXECUTE_STACK_CALL((void)execute_log(topics), topics + 2U, 0);
}

static void interpreter_execute_invalid(void)
{
  EXECUTE(invalid);
}

#ifndef EVMSAIL_INTERP_THREADED
static void interpreter_execute_simple(uint8_t opcode)
{
  if (opcode >= 0x80 && opcode <= 0x8f) {
    interpreter_execute_dup((uint64_t)opcode - 0x7f);
    return;
  }
  if (opcode >= 0x90 && opcode <= 0x9f) {
    interpreter_execute_swap((uint64_t)opcode - 0x8f);
    return;
  }
  if (opcode >= 0xa0 && opcode <= 0xa4) {
    interpreter_execute_log((uint8_t)(opcode - 0xa0));
    return;
  }

  switch (opcode) {
  case 0x00:
    EXECUTE(stop);
    return;
  case 0x01:
    EXECUTE_BINARY(GAS_VERYLOW, alu_add);
    return;
  case 0x02:
    EXECUTE_BINARY(GAS_LOW, alu_mul);
    return;
  case 0x03:
    EXECUTE_BINARY(GAS_VERYLOW, alu_sub);
    return;
  case 0x04:
    EXECUTE_BINARY(GAS_LOW, alu_div);
    return;
  case 0x05:
    EXECUTE_BINARY(GAS_LOW, alu_sdiv);
    return;
  case 0x06:
    EXECUTE_BINARY(GAS_LOW, alu_mod);
    return;
  case 0x07:
    EXECUTE_BINARY(GAS_LOW, alu_smod);
    return;
  case 0x08:
    EXECUTE_TERNARY(GAS_MID, alu_addmod);
    return;
  case 0x09:
    EXECUTE_TERNARY(GAS_MID, alu_mulmod);
    return;
  case 0x0a:
    interpreter_execute_exp();
    return;
  case 0x0b:
    EXECUTE_BINARY(GAS_LOW, alu_signextend);
    return;
  case 0x10:
    EXECUTE_BINARY(GAS_VERYLOW, alu_lt);
    return;
  case 0x11:
    EXECUTE_BINARY(GAS_VERYLOW, alu_gt);
    return;
  case 0x12:
    EXECUTE_BINARY(GAS_VERYLOW, alu_slt);
    return;
  case 0x13:
    EXECUTE_BINARY(GAS_VERYLOW, alu_sgt);
    return;
  case 0x14:
    EXECUTE_BINARY(GAS_VERYLOW, alu_eq);
    return;
  case 0x15:
    EXECUTE_UNARY(GAS_VERYLOW, alu_iszero);
    return;
  case 0x16:
    EXECUTE_BINARY(GAS_VERYLOW, alu_and);
    return;
  case 0x17:
    EXECUTE_BINARY(GAS_VERYLOW, alu_or);
    return;
  case 0x18:
    EXECUTE_BINARY(GAS_VERYLOW, alu_xor);
    return;
  case 0x19:
    EXECUTE_UNARY(GAS_VERYLOW, alu_not);
    return;
  case 0x1a:
    EXECUTE_BINARY(GAS_VERYLOW, alu_byte);
    return;
  case 0x1b:
    EXECUTE_BINARY(GAS_VERYLOW, alu_shl);
    return;
  case 0x1c:
    EXECUTE_BINARY(GAS_VERYLOW, alu_shr);
    return;
  case 0x1d:
    EXECUTE_BINARY(GAS_VERYLOW, alu_sar);
    return;
  case 0x1e:
    if (active_fork() >= EVMSAIL_FORK_OSAKA) {
      EXECUTE_UNARY(GAS_LOW, alu_clz);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x20:
    EXECUTE_STACK(keccak256, 2, 1);
    return;
  case 0x30:
    EXECUTE_STACK(address, 0, 1);
    return;
  case 0x31:
    EXECUTE_STACK(balance, 1, 1);
    return;
  case 0x32:
    EXECUTE_STACK(origin, 0, 1);
    return;
  case 0x33:
    EXECUTE_STACK(caller, 0, 1);
    return;
  case 0x34:
    EXECUTE_STACK(callvalue, 0, 1);
    return;
  case 0x35:
    EXECUTE_STACK(calldataload, 1, 1);
    return;
  case 0x36:
    EXECUTE_STACK(calldatasize, 0, 1);
    return;
  case 0x37:
    EXECUTE_STACK(calldatacopy, 3, 0);
    return;
  case 0x38:
    EXECUTE_STACK(codesize, 0, 1);
    return;
  case 0x39:
    EXECUTE_STACK(codecopy, 3, 0);
    return;
  case 0x3a:
    EXECUTE_STACK(gasprice, 0, 1);
    return;
  case 0x3b:
    EXECUTE_STACK(extcodesize, 1, 1);
    return;
  case 0x3c:
    EXECUTE_STACK(extcodecopy, 4, 0);
    return;
  case 0x3d:
    EXECUTE_STACK(returndatasize, 0, 1);
    return;
  case 0x3e:
    EXECUTE_STACK(returndatacopy, 3, 0);
    return;
  case 0x3f:
    EXECUTE_STACK(extcodehash, 1, 1);
    return;
  case 0x40:
    EXECUTE_STACK(blockhash, 1, 1);
    return;
  case 0x41:
    EXECUTE_STACK(coinbase, 0, 1);
    return;
  case 0x42:
    EXECUTE_STACK(timestamp, 0, 1);
    return;
  case 0x43:
    EXECUTE_STACK(number, 0, 1);
    return;
  case 0x44:
    EXECUTE_STACK(prevrandao, 0, 1);
    return;
  case 0x45:
    EXECUTE_STACK(gaslimit, 0, 1);
    return;
  case 0x46:
    EXECUTE_STACK(chainid, 0, 1);
    return;
  case 0x47:
    EXECUTE_STACK(selfbalance, 0, 1);
    return;
  case 0x48:
    if (active_fork() >= EVMSAIL_FORK_LONDON) {
      EXECUTE_STACK(basefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x49:
    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(blobhash, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x4a:
    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(blobbasefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x4b:
    if (active_fork() >= EVMSAIL_FORK_AMSTERDAM) {
      EXECUTE_STACK(slotnum, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x50:
    interpreter_execute_pop();
    return;
  case 0x51:
    EXECUTE_STACK(mload, 1, 1);
    return;
  case 0x52:
    EXECUTE_STACK(mstore, 2, 0);
    return;
  case 0x53:
    EXECUTE_STACK(mstore8, 2, 0);
    return;
  case 0x54:
    EXECUTE_STACK(sload, 1, 1);
    return;
  case 0x55:
    EXECUTE_STACK(sstore, 2, 0);
    return;
  case 0x56:
    EXECUTE_STACK(jump, 1, 0);
    return;
  case 0x57:
    EXECUTE_STACK(jumpi, 2, 0);
    return;
  case 0x58:
    EXECUTE_STACK(pc, 0, 1);
    return;
  case 0x59:
    EXECUTE_STACK(msize, 0, 1);
    return;
  case 0x5a:
    EXECUTE_STACK(gas, 0, 1);
    return;
  case 0x5b:
    EXECUTE(jumpdest);
    return;
  case 0x5c:
    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(tload, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x5d:
    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(tstore, 2, 0);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0x5e:
    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(mcopy, 3, 0);
    } else {
      interpreter_execute_invalid();
    }
    return;
  case 0xf0:
    EXECUTE_STACK_FRAME_ENTRY(create, 3, 1);
    return;
  case 0xf1:
    EXECUTE_STACK_FRAME_ENTRY(call, 7, 1);
    return;
  case 0xf2:
    EXECUTE_STACK_FRAME_ENTRY(callcode, 7, 1);
    return;
  case 0xf3:
    EXECUTE_STACK(return, 2, 0);
    return;
  case 0xf4:
    EXECUTE_STACK_FRAME_ENTRY(delegatecall, 6, 1);
    return;
  case 0xf5:
    EXECUTE_STACK_FRAME_ENTRY(create2, 4, 1);
    return;
  case 0xfa:
    EXECUTE_STACK_FRAME_ENTRY(staticcall, 6, 1);
    return;
  case 0xfd:
    EXECUTE_STACK(revert, 2, 0);
    return;
  case 0xff:
    EXECUTE_STACK(selfdestruct, 1, 0);
    return;
  default:
    interpreter_execute_invalid();
    return;
  }
}
#endif /* !EVMSAIL_INTERP_THREADED */

static Bytes frame_output(void)
{
  if (frame_status.kind != Kind_Halted) {
    return empty_slice();
  }

  switch (frame_status.variants.Halted.kind) {
  case Kind_HaltReturn:
    return frame_status.variants.Halted.variants.HaltReturn;
  case Kind_HaltRevert:
    return frame_status.variants.Halted.variants.HaltRevert;
  case Kind_HaltSelfDestruct:
  case Kind_HaltStop:
    return empty_slice();
  }
}


#ifdef EVMSAIL_INTERP_THREADED
/* Threaded-dispatch experiment: one 256-entry label table replaces the
 * range-check cascade, the switch bounds test, and the loop-back jump.
 * Semantics are byte-identical to the switch interpreter below. */
Bytes interpret(void)
{
  frame_stack_reset();
  current_account_context_enter(message.address);

  static const void *const interp_dispatch[256] = {
    [0x00] = &&op_00,
    [0x01] = &&op_01,
    [0x02] = &&op_02,
    [0x03] = &&op_03,
    [0x04] = &&op_04,
    [0x05] = &&op_05,
    [0x06] = &&op_06,
    [0x07] = &&op_07,
    [0x08] = &&op_08,
    [0x09] = &&op_09,
    [0x0a] = &&op_0a,
    [0x0b] = &&op_0b,
    [0x0c ... 0x0f] = &&op_invalid,
    [0x10] = &&op_10,
    [0x11] = &&op_11,
    [0x12] = &&op_12,
    [0x13] = &&op_13,
    [0x14] = &&op_14,
    [0x15] = &&op_15,
    [0x16] = &&op_16,
    [0x17] = &&op_17,
    [0x18] = &&op_18,
    [0x19] = &&op_19,
    [0x1a] = &&op_1a,
    [0x1b] = &&op_1b,
    [0x1c] = &&op_1c,
    [0x1d] = &&op_1d,
    [0x1e] = &&op_1e,
    [0x1f] = &&op_invalid,
    [0x20] = &&op_20,
    [0x21 ... 0x2f] = &&op_invalid,
    [0x30] = &&op_30,
    [0x31] = &&op_31,
    [0x32] = &&op_32,
    [0x33] = &&op_33,
    [0x34] = &&op_34,
    [0x35] = &&op_35,
    [0x36] = &&op_36,
    [0x37] = &&op_37,
    [0x38] = &&op_38,
    [0x39] = &&op_39,
    [0x3a] = &&op_3a,
    [0x3b] = &&op_3b,
    [0x3c] = &&op_3c,
    [0x3d] = &&op_3d,
    [0x3e] = &&op_3e,
    [0x3f] = &&op_3f,
    [0x40] = &&op_40,
    [0x41] = &&op_41,
    [0x42] = &&op_42,
    [0x43] = &&op_43,
    [0x44] = &&op_44,
    [0x45] = &&op_45,
    [0x46] = &&op_46,
    [0x47] = &&op_47,
    [0x48] = &&op_48,
    [0x49] = &&op_49,
    [0x4a] = &&op_4a,
    [0x4b] = &&op_4b,
    [0x4c ... 0x4f] = &&op_invalid,
    [0x50] = &&op_50,
    [0x51] = &&op_51,
    [0x52] = &&op_52,
    [0x53] = &&op_53,
    [0x54] = &&op_54,
    [0x55] = &&op_55,
    [0x56] = &&op_56,
    [0x57] = &&op_57,
    [0x58] = &&op_58,
    [0x59] = &&op_59,
    [0x5a] = &&op_5a,
    [0x5b] = &&op_5b,
    [0x5c] = &&op_5c,
    [0x5d] = &&op_5d,
    [0x5e] = &&op_5e,
    [0x5f] = &&op_push,
    [0x60] = &&op_push,
    [0x61] = &&op_push,
    [0x62] = &&op_push,
    [0x63] = &&op_push,
    [0x64] = &&op_push,
    [0x65] = &&op_push,
    [0x66] = &&op_push,
    [0x67] = &&op_push,
    [0x68] = &&op_push,
    [0x69] = &&op_push,
    [0x6a] = &&op_push,
    [0x6b] = &&op_push,
    [0x6c] = &&op_push,
    [0x6d] = &&op_push,
    [0x6e] = &&op_push,
    [0x6f] = &&op_push,
    [0x70] = &&op_push,
    [0x71] = &&op_push,
    [0x72] = &&op_push,
    [0x73] = &&op_push,
    [0x74] = &&op_push,
    [0x75] = &&op_push,
    [0x76] = &&op_push,
    [0x77] = &&op_push,
    [0x78] = &&op_push,
    [0x79] = &&op_push,
    [0x7a] = &&op_push,
    [0x7b] = &&op_push,
    [0x7c] = &&op_push,
    [0x7d] = &&op_push,
    [0x7e] = &&op_push,
    [0x7f] = &&op_push,
    [0x80] = &&op_dup,
    [0x81] = &&op_dup,
    [0x82] = &&op_dup,
    [0x83] = &&op_dup,
    [0x84] = &&op_dup,
    [0x85] = &&op_dup,
    [0x86] = &&op_dup,
    [0x87] = &&op_dup,
    [0x88] = &&op_dup,
    [0x89] = &&op_dup,
    [0x8a] = &&op_dup,
    [0x8b] = &&op_dup,
    [0x8c] = &&op_dup,
    [0x8d] = &&op_dup,
    [0x8e] = &&op_dup,
    [0x8f] = &&op_dup,
    [0x90] = &&op_swap,
    [0x91] = &&op_swap,
    [0x92] = &&op_swap,
    [0x93] = &&op_swap,
    [0x94] = &&op_swap,
    [0x95] = &&op_swap,
    [0x96] = &&op_swap,
    [0x97] = &&op_swap,
    [0x98] = &&op_swap,
    [0x99] = &&op_swap,
    [0x9a] = &&op_swap,
    [0x9b] = &&op_swap,
    [0x9c] = &&op_swap,
    [0x9d] = &&op_swap,
    [0x9e] = &&op_swap,
    [0x9f] = &&op_swap,
    [0xa0] = &&op_log,
    [0xa1] = &&op_log,
    [0xa2] = &&op_log,
    [0xa3] = &&op_log,
    [0xa4] = &&op_log,
    [0xa5 ... 0xe5] = &&op_invalid,
    [0xe6] = &&op_ext,
    [0xe7] = &&op_ext,
    [0xe8] = &&op_ext,
    [0xe9 ... 0xef] = &&op_invalid,
    [0xf0] = &&op_f0,
    [0xf1] = &&op_f1,
    [0xf2] = &&op_f2,
    [0xf3] = &&op_f3,
    [0xf4] = &&op_f4,
    [0xf5] = &&op_f5,
    [0xf6 ... 0xf9] = &&op_invalid,
    [0xfa] = &&op_fa,
    [0xfb ... 0xfc] = &&op_invalid,
    [0xfd] = &&op_fd,
    [0xfe] = &&op_invalid,
    [0xff] = &&op_ff,
  };

  uint8_t opcode;
  uint32_t immediate_offset;
  const struct CodeFields *code;

interp_next:
  if (frame_status.kind != Kind_Running) {
    Bytes output = frame_output();
    struct FrameContinuation continuation;
    memset(&continuation, 0, sizeof(continuation));
    continuation.kind = (enum kind_FrameContinuation)Kind_Empty;
    frame_stack_pop(&continuation);
    if (continuation.kind == Kind_Empty) {
      return output;
    }
    resume_frame(continuation, output);
    goto interp_next;
  }
  code = &frame_code;
  {
    uint32_t current = pc;
    if (current >= code->len) {
      EXECUTE(stop);
      goto interp_next;
    }
    opcode = code->bytes[current];
    immediate_offset = current + 1U;
    pc = immediate_offset;
  }
  goto *interp_dispatch[opcode];

  op_push:
  if (opcode == 0x5f && active_fork() < EVMSAIL_FORK_SHANGHAI) {
    interpreter_execute_invalid();
  } else {
    uint8_t width = (uint8_t)(opcode - 0x5f);
    pc = immediate_offset + width;
    interpreter_execute_push(width, read_push(code, immediate_offset, width));
  }
  goto interp_next;

  op_ext:
  if (active_fork() < EVMSAIL_FORK_AMSTERDAM) {
    goto op_invalid;
  }
  {
    uint8_t immediate = immediate_offset < code->len ? code->bytes[immediate_offset] : 0;
    bool valid = (opcode == 0xe8 ? (immediate <= 81 || immediate >= 128)
                                 : (immediate <= 90 || immediate >= 128)) != 0;
    if (valid) {
      pc = immediate_offset + 1;
    }
    if (opcode == 0xe6) {
      interpreter_execute_dupn(immediate, valid);
    } else if (opcode == 0xe7) {
      interpreter_execute_swapn(immediate, valid);
    } else {
      interpreter_execute_exchange(immediate, valid);
    }
  }
  goto interp_next;

  op_dup:
  interpreter_execute_dup((uint64_t)opcode - 0x7f);
  goto interp_next;

  op_swap:
  interpreter_execute_swap((uint64_t)opcode - 0x8f);
  goto interp_next;

  op_log:
  interpreter_execute_log((uint8_t)(opcode - 0xa0));
  goto interp_next;

  op_00:

    EXECUTE(stop);
    goto interp_next;
  op_01:

    EXECUTE_BINARY(GAS_VERYLOW, alu_add);
    goto interp_next;
  op_02:

    EXECUTE_BINARY(GAS_LOW, alu_mul);
    goto interp_next;
  op_03:

    EXECUTE_BINARY(GAS_VERYLOW, alu_sub);
    goto interp_next;
  op_04:

    EXECUTE_BINARY(GAS_LOW, alu_div);
    goto interp_next;
  op_05:

    EXECUTE_BINARY(GAS_LOW, alu_sdiv);
    goto interp_next;
  op_06:

    EXECUTE_BINARY(GAS_LOW, alu_mod);
    goto interp_next;
  op_07:

    EXECUTE_BINARY(GAS_LOW, alu_smod);
    goto interp_next;
  op_08:

    EXECUTE_TERNARY(GAS_MID, alu_addmod);
    goto interp_next;
  op_09:

    EXECUTE_TERNARY(GAS_MID, alu_mulmod);
    goto interp_next;
  op_0a:

    interpreter_execute_exp();
    goto interp_next;
  op_0b:

    EXECUTE_BINARY(GAS_LOW, alu_signextend);
    goto interp_next;
  op_10:

    EXECUTE_BINARY(GAS_VERYLOW, alu_lt);
    goto interp_next;
  op_11:

    EXECUTE_BINARY(GAS_VERYLOW, alu_gt);
    goto interp_next;
  op_12:

    EXECUTE_BINARY(GAS_VERYLOW, alu_slt);
    goto interp_next;
  op_13:

    EXECUTE_BINARY(GAS_VERYLOW, alu_sgt);
    goto interp_next;
  op_14:

    EXECUTE_BINARY(GAS_VERYLOW, alu_eq);
    goto interp_next;
  op_15:

    EXECUTE_UNARY(GAS_VERYLOW, alu_iszero);
    goto interp_next;
  op_16:

    EXECUTE_BINARY(GAS_VERYLOW, alu_and);
    goto interp_next;
  op_17:

    EXECUTE_BINARY(GAS_VERYLOW, alu_or);
    goto interp_next;
  op_18:

    EXECUTE_BINARY(GAS_VERYLOW, alu_xor);
    goto interp_next;
  op_19:

    EXECUTE_UNARY(GAS_VERYLOW, alu_not);
    goto interp_next;
  op_1a:

    EXECUTE_BINARY(GAS_VERYLOW, alu_byte);
    goto interp_next;
  op_1b:

    EXECUTE_BINARY(GAS_VERYLOW, alu_shl);
    goto interp_next;
  op_1c:

    EXECUTE_BINARY(GAS_VERYLOW, alu_shr);
    goto interp_next;
  op_1d:

    EXECUTE_BINARY(GAS_VERYLOW, alu_sar);
    goto interp_next;
  op_1e:

    if (active_fork() >= EVMSAIL_FORK_OSAKA) {
      EXECUTE_UNARY(GAS_LOW, alu_clz);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_20:

    EXECUTE_STACK(keccak256, 2, 1);
    goto interp_next;
  op_30:

    EXECUTE_STACK(address, 0, 1);
    goto interp_next;
  op_31:

    EXECUTE_STACK(balance, 1, 1);
    goto interp_next;
  op_32:

    EXECUTE_STACK(origin, 0, 1);
    goto interp_next;
  op_33:

    EXECUTE_STACK(caller, 0, 1);
    goto interp_next;
  op_34:

    EXECUTE_STACK(callvalue, 0, 1);
    goto interp_next;
  op_35:

    EXECUTE_STACK(calldataload, 1, 1);
    goto interp_next;
  op_36:

    EXECUTE_STACK(calldatasize, 0, 1);
    goto interp_next;
  op_37:

    EXECUTE_STACK(calldatacopy, 3, 0);
    goto interp_next;
  op_38:

    EXECUTE_STACK(codesize, 0, 1);
    goto interp_next;
  op_39:

    EXECUTE_STACK(codecopy, 3, 0);
    goto interp_next;
  op_3a:

    EXECUTE_STACK(gasprice, 0, 1);
    goto interp_next;
  op_3b:

    EXECUTE_STACK(extcodesize, 1, 1);
    goto interp_next;
  op_3c:

    EXECUTE_STACK(extcodecopy, 4, 0);
    goto interp_next;
  op_3d:

    EXECUTE_STACK(returndatasize, 0, 1);
    goto interp_next;
  op_3e:

    EXECUTE_STACK(returndatacopy, 3, 0);
    goto interp_next;
  op_3f:

    EXECUTE_STACK(extcodehash, 1, 1);
    goto interp_next;
  op_40:

    EXECUTE_STACK(blockhash, 1, 1);
    goto interp_next;
  op_41:

    EXECUTE_STACK(coinbase, 0, 1);
    goto interp_next;
  op_42:

    EXECUTE_STACK(timestamp, 0, 1);
    goto interp_next;
  op_43:

    EXECUTE_STACK(number, 0, 1);
    goto interp_next;
  op_44:

    EXECUTE_STACK(prevrandao, 0, 1);
    goto interp_next;
  op_45:

    EXECUTE_STACK(gaslimit, 0, 1);
    goto interp_next;
  op_46:

    EXECUTE_STACK(chainid, 0, 1);
    goto interp_next;
  op_47:

    EXECUTE_STACK(selfbalance, 0, 1);
    goto interp_next;
  op_48:

    if (active_fork() >= EVMSAIL_FORK_LONDON) {
      EXECUTE_STACK(basefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_49:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(blobhash, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_4a:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(blobbasefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_4b:

    if (active_fork() >= EVMSAIL_FORK_AMSTERDAM) {
      EXECUTE_STACK(slotnum, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_50:

    interpreter_execute_pop();
    goto interp_next;
  op_51:

    EXECUTE_STACK(mload, 1, 1);
    goto interp_next;
  op_52:

    EXECUTE_STACK(mstore, 2, 0);
    goto interp_next;
  op_53:

    EXECUTE_STACK(mstore8, 2, 0);
    goto interp_next;
  op_54:

    EXECUTE_STACK(sload, 1, 1);
    goto interp_next;
  op_55:

    EXECUTE_STACK(sstore, 2, 0);
    goto interp_next;
  op_56:

    EXECUTE_STACK(jump, 1, 0);
    goto interp_next;
  op_57:

    EXECUTE_STACK(jumpi, 2, 0);
    goto interp_next;
  op_58:

    EXECUTE_STACK(pc, 0, 1);
    goto interp_next;
  op_59:

    EXECUTE_STACK(msize, 0, 1);
    goto interp_next;
  op_5a:

    EXECUTE_STACK(gas, 0, 1);
    goto interp_next;
  op_5b:

    EXECUTE(jumpdest);
    goto interp_next;
  op_5c:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(tload, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_5d:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(tstore, 2, 0);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_5e:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      EXECUTE_STACK(mcopy, 3, 0);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_f0:

    EXECUTE_STACK_FRAME_ENTRY(create, 3, 1);
    goto interp_next;
  op_f1:

    EXECUTE_STACK_FRAME_ENTRY(call, 7, 1);
    goto interp_next;
  op_f2:

    EXECUTE_STACK_FRAME_ENTRY(callcode, 7, 1);
    goto interp_next;
  op_f3:

    EXECUTE_STACK(return, 2, 0);
    goto interp_next;
  op_f4:

    EXECUTE_STACK_FRAME_ENTRY(delegatecall, 6, 1);
    goto interp_next;
  op_f5:

    EXECUTE_STACK_FRAME_ENTRY(create2, 4, 1);
    goto interp_next;
  op_fa:

    EXECUTE_STACK_FRAME_ENTRY(staticcall, 6, 1);
    goto interp_next;
  op_fd:

    EXECUTE_STACK(revert, 2, 0);
    goto interp_next;
  op_ff:

    EXECUTE_STACK(selfdestruct, 1, 0);
    goto interp_next;
  op_invalid:

    interpreter_execute_invalid();
    goto interp_next;
}
#else
Bytes interpret(void)
{
  frame_stack_reset();
  current_account_context_enter(message.address);

  for (;;) {
    if (frame_status.kind == Kind_Running) {
      /* frame_code is a validated CodeRegionSlice. */
      const struct CodeFields *code = &frame_code;
      uint32_t current = pc;

      if (current >= code->len) {
        EXECUTE(stop);
        continue;
      }

      uint8_t opcode = code->bytes[current];
      uint32_t immediate_offset = current + 1U;
      pc = immediate_offset;

      if (opcode >= 0x5f && opcode <= 0x7f) {
        if (opcode == 0x5f && active_fork() < EVMSAIL_FORK_SHANGHAI) {
          interpreter_execute_invalid();
        } else {
          uint8_t width = (uint8_t)(opcode - 0x5f);
          pc = immediate_offset + width;
          interpreter_execute_push(width, read_push(code, immediate_offset, width));
        }
        continue;
      }

      if (active_fork() >= EVMSAIL_FORK_AMSTERDAM && opcode >= 0xe6 && opcode <= 0xe8) {
        uint8_t immediate = immediate_offset < code->len ? code->bytes[immediate_offset] : 0;
        bool valid = (opcode == 0xe8 ? (immediate <= 81 || immediate >= 128)
                                     : (immediate <= 90 || immediate >= 128)) != 0;
        if (valid) {
          pc = immediate_offset + 1;
        }
        if (opcode == 0xe6) {
          interpreter_execute_dupn(immediate, valid);
        } else if (opcode == 0xe7) {
          interpreter_execute_swapn(immediate, valid);
        } else {
          interpreter_execute_exchange(immediate, valid);
        }
        continue;
      }

      interpreter_execute_simple(opcode);
      continue;
    }

    Bytes output = frame_output();
    struct FrameContinuation continuation;
    memset(&continuation, 0, sizeof(continuation));
    continuation.kind = (enum kind_FrameContinuation)Kind_Empty;
    frame_stack_pop(&continuation);
    if (continuation.kind == Kind_Empty) {
      return output;
    }
    resume_frame(continuation, output);
  }
}
#endif /* EVMSAIL_INTERP_THREADED */
