/* Generated from sail/lib/state_trie.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zprepare_account_post_storage_root(struct zAcctEntry zentry)
{
  unit z8zE558;
  struct zAccount zcurrent;
  {
    struct zAcctValue z2zE1160;
    z2zE1160 = zentry.zvalue;
    zcurrent = z2zE1160.zcurr;
  }
  sail_fixed_bytes_20 z2zE1149;
  z2zE1149 = zentry.zaddr;
  unit z3zE1244;
  z3zE1244 = storage_block_iter_begin(z2zE1149);
  struct zTrieUpdateCursor zstorage_updates;
  CREATE(zTrieUpdateCursor)(&zstorage_updates);
  {
    struct zTrieUpdateSource z2zE1159;
    CREATE(zTrieUpdateSource)(&z2zE1159);
    {
      sail_fixed_bytes_20 z2zE1158;
      z2zE1158 = zentry.zaddr;
      zStorageTrieUpdates(&z2zE1159, z2zE1158);
    }
    ztrie_updates_begin(&zstorage_updates, z2zE1159);
    KILL(zTrieUpdateSource)(&z2zE1159);
  }
  sail_fixed_bytes_32 zbase_storage_root;
  {
    bool z2zE1156;
    z2zE1156 = zcurrent.zstorage_cleared;
    if (z2zE1156) {  zbase_storage_root = zEMPTY_TRIE_ROOT;  } else {
      struct zAccountInfo z2zE1157;
      z2zE1157 = zcurrent.zinfo;
      zbase_storage_root = z2zE1157.zstorage_root;
    }
  }
  sail_fixed_bytes_32 zstorage_root;
  {
    bool z2zE1154;
    {
      bool z2zE1153;
      {
        bool z2zE1151;
        z2zE1151 = zcurrent.zpresent;
        z2zE1153 = not(z2zE1151);
      }
      bool z3zE1245;
      if (z2zE1153) {  z3zE1245 = true;  } else {
        struct zAccountInfo z2zE1152;
        z2zE1152 = zcurrent.zinfo;
        z3zE1245 = zaccount_info_empty(z2zE1152);
      }
      z2zE1154 = z3zE1245;
    }
    if (z2zE1154) {  zstorage_root = zbase_storage_root;  } else {
      struct zTrieRootResult z2zE1155;
      {
        z2zE1155 = ztrie_root_cursor(zbase_storage_root, zstorage_updates);
        if (have_exception) {
          KILL(zTrieUpdateCursor)(&zstorage_updates);
          goto end_block_exception_1703;
        }
      }
      zstorage_root = z2zE1155.zroot;
    }
  }
  sail_fixed_bytes_20 z2zE1150;
  z2zE1150 = zentry.zaddr;
  z8zE558 = acct_post_storage_root_store(z2zE1150, zstorage_root);
  KILL(zTrieUpdateCursor)(&zstorage_updates);
end_function_1702: ;
  return z8zE558;
end_block_exception_1703: ;

  return UNIT;
}

unit zprepare_changed_account_post_storage_roots(unit z3zE1238)
{
  unit z8zE559;
  bool zpreparing;
  zpreparing = true;
  bool z3zE1242;
  unit z3zE1243;
while_1695: ;
  {
    z3zE1242 = zpreparing;
    if (!(z3zE1242)) goto wend_1696;
    struct zoptionzIRAcctEntryzK z2zE1148;
    CREATE(zoptionzIRAcctEntryzK)(&z2zE1148);
    acct_block_iter_next(&z2zE1148, UNIT);
    unit z3zE1239;
    {
      if (z2zE1148.kind != Kind_zSomezIRAcctEntryzK) goto case_1699;
      struct zAcctEntry zentry;
      zentry = z2zE1148.variants.zSomezIRAcctEntryzK;
      {
        z3zE1239 = zprepare_account_post_storage_root(zentry);
        if (have_exception) {
          KILL(zoptionzIRAcctEntryzK)(&z2zE1148);
          goto end_block_exception_1701;
        }
      }
      goto finish_match_1697;
    }
  case_1699: ;
    {
      /* complete */
      zpreparing = false;
      z3zE1239 = UNIT;
      goto finish_match_1697;
    }
  case_1698: ;
  finish_match_1697: ;
    z3zE1243 = z3zE1239;
    KILL(zoptionzIRAcctEntryzK)(&z2zE1148);
    goto while_1695;
  }
wend_1696: ;
  z8zE559 = UNIT;
end_function_1700: ;
  return z8zE559;
end_block_exception_1701: ;

  return UNIT;
}

sail_fixed_bytes_32 zcompute_state_root(unit z3zE1234)
{
  sail_fixed_bytes_32 z8zE560;
  unit z3zE1237;
  z3zE1237 = acct_block_iter_begin(UNIT);
  unit z3zE1236;
  {
    z3zE1236 = zprepare_changed_account_post_storage_roots(UNIT);
    if (have_exception) {  goto end_block_exception_1694;  }
  }
  unit z3zE1235;
  z3zE1235 = acct_block_iter_begin(UNIT);
  struct zTrieRootResult z2zE1147;
  {
    struct zTrieUpdateSource z2zE1146;
    CREATE(zTrieUpdateSource)(&z2zE1146);
    zChangedAccountTrieUpdates(&z2zE1146, UNIT);
    {
      z2zE1147 = ztrie_root(zk_parent_state_root, z2zE1146);
      if (have_exception) {
        KILL(zTrieUpdateSource)(&z2zE1146);
        goto end_block_exception_1694;
      }
    }
    KILL(zTrieUpdateSource)(&z2zE1146);
  }
  z8zE560 = z2zE1147.zroot;
end_function_1693: ;
  return z8zE560;
end_block_exception_1694: ;

  return fixed_bytes_32_zero();
}

