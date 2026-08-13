#pragma once

#include "evmsail/spec/primitives/system.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct StorageValue
struct StorageValue {
  u256 curr;
  u256 orig;
};

// struct StorageKey
struct StorageKey {
  bytes20 addr;
  u256 slot;
};

// struct StorageEntry
struct StorageEntry {
  struct StorageKey key;
  struct StorageValue value;
};

// struct StorageTrieEntry
struct StorageTrieEntry {
  bytes32 address_hash;
  struct StorageEntry entry;
  bytes32 slot_hash;
};

// union StorageBlockIterResult
enum kind_StorageBlockIterResult { Kind_StorageBlockIterExhausted, Kind_StorageBlockIterRow };

struct StorageBlockIterResult {
  enum kind_StorageBlockIterResult kind;
  union {
    struct { unit StorageBlockIterExhausted; };
    struct { struct StorageTrieEntry StorageBlockIterRow; };
  } variants;
};

// struct AccountInfo
struct AccountInfo {
  u256 balance;
  bytes32 code_hash;
  uint64_t nonce;
  bytes32 storage_root;
};

// struct Account
struct Account {
  bool created;
  struct AccountInfo info;
  bool present;
  bool selfdestructed;
  bool storage_cleared;
};

// struct AcctValue
struct AcctValue {
  struct Account curr;
  struct Account orig;
};

// struct AcctEntry
struct AcctEntry {
  bytes20 addr;
  struct AcctValue value;
};

// struct AcctTrieEntry
struct AcctTrieEntry {
  bytes32 address_hash;
  struct AcctEntry entry;
};

// union AcctBlockIterResult
enum kind_AcctBlockIterResult { Kind_AcctBlockIterExhausted, Kind_AcctBlockIterRow };

struct AcctBlockIterResult {
  enum kind_AcctBlockIterResult kind;
  union {
    struct { unit AcctBlockIterExhausted; };
    struct { struct AcctTrieEntry AcctBlockIterRow; };
  } variants;
};

struct Account k_aload(bytes32 /* arg_0 */, bytes20 /* arg_1 */);

void store_account(bytes20 /* arg_0 */, struct Account /* arg_1 */);

void store_account_info(bytes20 /* arg_0 */, struct Account /* arg_1 */, struct AccountInfo /* arg_2 */);

struct StorageValue storage_load_by_id(uint32_t /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */, bool /* arg_3 */);


#ifdef __cplusplus
}
#endif
