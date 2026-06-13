#ifndef HOST_NODEDB_H
#define HOST_NODEDB_H
#include "sail.h"
#include <stdint.h>

/* C-backed witness node-db (ffi/host_nodedb.c): keccak(node) -> (offset, len)
 * span into the SSZ witness buffer (ssz_src). Replaces the Sail assoc-list +
 * linear node_db_lookup, which made re-rooting / lookups O(N^2) (a linear scan
 * per child reference, plus the whole list deep-copied through every recursive
 * walk -- Sail value semantics). This is an O(1)-amortized open-addressing hash
 * map, so the witness walk is O(N).
 *
 * The map stores spans, not bytes: node bytes stay in the witness buffer and
 * are materialized by offset only when a node is actually visited. Only uint64
 * crosses the FFI (matching ffi/host_map.c); the 256-bit keccak key arrives as
 * four big-endian words, k3 most significant. nodedb_sel caches the matched
 * span so the off/len accessors need no re-lookup. */
unit nodedb_reset(const unit u);
unit nodedb_insert(uint64_t k3, uint64_t k2, uint64_t k1, uint64_t k0,
                   uint64_t off, uint64_t len);
uint64_t nodedb_sel(uint64_t k3, uint64_t k2, uint64_t k1, uint64_t k0);
uint64_t nodedb_sel_off(const unit u);
uint64_t nodedb_sel_len(const unit u);

#endif
