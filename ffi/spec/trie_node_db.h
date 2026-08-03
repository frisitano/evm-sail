#ifndef TRIE_NODE_DB_H
#define TRIE_NODE_DB_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
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
 * keccak key crosses the FFI as one fixed 32-byte value. Lookups are
 * argument-keyed span lookup (no cursor). The model-aware region implementation
 * packages a successful span with its source tag; this table stays unaware of
 * generated aggregate layouts. */
unit nodedb_reset(const unit u);
unit nodedb_insert(sail_fixed_bytes_32 kh, uint64_t off, uint64_t len);
bool nodedb_lookup_span(sail_fixed_bytes_32 kh, uint64_t *off, uint64_t *len);
void nodedb_insert_digest(const sail_fixed_bytes_32 *hash, uint64_t off, uint64_t len);

#endif
