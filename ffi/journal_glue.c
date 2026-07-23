/* Generated account/storage glue. This is compiled against the generated
 * model header named by EVMSAIL_MODEL_H, so structured Sail layouts are never
 * mirrored by hand in the scalar C stores. Call-frame journaling is private
 * to kernel_state.c and no generated journal value crosses this boundary. */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "state_db.h"
#include "value_convert.h"
#include <stdlib.h>

static void storage_value_out(struct zoptionzIRStorageValuezK *out,
                              uint64_t layer, sail_address addr,
                              sail_word slot) {
  sail_word curr, orig;
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
  storage_value_out(out, 0, key.zaddr, key.zslot);
}

void storage_block_get(struct zoptionzIRStorageValuezK *out,
                       struct zStorageKey key) {
  storage_value_out(out, 1, key.zaddr, key.zslot);
}

void storage_block_iter_next(struct zoptionzIRStorageEntryzK *out,
                             const sail_address addr) {
  struct zStorageEntry *entry = &out->variants.zSomezIRStorageEntryzK;
  if (storage_block_iter_next_probe(addr, &entry->zkey.zslot,
                                    &entry->zvalue.zcurr,
                                    &entry->zvalue.zorig)) {
    evmsail_address_assign(&entry->zkey.zaddr, addr);
    out->kind = Kind_zSomezIRStorageEntryzK;
  } else {
    out->kind = Kind_zNonezIRStorageEntryzK;
    out->variants.zNonezIRStorageEntryzK = UNIT;
  }
}

unit storage_tx_update(struct zStorageEntry entry) {
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
                     &account->zinfo.zcode_hash, &account->zpresent,
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

void acct_block_iter_next(struct zoptionzIRAcctEntryzK *out, const unit u) {
  (void)u;
  struct zAcctEntry *entry = &out->variants.zSomezIRAcctEntryzK;
  struct zAccount *curr = &entry->zvalue.zcurr;
  struct zAccount *orig = &entry->zvalue.zorig;
  if (acct_block_iter_next_probe(
          &entry->zaddr, &curr->zinfo.znonce,
          &curr->zinfo.zbalance,
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zpresent,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance,
          &orig->zinfo.zstorage_root, &orig->zinfo.zcode_hash, &orig->zpresent,
          &orig->zstorage_cleared, &orig->zcreated,
          &orig->zselfdestructed)) {
    out->kind = Kind_zSomezIRAcctEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctEntryzK;
    out->variants.zNonezIRAcctEntryzK = UNIT;
  }
}

void acct_debug_iter_next(struct zoptionzIRAcctEntryzK *out, const unit u) {
  (void)u;
  struct zAcctEntry *entry = &out->variants.zSomezIRAcctEntryzK;
  struct zAccount *curr = &entry->zvalue.zcurr;
  struct zAccount *orig = &entry->zvalue.zorig;
  if (acct_debug_iter_next_probe(
          &entry->zaddr, &curr->zinfo.znonce,
          &curr->zinfo.zbalance,
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zpresent,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance,
          &orig->zinfo.zstorage_root, &orig->zinfo.zcode_hash, &orig->zpresent,
          &orig->zstorage_cleared, &orig->zcreated,
          &orig->zselfdestructed)) {
    out->kind = Kind_zSomezIRAcctEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctEntryzK;
    out->variants.zNonezIRAcctEntryzK = UNIT;
  }
}

unit acct_tx_update(const sail_address addr, struct zAccount account) {
  return acct_tx_update_raw(
      addr, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zpresent,
      account.zstorage_cleared, account.zcreated, account.zselfdestructed);
}

unit acct_block_write(struct zAcctEntry entry) {
  struct zAccount *curr = &entry.zvalue.zcurr;
  struct zAccount *orig = &entry.zvalue.zorig;
  return acct_block_write_raw(
      entry.zaddr, curr->zinfo.znonce, curr->zinfo.zbalance,
      curr->zinfo.zstorage_root, curr->zinfo.zcode_hash, curr->zpresent,
      curr->zstorage_cleared, orig->zinfo.znonce, orig->zinfo.zbalance,
      orig->zinfo.zstorage_root, orig->zinfo.zcode_hash, orig->zpresent,
      orig->zstorage_cleared);
}

unit acct_block_cache(const sail_address addr, struct zAccount account) {
  return acct_block_cache_raw(
      addr, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zpresent,
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
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zpresent,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance, &orig->zinfo.zstorage_root,
          &orig->zinfo.zcode_hash, &orig->zpresent, &orig->zstorage_cleared,
          &orig->zcreated, &orig->zselfdestructed)) {
    out->kind = Kind_zSomezIRAcctEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctEntryzK;
    out->variants.zNonezIRAcctEntryzK = UNIT;
  }
}
