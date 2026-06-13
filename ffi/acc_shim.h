/* Sail marshalling shim for the zkvm-standards crypto accelerators.
 * Sail cannot form a (const uint8_t*, size_t) call, so the model streams input
 * bytes into a buffer here (acc_begin/push/push8), the shim calls the one-shot
 * standard zkvm_accelerators.h function for the selected id, and the model reads
 * the output back (acc_exec/ok/out). Only mach_bits cross the FFI. This shim is
 * guest glue; the swappable boundary is ffi/zkvm_accelerators.h. ids: 0=keccak256
 * (the standard's non-precompile accelerator), 2=sha256, 3=ripemd160, 4=identity
 * (done guest-side per the standard), 1/5..10 = EVM precompiles (typed standard
 * calls wired as they are implemented). */
#ifndef ACC_SHIM_H
#define ACC_SHIM_H
#include "sail.h"
#include <stdint.h>
#include "host_mem.h"   /* C-backed EVM memory externs (shared by every build path) */
#include "host_map.h"   /* C-backed layered state maps */
#include "host_stack.h" /* C-backed operand stack */
#include "host_word.h"  /* native word predicates */
#include "host_code.h"  /* JUMPDEST bitmap */
unit     acc_begin(uint64_t id);   /* bits(64): select accelerator id, clear buffers */
unit     acc_begin_mem(uint64_t id, uint64_t off, uint64_t len);  /* input := memory range */
unit     acc_push(uint64_t b);      /* bits(8):  append one input byte */
unit     acc_push8(uint64_t w);     /* bits(64): append 8 input bytes (big-endian) */
uint64_t acc_exec(unit u);          /* run the standard accelerator; returns output length */
uint64_t acc_ok(unit u);            /* bits(8): 1 = ZKVM_EOK, 0 = ZKVM_EFAIL */
uint64_t acc_out(uint64_t i);       /* bits(64) index -> bits(8): output byte i */
uint64_t acc_word(uint64_t i);      /* bits(64) -> bits(64): big-endian output word i (hash fast path) */
unit hr_reset(const unit u);
unit hr_clear(const unit u);
unit hr_discard(const unit u);
unit hr_capture(uint64_t off, uint64_t len);
unit hr_capture_acc(const unit u);
unit hr_adopt(const unit u);
uint64_t hr_len(const unit u);
uint64_t hr_pending_len(const unit u);
uint64_t hr_pending_byte(uint64_t i);
unit hr_to_mem(uint64_t dst, uint64_t off, uint64_t len);
unit hr_out_region(uint64_t dst, uint64_t want);
#endif
