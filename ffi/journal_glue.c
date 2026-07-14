/* Generated state/journal glue. This is compiled against the generated model
 * header named by EVMSAIL_MODEL_H, so structured Sail layouts are never
 * mirrored by hand in the scalar C stores. */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "state_db.h"
#include "lbits_convert.h"
#include <stdlib.h>

/* C-internal journal tags, mirrored from kernel_state.c. */
enum {
  GJT_EMPTY = 0, GJT_TRAN = 1, GJT_WARMA = 2,
  GJT_WARMS = 3, GJT_REFUND = 4
};

unit journal_push(struct zJEntry e) {
  switch (e.kind) {
  case Kind_zJTran:
    return journal_push_tran(e.variants.zJTran.ztup0,
                             e.variants.zJTran.ztup1,
                             e.variants.zJTran.ztup2);
  case Kind_zJWarmA:
    return journal_push_warma(e.variants.zJWarmA);
  case Kind_zJWarmS:
    return journal_push_warms(e.variants.zJWarmS.ztup0,
                              e.variants.zJWarmS.ztup1);
  case Kind_zJRefund:
    return journal_push_refund((uint64_t)e.variants.zJRefund);
  }
  return UNIT;
}

static void top_addr160(lbits *out) {
  journal_top_addr(out, UNIT);
  out->len = 160;
}

void journal_pop(struct zJEntry *out, unit u) {
  (void)u;
  switch (journal_top_tag(UNIT)) {
  case GJT_EMPTY:
    abort();
  case GJT_TRAN:
    out->kind = Kind_zJTran;
    top_addr160(&out->variants.zJTran.ztup0);
    journal_top_slot(&out->variants.zJTran.ztup1, UNIT);
    journal_top_val(&out->variants.zJTran.ztup2, UNIT);
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
  case GJT_REFUND:
    out->kind = Kind_zJRefund;
    out->variants.zJRefund = (int64_t)journal_top_refund(UNIT);
    break;
  default:
    abort();
  }
  journal_drop_top(UNIT);
}

static void storage_value_out(struct zoptionzIRStorageValuezK *out,
                              uint64_t layer, const lbits addr,
                              const lbits slot) {
  lbits curr, orig;
  if (storage_row_probe(layer, addr, slot, &curr, &orig)) {
    out->kind = Kind_zSomezIRStorageValuezK;
    out->variants.zSomezIRStorageValuezK.zcurr = curr;
    out->variants.zSomezIRStorageValuezK.zorig = orig;
  } else {
    out->kind = Kind_zNonezIRStorageValuezK;
    out->variants.zNonezIRStorageValuezK = UNIT;
  }
}

void storage_tx_get(struct zoptionzIRStorageValuezK *out,
                    struct zStorageKey key) {
  sail_expect_len(key.zaddr, 160);
  storage_value_out(out, 0, key.zaddr, key.zslot);
}

void storage_block_get(struct zoptionzIRStorageValuezK *out,
                       struct zStorageKey key) {
  sail_expect_len(key.zaddr, 160);
  storage_value_out(out, 1, key.zaddr, key.zslot);
}

unit storage_tx_update(struct zStorageEntry entry) {
  sail_expect_len(entry.zkey.zaddr, 160);
  return storage_tx_update_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_put(struct zStorageEntry entry) {
  return storage_block_put_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_cache(struct zStorageKey key, const sail_word value) {
  return storage_block_cache_raw(key.zaddr, key.zslot, value);
}

static void account_out(struct zoptionzIRAccountzK *out, uint64_t layer,
                        const sail_address addr) {
  struct zAccount *account = &out->variants.zSomezIRAccountzK;
  if (acct_row_probe(layer, addr, &account->zinfo.znonce,
                     &account->zinfo.zbalance, &account->zinfo.zstorage_root,
                     &account->zinfo.zcode_hash, &account->zexists,
                     &account->zstorage_cleared, &account->zcreated,
                     &account->zselfdestructed)) {
    out->kind = Kind_zSomezIRAccountzK;
  } else {
    out->kind = Kind_zNonezIRAccountzK;
    out->variants.zNonezIRAccountzK = UNIT;
  }
}

void acct_tx_get(struct zoptionzIRAccountzK *out, const sail_address addr) {
  account_out(out, 0, addr);
}

void acct_block_get(struct zoptionzIRAccountzK *out,
                    const sail_address addr) {
  account_out(out, 1, addr);
}

unit acct_tx_update(const sail_address addr, struct zAccount account) {
  return acct_tx_update_raw(
      addr, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zexists,
      account.zstorage_cleared, account.zcreated, account.zselfdestructed);
}

unit acct_block_write(struct zAcctEntry entry) {
  struct zAccount *curr = &entry.zvalue.zcurr;
  struct zAccount *orig = &entry.zvalue.zorig;
  return acct_block_write_raw(
      entry.zaddr, curr->zinfo.znonce, curr->zinfo.zbalance,
      curr->zinfo.zstorage_root, curr->zinfo.zcode_hash, curr->zexists,
      curr->zstorage_cleared, orig->zinfo.znonce, orig->zinfo.zbalance,
      orig->zinfo.zstorage_root, orig->zinfo.zcode_hash, orig->zexists,
      orig->zstorage_cleared);
}

unit acct_block_cache(const sail_address addr, struct zAccount account) {
  return acct_block_cache_raw(
      addr, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zexists,
      account.zstorage_cleared);
}

void storage_tx_pop(struct zoptionzIRStorageEntryzK *out, unit u) {
  (void)u;
  struct zStorageEntry *entry = &out->variants.zSomezIRStorageEntryzK;
  if (storage_tx_pop_probe(
          &entry->zkey.zaddr, &entry->zkey.zslot,
          &entry->zvalue.zcurr, &entry->zvalue.zorig)) {
    out->kind = Kind_zSomezIRStorageEntryzK;
  } else {
    out->kind = Kind_zNonezIRStorageEntryzK;
    out->variants.zNonezIRStorageEntryzK = UNIT;
  }
}

void acct_tx_pop_ascending(struct zoptionzIRAcctEntryzK *out, unit u) {
  (void)u;
  struct zAcctEntry *entry = &out->variants.zSomezIRAcctEntryzK;
  struct zAccount *curr = &entry->zvalue.zcurr;
  struct zAccount *orig = &entry->zvalue.zorig;
  if (acct_tx_pop_probe(
          &entry->zaddr, &curr->zinfo.znonce, &curr->zinfo.zbalance,
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zexists,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance, &orig->zinfo.zstorage_root,
          &orig->zinfo.zcode_hash, &orig->zexists, &orig->zstorage_cleared,
          &orig->zcreated, &orig->zselfdestructed)) {
    out->kind = Kind_zSomezIRAcctEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctEntryzK;
    out->variants.zNonezIRAcctEntryzK = UNIT;
  }
}
