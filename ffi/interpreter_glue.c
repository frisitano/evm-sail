/*
 * Optimized fetch/decode/run loop.
 *
 * This translation unit deliberately owns only interpreter control flow. The
 * readable Sail decoder remains the standard implementation, and all opcode
 * semantics, gas accounting, exceptions, calls, and creates remain in the
 * generated Sail handlers called below.
 */
#include EVMSAIL_MODEL_H

#include "interpreter_glue.h"

#include "byte_slice_glue.h"
#include "frame_stack.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct code_view {
  enum zByteSource source;
  uint64_t off;
  uint64_t len;
  const uint8_t *bytes;
  bool valid;
};

static struct zByteSliceFields empty_slice(void) {
  return (struct zByteSliceFields){
      .zlen = 0,
      .zoff = 0,
      .zsource = zStatelessInputSource,
  };
}

static bool same_code(const struct code_view *view,
                      const struct zByteSliceFields *code) {
  return view->valid && view->source == code->zsource &&
         view->off == code->zoff && view->len == code->zlen;
}

static void resolve_code(struct code_view *view) {
  const struct zByteSliceFields *code = &zframe_code.zbytes;
  if (same_code(view, code)) return;

  const uint8_t *bytes = NULL;
  uint64_t resolved_len = 0;
  if (!evmsail_resolve_byte_source(evmsail_source_kind(code->zsource),
                                   code->zoff, code->zlen, &bytes,
                                   &resolved_len) ||
      resolved_len != code->zlen) {
    abort();
  }

  view->source = code->zsource;
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
#define EXECUTE_ISZERO() ((void)zexecute_iszzero(UNIT))
#define EXECUTE_CLZ() ((void)zexecute_clzz(UNIT))
#define EXECUTE_CALLDATASIZE() ((void)zexecute_calldatasizze(UNIT))
#define EXECUTE_CODESIZE() ((void)zexecute_codesizze(UNIT))
#define EXECUTE_EXTCODESIZE() ((void)zexecute_extcodesizze(UNIT))
#define EXECUTE_RETURNDATASIZE() ((void)zexecute_returndatasizze(UNIT))
#define EXECUTE_MSIZE() ((void)zexecute_msizze(UNIT))

static void execute_push(uint64_t width, sail_u256 value) {
  (void)zexecute_push(width, value);
}

static void execute_dup(uint64_t depth) {
  (void)zexecute_dup(depth);
}

static void execute_swap(uint64_t depth) {
  (void)zexecute_swap(depth);
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
      EXECUTE(add);
      return;
    case 0x02:
      EXECUTE(mul);
      return;
    case 0x03:
      EXECUTE(sub);
      return;
    case 0x04:
      EXECUTE(div);
      return;
    case 0x05:
      EXECUTE(sdiv);
      return;
    case 0x06:
      EXECUTE(mod);
      return;
    case 0x07:
      EXECUTE(smod);
      return;
    case 0x08:
      EXECUTE(addmod);
      return;
    case 0x09:
      EXECUTE(mulmod);
      return;
    case 0x0a:
      EXECUTE(exp);
      return;
    case 0x0b:
      EXECUTE(signextend);
      return;
    case 0x10:
      EXECUTE(lt);
      return;
    case 0x11:
      EXECUTE(gt);
      return;
    case 0x12:
      EXECUTE(slt);
      return;
    case 0x13:
      EXECUTE(sgt);
      return;
    case 0x14:
      EXECUTE(eq);
      return;
    case 0x15:
      EXECUTE_ISZERO();
      return;
    case 0x16:
      EXECUTE(and);
      return;
    case 0x17:
      EXECUTE(or);
      return;
    case 0x18:
      EXECUTE(xor);
      return;
    case 0x19:
      EXECUTE(not);
      return;
    case 0x1a:
      EXECUTE(byte);
      return;
    case 0x1b:
      EXECUTE(shl);
      return;
    case 0x1c:
      EXECUTE(shr);
      return;
    case 0x1d:
      EXECUTE(sar);
      return;
    case 0x1e:
      if (zk_fork >= zOsaka)
        EXECUTE_CLZ();
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
      EXECUTE(pop);
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

static struct zByteSliceFields frame_output(void) {
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

struct zByteSliceFields evmsail_interpret(unit u) {
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
          (void)zexecute_dupn(immediate);
        else if (opcode == 0xe7)
          (void)zexecute_swapn(immediate);
        else
          (void)zexecute_exchange(immediate);
        continue;
      }

      execute_simple(opcode);
      continue;
    }

    struct zByteSliceFields output = frame_output();
    struct zFrameContinuation continuation = {0};
    continuation.kind = Kind_zEmpty;
    frame_stack_pop(&continuation, UNIT);
    if (continuation.kind == Kind_zEmpty) return output;
    zresume_frame(continuation, output);
  }
}
