/* Generated from sail/primitives/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_98(void) {

  uint64_t z3zE105;
  z3zE105 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zOSAKA_TRANSACTION_GAS_LIMIT_VALUE = z3zE105;
let_end_207: ;
}
void kill_letbind_98(void) {
}

void create_letbind_99(void) {

  uint64_t z3zE106;
  z3zE106 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zOSAKA_TRANSACTION_GAS_LIMIT = z3zE106;
let_end_208: ;
}
void kill_letbind_99(void) {
}

void create_letbind_100(void) {

  struct zBlobHashes z3zE108;
  struct zBlobHashes z3zE107;
  z3zE107.zbytes = zEMPTY_SLICE;
  z3zE107.zcount = UINT64_C(0);
  z3zE108 = z3zE107;
  zEMPTY_BLOB_HASHES = z3zE108;
let_end_209: ;
}
void kill_letbind_100(void) {
}

uint64_t ztx_type_byte(enum zTxType zt)
{
  uint64_t z8zE106;
  uint64_t z3zE2851;
  {
    if ((zLegacyTx != zt)) goto case_3328;
    z3zE2851 = UINT64_C(0x00);
    goto finish_match_3323;
  }
case_3328: ;
  {
    if ((zAccessListTx != zt)) goto case_3327;
    z3zE2851 = UINT64_C(0x01);
    goto finish_match_3323;
  }
case_3327: ;
  {
    if ((zFeeMarketTx != zt)) goto case_3326;
    z3zE2851 = UINT64_C(0x02);
    goto finish_match_3323;
  }
case_3326: ;
  {
    if ((zBlobTx != zt)) goto case_3325;
    z3zE2851 = UINT64_C(0x03);
    goto finish_match_3323;
  }
case_3325: ;
  {
    /* complete */
    z3zE2851 = UINT64_C(0x04);
    goto finish_match_3323;
  }
case_3324: ;
finish_match_3323: ;
  z8zE106 = z3zE2851;
end_function_3329: ;
  return z8zE106;
end_block_exception_3330: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_is_access_list(enum zTxType zt)
{
  bool z8zE107;
  bool z3zE2848;
  {
    if ((zAccessListTx != zt)) goto case_3320;
    z3zE2848 = true;
    goto finish_match_3318;
  }
case_3320: ;
  {
    z3zE2848 = false;
    goto finish_match_3318;
  }
case_3319: ;
finish_match_3318: ;
  z8zE107 = z3zE2848;
end_function_3321: ;
  return z8zE107;
end_block_exception_3322: ;

  return false;
}

bool ztx_is_dynamic_fee(enum zTxType zt)
{
  bool z8zE108;
  bool z3zE2843;
  {
    if ((zFeeMarketTx != zt)) goto case_3315;
    z3zE2843 = true;
    goto finish_match_3311;
  }
case_3315: ;
  {
    if ((zBlobTx != zt)) goto case_3314;
    z3zE2843 = true;
    goto finish_match_3311;
  }
case_3314: ;
  {
    if ((zSetCodeTx != zt)) goto case_3313;
    z3zE2843 = true;
    goto finish_match_3311;
  }
case_3313: ;
  {
    z3zE2843 = false;
    goto finish_match_3311;
  }
case_3312: ;
finish_match_3311: ;
  z8zE108 = z3zE2843;
end_function_3316: ;
  return z8zE108;
end_block_exception_3317: ;

  return false;
}

bool ztx_is_blob(enum zTxType zt)
{
  bool z8zE109;
  bool z3zE2840;
  {
    if ((zBlobTx != zt)) goto case_3308;
    z3zE2840 = true;
    goto finish_match_3306;
  }
case_3308: ;
  {
    z3zE2840 = false;
    goto finish_match_3306;
  }
case_3307: ;
finish_match_3306: ;
  z8zE109 = z3zE2840;
end_function_3309: ;
  return z8zE109;
end_block_exception_3310: ;

  return false;
}

bool ztx_is_set_code(enum zTxType zt)
{
  bool z8zE110;
  bool z3zE2837;
  {
    if ((zSetCodeTx != zt)) goto case_3303;
    z3zE2837 = true;
    goto finish_match_3301;
  }
case_3303: ;
  {
    z3zE2837 = false;
    goto finish_match_3301;
  }
case_3302: ;
finish_match_3301: ;
  z8zE110 = z3zE2837;
end_function_3304: ;
  return z8zE110;
end_block_exception_3305: ;

  return false;
}

