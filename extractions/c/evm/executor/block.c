/* Generated from sail/executor/block.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_311(void) {

  uint64_t z3zE369;
  z3zE369 = UINT64_C(2000000000000000000);
  zPRE_MERGE_BLOCK_REWARD = z3zE369;
let_end_1009: ;
}
void kill_letbind_311(void) {
}

uint64_t zremaining_block_gas(uint64_t zlimit, uint64_t zused)
{
  uint64_t z8zE660;
  bool z2zE521;
  z2zE521 = (!(zlimit < zused));
  if (z2zE521) {
    {    z8zE660 = (zlimit - zused);
    }
  } else {
    struct zexception z2zE522;
    CREATE(zexception)(&z2zE522);
    zInvalidBlock(&z2zE522, zGasUsedExceedsLimit);
    COPY(zexception)(current_exception, z2zE522);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block.sail:30.8-30.47");
    KILL(zexception)(&z2zE522);
    goto end_block_exception_1373;
    /* unreachable after throw */
    KILL(zexception)(&z2zE522);
  }
end_function_1372: ;
  return z8zE660;
end_block_exception_1373: ;

  return UINT64_C(0xdeadc0de);
}

unit zrun_block_start_system_calls(unit z3zE906)
{
  unit z8zE661;
  bool z2zE517;
  z2zE517 = zfork_gteq(zk_fork, zCancun);
  unit z3zE907;
  if (z2zE517) {
    sail_fixed_bytes_32 z2zE518;
    z2zE518 = zk_header.zparent_beacon_block_root;
    {
      z3zE907 = zsystem_call(zBEACON_ROOTS_ADDR, z2zE518);
      if (have_exception) {  goto end_block_exception_1371;  }
    }
  } else {  z3zE907 = UNIT;  }
  bool z2zE519;
  z2zE519 = zfork_gteq(zk_fork, zPrague);
  if (z2zE519) {
    sail_fixed_bytes_32 z2zE520;
    z2zE520 = zk_header.zparent_hash;
    {
      z8zE661 = zsystem_call(zHISTORY_STORAGE_ADDR, z2zE520);
      if (have_exception) {  goto end_block_exception_1371;  }
    }
  } else {  z8zE661 = UNIT;  }
end_function_1370: ;
  return z8zE661;
end_block_exception_1371: ;

  return UNIT;
}

struct zBlockExecutionResult zexecute_block_transactions(struct zBoundedSszzListRef ztransactions, struct zByteSliceFields zpublic_keys, uint64_t zheader_gas_limit)
{
  struct zBlockExecutionResult z8zE662;
  uint64_t zpublic_keys_length;
  zpublic_keys_length = zpublic_keys.zlen;
  uint64_t zpublic_key_length;
  zpublic_key_length = zPUBLIC_KEY_LENGTH;
  uint64_t zpublic_key_count_value;
  zpublic_key_count_value = (zpublic_keys_length / zpublic_key_length);
  bool z2zE487;
  {
    bool z2zE486;
    {
      uint64_t z2zE484;
      z2zE484 = ztransactions.zcount;
      z2zE486 = (zpublic_key_count_value != z2zE484);
    }
    bool z3zE881;
    if (z2zE486) {  z3zE881 = true;  } else {
      uint64_t z2zE485;
      {    z2zE485 = (zpublic_key_count_value * zpublic_key_length);
      }
      z3zE881 = (zpublic_keys_length != z2zE485);
    }
    z2zE487 = z3zE881;
  }
  unit z3zE882;
  if (z2zE487) {
    struct zexception z2zE488;
    CREATE(zexception)(&z2zE488);
    zInvalidBlock(&z2zE488, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE488);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block.sail:59.8-59.44");
    KILL(zexception)(&z2zE488);
    goto end_block_exception_1369;
    /* unreachable after throw */
    KILL(zexception)(&z2zE488);
  } else {  z3zE882 = UNIT;  }
  uint64_t zgas_limit;
  zgas_limit = zheader_gas_limit;
  uint64_t zexecution_gas_acc;
  zexecution_gas_acc = zGAS_ZERO;
  uint64_t zstate_gas_acc;
  zstate_gas_acc = zGAS_ZERO;
  uint64_t zblob_gas_acc;
  zblob_gas_acc = UINT64_C(0);
  sail_fixed_bytes_20 ztx0_to;
  ztx0_to = zZERO_ADDRESS;
  struct zReceiptAccumulator zreceipts;
  CREATE(zReceiptAccumulator)(&zreceipts);
  zreceipt_accumulator_empty(&zreceipts, UNIT);
  uint64_t zdeposits_start;
  zdeposits_start = zscratch_begin(UNIT);
  struct zBoundedSszzListCursor zcursor;
  zcursor = zsszz_list_cursor(ztransactions);
  struct zByteSliceFields zkeys;
  zkeys = zpublic_keys;
  bool z3zE899;
  unit z3zE900;
while_1364: ;
  {
    bool z2zE489;
    z2zE489 = zsszz_list_cursor_empty(zcursor);
    z3zE899 = not(z2zE489);
    if (!(z3zE899)) goto wend_1365;
    uint64_t zi;
    zi = zcursor.zindex;
    struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 z2zE490;
    {
      z2zE490 = zsszz_list_pop(zcursor);
      if (have_exception) {
        KILL(zReceiptAccumulator)(&zreceipts);
        goto end_block_exception_1369;
      }
    }
    unit z3zE883;
    {
      struct zByteSliceFields ztransaction;
      ztransaction = z2zE490.ztup0;
      struct zBoundedSszzListCursor znext;
      znext = z2zE490.ztup1;
      zcursor = znext;
      unit z3zE884;
      z3zE884 = UNIT;
      struct zByteSliceFields zkeys_fields;
      zkeys_fields = zkeys;
      uint64_t zkeys_length;
      zkeys_length = zkeys_fields.zlen;
      bool z2zE491;
      z2zE491 = (zkeys_length < zpublic_key_length);
      unit z3zE885;
      if (z2zE491) {
        struct zexception z2zE492;
        CREATE(zexception)(&z2zE492);
        zInvalidBlock(&z2zE492, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE492);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block.sail:77.12-77.48");
        KILL(zReceiptAccumulator)(&zreceipts);
        KILL(zexception)(&z2zE492);
        goto end_block_exception_1369;
        /* unreachable after throw */
        KILL(zexception)(&z2zE492);
      } else {  z3zE885 = UNIT;  }
      struct zByteSliceFields zpublic_key;
      zpublic_key = zsub_slice(zkeys_fields, UINT64_C(0), zPUBLIC_KEY_LENGTH);
      zkeys = zslice_suffix(zkeys_fields, zpublic_key_length);
      unit z3zE887;
      z3zE887 = UNIT;
      unit z3zE886;
      z3zE886 = zcycle_scope_start(zSCOPE_TX_DECODE);
      struct zTransaction ztx;
      CREATE(zTransaction)(&ztx);
      {
        zdecode_transaction(&ztx, ztransaction, zpublic_key);
        if (have_exception) {
          KILL(zReceiptAccumulator)(&zreceipts);
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1369;
        }
      }
      unit z3zE890;
      z3zE890 = zcycle_scope_end(zSCOPE_TX_DECODE);
      uint64_t z2zE493;
      {    z2zE493 = (zi + UINT64_C(1));
      }
      unit z3zE889;
      z3zE889 = bal_set_index(z2zE493);
      bool z2zE494;
      z2zE494 = (zi == UINT64_C(0));
      unit z3zE888;
      if (z2zE494) {
        ztx0_to = ztx.zrecipient;
        z3zE888 = UNIT;
      } else {  z3zE888 = UNIT;  }
      uint64_t zavailable_execution_gas;
      {
        zavailable_execution_gas = zremaining_block_gas(zgas_limit, zexecution_gas_acc);
        if (have_exception) {
          KILL(zReceiptAccumulator)(&zreceipts);
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1369;
        }
      }
      uint64_t zavailable_state_gas;
      {
        zavailable_state_gas = zremaining_block_gas(zgas_limit, zstate_gas_acc);
        if (have_exception) {
          KILL(zReceiptAccumulator)(&zreceipts);
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1369;
        }
      }
      uint64_t ztransaction_execution_limit;
      {
        bool z2zE512;
        {
          uint64_t z2zE511;
          z2zE511 = ztx.zgas_limit;
          z2zE512 = (zAMSTERDAM_TX_MAX_GAS < z2zE511);
        }
        if (z2zE512) {  ztransaction_execution_limit = zAMSTERDAM_TX_MAX_GAS;  } else {
          ztransaction_execution_limit = ztx.zgas_limit;
        }
      }
      bool ztransaction_fits;
      {
        bool z2zE507;
        z2zE507 = zfork_gteq(zk_fork, zAmsterdam);
        if (z2zE507) {
          bool z2zE509;
          z2zE509 = (!(zavailable_execution_gas < ztransaction_execution_limit));
          bool z3zE891;
          if (z2zE509) {
            uint64_t z2zE508;
            z2zE508 = ztx.zgas_limit;
            z3zE891 = (!(zavailable_state_gas < z2zE508));
          } else {  z3zE891 = false;  }
          ztransaction_fits = z3zE891;
        } else {
          uint64_t z2zE510;
          z2zE510 = ztx.zgas_limit;
          ztransaction_fits = (!(zavailable_execution_gas < z2zE510));
        }
      }
      bool z2zE495;
      z2zE495 = not(ztransaction_fits);
      if (z2zE495) {
        struct zexception z2zE496;
        CREATE(zexception)(&z2zE496);
        zInvalidBlock(&z2zE496, zGasUsedExceedsLimit);
        COPY(zexception)(current_exception, z2zE496);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block.sail:101.12-101.51");
        KILL(zReceiptAccumulator)(&zreceipts);
        KILL(zTransaction)(&ztx);
        KILL(zexception)(&z2zE496);
        goto end_block_exception_1369;
        /* unreachable after throw */
        KILL(zexception)(&z2zE496);
      } else {
        uint64_t ztx_blob_gas;
        {
          uint64_t z2zE506;
          {
            struct zBlobHashes z2zE505;
            z2zE505 = ztx.zblob_hashes;
            z2zE506 = z2zE505.zcount;
          }
          {
            ztx_blob_gas = ztransaction_blob_gas_for_count(z2zE506);
            if (have_exception) {
              KILL(zReceiptAccumulator)(&zreceipts);
              KILL(zTransaction)(&ztx);
              goto end_block_exception_1369;
            }
          }
        }
        uint64_t znext_blob_gas;
        {
          bool z2zE504;
          z2zE504 = zfork_lt(zk_fork, zCancun);
          if (z2zE504) {  znext_blob_gas = zblob_gas_acc;  } else {
            {
              znext_blob_gas = zchecked_block_blob_gas_add(zblob_gas_acc, ztx_blob_gas);
              if (have_exception) {
                KILL(zReceiptAccumulator)(&zreceipts);
                KILL(zTransaction)(&ztx);
                goto end_block_exception_1369;
              }
            }
          }
        }
        struct zReceipt zreceipt;
        CREATE(zReceipt)(&zreceipt);
        {
          zprocess_transaction(&zreceipt, ztx);
          if (have_exception) {
            KILL(zReceiptAccumulator)(&zreceipts);
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1369;
          }
        }
        uint64_t z2zE497;
        z2zE497 = zreceipt.zexecution_gas;
        {
          zexecution_gas_acc = zconserved_gas_add(zexecution_gas_acc, z2zE497);
          if (have_exception) {
            KILL(zReceiptAccumulator)(&zreceipts);
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1369;
          }
        }
        unit z3zE898;
        z3zE898 = UNIT;
        uint64_t z2zE498;
        z2zE498 = zreceipt.zstate_gas;
        {
          zstate_gas_acc = zconserved_gas_add(zstate_gas_acc, z2zE498);
          if (have_exception) {
            KILL(zReceiptAccumulator)(&zreceipts);
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1369;
          }
        }
        unit z3zE897;
        z3zE897 = UNIT;
        bool z2zE500;
        {
          bool z2zE499;
          z2zE499 = (zgas_limit < zexecution_gas_acc);
          bool z3zE893;
          if (z2zE499) {  z3zE893 = true;  } else {  z3zE893 = (zgas_limit < zstate_gas_acc);  }
          z2zE500 = z3zE893;
        }
        unit z3zE896;
        if (z2zE500) {
          struct zexception z2zE501;
          CREATE(zexception)(&z2zE501);
          zInvalidBlock(&z2zE501, zGasUsedExceedsLimit);
          COPY(zexception)(current_exception, z2zE501);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/block.sail:113.16-113.55");
          KILL(zReceiptAccumulator)(&zreceipts);
          KILL(zTransaction)(&ztx);
          KILL(zReceipt)(&zreceipt);
          KILL(zexception)(&z2zE501);
          goto end_block_exception_1369;
          /* unreachable after throw */
          KILL(zexception)(&z2zE501);
        } else {  z3zE896 = UNIT;  }
        uint64_t z2zE502;
        z2zE502 = znext.zindex;
        {
          zreceipt_accumulator_push(&zreceipts, zreceipts, zreceipt, z2zE502);
          if (have_exception) {
            KILL(zReceiptAccumulator)(&zreceipts);
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1369;
          }
        }
        unit z3zE895;
        z3zE895 = UNIT;
        zz5listz8z5structz0zzLogEntryz9 z2zE503;
        CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE503);
        COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE503, zreceipt.zlogs);
        unit z3zE894;
        {
          z3zE894 = zappend_deposit_logs(z2zE503);
          if (have_exception) {
            KILL(zReceiptAccumulator)(&zreceipts);
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE503);
            goto end_block_exception_1369;
          }
        }
        KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE503);
        zblob_gas_acc = znext_blob_gas;
        z3zE883 = UNIT;
        KILL(zReceipt)(&zreceipt);
      }
      KILL(zTransaction)(&ztx);
      goto finish_match_1366;
    }
  case_1367: ;
    sail_match_failure("execute_block_transactions");
  finish_match_1366: ;
    z3zE900 = z3zE883;
    goto while_1364;
  }
wend_1365: ;
  unit z3zE901;
  z3zE901 = UNIT;
  uint64_t zheader_gas_used;
  {
    bool z2zE516;
    {
      bool z2zE515;
      z2zE515 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE902;
      if (z2zE515) {  z3zE902 = (zexecution_gas_acc < zstate_gas_acc);  } else {  z3zE902 = false;  }
      z2zE516 = z3zE902;
    }
    if (z2zE516) {  zheader_gas_used = zstate_gas_acc;  } else {  zheader_gas_used = zexecution_gas_acc;  }
  }
  unit z3zE903;
  z3zE903 = zcycle_scope_start(zSCOPE_RECEIPTS_ROOT);
  sail_fixed_bytes_32 zreceipts_root;
  {
    zreceipts_root = zreceipt_accumulator_root(zreceipts);
    if (have_exception) {
      KILL(zReceiptAccumulator)(&zreceipts);
      goto end_block_exception_1369;
    }
  }
  unit z3zE904;
  z3zE904 = zcycle_scope_end(zSCOPE_RECEIPTS_ROOT);
  sail_fixed_bytes_256 z2zE513;
  z2zE513 = zreceipts.zbloom;
  struct zByteSliceFields z2zE514;
  z2zE514 = zscratch_finish(zdeposits_start);
  struct zBlockExecutionResult z3zE905;
  z3zE905.zblob_gas_used = zblob_gas_acc;
  z3zE905.zdeposits = z2zE514;
  z3zE905.zexecution_gas_used = zexecution_gas_acc;
  z3zE905.zfirst_tx_recipient = ztx0_to;
  z3zE905.zheader_gas_used = zheader_gas_used;
  z3zE905.zlogs_bloom = z2zE513;
  z3zE905.zreceipts_root = zreceipts_root;
  z3zE905.zrequests = zEMPTY_EXECUTION_REQUESTS;
  z3zE905.zstate_gas_used = zstate_gas_acc;
  z8zE662 = z3zE905;
  KILL(zReceiptAccumulator)(&zreceipts);
end_function_1368: ;
  return z8zE662;
end_block_exception_1369: ;
  struct zByteSliceFields z8zE1105 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1104 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1103 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1102 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1101 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1100 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zExecutionRequests z8zE1099 = { .zbuilder_deposits = z8zE1104, .zbuilder_exits = z8zE1103, .zconsolidations = z8zE1102, .zdeposits = z8zE1101, .zwithdrawals = z8zE1100 };
  struct zBlockExecutionResult z8zE1098 = { .zblob_gas_used = UINT64_C(0xdeadc0de), .zdeposits = z8zE1105, .zexecution_gas_used = UINT64_C(0xdeadc0de), .zfirst_tx_recipient = fixed_bytes_20_zero(), .zheader_gas_used = UINT64_C(0xdeadc0de), .zlogs_bloom = fixed_bytes_256_zero(), .zreceipts_root = fixed_bytes_32_zero(), .zrequests = z8zE1099, .zstate_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE1098;
}

unit zapply_withdrawals(struct zBoundedSszzListRef zwithdrawals)
{
  unit z8zE663;
  struct zBoundedSszzListRef zrest;
  zrest = zwithdrawals;
  bool z3zE879;
  unit z3zE880;
while_1358: ;
  {
    uint64_t z2zE478;
    z2zE478 = zrest.zcount;
    z3zE879 = (z2zE478 != UINT64_C(0));
    if (!(z3zE879)) goto wend_1359;
    struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 z2zE479;
    {
      z2zE479 = zsszz_fixed_list_pop(zrest, zWD_SIZE);
      if (have_exception) {  goto end_block_exception_1363;  }
    }
    unit z3zE876;
    {
      struct zByteSliceFields zwithdrawal_ref;
      zwithdrawal_ref = z2zE479.ztup0;
      struct zBoundedSszzListRef ztail;
      ztail = z2zE479.ztup1;
      zrest = ztail;
      unit z3zE877;
      z3zE877 = UNIT;
      struct zWithdrawal zwithdrawal;
      zwithdrawal = zdecode_withdrawal(zwithdrawal_ref);
      sail_fixed_bytes_20 z2zE482;
      z2zE482 = zwithdrawal.zaddress;
      sail_u128 z2zE483;
      {
        uint64_t z2zE481;
        {
          uint64_t z2zE480;
          z2zE480 = zwithdrawal.zamount;
          z2zE481 = zword_of_withdrawal_amount(z2zE480);
        }
        {
          sail_u256 z3zE3796;
          z3zE3796 = u256_of_fbits(z2zE481);
          sail_u256 z3zE3797;
          z3zE3797 = u256_of_fbits(UINT64_C(1000000000));
          sail_u256 z3zE3798;
          z3zE3798 = zalu_mul(z3zE3796, z3zE3797);
          z2zE483 = u128_of_u256(z3zE3798);
        }
      }
      {
        z3zE876 = zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u128zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE482, z2zE483);
        if (have_exception) {  goto end_block_exception_1363;  }
      }
      goto finish_match_1360;
    }
  case_1361: ;
    sail_match_failure("apply_withdrawals");
  finish_match_1360: ;
    z3zE880 = z3zE876;
    goto while_1358;
  }
wend_1359: ;
  z8zE663 = UNIT;
end_function_1362: ;
  return z8zE663;
end_block_exception_1363: ;

  return UNIT;
}

unit zapply_block_end_state(struct zBlockBody zbody)
{
  unit z8zE664;
  bool z2zE474;
  z2zE474 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE875;
  if (z2zE474) {
    struct zBoundedSszzListRef z2zE475;
    z2zE475 = zbody.zwithdrawals;
    {
      z3zE875 = zapply_withdrawals(z2zE475);
      if (have_exception) {  goto end_block_exception_1357;  }
    }
  } else {  z3zE875 = UNIT;  }
  bool z2zE476;
  z2zE476 = zfork_lt(zk_fork, zParis);
  unit z3zE874;
  if (z2zE476) {
    sail_fixed_bytes_20 z2zE477;
    z2zE477 = zk_coinbase(UNIT);
    {
      z3zE874 = zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE477, zPRE_MERGE_BLOCK_REWARD);
      if (have_exception) {  goto end_block_exception_1357;  }
    }
  } else {  z3zE874 = UNIT;  }
  z8zE664 = zk_tx_merge(UNIT);
end_function_1356: ;
  return z8zE664;
end_block_exception_1357: ;

  return UNIT;
}

struct zBlockExecutionResult zexecute_block_body(struct zBlockBody zbody, struct zByteSliceFields zpublic_keys, uint64_t zheader_gas_limit)
{
  struct zBlockExecutionResult z8zE665;
  unit z3zE865;
  z3zE865 = bal_reset(UNIT);
  unit z3zE864;
  z3zE864 = bal_set_index(UINT64_C(0));
  unit z3zE863;
  z3zE863 = zcycle_scope_start(zSCOPE_BLOCK_START);
  unit z3zE862;
  {
    z3zE862 = zrun_block_start_system_calls(UNIT);
    if (have_exception) {  goto end_block_exception_1355;  }
  }
  unit z3zE861;
  z3zE861 = zcycle_scope_end(zSCOPE_BLOCK_START);
  unit z3zE860;
  z3zE860 = zcycle_scope_start(zSCOPE_BLOCK_TRANSACTIONS);
  struct zBlockExecutionResult zresult;
  {
    struct zBoundedSszzListRef z2zE473;
    z2zE473 = zbody.ztransactions;
    {
      zresult = zexecute_block_transactions(z2zE473, zpublic_keys, zheader_gas_limit);
      if (have_exception) {  goto end_block_exception_1355;  }
    }
  }
  unit z3zE866;
  z3zE866 = zcycle_scope_end(zSCOPE_BLOCK_TRANSACTIONS);
  uint64_t zpost_tx_index;
  {
    uint64_t z2zE472;
    {
      struct zBoundedSszzListRef z2zE471;
      z2zE471 = zbody.ztransactions;
      z2zE472 = z2zE471.zcount;
    }
    {    zpost_tx_index = (z2zE472 + UINT64_C(1));
    }
  }
  unit z3zE871;
  z3zE871 = bal_set_index(zpost_tx_index);
  unit z3zE870;
  z3zE870 = zcycle_scope_start(zSCOPE_BLOCK_END_STATE);
  unit z3zE869;
  {
    z3zE869 = zapply_block_end_state(zbody);
    if (have_exception) {  goto end_block_exception_1355;  }
  }
  unit z3zE868;
  z3zE868 = zcycle_scope_end(zSCOPE_BLOCK_END_STATE);
  unit z3zE867;
  z3zE867 = zcycle_scope_start(zSCOPE_BLOCK_END_REQUESTS);
  struct zExecutionRequests zrequests;
  {
    bool z2zE469;
    z2zE469 = zfork_gteq(zk_fork, zPrague);
    if (z2zE469) {
      struct zByteSliceFields z2zE470;
      z2zE470 = zresult.zdeposits;
      {
        zrequests = zcollect_execution_requests(z2zE470);
        if (have_exception) {  goto end_block_exception_1355;  }
      }
    } else {  zrequests = zEMPTY_EXECUTION_REQUESTS;  }
  }
  unit z3zE872;
  z3zE872 = zcycle_scope_end(zSCOPE_BLOCK_END_REQUESTS);
  struct zBlockExecutionResult z3zE873;
  z3zE873 = zresult;
  z3zE873.zrequests = zrequests;
  z8zE665 = z3zE873;
end_function_1354: ;
  return z8zE665;
end_block_exception_1355: ;
  struct zByteSliceFields z8zE1113 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1112 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1111 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1110 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1109 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1108 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zExecutionRequests z8zE1107 = { .zbuilder_deposits = z8zE1112, .zbuilder_exits = z8zE1111, .zconsolidations = z8zE1110, .zdeposits = z8zE1109, .zwithdrawals = z8zE1108 };
  struct zBlockExecutionResult z8zE1106 = { .zblob_gas_used = UINT64_C(0xdeadc0de), .zdeposits = z8zE1113, .zexecution_gas_used = UINT64_C(0xdeadc0de), .zfirst_tx_recipient = fixed_bytes_20_zero(), .zheader_gas_used = UINT64_C(0xdeadc0de), .zlogs_bloom = fixed_bytes_256_zero(), .zreceipts_root = fixed_bytes_32_zero(), .zrequests = z8zE1107, .zstate_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE1106;
}

