#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes32 transaction_trie_root_(struct BoundedSszListRef txs)
{
  return transaction_trie_root(txs);
}

bytes32 withdrawals_trie_root_(struct BoundedSszListRef wds)
{
  return withdrawals_trie_root(wds);
}

uint32_t expected_payload_excess_blob_gas(struct WitnessContext witness)
{
  return next_excess_blob_gas((k_execution_profile).protocol, witness.parent_excess_blob_gas, witness.parent_blob_gas_used, witness.parent_base_fee_per_gas);
}

u320 expected_payload_base_fee(struct WitnessContext witness)
{
  if (!witness.parent_has_base_fee) {
    return u320_of_u64(UINT32_C(1000000000));
  }
  uint64_t target;
  if (UINT8_C(0) < (witness.parent_gas_limit >> 1)) {
    target = (witness.parent_gas_limit >> 1);
  } else {
    fatal_error(InvalidGasLimit);
  }
  uint64_t used = witness.parent_gas_used;
  if ((target == UINT8_C(0)) || (witness.parent_gas_limit < used)) {
    fatal_error(InvalidGasLimit);
  }
  if (used == target) {
    return u320_of_u256(witness.parent_base_fee_per_gas);
  }
  if (target < used) {
    u320 result_2_208 = u320_lt(u320_div_u64(u320_div_u64(u320_mul_widen(witness.parent_base_fee_per_gas, (used - target)), target), UINT8_C(8)), u320_of_u64(UINT8_C(1))) ? u320_of_u64(UINT8_C(1)) : u320_div_u64(u320_div_u64(u320_mul_widen(witness.parent_base_fee_per_gas, (used - target)), target), UINT8_C(8));
    return u320_add_widen(witness.parent_base_fee_per_gas, result_2_208);
  }
  u256 tmp_3_666 = u256_of_u320(u320_div_u64(u320_mul_widen(witness.parent_base_fee_per_gas, (target - used)), target));
  u320 integer_operand_3_3938 = u320_of_u256(witness.parent_base_fee_per_gas);
  u320 integer_operand_3_3939 = u320_of_u256(u256_div_u64(tmp_3_666, UINT8_C(8)));
  return u320_sub(integer_operand_3_3938, integer_operand_3_3939);
}

void validate_block_rlp_size(struct BlockHeader header, struct BoundedSszListRef transactions, struct BoundedSszListRef withdrawals)
{
  uint32_t tx_bytes = UINT32_C(0);
  struct BoundedSszListCursor cursor = ssz_list_cursor(transactions);
  while (true) {
    bool tmp_3_647;
    bool ssz_list_cursor_empty_result_2_182 = ssz_list_cursor_empty(cursor);
    tmp_3_647 = (bool)(!ssz_list_cursor_empty_result_2_182);
    if (!tmp_3_647) {
      break;
    }
    Bytes _8_1560_8_1817;
    struct BoundedSszListCursor _8_1561_8_1818 = ssz_list_pop(cursor, &_8_1560_8_1817);
    uint32_t size;
    if ((_8_1560_8_1817.len != UINT8_C(0)) && (_8_1560_8_1817.len <= UINT32_C(1073741824))) {
      if ((uint8_t)(stateless_input_slice_byte_Bytes_uint8_t_to_uint64_t(_8_1560_8_1817, UINT8_C(0))) >= UINT8_C(192)) {
        size = _8_1560_8_1817.len;
      } else {
        size = rlp_input_slice_size_Bytes_to_uint32_t(_8_1560_8_1817);
      }
    } else {
      fatal_error(RlpDecode);
    }
    uint32_t total = (uint32_t)((uint64_t)tx_bytes + (uint64_t)size);
    if (total <= UINT32_C(8388608)) {
      tx_bytes = total;
    } else {
      fatal_error(ExecutionInvalid);
    }
    cursor = _8_1561_8_1818;
  }
  uint32_t withdrawal_bytes = UINT32_C(0);
  struct BoundedSszListRef rest = withdrawals;
  while (rest.count != UINT8_C(0)) {
    Bytes _8_1612_8_1819;
    struct BoundedSszListRef _8_1613_8_1820 = ssz_fixed_list_pop(rest, WD_SIZE, &_8_1612_8_1819);
    uint32_t mark = scratch_begin();
    uint64_t total_3_661 = ((uint64_t)withdrawal_bytes + (uint64_t)(withdrawal_rlp(_8_1612_8_1819)).len);
    if (total_3_661 <= UINT32_C(8388608)) {
      withdrawal_bytes = (uint32_t)total_3_661;
    } else {
      fatal_error(ExecutionInvalid);
    }
    scratch_rewind(mark);
    rest = _8_1613_8_1820;
  }
  uint16_t _8_1568_8_1821;
  uint16_t _8_1569_8_1822 = block_header_rlp_lengths(header, &_8_1568_8_1821);
  uint32_t transactions_size = rlp_list_size_uint32_t_to_uint32_t(tx_bytes);
  uint32_t withdrawals_size = rlp_list_size_uint32_t_to_uint32_t(withdrawal_bytes);
  if (UINT32_C(8388608) < (rlp_list_size_uint32_t_to_uint32_t_variant_2((uint32_t)((uint64_t)(uint32_t)((uint64_t)(uint32_t)((uint64_t)_8_1569_8_1822 + (uint64_t)transactions_size) + UINT64_C(1)) + (uint64_t)withdrawals_size)))) {
    fatal_error(ExecutionInvalid);
  }
}

bytes32 execution_requests_hash_(struct StatelessInputRef input_ref)
{
  return execution_requests_hash(input_ref);
}

void validate_execution_payload(struct StatelessInput input, struct StatelessInputRef input_ref, struct WitnessContext witness)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct ExecutionPayload payload = input.payload;
  struct Block block = payload.block;
  struct BlockHeader header = block.header;
  struct BlockBody body = block.body;
  u128 parent_limit = (profile.fork == London) && !witness.parent_has_base_fee ? u128_mul_u64_u64(UINT8_C(2), witness.parent_gas_limit) : u128_of_u64(witness.parent_gas_limit);
  __int128 limit_delta = u64_lt_u128(header.gas_limit, parent_limit) ? ((__int128)u128_to_u64_unchecked(parent_limit) - (__int128)header.gas_limit) : ((__int128)header.gas_limit - (__int128)u128_to_u64_unchecked(parent_limit));
  if ((header.gas_limit < UINT16_C(5000)) || ((UINT64_C(9223372036854775808) <= header.gas_limit) || (u128_lt_u64(parent_limit, UINT16_C(5000)) || (!(limit_delta < (uint64_t)u128_to_u64_unchecked(u128_div_u64(parent_limit, UINT16_C(1024)))))))) {
    fatal_error(InvalidGasLimit);
  }
  if (profile.fork >= Osaka) {
    validate_block_rlp_size(header, body.transactions, body.withdrawals);
  }
  if (header.gas_limit < header.gas_used) {
    fatal_error(InvalidGasUsed);
  }
  if (neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(witness.parent_hash, header.parent_hash)) {
    fatal_error(InvalidParentHash);
  }
  bool tmp_3_635;
  if (profile.fork >= London) {
    tmp_3_635 = (bool)(!eq_u320(u320_of_u256(header.base_fee), (expected_payload_base_fee(witness))));
  } else {
    tmp_3_635 = false;
  }
  if (tmp_3_635) {
    fatal_error(ExecutionInvalid);
  }
  if (profile.fork >= Cancun) {
    if (header.excess_blob_gas != (expected_payload_excess_blob_gas(witness))) {
      fatal_error(InvalidExcessBlobGas);
    }
  }
  bytes32 transactions_root = transaction_trie_root_(body.transactions);
  bytes32 withdrawals_root;
  if (profile.fork >= Shanghai) {
    withdrawals_root = withdrawals_trie_root_(body.withdrawals);
  } else {
    withdrawals_root = EMPTY_TRIE_ROOT;
  }
  bytes32 requests_hash;
  if (profile.fork >= Prague) {
    requests_hash = execution_requests_hash_(input_ref);
  } else {
    requests_hash = ZERO_HASH;
  }
  bytes32 block_access_list_hash;
  if (profile.fork >= Amsterdam) {
    block_access_list_hash = host_keccak_stateless_input(body.block_access_list);
  } else {
    block_access_list_hash = ZERO_HASH;
  }
  bytes32 computed_block_hash = block_header_hash_(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
  if (neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(computed_block_hash, payload.expected_block_hash)) {
    fatal_error(InvalidBlockHash);
  }
}

