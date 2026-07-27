/* Generated from sail/primitives/evm.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_104(void) {

  struct zMessage z3zE115;
  struct zMessage z3zE114;
  z3zE114.zaddress = zZERO_ADDRESS;
  z3zE114.zcaller = zZERO_ADDRESS;
  z3zE114.zcode_address = zZERO_ADDRESS;
  z3zE114.zdepth = UINT64_C(0);
  z3zE114.zis_static = false;
  z3zE114.zstate_gas_reservoir = zGAS_ZERO;
  z3zE114.zvalue = zZERO_WORD;
  z3zE115 = z3zE114;
  zDEFAULT_MESSAGE = z3zE115;
let_end_220: ;
}
void kill_letbind_104(void) {
}

