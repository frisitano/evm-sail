/* Specification-ABI state adapters. This is compiled against the generated
 * model header named by EVMSAIL_MODEL_H, so structured Sail layouts are never
 * mirrored by hand in the scalar C stores. */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "state_db.h"
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>

static void storage_value_out(struct zoptionzIRStorageValuezK *out,
                              sail_fixed_bytes_20 addr,
                              sail_u256 slot) {
  sail_u256 curr, orig;
  if (storage_row_probe(1, addr, slot, &curr, &orig)) {
    out->kind = Kind_zSomezIRStorageValuezK;
    out->variants.zSomezIRStorageValuezK.zcurr = curr;
    out->variants.zSomezIRStorageValuezK.zorig = orig;
  } else {
    out->kind = Kind_zNonezIRStorageValuezK;
    out->variants.zNonezIRStorageValuezK = UNIT;
  }
}

void storage_tx_get(struct zStorageTxLookup *out, struct zStorageKey key) {
  sail_u256 curr, orig;
  const uint64_t status =
      storage_row_probe(0, key.zaddr, key.zslot, &curr, &orig);
  if (status == 1) {
    out->kind = Kind_zStorageTxHit;
    out->variants.zStorageTxHit.zcurr = curr;
    out->variants.zStorageTxHit.zorig = orig;
  } else if (status == 2) {
    out->kind = Kind_zStorageTxCleared;
    out->variants.zStorageTxCleared = UNIT;
  } else {
    out->kind = Kind_zStorageTxMiss;
    out->variants.zStorageTxMiss = UNIT;
  }
}

void storage_block_get(struct zoptionzIRStorageValuezK *out,
                       struct zStorageKey key) {
  storage_value_out(out, key.zaddr, key.zslot);
}

void storage_block_iter_next(struct zoptionzIRStorageTrieEntryzK *out,
                             const sail_fixed_bytes_20 addr) {
  struct zStorageTrieEntry *trie_entry =
      &out->variants.zSomezIRStorageTrieEntryzK;
  struct zStorageEntry *entry = &trie_entry->zentry;
  if (storage_block_iter_next_probe(
          addr, &entry->zkey.zslot, &entry->zvalue.zcurr,
          &entry->zvalue.zorig, &trie_entry->zaddress_hash,
          &trie_entry->zslot_hash)) {
    evmsail_address_assign(&entry->zkey.zaddr, addr);
    out->kind = Kind_zSomezIRStorageTrieEntryzK;
  } else {
    out->kind = Kind_zNonezIRStorageTrieEntryzK;
    out->variants.zNonezIRStorageTrieEntryzK = UNIT;
  }
}

static void account_out(struct zoptionzIRAccountzK *out, uint64_t layer,
                        const sail_fixed_bytes_20 addr) {
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

unit storage_tx_update(struct zStorageEntry entry) {
  return storage_tx_update_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_put(struct zStorageEntry entry) {
  return storage_block_put_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_cache(struct zStorageKey key, const sail_fixed_bytes_32 slot_hash,
                         const sail_u256 value) {
  return storage_block_cache_raw(key.zaddr, key.zslot, slot_hash, value);
}

void acct_tx_get(struct zoptionzIRAccountzK *out, const sail_fixed_bytes_20 addr) {
  account_out(out, 0, addr);
}

void acct_block_get(struct zoptionzIRAccountzK *out,
                    const sail_fixed_bytes_20 addr) {
  account_out(out, 1, addr);
}

void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *out,
                          const unit u) {
  (void)u;
  struct zAcctTrieEntry *trie_entry =
      &out->variants.zSomezIRAcctTrieEntryzK;
  struct zAcctEntry *entry = &trie_entry->zentry;
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
          &orig->zselfdestructed, &trie_entry->zaddress_hash)) {
    out->kind = Kind_zSomezIRAcctTrieEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctTrieEntryzK;
    out->variants.zNonezIRAcctTrieEntryzK = UNIT;
  }
}

unit acct_tx_update(const sail_fixed_bytes_20 addr, struct zAccount account) {
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

unit acct_block_cache(const sail_fixed_bytes_20 addr, const sail_fixed_bytes_32 address_hash,
                      struct zAccount account) {
  return acct_block_cache_raw(
      addr, address_hash, account.zinfo.znonce, account.zinfo.zbalance,
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

void acct_tx_pop(struct zoptionzIRAcctEntryzK *out, unit u) {
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

void bal_iter_next(struct zBalIterEntry *out, unit u) {
  (void)u;

  sail_fixed_bytes_20 address;
  sail_u256 slot;
  sail_u256 value;
  sail_fixed_bytes_32 code_hash;
  uint64_t index;
  uint64_t nonce;

  switch (bal_iter_next_probe(&address, &slot, &index, &value, &nonce,
                              &code_hash)) {
  case BAL_ITER_ACCOUNT:
    out->kind = Kind_zBalAccount;
    out->variants.zBalAccount = address;
    return;
  case BAL_ITER_STORAGE_CHANGE: {
    struct zBalStorageChangeEntry *entry =
        &out->variants.zBalStorageChange;
    out->kind = Kind_zBalStorageChange;
    entry->zslot = slot;
    entry->zindex = index;
    entry->zvalue = value;
    return;
  }
  case BAL_ITER_STORAGE_READ:
    out->kind = Kind_zBalStorageRead;
    out->variants.zBalStorageRead = slot;
    return;
  case BAL_ITER_BALANCE_CHANGE: {
    struct zBalBalanceChangeEntry *entry =
        &out->variants.zBalBalanceChange;
    out->kind = Kind_zBalBalanceChange;
    entry->zindex = index;
    entry->zvalue = value;
    return;
  }
  case BAL_ITER_NONCE_CHANGE: {
    struct zBalNonceChangeEntry *entry = &out->variants.zBalNonceChange;
    out->kind = Kind_zBalNonceChange;
    entry->zindex = index;
    entry->zvalue = nonce;
    return;
  }
  case BAL_ITER_CODE_CHANGE: {
    struct zBalCodeChangeEntry *entry = &out->variants.zBalCodeChange;
    out->kind = Kind_zBalCodeChange;
    entry->zindex = index;
    entry->zcode_hash = code_hash;
    return;
  }
  case BAL_ITER_ACCOUNT_END:
    out->kind = Kind_zBalAccountEnd;
    out->variants.zBalAccountEnd = UNIT;
    return;
  case BAL_ITER_EMPTY:
    out->kind = Kind_zBalEmpty;
    out->variants.zBalEmpty = UNIT;
    return;
  }

  abort();
}
