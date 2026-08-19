#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint64_t PRE_MERGE_BLOCK_REWARD = UINT64_C(2000000000000000000);


struct BlockGasUsageFields block_gas_usage_empty(void)
{
  return ((struct BlockGasUsageFields){.execution = UINT64_C(0), .receipts = u128_of_u64(UINT8_C(0)), .state = UINT64_C(0)});
}

void run_block_start_system_calls(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  if (profile.fork >= Cancun) {
    bytes32 result_2_225 = k_header.parent_beacon_block_root;
    system_call(BEACON_ROOTS_ADDR, result_2_225);
  }
  if (profile.fork >= Prague) {
    bytes32 result_2_228 = k_header.parent_hash;
    system_call(HISTORY_STORAGE_ADDR, result_2_228);
    return;
  }
}

struct BlockExecutionResult execute_block_transactions(struct BoundedSszListRef transactions, Bytes public_keys, Bytes expected_deposits)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct GasLimitsFields gas_limits = execution_profile.gas;
  uint8_t public_key_length = PUBLIC_KEY_LENGTH;
  if (((public_keys.len / (uint32_t)public_key_length) != transactions.count) || (public_keys.len != ((public_keys.len / (uint32_t)public_key_length) * (uint32_t)public_key_length))) {
    fatal_error(WitnessDeficient);
  }
  uint64_t gas_limit = gas_limits.block_limit;
  struct BlockGasUsageFields gas_usage = block_gas_usage_empty();
  uint32_t blob_gas_acc = UINT32_C(0);
  bytes20 tx0_to = ZERO_ADDRESS;
  receipt_store_begin();
  uint64_t transaction_logs_start = logs_tx_start();
  uint64_t transaction_logs_count = logs_tx_count();
  uint64_t logs_start = log_store_index_add(transaction_logs_start, transaction_logs_count);
  Bytes remaining_deposits = expected_deposits;
  struct BoundedSszListCursor cursor = ssz_list_cursor(transactions);
  Bytes keys = public_keys;
  bool initial_cursor_empty = ssz_list_cursor_empty(cursor);
  bool cursor_has_item = (bool)(!initial_cursor_empty);
  while (cursor_has_item) {
    uint32_t i = cursor.index;
    struct tuple_Bytes_BoundedSszListCursor ssz_list_pop_result_2_171 = ssz_list_pop(cursor);
    cursor = ssz_list_pop_result_2_171.tup1;
    Bytes keys_fields = keys;
    Bytes keys_fields_1_0;
    if (public_key_length <= keys_fields.len) {
      keys_fields_1_0 = keys_fields;
    } else {
      fatal_error(WitnessDeficient);
    }
    Bytes public_key = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(keys_fields_1_0, UINT8_C(0), PUBLIC_KEY_LENGTH);
    keys = stateless_input_slice_suffix_Bytes_uint8_t_to_Bytes(keys_fields_1_0, public_key_length);
    struct TransactionFields tx = decode_transaction_(ssz_list_pop_result_2_171.tup0, public_key);
    k_current_transaction_epoch = (i + UINT32_C(1));
    if (i == UINT8_C(0)) {
      tx0_to = tx.recipient;
    }
    struct BlockGasUsageFields usage = gas_usage;
    struct TransactionGasAllowanceFields allowance = transaction_gas_allowance(tx.gas_limit, gas_limits.transaction_total_limit, gas_limits.transaction_regular_limit);
    if (profile.fork >= Amsterdam) {
      if (((gas_limit - usage.execution) < allowance.regular) || ((gas_limit - usage.state) < allowance.total)) {
        fatal_error(GasUsedExceedsLimit);
      } else {
        uint32_t next_blob_gas = block_blob_gas_add_uint8_t_uint32_t_uint32_t_to_uint32_t(profile.blob_schedule.max, blob_gas_acc, (UINT32_C(131072) * (uint32_t)tx.blob_hashes.count));
        struct ReceiptFields receipt = process_transaction(tx, allowance);
        struct BlockGasUsageFields next_usage = block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint64_t_uint64_t_to_struct_BlockGasUsageFields(usage, receipt.execution_gas, receipt.state_gas, receipt.gas_used);
        gas_usage = next_usage;
        receipt_store_append(receipt, next_usage.receipts, i);
        remaining_deposits = authenticate_deposit_logs(receipt.logs, remaining_deposits);
        blob_gas_acc = next_blob_gas;
      }
    } else if ((gas_limit - usage.execution) < allowance.total) {
      fatal_error(GasUsedExceedsLimit);
    } else {
      uint32_t next_blob_gas_3_673;
      if (profile.fork < Cancun) {
        next_blob_gas_3_673 = blob_gas_acc;
      } else {
        next_blob_gas_3_673 = block_blob_gas_add(profile.blob_schedule.max, blob_gas_acc, (UINT32_C(131072) * (uint32_t)tx.blob_hashes.count));
      }
      struct ReceiptFields receipt_3_674 = process_transaction(tx, allowance);
      struct BlockGasUsageFields next_usage_3_675 = block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint8_t_uint64_t_to_struct_BlockGasUsageFields(usage, receipt_3_674.gas_used, UINT8_C(0), receipt_3_674.gas_used);
      gas_usage = next_usage_3_675;
      receipt_store_append(receipt_3_674, next_usage_3_675.receipts, i);
      if (profile.fork >= Prague) {
        remaining_deposits = authenticate_deposit_logs(receipt_3_674.logs, remaining_deposits);
      }
      blob_gas_acc = next_blob_gas_3_673;
    }
    bool cursor_empty = ssz_list_cursor_empty(cursor);
    cursor_has_item = (bool)(!cursor_empty);
  }
  uint32_t remaining_deposits_length = stateless_input_slice_length(remaining_deposits);
  bool result_2_212 = (bool)(profile.fork >= Prague);
  if (result_2_212 && (remaining_deposits_length != UINT8_C(0))) {
    fatal_error(InvalidExecutionRequests);
  }
  uint64_t header_gas_used;
  bool result_2_221 = (bool)(profile.fork >= Amsterdam);
  bool tmp_3_668 = (bool)(result_2_221 && (gas_usage.execution < gas_usage.state));
  header_gas_used = tmp_3_668 ? gas_usage.state : gas_usage.execution;
  bytes32 receipts_root = receipt_store_root(transactions.count);
  uint64_t retained_logs_start = logs_tx_start();
  uint64_t retained_logs_count = logs_tx_count();
  uint64_t retained = log_store_index_add(retained_logs_start, retained_logs_count);
  uint64_t logs_count = logs_start <= retained ? u128_to_u64(u128_sub_u64(u128_of_u64(retained), logs_start)) : UINT64_C(0);
  return ((struct BlockExecutionResult){.blob_gas_used = blob_gas_acc, .execution_gas_used = gas_usage.execution, .first_tx_recipient = tx0_to, .header_gas_used = header_gas_used, .logs = ((struct LogSeriesRef){.count = logs_count, .start = logs_start}), .receipts_root = receipts_root, .state_gas_used = gas_usage.state});
}

void apply_withdrawals(struct BoundedSszListRef withdrawals)
{
  struct BoundedSszListRef rest = withdrawals;
  while (true) {
    bool tmp_3_644;
    uint8_t result_2_162 = (uint8_t)rest.count;
    tmp_3_644 = (bool)(result_2_162 != UINT8_C(0));
    if (!tmp_3_644) {
      break;
    }
    struct tuple_Bytes_BoundedSszListRef ssz_fixed_list_pop_result_2_163 = ssz_fixed_list_pop(rest, WD_SIZE);
    rest = ssz_fixed_list_pop_result_2_163.tup1;
    struct Withdrawal withdrawal = decode_withdrawal_(ssz_fixed_list_pop_result_2_163.tup0);
    uint64_t amount = word_of_withdrawal_amount(withdrawal.amount);
    u128 amount_in_wei;
    u256 tmp_3_3761 = alu_mul(u256_of_fbits(amount), u256_of_fbits(UINT32_C(1000000000)));
    amount_in_wei = u128_of_u256_unchecked(tmp_3_3761);
    k_add_balance_bytes20_u128_to_unit(withdrawal.address, amount_in_wei);
  }
}

void apply_block_end_state(struct BlockBody body)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  if (profile.fork >= Shanghai) {
    apply_withdrawals(body.withdrawals);
  }
  if (profile.fork < Paris) {
    bytes20 coinbase = k_coinbase();
    k_add_balance_bytes20_uint64_t_to_unit(coinbase, PRE_MERGE_BLOCK_REWARD);
  }
  k_tx_merge();
}

struct BlockExecutionResult execute_block_body(struct BlockBody body, struct StatelessInputRef input_ref)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bal_reset();
  k_current_transaction_epoch = UINT32_C(0);
  warm_reset(k_current_transaction_epoch);
  run_block_start_system_calls();
  struct BlockExecutionResult result = execute_block_transactions(body.transactions, input_ref.public_keys, input_ref.deposits);
  k_current_transaction_epoch = (body.transactions.count + UINT32_C(1));
  warm_reset(k_current_transaction_epoch);
  apply_block_end_state(body);
  if (execution_profile.protocol.fork >= Prague) {
    validate_execution_requests(input_ref);
  }
  return result;
}

struct BlockGasUsageFields block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint64_t_uint64_t_to_struct_BlockGasUsageFields(struct BlockGasUsageFields usage, uint64_t add_execution, uint64_t add_state, uint64_t add_receipt)
{
  return ((struct BlockGasUsageFields){.execution = (add_execution + usage.execution), .receipts = u128_add_u64(usage.receipts, add_receipt), .state = (add_state + usage.state)});
}

struct BlockGasUsageFields block_gas_usage_add_struct_BlockGasUsageFields_uint64_t_uint8_t_uint64_t_to_struct_BlockGasUsageFields(struct BlockGasUsageFields usage, uint64_t add_execution, uint8_t add_state, uint64_t add_receipt)
{
  return ((struct BlockGasUsageFields){.execution = (add_execution + usage.execution), .receipts = u128_add_u64(usage.receipts, add_receipt), .state = (usage.state + (uint64_t)add_state)});
}

