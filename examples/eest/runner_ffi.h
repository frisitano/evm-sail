/* C-include for the EEST runner build (the ssz_src input source + crypto/
 * memory shims live in their own .c files; this just pulls the decls). */
#ifndef EL_IR_RUNNER_FFI_H
#define EL_IR_RUNNER_FFI_H
#include "sail.h"
#include "../../ffi/acc_shim.h"   /* zkvm-standards crypto accelerator shim (keccak/sha256/precompiles) */
#include "../../ffi/host_mem.h"     /* C-backed EVM memory (O(1) read/write, per-frame) */
/* SSZ byte-input source (definitions in runner_ffi.c: buffered stdin). */
void     ssz_src_len(sail_int rop, const unit u);
uint64_t ssz_src_byte(sail_int idx);
uint64_t ssz_src_le(sail_int off, sail_int n);
uint64_t ssz_src_be(sail_int off, sail_int n);
#endif
