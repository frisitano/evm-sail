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
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  return next_excess_blob_gas(execution_profile.protocol, witness.parent_excess_blob_gas, witness.parent_blob_gas_used, witness.parent_base_fee_per_gas);
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
  if (header.gas_limit < header.gas_used) {
    fatal_error(InvalidGasUsed);
  }
  bool result_2_128 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(witness.parent_hash, header.parent_hash);
  if (result_2_128) {
    fatal_error(InvalidParentHash);
  }
  uint32_t expected_excess_blob_gas = expected_payload_excess_blob_gas(witness);
  bool result_2_131 = (bool)(profile.fork >= Cancun);
  bool tmp_3_627 = (bool)(result_2_131 && (header.excess_blob_gas != expected_excess_blob_gas));
  if (tmp_3_627) {
    fatal_error(InvalidExcessBlobGas);
  }
  bool result_2_134 = (bool)(profile.fork >= Paris);
  if (result_2_134) {
    bytes32 transactions_root = transaction_trie_root_(block.body.transactions);
    bytes32 withdrawals_root;
    bool result_2_143 = (bool)(profile.fork >= Shanghai);
    if (result_2_143) {
      withdrawals_root = withdrawals_trie_root_(block.body.withdrawals);
    } else {
      withdrawals_root = EMPTY_TRIE_ROOT;
    }
    bytes32 requests_hash;
    bool result_2_141 = (bool)(profile.fork >= Prague);
    if (result_2_141) {
      requests_hash = execution_requests_hash_(input_ref);
    } else {
      requests_hash = ZERO_HASH;
    }
    bytes32 block_access_list_hash;
    bool result_2_138 = (bool)(profile.fork >= Amsterdam);
    if (result_2_138) {
      block_access_list_hash = host_keccak_stateless_input(block.body.block_access_list);
    } else {
      block_access_list_hash = ZERO_HASH;
    }
    bytes32 computed_block_hash = block_header_hash_(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
    bool result_2_136 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(computed_block_hash, payload.expected_block_hash);
    if (result_2_136) {
      fatal_error(InvalidBlockHash);
    }
    return;
  }
}

