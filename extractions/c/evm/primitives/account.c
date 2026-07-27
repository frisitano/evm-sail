/* Generated from sail/primitives/account.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_96(void) {

  struct zAccountInfo z3zE102;
  struct zAccountInfo z3zE101;
  z3zE101.zbalance = zZERO_WORD;
  z3zE101.zcode_hash = zKECCAK_EMPTY;
  z3zE101.znonce = UINT64_C(0);
  z3zE101.zstorage_root = zEMPTY_TRIE_ROOT;
  z3zE102 = z3zE101;
  zEMPTY_ACCOUNT_INFO = z3zE102;
let_end_205: ;
}
void kill_letbind_96(void) {
}

void create_letbind_97(void) {

  struct zAccount z3zE104;
  struct zAccount z3zE103;
  z3zE103.zcreated = false;
  z3zE103.zinfo = zEMPTY_ACCOUNT_INFO;
  z3zE103.zpresent = false;
  z3zE103.zselfdestructed = false;
  z3zE103.zstorage_cleared = true;
  z3zE104 = z3zE103;
  zEMPTY_ACCOUNT = z3zE104;
let_end_206: ;
}
void kill_letbind_97(void) {
}

