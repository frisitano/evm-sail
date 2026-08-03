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

#include "evmsail/evm/interpreter.h"
#include "evmsail/primitives/chain_config.h"

#include "evmsail/host/region_access.h"
#include "evmsail/host/frame_stack.h"
#include "host/stack_ops.h"
#include "workspace.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct code_view {
  uint64_t off;
  uint64_t len;
  const uint8_t *bytes;
  bool valid;
};

static struct OutputSliceFields empty_slice(void) {
  return (struct OutputSliceFields){
      .len = 0,
      .off = 0,
  };
}

static bool same_code(const struct code_view *view,
                      const struct CodeRegionSliceFields *code) {
  return view->valid && view->off == code->off && view->len == code->len;
}

static void resolve_code(struct code_view *view) {
  const struct CodeRegionSliceFields *code = &frame_code.bytes;
  if (same_code(view, code)) return;

  const uint8_t *bytes = code_ptr(code->off, code->len);
  if (!bytes) GUEST_ABORT();

  view->off = code->off;
  view->len = code->len;
  view->bytes = bytes;
  view->valid = true;
}

static U256 read_push(const struct code_view *view, uint64_t offset,
                           uint64_t width) {
  U256 value = {{0, 0, 0, 0}};
  uint64_t available = offset < view->len ? view->len - offset : 0;
  if (available > width) available = width;

  /* EVM immediates are big-endian; U256 stores least-significant limbs
   * first. Bytes beyond the end of code remain zero as required by PUSH. */
  for (uint64_t i = 0; i < available; i++) {
    uint64_t byte_from_low = width - 1 - i;
    value.limbs[byte_from_low / 8] |=
        (uint64_t)view->bytes[offset + i] << (8 * (byte_from_low % 8));
  }
  return value;
}

#define EXECUTE(name) ((void)execute_##name(UNIT))
#define EXECUTE_CALLDATASIZE() ((void)execute_calldatasize(UNIT))
#define EXECUTE_CODESIZE() ((void)execute_codesize(UNIT))
#define EXECUTE_EXTCODESIZE() ((void)execute_extcodesize(UNIT))
#define EXECUTE_RETURNDATASIZE() ((void)execute_returndatasize(UNIT))
#define EXECUTE_MSIZE() ((void)execute_msize(UNIT))

enum {
  GAS_BASE = 2,
  GAS_VERYLOW = 3,
  GAS_LOW = 5,
  GAS_MID = 8,
};

static bool charge_opcode(uint64_t amount) {
  if (frame_status.kind != Kind_Running) return false;
  if (amount == 0) return true;
  if (amount <= gas_remaining) {
    gas_remaining -= amount;
    return true;
  } else {
    (void)exc_halt(OutOfGas);
    return false;
  }
}

static bool stack_operation_succeeded(
    enum stack_rewrite_status status) {
  if (status == EVMSAIL_STACK_REWRITE_OK) return true;
  (void)exc_halt(status == EVMSAIL_STACK_REWRITE_UNDERFLOW
                      ? StackUnderflow
                      : StackOverflow);
  return false;
}

#define EXECUTE_STACK(name, inputs, outputs)                                 \
  do {                                                                       \
    if (stack_operation_succeeded(stack_validate(inputs, outputs)))          \
      EXECUTE(name);                                                         \
  } while (0)

#define EXECUTE_STACK_CALL(call, inputs, outputs)                            \
  do {                                                                       \
    if (stack_operation_succeeded(stack_validate(inputs, outputs)))          \
      call;                                                                  \
  } while (0)

/* The opcode set is closed. Expand the three stack shapes at each call site so
 * the selected ALU operation remains a direct call and can be inlined. */
#define EXECUTE_UNARY(gas, operation)                                        \
  do {                                                                       \
    if (!charge_opcode(gas)) break;                                          \
    U256 *rows = NULL;                                                       \
    if (stack_operation_succeeded(stack_rewrite(1, 1, &rows)))               \
      rows[0] = operation(rows[0]);                                          \
  } while (0)

#define EXECUTE_BINARY(gas, operation)                                       \
  do {                                                                       \
    if (!charge_opcode(gas)) break;                                          \
    U256 *rows = NULL;                                                       \
    if (stack_operation_succeeded(stack_rewrite(2, 1, &rows)))               \
      rows[0] = operation(rows[1], rows[0]);                                 \
  } while (0)

#define EXECUTE_TERNARY(gas, operation)                                      \
  do {                                                                       \
    if (!charge_opcode(gas)) break;                                          \
    U256 *rows = NULL;                                                       \
    if (stack_operation_succeeded(stack_rewrite(3, 1, &rows)))               \
      rows[0] = operation(rows[2], rows[1], rows[0]);                        \
  } while (0)

static void interpreter_execute_exp(void) {
  U256 *rows = NULL;
  if (!stack_operation_succeeded(stack_rewrite(2, 1, &rows))) return;
  if (!charge_opcode(exp_gas(rows[0]))) return;
  rows[0] = alu_exp(rows[1], rows[0]);
}

static void interpreter_execute_push(uint64_t width, U256 value) {
  if (!charge_opcode(width == 0 ? GAS_BASE : GAS_VERYLOW)) return;
  U256 *rows = NULL;
  if (!stack_operation_succeeded(stack_rewrite(0, 1, &rows))) return;
  rows[0] = value;
}

static void interpreter_execute_dup(uint64_t depth) {
  if (!charge_opcode(GAS_VERYLOW)) return;
  (void)stack_operation_succeeded(stack_dup((uint32_t)depth));
}

static void interpreter_execute_swap(uint64_t depth) {
  if (!charge_opcode(GAS_VERYLOW)) return;
  (void)stack_operation_succeeded(stack_swap((uint32_t)depth));
}

static void interpreter_execute_pop(void) {
  if (!charge_opcode(GAS_BASE)) return;
  U256 *rows = NULL;
  (void)stack_operation_succeeded(stack_rewrite(1, 0, &rows));
}

static bool interpreter_deep_stack_immediate_valid(uint8_t immediate) {
  return immediate <= 90 || immediate >= 128;
}

static uint32_t interpreter_deep_stack_index(uint8_t immediate) {
  return immediate <= 90 ? (uint32_t)immediate + 145
                         : (uint32_t)immediate - 111;
}

static bool interpreter_exchange_immediate_valid(uint8_t immediate) {
  return immediate <= 81 || immediate >= 128;
}

static void interpreter_execute_dupn(uint8_t immediate) {
  if (!interpreter_deep_stack_immediate_valid(immediate)) {
    (void)exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) return;
  (void)stack_operation_succeeded(
      stack_dup(interpreter_deep_stack_index(immediate)));
}

static void interpreter_execute_swapn(uint8_t immediate) {
  if (!interpreter_deep_stack_immediate_valid(immediate)) {
    (void)exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) return;
  (void)stack_operation_succeeded(
      stack_swap(interpreter_deep_stack_index(immediate)));
}

static void interpreter_execute_exchange(uint8_t immediate) {
  if (!interpreter_exchange_immediate_valid(immediate)) {
    (void)exc_halt(InvalidOpcode);
    return;
  }
  if (!charge_opcode(GAS_VERYLOW)) return;

  const uint8_t shifted = immediate ^ 0x8f;
  const uint32_t quotient = shifted >> 4;
  const uint32_t remainder = shifted & 0x0f;
  const uint32_t left = quotient < remainder ? quotient + 1 : remainder + 1;
  const uint32_t right = quotient < remainder ? remainder + 1 : 29 - quotient;
  (void)stack_operation_succeeded(stack_exchange(left, right));
}

static void interpreter_execute_log(uint64_t topics) {
  EXECUTE_STACK_CALL((void)execute_log(topics), topics + 2u, 0);
}

static void interpreter_execute_invalid(void) { EXECUTE(invalid); }

static void interpreter_execute_simple(uint8_t opcode) {
  if (opcode >= 0x80 && opcode <= 0x8f) {
    interpreter_execute_dup((uint64_t)opcode - 0x7f);
    return;
  }
  if (opcode >= 0x90 && opcode <= 0x9f) {
    interpreter_execute_swap((uint64_t)opcode - 0x8f);
    return;
  }
  if (opcode >= 0xa0 && opcode <= 0xa4) {
    interpreter_execute_log((uint64_t)opcode - 0xa0);
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
      if (active_fork() >= EVMSAIL_FORK_OSAKA)
        EXECUTE_UNARY(GAS_LOW, alu_clz);
      else
        interpreter_execute_invalid();
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
      EXECUTE_STACK_CALL(EXECUTE_CALLDATASIZE(), 0, 1);
      return;
    case 0x37:
      EXECUTE_STACK(calldatacopy, 3, 0);
      return;
    case 0x38:
      EXECUTE_STACK_CALL(EXECUTE_CODESIZE(), 0, 1);
      return;
    case 0x39:
      EXECUTE_STACK(codecopy, 3, 0);
      return;
    case 0x3a:
      EXECUTE_STACK(gasprice, 0, 1);
      return;
    case 0x3b:
      EXECUTE_STACK_CALL(EXECUTE_EXTCODESIZE(), 1, 1);
      return;
    case 0x3c:
      EXECUTE_STACK(extcodecopy, 4, 0);
      return;
    case 0x3d:
      EXECUTE_STACK_CALL(EXECUTE_RETURNDATASIZE(), 0, 1);
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
      if (active_fork() >= EVMSAIL_FORK_LONDON)
        EXECUTE_STACK(basefee, 0, 1);
      else
        interpreter_execute_invalid();
      return;
    case 0x49:
      if (active_fork() >= EVMSAIL_FORK_CANCUN)
        EXECUTE_STACK(blobhash, 1, 1);
      else
        interpreter_execute_invalid();
      return;
    case 0x4a:
      if (active_fork() >= EVMSAIL_FORK_CANCUN)
        EXECUTE_STACK(blobbasefee, 0, 1);
      else
        interpreter_execute_invalid();
      return;
    case 0x4b:
      if (active_fork() >= EVMSAIL_FORK_AMSTERDAM)
        EXECUTE_STACK(slotnum, 0, 1);
      else
        interpreter_execute_invalid();
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
      EXECUTE_STACK_CALL(EXECUTE_MSIZE(), 0, 1);
      return;
    case 0x5a:
      EXECUTE_STACK(gas, 0, 1);
      return;
    case 0x5b:
      EXECUTE(jumpdest);
      return;
    case 0x5c:
      if (active_fork() >= EVMSAIL_FORK_CANCUN)
        EXECUTE_STACK(tload, 1, 1);
      else
        interpreter_execute_invalid();
      return;
    case 0x5d:
      if (active_fork() >= EVMSAIL_FORK_CANCUN)
        EXECUTE_STACK(tstore, 2, 0);
      else
        interpreter_execute_invalid();
      return;
    case 0x5e:
      if (active_fork() >= EVMSAIL_FORK_CANCUN)
        EXECUTE_STACK(mcopy, 3, 0);
      else
        interpreter_execute_invalid();
      return;
    case 0xf0:
      EXECUTE_STACK(create, 3, 1);
      return;
    case 0xf1:
      EXECUTE_STACK(call, 7, 1);
      return;
    case 0xf2:
      EXECUTE_STACK(callcode, 7, 1);
      return;
    case 0xf3:
      EXECUTE_STACK(return, 2, 0);
      return;
    case 0xf4:
      EXECUTE_STACK(delegatecall, 6, 1);
      return;
    case 0xf5:
      EXECUTE_STACK(create2, 4, 1);
      return;
    case 0xfa:
      EXECUTE_STACK(staticcall, 6, 1);
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

static struct OutputSliceFields frame_output(void) {
  if (frame_status.kind != Kind_Halted) return empty_slice();

  switch (frame_status.variants.Halted.kind) {
    case Kind_HaltReturn:
      return frame_status.variants.Halted.variants.HaltReturn;
    case Kind_HaltRevert:
      return frame_status.variants.Halted.variants.HaltRevert;
    default:
      return empty_slice();
  }
}

struct OutputSliceFields interpret(unit u) {
  struct code_view code = {0};
  (void)u;
  frame_stack_reset(UNIT);

  for (;;) {
    if (have_exception) return empty_slice();

    if (frame_status.kind == Kind_Running) {
      resolve_code(&code);
      uint64_t current = pc;

      if (current >= code.len) {
        EXECUTE(stop);
        continue;
      }

      uint8_t opcode = code.bytes[current];
      uint64_t immediate_offset = current + 1;
      pc = immediate_offset;

      if (opcode >= 0x5f && opcode <= 0x7f) {
        if (opcode == 0x5f &&
            active_fork() < EVMSAIL_FORK_SHANGHAI) {
          interpreter_execute_invalid();
        } else {
          uint64_t width = (uint64_t)opcode - 0x5f;
          pc = immediate_offset + width;
          interpreter_execute_push(width,
                                 read_push(&code, immediate_offset, width));
        }
        continue;
      }

      if (active_fork() >= EVMSAIL_FORK_AMSTERDAM &&
          opcode >= 0xe6 && opcode <= 0xe8) {
        uint64_t immediate =
            immediate_offset < code.len ? code.bytes[immediate_offset] : 0;
        bool valid = opcode == 0xe8
                         ? (immediate <= 81 || immediate >= 128)
                         : (immediate <= 90 || immediate >= 128);
        if (valid) pc = immediate_offset + 1;
        if (opcode == 0xe6)
          interpreter_execute_dupn(immediate);
        else if (opcode == 0xe7)
          interpreter_execute_swapn(immediate);
        else
          interpreter_execute_exchange(immediate);
        continue;
      }

      interpreter_execute_simple(opcode);
      continue;
    }

    struct OutputSliceFields output = frame_output();
    struct FrameContinuation continuation = {0};
    continuation.kind = Kind_Empty;
    frame_stack_pop(&continuation, UNIT);
    if (continuation.kind == Kind_Empty) return output;
    resume_frame(continuation, output);
  }
}
