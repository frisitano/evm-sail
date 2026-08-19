#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes32 k_code_key(bytes20 a)
{
  struct Account k_aload_result_2_2054 = k_aload_(a);
  return k_aload_result_2_2054.info.code_hash;
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
  struct AccountInfo tmp_3_3118 = cur.info;
  tmp_3_3118.code_hash = h;
  store_account_info_(a, cur, tmp_3_3118);
}

void k_set_delegation(bytes20 a, bytes20 target)
{
  struct Account cur = k_aload_(a);
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  Bytes code_region = code_region_from_delegation(target);
  Bytes code = validated_code_slice(code_region);
  bytes32 h = code_db_insert(code, execution_profile.protocol.fork);
  struct AccountInfo tmp_3_3117 = cur.info;
  tmp_3_3117.code_hash = h;
  store_account_info_(a, cur, tmp_3_3117);
}

void k_clear_code(bytes20 a)
{
  struct Account cur = k_aload_(a);
  struct AccountInfo tmp_3_3116 = cur.info;
  tmp_3_3116.code_hash = KECCAK_EMPTY;
  store_account_info_(a, cur, tmp_3_3116);
}

struct tuple_bool_bytes20 k_deleg_target(bytes20 a)
{
  bytes32 h = k_code_key(a);
  struct AddressResult r = code_db_read_delegation(h);
  return ((struct tuple_bool_bytes20){.tup0 = r.success, .tup1 = r.address});
}

uint32_t k_get_code_size(bytes20 a)
{
  bytes32 code_key = k_code_key(a);
  struct CodeFields code = code_db_resolve(code_key);
  return code.len;
}

void k_code_copy(bytes20 a, uint32_t dst, u256 off, uint32_t len)
{
  bytes32 code_key = k_code_key(a);
  struct CodeFields code = code_db_resolve(code_key);
  Bytes bytes = code_bytes(code);
  code_slice_copy_word_offset(bytes, dst, off, len);
}

