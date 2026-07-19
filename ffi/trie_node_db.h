#ifndef TRIE_NODE_DB_H
#define TRIE_NODE_DB_H
#include "quantity_abi.h"
#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>

/* C-backed witness node-db (ffi/trie_node_db.c): keccak(node) -> (offset, len)
 * span into the SSZ witness buffer (ssz_src). Replaces the Sail assoc-list +
 * linear node_db_lookup, which made re-rooting / lookups O(N^2) (a linear scan
 * per child reference, plus the whole list deep-copied through every recursive
 * walk -- Sail value semantics). This is an O(1)-amortized open-addressing hash
 * map, so a lazy witness walk is O(depth).
 *
 * The map stores spans, not bytes: node bytes stay in the witness buffer and
 * are materialized by offset only when a node is actually visited. The 256-bit
 * keccak key crosses the FFI as a whole lbits value. Lookups are
 * argument-keyed getters (no cursor): nodedb_len(kh) is 0 exactly when the
 * key is absent (nodes are never empty), and a memoized last-key probe
 * serves the off/len pair with one table walk. */
unit nodedb_reset(const unit u);
unit nodedb_insert(sail_hash kh, EVMSAIL_BYTE_QUANTITY_PARAM(off),
                   EVMSAIL_BYTE_QUANTITY_PARAM(len));
#ifdef EVMSAIL_STANDARD_ABI
void nodedb_off(sail_int *out, sail_hash kh);
void nodedb_len(sail_int *out, sail_hash kh);
#else
uint64_t nodedb_off(sail_hash kh);
uint64_t nodedb_len(sail_hash kh);
#endif

#endif
