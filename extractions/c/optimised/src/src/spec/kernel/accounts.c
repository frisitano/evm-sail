#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct Account k_aload_(bytes20 a)
{
  return k_aload(k_parent_state_root, a);
}

bool account_info_empty(struct AccountInfo info)
{
  if (eq_bytes32(info.code_hash, KECCAK_EMPTY)) {
    if (info.nonce == UINT8_C(0)) {
      return eq_u256(info.balance, WORD_ZERO);
    }
    return false;
  }
  return false;
}

struct Account account_clear_storage(struct Account acc)
{
  return ((struct Account){.created = acc.created, .info = acc.info, .present = acc.present, .selfdestructed = acc.selfdestructed, .storage_cleared = true});
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
  return (k_aload_(a)).info.balance;
}

uint64_t k_get_nonce(bytes20 a)
{
  return (k_aload_(a)).info.nonce;
}

bool k_account_exists(bytes20 a)
{
  return (k_aload_(a)).present;
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
  bool anchored_storage = false;
  if (!acc.storage_cleared) {
    anchored_storage = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(info.storage_root, EMPTY_TRIE_ROOT);
  }
  bool has_code = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(info.code_hash, KECCAK_EMPTY);
  if (has_code || ((info.nonce != UINT8_C(0)) || anchored_storage)) {
    return true;
  }
  return storage_has_writes(a);
}

void k_transfer(bytes20 src, bytes20 dst, u256 v)
{
  struct Account src_acc = k_aload_(src);
  struct Account dst_acc = k_aload_(dst);
  if (eq_u256(v, WORD_ZERO) || eq_bytes20(src, dst)) {
    return;
  }
  store_account_info_(src, src_acc, ((struct AccountInfo){.balance = (alu_sub(src_acc.info.balance, v)), .code_hash = src_acc.info.code_hash, .nonce = src_acc.info.nonce, .storage_root = src_acc.info.storage_root}));
  store_account_info_(dst, dst_acc, ((struct AccountInfo){.balance = (alu_add(dst_acc.info.balance, v)), .code_hash = dst_acc.info.code_hash, .nonce = dst_acc.info.nonce, .storage_root = dst_acc.info.storage_root}));
  k_emit_transfer_log(src, dst, v);
}

void k_bump_nonce(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.info.nonce < UINT64_C(18446744073709551615)) {
    store_account_info_(a, cur, ((struct AccountInfo){.balance = cur.info.balance, .code_hash = cur.info.code_hash, .nonce = (cur.info.nonce + UINT64_C(1)), .storage_root = cur.info.storage_root}));
    return;
  }
  fatal_error(ExecutionInvalid);
}

void k_add_balance(bytes20 a, u256 v)
{
  struct Account cur = k_aload_(a);
  if (eq_u256(v, WORD_ZERO)) {
    return;
  }
  store_account_info_(a, cur, ((struct AccountInfo){.balance = (alu_add(cur.info.balance, v)), .code_hash = cur.info.code_hash, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

void k_sub_balance(bytes20 a, u256 v)
{
  struct Account cur = k_aload_(a);
  if (eq_u256(v, WORD_ZERO)) {
    return;
  }
  store_account_info_(a, cur, ((struct AccountInfo){.balance = (alu_sub(cur.info.balance, v)), .code_hash = cur.info.code_hash, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

void k_clear_storage(bytes20 a)
{
  struct Account cur = k_aload_(a);
  storage_tx_clear(a);
  store_account_(a, (account_clear_storage(cur)));
}

void k_add_balance_bytes20_u128_to_unit(bytes20 a, u128 v)
{
  struct Account cur = k_aload_(a);
  if (u256_eq_u128(WORD_ZERO, v)) {
    return;
  }
  store_account_info_(a, cur, ((struct AccountInfo){.balance = (alu_add_u256_u128_to_u256(cur.info.balance, v)), .code_hash = cur.info.code_hash, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

void k_add_balance_bytes20_uint64_t_to_unit(bytes20 a, uint64_t v)
{
  struct Account cur = k_aload_(a);
  if (u256_eq_u64(WORD_ZERO, v)) {
    return;
  }
  store_account_info_(a, cur, ((struct AccountInfo){.balance = (alu_add_u256_uint64_t_to_u256(cur.info.balance, v)), .code_hash = cur.info.code_hash, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

