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
  create_letbind_26();
  create_letbind_27();
  create_letbind_28();
  create_letbind_29();
  create_letbind_31();
  create_letbind_32();
  create_letbind_34();
  create_letbind_35();
  create_letbind_57();
  create_letbind_58();
  create_letbind_73();
  create_letbind_208();
  create_letbind_276();
  scratch_arena = EMPTY_SCRATCH_SLICE;
  k_parent_state_root = ZERO_HASH;
  k_n_headers = (uint16_t)UINT8_C(0);
  k_chain_id = (uint64_t)UINT8_C(1);
  k_execution_profile = DEFAULT_EXECUTION_PROFILE;
  struct BlockHeader tmp_3_121;
  tmp_3_121.base_fee = ZERO_WORD;
  tmp_3_121.blob_gas_used = (uint32_t)UINT8_C(0);
  tmp_3_121.excess_blob_gas = (uint32_t)UINT8_C(0);
  tmp_3_121.extra_data = EMPTY_STATELESS_INPUT_SLICE;
  tmp_3_121.fee_recipient = ZERO_ADDRESS;
  tmp_3_121.gas_limit = (uint64_t)UINT8_C(0);
  tmp_3_121.gas_used = (uint64_t)UINT8_C(0);
  Bytes tmp_3_122;
  tmp_3_122.bytes = NULL;
  tmp_3_122.len = (uint32_t)UINT16_C(256);
  tmp_3_121.logs_bloom = tmp_3_122;
  tmp_3_121.number = (uint64_t)UINT8_C(0);
  tmp_3_121.parent_beacon_block_root = ZERO_HASH;
  tmp_3_121.parent_hash = ZERO_HASH;
  tmp_3_121.prev_randao = ZERO_WORD;
  tmp_3_121.receipts_root = ZERO_HASH;
  tmp_3_121.slot_number = (uint64_t)UINT8_C(0);
  tmp_3_121.state_root = ZERO_HASH;
  tmp_3_121.timestamp = (uint64_t)UINT8_C(0);
  k_header = tmp_3_121;
  struct TxEnvFields tmp_3_123;
  tmp_3_123.blob_hashes = EMPTY_BLOB_HASHES;
  tmp_3_123.gas_price = ZERO_WORD;
  tmp_3_123.origin = ZERO_ADDRESS;
  k_tx = tmp_3_123;
  k_current_transaction_epoch = (uint32_t)UINT8_C(0);
  initialize_registers();
}

