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
#include "sail_abi.h"
#include <stdint.h>

/* Generated aggregate adapter implemented by hash_glue.c. */
struct zMemorySliceFields;
unit log_add_data_memory(struct zMemorySliceFields data);

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(const unit u);
bool warm_addr_touch(sail_address a);
bool warm_slot_touch(sail_address a, EVMSAIL_WORD_PARAM(s));

/* ---- EIP-7702 transaction-local authority tracker ---- */
unit authorization_tracker_reset(uint64_t count_hint);
bool authorization_tracker_seen(sail_address authority);
bool authorization_tracker_originally_delegated(sail_address authority);
bool authorization_tracker_delegation_set(sail_address authority);
unit authorization_tracker_commit(sail_address authority,
                                  bool originally_delegated,
                                  bool sets_delegation);

/* ---- BLOCKHASH ancestor table ---- */
unit ancestor_hash_write(uint64_t j, sail_hash h);
EVMSAIL_HASH_RETURN ancestor_hash_read(EVMSAIL_HASH_RESULT(result) uint64_t j);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit logs_reset(const unit u);
unit log_begin(sail_address a);        /* start a new record for emitter a      */
unit log_add_topic(sail_word t);       /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
unit log_add_data_word(sail_word value);
uint64_t logs_checkpoint(const unit u);
unit logs_revert(uint64_t checkpoint);
uint64_t log_count(const unit u);
uint64_t logs_tx_start(const unit u);
uint64_t logs_tx_count(const unit u);
EVMSAIL_ADDRESS_RETURN log_addr(EVMSAIL_ADDRESS_RESULT(result) uint64_t i);
uint64_t log_topic_count(uint64_t i);
EVMSAIL_WORD_RETURN log_topic(EVMSAIL_WORD_RESULT(result) uint64_t i,
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
sail_logs_bloom evmsail_block_logs_bloom(unit ignored);
bool evmsail_receipt_logs_bloom_write(uint64_t start, uint64_t count,
                                      uint8_t out[256]);

/* ---- semantic call-frame checkpoints ----
 * Sail sees only the returned token. These functions privately coordinate
 * account/storage undo cursors, transient storage, warm sets, and logs. */
unit host_state_checkpoint_reset(const unit u);
#ifdef EVMSAIL_STANDARD_ABI
void host_state_checkpoint(sail_int *result, const unit u);
unit host_state_revert(const sail_int checkpoint);
#else
uint64_t host_state_checkpoint(const unit u);
unit host_state_revert(uint64_t checkpoint);
#endif

/* Called by transient_storage.c before a public TSTORE write. */
unit state_journal_push_transient(sail_address a, sail_word slot,
                                  sail_word prior);

#endif
