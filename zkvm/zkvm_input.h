/* zkVM binding of the SSZ byte-source (sail/host/io.sail's ssz_src_byte/ssz_src_len)
 * to the private input (read_input's buffer). Pulls in keccak too so a single
 * `sail -c --c-include zkvm_input.h` covers all guest externs. */
#ifndef EL_INPUT_H
#define EL_INPUT_H
#include "sail.h"
#include "host_crypto.h"              /* direct host hash helpers */
#include "precompiles.h"              /* staged precompile input/execution */
#include "returndata.h"               /* C-backed RETURNDATA buffers */
#include "memory.h"
#include "transient_storage.h"
#include "stack.h"
#include "code_db.h"
#include "kernel_state.h"
#include "trie_node_db.h"
#include "state_db.h"
uint64_t ssz_src_len(const unit u);                  /* unit -> bits(64) (byte count) */
uint64_t ssz_src_byte(sail_int idx);                 /* int  -> bits(8) */
uint64_t ssz_src_le(sail_int off, sail_int n);       /* (int,int) -> bits(64): n<=8 LE */
uint64_t ssz_src_be(sail_int off, sail_int n);       /* (int,int) -> bits(64): n<=8 BE */
const uint8_t *evmsail_ssz_ptr(uint64_t off, uint64_t len);
unit     el_emit_out(uint64_t b);                    /* bits(8) -> unit: write_output one byte */
/* journal boundary (ffi/journal_glue.c): whole JEntry values cross as the
 * generated struct zJEntry -- declared incomplete here (this header precedes
 * the generated type definitions in the model translation unit). */
struct zJEntry;
unit journal_push(struct zJEntry e);                 /* JEntry -> unit */
void journal_pop(struct zJEntry *rop, unit u);       /* unit -> JEntry */
struct zStorageRow;                                  /* per-layer storage row views */
void storage_tx_row(struct zStorageRow *rop, const lbits a, const lbits s);
void storage_block_row(struct zStorageRow *rop, const lbits a, const lbits s);
#endif
