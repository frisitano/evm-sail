#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void validate_executed_block(struct Block block, struct BlockExecutionResult result)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct BlockHeader header = block.header;
  if (result.header_gas_used != header.gas_used) {
    fatal_error(InvalidGasUsed);
  }
  bool result_2_108 = (bool)(profile.fork >= Cancun);
  if (result_2_108 && (result.blob_gas_used != header.blob_gas_used)) {
    fatal_error(InvalidBlobGasUsed);
  }
  bytes32 poststate = compute_state_root();
  bool result_2_111 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(poststate, header.state_root);
  if (result_2_111) {
    fatal_error(InvalidStateRoot);
  }
  bool result_2_114 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(result.receipts_root, header.receipts_root);
  if (result_2_114) {
    fatal_error(InvalidReceiptsRoot);
  }
  bool logs_bloom_matches = block_logs_bloom_matches_(header.logs_bloom);
  if (!logs_bloom_matches) {
    fatal_error(InvalidLogsBloom);
  }
  if (profile.fork >= Amsterdam) {
    validate_block_access_list_(block.body.block_access_list, execution_profile.gas.block_limit);
    return;
  }
}

void verify_stateless_payload(struct StatelessInputRef input_ref)
{
  scratch_reset();
  struct StatelessInput input = decode_stateless_input(input_ref);
  struct WitnessContext witness = index_execution_witness(input_ref);
  validate_execution_payload(input, input_ref, witness);
  struct Block block = input.payload.block;
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork >= Amsterdam) {
    initialize_block_access_list_state_(block.body.block_access_list, k_parent_state_root, block.body.transactions.count);
  }
  struct BlockExecutionResult result = execute_block_body(block.body, input_ref);
  validate_executed_block(block, result);
}

