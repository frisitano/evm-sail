#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes32 k_code_key(bytes20 a)
{
  return (k_aload_(a)).info.code_hash;
}

bytes32 k_get_codehash(bytes20 a)
{
  struct Account acc = k_aload_(a);
  if (acc.present) {
    return acc.info.code_hash;
  }
  return ZERO_HASH;
}

void k_deploy_code(bytes20 a, Bytes code)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct Account cur = k_aload_(a);
  bytes32 h = code_db_insert(code, execution_profile.protocol.fork);
  store_account_info_(a, cur, ((struct AccountInfo){.balance = cur.info.balance, .code_hash = h, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

void k_set_delegation(bytes20 a, bytes20 target)
{
  struct Account cur = k_aload_(a);
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  Bytes code_region = code_region_from_delegation(target);
  Bytes code = validated_code_slice(code_region);
  bytes32 h = code_db_insert(code, execution_profile.protocol.fork);
  store_account_info_(a, cur, ((struct AccountInfo){.balance = cur.info.balance, .code_hash = h, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

void k_clear_code(bytes20 a)
{
  struct Account cur = k_aload_(a);
  store_account_info_(a, cur, ((struct AccountInfo){.balance = cur.info.balance, .code_hash = KECCAK_EMPTY, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

bytes20 k_deleg_target(bytes20 a, bool *restrict condition_8_1363)
{
  bytes32 h = k_code_key(a);
  struct AddressResult r = code_db_read_delegation(h);
  (*condition_8_1363) = r.success;
  return r.address;
}

uint32_t k_get_code_size(bytes20 a)
{
  bytes32 code_key = k_code_key(a);
  return (code_db_resolve(code_key)).len;
}

void k_code_copy(bytes20 a, uint32_t dst, u256 off, uint32_t len)
{
  bytes32 code_key = k_code_key(a);
  struct CodeFields code = code_db_resolve(code_key);
  code_slice_copy_word_offset((code_bytes(code)), dst, off, len);
}

