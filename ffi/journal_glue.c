/* Generated-type glue: the ONE hand-written C file that handles generated
 * Sail types, compiled per build against that build's model header
 * (EVMSAIL_MODEL_H, e.g. "zkvm_runner.h" / "zkvm_block.h") so it always sees
 * the exact generated layout -- never a hand-mirrored one. Two boundaries:
 *
 * (1) journal_push / journal_pop (sail/host/state.sail): whole JEntry union
 * values cross; entry storage stays in the scalar journal rows
 * (kernel_state.c), this file only (en/de)codes. Contract: LIFO --
 * journal_pop after journal_push(e) yields e; pop on an empty journal yields
 * JCheck without popping anything.
 *
 * (2) storage_tx_get / storage_block_get: per-layer option(StorageEntry)
 * views over state_db.c's row tables (storage_row_probe); the layer
 * precedence lives in Sail (k_sload / k_sload_orig), not here.
 */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "state_db.h"

/* C journal row tags (kernel_state.c jrn enum; C-internal -- the Sail side
 * no longer sees tags). */
enum {
  GJT_EMPTY = 0, GJT_CHECK = 1, GJT_ACCT = 2, GJT_TRAN = 3, GJT_WARMA = 4,
  GJT_WARMS = 5, GJT_LOG = 6, GJT_REFUND = 7, GJT_SELFD = 8, GJT_STOR = 9
};

unit journal_push(struct zJEntry e) {
  switch (e.kind) {
  case Kind_zJCheck:
    return journal_push_check(UNIT);
  case Kind_zJAcct:
    return journal_push_acct(
        e.variants.zJAcct.ztup0,
        (uint64_t)CONVERT_OF(mach_int, sail_int)(e.variants.zJAcct.ztup1.znonce),
        e.variants.zJAcct.ztup1.zbalance,
        e.variants.zJAcct.ztup1.zstorage_root,
        e.variants.zJAcct.ztup1.zcode_hash);
  case Kind_zJTran:
    return journal_push_tran(e.variants.zJTran.ztup0, e.variants.zJTran.ztup1,
                             e.variants.zJTran.ztup2);
  case Kind_zJStor:
    return journal_push_stor(e.variants.zJStor.ztup0, e.variants.zJStor.ztup1,
                             e.variants.zJStor.ztup2);
  case Kind_zJWarmA:
    return journal_push_warma(e.variants.zJWarmA);
  case Kind_zJWarmS:
    return journal_push_warms(e.variants.zJWarmS.ztup0,
                              e.variants.zJWarmS.ztup1);
  case Kind_zJLog:
    return journal_push_log(UNIT);
  case Kind_zJRefund:
    /* two's-complement roundtrip through the row's 64-bit word (the Sail side
     * previously pushed get_slice_int(64, refund, 0)) */
    return journal_push_refund(
        (uint64_t)CONVERT_OF(mach_int, sail_int)(e.variants.zJRefund));
  case Kind_zJSelfD:
    return journal_push_selfd(UNIT);
  }
  return UNIT;
}

/* address payloads are bits(160): read the row's 256-bit word back and narrow
 * the view (the stored magnitude fits in 160 bits -- it was pushed from an
 * lbits of length 160). Mirrors the former Sail decode's [159 .. 0]. */
static void top_addr160(lbits *rop) {
  journal_top_addr(rop, UNIT);
  rop->len = 160;
}

void journal_pop(struct zJEntry *out, unit u) {
  (void)u;
  uint64_t tag = journal_top_tag(UNIT);
  switch (tag) {
  case GJT_EMPTY: /* empty journal: a JCheck view; nothing to pop */
    out->kind = Kind_zJCheck;
    out->variants.zJCheck = UNIT;
    return;
  case GJT_ACCT:
    out->kind = Kind_zJAcct;
    top_addr160(&out->variants.zJAcct.ztup0);
    CONVERT_OF(sail_int, mach_int)(&out->variants.zJAcct.ztup1.znonce,
                                   (mach_int)journal_top_nonce(UNIT));
    journal_top_balance(&out->variants.zJAcct.ztup1.zbalance, UNIT);
    journal_top_sroot(&out->variants.zJAcct.ztup1.zstorage_root, UNIT);
    journal_top_chash(&out->variants.zJAcct.ztup1.zcode_hash, UNIT);
    break;
  case GJT_TRAN:
    out->kind = Kind_zJTran;
    top_addr160(&out->variants.zJTran.ztup0);
    journal_top_slot(&out->variants.zJTran.ztup1, UNIT);
    journal_top_val(&out->variants.zJTran.ztup2, UNIT);
    break;
  case GJT_STOR:
    out->kind = Kind_zJStor;
    top_addr160(&out->variants.zJStor.ztup0);
    journal_top_slot(&out->variants.zJStor.ztup1, UNIT);
    journal_top_val(&out->variants.zJStor.ztup2, UNIT);
    break;
  case GJT_WARMA:
    out->kind = Kind_zJWarmA;
    top_addr160(&out->variants.zJWarmA);
    break;
  case GJT_WARMS:
    out->kind = Kind_zJWarmS;
    top_addr160(&out->variants.zJWarmS.ztup0);
    journal_top_slot(&out->variants.zJWarmS.ztup1, UNIT);
    break;
  case GJT_LOG:
    out->kind = Kind_zJLog;
    out->variants.zJLog = UNIT;
    break;
  case GJT_REFUND:
    out->kind = Kind_zJRefund;
    CONVERT_OF(sail_int, mach_int)(&out->variants.zJRefund,
                                   (mach_int)journal_top_refund(UNIT));
    break;
  case GJT_SELFD:
    out->kind = Kind_zJSelfD;
    out->variants.zJSelfD = UNIT;
    break;
  default: /* GJT_CHECK: the checkpoint marker (no undo payload) */
    out->kind = Kind_zJCheck;
    out->variants.zJCheck = UNIT;
    break;
  }
  journal_drop_top(UNIT);
}

/* per-layer storage entry views: probe state_db's layer table and build
 * option(StorageEntry) -- None = the layer never touched the slot; Some
 * carries (curr, orig). The written/read distinction stays C-internal (a
 * cached read has curr == orig). */
static void storage_entry_out(struct zoptionzIRStorageEntryzK *out,
                              uint64_t layer, const lbits a, const lbits s) {
  lbits cur, orig;
  if (storage_row_probe(layer, a, s, &cur, &orig)) {
    out->kind = Kind_zSomezIRStorageEntryzK;
    out->variants.zSomezIRStorageEntryzK.zcurr = cur;
    out->variants.zSomezIRStorageEntryzK.zorig = orig;
  } else {
    out->kind = Kind_zNonezIRStorageEntryzK;
    out->variants.zNonezIRStorageEntryzK = UNIT;
  }
}

void storage_tx_get(struct zoptionzIRStorageEntryzK *out, const lbits a,
                    const lbits s) {
  storage_entry_out(out, 0, a, s);
}

void storage_block_get(struct zoptionzIRStorageEntryzK *out, const lbits a,
                       const lbits s) {
  storage_entry_out(out, 1, a, s);
}
