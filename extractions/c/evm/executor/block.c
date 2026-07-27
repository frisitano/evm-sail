/* Generated from sail/executor/block.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_332(void) {

  uint64_t z3zE358;
  z3zE358 = UINT64_C(2000000000000000000);
  zPRE_MERGE_BLOCK_REWARD = z3zE358;
let_end_1039: ;
}
void kill_letbind_332(void) {
}

uint64_t zremaining_block_gas(uint64_t zlimit, uint64_t zused)
{
  uint64_t z8zE679;
  bool z2zE235;
  z2zE235 = (!(zlimit < zused));
  if (z2zE235) {
    {    z8zE679 = (zlimit - zused);
    }
  } else {
    struct zexception z2zE236;
    CREATE(zexception)(&z2zE236);
    zInvalidBlock(&z2zE236, zGasUsedExceedsLimit);
    COPY(zexception)(current_exception, z2zE236);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block.sail:30.8-30.47");
    KILL(zexception)(&z2zE236);
    goto end_block_exception_1172;
    /* unreachable after throw */
    KILL(zexception)(&z2zE236);
  }
end_function_1171: ;
  return z8zE679;
end_block_exception_1172: ;

  return UINT64_C(0xdeadc0de);
}

unit zrun_block_start_system_calls(unit z3zE579)
{
  unit z8zE680;
  bool z2zE231;
  z2zE231 = zfork_gteq(zk_fork, zCancun);
  unit z3zE580;
  if (z2zE231) {
    sail_fixed_bytes_32 z2zE232;
    z2zE232 = zk_header.zparent_beacon_block_root;
    {
      z3zE580 = zsystem_call(zBEACON_ROOTS_ADDR, z2zE232);
      if (have_exception) {  goto end_block_exception_1170;  }
    }
  } else {  z3zE580 = UNIT;  }
  bool z2zE233;
  z2zE233 = zfork_gteq(zk_fork, zPrague);
  if (z2zE233) {
    sail_fixed_bytes_32 z2zE234;
    z2zE234 = zk_header.zparent_hash;
    {
      z8zE680 = zsystem_call(zHISTORY_STORAGE_ADDR, z2zE234);
      if (have_exception) {  goto end_block_exception_1170;  }
    }
  } else {  z8zE680 = UNIT;  }
end_function_1169: ;
  return z8zE680;
end_block_exception_1170: ;

  return UNIT;
}

struct zBlockExecutionResult zexecute_block_transactions(struct zBoundedSszzListRef ztransactions, struct zByteSliceFields zpublic_keys, uint64_t zheader_gas_limit)
{
  struct zBlockExecutionResult z8zE681;
  uint64_t zpublic_keys_length;
  zpublic_keys_length = zpublic_keys.zlen;
  uint64_t zpublic_key_length;
  zpublic_key_length = zPUBLIC_KEY_LENGTH;
  uint64_t zpublic_key_count_value;
  zpublic_key_count_value = (zpublic_keys_length / zpublic_key_length);
  bool z2zE202;
  {
    bool z2zE201;
    {
      uint64_t z2zE199;
      z2zE199 = ztransactions.zcount;
      z2zE201 = (zpublic_key_count_value != z2zE199);
    }
    bool z3zE554;
    if (z2zE201) {  z3zE554 = true;  } else {
      uint64_t z2zE200;
      {    z2zE200 = (zpublic_key_count_value * zpublic_key_length);
      }
      z3zE554 = (zpublic_keys_length != z2zE200);
    }
    z2zE202 = z3zE554;
  }
  unit z3zE555;
  if (z2zE202) {
    struct zexception z2zE203;
    CREATE(zexception)(&z2zE203);
    zInvalidBlock(&z2zE203, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE203);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block.sail:59.8-59.44");
    KILL(zexception)(&z2zE203);
    goto end_block_exception_1168;
    /* unreachable after throw */
    KILL(zexception)(&z2zE203);
  } else {  z3zE555 = UNIT;  }
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
  zreceipts = zreceipt_accumulator_empty(UNIT);
  uint64_t zdeposits_start;
  zdeposits_start = zscratch_begin(UNIT);
  struct zBoundedSszzListCursor zcursor;
  zcursor = zsszz_list_cursor(ztransactions);
  struct zByteSliceFields zkeys;
  zkeys = zpublic_keys;
  bool z3zE572;
  unit z3zE573;
while_1163: ;
  {
    bool z2zE204;
    z2zE204 = zsszz_list_cursor_empty(zcursor);
    z3zE572 = not(z2zE204);
    if (!(z3zE572)) goto wend_1164;
    uint64_t zi;
    zi = zcursor.zindex;
    struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 z2zE205;
    {
      z2zE205 = zsszz_list_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1168;  }
    }
    unit z3zE556;
    {
      struct zByteSliceFields ztransaction;
      ztransaction = z2zE205.ztup0;
      struct zBoundedSszzListCursor znext;
      znext = z2zE205.ztup1;
      zcursor = znext;
      unit z3zE557;
      z3zE557 = UNIT;
      struct zByteSliceFields zkeys_fields;
      zkeys_fields = zkeys;
      uint64_t zkeys_length;
      zkeys_length = zkeys_fields.zlen;
      bool z2zE206;
      z2zE206 = (zkeys_length < zpublic_key_length);
      unit z3zE558;
      if (z2zE206) {
        struct zexception z2zE207;
        CREATE(zexception)(&z2zE207);
        zInvalidBlock(&z2zE207, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE207);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block.sail:77.12-77.48");
        KILL(zexception)(&z2zE207);
        goto end_block_exception_1168;
        /* unreachable after throw */
        KILL(zexception)(&z2zE207);
      } else {  z3zE558 = UNIT;  }
      struct zByteSliceFields zpublic_key;
      zpublic_key = zsub_slice(zkeys_fields, UINT64_C(0), zPUBLIC_KEY_LENGTH);
      zkeys = zslice_suffix(zkeys_fields, zpublic_key_length);
      unit z3zE560;
      z3zE560 = UNIT;
      unit z3zE559;
      z3zE559 = zcycle_scope_start(zSCOPE_TX_DECODE);
      struct zTransaction ztx;
      CREATE(zTransaction)(&ztx);
      {
        zdecode_transaction(&ztx, ztransaction, zpublic_key);
        if (have_exception) {
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1168;
        }
      }
      unit z3zE563;
      z3zE563 = zcycle_scope_end(zSCOPE_TX_DECODE);
      {    zk_block_access_index = (zi + UINT64_C(1));
      }
      unit z3zE562;
      z3zE562 = UNIT;
      bool z2zE208;
      z2zE208 = (zi == UINT64_C(0));
      unit z3zE561;
      if (z2zE208) {
        ztx0_to = ztx.zrecipient;
        z3zE561 = UNIT;
      } else {  z3zE561 = UNIT;  }
      uint64_t zavailable_execution_gas;
      {
        zavailable_execution_gas = zremaining_block_gas(zgas_limit, zexecution_gas_acc);
        if (have_exception) {
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1168;
        }
      }
      uint64_t zavailable_state_gas;
      {
        zavailable_state_gas = zremaining_block_gas(zgas_limit, zstate_gas_acc);
        if (have_exception) {
          KILL(zTransaction)(&ztx);
          goto end_block_exception_1168;
        }
      }
      uint64_t ztransaction_execution_limit;
      {
        bool z2zE226;
        {
          uint64_t z2zE225;
          z2zE225 = ztx.zgas_limit;
          z2zE226 = (zAMSTERDAM_TX_MAX_GAS < z2zE225);
        }
        if (z2zE226) {  ztransaction_execution_limit = zAMSTERDAM_TX_MAX_GAS;  } else {
          ztransaction_execution_limit = ztx.zgas_limit;
        }
      }
      bool ztransaction_fits;
      {
        bool z2zE221;
        z2zE221 = zfork_gteq(zk_fork, zAmsterdam);
        if (z2zE221) {
          bool z2zE223;
          z2zE223 = (!(zavailable_execution_gas < ztransaction_execution_limit));
          bool z3zE564;
          if (z2zE223) {
            uint64_t z2zE222;
            z2zE222 = ztx.zgas_limit;
            z3zE564 = (!(zavailable_state_gas < z2zE222));
          } else {  z3zE564 = false;  }
          ztransaction_fits = z3zE564;
        } else {
          uint64_t z2zE224;
          z2zE224 = ztx.zgas_limit;
          ztransaction_fits = (!(zavailable_execution_gas < z2zE224));
        }
      }
      bool z2zE209;
      z2zE209 = not(ztransaction_fits);
      if (z2zE209) {
        struct zexception z2zE210;
        CREATE(zexception)(&z2zE210);
        zInvalidBlock(&z2zE210, zGasUsedExceedsLimit);
        COPY(zexception)(current_exception, z2zE210);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block.sail:101.12-101.51");
        KILL(zTransaction)(&ztx);
        KILL(zexception)(&z2zE210);
        goto end_block_exception_1168;
        /* unreachable after throw */
        KILL(zexception)(&z2zE210);
      } else {
        uint64_t ztx_blob_gas;
        {
          uint64_t z2zE220;
          {
            struct zBlobHashes z2zE219;
            z2zE219 = ztx.zblob_hashes;
            z2zE220 = z2zE219.zcount;
          }
          {
            ztx_blob_gas = ztransaction_blob_gas_for_count(z2zE220);
            if (have_exception) {
              KILL(zTransaction)(&ztx);
              goto end_block_exception_1168;
            }
          }
        }
        uint64_t znext_blob_gas;
        {
          bool z2zE218;
          z2zE218 = zfork_lt(zk_fork, zCancun);
          if (z2zE218) {  znext_blob_gas = zblob_gas_acc;  } else {
            {
              znext_blob_gas = zchecked_block_blob_gas_add(zblob_gas_acc, ztx_blob_gas);
              if (have_exception) {
                KILL(zTransaction)(&ztx);
                goto end_block_exception_1168;
              }
            }
          }
        }
        struct zReceipt zreceipt;
        CREATE(zReceipt)(&zreceipt);
        {
          zprocess_transaction(&zreceipt, ztx);
          if (have_exception) {
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1168;
          }
        }
        uint64_t z2zE211;
        z2zE211 = zreceipt.zexecution_gas;
        {
          zexecution_gas_acc = zconserved_gas_add(zexecution_gas_acc, z2zE211);
          if (have_exception) {
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1168;
          }
        }
        unit z3zE571;
        z3zE571 = UNIT;
        uint64_t z2zE212;
        z2zE212 = zreceipt.zstate_gas;
        {
          zstate_gas_acc = zconserved_gas_add(zstate_gas_acc, z2zE212);
          if (have_exception) {
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1168;
          }
        }
        unit z3zE570;
        z3zE570 = UNIT;
        bool z2zE214;
        {
          bool z2zE213;
          z2zE213 = (zgas_limit < zexecution_gas_acc);
          bool z3zE566;
          if (z2zE213) {  z3zE566 = true;  } else {  z3zE566 = (zgas_limit < zstate_gas_acc);  }
          z2zE214 = z3zE566;
        }
        unit z3zE569;
        if (z2zE214) {
          struct zexception z2zE215;
          CREATE(zexception)(&z2zE215);
          zInvalidBlock(&z2zE215, zGasUsedExceedsLimit);
          COPY(zexception)(current_exception, z2zE215);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/block.sail:113.16-113.55");
          KILL(zTransaction)(&ztx);
          KILL(zReceipt)(&zreceipt);
          KILL(zexception)(&z2zE215);
          goto end_block_exception_1168;
          /* unreachable after throw */
          KILL(zexception)(&z2zE215);
        } else {  z3zE569 = UNIT;  }
        uint64_t z2zE216;
        z2zE216 = znext.zindex;
        {
          zreceipts = zreceipt_accumulator_push(zreceipts, zreceipt, z2zE216);
          if (have_exception) {
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            goto end_block_exception_1168;
          }
        }
        unit z3zE568;
        z3zE568 = UNIT;
        zz5listz8z5structz0zzLogEntryz9 z2zE217;
        CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE217);
        COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE217, zreceipt.zlogs);
        unit z3zE567;
        {
          z3zE567 = zappend_deposit_logs(z2zE217);
          if (have_exception) {
            KILL(zTransaction)(&ztx);
            KILL(zReceipt)(&zreceipt);
            KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE217);
            goto end_block_exception_1168;
          }
        }
        KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE217);
        zblob_gas_acc = znext_blob_gas;
        z3zE556 = UNIT;
        KILL(zReceipt)(&zreceipt);
      }
      KILL(zTransaction)(&ztx);
      goto finish_match_1165;
    }
  case_1166: ;
    sail_match_failure("execute_block_transactions");
  finish_match_1165: ;
    z3zE573 = z3zE556;
    goto while_1163;
  }
wend_1164: ;
  unit z3zE574;
  z3zE574 = UNIT;
  uint64_t zheader_gas_used;
  {
    bool z2zE230;
    {
      bool z2zE229;
      z2zE229 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE575;
      if (z2zE229) {  z3zE575 = (zexecution_gas_acc < zstate_gas_acc);  } else {  z3zE575 = false;  }
      z2zE230 = z3zE575;
    }
    if (z2zE230) {  zheader_gas_used = zstate_gas_acc;  } else {  zheader_gas_used = zexecution_gas_acc;  }
  }
  unit z3zE576;
  z3zE576 = zcycle_scope_start(zSCOPE_RECEIPTS_ROOT);
  sail_fixed_bytes_32 zreceipts_root;
  {
    zreceipts_root = zreceipt_accumulator_root(zreceipts);
    if (have_exception) {  goto end_block_exception_1168;  }
  }
  unit z3zE577;
  z3zE577 = zcycle_scope_end(zSCOPE_RECEIPTS_ROOT);
  sail_fixed_bytes_256 z2zE227;
  z2zE227 = zreceipts.zbloom;
  struct zByteSliceFields z2zE228;
  z2zE228 = zscratch_finish(zdeposits_start);
  struct zBlockExecutionResult z3zE578;
  z3zE578.zblob_gas_used = zblob_gas_acc;
  z3zE578.zdeposits = z2zE228;
  z3zE578.zexecution_gas_used = zexecution_gas_acc;
  z3zE578.zfirst_tx_recipient = ztx0_to;
  z3zE578.zheader_gas_used = zheader_gas_used;
  z3zE578.zlogs_bloom = z2zE227;
  z3zE578.zreceipts_root = zreceipts_root;
  z3zE578.zrequests = zEMPTY_EXECUTION_REQUESTS;
  z3zE578.zstate_gas_used = zstate_gas_acc;
  z8zE681 = z3zE578;
end_function_1167: ;
  return z8zE681;
end_block_exception_1168: ;
  struct zByteSliceFields z8zE1013 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1012 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1011 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1010 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1009 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1008 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zExecutionRequests z8zE1007 = { .zbuilder_deposits = z8zE1012, .zbuilder_exits = z8zE1011, .zconsolidations = z8zE1010, .zdeposits = z8zE1009, .zwithdrawals = z8zE1008 };
  struct zBlockExecutionResult z8zE1006 = { .zblob_gas_used = UINT64_C(0xdeadc0de), .zdeposits = z8zE1013, .zexecution_gas_used = UINT64_C(0xdeadc0de), .zfirst_tx_recipient = fixed_bytes_20_zero(), .zheader_gas_used = UINT64_C(0xdeadc0de), .zlogs_bloom = fixed_bytes_256_zero(), .zreceipts_root = fixed_bytes_32_zero(), .zrequests = z8zE1007, .zstate_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE1006;
}

unit zapply_withdrawals(struct zBoundedSszzListRef zwithdrawals)
{
  unit z8zE682;
  struct zBoundedSszzListRef zrest;
  zrest = zwithdrawals;
  bool z3zE552;
  unit z3zE553;
while_1157: ;
  {
    uint64_t z2zE193;
    z2zE193 = zrest.zcount;
    z3zE552 = (z2zE193 != UINT64_C(0));
    if (!(z3zE552)) goto wend_1158;
    struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 z2zE194;
    {
      z2zE194 = zsszz_fixed_list_pop(zrest, zWD_SIZE);
      if (have_exception) {  goto end_block_exception_1162;  }
    }
    unit z3zE549;
    {
      struct zByteSliceFields zwithdrawal_ref;
      zwithdrawal_ref = z2zE194.ztup0;
      struct zBoundedSszzListRef ztail;
      ztail = z2zE194.ztup1;
      zrest = ztail;
      unit z3zE550;
      z3zE550 = UNIT;
      struct zWithdrawal zwithdrawal;
      zwithdrawal = zdecode_withdrawal(zwithdrawal_ref);
      sail_fixed_bytes_20 z2zE197;
      z2zE197 = zwithdrawal.zaddress;
      sail_u128 z2zE198;
      {
        uint64_t z2zE196;
        {
          uint64_t z2zE195;
          z2zE195 = zwithdrawal.zamount;
          z2zE196 = zword_of_withdrawal_amount(z2zE195);
        }
        {
          sail_u256 z3zE3048;
          z3zE3048 = u256_of_fbits(z2zE196);
          sail_u256 z3zE3049;
          z3zE3049 = u256_of_fbits(UINT64_C(1000000000));
          sail_u256 z3zE3050;
          z3zE3050 = zalu_mul(z3zE3048, z3zE3049);
          z2zE198 = u128_of_u256(z3zE3050);
        }
      }
      {
        z3zE549 = zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u128zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE197, z2zE198);
        if (have_exception) {  goto end_block_exception_1162;  }
      }
      goto finish_match_1159;
    }
  case_1160: ;
    sail_match_failure("apply_withdrawals");
  finish_match_1159: ;
    z3zE553 = z3zE549;
    goto while_1157;
  }
wend_1158: ;
  z8zE682 = UNIT;
end_function_1161: ;
  return z8zE682;
end_block_exception_1162: ;

  return UNIT;
}

unit zapply_block_end_state(struct zBlockBody zbody)
{
  unit z8zE683;
  bool z2zE189;
  z2zE189 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE548;
  if (z2zE189) {
    struct zBoundedSszzListRef z2zE190;
    z2zE190 = zbody.zwithdrawals;
    {
      z3zE548 = zapply_withdrawals(z2zE190);
      if (have_exception) {  goto end_block_exception_1156;  }
    }
  } else {  z3zE548 = UNIT;  }
  bool z2zE191;
  z2zE191 = zfork_lt(zk_fork, zParis);
  unit z3zE547;
  if (z2zE191) {
    sail_fixed_bytes_20 z2zE192;
    z2zE192 = zk_coinbase(UNIT);
    {
      z3zE547 = zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE192, zPRE_MERGE_BLOCK_REWARD);
      if (have_exception) {  goto end_block_exception_1156;  }
    }
  } else {  z3zE547 = UNIT;  }
  z8zE683 = zk_tx_merge(UNIT);
end_function_1155: ;
  return z8zE683;
end_block_exception_1156: ;

  return UNIT;
}

struct zBlockExecutionResult zexecute_block_body(struct zBlockBody zbody, struct zByteSliceFields zpublic_keys, uint64_t zheader_gas_limit)
{
  struct zBlockExecutionResult z8zE684;
  unit z3zE538;
  z3zE538 = bal_reset(UNIT);
  zk_block_access_index = UINT64_C(0);
  unit z3zE537;
  z3zE537 = UNIT;
  unit z3zE536;
  z3zE536 = zcycle_scope_start(zSCOPE_BLOCK_START);
  unit z3zE535;
  {
    z3zE535 = zrun_block_start_system_calls(UNIT);
    if (have_exception) {  goto end_block_exception_1154;  }
  }
  unit z3zE534;
  z3zE534 = zcycle_scope_end(zSCOPE_BLOCK_START);
  unit z3zE533;
  z3zE533 = zcycle_scope_start(zSCOPE_BLOCK_TRANSACTIONS);
  struct zBlockExecutionResult zresult;
  {
    struct zBoundedSszzListRef z2zE188;
    z2zE188 = zbody.ztransactions;
    {
      zresult = zexecute_block_transactions(z2zE188, zpublic_keys, zheader_gas_limit);
      if (have_exception) {  goto end_block_exception_1154;  }
    }
  }
  unit z3zE539;
  z3zE539 = zcycle_scope_end(zSCOPE_BLOCK_TRANSACTIONS);
  uint64_t zpost_tx_index;
  {
    uint64_t z2zE187;
    {
      struct zBoundedSszzListRef z2zE186;
      z2zE186 = zbody.ztransactions;
      z2zE187 = z2zE186.zcount;
    }
    {    zpost_tx_index = (z2zE187 + UINT64_C(1));
    }
  }
  zk_block_access_index = zpost_tx_index;
  unit z3zE544;
  z3zE544 = UNIT;
  unit z3zE543;
  z3zE543 = zcycle_scope_start(zSCOPE_BLOCK_END_STATE);
  unit z3zE542;
  {
    z3zE542 = zapply_block_end_state(zbody);
    if (have_exception) {  goto end_block_exception_1154;  }
  }
  unit z3zE541;
  z3zE541 = zcycle_scope_end(zSCOPE_BLOCK_END_STATE);
  unit z3zE540;
  z3zE540 = zcycle_scope_start(zSCOPE_BLOCK_END_REQUESTS);
  struct zExecutionRequests zrequests;
  {
    bool z2zE184;
    z2zE184 = zfork_gteq(zk_fork, zPrague);
    if (z2zE184) {
      struct zByteSliceFields z2zE185;
      z2zE185 = zresult.zdeposits;
      {
        zrequests = zcollect_execution_requests(z2zE185);
        if (have_exception) {  goto end_block_exception_1154;  }
      }
    } else {  zrequests = zEMPTY_EXECUTION_REQUESTS;  }
  }
  unit z3zE545;
  z3zE545 = zcycle_scope_end(zSCOPE_BLOCK_END_REQUESTS);
  struct zBlockExecutionResult z3zE546;
  z3zE546 = zresult;
  z3zE546.zrequests = zrequests;
  z8zE684 = z3zE546;
end_function_1153: ;
  return z8zE684;
end_block_exception_1154: ;
  struct zByteSliceFields z8zE1021 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1020 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1019 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1018 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1017 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zByteSliceFields z8zE1016 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zExecutionRequests z8zE1015 = { .zbuilder_deposits = z8zE1020, .zbuilder_exits = z8zE1019, .zconsolidations = z8zE1018, .zdeposits = z8zE1017, .zwithdrawals = z8zE1016 };
  struct zBlockExecutionResult z8zE1014 = { .zblob_gas_used = UINT64_C(0xdeadc0de), .zdeposits = z8zE1021, .zexecution_gas_used = UINT64_C(0xdeadc0de), .zfirst_tx_recipient = fixed_bytes_20_zero(), .zheader_gas_used = UINT64_C(0xdeadc0de), .zlogs_bloom = fixed_bytes_256_zero(), .zreceipts_root = fixed_bytes_32_zero(), .zrequests = z8zE1015, .zstate_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE1014;
}

