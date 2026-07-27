/* Generated from sail/lib/state_trie.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_32 zoptimizzed_hash_unwrap(struct zOptimizzedHashResult zresult)
{
  sail_fixed_bytes_32 z8zE604;
  sail_fixed_bytes_32 z3zE921;
  {
    if (zresult.kind != Kind_zOptimizzedHashOk) goto case_1455;
    sail_fixed_bytes_32 zvalue;
    zvalue = zresult.variants.zOptimizzedHashOk;
    z3zE921 = zvalue;
    goto finish_match_1453;
  }
case_1455: ;
  {
    /* complete */
    enum zBlockError zreason;
    zreason = zresult.variants.zOptimizzedHashError;
    struct zexception z2zE721;
    CREATE(zexception)(&z2zE721);
    zInvalidBlock(&z2zE721, zreason);
    COPY(zexception)(current_exception, z2zE721);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:229.38-229.64");
    KILL(zexception)(&z2zE721);
    goto end_block_exception_1457;
    /* unreachable after throw */
    KILL(zexception)(&z2zE721);
    goto finish_match_1453;
  }
case_1454: ;
finish_match_1453: ;
  z8zE604 = z3zE921;
end_function_1456: ;
  return z8zE604;
end_block_exception_1457: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zcompute_state_root(unit z3zE920)
{
  sail_fixed_bytes_32 z8zE605;
  struct zOptimizzedHashResult z2zE720;
  CREATE(zOptimizzedHashResult)(&z2zE720);
  evmsail_compute_state_root(&z2zE720, zk_parent_state_root);
  {
    z8zE605 = zoptimizzed_hash_unwrap(z2zE720);
    if (have_exception) {
      KILL(zOptimizzedHashResult)(&z2zE720);
      goto end_block_exception_1452;
    }
  }
  KILL(zOptimizzedHashResult)(&z2zE720);
end_function_1451: ;
  return z8zE605;
end_block_exception_1452: ;

  return fixed_bytes_32_zero();
}

