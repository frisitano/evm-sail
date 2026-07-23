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
  uint64_t z3zE3700;
  {
    if ((zLegacyTx != zt)) goto case_3759;
    z3zE3700 = UINT64_C(0x00);
    goto finish_match_3754;
  }
case_3759: ;
  {
    if ((zAccessListTx != zt)) goto case_3758;
    z3zE3700 = UINT64_C(0x01);
    goto finish_match_3754;
  }
case_3758: ;
  {
    if ((zFeeMarketTx != zt)) goto case_3757;
    z3zE3700 = UINT64_C(0x02);
    goto finish_match_3754;
  }
case_3757: ;
  {
    if ((zBlobTx != zt)) goto case_3756;
    z3zE3700 = UINT64_C(0x03);
    goto finish_match_3754;
  }
case_3756: ;
  {
    /* complete */
    z3zE3700 = UINT64_C(0x04);
    goto finish_match_3754;
  }
case_3755: ;
finish_match_3754: ;
  z8zE107 = z3zE3700;
end_function_3760: ;
  return z8zE107;
end_block_exception_3761: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_is_access_list(enum zTxType zt)
{
  bool z8zE108;
  bool z3zE3697;
  {
    if ((zAccessListTx != zt)) goto case_3751;
    z3zE3697 = true;
    goto finish_match_3749;
  }
case_3751: ;
  {
    z3zE3697 = false;
    goto finish_match_3749;
  }
case_3750: ;
finish_match_3749: ;
  z8zE108 = z3zE3697;
end_function_3752: ;
  return z8zE108;
end_block_exception_3753: ;

  return false;
}

bool ztx_is_dynamic_fee(enum zTxType zt)
{
  bool z8zE109;
  bool z3zE3692;
  {
    if ((zFeeMarketTx != zt)) goto case_3746;
    z3zE3692 = true;
    goto finish_match_3742;
  }
case_3746: ;
  {
    if ((zBlobTx != zt)) goto case_3745;
    z3zE3692 = true;
    goto finish_match_3742;
  }
case_3745: ;
  {
    if ((zSetCodeTx != zt)) goto case_3744;
    z3zE3692 = true;
    goto finish_match_3742;
  }
case_3744: ;
  {
    z3zE3692 = false;
    goto finish_match_3742;
  }
case_3743: ;
finish_match_3742: ;
  z8zE109 = z3zE3692;
end_function_3747: ;
  return z8zE109;
end_block_exception_3748: ;

  return false;
}

bool ztx_is_blob(enum zTxType zt)
{
  bool z8zE110;
  bool z3zE3689;
  {
    if ((zBlobTx != zt)) goto case_3739;
    z3zE3689 = true;
    goto finish_match_3737;
  }
case_3739: ;
  {
    z3zE3689 = false;
    goto finish_match_3737;
  }
case_3738: ;
finish_match_3737: ;
  z8zE110 = z3zE3689;
end_function_3740: ;
  return z8zE110;
end_block_exception_3741: ;

  return false;
}

bool ztx_is_set_code(enum zTxType zt)
{
  bool z8zE111;
  bool z3zE3686;
  {
    if ((zSetCodeTx != zt)) goto case_3734;
    z3zE3686 = true;
    goto finish_match_3732;
  }
case_3734: ;
  {
    z3zE3686 = false;
    goto finish_match_3732;
  }
case_3733: ;
finish_match_3732: ;
  z8zE111 = z3zE3686;
end_function_3735: ;
  return z8zE111;
end_block_exception_3736: ;

  return false;
}

