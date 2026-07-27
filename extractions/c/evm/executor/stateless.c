/* Generated from sail/executor/stateless.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zvalidate_executed_block(struct zBlock zblock, struct zStatelessInputRef zinput_ref, struct zBlockExecutionResult zresult)
{
  unit z8zE691;
  struct zBlockHeader zheader;
  zheader = zblock.zheader;
  bool z2zE18;
  z2zE18 = zfork_gteq(zk_fork, zPrague);
  unit z3zE464;
  if (z2zE18) {
    bool z2zE53;
    {
      bool z2zE52;
      {
        bool z2zE24;
        {
          zz5listz8z5unionz0zzBytesz9 z2zE22;
          CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE22);
          {
            struct zBytes z2zE21;
            CREATE(zBytes)(&z2zE21);
            {
              struct zByteSliceFields z2zE20;
              {
                struct zExecutionRequests z2zE19;
                z2zE19 = zresult.zrequests;
                z2zE20 = z2zE19.zdeposits;
              }
              zBytesSlice(&z2zE21, z2zE20);
            }
            zconsz3z5unionz0zzBytes(&z2zE22, z2zE21, z2zE22);
            KILL(zBytes)(&z2zE21);
          }
          struct zByteSliceFields z2zE23;
          z2zE23 = zinput_ref.zdeposits;
          z2zE24 = host_bytes_segments_equal_slice(z2zE22, z2zE23);
          KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE22);
        }
        z2zE52 = not(z2zE24);
      }
      bool z3zE473;
      if (z2zE52) {  z3zE473 = true;  } else {
        bool z2zE51;
        {
          bool z2zE30;
          {
            zz5listz8z5unionz0zzBytesz9 z2zE28;
            CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE28);
            {
              struct zBytes z2zE27;
              CREATE(zBytes)(&z2zE27);
              {
                struct zByteSliceFields z2zE26;
                {
                  struct zExecutionRequests z2zE25;
                  z2zE25 = zresult.zrequests;
                  z2zE26 = z2zE25.zwithdrawals;
                }
                zBytesSlice(&z2zE27, z2zE26);
              }
              zconsz3z5unionz0zzBytes(&z2zE28, z2zE27, z2zE28);
              KILL(zBytes)(&z2zE27);
            }
            struct zByteSliceFields z2zE29;
            z2zE29 = zinput_ref.zwithdrawal_requests;
            z2zE30 = host_bytes_segments_equal_slice(z2zE28, z2zE29);
            KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE28);
          }
          z2zE51 = not(z2zE30);
        }
        bool z3zE472;
        if (z2zE51) {  z3zE472 = true;  } else {
          bool z2zE50;
          {
            bool z2zE36;
            {
              zz5listz8z5unionz0zzBytesz9 z2zE34;
              CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE34);
              {
                struct zBytes z2zE33;
                CREATE(zBytes)(&z2zE33);
                {
                  struct zByteSliceFields z2zE32;
                  {
                    struct zExecutionRequests z2zE31;
                    z2zE31 = zresult.zrequests;
                    z2zE32 = z2zE31.zconsolidations;
                  }
                  zBytesSlice(&z2zE33, z2zE32);
                }
                zconsz3z5unionz0zzBytes(&z2zE34, z2zE33, z2zE34);
                KILL(zBytes)(&z2zE33);
              }
              struct zByteSliceFields z2zE35;
              z2zE35 = zinput_ref.zconsolidation_requests;
              z2zE36 = host_bytes_segments_equal_slice(z2zE34, z2zE35);
              KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE34);
            }
            z2zE50 = not(z2zE36);
          }
          bool z3zE471;
          if (z2zE50) {  z3zE471 = true;  } else {
            bool z2zE49;
            {
              bool z2zE42;
              {
                zz5listz8z5unionz0zzBytesz9 z2zE40;
                CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE40);
                {
                  struct zBytes z2zE39;
                  CREATE(zBytes)(&z2zE39);
                  {
                    struct zByteSliceFields z2zE38;
                    {
                      struct zExecutionRequests z2zE37;
                      z2zE37 = zresult.zrequests;
                      z2zE38 = z2zE37.zbuilder_deposits;
                    }
                    zBytesSlice(&z2zE39, z2zE38);
                  }
                  zconsz3z5unionz0zzBytes(&z2zE40, z2zE39, z2zE40);
                  KILL(zBytes)(&z2zE39);
                }
                struct zByteSliceFields z2zE41;
                z2zE41 = zinput_ref.zbuilder_deposit_requests;
                z2zE42 = host_bytes_segments_equal_slice(z2zE40, z2zE41);
                KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE40);
              }
              z2zE49 = not(z2zE42);
            }
            bool z3zE470;
            if (z2zE49) {  z3zE470 = true;  } else {
              bool z2zE48;
              {
                zz5listz8z5unionz0zzBytesz9 z2zE46;
                CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE46);
                {
                  struct zBytes z2zE45;
                  CREATE(zBytes)(&z2zE45);
                  {
                    struct zByteSliceFields z2zE44;
                    {
                      struct zExecutionRequests z2zE43;
                      z2zE43 = zresult.zrequests;
                      z2zE44 = z2zE43.zbuilder_exits;
                    }
                    zBytesSlice(&z2zE45, z2zE44);
                  }
                  zconsz3z5unionz0zzBytes(&z2zE46, z2zE45, z2zE46);
                  KILL(zBytes)(&z2zE45);
                }
                struct zByteSliceFields z2zE47;
                z2zE47 = zinput_ref.zbuilder_exit_requests;
                z2zE48 = host_bytes_segments_equal_slice(z2zE46, z2zE47);
                KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE46);
              }
              z3zE470 = not(z2zE48);
            }
            z3zE471 = z3zE470;
          }
          z3zE472 = z3zE471;
        }
        z3zE473 = z3zE472;
      }
      z2zE53 = z3zE473;
    }
    if (z2zE53) {
      struct zexception z2zE54;
      CREATE(zexception)(&z2zE54);
      zInvalidBlock(&z2zE54, zInvalidExecutionRequests);
      COPY(zexception)(current_exception, z2zE54);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/stateless.sail:40.12-40.56");
      KILL(zexception)(&z2zE54);
      goto end_block_exception_1140;
      /* unreachable after throw */
      KILL(zexception)(&z2zE54);
    } else {  z3zE464 = UNIT;  }
  } else {  z3zE464 = UNIT;  }
  bool z2zE57;
  {
    uint64_t z2zE55;
    z2zE55 = zresult.zheader_gas_used;
    uint64_t z2zE56;
    z2zE56 = zheader.zgas_used;
    z2zE57 = (z2zE55 != z2zE56);
  }
  unit z3zE463;
  if (z2zE57) {
    struct zexception z2zE58;
    CREATE(zexception)(&z2zE58);
    zInvalidBlock(&z2zE58, zInvalidGasUsed);
    COPY(zexception)(current_exception, z2zE58);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:44.8-44.42");
    KILL(zexception)(&z2zE58);
    goto end_block_exception_1140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE58);
  } else {  z3zE463 = UNIT;  }
  bool z2zE62;
  {
    bool z2zE61;
    z2zE61 = zfork_gteq(zk_fork, zCancun);
    bool z3zE460;
    if (z2zE61) {
      uint64_t z2zE59;
      z2zE59 = zresult.zblob_gas_used;
      uint64_t z2zE60;
      z2zE60 = zheader.zblob_gas_used;
      z3zE460 = (z2zE59 != z2zE60);
    } else {  z3zE460 = false;  }
    z2zE62 = z3zE460;
  }
  unit z3zE462;
  if (z2zE62) {
    struct zexception z2zE63;
    CREATE(zexception)(&z2zE63);
    zInvalidBlock(&z2zE63, zInvalidBlobGasUsed);
    COPY(zexception)(current_exception, z2zE63);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:47.8-47.46");
    KILL(zexception)(&z2zE63);
    goto end_block_exception_1140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE63);
  } else {  z3zE462 = UNIT;  }
  unit z3zE461;
  z3zE461 = zcycle_scope_start(zSCOPE_STATE_ROOT);
  sail_fixed_bytes_32 zpoststate;
  {
    zpoststate = zcompute_state_root(UNIT);
    if (have_exception) {  goto end_block_exception_1140;  }
  }
  unit z3zE477;
  z3zE477 = zcycle_scope_end(zSCOPE_STATE_ROOT);
  bool z2zE65;
  {
    sail_fixed_bytes_32 z2zE64;
    z2zE64 = zheader.zstate_root;
    z2zE65 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(zpoststate, z2zE64);
  }
  unit z3zE476;
  if (z2zE65) {
    struct zexception z2zE66;
    CREATE(zexception)(&z2zE66);
    zInvalidBlock(&z2zE66, zInvalidStateRoot);
    COPY(zexception)(current_exception, z2zE66);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:53.8-53.44");
    KILL(zexception)(&z2zE66);
    goto end_block_exception_1140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE66);
  } else {  z3zE476 = UNIT;  }
  bool z2zE69;
  {
    sail_fixed_bytes_32 z2zE67;
    z2zE67 = zresult.zreceipts_root;
    sail_fixed_bytes_32 z2zE68;
    z2zE68 = zheader.zreceipts_root;
    z2zE69 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE67, z2zE68);
  }
  unit z3zE475;
  if (z2zE69) {
    struct zexception z2zE70;
    CREATE(zexception)(&z2zE70);
    zInvalidBlock(&z2zE70, zInvalidReceiptsRoot);
    COPY(zexception)(current_exception, z2zE70);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:56.8-56.47");
    KILL(zexception)(&z2zE70);
    goto end_block_exception_1140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE70);
  } else {  z3zE475 = UNIT;  }
  bool z2zE74;
  {
    bool z2zE73;
    {
      sail_fixed_bytes_256 z2zE71;
      z2zE71 = zresult.zlogs_bloom;
      sail_fixed_bytes_256 z2zE72;
      z2zE72 = zheader.zlogs_bloom;
      z2zE73 = zlogs_bloom_equal(z2zE71, z2zE72);
    }
    z2zE74 = not(z2zE73);
  }
  unit z3zE474;
  if (z2zE74) {
    struct zexception z2zE75;
    CREATE(zexception)(&z2zE75);
    zInvalidBlock(&z2zE75, zInvalidLogsBloom);
    COPY(zexception)(current_exception, z2zE75);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:59.8-59.44");
    KILL(zexception)(&z2zE75);
    goto end_block_exception_1140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE75);
  } else {  z3zE474 = UNIT;  }
  bool z2zE76;
  z2zE76 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE76) {
    unit z3zE478;
    z3zE478 = zcycle_scope_start(zSCOPE_BLOCK_ACCESS_LIST);
    uint64_t zmaximum_items;
    {
      uint64_t z2zE79;
      z2zE79 = zheader.zgas_limit;
      zmaximum_items = (z2zE79 / UINT64_C(2000));
    }
    struct zByteSliceFields z2zE78;
    {
      struct zBlockBody z2zE77;
      z2zE77 = zblock.zbody;
      z2zE78 = z2zE77.zblock_access_list;
    }
    unit z3zE479;
    {
      z3zE479 = zvalidate_block_access_list(z2zE78, zmaximum_items);
      if (have_exception) {  goto end_block_exception_1140;  }
    }
    z8zE691 = zcycle_scope_end(zSCOPE_BLOCK_ACCESS_LIST);
  } else {  z8zE691 = UNIT;  }
end_function_1139: ;
  return z8zE691;
end_block_exception_1140: ;

  return UNIT;
}

void zverify_stateless_payload(struct zStatelessValidationResult *z8zE692, struct zStatelessInputRef zinput_ref)
{
  uint64_t zactive_scope;
  zactive_scope = zSCOPE_DECODE_INPUT;
  struct zStatelessValidationResult z3zE456;
  CREATE(zStatelessValidationResult)(&z3zE456);
  { /* try */
    unit z3zE439;
    z3zE439 = zcycle_scope_start(zactive_scope);
    unit z3zE438;
    z3zE438 = zscratch_reset(UNIT);
    struct zStatelessInput zinput;
    {
      zinput = zdecode_stateless_input(zinput_ref);
      if (have_exception) {  goto end_block_exception_1137;  }
    }
    struct zProtocolProfile z2zE12;
    z2zE12 = zinput_ref.zprotocol;
    zk_fork = z2zE12.zfork;
    unit z3zE443;
    z3zE443 = UNIT;
    unit z3zE442;
    z3zE442 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_INDEX_WITNESS;
    unit z3zE441;
    z3zE441 = UNIT;
    unit z3zE440;
    z3zE440 = zcycle_scope_start(zactive_scope);
    struct zWitnessContext zwitness;
    {
      zwitness = zindex_execution_witness(zinput_ref);
      if (have_exception) {  goto end_block_exception_1137;  }
    }
    unit z3zE450;
    z3zE450 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_VALIDATE_PAYLOAD;
    unit z3zE449;
    z3zE449 = UNIT;
    unit z3zE448;
    z3zE448 = zcycle_scope_start(zactive_scope);
    unit z3zE447;
    {
      z3zE447 = zvalidate_execution_payload(zinput, zinput_ref, zwitness);
      if (have_exception) {  goto end_block_exception_1137;  }
    }
    unit z3zE446;
    z3zE446 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_EXECUTE_BLOCK;
    unit z3zE445;
    z3zE445 = UNIT;
    unit z3zE444;
    z3zE444 = zcycle_scope_start(zactive_scope);
    struct zBlock zblock;
    {
      struct zExecutionPayload z2zE17;
      z2zE17 = zinput.zpayload;
      zblock = z2zE17.zblock;
    }
    struct zBlockExecutionResult zresult;
    {
      struct zBlockBody z2zE14;
      z2zE14 = zblock.zbody;
      struct zByteSliceFields z2zE15;
      z2zE15 = zinput_ref.zpublic_keys;
      uint64_t z2zE16;
      {
        struct zBlockHeader z2zE13;
        z2zE13 = zblock.zheader;
        z2zE16 = z2zE13.zgas_limit;
      }
      {
        zresult = zexecute_block_body(z2zE14, z2zE15, z2zE16);
        if (have_exception) {  goto end_block_exception_1137;  }
      }
    }
    unit z3zE455;
    z3zE455 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_VALIDATE_RESULT;
    unit z3zE454;
    z3zE454 = UNIT;
    unit z3zE453;
    z3zE453 = zcycle_scope_start(zactive_scope);
    unit z3zE452;
    {
      z3zE452 = zvalidate_executed_block(zblock, zinput_ref, zresult);
      if (have_exception) {  goto end_block_exception_1137;  }
    }
    unit z3zE451;
    z3zE451 = zcycle_scope_end(zactive_scope);
    zStatelessPayloadValid(&z3zE456, UNIT);
  end_block_exception_1137: ;
  }
  if (!(have_exception)) goto post_exception_handlers_1134;
  have_exception = false;
  {
    if ((*current_exception).kind != Kind_zInvalidBlock) goto try_1135;
    enum zBlockError zreason;
    zreason = (*current_exception).variants.zInvalidBlock;
    unit z3zE457;
    z3zE457 = zcycle_scope_end(zactive_scope);
    struct zStatelessValidationFailure z3zE458;
    z3zE458.zreason = zreason;
    z3zE458.zscope = zactive_scope;
    zStatelessPayloadInvalid(&z3zE456, z3zE458);
    goto post_exception_handlers_1134;
  }
try_1135: ;
  have_exception = true;
post_exception_handlers_1134: ;
  (*(z8zE692)) = z3zE456;
end_function_1136: ;
  goto end_function_3587;
end_block_exception_1138: ;
  goto end_function_3587;
end_function_3587: ;
}

