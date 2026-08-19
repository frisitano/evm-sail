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


struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_system_call_frame(bytes20 tgt, struct CodeFields code, struct CalldataSlice input, uint32_t memory_base)
{
  k_journal_checkpoint();
  StackPointer stack_top = stack_reset();
  return interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(SYSTEM_CALL_GAS_LIMIT, STATE_GAS_ZERO, STATE_GAS_SPILL_ZERO, GAS_REFUND_ZERO, stack_top, memory_base, MEMORY_HEIGHT_ZERO, SYSTEM_ADDRESS, tgt, tgt, ZERO_WORD, STATE_GAS_ZERO, false, UINT8_C(0), code, input);
}

void system_call(bytes20 tgt, bytes32 input)
{
  bytes32 code_hash = k_code_key(tgt);
  if (eq_bytes32(code_hash, KECCAK_EMPTY)) {
    return;
  }
  struct CodeFields code = code_db_resolve(code_hash);
  uint32_t initial_memory_base = MEMORY_BASE_ZERO;
  uint32_t initial_memory_height = MEMORY_HEIGHT_ZERO;
  struct MemoryRangeFields input_range = memory_range_uint8_t_uint8_t_to_struct_MemoryRangeFields(UINT8_C(0), SYSTEM_CALL_INPUT_LENGTH);
  uint32_t expanded_memory;
  if (input_range.len <= (UINT32_C(4294967295) - initial_memory_base)) {
    if (initial_memory_height < input_range.len) {
      mem_expand(initial_memory_base, initial_memory_height, input_range.len);
      expanded_memory = input_range.len;
    } else {
      expanded_memory = initial_memory_height;
    }
  } else {
    fatal_error(ExecutionInvalid);
  }
  u256 input_word = hash_to_word(input);
  mem_store(initial_memory_base, input_range.off, input_word);
  Bytes input_slice = active_memory_slice(initial_memory_base, expanded_memory, input_range.off, input_range.len);
  uint32_t child_memory_base = memory_absolute(initial_memory_base, expanded_memory);
  Bytes memory_input = evm_memory_slice(input_slice.bytes, input_slice.len);
  struct CalldataSlice frame_input = MemoryCalldata(memory_input);
  struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_system_call_frame_result_2_269 = run_system_call_frame(tgt, code, frame_input, child_memory_base);
  bool succeeded = frame_succeeded(run_system_call_frame_result_2_269.tup4);
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
  if (eq_bytes32(code_hash, KECCAK_EMPTY)) {
    fatal_error(ExecutionInvalid);
  }
  struct CodeFields code = code_db_resolve(code_hash);
  struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_system_call_frame_result_2_265 = run_system_call_frame(tgt, code, EMPTY_CALLDATA, MEMORY_BASE_ZERO);
  bool succeeded = frame_succeeded(run_system_call_frame_result_2_265.tup4);
  if (succeeded) {
    uint32_t start = scratch_reserve(run_system_call_frame_result_2_265.tup5.len);
    output_scratch_push_slice(run_system_call_frame_result_2_265.tup5);
    Bytes result = scratch_finish(start);
    k_journal_commit();
    k_tx_merge();
    return result;
  }
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
  if (data.len == DEPOSIT_EVENT_DATA_LENGTH) {
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
  bool tmp_3_696;
  if (!u256_eq_u64(pubkey_head, UINT8_C(160))) {
    tmp_3_696 = true;
  } else {
    bool tmp_3_695;
    if (!u256_eq_u64(withdrawal_credentials_head, UINT16_C(256))) {
      tmp_3_695 = true;
    } else {
      bool tmp_3_694;
      if (!u256_eq_u64(amount_head, UINT16_C(320))) {
        tmp_3_694 = true;
      } else {
        bool tmp_3_693;
        if (!u256_eq_u64(signature_head, UINT16_C(384))) {
          tmp_3_693 = true;
        } else {
          bool tmp_3_692;
          if (!u256_eq_u64(index_head, UINT16_C(512))) {
            tmp_3_692 = true;
          } else {
            bool tmp_3_691;
            if (!u256_eq_u64(pubkey_length, UINT8_C(48))) {
              tmp_3_691 = true;
            } else {
              bool tmp_3_690;
              if (!u256_eq_u64(withdrawal_credentials_length, UINT8_C(32))) {
                tmp_3_690 = true;
              } else {
                bool tmp_3_689 = (bool)((!u256_eq_u64(amount_length, UINT8_C(8))) || ((!u256_eq_u64(signature_length, UINT8_C(96))) || (!u256_eq_u64(index_length, UINT8_C(8)))));
                tmp_3_690 = tmp_3_689;
              }
              tmp_3_691 = tmp_3_690;
            }
            tmp_3_692 = tmp_3_691;
          }
          tmp_3_693 = tmp_3_692;
        }
        tmp_3_694 = tmp_3_693;
      }
      tmp_3_695 = tmp_3_694;
    }
    tmp_3_696 = tmp_3_695;
  }
  if (tmp_3_696) {
    fatal_error(InvalidExecutionRequests);
  }
  if (DEPOSIT_REQUEST_LENGTH <= expected.len) {
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
  if (execution_profile.protocol.fork >= Amsterdam) {
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

