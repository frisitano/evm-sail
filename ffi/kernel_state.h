/* C-backed execution-time kernel collections (see kernel_state.c): the
 * EIP-2929 warm address/slot sets, the LOG series, the SELFDESTRUCT and
 * EIP-6780 created-this-tx address sets, and the call-frame journal (undo log).
 *
 * These were Sail registers holding mutable data buffers; they now live behind
 * the abstract host interface (sail/iface/kernel_state.sail), C-backed here in
 * build mode. Declared here so the Sail-generated C call sites are prototyped
 * via `sail -c --c-include`. Addresses/words/hashes cross as whole lbits;
 * counts, tags, bytes and the refund word cross as mach_bits. */
#ifndef KERNEL_STATE_H
#define KERNEL_STATE_H
#include "sail.h"
#include <stdint.h>

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(const unit u);
bool warm_addr_contains(const lbits a);
unit warm_addr_insert(const lbits a);
unit warm_addr_remove(const lbits a);
bool warm_slot_contains(const lbits a, const lbits s);
unit warm_slot_insert(const lbits a, const lbits s);
unit warm_slot_remove(const lbits a, const lbits s);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit headerhash_reset(const unit u);
unit headerhash_push(const lbits h);
void headerhash_get(lbits *rop, uint64_t i);

unit logs_reset(const unit u);
unit log_begin(const lbits a);         /* start a new record for emitter a      */
unit log_add_topic(const lbits t);     /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
unit log_drop_last(const unit u);      /* drop the most recent record (revert)   */
uint64_t log_count(const unit u);
void log_addr(lbits *rop, uint64_t i);
uint64_t log_topic_count(uint64_t i);
void log_topic(lbits *rop, uint64_t i, uint64_t j);
uint64_t log_data_len(uint64_t i);
const uint8_t *log_data_ptr(uint64_t i);

/* ---- SELFDESTRUCT set (ordered; push/drop-last, contains, enumerate) ---- */
unit selfdestr_reset(const unit u);
unit selfdestr_push(const lbits a);
unit selfdestr_drop_last(const unit u);
bool selfdestr_contains(const lbits a);
uint64_t selfdestr_count(const unit u);
void selfdestr_get(lbits *rop, uint64_t i);

/* ---- EIP-6780 created-this-tx set (idempotent insert, contains) ---- */
unit created_reset(const unit u);
unit created_insert(const lbits a);
bool created_contains(const lbits a);

/* ---- call-frame journal (undo log) ----
 * push_* append an entry; commit drops the most recent checkpoint (folding the
 * frame's entries into the parent); revert reads the top entry (tag + payload),
 * dispatches its undo in Sail, then drops it, back to and including the
 * checkpoint. top_tag returns 0 (JT_EMPTY) when the journal is empty. */
unit journal_reset(const unit u);
unit journal_push_check(const unit u);
unit journal_push_acct(const lbits a, uint64_t nonce, const lbits bal, const lbits sroot, const lbits chash);
unit journal_push_tran(const lbits a, const lbits slot, const lbits val);
unit journal_push_stor(const lbits a, const lbits slot, const lbits prior);
unit journal_push_warma(const lbits a);
unit journal_push_warms(const lbits a, const lbits slot);
unit journal_push_log(const unit u);
unit journal_push_refund(uint64_t old);
unit journal_push_selfd(const unit u);
unit journal_commit(const unit u);
uint64_t journal_top_tag(const unit u);
unit journal_drop_top(const unit u);
void journal_top_addr(lbits *rop, const unit u);
uint64_t journal_top_nonce(const unit u);
void journal_top_balance(lbits *rop, const unit u);
void journal_top_sroot(lbits *rop, const unit u);
void journal_top_chash(lbits *rop, const unit u);
void journal_top_slot(lbits *rop, const unit u);
void journal_top_val(lbits *rop, const unit u);
uint64_t journal_top_refund(const unit u);

#endif
