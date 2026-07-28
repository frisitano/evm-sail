/* C-backed execution-time kernel collections (see kernel_state.c): the
 * EIP-2929 warm address/slot sets, the LOG series, and the private call-frame
 * checkpoint/undo machinery. SELFDESTRUCT/created markers live in transaction
 * Account rows.
 *
 * These were Sail registers holding mutable data buffers; they now live behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail and C-backed here. Declared here so the
 * Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * The shared ABI header exposes the fixed address/word/hash representations
 * selected by C specialization in both native build modes. */
#ifndef KERNEL_STATE_H
#define KERNEL_STATE_H
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdint.h>

/* Generated aggregate adapter implemented by hash.c. */
struct zMemorySliceFields;
unit log_add_data_memory(struct zMemorySliceFields data);

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(const unit u);
bool warm_addr_touch(sail_fixed_bytes_20 a);
bool warm_slot_touch(sail_fixed_bytes_20 a, const sail_u256 s);

/* ---- EIP-7702 transaction-local authority tracker ---- */
unit authorization_tracker_reset(uint64_t count_hint);
bool authorization_tracker_seen(sail_fixed_bytes_20 authority);
bool authorization_tracker_originally_delegated(sail_fixed_bytes_20 authority);
bool authorization_tracker_delegation_set(sail_fixed_bytes_20 authority);
unit authorization_tracker_commit(sail_fixed_bytes_20 authority,
                                  bool originally_delegated,
                                  bool sets_delegation);

/* ---- BLOCKHASH ancestor table ---- */
unit ancestor_hash_write(uint64_t j, sail_fixed_bytes_32 h);
sail_fixed_bytes_32 ancestor_hash_read( uint64_t j);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit logs_reset(const unit u);
unit log_begin(sail_fixed_bytes_20 a);        /* start a new record for emitter a      */
unit log_add_topic(sail_u256 t);       /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
unit log_add_data_word(sail_u256 value);
uint64_t logs_checkpoint(const unit u);
unit logs_revert(uint64_t checkpoint);
uint64_t log_count(const unit u);
uint64_t logs_tx_start(const unit u);
uint64_t logs_tx_count(const unit u);
sail_fixed_bytes_20 log_addr( uint64_t i);
uint64_t log_topic_count(uint64_t i);
sail_u256 log_topic( uint64_t i,
                              uint64_t j);
uint64_t log_data_len(uint64_t i);
unit logs_tx_reset(const unit u);
uint64_t log_data_off(uint64_t i);
const uint8_t *log_data_region(uint64_t off, uint64_t len);
bool log_data_configure_capacity(uint64_t capacity);
const uint8_t *log_data_base(void);
uint64_t log_data_length(void);

/* Optimized receipt and block bloom construction. The receipt writer sets
 * sparse bits directly in its final RLP span while maintaining the block-wide
 * 32-word accumulator, which is read once after all receipts are constructed. */
unit evmsail_block_logs_bloom_reset(unit ignored);
sail_fixed_bytes_256 evmsail_block_logs_bloom(unit ignored);
bool evmsail_receipt_logs_bloom_write(uint64_t start, uint64_t count,
                                      uint8_t out[256]);

/* ---- semantic call-frame checkpoints ----
 * Sail sees only the returned token. These functions privately coordinate
 * account/storage undo cursors, transient storage, warm sets, and logs. */
unit host_state_checkpoint_reset(const unit u);
uint64_t host_state_checkpoint(const unit u);
unit host_state_revert(uint64_t checkpoint);

/* Called by transient_storage.c before a public TSTORE write. */
unit state_journal_push_transient(sail_fixed_bytes_20 a, sail_u256 slot,
                                  sail_u256 prior);

#endif
