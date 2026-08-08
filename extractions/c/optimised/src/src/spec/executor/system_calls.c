#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t SYSTEM_CALL_INPUT_LENGTH = UINT8_C(32);


const uint16_t DEPOSIT_EVENT_DATA_LENGTH = UINT16_C(576);


const uint8_t DEPOSIT_PUBKEY_HEAD = UINT8_C(0);


const uint8_t DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD = UINT8_C(32);


const uint8_t DEPOSIT_AMOUNT_HEAD = UINT8_C(64);


const uint8_t DEPOSIT_SIGNATURE_HEAD = UINT8_C(96);


const uint8_t DEPOSIT_INDEX_HEAD = UINT8_C(128);


const uint8_t DEPOSIT_PUBKEY_LENGTH_WORD = UINT8_C(160);


const uint8_t DEPOSIT_PUBKEY_DATA = UINT8_C(192);


const uint8_t DEPOSIT_PUBKEY_LENGTH = UINT8_C(48);


const uint16_t DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD = UINT16_C(256);


const uint16_t DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA = UINT16_C(288);


const uint8_t DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH = UINT8_C(32);


const uint16_t DEPOSIT_AMOUNT_LENGTH_WORD = UINT16_C(320);


const uint16_t DEPOSIT_AMOUNT_DATA = UINT16_C(352);


const uint8_t DEPOSIT_AMOUNT_LENGTH = UINT8_C(8);


const uint16_t DEPOSIT_SIGNATURE_LENGTH_WORD = UINT16_C(384);


const uint16_t DEPOSIT_SIGNATURE_DATA = UINT16_C(416);


const uint8_t DEPOSIT_SIGNATURE_LENGTH = UINT8_C(96);


const uint16_t DEPOSIT_INDEX_LENGTH_WORD = UINT16_C(512);


const uint16_t DEPOSIT_INDEX_DATA = UINT16_C(544);


const uint8_t DEPOSIT_INDEX_LENGTH = UINT8_C(8);


const uint8_t DEPOSIT_REQUEST_LENGTH = UINT8_C(192);


const uint8_t DEPOSIT_REQUEST_PUBKEY = UINT8_C(0);


const uint8_t DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS = UINT8_C(48);


const uint8_t DEPOSIT_REQUEST_AMOUNT = UINT8_C(80);


const uint8_t DEPOSIT_REQUEST_SIGNATURE = UINT8_C(88);


const uint8_t DEPOSIT_REQUEST_INDEX = UINT8_C(184);


void enter_system_call_frame(bytes20 tgt, struct CalldataSlice input)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct GasLimitsFields gas_limits = execution_profile.gas;
  k_journal_checkpoint();
  pc = UINT32_C(0);
  call_depth = UINT16_C(0);
  gas_remaining = (uint64_t)gas_limits.system_regular_limit;
  state_gas_remaining = (uint64_t)gas_limits.system_state_limit;
  state_gas_spilled = STATE_GAS_SPILL_ZERO;
  frame_refund = GAS_REFUND_ZERO;
  stack_top = stack_reset();
  returndata_clear();
  frame_status = Running(UNIT);
  calldata = input;
  message = ((struct Message){.address = tgt, .caller = SYSTEM_ADDRESS, .code_address = tgt, .depth = UINT16_C(0), .is_static = false, .state_gas_reservoir = (uint64_t)gas_limits.system_state_limit, .value = ZERO_WORD});
  bytes32 code_hash = k_code_key(tgt);
  frame_code = code_db_resolve(code_hash);
}

void system_call(bytes20 tgt, bytes32 input)
{
  bytes32 code_hash = k_code_key(tgt);
  bool eq_anything_result_2_266 = eq_bytes32(code_hash, KECCAK_EMPTY);
  if (eq_anything_result_2_266) {
    return;
  }
  memory_reset();
  struct MemoryRangeFields input_range = memory_range_uint8_t_uint8_t_to_struct_MemoryRangeFields(UINT8_C(0), SYSTEM_CALL_INPUT_LENGTH);
  struct tuple_Bytes_Bytes result_2_268 = memory_expand_to(evm_memory, input_range.len);
  evm_memory = result_2_268.tup1;
  u256 input_word = hash_to_word(input);
  mem_store_word(input_range.off, input_word);
  struct tuple_Bytes_Bytes result_2_272 = active_memory_slice(evm_memory, input_range.off, input_range.len);
  evm_memory = result_2_272.tup1;
  Bytes parent_memory = memory_frame_enter();
  Bytes memory_input = evm_memory_slice(result_2_272.tup0.bytes, result_2_272.tup0.len);
  struct CalldataSlice frame_input = MemoryCalldata(memory_input);
  enter_system_call_frame(tgt, frame_input);
  interpret_();
  memory_frame_leave(parent_memory);
  bool succeeded = frame_succeeded();
  if (succeeded) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  k_tx_merge();
}

Bytes system_call_checked(bytes20 tgt)
{
  bytes32 code_hash = k_code_key(tgt);
  bool eq_anything_result_2_264 = eq_bytes32(code_hash, KECCAK_EMPTY);
  if (eq_anything_result_2_264) {
    fatal_error(ExecutionInvalid);
  }
  memory_reset();
  Bytes parent_memory = memory_frame_enter();
  enter_system_call_frame(tgt, EMPTY_CALLDATA);
  Bytes output = interpret_();
  bool succeeded = frame_succeeded();
  if (succeeded) {
    uint32_t start = scratch_reserve(output.len);
    output_scratch_push_slice(output);
    Bytes result = scratch_finish(start);
    memory_frame_leave(parent_memory);
    k_journal_commit();
    k_tx_merge();
    return result;
  }
  memory_frame_leave(parent_memory);
  k_journal_revert();
  k_tx_merge();
  fatal_error(ExecutionInvalid);
}

bool deposit_log_matches(uint64_t index)
{
  bytes20 address = log_addr(index);
  bool neq_anything_result_2_262 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(address, DEPOSIT_CONTRACT_ADDR);
  if (neq_anything_result_2_262) {
    return false;
  }
  uint64_t topic_count = log_topic_count(index);
  if (topic_count == UINT8_C(0)) {
    return false;
  }
  u256 first_topic = log_topic(index, UINT64_C(0));
  return eq_u256(first_topic, DEPOSIT_EVENT_TOPIC);
}

Bytes authenticate_deposit_request(Bytes data, Bytes expected)
{
  Bytes data_1_1;
  bool result_2_261 = (bool)(data.len == DEPOSIT_EVENT_DATA_LENGTH);
  if (result_2_261) {
    data_1_1 = data;
  } else {
    fatal_error(InvalidExecutionRequests);
  }
  u256 pubkey_head = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_PUBKEY_HEAD);
  u256 withdrawal_credentials_head = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD);
  u256 amount_head = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_AMOUNT_HEAD);
  u256 signature_head = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_SIGNATURE_HEAD);
  u256 index_head = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_INDEX_HEAD);
  u256 pubkey_length = log_data_slice_load_Bytes_uint8_t_to_u256(data_1_1, DEPOSIT_PUBKEY_LENGTH_WORD);
  u256 withdrawal_credentials_length = log_data_slice_load_Bytes_uint16_t_to_u256(data_1_1, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD);
  u256 amount_length = log_data_slice_load_Bytes_uint16_t_to_u256(data_1_1, DEPOSIT_AMOUNT_LENGTH_WORD);
  u256 signature_length = log_data_slice_load_Bytes_uint16_t_to_u256(data_1_1, DEPOSIT_SIGNATURE_LENGTH_WORD);
  u256 index_length = log_data_slice_load_Bytes_uint16_t_to_u256(data_1_1, DEPOSIT_INDEX_LENGTH_WORD);
  bool tmp_3_693;
  if (!u256_eq_u64(pubkey_head, UINT8_C(160))) {
    tmp_3_693 = true;
  } else {
    bool tmp_3_692;
    if (!u256_eq_u64(withdrawal_credentials_head, UINT16_C(256))) {
      tmp_3_692 = true;
    } else {
      bool tmp_3_691;
      if (!u256_eq_u64(amount_head, UINT16_C(320))) {
        tmp_3_691 = true;
      } else {
        bool tmp_3_690;
        if (!u256_eq_u64(signature_head, UINT16_C(384))) {
          tmp_3_690 = true;
        } else {
          bool tmp_3_689;
          if (!u256_eq_u64(index_head, UINT16_C(512))) {
            tmp_3_689 = true;
          } else {
            bool tmp_3_688;
            if (!u256_eq_u64(pubkey_length, UINT8_C(48))) {
              tmp_3_688 = true;
            } else {
              bool tmp_3_687;
              if (!u256_eq_u64(withdrawal_credentials_length, UINT8_C(32))) {
                tmp_3_687 = true;
              } else {
                bool tmp_3_686 = (bool)((!u256_eq_u64(amount_length, UINT8_C(8))) || ((!u256_eq_u64(signature_length, UINT8_C(96))) || (!u256_eq_u64(index_length, UINT8_C(8)))));
                tmp_3_687 = tmp_3_686;
              }
              tmp_3_688 = tmp_3_687;
            }
            tmp_3_689 = tmp_3_688;
          }
          tmp_3_690 = tmp_3_689;
        }
        tmp_3_691 = tmp_3_690;
      }
      tmp_3_692 = tmp_3_691;
    }
    tmp_3_693 = tmp_3_692;
  }
  if (tmp_3_693) {
    fatal_error(InvalidExecutionRequests);
  }
  bool result_2_258 = (bool)(DEPOSIT_REQUEST_LENGTH <= expected.len);
  if (result_2_258) {
    Bytes log_pubkey = log_data_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(data_1_1, DEPOSIT_PUBKEY_DATA, DEPOSIT_PUBKEY_LENGTH);
    Bytes expected_pubkey = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_PUBKEY, DEPOSIT_PUBKEY_LENGTH);
    bool pubkey_matches = log_input_slices_equal(log_pubkey, expected_pubkey);
    Bytes log_withdrawal_credentials = log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(data_1_1, DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH);
    Bytes expected_withdrawal_credentials = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS, DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH);
    bool withdrawal_credentials_match = log_input_slices_equal(log_withdrawal_credentials, expected_withdrawal_credentials);
    Bytes log_amount = log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(data_1_1, DEPOSIT_AMOUNT_DATA, DEPOSIT_AMOUNT_LENGTH);
    Bytes expected_amount = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_AMOUNT, DEPOSIT_AMOUNT_LENGTH);
    bool amount_matches = log_input_slices_equal(log_amount, expected_amount);
    Bytes log_signature = log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(data_1_1, DEPOSIT_SIGNATURE_DATA, DEPOSIT_SIGNATURE_LENGTH);
    Bytes expected_signature = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_SIGNATURE, DEPOSIT_SIGNATURE_LENGTH);
    bool signature_matches = log_input_slices_equal(log_signature, expected_signature);
    Bytes log_index = log_data_sub_slice_Bytes_uint16_t_uint8_t_to_Bytes(data_1_1, DEPOSIT_INDEX_DATA, DEPOSIT_INDEX_LENGTH);
    Bytes expected_index = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_INDEX, DEPOSIT_INDEX_LENGTH);
    bool index_matches = log_input_slices_equal(log_index, expected_index);
    if (!pubkey_matches || (!withdrawal_credentials_match || (!amount_matches || (!signature_matches || !index_matches)))) {
      fatal_error(InvalidExecutionRequests);
    }
    return stateless_input_slice_suffix_Bytes_uint8_t_to_Bytes(expected, DEPOSIT_REQUEST_LENGTH);
  }
  fatal_error(InvalidExecutionRequests);
}

Bytes authenticate_deposit_logs(struct LogSeriesRef logs, Bytes expected)
{
  Bytes remaining = expected;
  uint64_t offset = UINT64_C(0);
  while (offset < logs.count) {
    uint64_t index = log_store_index_add(logs.start, offset);
    bool matches = deposit_log_matches(index);
    if (matches) {
      Bytes data = read_log_data(index);
      remaining = authenticate_deposit_request(data, remaining);
    }
    offset = log_store_index_increment(offset);
  }
  return remaining;
}

void validate_request_stream(bytes20 tgt, Bytes expected)
{
  Bytes dequeued = system_call_checked(tgt);
  bool matches = scratch_input_slices_equal(dequeued, expected);
  if (matches) {
    return;
  }
  fatal_error(InvalidExecutionRequests);
}

void validate_execution_requests(struct StatelessInputRef input_ref)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  validate_request_stream(WITHDRAWAL_REQUEST_ADDR, input_ref.withdrawal_requests);
  validate_request_stream(CONSOLIDATION_REQUEST_ADDR, input_ref.consolidation_requests);
  bool result_2_238 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_238) {
    validate_request_stream(BUILDER_DEPOSIT_REQUEST_ADDR, input_ref.builder_deposit_requests);
    validate_request_stream(BUILDER_EXIT_REQUEST_ADDR, input_ref.builder_exit_requests);
    return;
  }
  uint32_t builder_deposit_length = stateless_input_slice_length(input_ref.builder_deposit_requests);
  uint32_t builder_exit_length = stateless_input_slice_length(input_ref.builder_exit_requests);
  if ((builder_deposit_length != UINT8_C(0)) || (builder_exit_length != UINT8_C(0))) {
    fatal_error(InvalidExecutionRequests);
  }
}

