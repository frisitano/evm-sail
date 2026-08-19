#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct TrieUpdate account_update(struct AcctTrieEntry trie_entry, bytes32 storage_root)
{
  struct Account current = trie_entry.entry.value.curr;
  struct TriePath key = path_new(trie_entry.address_hash, UINT8_C(64));
  bool account_empty = account_info_empty(current.info);
  if (!current.present || account_empty) {
    return ((struct TrieUpdate){.change = (TrieDelete(UNIT)), .key = key});
  }
  Bytes encoded_account = encode_state_account(current.info, storage_root);
  return ((struct TrieUpdate){.change = (TriePut(encoded_account)), .key = key});
}

bool account_value_changed(struct AcctValue value)
{
  if ((value.curr.info.nonce == value.orig.info.nonce) && eq_u256(value.curr.info.balance, value.orig.info.balance)) {
    if (eq_bytes32(value.curr.info.storage_root, value.orig.info.storage_root) && eq_bytes32(value.curr.info.code_hash, value.orig.info.code_hash)) {
      return (bool)((value.curr.present != value.orig.present) || (value.curr.storage_cleared != value.orig.storage_cleared));
    }
    return true;
  }
  return true;
}

struct TrieUpdate storage_update(struct StorageTrieEntry trie_entry)
{
  struct TriePath key = path_new(trie_entry.slot_hash, UINT8_C(64));
  struct TrieChange change;
  if (eq_u256(trie_entry.entry.value.curr, WORD_ZERO)) {
    change = TrieDelete(UNIT);
  } else {
    Bytes encoded_value = encode_storage_value(trie_entry.entry.value.curr);
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
      if (storage_value_changed(trie_entry.entry.value)) {
        result = ((struct TrieUpdateFetch){.available = true, .update = (storage_update(trie_entry))});
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

