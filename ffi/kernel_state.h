/* C-backed execution-time kernel collections (see kernel_state.c): the
 * EIP-2929 warm address/slot sets, the LOG series, and the call-frame journal
 * (undo log). SELFDESTRUCT/created markers live in transaction Account rows.
 *
 * These were Sail registers holding mutable data buffers; they now live behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail and C-backed here. Declared here so the
 * Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * Addresses/words/hashes cross as whole lbits; counts, tags, and bytes cross
 * as mach_bits. */
#ifndef KERNEL_STATE_H
#define KERNEL_STATE_H
#include "sail.h"
#include <stdint.h>

/* Generated aggregate adapters implemented by journal_glue.c/hash_glue.c. */
struct zJEntry;
struct zBytes;
struct node_zz5listz8z5bvz9;
struct node_zz5listz8z5structz0zzLogEntryz9;
unit journal_push(struct zJEntry entry);
void journal_pop(struct zJEntry *result, unit u);
unit log_append_record(const lbits address,
                       struct node_zz5listz8z5bvz9 *topics,
                       struct zBytes data);
void logs_read_all(struct node_zz5listz8z5structz0zzLogEntryz9 **result,
                   unit u);

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(const unit u);
bool warm_addr_touch(const lbits a);
unit warm_addr_remove(const lbits a);
bool warm_slot_touch(const lbits a, const lbits s);
unit warm_slot_remove(const lbits a, const lbits s);

/* ---- BLOCKHASH ancestor table ---- */
unit ancestor_hash_write(uint64_t j, const lbits h);
void ancestor_hash_read(lbits *rop, uint64_t j);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit logs_reset(const unit u);
unit log_begin(const lbits a);         /* start a new record for emitter a      */
unit log_add_topic(const lbits t);     /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
uint64_t logs_checkpoint(const unit u);
unit logs_revert(uint64_t checkpoint);
uint64_t log_count(const unit u);
void log_addr(lbits *rop, uint64_t i);
uint64_t log_topic_count(uint64_t i);
void log_topic(lbits *rop, uint64_t i, uint64_t j);
uint64_t log_data_len(uint64_t i);
unit logs_tx_reset(const unit u); /* records only; data arena persists  */
uint64_t log_data_off(uint64_t i);
const uint8_t *log_data_region(uint64_t off, uint64_t len);

/* ---- call-frame journal (undo log) ----
 * A frame checkpoint is the current length. Revert dispatches entries from
 * the top down to that saved length; commit discards only the caller's scalar
 * checkpoint token, leaving child entries available to an enclosing revert. */
unit journal_reset(const unit u);
uint64_t journal_len(const unit u);
unit journal_push_tran(const lbits a, const lbits slot, const lbits val);
unit journal_push_warma(const lbits a);
unit journal_push_warms(const lbits a, const lbits slot);
uint64_t journal_top_tag(const unit u);
unit journal_drop_top(const unit u);
void journal_top_addr(lbits *rop, const unit u);
void journal_top_slot(lbits *rop, const unit u);
void journal_top_val(lbits *rop, const unit u);

#endif
