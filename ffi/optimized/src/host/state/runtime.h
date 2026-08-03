/* C-backed execution-time kernel collections: the EIP-2929 warm address/slot
 * sets and EIP-7702 authority tracker. The LOG series lives in logs.c and is
 * re-exported here so existing aggregate includers keep one surface. State
 * rollback is owned by journal.c; SELFDESTRUCT/created markers live in
 * account state.
 *
 * These were Sail registers holding mutable data buffers; they now live behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail and C-backed here. Declared here so the
 * Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * The shared ABI header exposes the fixed address/word/hash representations
 * selected by C specialization in both native build modes. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_RUNTIME_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_RUNTIME_H
#include "evmsail/prelude.h"
#include "evmsail/host/state/primitives.h"
#include "host/state/logs.h"
#include <stdint.h>

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(uint64_t current_transaction_epoch);
bool account_is_warm(Address a);
unit account_mark_warm(Address a);
bool storage_is_warm(Address a, const U256 s);
unit storage_mark_warm(Address a, const U256 s);

/* ---- EIP-7702 transaction-local authority tracker ---- */
unit authorization_tracker_reset(uint64_t count_hint);
bool authorization_tracker_seen(Address authority);
bool authorization_tracker_originally_delegated(Address authority);
bool authorization_tracker_delegation_set(Address authority);
unit authorization_tracker_commit(Address authority,
                                  bool originally_delegated,
                                  bool sets_delegation);

/* ---- LOG series (ordered; variable topics/data) ---- */
#endif
