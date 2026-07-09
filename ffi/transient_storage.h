/* C-backed transient storage table (see transient_storage.c).
 *
 * Persistent account storage is implemented separately by storage_wset_* in
 * state_db.c/.h, keyed by secure trie keys. Warm access sets live in Sail.
 *
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. Addresses/slots/values cross as whole lbits. */
#ifndef TRANSIENT_STORAGE_H
#define TRANSIENT_STORAGE_H
#include "sail.h"
#include <stdint.h>

unit transient_storage_reset(const unit u);
unit transient_storage_write(const lbits addr, const lbits slot, const lbits v);
void transient_storage_read(lbits *rop, const lbits addr, const lbits slot);

#endif
