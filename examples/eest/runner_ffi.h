/* Declaration of the runner's stdin extern, injected into the Sail-generated
 * C via `sail -c --c-include runner_ffi.h` so the call site is declared. */
#ifndef EL_IR_RUNNER_FFI_H
#define EL_IR_RUNNER_FFI_H
#include "sail.h"
#include "../../ffi/acc_shim.h"   /* zkvm-standards crypto accelerator shim (keccak/sha256/precompiles) */
#include "../../ffi/el_mem.h"     /* C-backed EVM memory (O(1) read/write, per-frame) */
void next_int(sail_int rop, const unit u);
#endif
