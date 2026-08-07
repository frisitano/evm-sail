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
