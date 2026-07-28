/* Optimized whole-operation state access. The generated fixed-layout account
 * types cross directly into the scalar C stores; the readable Sail bodies
 * remain the spec implementation. */
#include EVMSAIL_MODEL_H
#include "kernel_state.h"
#include "exception.h"
#include "state_db.h"
#include "value_convert.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "mpt.h"

static const uint8_t optimized_empty_trie_root[32] = {
    0x56, 0xe8, 0x1f, 0x17, 0x1b, 0xcc, 0x55, 0xa6,
    0xff, 0x83, 0x45, 0xe6, 0x92, 0xc0, 0xf8, 0x6e,
    0x5b, 0x48, 0xe0, 0x1b, 0x99, 0x6c, 0xad, 0xc0,
    0x01, 0x62, 0x2f, 0xb5, 0xe3, 0x63, 0xb4, 0x21,
};

static const uint8_t optimized_keccak_empty[32] = {
    0xc5, 0xd2, 0x46, 0x01, 0x86, 0xf7, 0x23, 0x3c,
    0x92, 0x7e, 0x7d, 0xb2, 0xdc, 0xc7, 0x03, 0xc0,
    0xe5, 0x00, 0xb6, 0x53, 0xca, 0x82, 0x27, 0x3b,
    0x7b, 0xfa, 0xd8, 0x04, 0x5d, 0x85, 0xa4, 0x70,
};

static bool optimized_word_zero(sail_u256 value) {
  return (value.limbs[0] | value.limbs[1] | value.limbs[2] |
          value.limbs[3]) == 0;
}

static bool optimized_word_equal(sail_u256 left, sail_u256 right) {
  return memcmp(&left, &right, sizeof(left)) == 0;
}

static bool optimized_hash_equal(sail_fixed_bytes_32 left, sail_fixed_bytes_32 right) {
  return memcmp(&left, &right, sizeof(left)) == 0;
}

static struct zAccount optimized_empty_account(void) {
  struct zAccount account;
  memset(&account, 0, sizeof(account));
  memcpy(account.zinfo.zstorage_root.bytes, optimized_empty_trie_root,
         sizeof(optimized_empty_trie_root));
  memcpy(account.zinfo.zcode_hash.bytes, optimized_keccak_empty,
         sizeof(optimized_keccak_empty));
  account.zstorage_cleared = true;
  return account;
}

static bool optimized_account_info_empty(const struct zAccountInfo *info) {
  return info->znonce == 0 && optimized_word_zero(info->zbalance) &&
         memcmp(info->zcode_hash.bytes, optimized_keccak_empty,
                sizeof(optimized_keccak_empty)) == 0;
}

static bool optimized_account_row(struct zAccount *account, uint64_t layer,
                                  sail_fixed_bytes_20 address) {
  return acct_row_probe(
      layer, address, &account->zinfo.znonce, &account->zinfo.zbalance,
      &account->zinfo.zstorage_root, &account->zinfo.zcode_hash,
      &account->zpresent, &account->zstorage_cleared, &account->zcreated,
      &account->zselfdestructed);
}

static enum zBlockError optimized_witness_error(uint64_t status) {
  return status == 1 ? zWitnessDeficient : zRlpDecode;
}

struct zAccount evmsail_k_aload(sail_fixed_bytes_32 parent_state_root,
                                sail_fixed_bytes_20 address) {
  struct zAccount account = optimized_empty_account();
  if (optimized_account_row(&account, 0, address))
    return account;

  bal_note_account_touch(address);
  if (optimized_account_row(&account, 1, address))
    return account;

  sail_fixed_bytes_32 address_hash;
  struct zAccountInfo info;
  bool found = false;
  acct_secure_key(address, &address_hash);
  const uint64_t status = evmsail_stateless_account_read(
      parent_state_root, address_hash, &info, &found);
  if (status != 0) {
    evmsail_throw_invalid_block(optimized_witness_error(status),
                                "optimized k_aload");
    return account;
  }
  if (found) {
    account.zinfo = info;
    account.zpresent = true;
    account.zstorage_cleared = false;
  }
  acct_block_cache_raw(
      address, address_hash, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zpresent,
      account.zstorage_cleared);
  return account;
}

struct zStorageValue evmsail_k_sload(sail_fixed_bytes_32 parent_state_root,
                                     sail_fixed_bytes_20 address, sail_u256 slot) {
  struct zStorageValue value;
  memset(&value, 0, sizeof(value));

  const uint64_t tx_status =
      storage_row_probe(0, address, slot, &value.zcurr, &value.zorig);
  if (tx_status == 1) {
    return value;
  }

  bal_note_storage_read(address, slot);
  if (tx_status == 2)
    return value;

  if (storage_row_probe(1, address, slot, &value.zcurr, &value.zorig)) {
    value.zorig = value.zcurr;
    return value;
  }

  const struct zAccount account = evmsail_k_aload(parent_state_root, address);
  if (have_exception) return value;
  sail_fixed_bytes_32 slot_hash;
  storage_secure_key(slot, &slot_hash);
  if (!account.zstorage_cleared) {
    const uint64_t status = evmsail_stateless_storage_read(
        account.zinfo.zstorage_root, slot_hash, &value.zcurr);
    if (status != 0) {
      evmsail_throw_invalid_block(optimized_witness_error(status),
                                  "optimized k_sload");
      return value;
    }
  }
  value.zorig = value.zcurr;
  storage_block_cache_raw(address, slot, slot_hash, value.zcurr);
  return value;
}

unit evmsail_k_sstore(sail_fixed_bytes_20 address, sail_u256 slot,
                      sail_u256 current, sail_u256 original) {
  return storage_tx_update_raw(address, slot, current, original);
}

unit evmsail_store_account(sail_fixed_bytes_20 address, struct zAccount account) {
  return acct_tx_update_raw(
      address, account.zinfo.znonce, account.zinfo.zbalance,
      account.zinfo.zstorage_root, account.zinfo.zcode_hash, account.zpresent,
      account.zstorage_cleared, account.zcreated, account.zselfdestructed);
}

unit evmsail_store_account_info(sail_fixed_bytes_20 address,
                                struct zAccount account,
                                struct zAccountInfo info) {
  const bool empty = optimized_account_info_empty(&info);
  if (empty) storage_tx_clear(address);

  struct zAccount next = account;
  if (empty) {
    memset(&next.zinfo, 0, sizeof(next.zinfo));
    next.zinfo.zstorage_root = account.zinfo.zstorage_root;
    memcpy(next.zinfo.zcode_hash.bytes, optimized_keccak_empty,
           sizeof(optimized_keccak_empty));
    next.zpresent = false;
    next.zstorage_cleared = true;
  } else {
    next.zinfo = info;
    next.zpresent = true;
  }

  if (!optimized_hash_equal(next.zinfo.zstorage_root,
                            account.zinfo.zstorage_root) ||
      next.zpresent != account.zpresent ||
      next.zstorage_cleared != account.zstorage_cleared) {
    return evmsail_store_account(address, next);
  }
  if (!optimized_word_equal(next.zinfo.zbalance, account.zinfo.zbalance))
    acct_tx_set_balance(address, next.zinfo.zbalance);
  if (next.zinfo.znonce != account.zinfo.znonce)
    acct_tx_set_nonce(address, next.zinfo.znonce);
  if (!optimized_hash_equal(next.zinfo.zcode_hash, account.zinfo.zcode_hash))
    acct_tx_set_code_hash(address, next.zinfo.zcode_hash);
  return UNIT;
}

/* Cold generated-aggregate adapters retained only by native diagnostics.
 * Production optimized generation eliminates both these calls and their
 * aggregate layouts. */
#ifdef EVMSAIL_NATIVE_DEBUG_AGGREGATES
void acct_block_get(struct zoptionzIRAccountzK *out,
                    const sail_fixed_bytes_20 address) {
  struct zAccount *account = &out->variants.zSomezIRAccountzK;
  if (optimized_account_row(account, 1, address)) {
    out->kind = Kind_zSomezIRAccountzK;
  } else {
    out->kind = Kind_zNonezIRAccountzK;
    out->variants.zNonezIRAccountzK = UNIT;
  }
}

void storage_block_iter_next(struct zoptionzIRStorageTrieEntryzK *out,
                             const sail_fixed_bytes_20 address) {
  struct zStorageTrieEntry *trie_entry =
      &out->variants.zSomezIRStorageTrieEntryzK;
  struct zStorageEntry *entry = &trie_entry->zentry;
  if (storage_block_iter_next_probe(
          address, &entry->zkey.zslot, &entry->zvalue.zcurr,
          &entry->zvalue.zorig, &trie_entry->zaddress_hash,
          &trie_entry->zslot_hash)) {
    evmsail_address_assign(&entry->zkey.zaddr, address);
    out->kind = Kind_zSomezIRStorageTrieEntryzK;
  } else {
    out->kind = Kind_zNonezIRStorageTrieEntryzK;
    out->variants.zNonezIRStorageTrieEntryzK = UNIT;
  }
}

void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *out, unit u) {
  (void)u;
  struct zAcctTrieEntry *trie_entry =
      &out->variants.zSomezIRAcctTrieEntryzK;
  struct zAcctEntry *entry = &trie_entry->zentry;
  struct zAccount *curr = &entry->zvalue.zcurr;
  struct zAccount *orig = &entry->zvalue.zorig;
  if (acct_block_iter_next_probe(
          &entry->zaddr, &curr->zinfo.znonce, &curr->zinfo.zbalance,
          &curr->zinfo.zstorage_root, &curr->zinfo.zcode_hash, &curr->zpresent,
          &curr->zstorage_cleared, &curr->zcreated, &curr->zselfdestructed,
          &orig->zinfo.znonce, &orig->zinfo.zbalance,
          &orig->zinfo.zstorage_root, &orig->zinfo.zcode_hash, &orig->zpresent,
          &orig->zstorage_cleared, &orig->zcreated, &orig->zselfdestructed,
          &trie_entry->zaddress_hash)) {
    out->kind = Kind_zSomezIRAcctTrieEntryzK;
  } else {
    out->kind = Kind_zNonezIRAcctTrieEntryzK;
    out->variants.zNonezIRAcctTrieEntryzK = UNIT;
  }
}
#endif

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
    struct zBalStorageChangeEntry *entry = &out->variants.zBalStorageChange;
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
    struct zBalBalanceChangeEntry *entry = &out->variants.zBalBalanceChange;
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
