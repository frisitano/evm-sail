#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void zmain(void)
{
  scratch_reset();
  struct StatelessInputRef input_ref;
  Bytes stateless_input_result_2_83 = stateless_input();
  input_ref = decode_stateless_input_ref_(stateless_input_result_2_83);
  fatal_error_set_input(input_ref);
  verify_stateless_payload(input_ref);
  write_validation_result(input_ref, true);
}

void evmsail_model_init(void)
{
  create_letbind_7();
  create_letbind_8();
  create_letbind_11();
  create_letbind_25();
  create_letbind_26();
  create_letbind_27();
  create_letbind_28();
  create_letbind_30();
  create_letbind_31();
  create_letbind_33();
  create_letbind_34();
  create_letbind_56();
  create_letbind_57();
  create_letbind_72();
  create_letbind_77();
  create_letbind_205();
  create_letbind_273();
  scratch_arena = EMPTY_SCRATCH_SLICE;
  k_parent_state_root = ZERO_HASH;
  k_n_headers = (uint16_t)UINT8_C(0);
  k_chain_id = (uint64_t)UINT8_C(1);
  k_execution_profile = DEFAULT_EXECUTION_PROFILE;
  struct BlockHeader tmp_3_120;
  tmp_3_120.base_fee = ZERO_WORD;
  tmp_3_120.blob_gas_used = (uint32_t)UINT8_C(0);
  tmp_3_120.excess_blob_gas = (uint32_t)UINT8_C(0);
  tmp_3_120.extra_data = EMPTY_STATELESS_INPUT_SLICE;
  tmp_3_120.fee_recipient = ZERO_ADDRESS;
  tmp_3_120.gas_limit = (uint64_t)UINT8_C(0);
  tmp_3_120.gas_used = (uint64_t)UINT8_C(0);
  Bytes tmp_3_121;
  tmp_3_121.bytes = NULL;
  tmp_3_121.len = (uint32_t)UINT16_C(256);
  tmp_3_120.logs_bloom = tmp_3_121;
  tmp_3_120.number = (uint64_t)UINT8_C(0);
  tmp_3_120.parent_beacon_block_root = ZERO_HASH;
  tmp_3_120.parent_hash = ZERO_HASH;
  tmp_3_120.prev_randao = ZERO_WORD;
  tmp_3_120.receipts_root = ZERO_HASH;
  tmp_3_120.slot_number = (uint64_t)UINT8_C(0);
  tmp_3_120.state_root = ZERO_HASH;
  tmp_3_120.timestamp = (uint64_t)UINT8_C(0);
  k_header = tmp_3_120;
  struct TxEnvFields tmp_3_122;
  tmp_3_122.blob_hashes = EMPTY_BLOB_HASHES;
  tmp_3_122.gas_price = ZERO_WORD;
  tmp_3_122.origin = ZERO_ADDRESS;
  k_tx = tmp_3_122;
  k_current_transaction_epoch = (uint32_t)UINT8_C(0);
  pc = (uint32_t)UINT8_C(0);
  gas_remaining = GAS_ZERO;
  stack_top = UINT64_C(0x0000000000000000);
  state_gas_remaining = GAS_ZERO;
  state_gas_spilled = STATE_GAS_SPILL_ZERO;
  frame_refund = GAS_REFUND_ZERO;
  frame_status = Running(UNIT);
  message = DEFAULT_MESSAGE;
  call_depth = (uint16_t)UINT8_C(0);
  frame_code = EMPTY_CODE;
  calldata = EMPTY_CALLDATA;
  returndata = EMPTY_OUTPUT_SLICE;
  evm_memory = EMPTY_EVM_MEMORY_SLICE;
  initialize_registers();
}

