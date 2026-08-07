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
struct zEvmMemorySliceFields;
unit log_add_data_memory(struct zEvmMemorySliceFields data);

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(uint64_t current_transaction_epoch);
bool account_is_warm(fixed_bytes_20 a);
unit account_mark_warm(fixed_bytes_20 a);
bool storage_is_warm(fixed_bytes_20 a, const u256 s);
unit storage_mark_warm(fixed_bytes_20 a, const u256 s);

/* ---- EIP-7702 transaction-local authority tracker ---- */
unit authorization_tracker_reset(uint64_t count_hint);
bool authorization_tracker_seen(fixed_bytes_20 authority);
bool authorization_tracker_originally_delegated(fixed_bytes_20 authority);
bool authorization_tracker_delegation_set(fixed_bytes_20 authority);
unit authorization_tracker_commit(fixed_bytes_20 authority,
                                  bool originally_delegated,
                                  bool sets_delegation);

/* ---- BLOCKHASH ancestor table ---- */
unit ancestor_hash_write(uint64_t j, fixed_bytes_32 h);
fixed_bytes_32 ancestor_hash_read( uint64_t j);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit logs_reset(const unit u);
unit log_begin(fixed_bytes_20 a);        /* start a new record for emitter a      */
unit log_add_topic(u256 t);       /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
unit log_add_data_word(u256 value);
void logs_revert_last(void);
uint64_t log_count(const unit u);
uint64_t logs_tx_start(const unit u);
uint64_t logs_tx_count(const unit u);
fixed_bytes_20 log_addr( uint64_t i);
uint64_t log_topic_count(uint64_t i);
u256 log_topic( uint64_t i,
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
fixed_bytes_256 evmsail_block_logs_bloom(unit ignored);
bool evmsail_receipt_logs_bloom_write(uint64_t start, uint64_t count,
                                      uint8_t out[256]);

/* State-journal frame boundaries. Checkpoints and commits are structural
 * markers in the journal itself; no subsystem cursor is captured or passed
 * through the Sail ABI. */
unit state_journal_reset(const unit u);
unit state_journal_checkpoint(const unit u);
unit state_journal_revert(const unit u);
unit state_journal_commit(const unit u);

/* Spec-backend implementation hooks. Account and storage retain their typed
 * GMP-owning undo arenas, while these compact journal entries preserve the
 * global mutation order needed for frame reversion. */
uint32_t state_journal_current_frame_marker(void);
void state_journal_push_account_undo(void);
void state_journal_push_storage_undo(void);

/* Called by transient_storage.c before a public TSTORE write. */
unit state_journal_push_transient(fixed_bytes_20 a, u256 slot,
                                  u256 prior);

#endif
