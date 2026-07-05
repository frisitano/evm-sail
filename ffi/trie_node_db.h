#ifndef TRIE_NODE_DB_H
#define TRIE_NODE_DB_H
#include "sail.h"
#include <stdbool.h>
#include <stdint.h>

/* C-backed witness node-db (ffi/trie_node_db.c): keccak(node) -> (offset, len)
 * span into the SSZ witness buffer (ssz_src). Replaces the Sail assoc-list +
 * linear node_db_lookup, which made re-rooting / lookups O(N^2) (a linear scan
 * per child reference, plus the whole list deep-copied through every recursive
 * walk -- Sail value semantics). This is an O(1)-amortized open-addressing hash
 * map, so the witness walk is O(N).
 *
 * The map stores spans, not bytes: node bytes stay in the witness buffer and
 * are materialized by offset only when a node is actually visited. The 256-bit
 * keccak key crosses the FFI as a whole lbits value. nodedb_sel caches the
 * matched span so the off/len accessors need no re-lookup. */
unit nodedb_reset(const unit u);
unit nodedb_insert(const lbits kh, uint64_t off, uint64_t len);
bool nodedb_sel(const lbits kh);
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
unit acctdb_insert(const lbits kh, uint64_t off, uint64_t len);
bool acctdb_sel(const lbits kh);
uint64_t acctdb_sel_off(const unit u);
uint64_t acctdb_sel_len(const unit u);
/* iteration over the harvested account leaves (the storage-harvest pass, which
 * authenticates every account's storage trie against its committed root). */
uint64_t acctdb_count(const unit u);
unit acctdb_at(uint64_t idx);
void acctdb_at_key(lbits *rop, const unit u); /* keccak(addr) of the cached row */
uint64_t acctdb_at_off(const unit u);
uint64_t acctdb_at_len(const unit u);

/* Witness storage-slot store: (keccak(addr), keccak(slot)) -> 256-bit value.
 * Harvested + authenticated during the witness pass; serves stateless storage
 * reads (point lookup) and the post-state-root pass (iteration). */
unit slotdb_reset(const unit u);
unit slotdb_insert(const lbits acct, const lbits slot, const lbits val);
bool slotdb_sel(const lbits acct, const lbits slot);
void slotdb_selval(lbits *rop, const unit u);
bool slotdb_sel_existed(const unit u);
uint64_t slotdb_count(const unit u);
unit slotdb_at(uint64_t idx);
void slotdb_at_acct(lbits *rop, const unit u);
void slotdb_at_slot(lbits *rop, const unit u);
void slotdb_at_val(lbits *rop, const unit u);
bool slotdb_at_existed(const unit u);

/* Storage-harvest completeness for optional proofs. Marked incomplete when a
 * storage re-root preserves a blinded child hash, so the post-state root code
 * knows slotdb is not a complete live map for that account. */
unit storage_mark_incomplete(const lbits a);
bool storage_harvest_complete(const lbits a);

#endif
