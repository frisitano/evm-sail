/*
 * Optimized fetch/decode/run loop in the state-passing convention.
 *
 * The readable Sail decoder and opcode bodies remain the standard
 * implementation. This optimized override carries the hot machine state --
 * program counter, gas, operand-stack cursor, and memory cursor -- in loop
 * locals, executes stack-local opcodes directly against the carried cursor,
 * and calls the generated Sail state-passing handlers for everything else,
 * threading the same carried values through their arguments and results.
 * The frame registers are touched only at frame boundaries: when a frame
 * stops (publish before unwind/resume) and after a resume or child entry
 * (reload). No handler reads or writes the registers mid-frame.
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
#include <stddef.h>
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

enum {
  GAS_JUMPDEST = 1,
  GAS_BASE = 2,
  GAS_VERYLOW = 3,
  GAS_LOW = 5,
  GAS_MID = 8,
};

static uint32_t interpreter_deep_stack_index(uint8_t immediate)
{
  return immediate <= 90 ? (uint32_t)immediate + 145 : (uint32_t)immediate - 111;
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

/* Threaded dispatch: one 256-entry label table replaces the range-check
 * cascade, the switch bounds test, and the loop-back jump. Semantics are
 * byte-identical to the canonical fetch/execute pipeline. */
Bytes interpret(void)
{
  frame_stack_reset();
  current_account_context_enter(message.address);

/* Loop-local machine state: the carried pc, gas, stack cursor, and memory
 * cursor. In-TU fast-path arms operate on these locals directly and never
 * call exc_halt from inside an operation: every failure branches to the
 * single interp_exc tail, which performs the carried-state halt once.
 * Generated Sail handlers receive the locals as arguments and return the
 * updated state; they never touch the frame registers. */
#define T_NEXT                                                                                     \
  do {                                                                                             \
    if (pc_local >= clen)                                                                          \
      goto op_end_of_code;                                                                         \
    opcode = cbytes[pc_local];                                                                     \
    immediate_offset = pc_local + 1U;                                                              \
    pc_local = immediate_offset;                                                                   \
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

/* The Yellow Paper stack precondition, against the carried cursor. */
#define T_CHECK(inputs, outputs)                                                                   \
  do {                                                                                             \
    uint32_t stack_height_now = (uint32_t)(sp - sbase);                                            \
    if (stack_height_now < (uint32_t)(inputs)) {                                                   \
      halt_reason = StackUnderflow;                                                                \
      goto interp_exc;                                                                             \
    }                                                                                              \
    if ((uint32_t)(outputs) > 1024U - (stack_height_now - (uint32_t)(inputs))) {                   \
      halt_reason = StackOverflow;                                                                 \
      goto interp_exc;                                                                             \
    }                                                                                              \
  } while (0)

/* Generated state-passing handler calls, by carried-state family. */
#define T_CALL_A(name, inputs, outputs)                                                            \
  do {                                                                                             \
    T_CHECK(inputs, outputs);                                                                      \
    struct tuple_bits_64_uint_64 r = execute_##name((uint64_t)(uintptr_t)sp, lgas);                \
    sp = (u256 *)(uintptr_t)r.tup0;                                                                \
    lgas = r.tup1;                                                                                 \
  } while (0);                                                                                     \
  goto interp_next

#define T_CALL_B(name, inputs, outputs)                                                            \
  do {                                                                                             \
    T_CHECK(inputs, outputs);                                                                      \
    struct tuple_bits_64_Bytes_uint_64 r = execute_##name((uint64_t)(uintptr_t)sp, lmem, lgas);    \
    sp = (u256 *)(uintptr_t)r.tup0;                                                                \
    lmem = r.tup1;                                                                                 \
    lgas = r.tup2;                                                                                 \
  } while (0);                                                                                     \
  goto interp_next

#define T_CALL_FLOW(name, inputs, outputs)                                                         \
  do {                                                                                             \
    T_CHECK(inputs, outputs);                                                                      \
    struct tuple_uint_32_bits_64_uint_64 r =                                                       \
        execute_##name(pc_local, (uint64_t)(uintptr_t)sp, lgas);                                   \
    pc_local = r.tup0;                                                                             \
    sp = (u256 *)(uintptr_t)r.tup1;                                                                \
    lgas = r.tup2;                                                                                 \
  } while (0);                                                                                     \
  goto interp_next

/* CALL- and CREATE-family handlers install a child frame only on their
 * successful frame-entry path; the code slice, frame base, and account
 * context are reloaded exactly at that semantic boundary. */
#define T_CALL_SYSTEM(name, inputs, outputs)                                                       \
  do {                                                                                             \
    T_CHECK(inputs, outputs);                                                                      \
    uint16_t parent_depth = call_depth;                                                            \
    struct tuple_uint_32_bits_64_Bytes_uint_64 r =                                                 \
        execute_##name(pc_local, (uint64_t)(uintptr_t)sp, lmem, lgas);                             \
    pc_local = r.tup0;                                                                             \
    sp = (u256 *)(uintptr_t)r.tup1;                                                                \
    lmem = r.tup2;                                                                                 \
    lgas = r.tup3;                                                                                 \
    if (call_depth != parent_depth) {                                                              \
      current_account_context_enter(message.address);                                              \
      cbytes = frame_code.bytes;                                                                   \
      clen = frame_code.len;                                                                       \
      sbase = stack_frame_base();                                                                  \
    }                                                                                              \
  } while (0);                                                                                     \
  goto interp_next

#define T_UNARY(gas, operation)                                                                    \
  do {                                                                                             \
    T_CHECK(1, 1);                                                                                 \
    T_CHARGE(gas);                                                                                 \
    sp[-1] = operation(sp[-1]);                                                                    \
  } while (0);                                                                                     \
  T_NEXT

#define T_BINARY(gas, operation)                                                                   \
  do {                                                                                             \
    T_CHECK(2, 1);                                                                                 \
    T_CHARGE(gas);                                                                                 \
    sp[-2] = operation(sp[-1], sp[-2]);                                                            \
    sp -= 1;                                                                                       \
  } while (0);                                                                                     \
  T_NEXT

#define T_TERNARY(gas, operation)                                                                  \
  do {                                                                                             \
    T_CHECK(3, 1);                                                                                 \
    T_CHARGE(gas);                                                                                 \
    sp[-3] = operation(sp[-1], sp[-2], sp[-3]);                                                    \
    sp -= 2;                                                                                       \
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
      [0x5f ... 0x7f] = &&op_push,
      [0x80 ... 0x8f] = &&op_dup,
      [0x90 ... 0x9f] = &&op_swap,
      [0xa0 ... 0xa4] = &&op_log,
      [0xa5 ... 0xe5] = &&op_invalid,
      [0xe6 ... 0xe8] = &&op_ext,
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
  uint32_t pc_local = pc;
  uint64_t lgas = gas_remaining;
  u256 *sp = (u256 *)(uintptr_t)stack_top;
  u256 *sbase = stack_frame_base();
  Bytes lmem = evm_memory;
  const uint8_t *cbytes = frame_code.bytes;
  uint32_t clen = frame_code.len;
  enum ExceptionKind halt_reason;

interp_next:
  if (frame_status.kind != Kind_Running)
    goto interp_halted;
  T_NEXT;

interp_halted: {
  /* The frame stopped: publish the carried state at this one frame
     * boundary, then unwind or resume through the registers. */
  pc = pc_local;
  gas_remaining = lgas;
  stack_top = (uint64_t)(uintptr_t)sp;
  evm_memory = lmem;
  Bytes output = frame_output();
  struct FrameContinuation continuation = frame_stack_pop();
  if (continuation.kind == Kind_Empty) {
    return output;
  }
  resume_frame(continuation, output);
  pc_local = pc;
  lgas = gas_remaining;
  sp = (u256 *)(uintptr_t)stack_top;
  sbase = stack_frame_base();
  lmem = evm_memory;
  cbytes = frame_code.bytes;
  clen = frame_code.len;
  goto interp_next;
}

op_end_of_code:
  /* Past the end of code the frame implicitly executes STOP (YP). */
  execute_stop();
  goto interp_next;

interp_exc:
  /* The single in-TU halt site: the carried-gas exceptional halt, exactly
   * the canonical exc_halt read-then-zero semantics. */
  lgas = exc_halt(lgas, halt_reason);
  goto interp_halted;

op_push:
  if (opcode == 0x5f && active_fork() < EVMSAIL_FORK_SHANGHAI) {
    goto op_invalid;
  }
  {
    uint8_t width = (uint8_t)(opcode - 0x5f);
    u256 value = read_push(code, immediate_offset, width);
    pc_local = immediate_offset + width;
    T_CHECK(0, 1);
    T_CHARGE(width == 0 ? GAS_BASE : GAS_VERYLOW);
    sp[0] = value;
    sp += 1;
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
      pc_local = immediate_offset + 1;
    } else {
      halt_reason = InvalidOpcode;
      goto interp_exc;
    }
    if (opcode == 0xe6) {
      uint32_t n = interpreter_deep_stack_index(immediate);
      T_CHECK(n, n + 1);
      T_CHARGE(GAS_VERYLOW);
      sp[0] = sp[-(ptrdiff_t)n];
      sp += 1;
    } else if (opcode == 0xe7) {
      uint32_t n = interpreter_deep_stack_index(immediate);
      T_CHECK(n + 1, n + 1);
      T_CHARGE(GAS_VERYLOW);
      u256 swapped = sp[-1];
      sp[-1] = sp[-1 - (ptrdiff_t)n];
      sp[-1 - (ptrdiff_t)n] = swapped;
    } else {
      const uint8_t shifted = immediate ^ 0x8f;
      const uint32_t quotient = shifted >> 4;
      const uint32_t remainder = shifted & 0x0f;
      const uint32_t left = quotient < remainder ? quotient + 1 : remainder + 1;
      const uint32_t right = quotient < remainder ? remainder + 1 : 29 - quotient;
      const uint32_t higher = left > right ? left : right;
      T_CHECK(higher + 1, higher + 1);
      T_CHARGE(GAS_VERYLOW);
      u256 exchanged = sp[-1 - (ptrdiff_t)left];
      sp[-1 - (ptrdiff_t)left] = sp[-1 - (ptrdiff_t)right];
      sp[-1 - (ptrdiff_t)right] = exchanged;
    }
  }
  T_NEXT;

op_dup: {
  uint32_t n = (uint32_t)(opcode - 0x7f);
  T_CHECK(n, n + 1);
  T_CHARGE(GAS_VERYLOW);
  sp[0] = sp[-(ptrdiff_t)n];
  sp += 1;
}
  T_NEXT;

op_swap: {
  uint32_t n = (uint32_t)(opcode - 0x8f);
  T_CHECK(n + 1, n + 1);
  T_CHARGE(GAS_VERYLOW);
  u256 swapped = sp[-1];
  sp[-1] = sp[-1 - (ptrdiff_t)n];
  sp[-1 - (ptrdiff_t)n] = swapped;
}
  T_NEXT;

op_log: {
  uint8_t topics = (uint8_t)(opcode - 0xa0);
  T_CHECK((uint32_t)topics + 2U, 0);
  struct tuple_bits_64_Bytes_uint_64 r = execute_log(topics, (uint64_t)(uintptr_t)sp, lmem, lgas);
  sp = (u256 *)(uintptr_t)r.tup0;
  lmem = r.tup1;
  lgas = r.tup2;
}
  goto interp_next;

op_00:
  execute_stop();
  goto interp_next;
op_01:
  T_BINARY(GAS_VERYLOW, alu_add);
op_02:
  T_BINARY(GAS_LOW, alu_mul);
op_03:
  T_BINARY(GAS_VERYLOW, alu_sub);
op_04:
  T_BINARY(GAS_LOW, alu_div);
op_05:
  T_BINARY(GAS_LOW, alu_sdiv);
op_06:
  T_BINARY(GAS_LOW, alu_mod);
op_07:
  T_BINARY(GAS_LOW, alu_smod);
op_08:
  T_TERNARY(GAS_MID, alu_addmod);
op_09:
  T_TERNARY(GAS_MID, alu_mulmod);
op_0a: {
  T_CHECK(2, 1);
  T_CHARGE(exp_gas(sp[-2]));
  sp[-2] = alu_exp(sp[-1], sp[-2]);
  sp -= 1;
}
  T_NEXT;
op_0b:
  T_BINARY(GAS_LOW, alu_signextend);
op_10:
  T_BINARY(GAS_VERYLOW, alu_lt);
op_11:
  T_BINARY(GAS_VERYLOW, alu_gt);
op_12:
  T_BINARY(GAS_VERYLOW, alu_slt);
op_13:
  T_BINARY(GAS_VERYLOW, alu_sgt);
op_14:
  T_BINARY(GAS_VERYLOW, alu_eq);
op_15:
  T_UNARY(GAS_VERYLOW, alu_iszero);
op_16:
  T_BINARY(GAS_VERYLOW, alu_and);
op_17:
  T_BINARY(GAS_VERYLOW, alu_or);
op_18:
  T_BINARY(GAS_VERYLOW, alu_xor);
op_19:
  T_UNARY(GAS_VERYLOW, alu_not);
op_1a:
  T_BINARY(GAS_VERYLOW, alu_byte);
op_1b:
  T_BINARY(GAS_VERYLOW, alu_shl);
op_1c:
  T_BINARY(GAS_VERYLOW, alu_shr);
op_1d:
  T_BINARY(GAS_VERYLOW, alu_sar);
op_1e:
  if (active_fork() >= EVMSAIL_FORK_OSAKA) {
    T_UNARY(GAS_LOW, alu_clz);
  } else {
    goto op_invalid;
  }
op_20:
  T_CALL_B(keccak256, 2, 1);
op_30:
  T_CALL_A(address, 0, 1);
op_31:
  T_CALL_A(balance, 1, 1);
op_32:
  T_CALL_A(origin, 0, 1);
op_33:
  T_CALL_A(caller, 0, 1);
op_34:
  T_CALL_A(callvalue, 0, 1);
op_35:
  T_CALL_A(calldataload, 1, 1);
op_36:
  T_CALL_A(calldatasize, 0, 1);
op_37:
  T_CALL_B(calldatacopy, 3, 0);
op_38:
  T_CALL_A(codesize, 0, 1);
op_39:
  T_CALL_B(codecopy, 3, 0);
op_3a:
  T_CALL_A(gasprice, 0, 1);
op_3b:
  T_CALL_A(extcodesize, 1, 1);
op_3c:
  T_CALL_B(extcodecopy, 4, 0);
op_3d:
  T_CALL_A(returndatasize, 0, 1);
op_3e:
  T_CALL_B(returndatacopy, 3, 0);
op_3f:
  T_CALL_A(extcodehash, 1, 1);
op_40:
  T_CALL_A(blockhash, 1, 1);
op_41:
  T_CALL_A(coinbase, 0, 1);
op_42:
  T_CALL_A(timestamp, 0, 1);
op_43:
  T_CALL_A(number, 0, 1);
op_44:
  T_CALL_A(prevrandao, 0, 1);
op_45:
  T_CALL_A(gaslimit, 0, 1);
op_46:
  T_CALL_A(chainid, 0, 1);
op_47:
  T_CALL_A(selfbalance, 0, 1);
op_48:
  if (active_fork() >= EVMSAIL_FORK_LONDON) {
    T_CALL_A(basefee, 0, 1);
  } else {
    goto op_invalid;
  }
op_49:
  if (active_fork() >= EVMSAIL_FORK_CANCUN) {
    T_CALL_A(blobhash, 1, 1);
  } else {
    goto op_invalid;
  }
op_4a:
  if (active_fork() >= EVMSAIL_FORK_CANCUN) {
    T_CALL_A(blobbasefee, 0, 1);
  } else {
    goto op_invalid;
  }
op_4b:
  if (active_fork() >= EVMSAIL_FORK_AMSTERDAM) {
    T_CALL_A(slotnum, 0, 1);
  } else {
    goto op_invalid;
  }
op_50: {
  T_CHECK(1, 0);
  T_CHARGE(GAS_BASE);
  sp -= 1;
}
  T_NEXT;
op_51:
  T_CALL_B(mload, 1, 1);
op_52:
  T_CALL_B(mstore, 2, 0);
op_53:
  T_CALL_B(mstore8, 2, 0);
op_54:
  T_CALL_A(sload_, 1, 1);
op_55:
  T_CALL_A(sstore_, 2, 0);
op_56:
  T_CALL_FLOW(jump, 1, 0);
op_57:
  T_CALL_FLOW(jumpi, 2, 0);
op_58: {
  /* PC pushes the carried, already-advanced counter minus one. The arm
     * avoids the register-file pc compatibility macro entirely. */
  T_CHECK(0, 1);
  T_CHARGE(GAS_BASE);
  sp[0] = (u256){{(uint64_t)(pc_local - 1U), 0, 0, 0}};
  sp += 1;
}
  T_NEXT;
op_59:
  T_CALL_B(msize, 0, 1);
op_5a: {
  /* GAS pushes the carried gas remaining after its own charge. */
  T_CHECK(0, 1);
  T_CHARGE(GAS_BASE);
  sp[0] = (u256){{lgas, 0, 0, 0}};
  sp += 1;
}
  T_NEXT;
op_5b:
  T_CHARGE(GAS_JUMPDEST);
  T_NEXT;
op_5c:
  if (active_fork() >= EVMSAIL_FORK_CANCUN) {
    T_CALL_A(tload, 1, 1);
  } else {
    goto op_invalid;
  }
op_5d:
  if (active_fork() >= EVMSAIL_FORK_CANCUN) {
    T_CALL_A(tstore, 2, 0);
  } else {
    goto op_invalid;
  }
op_5e:
  if (active_fork() >= EVMSAIL_FORK_CANCUN) {
    T_CALL_B(mcopy, 3, 0);
  } else {
    goto op_invalid;
  }
op_f0:
  T_CALL_SYSTEM(create, 3, 1);
op_f1:
  T_CALL_SYSTEM(call, 7, 1);
op_f2:
  T_CALL_SYSTEM(callcode, 7, 1);
op_f3:
  T_CALL_B(return, 2, 0);
op_f4:
  T_CALL_SYSTEM(delegatecall, 6, 1);
op_f5:
  T_CALL_SYSTEM(create2, 4, 1);
op_fa:
  T_CALL_SYSTEM(staticcall, 6, 1);
op_fd:
  T_CALL_B(revert, 2, 0);
op_ff:
  T_CALL_A(selfdestruct, 1, 0);
op_invalid:
  /* INVALID consumes all carried gas before the exceptional halt. */
  lgas = 0;
  halt_reason = InvalidOpcode;
  goto interp_exc;
}
