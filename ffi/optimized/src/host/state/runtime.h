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

#endif
