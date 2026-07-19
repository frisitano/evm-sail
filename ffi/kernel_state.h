/* C-backed execution-time kernel collections (see kernel_state.c): the
 * EIP-2929 warm address/slot sets, the LOG series, and the private call-frame
 * checkpoint/undo machinery. SELFDESTRUCT/created markers live in transaction
 * Account rows.
 *
 * These were Sail registers holding mutable data buffers; they now live behind
 * the abstract host interfaces declared in sail/host/state.sail and
 * sail/host/environment.sail and C-backed here. Declared here so the
 * Sail-generated C call sites are prototyped via `sail -c --c-include`.
 * The shared ABI header selects canonical Sail values for the standard build
 * and inline address/word/hash values for the optimized build. */
#ifndef KERNEL_STATE_H
#define KERNEL_STATE_H
#include "sail_abi.h"
#include <stdint.h>

/* Generated aggregate adapters implemented by journal_glue.c/hash_glue.c. */
struct zBytes;
struct node_zz5listz8z5bvz9;
struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9;
struct node_zz5listz8z5structz0zzLogEntryz9;
#ifdef EVMSAIL_STANDARD_ABI
typedef struct node_zz5listz8z5bvz9 *evmsail_word_list;
#else
typedef struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9
    *evmsail_word_list;
#endif
unit log_append_record(sail_address address, evmsail_word_list topics,
                       struct zBytes data);
void logs_read_all(struct node_zz5listz8z5structz0zzLogEntryz9 **result,
                   unit u);

/* ---- EIP-2929 warm sets ---- */
unit warm_reset(const unit u);
bool warm_addr_touch(sail_address a);
bool warm_slot_touch(sail_address a, sail_word s);

/* ---- BLOCKHASH ancestor table ---- */
unit ancestor_hash_write(uint64_t j, sail_hash h);
EVMSAIL_HASH_RETURN ancestor_hash_read(EVMSAIL_HASH_RESULT(result) uint64_t j);

/* ---- LOG series (ordered; variable topics/data) ---- */
unit logs_reset(const unit u);
unit log_begin(sail_address a);        /* start a new record for emitter a      */
unit log_add_topic(sail_word t);       /* append a topic to the current record  */
unit log_add_data_bulk(const uint8_t *p, uint64_t n); /* append data bytes (one memcpy) */
uint64_t logs_checkpoint(const unit u);
unit logs_revert(uint64_t checkpoint);
uint64_t log_count(const unit u);
EVMSAIL_ADDRESS_RETURN log_addr(EVMSAIL_ADDRESS_RESULT(result) uint64_t i);
uint64_t log_topic_count(uint64_t i);
EVMSAIL_WORD_RETURN log_topic(EVMSAIL_WORD_RESULT(result) uint64_t i,
                              uint64_t j);
uint64_t log_data_len(uint64_t i);
unit logs_tx_reset(const unit u); /* records only; data arena persists  */
uint64_t log_data_off(uint64_t i);
const uint8_t *log_data_region(uint64_t off, uint64_t len);

/* ---- semantic call-frame checkpoints ----
 * Sail sees only the returned token. These functions privately coordinate
 * account/storage undo cursors, transient storage, warm sets, and logs. */
unit host_state_checkpoint_reset(const unit u);
uint64_t host_state_checkpoint(const unit u);
unit host_state_revert(uint64_t checkpoint);

/* Called by transient_storage.c before a public TSTORE write. */
unit state_journal_push_transient(sail_address a, sail_word slot,
                                  sail_word prior);

#endif
