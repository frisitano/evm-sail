/*
 * Optimized fetch/decode/run loop.
 *
 * The readable Sail decoder and opcode bodies remain the standard
 * implementation. This optimized override executes stack-local opcodes
 * against the C stack window and calls their generated Sail ALU functions
 * directly. Stateful, control-flow, call, and create opcodes continue to use
 * the generated Sail handlers below.
 */
#include EVMSAIL_MODEL_H

#include "interpreter_glue.h"

#include "region_access.h"
#include "frame_stack.h"
#include "optimized_stack.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct code_view {
  uint64_t off;
  uint64_t len;
  const uint8_t *bytes;
  bool valid;
};

static struct zOutputSliceFields empty_slice(void) {
  return (struct zOutputSliceFields){
      .zlen = 0,
      .zoff = 0,
  };
}

static bool same_code(const struct code_view *view,
                      const struct zCodeRegionSliceFields *code) {
  return view->valid && view->off == code->zoff && view->len == code->zlen;
}

static void resolve_code(struct code_view *view) {
  const struct zCodeRegionSliceFields *code = &zframe_code.zbytes;
  if (same_code(view, code)) return;

  const uint8_t *bytes = evmsail_code_ptr(code->zoff, code->zlen);
  if (!bytes) abort();

  view->off = code->zoff;
  view->len = code->zlen;
  view->bytes = bytes;
  view->valid = true;
}

static sail_u256 read_push(const struct code_view *view, uint64_t offset,
                           uint64_t width) {
  sail_u256 value = {{0, 0, 0, 0}};
  uint64_t available = offset < view->len ? view->len - offset : 0;
  if (available > width) available = width;

  /* EVM immediates are big-endian; sail_u256 stores least-significant limbs
   * first. Bytes beyond the end of code remain zero as required by PUSH. */
  for (uint64_t i = 0; i < available; i++) {
    uint64_t byte_from_low = width - 1 - i;
    value.limbs[byte_from_low / 8] |=
        (uint64_t)view->bytes[offset + i] << (8 * (byte_from_low % 8));
  }
  return value;
}

#define EXECUTE(name) ((void)zexecute_##name(UNIT))
#define EXECUTE_CALLDATASIZE() ((void)zexecute_calldatasizze(UNIT))
#define EXECUTE_CODESIZE() ((void)zexecute_codesizze(UNIT))
#define EXECUTE_EXTCODESIZE() ((void)zexecute_extcodesizze(UNIT))
#define EXECUTE_RETURNDATASIZE() ((void)zexecute_returndatasizze(UNIT))
#define EXECUTE_MSIZE() ((void)zexecute_msizze(UNIT))

typedef sail_u256 (*unary_alu)(sail_u256);
typedef sail_u256 (*binary_alu)(sail_u256, sail_u256);
typedef sail_u256 (*ternary_alu)(sail_u256, sail_u256, sail_u256);

enum {
  GAS_BASE = 2,
  GAS_VERYLOW = 3,
  GAS_LOW = 5,
  GAS_MID = 8,
};

static void charge_opcode(uint64_t amount) {
  if (zframe_status.kind != Kind_zRunning || amount == 0) return;
  if (amount <= zgas_remaining) {
    zgas_remaining -= amount;
  } else {
    (void)zexc_halt(zOutOfGas);
  }
}

static bool stack_rewrite_succeeded(
    enum evmsail_stack_rewrite_status status) {
  if (status == EVMSAIL_STACK_REWRITE_OK) return true;
  (void)zexc_halt(status == EVMSAIL_STACK_REWRITE_UNDERFLOW
                      ? zStackUnderflow
                      : zStackOverflow);
  return false;
}

static void execute_unary(uint64_t gas, unary_alu operation) {
  charge_opcode(gas);
  sail_u256 *rows = NULL;
  if (!stack_rewrite_succeeded(evmsail_stack_rewrite(1, 1, &rows))) return;
  rows[0] = operation(rows[0]);
}

static void execute_binary(uint64_t gas, binary_alu operation) {
  charge_opcode(gas);
  sail_u256 *rows = NULL;
  if (!stack_rewrite_succeeded(evmsail_stack_rewrite(2, 1, &rows))) return;
  rows[0] = operation(rows[1], rows[0]);
}

static void execute_ternary(uint64_t gas, ternary_alu operation) {
  charge_opcode(gas);
  sail_u256 *rows = NULL;
  if (!stack_rewrite_succeeded(evmsail_stack_rewrite(3, 1, &rows))) return;
  rows[0] = operation(rows[2], rows[1], rows[0]);
}

static void execute_exp(void) {
  sail_u256 *rows = NULL;
  if (!stack_rewrite_succeeded(evmsail_stack_rewrite(2, 1, &rows))) return;
  charge_opcode(zexp_gas(rows[0]));
  rows[0] = zalu_exp(rows[1], rows[0]);
}

static void execute_push(uint64_t width, sail_u256 value) {
  charge_opcode(width == 0 ? GAS_BASE : GAS_VERYLOW);
  sail_u256 *rows = NULL;
  if (!stack_rewrite_succeeded(evmsail_stack_rewrite(0, 1, &rows))) return;
  rows[0] = value;
}

static void execute_dup(uint64_t depth) {
  charge_opcode(GAS_VERYLOW);
  (void)stack_rewrite_succeeded(evmsail_stack_dup((uint32_t)depth));
}

static void execute_swap(uint64_t depth) {
  charge_opcode(GAS_VERYLOW);
  (void)stack_rewrite_succeeded(evmsail_stack_swap((uint32_t)depth));
}

static void execute_pop(void) {
  charge_opcode(GAS_BASE);
  sail_u256 *rows = NULL;
  (void)stack_rewrite_succeeded(evmsail_stack_rewrite(1, 0, &rows));
}

static bool deep_stack_immediate_valid(uint8_t immediate) {
  return immediate <= 90 || immediate >= 128;
}

static uint32_t deep_stack_index(uint8_t immediate) {
  return immediate <= 90 ? (uint32_t)immediate + 145
                         : (uint32_t)immediate - 111;
}

static bool exchange_immediate_valid(uint8_t immediate) {
  return immediate <= 81 || immediate >= 128;
}

static void execute_dupn(uint8_t immediate) {
  charge_opcode(GAS_VERYLOW);
  if (zframe_status.kind != Kind_zRunning) return;
  if (!deep_stack_immediate_valid(immediate)) {
    (void)zexc_halt(zInvalidOpcode);
    return;
  }
  (void)stack_rewrite_succeeded(
      evmsail_stack_dup(deep_stack_index(immediate)));
}

static void execute_swapn(uint8_t immediate) {
  charge_opcode(GAS_VERYLOW);
  if (zframe_status.kind != Kind_zRunning) return;
  if (!deep_stack_immediate_valid(immediate)) {
    (void)zexc_halt(zInvalidOpcode);
    return;
  }
  (void)stack_rewrite_succeeded(
      evmsail_stack_swap(deep_stack_index(immediate)));
}

static void execute_exchange(uint8_t immediate) {
  charge_opcode(GAS_VERYLOW);
  if (zframe_status.kind != Kind_zRunning) return;
  if (!exchange_immediate_valid(immediate)) {
    (void)zexc_halt(zInvalidOpcode);
    return;
  }

  const uint8_t shifted = immediate ^ 0x8f;
  const uint32_t quotient = shifted >> 4;
  const uint32_t remainder = shifted & 0x0f;
  const uint32_t left = quotient < remainder ? quotient + 1 : remainder + 1;
  const uint32_t right = quotient < remainder ? remainder + 1 : 29 - quotient;
  (void)stack_rewrite_succeeded(evmsail_stack_exchange(left, right));
}

static void execute_log(uint64_t topics) {
  (void)zexecute_log(topics);
}

static void execute_invalid(void) { EXECUTE(invalid); }

static void execute_simple(uint8_t opcode) {
  if (opcode >= 0x80 && opcode <= 0x8f) {
    execute_dup((uint64_t)opcode - 0x7f);
    return;
  }
  if (opcode >= 0x90 && opcode <= 0x9f) {
    execute_swap((uint64_t)opcode - 0x8f);
    return;
  }
  if (opcode >= 0xa0 && opcode <= 0xa4) {
    execute_log((uint64_t)opcode - 0xa0);
    return;
  }

  switch (opcode) {
    case 0x00:
      EXECUTE(stop);
      return;
    case 0x01:
      execute_binary(GAS_VERYLOW, zalu_add);
      return;
    case 0x02:
      execute_binary(GAS_LOW, zalu_mul);
      return;
    case 0x03:
      execute_binary(GAS_VERYLOW, zalu_sub);
      return;
    case 0x04:
      execute_binary(GAS_LOW, zalu_div);
      return;
    case 0x05:
      execute_binary(GAS_LOW, zalu_sdiv);
      return;
    case 0x06:
      execute_binary(GAS_LOW, zalu_mod);
      return;
    case 0x07:
      execute_binary(GAS_LOW, zalu_smod);
      return;
    case 0x08:
      execute_ternary(GAS_MID, zalu_addmod);
      return;
    case 0x09:
      execute_ternary(GAS_MID, zalu_mulmod);
      return;
    case 0x0a:
      execute_exp();
      return;
    case 0x0b:
      execute_binary(GAS_LOW, zalu_signextend);
      return;
    case 0x10:
      execute_binary(GAS_VERYLOW, zalu_lt);
      return;
    case 0x11:
      execute_binary(GAS_VERYLOW, zalu_gt);
      return;
    case 0x12:
      execute_binary(GAS_VERYLOW, zalu_slt);
      return;
    case 0x13:
      execute_binary(GAS_VERYLOW, zalu_sgt);
      return;
    case 0x14:
      execute_binary(GAS_VERYLOW, zalu_eq);
      return;
    case 0x15:
      execute_unary(GAS_VERYLOW, zalu_iszzero);
      return;
    case 0x16:
      execute_binary(GAS_VERYLOW, zalu_and);
      return;
    case 0x17:
      execute_binary(GAS_VERYLOW, zalu_or);
      return;
    case 0x18:
      execute_binary(GAS_VERYLOW, zalu_xor);
      return;
    case 0x19:
      execute_unary(GAS_VERYLOW, zalu_not);
      return;
    case 0x1a:
      execute_binary(GAS_VERYLOW, zalu_byte);
      return;
    case 0x1b:
      execute_binary(GAS_VERYLOW, zalu_shl);
      return;
    case 0x1c:
      execute_binary(GAS_VERYLOW, zalu_shr);
      return;
    case 0x1d:
      execute_binary(GAS_VERYLOW, zalu_sar);
      return;
    case 0x1e:
      if (zk_fork >= zOsaka)
        execute_unary(GAS_LOW, zalu_clzz);
      else
        execute_invalid();
      return;
    case 0x20:
      EXECUTE(keccak256);
      return;
    case 0x30:
      EXECUTE(address);
      return;
    case 0x31:
      EXECUTE(balance);
      return;
    case 0x32:
      EXECUTE(origin);
      return;
    case 0x33:
      EXECUTE(caller);
      return;
    case 0x34:
      EXECUTE(callvalue);
      return;
    case 0x35:
      EXECUTE(calldataload);
      return;
    case 0x36:
      EXECUTE_CALLDATASIZE();
      return;
    case 0x37:
      EXECUTE(calldatacopy);
      return;
    case 0x38:
      EXECUTE_CODESIZE();
      return;
    case 0x39:
      EXECUTE(codecopy);
      return;
    case 0x3a:
      EXECUTE(gasprice);
      return;
    case 0x3b:
      EXECUTE_EXTCODESIZE();
      return;
    case 0x3c:
      EXECUTE(extcodecopy);
      return;
    case 0x3d:
      EXECUTE_RETURNDATASIZE();
      return;
    case 0x3e:
      EXECUTE(returndatacopy);
      return;
    case 0x3f:
      EXECUTE(extcodehash);
      return;
    case 0x40:
      EXECUTE(blockhash);
      return;
    case 0x41:
      EXECUTE(coinbase);
      return;
    case 0x42:
      EXECUTE(timestamp);
      return;
    case 0x43:
      EXECUTE(number);
      return;
    case 0x44:
      EXECUTE(prevrandao);
      return;
    case 0x45:
      EXECUTE(gaslimit);
      return;
    case 0x46:
      EXECUTE(chainid);
      return;
    case 0x47:
      EXECUTE(selfbalance);
      return;
    case 0x48:
      if (zk_fork >= zLondon)
        EXECUTE(basefee);
      else
        execute_invalid();
      return;
    case 0x49:
      if (zk_fork >= zCancun)
        EXECUTE(blobhash);
      else
        execute_invalid();
      return;
    case 0x4a:
      if (zk_fork >= zCancun)
        EXECUTE(blobbasefee);
      else
        execute_invalid();
      return;
    case 0x4b:
      if (zk_fork >= zAmsterdam)
        EXECUTE(slotnum);
      else
        execute_invalid();
      return;
    case 0x50:
      execute_pop();
      return;
    case 0x51:
      EXECUTE(mload);
      return;
    case 0x52:
      EXECUTE(mstore);
      return;
    case 0x53:
      EXECUTE(mstore8);
      return;
    case 0x54:
      EXECUTE(sload);
      return;
    case 0x55:
      EXECUTE(sstore);
      return;
    case 0x56:
      EXECUTE(jump);
      return;
    case 0x57:
      EXECUTE(jumpi);
      return;
    case 0x58:
      EXECUTE(pc);
      return;
    case 0x59:
      EXECUTE_MSIZE();
      return;
    case 0x5a:
      EXECUTE(gas);
      return;
    case 0x5b:
      EXECUTE(jumpdest);
      return;
    case 0x5c:
      if (zk_fork >= zCancun)
        EXECUTE(tload);
      else
        execute_invalid();
      return;
    case 0x5d:
      if (zk_fork >= zCancun)
        EXECUTE(tstore);
      else
        execute_invalid();
      return;
    case 0x5e:
      if (zk_fork >= zCancun)
        EXECUTE(mcopy);
      else
        execute_invalid();
      return;
    case 0xf0:
      EXECUTE(create);
      return;
    case 0xf1:
      EXECUTE(call);
      return;
    case 0xf2:
      EXECUTE(callcode);
      return;
    case 0xf3:
      EXECUTE(return);
      return;
    case 0xf4:
      EXECUTE(delegatecall);
      return;
    case 0xf5:
      EXECUTE(create2);
      return;
    case 0xfa:
      EXECUTE(staticcall);
      return;
    case 0xfd:
      EXECUTE(revert);
      return;
    case 0xff:
      EXECUTE(selfdestruct);
      return;
    default:
      execute_invalid();
      return;
  }
}

static struct zOutputSliceFields frame_output(void) {
  if (zframe_status.kind != Kind_zHalted) return empty_slice();

  switch (zframe_status.variants.zHalted.kind) {
    case Kind_zHaltReturn:
      return zframe_status.variants.zHalted.variants.zHaltReturn;
    case Kind_zHaltRevert:
      return zframe_status.variants.zHalted.variants.zHaltRevert;
    default:
      return empty_slice();
  }
}

struct zOutputSliceFields evmsail_interpret(unit u) {
  struct code_view code = {0};
  (void)u;
  frame_stack_reset(UNIT);

  for (;;) {
    if (have_exception) return empty_slice();

    if (zframe_status.kind == Kind_zRunning) {
      resolve_code(&code);
      uint64_t current = zpc;

      if (current >= code.len) {
        EXECUTE(stop);
        continue;
      }

      uint8_t opcode = code.bytes[current];
      uint64_t immediate_offset = current + 1;
      zpc = immediate_offset;

      if (opcode >= 0x5f && opcode <= 0x7f) {
        if (opcode == 0x5f && zk_fork < zShanghai) {
          execute_invalid();
        } else {
          uint64_t width = (uint64_t)opcode - 0x5f;
          zpc = immediate_offset + width;
          execute_push(width, read_push(&code, immediate_offset, width));
        }
        continue;
      }

      if (zk_fork >= zAmsterdam && opcode >= 0xe6 && opcode <= 0xe8) {
        uint64_t immediate =
            immediate_offset < code.len ? code.bytes[immediate_offset] : 0;
        bool valid = opcode == 0xe8
                         ? (immediate <= 81 || immediate >= 128)
                         : (immediate <= 90 || immediate >= 128);
        if (valid) zpc = immediate_offset + 1;
        if (opcode == 0xe6)
          execute_dupn(immediate);
        else if (opcode == 0xe7)
          execute_swapn(immediate);
        else
          execute_exchange(immediate);
        continue;
      }

      execute_simple(opcode);
      continue;
    }

    struct zOutputSliceFields output = frame_output();
    struct zFrameContinuation continuation = {0};
    continuation.kind = Kind_zEmpty;
    frame_stack_pop(&continuation, UNIT);
    if (continuation.kind == Kind_zEmpty) return output;
    zresume_frame(continuation, output);
  }
}
