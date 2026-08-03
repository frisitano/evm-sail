/* C-backed transient storage table (see transient_storage.c).
 *
 * Persistent account storage is implemented separately in kernel/state.c/.h,
 * keyed by secure trie keys. Warm access sets and rollback coordination live
 * in kernel_state.c.
 *
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. The shared ABI header selects the canonical or
 * optimized nominal address/word representations. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_TRANSIENT_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_TRANSIENT_H
#include "evmsail/prelude.h"
#include <stdint.h>

unit transient_storage_reset(const unit u);
unit transient_storage_write(Address addr, const U256 slot,
                             const U256 v);
/* Private rollback write: does not append another undo entry. */
unit transient_storage_restore(Address addr, U256 slot,
                               U256 v);
U256 transient_storage_read(Address addr,
                                 const U256 slot);

#endif
