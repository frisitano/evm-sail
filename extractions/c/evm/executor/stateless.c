/* Generated from sail/executor/stateless.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zvalidate_executed_block(struct zBlock zblock, struct zStatelessInputRef zinput_ref, struct zBlockExecutionResult zresult)
{
  unit z8zE676;
  struct zBlockHeader zheader;
  zheader = zblock.zheader;
  bool z2zE278;
  z2zE278 = zfork_gteq(zk_fork, zPrague);
  unit z3zE770;
  if (z2zE278) {
    bool z2zE313;
    {
      bool z2zE312;
      {
        bool z2zE284;
        {
          zz5listz8z5unionz0zzBytesz9 z2zE282;
          CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE282);
          {
            struct zBytes z2zE281;
            CREATE(zBytes)(&z2zE281);
            {
              struct zByteSliceFields z2zE280;
              {
                struct zExecutionRequests z2zE279;
                z2zE279 = zresult.zrequests;
                z2zE280 = z2zE279.zdeposits;
              }
              zBytesSlice(&z2zE281, z2zE280);
            }
            zconsz3z5unionz0zzBytes(&z2zE282, z2zE281, z2zE282);
            KILL(zBytes)(&z2zE281);
          }
          struct zByteSliceFields z2zE283;
          z2zE283 = zinput_ref.zdeposits;
          z2zE284 = host_bytes_segments_equal_slice(z2zE282, z2zE283);
          KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE282);
        }
        z2zE312 = not(z2zE284);
      }
      bool z3zE779;
      if (z2zE312) {  z3zE779 = true;  } else {
        bool z2zE311;
        {
          bool z2zE290;
          {
            zz5listz8z5unionz0zzBytesz9 z2zE288;
            CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE288);
            {
              struct zBytes z2zE287;
              CREATE(zBytes)(&z2zE287);
              {
                struct zByteSliceFields z2zE286;
                {
                  struct zExecutionRequests z2zE285;
                  z2zE285 = zresult.zrequests;
                  z2zE286 = z2zE285.zwithdrawals;
                }
                zBytesSlice(&z2zE287, z2zE286);
              }
              zconsz3z5unionz0zzBytes(&z2zE288, z2zE287, z2zE288);
              KILL(zBytes)(&z2zE287);
            }
            struct zByteSliceFields z2zE289;
            z2zE289 = zinput_ref.zwithdrawal_requests;
            z2zE290 = host_bytes_segments_equal_slice(z2zE288, z2zE289);
            KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE288);
          }
          z2zE311 = not(z2zE290);
        }
        bool z3zE778;
        if (z2zE311) {  z3zE778 = true;  } else {
          bool z2zE310;
          {
            bool z2zE296;
            {
              zz5listz8z5unionz0zzBytesz9 z2zE294;
              CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE294);
              {
                struct zBytes z2zE293;
                CREATE(zBytes)(&z2zE293);
                {
                  struct zByteSliceFields z2zE292;
                  {
                    struct zExecutionRequests z2zE291;
                    z2zE291 = zresult.zrequests;
                    z2zE292 = z2zE291.zconsolidations;
                  }
                  zBytesSlice(&z2zE293, z2zE292);
                }
                zconsz3z5unionz0zzBytes(&z2zE294, z2zE293, z2zE294);
                KILL(zBytes)(&z2zE293);
              }
              struct zByteSliceFields z2zE295;
              z2zE295 = zinput_ref.zconsolidation_requests;
              z2zE296 = host_bytes_segments_equal_slice(z2zE294, z2zE295);
              KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE294);
            }
            z2zE310 = not(z2zE296);
          }
          bool z3zE777;
          if (z2zE310) {  z3zE777 = true;  } else {
            bool z2zE309;
            {
              bool z2zE302;
              {
                zz5listz8z5unionz0zzBytesz9 z2zE300;
                CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE300);
                {
                  struct zBytes z2zE299;
                  CREATE(zBytes)(&z2zE299);
                  {
                    struct zByteSliceFields z2zE298;
                    {
                      struct zExecutionRequests z2zE297;
                      z2zE297 = zresult.zrequests;
                      z2zE298 = z2zE297.zbuilder_deposits;
                    }
                    zBytesSlice(&z2zE299, z2zE298);
                  }
                  zconsz3z5unionz0zzBytes(&z2zE300, z2zE299, z2zE300);
                  KILL(zBytes)(&z2zE299);
                }
                struct zByteSliceFields z2zE301;
                z2zE301 = zinput_ref.zbuilder_deposit_requests;
                z2zE302 = host_bytes_segments_equal_slice(z2zE300, z2zE301);
                KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE300);
              }
              z2zE309 = not(z2zE302);
            }
            bool z3zE776;
            if (z2zE309) {  z3zE776 = true;  } else {
              bool z2zE308;
              {
                zz5listz8z5unionz0zzBytesz9 z2zE306;
                CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE306);
                {
                  struct zBytes z2zE305;
                  CREATE(zBytes)(&z2zE305);
                  {
                    struct zByteSliceFields z2zE304;
                    {
                      struct zExecutionRequests z2zE303;
                      z2zE303 = zresult.zrequests;
                      z2zE304 = z2zE303.zbuilder_exits;
                    }
                    zBytesSlice(&z2zE305, z2zE304);
                  }
                  zconsz3z5unionz0zzBytes(&z2zE306, z2zE305, z2zE306);
                  KILL(zBytes)(&z2zE305);
                }
                struct zByteSliceFields z2zE307;
                z2zE307 = zinput_ref.zbuilder_exit_requests;
                z2zE308 = host_bytes_segments_equal_slice(z2zE306, z2zE307);
                KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE306);
              }
              z3zE776 = not(z2zE308);
            }
            z3zE777 = z3zE776;
          }
          z3zE778 = z3zE777;
        }
        z3zE779 = z3zE778;
      }
      z2zE313 = z3zE779;
    }
    if (z2zE313) {
      struct zexception z2zE314;
      CREATE(zexception)(&z2zE314);
      zInvalidBlock(&z2zE314, zInvalidExecutionRequests);
      COPY(zexception)(current_exception, z2zE314);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/stateless.sail:40.12-40.56");
      KILL(zexception)(&z2zE314);
      goto end_block_exception_1334;
      /* unreachable after throw */
      KILL(zexception)(&z2zE314);
    } else {  z3zE770 = UNIT;  }
  } else {  z3zE770 = UNIT;  }
  bool z2zE317;
  {
    uint64_t z2zE315;
    z2zE315 = zresult.zheader_gas_used;
    uint64_t z2zE316;
    z2zE316 = zheader.zgas_used;
    z2zE317 = (z2zE315 != z2zE316);
  }
  unit z3zE769;
  if (z2zE317) {
    struct zexception z2zE318;
    CREATE(zexception)(&z2zE318);
    zInvalidBlock(&z2zE318, zInvalidGasUsed);
    COPY(zexception)(current_exception, z2zE318);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:44.8-44.42");
    KILL(zexception)(&z2zE318);
    goto end_block_exception_1334;
    /* unreachable after throw */
    KILL(zexception)(&z2zE318);
  } else {  z3zE769 = UNIT;  }
  bool z2zE322;
  {
    bool z2zE321;
    z2zE321 = zfork_gteq(zk_fork, zCancun);
    bool z3zE766;
    if (z2zE321) {
      uint64_t z2zE319;
      z2zE319 = zresult.zblob_gas_used;
      uint64_t z2zE320;
      z2zE320 = zheader.zblob_gas_used;
      z3zE766 = (z2zE319 != z2zE320);
    } else {  z3zE766 = false;  }
    z2zE322 = z3zE766;
  }
  unit z3zE768;
  if (z2zE322) {
    struct zexception z2zE323;
    CREATE(zexception)(&z2zE323);
    zInvalidBlock(&z2zE323, zInvalidBlobGasUsed);
    COPY(zexception)(current_exception, z2zE323);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:47.8-47.46");
    KILL(zexception)(&z2zE323);
    goto end_block_exception_1334;
    /* unreachable after throw */
    KILL(zexception)(&z2zE323);
  } else {  z3zE768 = UNIT;  }
  unit z3zE767;
  z3zE767 = zcycle_scope_start(zSCOPE_STATE_ROOT);
  sail_fixed_bytes_32 zpoststate;
  {
    zpoststate = zcompute_state_root(UNIT);
    if (have_exception) {  goto end_block_exception_1334;  }
  }
  unit z3zE783;
  z3zE783 = zcycle_scope_end(zSCOPE_STATE_ROOT);
  bool z2zE325;
  {
    sail_fixed_bytes_32 z2zE324;
    z2zE324 = zheader.zstate_root;
    z2zE325 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(zpoststate, z2zE324);
  }
  unit z3zE782;
  if (z2zE325) {
    struct zexception z2zE326;
    CREATE(zexception)(&z2zE326);
    zInvalidBlock(&z2zE326, zInvalidStateRoot);
    COPY(zexception)(current_exception, z2zE326);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:53.8-53.44");
    KILL(zexception)(&z2zE326);
    goto end_block_exception_1334;
    /* unreachable after throw */
    KILL(zexception)(&z2zE326);
  } else {  z3zE782 = UNIT;  }
  bool z2zE329;
  {
    sail_fixed_bytes_32 z2zE327;
    z2zE327 = zresult.zreceipts_root;
    sail_fixed_bytes_32 z2zE328;
    z2zE328 = zheader.zreceipts_root;
    z2zE329 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE327, z2zE328);
  }
  unit z3zE781;
  if (z2zE329) {
    struct zexception z2zE330;
    CREATE(zexception)(&z2zE330);
    zInvalidBlock(&z2zE330, zInvalidReceiptsRoot);
    COPY(zexception)(current_exception, z2zE330);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:56.8-56.47");
    KILL(zexception)(&z2zE330);
    goto end_block_exception_1334;
    /* unreachable after throw */
    KILL(zexception)(&z2zE330);
  } else {  z3zE781 = UNIT;  }
  bool z2zE334;
  {
    bool z2zE333;
    {
      sail_fixed_bytes_256 z2zE331;
      z2zE331 = zresult.zlogs_bloom;
      sail_fixed_bytes_256 z2zE332;
      z2zE332 = zheader.zlogs_bloom;
      z2zE333 = zlogs_bloom_equal(z2zE331, z2zE332);
    }
    z2zE334 = not(z2zE333);
  }
  unit z3zE780;
  if (z2zE334) {
    struct zexception z2zE335;
    CREATE(zexception)(&z2zE335);
    zInvalidBlock(&z2zE335, zInvalidLogsBloom);
    COPY(zexception)(current_exception, z2zE335);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/stateless.sail:59.8-59.44");
    KILL(zexception)(&z2zE335);
    goto end_block_exception_1334;
    /* unreachable after throw */
    KILL(zexception)(&z2zE335);
  } else {  z3zE780 = UNIT;  }
  bool z2zE336;
  z2zE336 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE336) {
    unit z3zE784;
    z3zE784 = zcycle_scope_start(zSCOPE_BLOCK_ACCESS_LIST);
    struct zEncodedBlockAccessList zblock_access_list;
    {
      zblock_access_list = zencode_block_access_list(UNIT);
      if (have_exception) {  goto end_block_exception_1334;  }
    }
    unit z3zE785;
    z3zE785 = zcycle_scope_end(zSCOPE_BLOCK_ACCESS_LIST);
    uint64_t zmaximum_items;
    {
      uint64_t z2zE348;
      z2zE348 = zheader.zgas_limit;
      zmaximum_items = (z2zE348 / UINT64_C(2000));
    }
    bool z2zE338;
    {
      uint64_t z2zE337;
      z2zE337 = zblock_access_list.zitem_count;
      z2zE338 = (zmaximum_items < z2zE337);
    }
    unit z3zE786;
    if (z2zE338) {
      struct zexception z2zE339;
      CREATE(zexception)(&z2zE339);
      zInvalidBlock(&z2zE339, zBlockAccessListTooLarge);
      COPY(zexception)(current_exception, z2zE339);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/stateless.sail:67.12-67.55");
      KILL(zexception)(&z2zE339);
      goto end_block_exception_1334;
      /* unreachable after throw */
      KILL(zexception)(&z2zE339);
    } else {  z3zE786 = UNIT;  }
    bool z2zE346;
    {
      bool z2zE345;
      {
        zz5listz8z5unionz0zzBytesz9 z2zE343;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE343);
        {
          struct zBytes z2zE341;
          CREATE(zBytes)(&z2zE341);
          {
            struct zByteSliceFields z2zE340;
            z2zE340 = zblock_access_list.zbytes;
            zBytesSlice(&z2zE341, z2zE340);
          }
          zconsz3z5unionz0zzBytes(&z2zE343, z2zE341, z2zE343);
          KILL(zBytes)(&z2zE341);
        }
        struct zByteSliceFields z2zE344;
        {
          struct zBlockBody z2zE342;
          z2zE342 = zblock.zbody;
          z2zE344 = z2zE342.zblock_access_list;
        }
        z2zE345 = host_bytes_segments_equal_slice(z2zE343, z2zE344);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE343);
      }
      z2zE346 = not(z2zE345);
    }
    if (z2zE346) {
      struct zexception z2zE347;
      CREATE(zexception)(&z2zE347);
      zInvalidBlock(&z2zE347, zInvalidBlockAccessList);
      COPY(zexception)(current_exception, z2zE347);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/stateless.sail:70.12-70.54");
      KILL(zexception)(&z2zE347);
      goto end_block_exception_1334;
      /* unreachable after throw */
      KILL(zexception)(&z2zE347);
    } else {  z8zE676 = UNIT;  }
  } else {  z8zE676 = UNIT;  }
end_function_1333: ;
  return z8zE676;
end_block_exception_1334: ;

  return UNIT;
}

void zverify_stateless_payload(struct zStatelessValidationResult *z8zE677, struct zStatelessInputRef zinput_ref)
{
  uint64_t zactive_scope;
  zactive_scope = zSCOPE_DECODE_INPUT;
  struct zStatelessValidationResult z3zE762;
  CREATE(zStatelessValidationResult)(&z3zE762);
  { /* try */
    unit z3zE745;
    z3zE745 = zcycle_scope_start(zactive_scope);
    unit z3zE744;
    z3zE744 = zscratch_reset(UNIT);
    struct zStatelessInput zinput;
    {
      zinput = zdecode_stateless_input(zinput_ref);
      if (have_exception) {  goto end_block_exception_1331;  }
    }
    struct zProtocolProfile z2zE272;
    z2zE272 = zinput_ref.zprotocol;
    zk_fork = z2zE272.zfork;
    unit z3zE749;
    z3zE749 = UNIT;
    unit z3zE748;
    z3zE748 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_INDEX_WITNESS;
    unit z3zE747;
    z3zE747 = UNIT;
    unit z3zE746;
    z3zE746 = zcycle_scope_start(zactive_scope);
    struct zWitnessContext zwitness;
    {
      zwitness = zindex_execution_witness(zinput_ref);
      if (have_exception) {  goto end_block_exception_1331;  }
    }
    unit z3zE756;
    z3zE756 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_VALIDATE_PAYLOAD;
    unit z3zE755;
    z3zE755 = UNIT;
    unit z3zE754;
    z3zE754 = zcycle_scope_start(zactive_scope);
    unit z3zE753;
    {
      z3zE753 = zvalidate_execution_payload(zinput, zinput_ref, zwitness);
      if (have_exception) {  goto end_block_exception_1331;  }
    }
    unit z3zE752;
    z3zE752 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_EXECUTE_BLOCK;
    unit z3zE751;
    z3zE751 = UNIT;
    unit z3zE750;
    z3zE750 = zcycle_scope_start(zactive_scope);
    struct zBlock zblock;
    {
      struct zExecutionPayload z2zE277;
      z2zE277 = zinput.zpayload;
      zblock = z2zE277.zblock;
    }
    struct zBlockExecutionResult zresult;
    {
      struct zBlockBody z2zE274;
      z2zE274 = zblock.zbody;
      struct zByteSliceFields z2zE275;
      z2zE275 = zinput_ref.zpublic_keys;
      uint64_t z2zE276;
      {
        struct zBlockHeader z2zE273;
        z2zE273 = zblock.zheader;
        z2zE276 = z2zE273.zgas_limit;
      }
      {
        zresult = zexecute_block_body(z2zE274, z2zE275, z2zE276);
        if (have_exception) {  goto end_block_exception_1331;  }
      }
    }
    unit z3zE761;
    z3zE761 = zcycle_scope_end(zactive_scope);
    zactive_scope = zSCOPE_VALIDATE_RESULT;
    unit z3zE760;
    z3zE760 = UNIT;
    unit z3zE759;
    z3zE759 = zcycle_scope_start(zactive_scope);
    unit z3zE758;
    {
      z3zE758 = zvalidate_executed_block(zblock, zinput_ref, zresult);
      if (have_exception) {  goto end_block_exception_1331;  }
    }
    unit z3zE757;
    z3zE757 = zcycle_scope_end(zactive_scope);
    zStatelessPayloadValid(&z3zE762, UNIT);
  end_block_exception_1331: ;
  }
  if (!(have_exception)) goto post_exception_handlers_1328;
  have_exception = false;
  {
    if ((*current_exception).kind != Kind_zInvalidBlock) goto try_1329;
    enum zBlockError zreason;
    zreason = (*current_exception).variants.zInvalidBlock;
    unit z3zE763;
    z3zE763 = zcycle_scope_end(zactive_scope);
    struct zStatelessValidationFailure z3zE764;
    z3zE764.zreason = zreason;
    z3zE764.zscope = zactive_scope;
    zStatelessPayloadInvalid(&z3zE762, z3zE764);
    goto post_exception_handlers_1328;
  }
try_1329: ;
  have_exception = true;
post_exception_handlers_1328: ;
  COPY(zStatelessValidationResult)((*(&z8zE677)), z3zE762);
end_function_1330: ;
  goto end_function_4016;
end_block_exception_1332: ;
  goto end_function_4016;
end_function_4016: ;
}

