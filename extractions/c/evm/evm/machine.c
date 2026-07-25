/* Generated from sail/evm/machine.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_98(void) {

  uint64_t z3zE131;
  z3zE131 = UINT64_C(1024);
  zDEPTH_LIMIT = z3zE131;
let_end_401: ;
}
void kill_letbind_98(void) {
}

unit zframe_stack_reset(unit z3zE2981)
{
  unit z8zE289;
  zframe_stack_top = UINT64_C(0);
  z8zE289 = UNIT;
end_function_3003: ;
  return z8zE289;
end_block_exception_3004: ;

  return UNIT;
}

bool zframe_stack_is_empty(unit z3zE2980)
{
  bool z8zE290;
  z8zE290 = (zframe_stack_top == UINT64_C(0));
end_function_3001: ;
  return z8zE290;
end_block_exception_3002: ;

  return false;
}

unit zframe_stack_push(struct zFrameContinuation zcontinuation)
{
  unit z8zE291;
  uint64_t ztop;
  ztop = zframe_stack_top;
  bool z2zE2851;
  z2zE2851 = (ztop < zDEPTH_LIMIT);
  if (z2zE2851) {
    fast_unsigned_vector_update_zz5vecz8z5unionz0zzFrameContinuationz9(&zframe_stack, zframe_stack, ztop, zcontinuation);
    unit z3zE2979;
    z3zE2979 = UNIT;
    {    zframe_stack_top = (ztop + UINT64_C(1));
    }
    z8zE291 = UNIT;
  } else {  z8zE291 = sail_assert(false, "sail/evm/machine.sail:59.20-59.21");  }
end_function_2999: ;
  return z8zE291;
end_block_exception_3000: ;

  return UNIT;
}

void zframe_stack_pop(struct zFrameContinuation *z8zE292, unit z3zE2976)
{
  uint64_t ztop;
  ztop = zframe_stack_top;
  bool z2zE2850;
  z2zE2850 = (UINT64_C(0) < ztop);
  if (z2zE2850) {
    uint64_t zparent_top;
    {    zparent_top = (ztop - UINT64_C(1));
    }
    zframe_stack_top = zparent_top;
    unit z3zE2978;
    z3zE2978 = UNIT;
    fast_unsigned_vector_access_zz5vecz8z5unionz0zzFrameContinuationz9((*(&z8zE292)), zframe_stack, zparent_top);
  } else {
    unit z3zE2977;
    z3zE2977 = sail_assert(false, "sail/evm/machine.sail:71.20-71.21");
    sail_match_failure("frame_stack_pop");
    /* unreachable after exit */
  }
end_function_2997: ;
  goto end_function_4055;
end_block_exception_2998: ;
  goto end_function_4055;
end_function_4055: ;
}

__int128 zvalidated_refund_add(__int128 zleft, __int128 zright)
{
  __int128 z8zE293;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2848;
  {
    bool z2zE2847;
    z2zE2847 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2975;
    if (z2zE2847) {
      z3zE2975 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2975 = false;  }
    z2zE2848 = z3zE2975;
  }
  if (z2zE2848) {  z8zE293 = ztotal;  } else {
    struct zexception z2zE2849;
    CREATE(zexception)(&z2zE2849);
    zInvalidBlock(&z2zE2849, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2849);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:95.8-95.44");
    KILL(zexception)(&z2zE2849);
    goto end_block_exception_2996;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2849);
  }
end_function_2995: ;
  return z8zE293;
end_block_exception_2996: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

unit zrecord_refund(__int128 zdelta)
{
  unit z8zE294;
  {
    zframe_refund = zvalidated_refund_add(zframe_refund, zdelta);
    if (have_exception) {  goto end_block_exception_2994;  }
  }
  z8zE294 = UNIT;
end_function_2993: ;
  return z8zE294;
end_block_exception_2994: ;

  return UNIT;
}

uint64_t zframe_code_len(unit z3zE2974)
{
  uint64_t z8zE295;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zlength;
  zlength = zcode.zlen;
  z8zE295 = zlength;
end_function_2991: ;
  return z8zE295;
end_block_exception_2992: ;

  return UINT64_C(0xdeadc0de);
}

bool zframe_jumpdest_valid(uint64_t zdest)
{
  bool z8zE296;
  struct zByteSliceFields zcode;
  zcode = zframe_code.zbytes;
  uint64_t zlength;
  zlength = zcode.zlen;
  uint64_t z2zE2846;
  z2zE2846 = zframe_code.zjumpdests;
  z8zE296 = jumpdest_ref_contains(z2zE2846, zlength, zdest);
end_function_2989: ;
  return z8zE296;
end_block_exception_2990: ;

  return false;
}

void create_letbind_99(void) {

  uint64_t z3zE132;
  z3zE132 = UINT64_C(1024);
  zSTACK_LIMIT = z3zE132;
let_end_410: ;
}
void kill_letbind_99(void) {
}

uint64_t zconserved_gas_add(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE297;
  bool z2zE2844;
  {
    uint64_t z2zE2843;
    {    z2zE2843 = (UINT64_C(18446744073709551615) - zleft);
    }
    z2zE2844 = (!(z2zE2843 < zright));
  }
  if (z2zE2844) {
    {    z8zE297 = (zleft + zright);
    }
  } else {
    struct zexception z2zE2845;
    CREATE(zexception)(&z2zE2845);
    zInvalidBlock(&z2zE2845, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2845);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:179.8-179.44");
    KILL(zexception)(&z2zE2845);
    goto end_block_exception_2988;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2845);
  }
end_function_2987: ;
  return z8zE297;
end_block_exception_2988: ;

  return UINT64_C(0xdeadc0de);
}

unit zrefill_frame_state_gas(unit z3zE2971)
{
  unit z8zE298;
  bool z2zE2842;
  z2zE2842 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE2842) {
    {
      zgas_remaining = zconserved_gas_add(zgas_remaining, zstate_gas_spilled);
      if (have_exception) {  goto end_block_exception_2986;  }
    }
    unit z3zE2973;
    z3zE2973 = UNIT;
    zstate_gas_remaining = zmessage.zstate_gas_reservoir;
    unit z3zE2972;
    z3zE2972 = UNIT;
    zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
    z8zE298 = UNIT;
  } else {  z8zE298 = UNIT;  }
end_function_2985: ;
  return z8zE298;
end_block_exception_2986: ;

  return UNIT;
}

__int128 zframe_state_gas_used(unit z3zE2967)
{
  __int128 z8zE299;
  uint64_t zentry;
  zentry = zmessage.zstate_gas_reservoir;
  uint64_t zremaining;
  zremaining = zstate_gas_remaining;
  uint64_t zspilled;
  zspilled = zstate_gas_spilled;
  __int128 z2zE2841;
  {
    __int128 z3zE2968;
    z3zE2968 = (__int128)(zentry);
    __int128 z3zE2969;
    z3zE2969 = (__int128)(zremaining);
    z2zE2841 = (z3zE2968 - z3zE2969);
  }
  {
    __int128 z3zE2970;
    z3zE2970 = (__int128)(zspilled);
    z8zE299 = (z2zE2841 + z3zE2970);
  }
end_function_2983: ;
  return z8zE299;
end_block_exception_2984: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

unit zexc_halt(enum zExceptionKind zk)
{
  unit z8zE300;
  unit z3zE2966;
  {
    z3zE2966 = zrefill_frame_state_gas(UNIT);
    if (have_exception) {  goto end_block_exception_2982;  }
  }
  zgas_remaining = zGAS_ZERO;
  unit z3zE2965;
  z3zE2965 = UNIT;
  zExceptional(&zframe_status, zk);
  z8zE300 = UNIT;
end_function_2981: ;
  return z8zE300;
end_block_exception_2982: ;

  return UNIT;
}

uint64_t zstack_height(unit z3zE2963)
{
  uint64_t z8zE301;
  uint64_t zheight;
  zheight = stack_depth(UNIT);
  bool z2zE2840;
  z2zE2840 = (!(zSTACK_LIMIT < zheight));
  unit z3zE2964;
  z3zE2964 = sail_assert(z2zE2840, "sail/evm/machine.sail:168.32-168.33");
  z8zE301 = zheight;
end_function_2979: ;
  return z8zE301;
end_block_exception_2980: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zpeek(uint64_t zn)
{
  sail_u256 z8zE302;
  z8zE302 = stack_peek_word(zn);
end_function_2977: ;
  return z8zE302;
end_block_exception_2978: ;

  return u256_zero();
}

unit zpush_word(sail_u256 zw)
{
  unit z8zE303;
  bool z2zE2839;
  {
    uint64_t z2zE2838;
    z2zE2838 = zstack_height(UNIT);
    z2zE2839 = (!(z2zE2838 < zSTACK_LIMIT));
  }
  if (z2zE2839) {
    {
      z8zE303 = zexc_halt(zStackOverflow);
      if (have_exception) {  goto end_block_exception_2976;  }
    }
  } else {  z8zE303 = stack_push_word(zw);  }
end_function_2975: ;
  return z8zE303;
end_block_exception_2976: ;

  return UNIT;
}

unit zpush_gas(uint64_t zvalue)
{
  unit z8zE304;
  sail_u256 z2zE2837;
  z2zE2837 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  {
    z8zE304 = zpush_word(z2zE2837);
    if (have_exception) {  goto end_block_exception_2974;  }
  }
end_function_2973: ;
  return z8zE304;
end_block_exception_2974: ;

  return UNIT;
}

sail_u256 zpop(unit z3zE2961)
{
  sail_u256 z8zE305;
  bool z2zE2836;
  {
    uint64_t z2zE2835;
    z2zE2835 = zstack_height(UNIT);
    z2zE2836 = (z2zE2835 == UINT64_C(0));
  }
  if (z2zE2836) {
    unit z3zE2962;
    {
      z3zE2962 = zexc_halt(zStackUnderflow);
      if (have_exception) {  goto end_block_exception_2972;  }
    }
    z8zE305 = zWORD_ZERO;
  } else {  z8zE305 = stack_pop_word(UNIT);  }
end_function_2971: ;
  return z8zE305;
end_block_exception_2972: ;

  return u256_zero();
}

unit zstack_set(uint64_t zn, sail_u256 zw)
{
  unit z8zE306;
  z8zE306 = stack_set_word(zn, zw);
end_function_2969: ;
  return z8zE306;
end_block_exception_2970: ;

  return UNIT;
}

bool zis_running(unit z3zE2957)
{
  bool z8zE307;
  bool z3zE2958;
  {
    if (zframe_status.kind != Kind_zRunning) goto case_2966;
    z3zE2958 = true;
    goto finish_match_2964;
  }
case_2966: ;
  {
    z3zE2958 = false;
    goto finish_match_2964;
  }
case_2965: ;
finish_match_2964: ;
  z8zE307 = z3zE2958;
end_function_2967: ;
  return z8zE307;
end_block_exception_2968: ;

  return false;
}

unit zcalldata_install(struct zByteSliceFields zdata)
{
  unit z8zE308;
  zcalldata = zdata;
  z8zE308 = UNIT;
end_function_2962: ;
  return z8zE308;
end_block_exception_2963: ;

  return UNIT;
}

unit zreturndata_clear(unit z3zE2956)
{
  unit z8zE309;
  zreturndata = zEMPTY_SLICE;
  z8zE309 = UNIT;
end_function_2960: ;
  return z8zE309;
end_block_exception_2961: ;

  return UNIT;
}

uint64_t zreturndata_sizze(unit z3zE2955)
{
  uint64_t z8zE310;
  struct zByteSliceFields zdata;
  zdata = zreturndata;
  z8zE310 = zdata.zlen;
end_function_2958: ;
  return z8zE310;
end_block_exception_2959: ;

  return UINT64_C(0xdeadc0de);
}

unit zreturndata_copy(uint64_t zdst, uint64_t zoff, uint64_t zlen)
{
  unit z8zE311;
  z8zE311 = zslice_copy(zreturndata, zdst, zoff, zlen);
end_function_2956: ;
  return z8zE311;
end_block_exception_2957: ;

  return UNIT;
}

unit zreturndata_copy_prefix(uint64_t zdst, uint64_t zwant)
{
  unit z8zE312;
  uint64_t zwanted;
  zwanted = zwant;
  uint64_t zavailable;
  zavailable = zreturndata_sizze(UNIT);
  uint64_t zcopy_length;
  {
    bool z2zE2834;
    z2zE2834 = (zwanted < zavailable);
    if (z2zE2834) {  zcopy_length = zwanted;  } else {  zcopy_length = zavailable;  }
  }
  z8zE312 = zslice_copy(zreturndata, zdst, UINT64_C(0), zcopy_length);
end_function_2954: ;
  return z8zE312;
end_block_exception_2955: ;

  return UNIT;
}

uint64_t zreturndata_remaining(uint64_t zavailable, uint64_t zoffset)
{
  uint64_t z8zE313;
  {    z8zE313 = (zavailable - zoffset);
  }
end_function_2952: ;
  return z8zE313;
end_block_exception_2953: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidated_returndata_copy(uint64_t zdst, sail_u256 zsource_offset, sail_u256 zlength)
{
  unit z8zE314;
  uint64_t z2zE2831;
  z2zE2831 = zreturndata_sizze(UNIT);
  unit z3zE2953;
  {
    uint64_t zavailable;
    zavailable = z2zE2831;
    bool z2zE2832;
    z2zE2832 = (!u64_lt_u256(zavailable, zsource_offset));
    if (z2zE2832) {
      uint64_t zremaining;
      {
        uint64_t z3zE3787;
        z3zE3787 = u256_to_u64(zsource_offset);
        zremaining = zreturndata_remaining(zavailable, z3zE3787);
      }
      uint64_t zbounded_source_offset;
      zbounded_source_offset = u256_to_u64(zsource_offset);
      bool z2zE2833;
      z2zE2833 = (!u64_lt_u256(zremaining, zlength));
      if (z2zE2833) {
        uint64_t zbounded_length;
        zbounded_length = u256_to_u64(zlength);
        z3zE2953 = zreturndata_copy(zdst, zbounded_source_offset, zbounded_length);
      } else {
        {
          z3zE2953 = zexc_halt(zInvalidOpcode);
          if (have_exception) {  goto end_block_exception_2951;  }
        }
      }
    } else {
      {
        z3zE2953 = zexc_halt(zInvalidOpcode);
        if (have_exception) {  goto end_block_exception_2951;  }
      }
    }
    goto finish_match_2948;
  }
case_2949: ;
  sail_match_failure("validated_returndata_copy");
finish_match_2948: ;
  z8zE314 = z3zE2953;
end_function_2950: ;
  return z8zE314;
end_block_exception_2951: ;

  return UNIT;
}

unit zreturndata_copy_words(uint64_t zdst, sail_u256 zsource_offset, sail_u256 zlength)
{
  unit z8zE315;
  {
    z8zE315 = zvalidated_returndata_copy(zdst, zsource_offset, zlength);
    if (have_exception) {  goto end_block_exception_2947;  }
  }
end_function_2946: ;
  return z8zE315;
end_block_exception_2947: ;

  return UNIT;
}

uint64_t zevm_memory_high_water(unit z3zE2952)
{
  uint64_t z8zE316;
  struct zByteSliceFields zmemory;
  zmemory = zevm_memory;
  uint64_t zlength;
  zlength = zmemory.zlen;
  z8zE316 = zlength;
end_function_2944: ;
  return z8zE316;
end_block_exception_2945: ;

  return UINT64_C(0xdeadc0de);
}

unit zmemory_reset(unit z3zE2950)
{
  unit z8zE317;
  unit z3zE2951;
  z3zE2951 = mem_clear(UNIT);
  zevm_memory = zbyte_slice(zEvmMemorySource, UINT64_C(0), UINT64_C(0));
  z8zE317 = UNIT;
end_function_2942: ;
  return z8zE317;
end_block_exception_2943: ;

  return UNIT;
}

struct zByteSliceFields zmemory_expand_to(uint64_t znew_sizze)
{
  struct zByteSliceFields z8zE318;
  struct zByteSliceFields zmemory;
  zmemory = zevm_memory;
  bool z2zE2830;
  {
    uint64_t z2zE2829;
    z2zE2829 = zmemory.zlen;
    z2zE2830 = (z2zE2829 < znew_sizze);
  }
  if (z2zE2830) {
    struct zByteSliceFields zexpanded;
    zexpanded = mem_expand(znew_sizze);
    zevm_memory = zexpanded;
    unit z3zE2949;
    z3zE2949 = UNIT;
    z8zE318 = zexpanded;
  } else {  z8zE318 = zsub_slice(zmemory, UINT64_C(0), znew_sizze);  }
end_function_2940: ;
  return z8zE318;
end_block_exception_2941: ;
  struct zByteSliceFields z8zE882 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE882;
}

struct zByteSliceFields zmemory_byte_slice(uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE319;
  bool z2zE2826;
  z2zE2826 = (zlen == UINT64_C(0));
  if (z2zE2826) {  z8zE319 = zEMPTY_SLICE;  } else {
    struct zByteSliceFields z2zE2828;
    {
      uint64_t z2zE2827;
      {    z2zE2827 = (zoff + zlen);
      }
      z2zE2828 = zmemory_expand_to(z2zE2827);
    }
    z8zE319 = zsub_slice(z2zE2828, zoff, zlen);
  }
end_function_2938: ;
  return z8zE319;
end_block_exception_2939: ;
  struct zByteSliceFields z8zE883 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE883;
}

struct zByteSliceFields zmemory_code_slice(uint64_t zoff, uint64_t zlen)
{
  struct zByteSliceFields z8zE320;
  bool z2zE2822;
  z2zE2822 = (zlen == UINT64_C(0));
  if (z2zE2822) {  z8zE320 = zEMPTY_CODE_SLICE;  } else {
    struct zByteSliceFields z2zE2825;
    {
      struct zByteSliceFields z2zE2824;
      {
        uint64_t z2zE2823;
        {    z2zE2823 = (zoff + zlen);
        }
        z2zE2824 = zmemory_expand_to(z2zE2823);
      }
      z2zE2825 = zsub_slice(z2zE2824, zoff, zlen);
    }
    z8zE320 = zcode_slice(z2zE2825);
  }
end_function_2936: ;
  return z8zE320;
end_block_exception_2937: ;
  struct zByteSliceFields z8zE884 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE884;
}

struct zByteSliceFields zmemory_frame_enter(unit z3zE2947)
{
  struct zByteSliceFields z8zE321;
  struct zByteSliceFields zparent;
  zparent = zevm_memory;
  uint64_t zbase;
  zbase = mem_frame_enter(UNIT);
  zevm_memory = zbyte_slice(zEvmMemorySource, zbase, UINT64_C(0));
  unit z3zE2948;
  z3zE2948 = UNIT;
  z8zE321 = zparent;
end_function_2934: ;
  return z8zE321;
end_block_exception_2935: ;
  struct zByteSliceFields z8zE885 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE885;
}

unit zmemory_frame_leave(struct zByteSliceFields zparent)
{
  unit z8zE322;
  unit z3zE2946;
  z3zE2946 = mem_frame_leave(UNIT);
  zevm_memory = zparent;
  z8zE322 = UNIT;
end_function_2932: ;
  return z8zE322;
end_block_exception_2933: ;

  return UNIT;
}

void zsuspend_frame(struct zFrameCheckpoint *z8zE323, unit z3zE2943)
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
  COPY(zFrameStatus)(&zsaved_status, zframe_status);
  struct zMessage zsaved_message;
  zsaved_message = zmessage;
  uint64_t zsaved_depth;
  zsaved_depth = zcall_depth;
  struct zCode zsaved_code;
  zsaved_code = zframe_code;
  struct zByteSliceFields zsaved_calldata;
  zsaved_calldata = zcalldata;
  unit z3zE2944;
  z3zE2944 = stack_enter_frame(UNIT);
  struct zByteSliceFields zsaved_memory;
  zsaved_memory = zmemory_frame_enter(UNIT);
  struct zFrameCheckpoint z3zE2945;
  CREATE(zFrameCheckpoint)(&z3zE2945);
  z3zE2945.zcall_depth = zsaved_depth;
  z3zE2945.zcalldata = zsaved_calldata;
  z3zE2945.zcode = zsaved_code;
  z3zE2945.zgas_remaining = zsaved_gas;
  z3zE2945.zmemory = zsaved_memory;
  z3zE2945.zmessage = zsaved_message;
  z3zE2945.zpc = zsaved_pc;
  z3zE2945.zrefund = zsaved_refund;
  z3zE2945.zstate = zstate;
  z3zE2945.zstate_gas_remaining = zsaved_state_gas;
  z3zE2945.zstate_gas_spilled = zsaved_state_spill;
  COPY(zFrameStatus)(&((&z3zE2945)->zstatus), zsaved_status);
  COPY(zFrameCheckpoint)((*(&z8zE323)), z3zE2945);
  KILL(zFrameCheckpoint)(&z3zE2945);
  KILL(zFrameStatus)(&zsaved_status);
end_function_2930: ;
  goto end_function_4054;
end_block_exception_2931: ;
  goto end_function_4054;
end_function_4054: ;
}

unit zrestore_frame(struct zFrameCheckpoint zcheckpoint)
{
  unit z8zE324;
  unit z3zE2942;
  z3zE2942 = stack_leave_frame(UNIT);
  struct zByteSliceFields z2zE2821;
  z2zE2821 = zcheckpoint.zmemory;
  unit z3zE2941;
  z3zE2941 = zmemory_frame_leave(z2zE2821);
  zpc = zcheckpoint.zpc;
  unit z3zE2940;
  z3zE2940 = UNIT;
  zgas_remaining = zcheckpoint.zgas_remaining;
  unit z3zE2939;
  z3zE2939 = UNIT;
  zstate_gas_remaining = zcheckpoint.zstate_gas_remaining;
  unit z3zE2938;
  z3zE2938 = UNIT;
  zstate_gas_spilled = zcheckpoint.zstate_gas_spilled;
  unit z3zE2937;
  z3zE2937 = UNIT;
  zframe_refund = zcheckpoint.zrefund;
  unit z3zE2936;
  z3zE2936 = UNIT;
  COPY(zFrameStatus)(&zframe_status, zcheckpoint.zstatus);
  unit z3zE2935;
  z3zE2935 = UNIT;
  zmessage = zcheckpoint.zmessage;
  unit z3zE2934;
  z3zE2934 = UNIT;
  zcall_depth = zcheckpoint.zcall_depth;
  unit z3zE2933;
  z3zE2933 = UNIT;
  zframe_code = zcheckpoint.zcode;
  unit z3zE2932;
  z3zE2932 = UNIT;
  zcalldata = zcheckpoint.zcalldata;
  z8zE324 = UNIT;
end_function_2928: ;
  return z8zE324;
end_block_exception_2929: ;

  return UNIT;
}

unit zmem_set_byte(uint64_t zoff, uint64_t zv)
{
  unit z8zE325;
  bool z2zE2820;
  z2zE2820 = zis_running(UNIT);
  if (z2zE2820) {  z8zE325 = mem_write_byte(zoff, zv);  } else {  z8zE325 = UNIT;  }
end_function_2926: ;
  return z8zE325;
end_block_exception_2927: ;

  return UNIT;
}

sail_u256 zmem_load(uint64_t zoff)
{
  sail_u256 z8zE326;
  bool z2zE2819;
  z2zE2819 = zis_running(UNIT);
  if (z2zE2819) {  z8zE326 = mem_load_word(zoff);  } else {  z8zE326 = zZERO_WORD;  }
end_function_2924: ;
  return z8zE326;
end_block_exception_2925: ;

  return u256_zero();
}

unit zmem_store(uint64_t zoff, sail_u256 zw)
{
  unit z8zE327;
  bool z2zE2818;
  z2zE2818 = zis_running(UNIT);
  if (z2zE2818) {  z8zE327 = mem_store_word(zoff, zw);  } else {  z8zE327 = UNIT;  }
end_function_2922: ;
  return z8zE327;
end_block_exception_2923: ;

  return UNIT;
}

unit zmem_store_byte(uint64_t zoff, sail_u256 zw)
{
  unit z8zE328;
  uint64_t z2zE2817;
  z2zE2817 = zword_low_byte(zw);
  z8zE328 = zmem_set_byte(zoff, z2zE2817);
end_function_2920: ;
  return z8zE328;
end_block_exception_2921: ;

  return UNIT;
}

unit zmem_mcopy(uint64_t zdst, uint64_t zsrc, uint64_t zlen)
{
  unit z8zE329;
  bool z2zE2816;
  z2zE2816 = (zlen != UINT64_C(0));
  if (z2zE2816) {  z8zE329 = mem_move(zdst, zsrc, zlen);  } else {  z8zE329 = UNIT;  }
end_function_2918: ;
  return z8zE329;
end_block_exception_2919: ;

  return UNIT;
}

sail_u256 zmem_keccak(struct zMemoryRangeFields zrange)
{
  sail_u256 z8zE330;
  sail_fixed_bytes_32 z2zE2815;
  {
    struct zByteSliceFields z2zE2814;
    {
      uint64_t z2zE2812;
      z2zE2812 = zrange.zoff;
      uint64_t z2zE2813;
      z2zE2813 = zrange.zlen;
      z2zE2814 = zmemory_byte_slice(z2zE2812, z2zE2813);
    }
    z2zE2815 = zkeccak256_slice(z2zE2814);
  }
  z8zE330 = zhash_to_word(z2zE2815);
end_function_2916: ;
  return z8zE330;
end_block_exception_2917: ;

  return u256_zero();
}

