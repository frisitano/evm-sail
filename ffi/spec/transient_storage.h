/* C-backed transient storage table (see transient_storage.c).
 *
 * Persistent account storage is implemented separately in state_db.c/.h,
 * keyed by secure trie keys. Warm access sets and rollback coordination live
 * in kernel_state.c.
 *
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. The shared ABI header selects the canonical or
 * optimized nominal address/word representations. */
#ifndef TRANSIENT_STORAGE_H
#define TRANSIENT_STORAGE_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdint.h>

unit transient_storage_reset(const unit u);
unit transient_storage_write(sail_fixed_bytes_20 addr, const sail_u256 slot,
                             const sail_u256 v);
/* Private rollback write: does not append another undo entry. */
unit transient_storage_restore(sail_fixed_bytes_20 addr, sail_u256 slot,
                               sail_u256 v);
sail_u256 transient_storage_read(sail_fixed_bytes_20 addr,
                                 const sail_u256 slot);

#endif
