#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void zmain(void)
{
  scratch_reset();
  Bytes input_bytes = stateless_input();
  struct StatelessInputRef input_ref = decode_stateless_input_ref_(input_bytes);
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
  k_header.base_fee = ZERO_WORD;
  k_header.blob_gas_used = (uint32_t)UINT8_C(0);
  k_header.excess_blob_gas = (uint32_t)UINT8_C(0);
  k_header.extra_data = EMPTY_STATELESS_INPUT_SLICE;
  k_header.fee_recipient = ZERO_ADDRESS;
  k_header.gas_limit = (uint64_t)UINT8_C(0);
  k_header.gas_used = (uint64_t)UINT8_C(0);
  k_header.logs_bloom.bytes = NULL;
  k_header.logs_bloom.len = (uint32_t)UINT16_C(256);
  k_header.number = (uint64_t)UINT8_C(0);
  k_header.parent_beacon_block_root = ZERO_HASH;
  k_header.parent_hash = ZERO_HASH;
  k_header.prev_randao = ZERO_WORD;
  k_header.receipts_root = ZERO_HASH;
  k_header.slot_number = (uint64_t)UINT8_C(0);
  k_header.state_root = ZERO_HASH;
  k_header.timestamp = (uint64_t)UINT8_C(0);
  k_tx.blob_hashes = EMPTY_BLOB_HASHES;
  k_tx.gas_price = ZERO_WORD;
  k_tx.origin = ZERO_ADDRESS;
  k_current_transaction_epoch = (uint32_t)UINT8_C(0);
  initialize_registers();
}

