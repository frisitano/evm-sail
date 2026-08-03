/* Generated aggregate adapters for bindings declared by sail/host/state.sail.
 *
 * The production optimized guest specializes these cold aggregate calls away;
 * they remain available only to native diagnostics. Mutable state storage and
 * transaction-log machinery live in the private state_store/state_runtime
 * backends rather than masquerading as Sail module owners. */
#include "evmsail/prelude.h"

#include "evmsail/host/state.h"
#include "host/state/internal.h"
#include "host/state/store.h"
#include "primitives/value.h"

#include <stdint.h>

#ifdef EVMSAIL_NATIVE_DEBUG_AGGREGATES
void acct_block_get(struct zoptionzIRAccountzK *out,
                    const Address address) {
  struct zAccount *account = &out->variants.zSomezIRAccountzK;
  AccountView view;
  if (account_block_view(&address, &view)) {
    account->zinfo.znonce = view.value.nonce;
    account->zinfo.zbalance = view.value.balance;
    account->zinfo.zstorage_root = view.storage_root;
    account->zinfo.zcode_hash = view.value.code_hash;
    account->zpresent = view.value.exists;
    account->zstorage_cleared = view.storage_cleared;
    account->zcreated = view.created;
    account->zselfdestructed = view.selfdestructed;
    out->kind = Kind_zSomezIRAccountzK;
  } else {
    out->kind = Kind_zNonezIRAccountzK;
    out->variants.zNonezIRAccountzK = UNIT;
  }
}
#endif

void storage_block_iter_next(struct zoptionzIRStorageTrieEntryzK *out,
                             const Address address) {
  struct zStorageTrieEntry *trie_entry =
      &out->variants.zSomezIRStorageTrieEntryzK;
  struct zStorageEntry *entry = &trie_entry->zentry;
  if (storage_block_iter_next_probe(
          address, &entry->zkey.zslot, &entry->zvalue.zcurr,
          &entry->zvalue.zorig, &trie_entry->zaddress_hash,
          &trie_entry->zslot_hash)) {
    address_assign(&entry->zkey.zaddr, address);
    out->kind = Kind_zSomezIRStorageTrieEntryzK;
  } else {
    out->kind = Kind_zNonezIRStorageTrieEntryzK;
    out->variants.zNonezIRStorageTrieEntryzK = UNIT;
  }
}

void acct_block_iter_next(struct zoptionzIRAcctTrieEntryzK *out, unit ignored) {
  (void)ignored;
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
