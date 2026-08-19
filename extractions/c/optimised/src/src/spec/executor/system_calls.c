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


Bytes run_system_call_frame(bytes20 tgt, struct CodeFields code, struct CalldataSlice input, uint32_t memory_base, uint64_t *restrict field_0_8_1505, uint64_t *restrict field_1_8_1506, uint32_t *restrict field_2_8_1507, __int128 *restrict field_3_8_1508, struct FrameStatus *restrict framestatus_8_1509)
{
  k_journal_checkpoint();
  StackPointer stack_top = stack_reset();
  return interpret_uint32_t_uint8_t_uint8_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint8_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(SYSTEM_CALL_GAS_LIMIT, STATE_GAS_ZERO, STATE_GAS_SPILL_ZERO, GAS_REFUND_ZERO, stack_top, memory_base, MEMORY_HEIGHT_ZERO, SYSTEM_ADDRESS, tgt, tgt, ZERO_WORD, STATE_GAS_ZERO, false, UINT8_C(0), code, input, field_0_8_1505, field_1_8_1506, field_2_8_1507, field_3_8_1508, framestatus_8_1509);
}

void system_call(bytes20 tgt, bytes32 input)
{
  bytes32 code_hash = k_code_key(tgt);
  if (eq_bytes32(code_hash, KECCAK_EMPTY)) {
    return;
  }
  struct CodeFields code = code_db_resolve(code_hash);
  struct MemoryRangeFields input_range = memory_range_uint8_t_uint8_t_to_struct_MemoryRangeFields(UINT8_C(0), SYSTEM_CALL_INPUT_LENGTH);
  uint32_t expanded_memory;
  if (input_range.len <= (UINT32_C(4294967295) - MEMORY_BASE_ZERO)) {
    if (MEMORY_HEIGHT_ZERO < input_range.len) {
      mem_expand(MEMORY_BASE_ZERO, MEMORY_HEIGHT_ZERO, input_range.len);
      expanded_memory = input_range.len;
    } else {
      expanded_memory = MEMORY_HEIGHT_ZERO;
    }
  } else {
    fatal_error(ExecutionInvalid);
  }
  mem_store(MEMORY_BASE_ZERO, input_range.off, (hash_to_word(input)));
  Bytes input_slice = active_memory_slice(MEMORY_BASE_ZERO, expanded_memory, input_range.off, input_range.len);
  uint32_t child_memory_base = memory_absolute(MEMORY_BASE_ZERO, expanded_memory);
  Bytes memory_input = evm_memory_slice(input_slice.bytes, input_slice.len);
  struct CalldataSlice frame_input = MemoryCalldata(memory_input);
  uint64_t _8_1505_8_1734;
  uint64_t _8_1506_8_1735;
  uint32_t _8_1507_8_1736;
  __int128 _8_1508_8_1737;
  struct FrameStatus _8_1509_8_1738;
  Bytes _8_1510_8_1739 = run_system_call_frame(tgt, code, frame_input, child_memory_base, &_8_1505_8_1734, &_8_1506_8_1735, &_8_1507_8_1736, &_8_1508_8_1737, &_8_1509_8_1738);
  if (frame_succeeded(_8_1509_8_1738)) {
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
  uint64_t _8_1505_8_1740;
  uint64_t _8_1506_8_1741;
  uint32_t _8_1507_8_1742;
  __int128 _8_1508_8_1743;
  struct FrameStatus _8_1509_8_1744;
  Bytes _8_1510_8_1745 = run_system_call_frame(tgt, code, EMPTY_CALLDATA, MEMORY_BASE_ZERO, &_8_1505_8_1740, &_8_1506_8_1741, &_8_1507_8_1742, &_8_1508_8_1743, &_8_1509_8_1744);
  if (frame_succeeded(_8_1509_8_1744)) {
    uint32_t start = scratch_reserve(_8_1510_8_1745.len);
    output_scratch_push_slice(_8_1510_8_1745);
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
  if (neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(address, DEPOSIT_CONTRACT_ADDR)) {
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
  if (!u256_eq_u64(pubkey_head, UINT8_C(160))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(withdrawal_credentials_head, UINT16_C(256))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(amount_head, UINT16_C(320))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(signature_head, UINT16_C(384))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(index_head, UINT16_C(512))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(pubkey_length, UINT8_C(48))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(withdrawal_credentials_length, UINT8_C(32))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(amount_length, UINT8_C(8))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(signature_length, UINT8_C(96))) {
    fatal_error(InvalidExecutionRequests);
  }
  if (!u256_eq_u64(index_length, UINT8_C(8))) {
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
    if (deposit_log_matches(index)) {
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

