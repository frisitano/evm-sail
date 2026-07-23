/* Generated from sail/primitives/account.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_72(void) {

  struct zAccountInfo z3zE89;
  struct zAccountInfo z3zE88;
  z3zE88.zbalance = zZERO_WORD;
  z3zE88.zcode_hash = zKECCAK_EMPTY;
  z3zE88.znonce = UINT64_C(0);
  z3zE88.zstorage_root = zEMPTY_TRIE_ROOT;
  z3zE89 = z3zE88;
  zEMPTY_ACCOUNT_INFO = z3zE89;
let_end_181: ;
}
void kill_letbind_72(void) {
}

void create_letbind_73(void) {

  struct zAccount z3zE91;
  struct zAccount z3zE90;
  z3zE90.zcreated = false;
  z3zE90.zinfo = zEMPTY_ACCOUNT_INFO;
  z3zE90.zpresent = false;
  z3zE90.zselfdestructed = false;
  z3zE90.zstorage_cleared = true;
  z3zE91 = z3zE90;
  zEMPTY_ACCOUNT = z3zE91;
let_end_182: ;
}
void kill_letbind_73(void) {
}

struct zAccount zaccount_from_info(struct zAccountInfo zinfo)
{
  struct zAccount z8zE106;
  struct zAccount z3zE3706;
  z3zE3706.zcreated = false;
  z3zE3706.zinfo = zinfo;
  z3zE3706.zpresent = true;
  z3zE3706.zselfdestructed = false;
  z3zE3706.zstorage_cleared = false;
  z8zE106 = z3zE3706;
end_function_3762: ;
  return z8zE106;
end_block_exception_3763: ;
  struct zAccountInfo z8zE831 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE830 = { .zcreated = false, .zinfo = z8zE831, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE830;
}

