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
 * precedence lives in Sail (k_sload / k_sstore), not here.
 *
 * (3) acct_tx_get / acct_block_get: per-layer option(Account) views over
 * state_db.c's account tables (acct_row_probe); tx-over-block precedence
 * lives in Sail (k_aload).
 *
 * (4) storage_tx_pop / acct_tx_pop: side-effect-free drain pops over the
 * tx layers -- k_tx_merge IS the merge: it decides the EIP-7928 records
 * (bal_note_* sinks) and the block propagation (storage_block_* /
 * acct_block_* hooks) per popped row.
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
static void storage_value_out(struct zoptionzIRStorageValuezK *out,
                              uint64_t layer, const lbits a, const lbits s) {
  lbits cur, orig;
  if (storage_row_probe(layer, a, s, &cur, &orig)) {
    out->kind = Kind_zSomezIRStorageValuezK;
    out->variants.zSomezIRStorageValuezK.zcurr = cur;
    out->variants.zSomezIRStorageValuezK.zorig = orig;
  } else {
    out->kind = Kind_zNonezIRStorageValuezK;
    out->variants.zNonezIRStorageValuezK = UNIT;
  }
}

void storage_tx_get(struct zoptionzIRStorageValuezK *out, const lbits a,
                    const lbits s) {
  storage_value_out(out, 0, a, s);
}

void storage_block_get(struct zoptionzIRStorageValuezK *out, const lbits a,
                       const lbits s) {
  storage_value_out(out, 1, a, s);
}

/* per-layer account views: probe state_db's layer table and build
 * option(Account). */
static void acct_entry_out(struct zoptionzIRAccountzK *out, uint64_t layer,
                           const lbits a) {
  uint64_t nonce;
  lbits bal, sroot, chash;
  if (acct_row_probe(layer, a, &nonce, &bal, &sroot, &chash)) {
    out->kind = Kind_zSomezIRAccountzK;
    struct zAccount *acc = &out->variants.zSomezIRAccountzK;
    CONVERT_OF(sail_int, mach_int)(&acc->znonce, (mach_int)nonce);
    acc->zbalance = bal;
    acc->zstorage_root = sroot;
    acc->zcode_hash = chash;
  } else {
    out->kind = Kind_zNonezIRAccountzK;
    out->variants.zNonezIRAccountzK = UNIT;
  }
}

void acct_tx_get(struct zoptionzIRAccountzK *out, const lbits a) {
  acct_entry_out(out, 0, a);
}

void acct_block_get(struct zoptionzIRAccountzK *out, const lbits a) {
  acct_entry_out(out, 1, a);
}

/* k_tx_merge drain pops: side-effect-free option(row) views over the tx
 * layers; None = drained (the C probe resets the table). Sail decides the
 * EIP-7928 records and the block-layer propagation per row. */
void storage_tx_pop(struct zoptionzIRStorageEntryzK *out, unit u) {
  (void)u;
  struct zStorageEntry *e = &out->variants.zSomezIRStorageEntryzK;
  uint64_t flags = storage_tx_pop_probe(&e->zkey.zaddr, &e->zkey.zslot,
                                        &e->zvalue.zcurr, &e->zvalue.zorig);
  if (flags == 0) {
    out->kind = Kind_zNonezIRStorageEntryzK;
    out->variants.zNonezIRStorageEntryzK = UNIT;
    return;
  }
  out->kind = Kind_zSomezIRStorageEntryzK;
}

void acct_tx_pop(struct zoptionzIRAcctEntryzK *out, unit u) {
  (void)u;
  struct zAcctEntry *e = &out->variants.zSomezIRAcctEntryzK;
  uint64_t cn = 0, on = 0;
  uint64_t flags = acct_tx_pop_probe(&e->zaddr, &cn, &e->zvalue.zcurr.zbalance,
                                     &e->zvalue.zcurr.zstorage_root, &e->zvalue.zcurr.zcode_hash, &on,
                                     &e->zvalue.zorig.zbalance, &e->zvalue.zorig.zstorage_root,
                                     &e->zvalue.zorig.zcode_hash);
  if (flags == 0) {
    out->kind = Kind_zNonezIRAcctEntryzK;
    out->variants.zNonezIRAcctEntryzK = UNIT;
    return;
  }
  out->kind = Kind_zSomezIRAcctEntryzK;
  CONVERT_OF(sail_int, mach_int)(&e->zvalue.zcurr.znonce, (mach_int)cn);
  CONVERT_OF(sail_int, mach_int)(&e->zvalue.zorig.znonce, (mach_int)on);
}

/* state-root enumeration views: unfiltered block entries; Sail filters. The
 * storage entry's key.addr is the caller's query address. */
void storage_block_row(struct zStorageEntry *out, const lbits a, uint64_t j) {
  out->zkey.zaddr = a;
  storage_block_probe_row(a, j, &out->zkey.zslot, &out->zvalue.zcurr, &out->zvalue.zorig);
}

void acct_block_row(struct zAcctEntry *out, uint64_t i) {
  uint64_t cn = 0, on = 0;
  acct_block_probe_row(i, &out->zaddr, &cn, &out->zvalue.zcurr.zbalance,
                       &out->zvalue.zcurr.zstorage_root, &out->zvalue.zcurr.zcode_hash, &on,
                       &out->zvalue.zorig.zbalance, &out->zvalue.zorig.zstorage_root,
                       &out->zvalue.zorig.zcode_hash);
  CONVERT_OF(sail_int, mach_int)(&out->zvalue.zcurr.znonce, (mach_int)cn);
  CONVERT_OF(sail_int, mach_int)(&out->zvalue.zorig.znonce, (mach_int)on);
}

/* EIP-6780 deletion drain: pop one of the address's tx rows as option(slot);
 * Sail records the read and loops until None. */
void storage_tx_wipe(struct zoptionzIbzK *out, const lbits a) {
  lbits slot;
  if (storage_tx_wipe_probe(a, &slot)) {
    out->kind = Kind_zSomezIbzK;
    out->variants.zSomezIbzK = slot;
  } else {
    out->kind = Kind_zNonezIbzK;
    out->variants.zNonezIbzK = UNIT;
  }
}
