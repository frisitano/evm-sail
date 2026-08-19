#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

// register zk_parent_state_root
bytes32 k_parent_state_root;

// register zk_n_headers
uint16_t k_n_headers;

// register zk_chain_id
uint64_t k_chain_id;

// register zk_execution_profile
struct ExecutionProfileFields k_execution_profile;

// register zk_header
struct BlockHeader k_header;

// register zk_tx
struct TxEnvFields k_tx;

// register zk_current_transaction_epoch
uint32_t k_current_transaction_epoch;

u256 k_env(enum EnvField f)
{
  struct TxEnvFields active_tx = k_tx;
  switch (f) {
  case F_Number:
  {
    uint64_t number = word_of_block_number((k_header.number));
    return u256_uint64_t_to_u256(number);
  }
  case F_Timestamp:
  {
    uint64_t timestamp = word_of_block_timestamp((k_header.timestamp));
    return u256_uint64_t_to_u256(timestamp);
  }
  case F_Coinbase:
  {
    return address_to_word((k_header.fee_recipient));
  }
  case F_BaseFee:
    return k_header.base_fee;
  case F_ChainId:
  {
    return u256_uint64_t_to_u256((word_of_chain_identifier(k_chain_id)));
  }
  case F_GasLimit:
  {
    return u256_uint64_t_to_u256((k_header.gas_limit));
  }
  case F_PrevRandao:
    return k_header.prev_randao;
  case F_Origin:
    return address_to_word(active_tx.origin);
  case F_GasPrice:
    return active_tx.gas_price;
  case F_SlotNumber:
  {
    uint64_t slot_number = word_of_slot_number((k_header.slot_number));
    return u256_uint64_t_to_u256(slot_number);
  }
  }
}

bytes20 k_coinbase(void)
{
  return k_header.fee_recipient;
}

u256 blockhash_word_distance(u256 current, u256 number)
{
  return u256_sub(current, number);
}

bytes32 k_blockhash(u256 number_word)
{
  uint64_t current = k_header.number;
  uint64_t current_number = word_of_block_number(current);
  u256 current_word = u256_uint64_t_to_u256(current_number);
  if (u256_lt(number_word, current_word)) {
    u256 distance_word = blockhash_word_distance(current_word, number_word);
    if (!u64_lt_u256(UINT16_C(256), distance_word)) {
      uint16_t distance = (uint16_t)u256_to_u64_unchecked(distance_word);
      if (k_n_headers < distance) {
        fatal_error(WitnessDeficient);
      }
      return ancestor_hash_read((uint8_t)((uint16_t)((uint32_t)distance - (uint32_t)UINT16_C(1))));
    }
    return ZERO_HASH;
  }
  return ZERO_HASH;
}

u256 k_blobhash(u256 index_word)
{
  struct TxEnvFields active_tx = k_tx;
  if (u256_lt_u64(index_word, active_tx.blob_hashes.count)) {
    return stateless_input_slice_load_n(active_tx.blob_hashes.bytes, (((uint32_t)UINT16_C(33) * (uint32_t)(uint16_t)(uint8_t)u256_to_u64_unchecked(index_word)) + (uint32_t)UINT16_C(1)), WORD_BYTE_LENGTH);
  }
  return ZERO_WORD;
}

bytes20 k_create_addr(bytes20 a, uint64_t nonce)
{
  return create_address(a, nonce);
}

bytes20 k_create2_addr(bytes20 a, u256 salt, bytes32 inithash)
{
  return create2_address_(a, salt, inithash);
}

