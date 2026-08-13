#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct Account k_aload_(bytes20 a)
{
  return k_aload(k_parent_state_root, a);
}

bool account_info_empty(struct AccountInfo info)
{
  bool result_2_2117 = eq_bytes32(info.code_hash, KECCAK_EMPTY);
  if (result_2_2117) {
    if (info.nonce == UINT8_C(0)) {
      return eq_u256(info.balance, WORD_ZERO);
    }
    return false;
  }
  return false;
}

struct Account account_clear_storage(struct Account acc)
{
  struct Account tmp_3_3391 = acc;
  tmp_3_3391.storage_cleared = true;
  return tmp_3_3391;
}

void store_account_(bytes20 a, struct Account account)
{
  store_account(a, account);
}

void store_account_info_(bytes20 a, struct Account account, struct AccountInfo info)
{
  store_account_info(a, account, info);
}

u256 k_get_balance(bytes20 a)
{
  struct Account k_aload_result_2_2111 = k_aload_(a);
  return k_aload_result_2_2111.info.balance;
}

uint64_t k_get_nonce(bytes20 a)
{
  struct Account k_aload_result_2_2109 = k_aload_(a);
  return k_aload_result_2_2109.info.nonce;
}

bool k_account_exists(bytes20 a)
{
  struct Account k_aload_result_2_2108 = k_aload_(a);
  return k_aload_result_2_2108.present;
}

bool k_account_is_empty(bytes20 a)
{
  struct Account account = k_aload_(a);
  return account_info_empty(account.info);
}

bool k_account_occupied(bytes20 a)
{
  struct Account acc = k_aload_(a);
  struct AccountInfo info = acc.info;
  bool tmp_3_3388;
  if (acc.storage_cleared) {
    tmp_3_3388 = false;
  } else {
    tmp_3_3388 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(info.storage_root, EMPTY_TRIE_ROOT);
  }
  bool result_2_2102 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(info.code_hash, KECCAK_EMPTY);
  bool tmp_3_3390 = (bool)(result_2_2102 || ((info.nonce != UINT8_C(0)) || tmp_3_3388));
  if (tmp_3_3390) {
    return true;
  }
  return storage_has_writes(a);
}

void k_transfer(bytes20 src, bytes20 dst, u256 v)
{
  struct Account src_acc = k_aload_(src);
  struct Account dst_acc = k_aload_(dst);
  bool value_is_zero = eq_u256(v, WORD_ZERO);
  if (value_is_zero || eq_bytes20(src, dst)) {
    return;
  }
  u256 source_balance = alu_sub(src_acc.info.balance, v);
  struct AccountInfo tmp_3_3384 = src_acc.info;
  tmp_3_3384.balance = source_balance;
  store_account_info_(src, src_acc, tmp_3_3384);
  u256 destination_balance = alu_add(dst_acc.info.balance, v);
  struct AccountInfo tmp_3_3386 = dst_acc.info;
  tmp_3_3386.balance = destination_balance;
  store_account_info_(dst, dst_acc, tmp_3_3386);
  k_emit_transfer_log(src, dst, v);
}

void k_bump_nonce(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.info.nonce < UINT64_C(18446744073709551615)) {
    struct AccountInfo tmp_3_3381 = cur.info;
    tmp_3_3381.nonce = (cur.info.nonce + UINT64_C(1));
    store_account_info_(a, cur, tmp_3_3381);
    return;
  }
  fatal_error(ExecutionInvalid);
}

void k_add_balance(bytes20 a, u256 v)
{
  struct Account cur = k_aload_(a);
  bool value_is_zero = eq_u256(v, WORD_ZERO);
  if (value_is_zero) {
    return;
  }
  u256 balance = alu_add(cur.info.balance, v);
  struct AccountInfo tmp_3_3380 = cur.info;
  tmp_3_3380.balance = balance;
  store_account_info_(a, cur, tmp_3_3380);
}

void k_sub_balance(bytes20 a, u256 v)
{
  struct Account cur = k_aload_(a);
  bool value_is_zero = eq_u256(v, WORD_ZERO);
  if (value_is_zero) {
    return;
  }
  u256 balance = alu_sub(cur.info.balance, v);
  struct AccountInfo tmp_3_3379 = cur.info;
  tmp_3_3379.balance = balance;
  store_account_info_(a, cur, tmp_3_3379);
}

void k_clear_storage(bytes20 a)
{
  struct Account cur = k_aload_(a);
  storage_tx_clear(a);
  struct Account cleared = account_clear_storage(cur);
  store_account_(a, cleared);
}

void k_add_balance_bytes20_u128_to_unit(bytes20 a, u128 v)
{
  struct Account cur = k_aload_(a);
  bool value_is_zero = u256_eq_u128(WORD_ZERO, v);
  if (value_is_zero) {
    return;
  }
  u256 balance = alu_add_u256_u128_to_u256(cur.info.balance, v);
  struct AccountInfo tmp_3_3380 = cur.info;
  tmp_3_3380.balance = balance;
  store_account_info_(a, cur, tmp_3_3380);
}

void k_add_balance_bytes20_uint64_t_to_unit(bytes20 a, uint64_t v)
{
  struct Account cur = k_aload_(a);
  bool value_is_zero = u256_eq_u64(WORD_ZERO, v);
  if (value_is_zero) {
    return;
  }
  u256 balance = alu_add_u256_uint64_t_to_u256(cur.info.balance, v);
  struct AccountInfo tmp_3_3380 = cur.info;
  tmp_3_3380.balance = balance;
  store_account_info_(a, cur, tmp_3_3380);
}

