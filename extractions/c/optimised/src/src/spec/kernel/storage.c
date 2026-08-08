#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bool k_account_is_warm(bytes20 a)
{
  enum PrecompileId precompile_id = precompile_id_for_address(a);
  bool neq_anything_result_2_1960 = neq_anything_EPrecompileId__(precompile_id, NotPrecompile);
  if (neq_anything_result_2_1960) {
    return true;
  }
  return account_is_warm(a);
}

void k_account_mark_warm(bytes20 a)
{
  enum PrecompileId precompile_id = precompile_id_for_address(a);
  bool neq_anything_result_2_1959 = neq_anything_EPrecompileId__(precompile_id, NotPrecompile);
  if (neq_anything_result_2_1959) {
    return;
  }
  account_mark_warm(a);
}

void k_prewarm_slot(bytes20 a, u256 s)
{
  storage_mark_warm(a, s);
}

u256 k_tload(bytes20 a, u256 s)
{
  return transient_storage_read(a, s);
}

void k_tstore(bytes20 a, u256 s, u256 v)
{
  transient_storage_write(a, s, v);
}

