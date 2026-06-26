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

/* Witness account-leaf store: keccak(addr) -> (offset, len) span of the account's
 * leaf node in the SSZ witness buffer. Recorded during the single state re-root
 * walk (we know keccak(addr) = the 64-nibble path, and recover the leaf node's
 * span via keccak(node) -> nodedb). A stateless account read hashes the address
 * once, looks the span up here (O(1)), and decodes the leaf lazily -- so accounts
 * the block never touches are never decoded, and nothing keeps leaf bytes Sail-
 * side. Same span-not-bytes model as the node-db. */
unit acctdb_reset(const unit u);
unit acctdb_insert(uint64_t k3, uint64_t k2, uint64_t k1, uint64_t k0,
                   uint64_t off, uint64_t len);
uint64_t acctdb_sel(uint64_t k3, uint64_t k2, uint64_t k1, uint64_t k0);
uint64_t acctdb_sel_off(const unit u);
uint64_t acctdb_sel_len(const unit u);
/* iteration over the harvested account leaves (the storage-harvest pass, which
 * authenticates every account's storage trie against its committed root). */
uint64_t acctdb_count(const unit u);
unit acctdb_at(uint64_t idx);
uint64_t acctdb_at_key(uint64_t w); /* keccak(addr) word (3=most significant) */
uint64_t acctdb_at_off(const unit u);
uint64_t acctdb_at_len(const unit u);

/* Witness storage-slot store: (keccak(addr), keccak(slot)) -> 256-bit value.
 * Harvested + authenticated during the witness pass; serves stateless storage
 * reads (point lookup) and the post-state-root pass (iteration). */
unit slotdb_reset(const unit u);
unit slotdb_insert(uint64_t a3, uint64_t a2, uint64_t a1, uint64_t a0,
                   uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0,
                   uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0);
uint64_t slotdb_sel(uint64_t a3, uint64_t a2, uint64_t a1, uint64_t a0,
                    uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0);
uint64_t slotdb_selval(uint64_t i);
uint64_t slotdb_count(const unit u);
unit slotdb_at(uint64_t idx);
uint64_t slotdb_at_acct(uint64_t w);
uint64_t slotdb_at_slot(uint64_t w);
uint64_t slotdb_at_val(uint64_t w);

#endif
