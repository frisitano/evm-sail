/* C-backed account and persistent-storage state database for evm-sail.
 *
 * One account table is keyed by raw address and one storage table by raw
 * (address, slot). Each row carries cumulative block state, an optional
 * transaction projection, and BAL metadata. Storage generations are lifecycle
 * metadata rather than row identity. Cached secure trie hashes are derived
 * once when witness material is first loaded and used only for MPT traversal. */
#ifndef STATE_DB_H
#define STATE_DB_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdbool.h>
#include <stdint.h>

/* Generated account/storage adapters implemented by spec/state.c or
 * optimised/state.c. The Sail
 * operations already live in host/state.sail; these declarations only fix the
 * generated C calling convention for aggregate arguments and results. */
struct zStorageTxLookup;
struct zStorageBlockRow;
struct zStorageTxPopResult;
struct zStorageBlockIterResult;
struct zStorageKey;
struct zStorageEntry;
struct zBalIterEntry;
void storage_tx_get(struct zStorageTxLookup *result, struct zStorageKey key);
struct zStorageBlockRow storage_block_get(struct zStorageKey key);
void storage_block_iter_next(struct zStorageBlockIterResult *result,
                             fixed_bytes_20 address);
unit storage_tx_update(struct zStorageEntry entry);
unit storage_block_put(struct zStorageEntry entry);
unit storage_block_cache(struct zStorageKey key, fixed_bytes_32 slot_hash,
                         const u256 value);

struct zAccountRow;
struct zAcctTxPopResult;
struct zAcctBlockIterResult;
struct zAccount;
struct zAcctEntry;
struct zAccountRow acct_tx_get(fixed_bytes_20 address);
struct zAccountRow acct_block_get(fixed_bytes_20 address);
void acct_block_iter_next(struct zAcctBlockIterResult *result, unit u);
unit acct_tx_update(fixed_bytes_20 address, struct zAccount account);
unit acct_block_write(struct zAcctEntry entry);
unit acct_block_cache(fixed_bytes_20 address, fixed_bytes_32 address_hash,
                      struct zAccount account);

void storage_tx_pop(struct zStorageTxPopResult *result, unit u);
void acct_tx_pop(struct zAcctTxPopResult *result, unit u);

/* Shared persistent-storage rows with transaction and cumulative projections. */
unit storage_db_reset(const unit u);
/* Per-layer row probe (layer 0 = tx writes, 1 = block cache/update map). */
uint64_t storage_row_probe(uint64_t layer, fixed_bytes_20 a, u256 s,
                           u256 *cur, u256 *orig);
unit storage_tx_update_raw(fixed_bytes_20 a, u256 s, u256 v,
                           u256 orig);
void storage_secure_key(u256 slot, fixed_bytes_32 *slot_hash);
void storage_tx_revert_last(void);
unit storage_tx_clear(fixed_bytes_20 a);
unit storage_tx_reset(const unit u);
bool storage_has_writes(fixed_bytes_20 a);
unit storage_block_clear(fixed_bytes_20 a);

/* Debug snapshot enumeration over the cumulative storage table, grouped by
   keccak(address). This is not part of the Sail state-root interface. */
uint64_t storage_dump_count(u256 ak);
u256 storage_dump_slot(u256 ak, uint64_t j);
u256 storage_dump_value(u256 ak, uint64_t j);

/* Shared account rows with transaction and cumulative projections. */
unit acct_db_reset(const unit u);
/* per-layer account probe (layer 0 = tx, 1 = block): 0 = absent, 1 = present.
   Transaction-only fields are zero for block rows. */
uint64_t acct_row_probe(uint64_t layer, fixed_bytes_20 a, uint64_t *nonce,
                        u256 *bal, fixed_bytes_32 *sroot, fixed_bytes_32 *chash,
                        bool *exists, bool *storage_cleared,
                        bool *created, bool *selfdestructed);
unit acct_tx_update_raw(fixed_bytes_20 a, uint64_t nonce,
                        u256 bal, fixed_bytes_32 sroot, fixed_bytes_32 chash,
                        bool exists, bool storage_cleared, bool created,
                        bool selfdestructed);
void acct_secure_key(fixed_bytes_20 address, fixed_bytes_32 *address_hash);
unit acct_tx_set_balance(fixed_bytes_20 a, const u256 balance);
unit acct_tx_set_nonce(fixed_bytes_20 a, uint64_t nonce);
unit acct_tx_set_code_hash(fixed_bytes_20 a, fixed_bytes_32 code_hash);
void acct_tx_revert_last(void);
unit acct_tx_reset(const unit u);
/* Optimized whole-operation transaction merge. The standard build retains
 * the explicit Sail account-then-storage loops. */
unit evmsail_tx_merge(bool cancun_or_later, bool amsterdam_or_later,
                      uint64_t block_access_index);

/* Debug snapshot enumeration over cumulative account state. */
uint64_t acct_dump_count(const unit u);
u256 acct_dump_hkey(uint64_t i);
fixed_bytes_20 acct_dump_address(uint64_t i);
uint64_t acct_dump_nonce(uint64_t i);
u256 acct_dump_balance(uint64_t i);
u256 acct_dump_storage_root(uint64_t i);
u256 acct_dump_code_hash(uint64_t i);

/* EIP-7928 metadata shares the cumulative raw-key account/storage rows.
 * Sail owns canonical validation and consumes a non-destructive canonical
 * stream through generated-layout bindings. */
unit bal_reset(const unit u);
unit bal_prepare_iter(const unit u);
void bal_iter_next(struct zBalIterEntry *result, unit u);
enum bal_iter_tag {
  BAL_ITER_EMPTY = 0,
  BAL_ITER_ACCOUNT = 1,
  BAL_ITER_STORAGE_CHANGE = 2,
  BAL_ITER_STORAGE_READ = 3,
  BAL_ITER_BALANCE_CHANGE = 4,
  BAL_ITER_NONCE_CHANGE = 5,
  BAL_ITER_CODE_CHANGE = 6,
  BAL_ITER_ACCOUNT_END = 7
};
enum bal_iter_tag bal_iter_next_probe(fixed_bytes_20 *address, u256 *slot,
                                      uint64_t *index, u256 *value,
                                      uint64_t *nonce, fixed_bytes_32 *code_hash);

/* EIP-7928 record sinks and transaction-row enumeration. */
unit bal_note_storage_change(uint64_t index, fixed_bytes_20 a,
                             const u256 slot,
                             const u256 val);
unit bal_note_account_touch(fixed_bytes_20 a);
unit bal_note_storage_read(fixed_bytes_20 a, const u256 slot);
unit bal_note_balance_change(uint64_t index, fixed_bytes_20 a,
                             const u256 val);
unit bal_note_nonce_change(uint64_t index, fixed_bytes_20 a, uint64_t nonce);
unit bal_note_code_change(uint64_t index, fixed_bytes_20 a, fixed_bytes_32 chash);
/* Non-destructive ascending state-root iterators over the cumulative maps.
 * The indexed accessors expose the same prepared views directly to the
 * optimized C root builder, without routing its private cursors through the
 * global iterator state used by generated standard C. */
uint32_t storage_block_updates_prepare(fixed_bytes_20 a);
uint64_t storage_block_update_probe_at(uint32_t index, u256 *slot,
                                       u256 *curr, u256 *orig,
                                       fixed_bytes_32 *address_hash,
                                       fixed_bytes_32 *slot_hash);
unit storage_block_iter_begin(fixed_bytes_20 a);
uint64_t storage_block_iter_next_probe(fixed_bytes_20 a, u256 *slot,
                                       u256 *curr, u256 *orig,
                                       fixed_bytes_32 *address_hash,
                                       fixed_bytes_32 *slot_hash);
uint32_t acct_block_updates_prepare(void);
unit acct_block_iter_begin(const unit u);
uint64_t acct_block_iter_next_probe(fixed_bytes_20 *addr, uint64_t *cn,
                                    u256 *cb, fixed_bytes_32 *cs,
                                    fixed_bytes_32 *cc, bool *ce, bool *csc,
                                    bool *ccr, bool *csd, uint64_t *on,
                                    u256 *ob, fixed_bytes_32 *os,
                                    fixed_bytes_32 *oc, bool *oe, bool *osc,
                                    bool *ocr, bool *osd,
                                    fixed_bytes_32 *address_hash);
/* k_tx_merge drain pops (side-effect-free) + block propagation hooks */
uint64_t storage_tx_pop_probe(fixed_bytes_20 *addr, u256 *slot,
                              u256 *curr, u256 *orig);
unit storage_block_put_raw(fixed_bytes_20 a, u256 s_, u256 curr,
                           u256 orig);
unit storage_block_cache_raw(fixed_bytes_20 a, u256 s_, fixed_bytes_32 slot_hash,
                             u256 v);
uint64_t acct_tx_pop_probe(fixed_bytes_20 *addr,
                           uint64_t *cn, u256 *cb,
                           fixed_bytes_32 *cs, fixed_bytes_32 *cc,
                           bool *ce, bool *csc, bool *ccr, bool *csd,
                           uint64_t *on, u256 *ob,
                           fixed_bytes_32 *os, fixed_bytes_32 *oc,
                           bool *oe, bool *osc, bool *ocr, bool *osd);
unit acct_block_write_raw(fixed_bytes_20 a, uint64_t nonce, u256 bal,
                          fixed_bytes_32 sroot, fixed_bytes_32 chash,
                          bool exists, bool storage_cleared,
                          uint64_t ononce, u256 obal,
                          fixed_bytes_32 osroot, fixed_bytes_32 ochash,
                          bool oexists, bool ostorage_cleared);
unit acct_block_cache_raw(fixed_bytes_20 a, fixed_bytes_32 address_hash,
                          uint64_t nonce, u256 bal,
                          fixed_bytes_32 sroot, fixed_bytes_32 chash,
                          bool exists, bool storage_cleared);
#endif
