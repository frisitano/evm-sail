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
  GAS_JUMPDEST = 1,
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
      ((void)execute_##name());                                                                               \
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

/* The opcode set is closed. Expand the three stack shapes at each call site so
 * the selected ALU operation remains a direct call and can be inlined. */

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

/* Threaded-dispatch experiment: one 256-entry label table replaces the
 * range-check cascade, the switch bounds test, and the loop-back jump.
 * Semantics are byte-identical to the switch interpreter below. */
Bytes interpret(void)
{
  frame_stack_reset();
  current_account_context_enter(message.address);

/* Loop-local machine state: pc and gas live in locals inside the dispatch
 * loop. Arms whose handlers are generated Sail bodies publish the locals to
 * the model registers first and reload after (T_STACK and friends). The
 * in-TU fast-path arms never touch the model registers and never call
 * exc_halt from inside an operation: every failure branches to the single
 * interp_exc tail, which publishes gas and performs the halt once. */
#define T_SYNC_IN()                                                                                \
  do {                                                                                             \
    pc = lpc;                                                                                      \
    gas_remaining = lgas;                                                                          \
  } while (0)

#define T_SYNC_OUT()                                                                               \
  do {                                                                                             \
    lgas = gas_remaining;                                                                          \
    lpc = pc;                                                                                      \
  } while (0)

#define T_EXECUTE(name)                                                                            \
  do {                                                                                             \
    T_SYNC_IN();                                                                                   \
    EXECUTE(name);                                                                                 \
    T_SYNC_OUT();                                                                                  \
  } while (0)

#define T_STACK(name, inputs, outputs)                                                             \
  do {                                                                                             \
    T_SYNC_IN();                                                                                   \
    EXECUTE_STACK(name, inputs, outputs);                                                          \
    T_SYNC_OUT();                                                                                  \
  } while (0)

#define T_STACK_FRAME_ENTRY(name, inputs, outputs)                                                 \
  do {                                                                                             \
    uint16_t parent_depth = call_depth;                                                            \
    T_STACK(name, inputs, outputs);                                                                \
    if (call_depth != parent_depth) {                                                              \
      current_account_context_enter(message.address);                                              \
      cbytes = frame_code.bytes;                                                                   \
      clen = frame_code.len;                                                                       \
    }                                                                                              \
  } while (0)

#define T_NEXT                                                                                     \
  do {                                                                                             \
    if (lpc >= clen)                                                                               \
      goto op_end_of_code;                                                                         \
    opcode = cbytes[lpc];                                                                          \
    immediate_offset = lpc + 1U;                                                                   \
    lpc = immediate_offset;                                                                        \
    goto *interp_dispatch[opcode];                                                                 \
  } while (0)

#define T_CHARGE(amount)                                                                           \
  do {                                                                                             \
    uint64_t charge_amount = (amount);                                                             \
    if (charge_amount > lgas) {                                                                    \
      halt_reason = OutOfGas;                                                                      \
      goto interp_exc;                                                                             \
    }                                                                                              \
    lgas -= charge_amount;                                                                         \
  } while (0)

#define T_STACKOP(status_expression)                                                               \
  do {                                                                                             \
    enum stack_rewrite_status stack_status = (status_expression);                                  \
    if (stack_status != EVMSAIL_STACK_REWRITE_OK) {                                                \
      halt_reason = stack_status == EVMSAIL_STACK_REWRITE_UNDERFLOW ? StackUnderflow               \
                                                                    : StackOverflow;               \
      goto interp_exc;                                                                             \
    }                                                                                              \
  } while (0)

#define T_UNARY(gas, operation)                                                                    \
  do {                                                                                             \
    T_CHARGE(gas);                                                                                 \
    u256 *rows = NULL;                                                                             \
    T_STACKOP(stack_rewrite(1, 1, &rows));                                                         \
    rows[0] = operation(rows[0]);                                                                  \
  } while (0);                                                                                     \
  T_NEXT

#define T_BINARY(gas, operation)                                                                   \
  do {                                                                                             \
    T_CHARGE(gas);                                                                                 \
    u256 *rows = NULL;                                                                             \
    T_STACKOP(stack_rewrite(2, 1, &rows));                                                         \
    rows[0] = operation(rows[1], rows[0]);                                                         \
  } while (0);                                                                                     \
  T_NEXT

#define T_TERNARY(gas, operation)                                                                  \
  do {                                                                                             \
    T_CHARGE(gas);                                                                                 \
    u256 *rows = NULL;                                                                             \
    T_STACKOP(stack_rewrite(3, 1, &rows));                                                         \
    rows[0] = operation(rows[2], rows[1], rows[0]);                                                \
  } while (0);                                                                                     \
  T_NEXT

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
  const struct CodeFields *code = &frame_code;
  uint32_t lpc = pc;
  uint64_t lgas = gas_remaining;
  const uint8_t *cbytes = frame_code.bytes;
  uint32_t clen = frame_code.len;
  enum ExceptionKind halt_reason;

interp_next:
  if (frame_status.kind != Kind_Running)
    goto interp_halted;
  T_NEXT;

interp_halted:
  {
    Bytes output = frame_output();
    struct FrameContinuation continuation = frame_stack_pop();
    if (continuation.kind == Kind_Empty) {
      return output;
    }
    resume_frame(continuation, output);
    lpc = pc;
    lgas = gas_remaining;
    cbytes = frame_code.bytes;
    clen = frame_code.len;
    goto interp_next;
  }

op_end_of_code:
  T_EXECUTE(stop);
  goto interp_next;

interp_exc:
  /* The single in-TU halt site: publish the local gas the halt semantics
   * read, then perform the canonical exceptional halt exactly once. */
  gas_remaining = lgas;
  exc_halt(halt_reason);
  goto interp_halted;

  op_push:
  if (opcode == 0x5f && active_fork() < EVMSAIL_FORK_SHANGHAI) {
    T_SYNC_IN();
    interpreter_execute_invalid();
    T_SYNC_OUT();
    goto interp_next;
  }
  {
    uint8_t width = (uint8_t)(opcode - 0x5f);
    u256 value = read_push(code, immediate_offset, width);
    lpc = immediate_offset + width;
    T_CHARGE(width == 0 ? GAS_BASE : GAS_VERYLOW);
    u256 *rows = NULL;
    T_STACKOP(stack_rewrite(0, 1, &rows));
    rows[0] = value;
  }
  T_NEXT;

  op_ext:
  if (active_fork() < EVMSAIL_FORK_AMSTERDAM) {
    goto op_invalid;
  }
  {
    uint8_t immediate = immediate_offset < clen ? cbytes[immediate_offset] : 0;
    bool valid = (opcode == 0xe8 ? (immediate <= 81 || immediate >= 128)
                                 : (immediate <= 90 || immediate >= 128)) != 0;
    if (valid) {
      lpc = immediate_offset + 1;
    }
    T_SYNC_IN();
    if (opcode == 0xe6) {
      interpreter_execute_dupn(immediate, valid);
    } else if (opcode == 0xe7) {
      interpreter_execute_swapn(immediate, valid);
    } else {
      interpreter_execute_exchange(immediate, valid);
    }
    T_SYNC_OUT();
  }
  goto interp_next;

  op_dup:
  T_CHARGE(GAS_VERYLOW);
  T_STACKOP(stack_dup((uint32_t)(opcode - 0x7f)));
  T_NEXT;

  op_swap:
  T_CHARGE(GAS_VERYLOW);
  T_STACKOP(stack_swap((uint32_t)(opcode - 0x8f)));
  T_NEXT;

  op_log:
  T_SYNC_IN();
  interpreter_execute_log((uint8_t)(opcode - 0xa0));
  T_SYNC_OUT();
  goto interp_next;

  op_00:

    T_EXECUTE(stop);
    goto interp_next;
  op_01:

    T_BINARY(GAS_VERYLOW, alu_add);
    goto interp_next;
  op_02:

    T_BINARY(GAS_LOW, alu_mul);
    goto interp_next;
  op_03:

    T_BINARY(GAS_VERYLOW, alu_sub);
    goto interp_next;
  op_04:

    T_BINARY(GAS_LOW, alu_div);
    goto interp_next;
  op_05:

    T_BINARY(GAS_LOW, alu_sdiv);
    goto interp_next;
  op_06:

    T_BINARY(GAS_LOW, alu_mod);
    goto interp_next;
  op_07:

    T_BINARY(GAS_LOW, alu_smod);
    goto interp_next;
  op_08:

    T_TERNARY(GAS_MID, alu_addmod);
    goto interp_next;
  op_09:

    T_TERNARY(GAS_MID, alu_mulmod);
    goto interp_next;
  op_0a:
  {
    u256 *rows = NULL;
    T_STACKOP(stack_rewrite(2, 1, &rows));
    T_CHARGE(exp_gas(rows[0]));
    rows[0] = alu_exp(rows[1], rows[0]);
  }
  T_NEXT;
  op_0b:

    T_BINARY(GAS_LOW, alu_signextend);
    goto interp_next;
  op_10:

    T_BINARY(GAS_VERYLOW, alu_lt);
    goto interp_next;
  op_11:

    T_BINARY(GAS_VERYLOW, alu_gt);
    goto interp_next;
  op_12:

    T_BINARY(GAS_VERYLOW, alu_slt);
    goto interp_next;
  op_13:

    T_BINARY(GAS_VERYLOW, alu_sgt);
    goto interp_next;
  op_14:

    T_BINARY(GAS_VERYLOW, alu_eq);
    goto interp_next;
  op_15:

    T_UNARY(GAS_VERYLOW, alu_iszero);
    goto interp_next;
  op_16:

    T_BINARY(GAS_VERYLOW, alu_and);
    goto interp_next;
  op_17:

    T_BINARY(GAS_VERYLOW, alu_or);
    goto interp_next;
  op_18:

    T_BINARY(GAS_VERYLOW, alu_xor);
    goto interp_next;
  op_19:

    T_UNARY(GAS_VERYLOW, alu_not);
    goto interp_next;
  op_1a:

    T_BINARY(GAS_VERYLOW, alu_byte);
    goto interp_next;
  op_1b:

    T_BINARY(GAS_VERYLOW, alu_shl);
    goto interp_next;
  op_1c:

    T_BINARY(GAS_VERYLOW, alu_shr);
    goto interp_next;
  op_1d:

    T_BINARY(GAS_VERYLOW, alu_sar);
    goto interp_next;
  op_1e:

    if (active_fork() >= EVMSAIL_FORK_OSAKA) {
      T_UNARY(GAS_LOW, alu_clz);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_20:

    T_STACK(keccak256, 2, 1);
    goto interp_next;
  op_30:

    T_STACK(address, 0, 1);
    goto interp_next;
  op_31:

    T_STACK(balance, 1, 1);
    goto interp_next;
  op_32:

    T_STACK(origin, 0, 1);
    goto interp_next;
  op_33:

    T_STACK(caller, 0, 1);
    goto interp_next;
  op_34:

    T_STACK(callvalue, 0, 1);
    goto interp_next;
  op_35:

    T_STACK(calldataload, 1, 1);
    goto interp_next;
  op_36:

    T_STACK(calldatasize, 0, 1);
    goto interp_next;
  op_37:

    T_STACK(calldatacopy, 3, 0);
    goto interp_next;
  op_38:

    T_STACK(codesize, 0, 1);
    goto interp_next;
  op_39:

    T_STACK(codecopy, 3, 0);
    goto interp_next;
  op_3a:

    T_STACK(gasprice, 0, 1);
    goto interp_next;
  op_3b:

    T_STACK(extcodesize, 1, 1);
    goto interp_next;
  op_3c:

    T_STACK(extcodecopy, 4, 0);
    goto interp_next;
  op_3d:

    T_STACK(returndatasize, 0, 1);
    goto interp_next;
  op_3e:

    T_STACK(returndatacopy, 3, 0);
    goto interp_next;
  op_3f:

    T_STACK(extcodehash, 1, 1);
    goto interp_next;
  op_40:

    T_STACK(blockhash, 1, 1);
    goto interp_next;
  op_41:

    T_STACK(coinbase, 0, 1);
    goto interp_next;
  op_42:

    T_STACK(timestamp, 0, 1);
    goto interp_next;
  op_43:

    T_STACK(number, 0, 1);
    goto interp_next;
  op_44:

    T_STACK(prevrandao, 0, 1);
    goto interp_next;
  op_45:

    T_STACK(gaslimit, 0, 1);
    goto interp_next;
  op_46:

    T_STACK(chainid, 0, 1);
    goto interp_next;
  op_47:

    T_STACK(selfbalance, 0, 1);
    goto interp_next;
  op_48:

    if (active_fork() >= EVMSAIL_FORK_LONDON) {
      T_STACK(basefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_49:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      T_STACK(blobhash, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_4a:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      T_STACK(blobbasefee, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_4b:

    if (active_fork() >= EVMSAIL_FORK_AMSTERDAM) {
      T_STACK(slotnum, 0, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_50:
  {
    T_CHARGE(GAS_BASE);
    u256 *rows = NULL;
    T_STACKOP(stack_rewrite(1, 0, &rows));
  }
  T_NEXT;
  op_51:

    T_STACK(mload, 1, 1);
    goto interp_next;
  op_52:

    T_STACK(mstore, 2, 0);
    goto interp_next;
  op_53:

    T_STACK(mstore8, 2, 0);
    goto interp_next;
  op_54:

    T_STACK(sload, 1, 1);
    goto interp_next;
  op_55:

    T_STACK(sstore, 2, 0);
    goto interp_next;
  op_56:

    T_STACK(jump, 1, 0);
    goto interp_next;
  op_57:

    T_STACK(jumpi, 2, 0);
    goto interp_next;
  op_58:

    /* The op name collides with the pc register-file compatibility macro;
     * the direct EXECUTE_STACK call keeps the token paste unexpanded. */
    T_SYNC_IN();
    EXECUTE_STACK(pc, 0, 1);
    T_SYNC_OUT();
    goto interp_next;
  op_59:

    T_STACK(msize, 0, 1);
    goto interp_next;
  op_5a:

    T_STACK(gas, 0, 1);
    goto interp_next;
  op_5b:

    T_CHARGE(GAS_JUMPDEST);
    T_NEXT;
  op_5c:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      T_STACK(tload, 1, 1);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_5d:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      T_STACK(tstore, 2, 0);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_5e:

    if (active_fork() >= EVMSAIL_FORK_CANCUN) {
      T_STACK(mcopy, 3, 0);
    } else {
      interpreter_execute_invalid();
    }
    goto interp_next;
  op_f0:

    T_STACK_FRAME_ENTRY(create, 3, 1);
    goto interp_next;
  op_f1:

    T_STACK_FRAME_ENTRY(call, 7, 1);
    goto interp_next;
  op_f2:

    T_STACK_FRAME_ENTRY(callcode, 7, 1);
    goto interp_next;
  op_f3:

    T_STACK(return, 2, 0);
    goto interp_next;
  op_f4:

    T_STACK_FRAME_ENTRY(delegatecall, 6, 1);
    goto interp_next;
  op_f5:

    T_STACK_FRAME_ENTRY(create2, 4, 1);
    goto interp_next;
  op_fa:

    T_STACK_FRAME_ENTRY(staticcall, 6, 1);
    goto interp_next;
  op_fd:

    T_STACK(revert, 2, 0);
    goto interp_next;
  op_ff:

    T_STACK(selfdestruct, 1, 0);
    goto interp_next;
  op_invalid:

    T_SYNC_IN();
    interpreter_execute_invalid();
    T_SYNC_OUT();
    goto interp_next;
}
