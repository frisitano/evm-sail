/* Generated from sail/kernel/lifecycle.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zk_state_checkpoint(unit z3zE2372)
{
  uint64_t z8zE254;
  z8zE254 = host_state_checkpoint(UNIT);
end_function_2735: ;
  return z8zE254;
end_block_exception_2736: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_set_header(struct zBlockHeader zh)
{
  unit z8zE255;
  zk_header = zh;
  z8zE255 = UNIT;
end_function_2733: ;
  return z8zE255;
end_block_exception_2734: ;

  return UNIT;
}

unit zk_set_tx(struct zTxEnv zenv)
{
  unit z8zE256;
  zk_tx = zenv;
  z8zE256 = UNIT;
end_function_2731: ;
  return z8zE256;
end_block_exception_2732: ;

  return UNIT;
}

unit zk_tx_reset(unit z3zE2366)
{
  unit z8zE257;
  unit z3zE2371;
  z3zE2371 = acct_tx_reset(UNIT);
  unit z3zE2370;
  z3zE2370 = storage_tx_reset(UNIT);
  unit z3zE2369;
  z3zE2369 = warm_reset(UNIT);
  unit z3zE2368;
  z3zE2368 = transient_storage_reset(UNIT);
  unit z3zE2367;
  z3zE2367 = logs_tx_reset(UNIT);
  z8zE257 = host_state_checkpoint_reset(UNIT);
end_function_2729: ;
  return z8zE257;
end_block_exception_2730: ;

  return UNIT;
}

unit zk_tx_merge(unit z3zE2363)
{
  unit z8zE258;
  unit z3zE2365;
  z3zE2365 = zcycle_scope_start(zSCOPE_TX_MERGE);
  bool z2zE2413;
  z2zE2413 = zfork_gteq(zk_fork, zCancun);
  bool z2zE2414;
  z2zE2414 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE2364;
  z3zE2364 = evmsail_tx_merge(z2zE2413, z2zE2414, zk_block_access_index);
  z8zE258 = zcycle_scope_end(zSCOPE_TX_MERGE);
end_function_2727: ;
  return z8zE258;
end_block_exception_2728: ;

  return UNIT;
}

unit zk_revert(uint64_t zcheckpoint)
{
  unit z8zE259;
  z8zE259 = host_state_revert(zcheckpoint);
end_function_2725: ;
  return z8zE259;
end_block_exception_2726: ;

  return UNIT;
}
