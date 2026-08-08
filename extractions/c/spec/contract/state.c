/* Specification-ABI state adapters. This is compiled against the generated
 * model header named by EVMSAIL_MODEL_H, so structured Sail layouts are never
 * mirrored by hand in the scalar C stores. */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "state_db.h"
#include "value_convert.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static struct zStorageBlockRow storage_value_out(fixed_bytes_20 addr,
                                                 u256 slot) {
  struct zStorageBlockRow out;
  u256 curr, orig;
  if (storage_row_probe(1, addr, slot, &curr, &orig)) {
    out.zfound = true;
    out.zvalue.zcurr = curr;
    out.zvalue.zorig = orig;
  } else {
    out.zfound = false;
    memset(&out.zvalue, 0, sizeof(out.zvalue));
  }
  return out;
}

void storage_tx_get(struct zStorageTxLookup *out, struct zStorageKey key) {
  u256 curr, orig;
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

struct zStorageBlockRow storage_block_get(struct zStorageKey key) {
  return storage_value_out(key.zaddr, key.zslot);
}

void storage_block_iter_next(struct zStorageBlockIterResult *out,
                             const fixed_bytes_20 addr) {
  struct zStorageTrieEntry *trie_entry =
      &out->variants.zStorageBlockIterRow;
  struct zStorageEntry *entry = &trie_entry->zentry;
  if (storage_block_iter_next_probe(
          addr, &entry->zkey.zslot, &entry->zvalue.zcurr,
          &entry->zvalue.zorig, &trie_entry->zaddress_hash,
          &trie_entry->zslot_hash)) {
    evmsail_address_assign(&entry->zkey.zaddr, addr);
    out->kind = Kind_zStorageBlockIterRow;
  } else {
    out->kind = Kind_zStorageBlockIterExhausted;
    out->variants.zStorageBlockIterExhausted = UNIT;
  }
}

static struct zAccountRow account_out(uint64_t layer,
                                      const fixed_bytes_20 addr) {
  struct zAccountRow out;
  struct zAccount *account = &out.zaccount;
  if (acct_row_probe(layer, addr, &account->zinfo.znonce,
                     &account->zinfo.zbalance, &account->zinfo.zstorage_root,
                     &account->zinfo.zcode_hash, &account->zpresent,
                     &account->zstorage_cleared, &account->zcreated,
                     &account->zselfdestructed)) {
    out.zfound = true;
  } else {
    out.zfound = false;
    memset(account, 0, sizeof(*account));
  }
  return out;
}

unit storage_tx_update(struct zStorageEntry entry) {
  return storage_tx_update_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_put(struct zStorageEntry entry) {
  return storage_block_put_raw(entry.zkey.zaddr, entry.zkey.zslot,
                               entry.zvalue.zcurr, entry.zvalue.zorig);
}

unit storage_block_cache(struct zStorageKey key, const fixed_bytes_32 slot_hash,
                         const u256 value) {
  return storage_block_cache_raw(key.zaddr, key.zslot, slot_hash, value);
}

struct zAccountRow acct_tx_get(const fixed_bytes_20 addr) {
  return account_out(0, addr);
}

struct zAccountRow acct_block_get(const fixed_bytes_20 addr) {
  return account_out(1, addr);
}

void acct_block_iter_next(struct zAcctBlockIterResult *out,
                          const unit u) {
  (void)u;
  struct zAcctTrieEntry *trie_entry =
      &out->variants.zAcctBlockIterRow;
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
    out->kind = Kind_zAcctBlockIterRow;
  } else {
    out->kind = Kind_zAcctBlockIterExhausted;
    out->variants.zAcctBlockIterExhausted = UNIT;
  }
}

unit acct_tx_update(const fixed_bytes_20 addr, struct zAccount account) {
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

unit acct_block_cache(const fixed_bytes_20 addr, const fixed_bytes_32 address_hash,
                      struct zAccount account) {
  return acct_block_cache_raw(
      addr, address_hash, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zpresent,
      account.zstorage_cleared);
}

void storage_tx_pop(struct zStorageTxPopResult *out, unit u) {
  (void)u;
  struct zStorageEntry *entry = &out->variants.zStorageTxPopRow;
  if (storage_tx_pop_probe(
          &entry->zkey.zaddr, &entry->zkey.zslot,
          &entry->zvalue.zcurr, &entry->zvalue.zorig)) {
    out->kind = Kind_zStorageTxPopRow;
  } else {
    out->kind = Kind_zStorageTxPopExhausted;
    out->variants.zStorageTxPopExhausted = UNIT;
  }
}

void acct_tx_pop(struct zAcctTxPopResult *out, unit u) {
  (void)u;
  struct zAcctEntry *entry = &out->variants.zAcctTxPopRow;
  struct zAccount *curr = &entry->zvalue.zcurr;
  struct zAccount *orig = &entry->zvalue.zorig;
  if (acct_tx_pop_probe(
          &entry->zaddr, &curr->zinfo.znonce, &curr->zinfo.zbalance,
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zpresent,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance, &orig->zinfo.zstorage_root,
          &orig->zinfo.zcode_hash, &orig->zpresent, &orig->zstorage_cleared,
          &orig->zcreated, &orig->zselfdestructed)) {
    out->kind = Kind_zAcctTxPopRow;
  } else {
    out->kind = Kind_zAcctTxPopExhausted;
    out->variants.zAcctTxPopExhausted = UNIT;
  }
}

void bal_iter_next(struct zBalIterEntry *out, unit u) {
  (void)u;

  fixed_bytes_20 address;
  u256 slot;
  u256 value;
  fixed_bytes_32 code_hash;
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
