#pragma once

#include "evmsail/spec/primitives/tx.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct Withdrawal
struct Withdrawal {
  bytes20 address;
  uint64_t amount;
  uint64_t index;
  uint64_t validator_index;
};

#ifndef SAIL_FIXED_BYTES_256_DEFINED
#define SAIL_FIXED_BYTES_256_DEFINED
typedef struct { uint8_t bytes[256]; } bytes256;
#endif

// struct BlockHeader
struct BlockHeader {
  u256 base_fee;
  uint32_t blob_gas_used;
  uint32_t excess_blob_gas;
  Bytes extra_data;
  bytes20 fee_recipient;
  uint64_t gas_limit;
  uint64_t gas_used;
  Bytes logs_bloom;
  uint64_t number;
  bytes32 parent_beacon_block_root;
  bytes32 parent_hash;
  u256 prev_randao;
  bytes32 receipts_root;
  uint64_t slot_number;
  bytes32 state_root;
  uint64_t timestamp;
};

// struct BlockBody
struct BlockBody {
  Bytes block_access_list;
  struct BoundedSszListRef transactions;
  struct BoundedSszListRef withdrawals;
};

// struct Block
struct Block {
  struct BlockBody body;
  struct BlockHeader header;
};

// struct ExecutionPayload
struct ExecutionPayload {
  struct Block block;
  bytes32 expected_block_hash;
};

struct Withdrawal decode_withdrawal(Bytes /* arg_0 */);

bytes32 block_header_hash(struct BlockHeader /* arg_0 */, bytes32 /* arg_1 */, bytes32 /* arg_2 */, bytes32 /* arg_3 */, bytes32 /* arg_4 */);


#ifdef __cplusplus
}
#endif
