#pragma once

#include "evmsail/spec/evm/halt.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct BoundedSszListRef
struct BoundedSszListRef {
  Bytes bytes;
  uint32_t count;
  uint32_t max_item_length;
};

// struct BoundedSszListCursor
struct BoundedSszListCursor {
  uint32_t current;
  uint32_t index;
  struct BoundedSszListRef items;
};

// struct tuple_Bytes_BoundedSszListRef
struct tuple_Bytes_BoundedSszListRef {
  Bytes tup0;
  struct BoundedSszListRef tup1;
};

// struct tuple_Bytes_BoundedSszListCursor
struct tuple_Bytes_BoundedSszListCursor {
  Bytes tup0;
  struct BoundedSszListCursor tup1;
};

void index_witness_nodes(struct BoundedSszListRef /* arg_0 */);

void index_witness_codes(struct BoundedSszListRef /* arg_0 */, bool /* arg_1 */);

bytes32 transaction_trie_root(struct BoundedSszListRef /* arg_0 */);

bytes32 withdrawals_trie_root(struct BoundedSszListRef /* arg_0 */);


#ifdef __cplusplus
}
#endif
