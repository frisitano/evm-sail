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
#include "sail_abi.h"
#include <stdint.h>

unit transient_storage_reset(const unit u);
unit transient_storage_write(sail_address addr, sail_word slot, sail_word v);
/* Private rollback write: does not append another undo entry. */
unit transient_storage_restore(sail_address addr, sail_word slot, sail_word v);
EVMSAIL_WORD_RETURN transient_storage_read(
    EVMSAIL_WORD_RESULT(result) sail_address addr, sail_word slot);

#endif
