/* Generated from sail/evm/machine.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_118(void) {

  uint64_t z3zE137;
  z3zE137 = UINT64_C(1024);
  zDEPTH_LIMIT = z3zE137;
let_end_391: ;
}
void kill_letbind_118(void) {
}

__int128 zvalidated_refund_add(__int128 zleft, __int128 zright)
{
  __int128 z8zE260;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2411;
  {
    bool z2zE2410;
    z2zE2410 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2362;
    if (z2zE2410) {
      z3zE2362 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2362 = false;  }
    z2zE2411 = z3zE2362;
  }
  if (z2zE2411) {  z8zE260 = ztotal;  } else {
    struct zexception z2zE2412;
    CREATE(zexception)(&z2zE2412);
    zInvalidBlock(&z2zE2412, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2412);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:58.8-58.44");
    KILL(zexception)(&z2zE2412);
    goto end_block_exception_2724;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2412);
  }
end_function_2723: ;
  return z8zE260;
end_block_exception_2724: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

unit zrecord_refund(__int128 zdelta)
{
  unit z8zE261;
  {
    zframe_refund = zvalidated_refund_add(zframe_refund, zdelta);
    if (have_exception) {  goto end_block_exception_2722;  }
  }
  z8zE261 = UNIT;
end_function_2721: ;
  return z8zE261;
end_block_exception_2722: ;

  return UNIT;
}

uint64_t zframe_code_len(unit z3zE2361)
{
  uint64_t z8zE262;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zlength;
  zlength = zcode.zlen;
  z8zE262 = zlength;
end_function_2719: ;
  return z8zE262;
end_block_exception_2720: ;

  return UINT64_C(0xdeadc0de);
}

bool zframe_jumpdest_valid(uint64_t zdest)
{
  bool z8zE263;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zlength;
  zlength = zcode.zlen;
  uint64_t z2zE2409;
  z2zE2409 = zframe_code.zjumpdests;
  z8zE263 = jumpdest_ref_contains(z2zE2409, zlength, zdest);
end_function_2717: ;
  return z8zE263;
end_block_exception_2718: ;

  return false;
}

void create_letbind_119(void) {

  uint64_t z3zE138;
  z3zE138 = UINT64_C(1024);
  zSTACK_LIMIT = z3zE138;
let_end_396: ;
}
void kill_letbind_119(void) {
}

uint64_t zconserved_gas_add(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE264;
  bool z2zE2407;
  {
    uint64_t z2zE2406;
    {    z2zE2406 = (UINT64_C(18446744073709551615) - zleft);
    }
    z2zE2407 = (!(z2zE2406 < zright));
  }
  if (z2zE2407) {
    {    z8zE264 = (zleft + zright);
    }
  } else {
    struct zexception z2zE2408;
    CREATE(zexception)(&z2zE2408);
    zInvalidBlock(&z2zE2408, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2408);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:444.8-444.44");
    KILL(zexception)(&z2zE2408);
    goto end_block_exception_2716;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2408);
  }
end_function_2715: ;
  return z8zE264;
end_block_exception_2716: ;

  return UINT64_C(0xdeadc0de);
}

unit zrefill_frame_state_gas(unit z3zE2358)
{
  unit z8zE265;
  bool z2zE2405;
  z2zE2405 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2405) {
    {
      zgas_remaining = zconserved_gas_add(zgas_remaining, zstate_gas_spilled);
      if (have_exception) {  goto end_block_exception_2714;  }
    }
    unit z3zE2360;
    z3zE2360 = UNIT;
    zstate_gas_remaining = zmessage.zstate_gas_reservoir;
    unit z3zE2359;
    z3zE2359 = UNIT;
    zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
    z8zE265 = UNIT;
  } else {  z8zE265 = UNIT;  }
end_function_2713: ;
  return z8zE265;
end_block_exception_2714: ;

  return UNIT;
}

__int128 zframe_state_gas_used(unit z3zE2354)
{
  __int128 z8zE266;
  uint64_t zentry;
  zentry = zmessage.zstate_gas_reservoir;
  uint64_t zremaining;
  zremaining = zstate_gas_remaining;
  uint64_t zspilled;
  zspilled = zstate_gas_spilled;
  __int128 z2zE2404;
  {
    __int128 z3zE2355;
    z3zE2355 = (__int128)(zentry);
    __int128 z3zE2356;
    z3zE2356 = (__int128)(zremaining);
    z2zE2404 = (z3zE2355 - z3zE2356);
  }
  {
    __int128 z3zE2357;
    z3zE2357 = (__int128)(zspilled);
    z8zE266 = (z2zE2404 + z3zE2357);
  }
end_function_2711: ;
  return z8zE266;
end_block_exception_2712: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

unit zexc_halt(enum zExceptionKind zk)
{
  unit z8zE267;
  unit z3zE2353;
  {
    z3zE2353 = zrefill_frame_state_gas(UNIT);
    if (have_exception) {  goto end_block_exception_2710;  }
  }
  zgas_remaining = zGAS_ZERO;
  unit z3zE2352;
  z3zE2352 = UNIT;
  zExceptional(&zframe_status, zk);
  z8zE267 = UNIT;
end_function_2709: ;
  return z8zE267;
end_block_exception_2710: ;

  return UNIT;
}

uint64_t zstack_height(unit z3zE2350)
{
  uint64_t z8zE268;
  uint64_t zheight;
  zheight = stack_depth(UNIT);
  bool z2zE2403;
  z2zE2403 = (!(zSTACK_LIMIT < zheight));
  unit z3zE2351;
  z3zE2351 = sail_assert(z2zE2403, "sail/evm/machine.sail:131.32-131.33");
  z8zE268 = zheight;
end_function_2707: ;
  return z8zE268;
end_block_exception_2708: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zpeek(uint64_t zn)
{
  sail_u256 z8zE269;
  z8zE269 = stack_peek_word(zn);
end_function_2705: ;
  return z8zE269;
end_block_exception_2706: ;

  return u256_zero();
}

unit zpush_word(sail_u256 zw)
{
  unit z8zE270;
  bool z2zE2402;
  {
    uint64_t z2zE2401;
    z2zE2401 = zstack_height(UNIT);
    z2zE2402 = (!(z2zE2401 < zSTACK_LIMIT));
  }
  if (z2zE2402) {
    {
      z8zE270 = zexc_halt(zStackOverflow);
      if (have_exception) {  goto end_block_exception_2704;  }
    }
  } else {  z8zE270 = stack_push_word(zw);  }
end_function_2703: ;
  return z8zE270;
end_block_exception_2704: ;

  return UNIT;
}

unit zpush_gas(uint64_t zvalue)
{
  unit z8zE271;
  sail_u256 z2zE2400;
  z2zE2400 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  {
    z8zE271 = zpush_word(z2zE2400);
    if (have_exception) {  goto end_block_exception_2702;  }
  }
end_function_2701: ;
  return z8zE271;
end_block_exception_2702: ;

  return UNIT;
}

sail_u256 zpop(unit z3zE2348)
{
  sail_u256 z8zE272;
  bool z2zE2399;
  {
    uint64_t z2zE2398;
    z2zE2398 = zstack_height(UNIT);
    z2zE2399 = (z2zE2398 == UINT64_C(0));
  }
  if (z2zE2399) {
    unit z3zE2349;
    {
      z3zE2349 = zexc_halt(zStackUnderflow);
      if (have_exception) {  goto end_block_exception_2700;  }
    }
    z8zE272 = zWORD_ZERO;
  } else {  z8zE272 = stack_pop_word(UNIT);  }
end_function_2699: ;
  return z8zE272;
end_block_exception_2700: ;

  return u256_zero();
}

unit zstack_set(uint64_t zn, sail_u256 zw)
{
  unit z8zE273;
  z8zE273 = stack_set_word(zn, zw);
end_function_2697: ;
  return z8zE273;
end_block_exception_2698: ;

  return UNIT;
}

bool zis_running(unit z3zE2344)
{
  bool z8zE274;
  bool z3zE2345;
  {
    if (zframe_status.kind != Kind_zRunning) goto case_2694;
    z3zE2345 = true;
    goto finish_match_2692;
  }
case_2694: ;
  {
    z3zE2345 = false;
    goto finish_match_2692;
  }
case_2693: ;
finish_match_2692: ;
  z8zE274 = z3zE2345;
end_function_2695: ;
  return z8zE274;
end_block_exception_2696: ;

  return false;
}

unit zcalldata_install(struct zByteSliceFields zdata)
{
  unit z8zE275;
  zcalldata = zdata;
  z8zE275 = UNIT;
end_function_2690: ;
  return z8zE275;
end_block_exception_2691: ;

  return UNIT;
}

unit zreturndata_clear(unit z3zE2343)
{
  unit z8zE276;
  zreturndata = zEMPTY_SLICE;
  z8zE276 = UNIT;
end_function_2688: ;
  return z8zE276;
end_block_exception_2689: ;

  return UNIT;
}

uint64_t zreturndata_sizze(unit z3zE2342)
{
  uint64_t z8zE277;
  struct zByteSliceFields zdata;
  zdata = zreturndata;
  z8zE277 = zdata.zlen;
end_function_2686: ;
  return z8zE277;
end_block_exception_2687: ;

  return UINT64_C(0xdeadc0de);
}

unit zreturndata_copy(uint64_t zdst, uint64_t zoff, uint64_t zlen)
{
  unit z8zE278;
  z8zE278 = zslice_copy(zreturndata, zdst, zoff, zlen);
end_function_2684: ;
  return z8zE278;
end_block_exception_2685: ;

  return UNIT;
}

unit zreturndata_copy_prefix(uint64_t zdst, uint64_t zwant)
{
  unit z8zE279;
  uint64_t zwanted;
  zwanted = zwant;
  uint64_t zavailable;
  zavailable = zreturndata_sizze(UNIT);
  uint64_t zcopy_length;
  {
    bool z2zE2397;
    z2zE2397 = (zwanted < zavailable);
    if (z2zE2397) {  zcopy_length = zwanted;  } else {  zcopy_length = zavailable;  }
  }
  z8zE279 = zslice_copy(zreturndata, zdst, UINT64_C(0), zcopy_length);
end_function_2682: ;
  return z8zE279;
end_block_exception_2683: ;

  return UNIT;
}

uint64_t zreturndata_remaining(uint64_t zavailable, uint64_t zoffset)
{
  uint64_t z8zE280;
  {    z8zE280 = (zavailable - zoffset);
  }
end_function_2680: ;
  return z8zE280;
end_block_exception_2681: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidated_returndata_copy(uint64_t zdst, sail_u256 zsource_offset, sail_u256 zlength)
{
  unit z8zE281;
  uint64_t z2zE2394;
  z2zE2394 = zreturndata_sizze(UNIT);
  unit z3zE2340;
  {
    uint64_t zavailable;
    zavailable = z2zE2394;
    bool z2zE2395;
    z2zE2395 = (!u64_lt_u256(zavailable, zsource_offset));
    if (z2zE2395) {
      uint64_t zremaining;
      {
        uint64_t z3zE3021;
        z3zE3021 = u256_to_u64(zsource_offset);
        zremaining = zreturndata_remaining(zavailable, z3zE3021);
      }
      uint64_t zbounded_source_offset;
      zbounded_source_offset = u256_to_u64(zsource_offset);
      bool z2zE2396;
      z2zE2396 = (!u64_lt_u256(zremaining, zlength));
      if (z2zE2396) {
        uint64_t zbounded_length;
        zbounded_length = u256_to_u64(zlength);
        z3zE2340 = zreturndata_copy(zdst, zbounded_source_offset, zbounded_length);
      } else {
        {
          z3zE2340 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2679;  }
        }
      }
    } else {
      {
        z3zE2340 = zexc_halt(zInvalidOpcode);
        if (have_exception) {  goto end_block_exception_2679;  }
      }
    }
    goto finish_match_2676;
  }
case_2677: ;
  sail_match_failure("validated_returndata_copy");
finish_match_2676: ;
  z8zE281 = z3zE2340;
end_function_2678: ;
  return z8zE281;
end_block_exception_2679: ;

  return UNIT;
}

unit zreturndata_copy_words(uint64_t zdst, sail_u256 zsource_offset, sail_u256 zlength)
{
  unit z8zE282;
  {
    z8zE282 = zvalidated_returndata_copy(zdst, zsource_offset, zlength);
    if (have_exception) {  goto end_block_exception_2675;  }
  }
end_function_2674: ;
  return z8zE282;
end_block_exception_2675: ;

  return UNIT;
}

uint64_t zevm_memory_high_water(unit z3zE2339)
{
  uint64_t z8zE283;
  struct zByteSliceFields zmemory;
  zmemory = zevm_memory;
  uint64_t zlength;
  zlength = zmemory.zlen;
  z8zE283 = zlength;
end_function_2672: ;
  return z8zE283;
end_block_exception_2673: ;

  return UINT64_C(0xdeadc0de);
}

unit zmemory_reset(unit z3zE2337)
{
  unit z8zE284;
  unit z3zE2338;
  z3zE2338 = mem_clear(UNIT);
  zevm_memory = zbyte_slice(zEvmMemorySource, UINT64_C(0), UINT64_C(0));
  z8zE284 = UNIT;
end_function_2670: ;
  return z8zE284;
end_block_exception_2671: ;

  return UNIT;
}

struct zByteSliceFields zmemory_expand_to(uint64_t znew_sizze)
{
  struct zByteSliceFields z8zE285;
  struct zByteSliceFields zmemory;
  zmemory = zevm_memory;
  bool z2zE2393;
  {
    uint64_t z2zE2392;
    z2zE2392 = zmemory.zlen;
    z2zE2393 = (z2zE2392 < znew_sizze);
  }
  if (z2zE2393) {
    struct zByteSliceFields zexpanded;
    zexpanded = mem_expand(znew_sizze);
    zevm_memory = zexpanded;
    unit z3zE2336;
    z3zE2336 = UNIT;
    z8zE285 = zexpanded;
  } else {  z8zE285 = zsub_slice(zmemory, UINT64_C(0), znew_sizze);  }
end_function_2668: ;
  return z8zE285;
end_block_exception_2669: ;
  struct zByteSliceFields z8zE813 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE813;
}

struct zByteSliceFields zmemory_byte_slice(uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE286;
  bool z2zE2389;
  z2zE2389 = (zlen == UINT64_C(0));
  if (z2zE2389) {  z8zE286 = zEMPTY_SLICE;  } else {
    struct zByteSliceFields z2zE2391;
    {
      uint64_t z2zE2390;
      {    z2zE2390 = (zoff + zlen);
      }
      z2zE2391 = zmemory_expand_to(z2zE2390);
    }
    z8zE286 = zsub_slice(z2zE2391, zoff, zlen);
  }
end_function_2666: ;
  return z8zE286;
end_block_exception_2667: ;
  struct zByteSliceFields z8zE814 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE814;
}

struct zByteSliceFields zmemory_code_slice(uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE287;
  bool z2zE2385;
  z2zE2385 = (zlen == UINT64_C(0));
  if (z2zE2385) {  z8zE287 = zEMPTY_CODE_SLICE;  } else {
    struct zByteSliceFields z2zE2388;
    {
      struct zByteSliceFields z2zE2387;
      {
        uint64_t z2zE2386;
        {    z2zE2386 = (zoff + zlen);
        }
        z2zE2387 = zmemory_expand_to(z2zE2386);
      }
      z2zE2388 = zsub_slice(z2zE2387, zoff, zlen);
    }
    z8zE287 = zcode_slice(z2zE2388);
  }
end_function_2664: ;
  return z8zE287;
end_block_exception_2665: ;
  struct zByteSliceFields z8zE815 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE815;
}

struct zByteSliceFields zmemory_frame_enter(unit z3zE2334)
{
  struct zByteSliceFields z8zE288;
  struct zByteSliceFields zparent;
  zparent = zevm_memory;
  uint64_t zbase;
  zbase = mem_frame_enter(UNIT);
  zevm_memory = zbyte_slice(zEvmMemorySource, zbase, UINT64_C(0));
  unit z3zE2335;
  z3zE2335 = UNIT;
  z8zE288 = zparent;
end_function_2662: ;
  return z8zE288;
end_block_exception_2663: ;
  struct zByteSliceFields z8zE816 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE816;
}

unit zmemory_frame_leave(struct zByteSliceFields zparent)
{
  unit z8zE289;
  unit z3zE2333;
  z3zE2333 = mem_frame_leave(UNIT);
  zevm_memory = zparent;
  z8zE289 = UNIT;
end_function_2660: ;
  return z8zE289;
end_block_exception_2661: ;

  return UNIT;
}

void zsuspend_frame(struct zFrameCheckpoint *z8zE290, unit z3zE2330)
{
  uint64_t zstate;
  zstate = zk_state_checkpoint(UNIT);
  uint64_t zsaved_pc;
  zsaved_pc = zpc;
  uint64_t zsaved_gas;
  zsaved_gas = zgas_remaining;
  uint64_t zsaved_state_gas;
  zsaved_state_gas = zstate_gas_remaining;
  uint64_t zsaved_state_spill;
  zsaved_state_spill = zstate_gas_spilled;
  __int128 zsaved_refund;
  zsaved_refund = zframe_refund;
  struct zFrameStatus zsaved_status;
  CREATE(zFrameStatus)(&zsaved_status);
  zsaved_status = zframe_status;
  struct zMessage zsaved_message;
  zsaved_message = zmessage;
  uint64_t zsaved_depth;
  zsaved_depth = zcall_depth;
  struct zCode zsaved_code;
  zsaved_code = zframe_code;
  struct zByteSliceFields zsaved_calldata;
  zsaved_calldata = zcalldata;
  unit z3zE2331;
  z3zE2331 = stack_enter_frame(UNIT);
  struct zByteSliceFields zsaved_memory;
  zsaved_memory = zmemory_frame_enter(UNIT);
  struct zFrameCheckpoint z3zE2332;
  CREATE(zFrameCheckpoint)(&z3zE2332);
  z3zE2332.zcall_depth = zsaved_depth;
  z3zE2332.zcalldata = zsaved_calldata;
  z3zE2332.zcode = zsaved_code;
  z3zE2332.zgas_remaining = zsaved_gas;
  z3zE2332.zmemory = zsaved_memory;
  z3zE2332.zmessage = zsaved_message;
  z3zE2332.zpc = zsaved_pc;
  z3zE2332.zrefund = zsaved_refund;
  z3zE2332.zstate = zstate;
  z3zE2332.zstate_gas_remaining = zsaved_state_gas;
  z3zE2332.zstate_gas_spilled = zsaved_state_spill;
  z3zE2332.zstatus = zsaved_status;
  (*(z8zE290)) = z3zE2332;
  KILL(zFrameCheckpoint)(&z3zE2332);
  KILL(zFrameStatus)(&zsaved_status);
end_function_2658: ;
  goto end_function_3641;
end_block_exception_2659: ;
  goto end_function_3641;
end_function_3641: ;
}

unit zrestore_frame(struct zFrameCheckpoint zcheckpoint)
{
  unit z8zE291;
  unit z3zE2329;
  z3zE2329 = stack_leave_frame(UNIT);
  struct zByteSliceFields z2zE2384;
  z2zE2384 = zcheckpoint.zmemory;
  unit z3zE2328;
  z3zE2328 = zmemory_frame_leave(z2zE2384);
  zpc = zcheckpoint.zpc;
  unit z3zE2327;
  z3zE2327 = UNIT;
  zgas_remaining = zcheckpoint.zgas_remaining;
  unit z3zE2326;
  z3zE2326 = UNIT;
  zstate_gas_remaining = zcheckpoint.zstate_gas_remaining;
  unit z3zE2325;
  z3zE2325 = UNIT;
  zstate_gas_spilled = zcheckpoint.zstate_gas_spilled;
  unit z3zE2324;
  z3zE2324 = UNIT;
  zframe_refund = zcheckpoint.zrefund;
  unit z3zE2323;
  z3zE2323 = UNIT;
  zframe_status = zcheckpoint.zstatus;
  unit z3zE2322;
  z3zE2322 = UNIT;
  zmessage = zcheckpoint.zmessage;
  unit z3zE2321;
  z3zE2321 = UNIT;
  zcall_depth = zcheckpoint.zcall_depth;
  unit z3zE2320;
  z3zE2320 = UNIT;
  zframe_code = zcheckpoint.zcode;
  unit z3zE2319;
  z3zE2319 = UNIT;
  zcalldata = zcheckpoint.zcalldata;
  z8zE291 = UNIT;
end_function_2656: ;
  return z8zE291;
end_block_exception_2657: ;

  return UNIT;
}

unit zmem_set_byte(uint64_t zoff, uint64_t zv)
{
  unit z8zE292;
  bool z2zE2383;
  z2zE2383 = zis_running(UNIT);
  if (z2zE2383) {  z8zE292 = mem_write_byte(zoff, zv);  } else {  z8zE292 = UNIT;  }
end_function_2654: ;
  return z8zE292;
end_block_exception_2655: ;

  return UNIT;
}

sail_u256 zmem_load(uint64_t zoff)
{
  sail_u256 z8zE293;
  bool z2zE2382;
  z2zE2382 = zis_running(UNIT);
  if (z2zE2382) {  z8zE293 = mem_load_word(zoff);  } else {  z8zE293 = zZERO_WORD;  }
end_function_2652: ;
  return z8zE293;
end_block_exception_2653: ;

  return u256_zero();
}

unit zmem_store(uint64_t zoff, sail_u256 zw)
{
  unit z8zE294;
  bool z2zE2381;
  z2zE2381 = zis_running(UNIT);
  if (z2zE2381) {  z8zE294 = mem_store_word(zoff, zw);  } else {  z8zE294 = UNIT;  }
end_function_2650: ;
  return z8zE294;
end_block_exception_2651: ;

  return UNIT;
}

unit zmem_store_byte(uint64_t zoff, sail_u256 zw)
{
  unit z8zE295;
  uint64_t z2zE2380;
  z2zE2380 = zword_low_byte(zw);
  z8zE295 = zmem_set_byte(zoff, z2zE2380);
end_function_2648: ;
  return z8zE295;
end_block_exception_2649: ;

  return UNIT;
}

unit zmem_mcopy(uint64_t zdst, uint64_t zsrc, uint64_t zlen)
{
  unit z8zE296;
  bool z2zE2379;
  z2zE2379 = (zlen != UINT64_C(0));
  if (z2zE2379) {  z8zE296 = mem_move(zdst, zsrc, zlen);  } else {  z8zE296 = UNIT;  }
end_function_2646: ;
  return z8zE296;
end_block_exception_2647: ;

  return UNIT;
}

sail_u256 zmem_keccak(struct zMemoryRangeFields zrange)
{
  sail_u256 z8zE297;
  sail_fixed_bytes_32 z2zE2378;
  {
    struct zByteSliceFields z2zE2377;
    {
      uint64_t z2zE2375;
      z2zE2375 = zrange.zoff;
      uint64_t z2zE2376;
      z2zE2376 = zrange.zlen;
      z2zE2377 = zmemory_byte_slice(z2zE2375, z2zE2376);
    }
    z2zE2378 = zkeccak256_slice(z2zE2377);
  }
  z8zE297 = evmsail_hash_to_word(z2zE2378);
end_function_2644: ;
  return z8zE297;
end_block_exception_2645: ;

  return u256_zero();
}

