/* C-backed EVM memory for the evm-sail model.
 *
 * Sail forbids recursive types, so an in-model balanced tree is impossible and a
 * list-backed memory is O(n) per access (O(n^2) over a fill/copy loop). Following
 * revm's SharedMemory, memory lives in C as ONE arena with per-frame
 * checkpoints: frame d owns arena[f_base[d] ..), and a sub-call's frame begins
 * where the (suspended) caller's ESTABLISHED extent ends. The standard ABI
 * identifies slices with absolute arena offsets so growth cannot dangle. The
 * optimized ABI uses stable pointers into the fixed backing arena, so
 * suspended frames cannot be invalidated by storage movement.
 *
 * Establishment discipline (no zeroing on frame exit): f_len[d] is the extent
 * the frame has WRITTEN OR ZEROED. Reads beyond it return 0; any write or
 * region grant that extends it first zero-fills the gap, so a dead child's
 * bytes (which live above the parent's extent) can never be observed.
 * mem_frame_leave just pops the checkpoint. The canonical generated-C ABI
 * carries mathematical byte quantities; the production ABI represents the
 * protocol-bounded values directly as uint64_t. */
#include "evmsail/prelude.h"
#include "primitives/value.h"
#include "evmsail/host/region_access.h"
#include "workspace.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MEMORY_MAXDEPTH 1100 /* call_depth_limit (1024) + tx frame + slack */

static uint8_t *arena;
static size_t *f_base;
static size_t *f_len; /* established (written-or-zeroed) extent */
static int h_top = 0;

void memory_workspace_bind(void) {
  WORKSPACE_BIND(arena, GUEST_EVM_MEMORY_BYTES);
  WORKSPACE_BIND(f_base, MEMORY_MAXDEPTH);
  WORKSPACE_BIND(f_len, MEMORY_MAXDEPTH);
}

/* extend the current frame's established extent to `end` (frame-relative),
 * zero-filling the gap. Sail has already established and charged the range;
 * a failure here is a host-contract violation rather than EVM OutOfGas. */
static void f_establish(uint64_t end) {
  if (end <= f_len[h_top]) return;
  size_t base = f_base[h_top];
  if (end > GUEST_EVM_MEMORY_BYTES ||
      base > GUEST_EVM_MEMORY_BYTES - (size_t)end)
    GUEST_ABORT();
  memset(arena + base + f_len[h_top], 0, (size_t)end - f_len[h_top]);
  f_len[h_top] = (size_t)end;
}

/* Establish a range that the caller will overwrite completely. Only an
 * untouched gap before the destination needs materializing as zero; bytes in
 * the destination itself must not be cleared immediately before the write. */
static void f_establish_write(uint64_t off, uint64_t end) {
  if (end <= f_len[h_top]) return;
  const size_t base = f_base[h_top];
  const size_t established = f_len[h_top];
  if (end > GUEST_EVM_MEMORY_BYTES ||
      base > GUEST_EVM_MEMORY_BYTES - (size_t)end)
    GUEST_ABORT();
  if (off > established)
    memset(arena + base + established, 0, (size_t)off - established);
  f_len[h_top] = (size_t)end;
}

static uint8_t *frame_write_region(uint64_t off, uint64_t len);

/* Clear back to a single empty top-level frame (called per transaction); the
 * arena remains bound to its predefined workspace region. */
unit mem_clear(const unit u) {
  (void)u;
  h_top = 0;
  f_base[0] = 0;
  f_len[0] = 0;
  return UNIT;
}

/* Enter a sub-call and return the fresh frame's absolute arena base. Sail
 * stores that base in the active frame's nominal EvmMemorySlice. */
static uint64_t mem_frame_enter_value(void) {
  /* The EVM call-depth rule makes this host-capacity failure unreachable. */
  if (h_top + 1 >= MEMORY_MAXDEPTH) GUEST_ABORT();
  h_top++;
  f_base[h_top] = f_base[h_top - 1] + f_len[h_top - 1];
  f_len[h_top] = 0;
  return (uint64_t)f_base[h_top];
}

uint32_t mem_frame_enter(const unit u) {
  (void)u;
  return mem_frame_enter_value();
}

/* leave a sub-call: pop the checkpoint. The dead frame's bytes sit above the
 * parent's established extent, so establishment zero-fills over them before
 * the parent could ever read them. */
unit mem_frame_leave(const unit u) {
  (void)u;
  if (h_top > 0) h_top--;
  return UNIT;
}

/* byte-quantity offset -> bits(8) byte (0 beyond the established extent) */
uint64_t mem_read_byte(uint64_t off) {
  uint64_t offset = off;
  return (offset < f_len[h_top])
             ? (uint64_t)arena[f_base[h_top] + offset]
             : 0;
}

/* Current call-frame depth. */
uint64_t hm_depth(const unit u) { (void)u; return (uint64_t)h_top; }

/* Materialize the logical extent whose affordability Sail already proved and
 * return its absolute arena base. region_access.c packages this span in the
 * generated EvmMemorySlice representation. */
uint64_t evm_memory_expand(uint64_t len) {
  f_establish(len);
  return (uint64_t)f_base[h_top];
}

/* establish + a WRITE pointer to [off, off+len) of the current frame (the
 * gas-side watermark is raised by charge_expansion before any copy opcode
 * writes) */
static uint8_t *frame_write_region(uint64_t off, uint64_t len) {
  if (len == 0) return NULL;
  if (off > UINT64_MAX - len) GUEST_ABORT();
  f_establish_write(off, off + len);
  return arena + f_base[h_top] + off;
}

/* MLOAD: the 32-byte big-endian word at off. No establishment -- reads past
 * the extent are zeros, and charge_expansion precedes every MLOAD so the
 * gas-side watermark already covers the range. */
U256 mem_load_word(uint32_t off) {
  uint64_t offset = off;
  uint8_t buf[32];
  for (int i = 0; i < 32; i++) {
    uint64_t o = offset + (uint64_t)i;
    buf[i] = (o >= offset && o < f_len[h_top])
                 ? arena[f_base[h_top] + o]
                 : 0;
  }
  return (be_bytes_to_sail_word(buf));
}

/* MSTORE: the 32-byte big-endian word at off (establish + one memcpy) */
unit mem_store_word(uint32_t off, U256 w) {
  uint8_t buf[32];
  sail_word_to_be_bytes(buf, (w));
  uint8_t *d = frame_write_region(off, 32);
  memcpy(d, buf, 32);
  return UNIT;
}

/* MCOPY: overlapping-safe copy within the current frame. Both ranges are
 * established before either pointer is taken. */
unit mem_move(uint32_t dst, uint32_t src, uint32_t len) {
  uint64_t dst_value = dst;
  uint64_t src_value = src;
  uint64_t len_value = len;
  if (!len_value) return UNIT;
  if (src_value > UINT64_MAX - len_value ||
      dst_value > UINT64_MAX - len_value)
    GUEST_ABORT();
  f_establish(src_value + len_value);
  uint8_t *destination = frame_write_region(dst_value, len_value);
  memmove(destination,
          arena + f_base[h_top] + src_value, len_value);
  return UNIT;
}

/* (byte-quantity offset, bits(8) value) -> unit */
unit mem_write_byte(uint32_t off, uint64_t v) {
  uint64_t offset = off;
  *frame_write_region(offset, 1) = (uint8_t)(v & 0xff);
  return UNIT;
}

/* Read an absolute span minted by subslicing an active or suspended Sail
 * EVM-memory frame pointer. Well-formed slices never exceed an established
 * frame; the capacity check keeps malformed host inputs fail-closed. */
const uint8_t *evm_memory_region(uint64_t off, uint64_t len) {
  if (len == 0 || off > GUEST_EVM_MEMORY_BYTES ||
      len > GUEST_EVM_MEMORY_BYTES - off)
    return NULL;
  return arena + off;
}

const uint8_t *evm_memory_base(void) { return arena; }
uint64_t evm_memory_capacity(void) { return GUEST_EVM_MEMORY_BYTES; }

uint8_t *evm_memory_write_region(uint64_t off, uint64_t len) {
  return frame_write_region(off, len);
}
