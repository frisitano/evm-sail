/* Generated from sail/primitives/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_74(void) {

  uint64_t z3zE92;
  z3zE92 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zOSAKA_TRANSACTION_GAS_LIMIT_VALUE = z3zE92;
let_end_184: ;
}
void kill_letbind_74(void) {
}

void create_letbind_75(void) {

  uint64_t z3zE93;
  z3zE93 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zOSAKA_TRANSACTION_GAS_LIMIT = z3zE93;
let_end_185: ;
}
void kill_letbind_75(void) {
}

void create_letbind_76(void) {

  struct zBlobHashes z3zE95;
  struct zBlobHashes z3zE94;
  z3zE94.zbytes = zEMPTY_SLICE;
  z3zE94.zcount = UINT64_C(0);
  z3zE95 = z3zE94;
  zEMPTY_BLOB_HASHES = z3zE95;
let_end_186: ;
}
void kill_letbind_76(void) {
}

uint64_t ztx_type_byte(enum zTxType zt)
{
  uint64_t z8zE107;
  uint64_t z3zE3629;
  {
    if ((zLegacyTx != zt)) goto case_3740;
    z3zE3629 = UINT64_C(0x00);
    goto finish_match_3735;
  }
case_3740: ;
  {
    if ((zAccessListTx != zt)) goto case_3739;
    z3zE3629 = UINT64_C(0x01);
    goto finish_match_3735;
  }
case_3739: ;
  {
    if ((zFeeMarketTx != zt)) goto case_3738;
    z3zE3629 = UINT64_C(0x02);
    goto finish_match_3735;
  }
case_3738: ;
  {
    if ((zBlobTx != zt)) goto case_3737;
    z3zE3629 = UINT64_C(0x03);
    goto finish_match_3735;
  }
case_3737: ;
  {
    /* complete */
    z3zE3629 = UINT64_C(0x04);
    goto finish_match_3735;
  }
case_3736: ;
finish_match_3735: ;
  z8zE107 = z3zE3629;
end_function_3741: ;
  return z8zE107;
end_block_exception_3742: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_is_access_list(enum zTxType zt)
{
  bool z8zE108;
  bool z3zE3626;
  {
    if ((zAccessListTx != zt)) goto case_3732;
    z3zE3626 = true;
    goto finish_match_3730;
  }
case_3732: ;
  {
    z3zE3626 = false;
    goto finish_match_3730;
  }
case_3731: ;
finish_match_3730: ;
  z8zE108 = z3zE3626;
end_function_3733: ;
  return z8zE108;
end_block_exception_3734: ;

  return false;
}

bool ztx_is_dynamic_fee(enum zTxType zt)
{
  bool z8zE109;
  bool z3zE3621;
  {
    if ((zFeeMarketTx != zt)) goto case_3727;
    z3zE3621 = true;
    goto finish_match_3723;
  }
case_3727: ;
  {
    if ((zBlobTx != zt)) goto case_3726;
    z3zE3621 = true;
    goto finish_match_3723;
  }
case_3726: ;
  {
    if ((zSetCodeTx != zt)) goto case_3725;
    z3zE3621 = true;
    goto finish_match_3723;
  }
case_3725: ;
  {
    z3zE3621 = false;
    goto finish_match_3723;
  }
case_3724: ;
finish_match_3723: ;
  z8zE109 = z3zE3621;
end_function_3728: ;
  return z8zE109;
end_block_exception_3729: ;

  return false;
}

bool ztx_is_blob(enum zTxType zt)
{
  bool z8zE110;
  bool z3zE3618;
  {
    if ((zBlobTx != zt)) goto case_3720;
    z3zE3618 = true;
    goto finish_match_3718;
  }
case_3720: ;
  {
    z3zE3618 = false;
    goto finish_match_3718;
  }
case_3719: ;
finish_match_3718: ;
  z8zE110 = z3zE3618;
end_function_3721: ;
  return z8zE110;
end_block_exception_3722: ;

  return false;
}

bool ztx_is_set_code(enum zTxType zt)
{
  bool z8zE111;
  bool z3zE3615;
  {
    if ((zSetCodeTx != zt)) goto case_3715;
    z3zE3615 = true;
    goto finish_match_3713;
  }
case_3715: ;
  {
    z3zE3615 = false;
    goto finish_match_3713;
  }
case_3714: ;
finish_match_3713: ;
  z8zE111 = z3zE3615;
end_function_3716: ;
  return z8zE111;
end_block_exception_3717: ;

  return false;
}

