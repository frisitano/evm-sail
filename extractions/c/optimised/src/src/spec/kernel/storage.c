#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bool k_account_is_warm(bytes20 a)
{
  enum PrecompileId precompile_id = precompile_id_for_address(a);
  if (neq_anything_EPrecompileId__(precompile_id, NotPrecompile)) {
    return true;
  }
  return account_is_warm(a);
}

void k_account_mark_warm(bytes20 a)
{
  enum PrecompileId precompile_id = precompile_id_for_address(a);
  if (neq_anything_EPrecompileId__(precompile_id, NotPrecompile)) {
    return;
  }
  require_historical_state_access(a, (None_R__sail_c_repr_u256_(UNIT)));
  account_mark_warm(a);
}

void k_prewarm_slot(bytes20 a, u256 s)
{
  require_historical_state_access(a, (Some_R__sail_c_repr_u256_(s)));
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

