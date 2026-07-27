/* Generated from sail/host/kernel/storage.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
bool zk_access_account(sail_fixed_bytes_20 za)
{
  bool z8zE213;
  z8zE213 = warm_addr_touch(za);
end_function_2828: ;
  return z8zE213;
end_block_exception_2829: ;

  return false;
}

bool zk_slot_is_warm(sail_fixed_bytes_20 za, sail_u256 zs)
{
  bool z8zE214;
  z8zE214 = warm_slot_touch(za, zs);
end_function_2826: ;
  return z8zE214;
end_block_exception_2827: ;

  return false;
}

struct zAccount zoptimizzed_account_unwrap(struct zOptimizzedAccountResult zresult)
{
  struct zAccount z8zE215;
  struct zAccount z3zE2411;
  {
    if (zresult.kind != Kind_zOptimizzedAccountOk) goto case_2823;
    struct zAccount zvalue;
    zvalue = zresult.variants.zOptimizzedAccountOk;
    z3zE2411 = zvalue;
    goto finish_match_2821;
  }
case_2823: ;
  {
    /* complete */
    enum zBlockError zreason;
    zreason = zresult.variants.zOptimizzedAccountError;
    struct zexception z2zE2513;
    CREATE(zexception)(&z2zE2513);
    zInvalidBlock(&z2zE2513, zreason);
    COPY(zexception)(current_exception, z2zE2513);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:235.41-235.67");
    KILL(zexception)(&z2zE2513);
    goto end_block_exception_2825;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2513);
    goto finish_match_2821;
  }
case_2822: ;
finish_match_2821: ;
  z8zE215 = z3zE2411;
end_function_2824: ;
  return z8zE215;
end_block_exception_2825: ;
  struct zAccountInfo z8zE805 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE804 = { .zcreated = false, .zinfo = z8zE805, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE804;
}

struct zAccount zk_aload(sail_fixed_bytes_20 za)
{
  struct zAccount z8zE216;
  struct zOptimizzedAccountResult z2zE2512;
  CREATE(zOptimizzedAccountResult)(&z2zE2512);
  evmsail_k_aload(&z2zE2512, zk_parent_state_root, za);
  {
    z8zE216 = zoptimizzed_account_unwrap(z2zE2512);
    if (have_exception) {
      KILL(zOptimizzedAccountResult)(&z2zE2512);
      goto end_block_exception_2820;
    }
  }
  KILL(zOptimizzedAccountResult)(&z2zE2512);
end_function_2819: ;
  return z8zE216;
end_block_exception_2820: ;
  struct zAccountInfo z8zE807 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE806 = { .zcreated = false, .zinfo = z8zE807, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE806;
}

unit zoptimizzed_unit_unwrap(struct zOptimizzedUnitResult zresult)
{
  unit z8zE217;
  unit z3zE2408;
  {
    if (zresult.kind != Kind_zOptimizzedUnitOk) goto case_2816;
    z3zE2408 = UNIT;
    goto finish_match_2814;
  }
case_2816: ;
  {
    /* complete */
    enum zBlockError zreason;
    zreason = zresult.variants.zOptimizzedUnitError;
    struct zexception z2zE2511;
    CREATE(zexception)(&z2zE2511);
    zInvalidBlock(&z2zE2511, zreason);
    COPY(zexception)(current_exception, z2zE2511);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:223.38-223.64");
    KILL(zexception)(&z2zE2511);
    goto end_block_exception_2818;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2511);
    goto finish_match_2814;
  }
case_2815: ;
finish_match_2814: ;
  z8zE217 = z3zE2408;
end_function_2817: ;
  return z8zE217;
end_block_exception_2818: ;

  return UNIT;
}

struct zStorageValue zoptimizzed_storage_unwrap(struct zOptimizzedStorageResult zresult)
{
  struct zStorageValue z8zE218;
  struct zStorageValue z3zE2405;
  {
    if (zresult.kind != Kind_zOptimizzedStorageOk) goto case_2811;
    struct zStorageValue zvalue;
    zvalue = zresult.variants.zOptimizzedStorageOk;
    z3zE2405 = zvalue;
    goto finish_match_2809;
  }
case_2811: ;
  {
    /* complete */
    enum zBlockError zreason;
    zreason = zresult.variants.zOptimizzedStorageError;
    struct zexception z2zE2510;
    CREATE(zexception)(&z2zE2510);
    zInvalidBlock(&z2zE2510, zreason);
    COPY(zexception)(current_exception, z2zE2510);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:241.41-241.67");
    KILL(zexception)(&z2zE2510);
    goto end_block_exception_2813;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2510);
    goto finish_match_2809;
  }
case_2810: ;
finish_match_2809: ;
  z8zE218 = z3zE2405;
end_function_2812: ;
  return z8zE218;
end_block_exception_2813: ;
  struct zStorageValue z8zE808 = { .zcurr = u256_zero(), .zorig = u256_zero() };
  return z8zE808;
}

struct zStorageValue zk_sload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  struct zStorageValue z8zE219;
  struct zOptimizzedStorageResult z2zE2509;
  CREATE(zOptimizzedStorageResult)(&z2zE2509);
  evmsail_k_sload(&z2zE2509, zk_parent_state_root, za, zs);
  {
    z8zE219 = zoptimizzed_storage_unwrap(z2zE2509);
    if (have_exception) {
      KILL(zOptimizzedStorageResult)(&z2zE2509);
      goto end_block_exception_2808;
    }
  }
  KILL(zOptimizzedStorageResult)(&z2zE2509);
end_function_2807: ;
  return z8zE219;
end_block_exception_2808: ;
  struct zStorageValue z8zE809 = { .zcurr = u256_zero(), .zorig = u256_zero() };
  return z8zE809;
}

unit zk_sstore(sail_fixed_bytes_20 za, sail_u256 zs, struct zStorageValue zvalue)
{
  unit z8zE220;
  sail_u256 z2zE2507;
  z2zE2507 = zvalue.zcurr;
  sail_u256 z2zE2508;
  z2zE2508 = zvalue.zorig;
  z8zE220 = evmsail_k_sstore(za, zs, z2zE2507, z2zE2508);
end_function_2805: ;
  return z8zE220;
end_block_exception_2806: ;

  return UNIT;
}

sail_u256 zk_tload(sail_fixed_bytes_20 za, sail_u256 zs)
{
  sail_u256 z8zE221;
  z8zE221 = transient_storage_read(za, zs);
end_function_2803: ;
  return z8zE221;
end_block_exception_2804: ;

  return u256_zero();
}

unit zk_tstore(sail_fixed_bytes_20 za, sail_u256 zs, sail_u256 zv)
{
  unit z8zE222;
  z8zE222 = transient_storage_write(za, zs, zv);
end_function_2801: ;
  return z8zE222;
end_block_exception_2802: ;

  return UNIT;
}

