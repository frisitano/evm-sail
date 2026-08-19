#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct TrieUpdate account_update(struct AcctTrieEntry trie_entry, bytes32 storage_root)
{
  struct Account current = trie_entry.entry.value.curr;
  struct TriePath key = path_new(trie_entry.address_hash, UINT8_C(64));
  bool account_empty = account_info_empty(current.info);
  if (!current.present || account_empty) {
    struct TrieChange TrieDelete_result_2_525 = TrieDelete(UNIT);
    return ((struct TrieUpdate){.change = TrieDelete_result_2_525, .key = key});
  }
  Bytes encoded_account = encode_state_account(current.info, storage_root);
  struct TrieChange TriePut_result_2_526 = TriePut(encoded_account);
  return ((struct TrieUpdate){.change = TriePut_result_2_526, .key = key});
}

bool account_value_changed(struct AcctValue value)
{
  if ((value.curr.info.nonce == value.orig.info.nonce) && eq_u256(value.curr.info.balance, value.orig.info.balance)) {
    if (eq_bytes32(value.curr.info.storage_root, value.orig.info.storage_root) && eq_bytes32(value.curr.info.code_hash, value.orig.info.code_hash)) {
      if (value.curr.present == value.orig.present) {
        return (bool)(value.curr.storage_cleared != value.orig.storage_cleared);
      }
      return true;
    }
    return true;
  }
  return true;
}

struct TrieUpdate storage_update(struct StorageTrieEntry trie_entry)
{
  struct StorageEntry entry = trie_entry.entry;
  struct TriePath key = path_new(trie_entry.slot_hash, UINT8_C(64));
  bool value_is_zero = eq_u256(entry.value.curr, WORD_ZERO);
  struct TrieChange change;
  if (value_is_zero) {
    change = TrieDelete(UNIT);
  } else {
    Bytes encoded_value = encode_storage_value(entry.value.curr);
    change = TriePut(encoded_value);
  }
  return ((struct TrieUpdate){.change = change, .key = key});
}

bool storage_value_changed(struct StorageValue value)
{
  return (bool)(!eq_u256(value.curr, value.orig));
}

struct TrieUpdateFetch next_storage_trie_update(bytes20 addr)
{
  bool searching = true;
  struct TrieUpdateFetch result = ((struct TrieUpdateFetch){.available = false, .update = EMPTY_TRIE_UPDATE});
  while (searching) {
    struct StorageBlockIterResult iterator_item = storage_block_iter_next(addr);
    switch (iterator_item.kind) {
    case Kind_StorageBlockIterRow:
    {
      struct StorageTrieEntry trie_entry = iterator_item.variants.StorageBlockIterRow;
      bool changed = storage_value_changed(trie_entry.entry.value);
      if (changed) {
        struct TrieUpdate storage_update_result_2_428 = storage_update(trie_entry);
        result = ((struct TrieUpdateFetch){.available = true, .update = storage_update_result_2_428});
        searching = false;
      }
      break;
    }
    case Kind_StorageBlockIterExhausted:
      searching = false;
      break;
    }
  }
  return result;
}

