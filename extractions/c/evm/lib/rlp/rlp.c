/* Generated from sail/lib/rlp/rlp.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_107(void) {

  uint64_t z3zE118;
  z3zE118 = UINT64_C(55);
  zRLP_SHORT_LENGTH_LIMIT = z3zE118;
let_end_255: ;
}
void kill_letbind_107(void) {
}

void create_letbind_108(void) {

  uint64_t z3zE119;
  z3zE119 = zWORD_BYTE_LENGTH;
  zRLP_WORD_LENGTH_LIMIT = z3zE119;
let_end_256: ;
}
void kill_letbind_108(void) {
}

void create_letbind_109(void) {

  uint64_t z3zE120;
  z3zE120 = zEIGHT_BYTE_LENGTH;
  zRLP_UINT64_LENGTH_LIMIT = z3zE120;
let_end_257: ;
}
void kill_letbind_109(void) {
}

void create_letbind_110(void) {

  uint64_t z3zE121;
  z3zE121 = UINT64_C(33);
  zRLP_ENCODED_WORD_LENGTH = z3zE121;
let_end_258: ;
}
void kill_letbind_110(void) {
}

void create_letbind_111(void) {

  uint64_t z3zE122;
  z3zE122 = UINT64_C(21);
  zRLP_ENCODED_ADDRESS_LENGTH = z3zE122;
let_end_259: ;
}
void kill_letbind_111(void) {
}

uint64_t zrlp_scratch_small_length(uint64_t zvalue)
{
  uint64_t z8zE145;
  z8zE145 = zvalue;
end_function_3211: ;
  return z8zE145;
end_block_exception_3212: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_byte_count(uint64_t zindex)
{
  uint64_t z8zE146;
  {    z8zE146 = (zindex + UINT64_C(1));
  }
end_function_3207: ;
  return z8zE146;
end_block_exception_3208: ;

  return UINT64_C(0xdeadc0de);
}

void zminimal_word_bytes(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE147, sail_u256 zw)
{
  sail_u256 zremaining;
  zremaining = zw;
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  uint64_t zlen;
  zlen = UINT64_C(0);
  int64_t z3zE2775;
  {    z3zE2775 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2776;
  {    z3zE2776 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2777;
  {    z3zE2777 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2775;
    unit z3zE2778;
  for_start_3203: ;
    {
      if ((z3zE2776 < zbyte_index)) goto for_end_3204;
      bool z2zE3036;
      z2zE3036 = zword_nonzzero(zremaining);
      if (z2zE3036) {
        uint64_t z2zE3037;
        z2zE3037 = zword_low_byte(zremaining);
        zconsz3z5bv8(&zout, z2zE3037, zout);
        unit z3zE2780;
        z3zE2780 = UNIT;
        zremaining = zword_shift_right(zremaining, UINT64_C(8));
        unit z3zE2779;
        z3zE2779 = UNIT;
        {
          uint64_t z3zE3019;
          z3zE3019 = (uint64_t)(zbyte_index);
          zlen = zword_byte_count(z3zE3019);
        }
        z3zE2778 = UNIT;
      } else {  z3zE2778 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE2777);
      goto for_start_3203;
    }
  for_end_3204: ;
  }
  unit z3zE2781;
  z3zE2781 = UNIT;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE2782;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2782);
  COPY(zz5listz8z5bv8z9)(&((&z3zE2782)->ztup0), zout);
  z3zE2782.ztup1 = zlen;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE147)), z3zE2782);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE2782);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3205: ;
  goto end_function_3653;
end_block_exception_3206: ;
  goto end_function_3653;
end_function_3653: ;
}

uint64_t zrlp_nat_length_byte(uint64_t zvalue)
{
  uint64_t z8zE148;
  z8zE148 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
end_function_3201: ;
  return z8zE148;
end_block_exception_3202: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_minimal_word_len(sail_u256 zw)
{
  uint64_t z8zE149;
  z8zE149 = zword_byte_length(zw);
end_function_3189: ;
  return z8zE149;
end_block_exception_3190: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_word_sizze(sail_u256 zw)
{
  uint64_t z8zE150;
  uint64_t zlen;
  zlen = zrlp_minimal_word_len(zw);
  uint64_t zfirst;
  {
    bool z2zE3005;
    z2zE3005 = (zlen == UINT64_C(1));
    if (z2zE3005) {  zfirst = zword_low_byte(zw);  } else {  zfirst = UINT64_C(0x00);  }
  }
  bool z2zE3004;
  {
    bool z2zE3003;
    z2zE3003 = (zlen == UINT64_C(1));
    bool z3zE2756;
    if (z2zE3003) {
      uint64_t z2zE3002;
      z2zE3002 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2756 = (z2zE3002 == UINT64_C(0b0));
    } else {  z3zE2756 = false;  }
    z2zE3004 = z3zE2756;
  }
  if (z2zE3004) {  z8zE150 = UINT64_C(1);  } else {
    {    z8zE150 = (UINT64_C(1) + zlen);
    }
  }
end_function_3178: ;
  return z8zE150;
end_block_exception_3179: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_nat_sizze(uint64_t zn)
{
  uint64_t z8zE151;
  sail_u256 z2zE3001;
  z2zE3001 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE151 = zrlp_uint_word_sizze(z2zE3001);
end_function_3176: ;
  return z8zE151;
end_block_exception_3177: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_list_sizze(uint64_t zcontent_len)
{
  uint64_t z8zE152;
  uint64_t z2zE3000;
  {
    uint64_t z2zE2999;
    z2zE2999 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    z2zE3000 = zrlp_scratch_small_length(z2zE2999);
  }
  {
    z8zE152 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE3000);
    if (have_exception) {  goto end_block_exception_3175;  }
  }
end_function_3174: ;
  return z8zE152;
end_block_exception_3175: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_slice_sizze(struct zByteSliceFields zdata)
{
  uint64_t z8zE153;
  uint64_t zlength;
  zlength = zdata.zlen;
  uint64_t zfirst;
  {
    bool z2zE2998;
    z2zE2998 = (zlength == UINT64_C(0));
    if (z2zE2998) {  zfirst = UINT64_C(0x00);  } else {  zfirst = zslice_byte(zdata, UINT64_C(0));  }
  }
  bool z2zE2995;
  {
    bool z2zE2994;
    z2zE2994 = (zlength == UINT64_C(1));
    bool z3zE2755;
    if (z2zE2994) {
      uint64_t z2zE2993;
      z2zE2993 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2755 = (z2zE2993 == UINT64_C(0b0));
    } else {  z3zE2755 = false;  }
    z2zE2995 = z3zE2755;
  }
  if (z2zE2995) {  z8zE153 = UINT64_C(1);  } else {
    uint64_t z2zE2997;
    {
      uint64_t z2zE2996;
      z2zE2996 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE2997 = zrlp_scratch_small_length(z2zE2996);
    }
    {
      z8zE153 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE2997);
      if (have_exception) {  goto end_block_exception_3173;  }
    }
  }
end_function_3172: ;
  return z8zE153;
end_block_exception_3173: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_bytes_sizze(zz5listz8z5bv8z9 zdata, uint64_t zlength)
{
  uint64_t z8zE154;
  uint64_t zfirst;
  {
    uint64_t z3zE2751;
    {
      if ((zdata == NULL)) goto case_3169;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE2751 = zb;
      goto finish_match_3167;
    }
  case_3169: ;
    {
      /* complete */
      z3zE2751 = UINT64_C(0x00);
      goto finish_match_3167;
    }
  case_3168: ;
  finish_match_3167: ;
    zfirst = z3zE2751;
  }
  bool z2zE2990;
  {
    bool z2zE2989;
    z2zE2989 = (zlength == UINT64_C(1));
    bool z3zE2754;
    if (z2zE2989) {
      uint64_t z2zE2988;
      z2zE2988 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE2754 = (z2zE2988 == UINT64_C(0b0));
    } else {  z3zE2754 = false;  }
    z2zE2990 = z3zE2754;
  }
  if (z2zE2990) {  z8zE154 = UINT64_C(1);  } else {
    uint64_t z2zE2992;
    {
      uint64_t z2zE2991;
      z2zE2991 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE2992 = zrlp_scratch_small_length(z2zE2991);
    }
    {
      z8zE154 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE2992);
      if (have_exception) {  goto end_block_exception_3171;  }
    }
  }
end_function_3170: ;
  return z8zE154;
end_block_exception_3171: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE155;
  uint64_t zfirst;
  {
    uint64_t z3zE2743;
    {
      if ((zdata == NULL)) goto case_3156;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE2743 = zb;
      goto finish_match_3154;
    }
  case_3156: ;
    {
      /* complete */
      z3zE2743 = UINT64_C(0x00);
      goto finish_match_3154;
    }
  case_3155: ;
  finish_match_3154: ;
    zfirst = z3zE2743;
  }
  unit z3zE2746;
  z3zE2746 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlen, zfirst);
  z8zE155 = zscratch_push_bytes(zdata, zlen);
end_function_3157: ;
  return z8zE155;
end_block_exception_3158: ;

  return UNIT;
}

unit zrlp_write_slice(struct zByteSliceFields zdata)
{
  unit z8zE156;
  uint64_t z2zE2984;
  z2zE2984 = zdata.zlen;
  uint64_t z2zE2985;
  {
    bool z2zE2983;
    {
      uint64_t z2zE2982;
      z2zE2982 = zdata.zlen;
      z2zE2983 = (z2zE2982 == UINT64_C(0));
    }
    if (z2zE2983) {  z2zE2985 = UINT64_C(0x00);  } else {  z2zE2985 = zslice_byte(zdata, UINT64_C(0));  }
  }
  unit z3zE2742;
  z3zE2742 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2984, z2zE2985);
  z8zE156 = zscratch_push_slice(zdata);
end_function_3152: ;
  return z8zE156;
end_block_exception_3153: ;

  return UNIT;
}

unit zrlp_write_uint_word(sail_u256 zw)
{
  unit z8zE157;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2981;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2981);
  zminimal_word_bytes(&z2zE2981, zw);
  unit z3zE2740;
  {
    zz5listz8z5bv8z9 zbytes;
    CREATE(zz5listz8z5bv8z9)(&zbytes);
    COPY(zz5listz8z5bv8z9)(&zbytes, z2zE2981.ztup0);
    uint64_t zlen;
    zlen = z2zE2981.ztup1;
    z3zE2740 = zrlp_write_bytes(zbytes, zlen);
    KILL(zz5listz8z5bv8z9)(&zbytes);
    goto finish_match_3148;
  }
case_3149: ;
  sail_match_failure("rlp_write_uint_word");
finish_match_3148: ;
  z8zE157 = z3zE2740;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2981);
end_function_3150: ;
  return z8zE157;
end_block_exception_3151: ;

  return UNIT;
}

unit zrlp_write_uint_nat(uint64_t zn)
{
  unit z8zE158;
  sail_u256 z2zE2980;
  z2zE2980 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE158 = zrlp_write_uint_word(z2zE2980);
end_function_3146: ;
  return z8zE158;
end_block_exception_3147: ;

  return UNIT;
}

unit zrlp_write_word(sail_u256 zw)
{
  unit z8zE159;
  zz5listz8z5bv8z9 z2zE2979;
  CREATE(zz5listz8z5bv8z9)(&z2zE2979);
  zword_to_bytes32(&z2zE2979, zw);
  z8zE159 = zrlp_write_bytes(z2zE2979, zWORD_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE2979);
end_function_3144: ;
  return z8zE159;
end_block_exception_3145: ;

  return UNIT;
}

unit zrlp_write_addr(sail_fixed_bytes_20 za)
{
  unit z8zE160;
  zz5listz8z5bv8z9 z2zE2978;
  CREATE(zz5listz8z5bv8z9)(&z2zE2978);
  zaddress_to_bytes(&z2zE2978, za);
  z8zE160 = zrlp_write_bytes(z2zE2978, zADDRESS_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE2978);
end_function_3142: ;
  return z8zE160;
end_block_exception_3143: ;

  return UNIT;
}

struct zByteSliceFields zrlp_finish(uint64_t zstart)
{
  struct zByteSliceFields z8zE161;
  z8zE161 = zscratch_finish(zstart);
end_function_3140: ;
  return z8zE161;
end_block_exception_3141: ;
  struct zByteSliceFields z8zE785 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE785;
}

uint64_t zrlp_uint64_append(uint64_t z_width, uint64_t zprefix, uint64_t znext)
{
  uint64_t z8zE162;
  uint64_t z2zE2976;
  {    z2zE2976 = (zprefix * UINT64_C(256));
  }
  uint64_t z2zE2977;
  z2zE2977 = ((uint64_t) znext);
  {    z8zE162 = (z2zE2976 + z2zE2977);
  }
end_function_3138: ;
  return z8zE162;
end_block_exception_3139: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint64_width(struct zByteSliceFields zcontent, uint64_t zwidth)
{
  uint64_t z8zE163;
  bool z2zE2971;
  z2zE2971 = (zwidth == UINT64_C(0));
  if (z2zE2971) {  z8zE163 = UINT64_C(0);  } else {
    uint64_t z2zE2974;
    {
      uint64_t z2zE2972;
      {    z2zE2972 = (zwidth - UINT64_C(1));
      }
      z2zE2974 = zrlp_uint64_width(zcontent, z2zE2972);
    }
    uint64_t z2zE2975;
    {
      uint64_t z2zE2973;
      {    z2zE2973 = (zwidth - UINT64_C(1));
      }
      z2zE2975 = zslice_byte(zcontent, z2zE2973);
    }
    z8zE163 = zrlp_uint64_append(zwidth, z2zE2974, z2zE2975);
  }
end_function_3136: ;
  return z8zE163;
end_block_exception_3137: ;

  return UINT64_C(0xdeadc0de);
}

bool zrlp_bytes_equal_at(zz5listz8z5bv8z9 zexpected, struct zByteSliceFields zsource, uint64_t zstart)
{
  bool z8zE164;
  zz5listz8z5bv8z9 zrest;
  CREATE(zz5listz8z5bv8z9)(&zrest);
  COPY(zz5listz8z5bv8z9)(&zrest, zexpected);
  uint64_t zcurrent;
  zcurrent = zstart;
  bool zequal;
  zequal = true;
  int64_t z3zE2726;
  {    z3zE2726 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2727;
  {    z3zE2727 = (int64_t)(UINT64_C(7));
  }
  int64_t z3zE2728;
  {    z3zE2728 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE2726;
    unit z3zE2735;
  for_start_3126: ;
    {
      if ((z3zE2727 < zbyte_index)) goto for_end_3127;
      unit z3zE2729;
      {
        if ((zrest == NULL)) goto case_3130;
        uint64_t zb;
        zb = (*zrest).hd;
        zz5listz8z5bv8z9 ztail;
        CREATE(zz5listz8z5bv8z9)(&ztail);
        COPY(zz5listz8z5bv8z9)(&ztail, (*zrest).tl);
        uint64_t zposition;
        zposition = zcurrent;
        uint64_t zsource_length;
        zsource_length = zsource.zlen;
        bool z2zE2969;
        z2zE2969 = (zposition < zsource_length);
        unit z3zE2731;
        if (z2zE2969) {
          bool z3zE2732;
          if (zequal) {
            uint64_t z2zE2970;
            z2zE2970 = zslice_byte(zsource, zcurrent);
            z3zE2732 = (z2zE2970 == zb);
          } else {  z3zE2732 = false;  }
          zequal = z3zE2732;
          unit z3zE2733;
          z3zE2733 = UNIT;
          {    zcurrent = (zposition + UINT64_C(1));
          }
          z3zE2731 = UNIT;
        } else {
          zequal = false;
          z3zE2731 = UNIT;
        }
        COPY(zz5listz8z5bv8z9)(&zrest, ztail);
        z3zE2729 = UNIT;
        KILL(zz5listz8z5bv8z9)(&ztail);
        goto finish_match_3128;
      }
    case_3130: ;
      {
        /* complete */
        z3zE2729 = UNIT;
        goto finish_match_3128;
      }
    case_3129: ;
    finish_match_3128: ;
      z3zE2735 = z3zE2729;
      zbyte_index = (zbyte_index + z3zE2728);
      goto for_start_3126;
    }
  for_end_3127: ;
  }
  unit z3zE2736;
  z3zE2736 = UNIT;
  bool z3zE2737;
  {
    if (!((zrest == NULL))) goto case_3133;
    z3zE2737 = zequal;
    goto finish_match_3131;
  }
case_3133: ;
  {
    z3zE2737 = false;
    goto finish_match_3131;
  }
case_3132: ;
finish_match_3131: ;
  z8zE164 = z3zE2737;
  KILL(zz5listz8z5bv8z9)(&zrest);
end_function_3134: ;
  return z8zE164;
end_block_exception_3135: ;

  return false;
}

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 zrlp_ref_hdr(struct zByteSliceFields zb)
{
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE165;
  uint64_t zsource_len;
  zsource_len = zb.zlen;
  bool z2zE2950;
  z2zE2950 = (zsource_len == UINT64_C(0));
  unit z3zE2716;
  if (z2zE2950) {
    struct zexception z2zE2951;
    CREATE(zexception)(&z2zE2951);
    zInvalidBlock(&z2zE2951, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2951);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:428.8-428.37");
    KILL(zexception)(&z2zE2951);
    goto end_block_exception_3125;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2951);
  } else {  z3zE2716 = UNIT;  }
  uint64_t zh;
  {
    uint64_t z2zE2968;
    z2zE2968 = zslice_byte(zb, UINT64_C(0));
    zh = ((uint64_t) z2zE2968);
  }
  bool z2zE2952;
  z2zE2952 = (zh < UINT64_C(128));
  if (z2zE2952) {
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE2723;
    z3zE2723.ztup0 = false;
    z3zE2723.ztup1 = UINT64_C(0);
    z3zE2723.ztup2 = UINT64_C(1);
    z8zE165 = z3zE2723;
  } else {
    bool z2zE2953;
    z2zE2953 = (zh < UINT64_C(184));
    if (z2zE2953) {
      uint64_t z2zE2954;
      {    z2zE2954 = (zh - UINT64_C(128));
      }
      struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE2722;
      z3zE2722.ztup0 = false;
      z3zE2722.ztup1 = UINT64_C(1);
      z3zE2722.ztup2 = z2zE2954;
      z8zE165 = z3zE2722;
    } else {
      bool z2zE2955;
      z2zE2955 = (zh < UINT64_C(192));
      if (z2zE2955) {
        uint64_t zlength_width;
        {    zlength_width = (zh - UINT64_C(183));
        }
        bool z2zE2957;
        {
          uint64_t z2zE2956;
          {    z2zE2956 = (zsource_len - UINT64_C(1));
          }
          z2zE2957 = (z2zE2956 < zlength_width);
        }
        unit z3zE2720;
        if (z2zE2957) {
          struct zexception z2zE2958;
          CREATE(zexception)(&z2zE2958);
          zInvalidBlock(&z2zE2958, zRlpDecode);
          COPY(zexception)(current_exception, z2zE2958);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:440.20-440.49");
          KILL(zexception)(&z2zE2958);
          goto end_block_exception_3125;
          /* unreachable after throw */
          KILL(zexception)(&z2zE2958);
        } else {  z3zE2720 = UNIT;  }
        struct zByteSliceFields zlength_bytes;
        zlength_bytes = zsub_slice(zb, UINT64_C(1), zlength_width);
        uint64_t z2zE2959;
        {    z2zE2959 = (UINT64_C(1) + zlength_width);
        }
        uint64_t z2zE2960;
        z2zE2960 = zrlp_uint64_width(zlength_bytes, zlength_width);
        struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE2721;
        z3zE2721.ztup0 = false;
        z3zE2721.ztup1 = z2zE2959;
        z3zE2721.ztup2 = z2zE2960;
        z8zE165 = z3zE2721;
      } else {
        bool z2zE2961;
        z2zE2961 = (zh < UINT64_C(248));
        if (z2zE2961) {
          uint64_t z2zE2962;
          {    z2zE2962 = (zh - UINT64_C(192));
          }
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE2719;
          z3zE2719.ztup0 = true;
          z3zE2719.ztup1 = UINT64_C(1);
          z3zE2719.ztup2 = z2zE2962;
          z8zE165 = z3zE2719;
        } else {
          uint64_t z3zE2724;
          {    z3zE2724 = (zh - UINT64_C(247));
          }
          bool z2zE2964;
          {
            uint64_t z2zE2963;
            {    z2zE2963 = (zsource_len - UINT64_C(1));
            }
            z2zE2964 = (z2zE2963 < z3zE2724);
          }
          unit z3zE2717;
          if (z2zE2964) {
            struct zexception z2zE2965;
            CREATE(zexception)(&z2zE2965);
            zInvalidBlock(&z2zE2965, zRlpDecode);
            COPY(zexception)(current_exception, z2zE2965);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:450.24-450.53");
            KILL(zexception)(&z2zE2965);
            goto end_block_exception_3125;
            /* unreachable after throw */
            KILL(zexception)(&z2zE2965);
          } else {  z3zE2717 = UNIT;  }
          struct zByteSliceFields z3zE2725;
          z3zE2725 = zsub_slice(zb, UINT64_C(1), z3zE2724);
          uint64_t z2zE2966;
          {    z2zE2966 = (UINT64_C(1) + z3zE2724);
          }
          uint64_t z2zE2967;
          z2zE2967 = zrlp_uint64_width(z3zE2725, z3zE2724);
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE2718;
          z3zE2718.ztup0 = true;
          z3zE2718.ztup1 = z2zE2966;
          z3zE2718.ztup2 = z2zE2967;
          z8zE165 = z3zE2718;
        }
      }
    }
  }
end_function_3124: ;
  return z8zE165;
end_block_exception_3125: ;
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE786 = { .ztup0 = false, .ztup1 = UINT64_C(0xdeadc0de), .ztup2 = UINT64_C(0xdeadc0de) };
  return z8zE786;
}

struct zByteSliceFields zrlp_ref_cursor(struct zRlpFieldRef zf)
{
  struct zByteSliceFields z8zE166;
  bool z2zE2942;
  z2zE2942 = zf.zis_list;
  if (z2zE2942) {
    struct zByteSliceFields z2zE2946;
    z2zE2946 = zf.zsource;
    uint64_t z2zE2947;
    {
      uint64_t z2zE2944;
      {
        struct zByteSliceFields z2zE2943;
        z2zE2943 = zf.zsource;
        z2zE2944 = z2zE2943.zlen;
      }
      uint64_t z2zE2945;
      z2zE2945 = zf.zcontent_len;
      {    z2zE2947 = (z2zE2944 - z2zE2945);
      }
    }
    uint64_t z2zE2948;
    z2zE2948 = zf.zcontent_len;
    z8zE166 = zsub_slice(z2zE2946, z2zE2947, z2zE2948);
  } else {
    struct zexception z2zE2949;
    CREATE(zexception)(&z2zE2949);
    zInvalidBlock(&z2zE2949, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2949);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:473.8-473.37");
    KILL(zexception)(&z2zE2949);
    goto end_block_exception_3123;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2949);
  }
end_function_3122: ;
  return z8zE166;
end_block_exception_3123: ;
  struct zByteSliceFields z8zE787 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE787;
}

struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 zrlp_cursor_pop(struct zByteSliceFields zcursor)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z8zE167;
  bool z2zE2925;
  {
    uint64_t z2zE2924;
    z2zE2924 = zcursor.zlen;
    z2zE2925 = (z2zE2924 == UINT64_C(0));
  }
  unit z3zE2704;
  if (z2zE2925) {
    struct zexception z2zE2926;
    CREATE(zexception)(&z2zE2926);
    zInvalidBlock(&z2zE2926, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2926);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:489.8-489.37");
    KILL(zexception)(&z2zE2926);
    goto end_block_exception_3121;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2926);
  } else {  z3zE2704 = UNIT;  }
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE2927;
  {
    z2zE2927 = zrlp_ref_hdr(zcursor);
    if (have_exception) {  goto end_block_exception_3121;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z3zE2705;
  {
    bool zis_list;
    zis_list = z2zE2927.ztup0;
    uint64_t zcontent_off;
    zcontent_off = z2zE2927.ztup1;
    uint64_t zcontent_len_value;
    zcontent_len_value = z2zE2927.ztup2;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z3zE2706;
    {
      uint64_t zcontent_len;
      zcontent_len = zcontent_len_value;
      bool z2zE2929;
      {
        uint64_t z2zE2928;
        z2zE2928 = zcursor.zlen;
        z2zE2929 = (z2zE2928 < zcontent_off);
      }
      unit z3zE2708;
      if (z2zE2929) {
        struct zexception z2zE2930;
        CREATE(zexception)(&z2zE2930);
        zInvalidBlock(&z2zE2930, zRlpDecode);
        COPY(zexception)(current_exception, z2zE2930);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:494.8-494.37");
        KILL(zexception)(&z2zE2930);
        goto end_block_exception_3121;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2930);
      } else {  z3zE2708 = UNIT;  }
      bool z2zE2933;
      {
        uint64_t z2zE2932;
        {
          uint64_t z2zE2931;
          z2zE2931 = zcursor.zlen;
          {    z2zE2932 = (z2zE2931 - zcontent_off);
          }
        }
        z2zE2933 = (z2zE2932 < zcontent_len);
      }
      unit z3zE2707;
      if (z2zE2933) {
        struct zexception z2zE2934;
        CREATE(zexception)(&z2zE2934);
        zInvalidBlock(&z2zE2934, zRlpDecode);
        COPY(zexception)(current_exception, z2zE2934);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:497.8-497.37");
        KILL(zexception)(&z2zE2934);
        goto end_block_exception_3121;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2934);
      } else {  z3zE2707 = UNIT;  }
      uint64_t z2zE2935;
      {    z2zE2935 = (zcontent_off + zcontent_len);
      }
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z3zE2709;
      {
        uint64_t zfull_len;
        zfull_len = z2zE2935;
        bool z2zE2938;
        {
          bool z2zE2937;
          z2zE2937 = (UINT64_C(0) < zfull_len);
          bool z3zE2710;
          if (z2zE2937) {
            uint64_t z2zE2936;
            z2zE2936 = zcursor.zlen;
            z3zE2710 = (!(z2zE2936 < zfull_len));
          } else {  z3zE2710 = false;  }
          z2zE2938 = z3zE2710;
        }
        if (z2zE2938) {
          struct zByteSliceFields zfield_source;
          zfield_source = zsub_slice(zcursor, UINT64_C(0), zfull_len);
          struct zRlpFieldRef zfield;
          {
            struct zRlpFieldRef z3zE2712;
            z3zE2712.zcontent_len = zcontent_len;
            z3zE2712.zis_list = zis_list;
            z3zE2712.zsource = zfield_source;
            zfield = z3zE2712;
          }
          struct zByteSliceFields zadvanced;
          {
            uint64_t z2zE2940;
            {
              uint64_t z2zE2939;
              z2zE2939 = zcursor.zlen;
              {    z2zE2940 = (z2zE2939 - zfull_len);
              }
            }
            zadvanced = zsub_slice(zcursor, zfull_len, z2zE2940);
          }
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z3zE2713;
          z3zE2713.ztup0 = zfield;
          z3zE2713.ztup1 = zadvanced;
          z3zE2709 = z3zE2713;
        } else {
          struct zexception z2zE2941;
          CREATE(zexception)(&z2zE2941);
          zInvalidBlock(&z2zE2941, zRlpDecode);
          COPY(zexception)(current_exception, z2zE2941);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:510.8-510.37");
          KILL(zexception)(&z2zE2941);
          goto end_block_exception_3121;
          /* unreachable after throw */
          KILL(zexception)(&z2zE2941);
        }
        goto finish_match_3118;
      }
    case_3119: ;
      sail_match_failure("rlp_cursor_pop");
    finish_match_3118: ;
      z3zE2706 = z3zE2709;
      goto finish_match_3116;
    }
  case_3117: ;
    sail_match_failure("rlp_cursor_pop");
  finish_match_3116: ;
    z3zE2705 = z3zE2706;
    goto finish_match_3114;
  }
case_3115: ;
  sail_match_failure("rlp_cursor_pop");
finish_match_3114: ;
  z8zE167 = z3zE2705;
end_function_3120: ;
  return z8zE167;
end_block_exception_3121: ;
  struct zByteSliceFields z8zE791 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRef z8zE790 = { .zcontent_len = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE791 };
  struct zByteSliceFields z8zE789 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z8zE788 = { .ztup0 = z8zE790, .ztup1 = z8zE789 };
  return z8zE788;
}

unit zrlp_cursor_expect_end(struct zByteSliceFields zcursor)
{
  unit z8zE168;
  bool z2zE2922;
  {
    uint64_t z2zE2921;
    z2zE2921 = zcursor.zlen;
    z2zE2922 = (z2zE2921 == UINT64_C(0));
  }
  if (z2zE2922) {  z8zE168 = UNIT;  } else {
    struct zexception z2zE2923;
    CREATE(zexception)(&z2zE2923);
    zInvalidBlock(&z2zE2923, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2923);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:524.8-524.37");
    KILL(zexception)(&z2zE2923);
    goto end_block_exception_3113;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2923);
  }
end_function_3112: ;
  return z8zE168;
end_block_exception_3113: ;

  return UNIT;
}

struct zRlpFieldRef zrlp_single_ref(struct zByteSliceFields zitem)
{
  struct zRlpFieldRef z8zE169;
  uint64_t zitem_length;
  zitem_length = zitem.zlen;
  bool z2zE2913;
  z2zE2913 = (zitem_length == UINT64_C(0));
  if (z2zE2913) {
    struct zexception z2zE2914;
    CREATE(zexception)(&z2zE2914);
    zInvalidBlock(&z2zE2914, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2914);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:537.8-537.37");
    KILL(zexception)(&z2zE2914);
    goto end_block_exception_3111;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2914);
  } else {
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE2915;
    {
      z2zE2915 = zrlp_ref_hdr(zitem);
      if (have_exception) {  goto end_block_exception_3111;  }
    }
    struct zRlpFieldRef z3zE2697;
    {
      bool zis_list;
      zis_list = z2zE2915.ztup0;
      uint64_t zcontent_off;
      zcontent_off = z2zE2915.ztup1;
      uint64_t zcontent_len_value;
      zcontent_len_value = z2zE2915.ztup2;
      struct zRlpFieldRef z3zE2698;
      {
        uint64_t zcontent_len;
        zcontent_len = zcontent_len_value;
        bool z2zE2916;
        z2zE2916 = (zitem_length < zcontent_off);
        unit z3zE2700;
        if (z2zE2916) {
          struct zexception z2zE2917;
          CREATE(zexception)(&z2zE2917);
          zInvalidBlock(&z2zE2917, zRlpDecode);
          COPY(zexception)(current_exception, z2zE2917);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:542.12-542.41");
          KILL(zexception)(&z2zE2917);
          goto end_block_exception_3111;
          /* unreachable after throw */
          KILL(zexception)(&z2zE2917);
        } else {  z3zE2700 = UNIT;  }
        bool z2zE2919;
        {
          uint64_t z2zE2918;
          {    z2zE2918 = (zitem_length - zcontent_off);
          }
          z2zE2919 = (zcontent_len != z2zE2918);
        }
        unit z3zE2699;
        if (z2zE2919) {
          struct zexception z2zE2920;
          CREATE(zexception)(&z2zE2920);
          zInvalidBlock(&z2zE2920, zRlpDecode);
          COPY(zexception)(current_exception, z2zE2920);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:545.12-545.41");
          KILL(zexception)(&z2zE2920);
          goto end_block_exception_3111;
          /* unreachable after throw */
          KILL(zexception)(&z2zE2920);
        } else {  z3zE2699 = UNIT;  }
        struct zRlpFieldRef zfield;
        {
          struct zRlpFieldRef z3zE2701;
          z3zE2701.zcontent_len = zcontent_len;
          z3zE2701.zis_list = zis_list;
          z3zE2701.zsource = zitem;
          zfield = z3zE2701;
        }
        z3zE2698 = zfield;
        goto finish_match_3108;
      }
    case_3109: ;
      sail_match_failure("rlp_single_ref");
    finish_match_3108: ;
      z3zE2697 = z3zE2698;
      goto finish_match_3106;
    }
  case_3107: ;
    sail_match_failure("rlp_single_ref");
  finish_match_3106: ;
    z8zE169 = z3zE2697;
  }
end_function_3110: ;
  return z8zE169;
end_block_exception_3111: ;
  struct zByteSliceFields z8zE793 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRef z8zE792 = { .zcontent_len = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE793 };
  return z8zE792;
}

struct zByteSliceFields zrlp_node_cursor(struct zByteSliceFields znode)
{
  struct zByteSliceFields z8zE170;
  struct zRlpFieldRef z2zE2912;
  {
    z2zE2912 = zrlp_single_ref(znode);
    if (have_exception) {  goto end_block_exception_3105;  }
  }
  {
    z8zE170 = zrlp_ref_cursor(z2zE2912);
    if (have_exception) {  goto end_block_exception_3105;  }
  }
end_function_3104: ;
  return z8zE170;
end_block_exception_3105: ;
  struct zByteSliceFields z8zE794 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE794;
}

struct zByteSliceFields zrlp_ref_content(struct zRlpFieldRef zf)
{
  struct zByteSliceFields z8zE171;
  struct zByteSliceFields z2zE2909;
  z2zE2909 = zf.zsource;
  uint64_t z2zE2910;
  {
    uint64_t z2zE2907;
    {
      struct zByteSliceFields z2zE2906;
      z2zE2906 = zf.zsource;
      z2zE2907 = z2zE2906.zlen;
    }
    uint64_t z2zE2908;
    z2zE2908 = zf.zcontent_len;
    {    z2zE2910 = (z2zE2907 - z2zE2908);
    }
  }
  uint64_t z2zE2911;
  z2zE2911 = zf.zcontent_len;
  z8zE171 = zsub_slice(z2zE2909, z2zE2910, z2zE2911);
end_function_3102: ;
  return z8zE171;
end_block_exception_3103: ;
  struct zByteSliceFields z8zE795 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE795;
}

bool zrlp_ref_framing_canonical(struct zRlpFieldRef zf)
{
  bool z8zE172;
  uint64_t zn;
  zn = zf.zcontent_len;
  uint64_t zpayload_length;
  zpayload_length = zn;
  uint64_t zfull_length;
  {
    struct zByteSliceFields z2zE2905;
    z2zE2905 = zf.zsource;
    zfull_length = z2zE2905.zlen;
  }
  uint64_t zcontent_offset;
  {    zcontent_offset = (zfull_length - zpayload_length);
  }
  struct zByteSliceFields zsource;
  zsource = zf.zsource;
  uint64_t zsource_length;
  zsource_length = zsource.zlen;
  bool z2zE2854;
  z2zE2854 = (zfull_length == UINT64_C(0));
  if (z2zE2854) {  z8zE172 = false;  } else {
    bool z2zE2855;
    z2zE2855 = zf.zis_list;
    if (z2zE2855) {
      bool z2zE2856;
      z2zE2856 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
      if (z2zE2856) {
        bool z2zE2862;
        {
          uint64_t z2zE2857;
          {    z2zE2857 = (zpayload_length + UINT64_C(1));
          }
          z2zE2862 = (zfull_length == z2zE2857);
        }
        bool z3zE2694;
        if (z2zE2862) {
          uint64_t z2zE2860;
          {
            struct zByteSliceFields z2zE2858;
            z2zE2858 = zf.zsource;
            z2zE2860 = zslice_byte(z2zE2858, UINT64_C(0));
          }
          uint64_t z2zE2861;
          {
            uint64_t z2zE2859;
            z2zE2859 = zrlp_nat_length_byte(zn);
            z2zE2861 = ((UINT64_C(0xC0) + z2zE2859) & UINT64_C(0xFF));
          }
          z3zE2694 = (z2zE2860 == z2zE2861);
        } else {  z3zE2694 = false;  }
        z8zE172 = z3zE2694;
      } else {
        struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2864;
        CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2864);
        {
          sail_u256 z2zE2863;
          z2zE2863 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
          zminimal_word_bytes(&z2zE2864, z2zE2863);
        }
        bool z3zE2690;
        {
          zz5listz8z5bv8z9 zlen_bytes;
          CREATE(zz5listz8z5bv8z9)(&zlen_bytes);
          COPY(zz5listz8z5bv8z9)(&zlen_bytes, z2zE2864.ztup0);
          uint64_t zlength_width;
          zlength_width = z2zE2864.ztup1;
          bool z2zE2874;
          {
            sail_u128 z2zE2866;
            {
              uint64_t z2zE2865;
              {    z2zE2865 = (UINT64_C(1) + zlength_width);
              }
              z2zE2866 = u128_add_u64_u64(z2zE2865, zpayload_length);
            }
            z2zE2874 = u128_eq_u64(z2zE2866, zfull_length);
          }
          bool z3zE2692;
          if (z2zE2874) {
            bool z2zE2873;
            {
              uint64_t z2zE2869;
              {
                struct zByteSliceFields z2zE2867;
                z2zE2867 = zf.zsource;
                z2zE2869 = zslice_byte(z2zE2867, UINT64_C(0));
              }
              uint64_t z2zE2870;
              {
                uint64_t z2zE2868;
                z2zE2868 = zrlp_nat_length_byte(zlength_width);
                z2zE2870 = ((UINT64_C(0xF7) + z2zE2868) & UINT64_C(0xFF));
              }
              z2zE2873 = (z2zE2869 == z2zE2870);
            }
            bool z3zE2691;
            if (z2zE2873) {
              bool z2zE2871;
              z2zE2871 = (UINT64_C(0) < zsource_length);
              struct zByteSliceFields z2zE2872;
              z2zE2872 = zf.zsource;
              z3zE2691 = zrlp_bytes_equal_at(zlen_bytes, z2zE2872, UINT64_C(1));
            } else {  z3zE2691 = false;  }
            z3zE2692 = z3zE2691;
          } else {  z3zE2692 = false;  }
          z3zE2690 = z3zE2692;
          KILL(zz5listz8z5bv8z9)(&zlen_bytes);
          goto finish_match_3098;
        }
      case_3099: ;
        sail_match_failure("rlp_ref_framing_canonical");
      finish_match_3098: ;
        z8zE172 = z3zE2690;
        KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2864);
      }
    } else {
      bool z2zE2875;
      z2zE2875 = (zpayload_length == UINT64_C(0));
      if (z2zE2875) {
        bool z2zE2878;
        z2zE2878 = (zfull_length == UINT64_C(1));
        bool z3zE2689;
        if (z2zE2878) {
          uint64_t z2zE2877;
          {
            struct zByteSliceFields z2zE2876;
            z2zE2876 = zf.zsource;
            z2zE2877 = zslice_byte(z2zE2876, UINT64_C(0));
          }
          z3zE2689 = (z2zE2877 == UINT64_C(0x80));
        } else {  z3zE2689 = false;  }
        z8zE172 = z3zE2689;
      } else {
        uint64_t zfirst;
        {
          struct zByteSliceFields z2zE2904;
          z2zE2904 = zf.zsource;
          zfirst = zslice_byte(z2zE2904, zcontent_offset);
        }
        bool z2zE2881;
        {
          bool z2zE2880;
          z2zE2880 = (zpayload_length == UINT64_C(1));
          bool z3zE2682;
          if (z2zE2880) {
            uint64_t z2zE2879;
            z2zE2879 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
            z3zE2682 = (z2zE2879 == UINT64_C(0b0));
          } else {  z3zE2682 = false;  }
          z2zE2881 = z3zE2682;
        }
        if (z2zE2881) {
          bool z2zE2884;
          z2zE2884 = (zfull_length == UINT64_C(1));
          bool z3zE2688;
          if (z2zE2884) {
            uint64_t z2zE2883;
            {
              struct zByteSliceFields z2zE2882;
              z2zE2882 = zf.zsource;
              z2zE2883 = zslice_byte(z2zE2882, UINT64_C(0));
            }
            z3zE2688 = (z2zE2883 == zfirst);
          } else {  z3zE2688 = false;  }
          z8zE172 = z3zE2688;
        } else {
          bool z2zE2885;
          z2zE2885 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
          if (z2zE2885) {
            bool z2zE2891;
            {
              uint64_t z2zE2886;
              {    z2zE2886 = (zpayload_length + UINT64_C(1));
              }
              z2zE2891 = (zfull_length == z2zE2886);
            }
            bool z3zE2687;
            if (z2zE2891) {
              uint64_t z2zE2889;
              {
                struct zByteSliceFields z2zE2887;
                z2zE2887 = zf.zsource;
                z2zE2889 = zslice_byte(z2zE2887, UINT64_C(0));
              }
              uint64_t z2zE2890;
              {
                uint64_t z2zE2888;
                z2zE2888 = zrlp_nat_length_byte(zn);
                z2zE2890 = ((UINT64_C(0x80) + z2zE2888) & UINT64_C(0xFF));
              }
              z3zE2687 = (z2zE2889 == z2zE2890);
            } else {  z3zE2687 = false;  }
            z8zE172 = z3zE2687;
          } else {
            struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2893;
            CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2893);
            {
              sail_u256 z2zE2892;
              z2zE2892 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
              zminimal_word_bytes(&z2zE2893, z2zE2892);
            }
            bool z3zE2683;
            {
              zz5listz8z5bv8z9 z3zE2695;
              CREATE(zz5listz8z5bv8z9)(&z3zE2695);
              COPY(zz5listz8z5bv8z9)(&z3zE2695, z2zE2893.ztup0);
              uint64_t z3zE2696;
              z3zE2696 = z2zE2893.ztup1;
              bool z2zE2903;
              {
                sail_u128 z2zE2895;
                {
                  uint64_t z2zE2894;
                  {    z2zE2894 = (UINT64_C(1) + z3zE2696);
                  }
                  z2zE2895 = u128_add_u64_u64(z2zE2894, zpayload_length);
                }
                z2zE2903 = u128_eq_u64(z2zE2895, zfull_length);
              }
              bool z3zE2685;
              if (z2zE2903) {
                bool z2zE2902;
                {
                  uint64_t z2zE2898;
                  {
                    struct zByteSliceFields z2zE2896;
                    z2zE2896 = zf.zsource;
                    z2zE2898 = zslice_byte(z2zE2896, UINT64_C(0));
                  }
                  uint64_t z2zE2899;
                  {
                    uint64_t z2zE2897;
                    z2zE2897 = zrlp_nat_length_byte(z3zE2696);
                    z2zE2899 = ((UINT64_C(0xB7) + z2zE2897) & UINT64_C(0xFF));
                  }
                  z2zE2902 = (z2zE2898 == z2zE2899);
                }
                bool z3zE2684;
                if (z2zE2902) {
                  bool z2zE2900;
                  z2zE2900 = (UINT64_C(0) < zsource_length);
                  struct zByteSliceFields z2zE2901;
                  z2zE2901 = zf.zsource;
                  z3zE2684 = zrlp_bytes_equal_at(z3zE2695, z2zE2901, UINT64_C(1));
                } else {  z3zE2684 = false;  }
                z3zE2685 = z3zE2684;
              } else {  z3zE2685 = false;  }
              z3zE2683 = z3zE2685;
              KILL(zz5listz8z5bv8z9)(&z3zE2695);
              goto finish_match_3096;
            }
          case_3097: ;
            sail_match_failure("rlp_ref_framing_canonical");
          finish_match_3096: ;
            z8zE172 = z3zE2683;
            KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2893);
          }
        }
      }
    }
  }
end_function_3100: ;
  return z8zE172;
end_block_exception_3101: ;

  return false;
}

bool zrlp_ref_bytes_canonical(struct zRlpFieldRef zf)
{
  bool z8zE173;
  bool z2zE2853;
  {
    bool z2zE2852;
    z2zE2852 = zf.zis_list;
    z2zE2853 = not(z2zE2852);
  }
  bool z3zE2681;
  if (z2zE2853) {  z3zE2681 = zrlp_ref_framing_canonical(zf);  } else {  z3zE2681 = false;  }
  z8zE173 = z3zE2681;
end_function_3094: ;
  return z8zE173;
end_block_exception_3095: ;

  return false;
}

bool zrlp_ref_uint_canonical(struct zRlpFieldRef zf)
{
  bool z8zE174;
  bool z2zE2843;
  {
    bool z2zE2842;
    z2zE2842 = zrlp_ref_bytes_canonical(zf);
    z2zE2843 = not(z2zE2842);
  }
  if (z2zE2843) {  z8zE174 = false;  } else {
    bool z2zE2851;
    {
      uint64_t z2zE2844;
      z2zE2844 = zf.zcontent_len;
      z2zE2851 = (z2zE2844 == UINT64_C(0));
    }
    bool z3zE2680;
    if (z2zE2851) {  z3zE2680 = true;  } else {
      uint64_t z2zE2850;
      {
        struct zByteSliceFields z2zE2848;
        z2zE2848 = zf.zsource;
        uint64_t z2zE2849;
        {
          uint64_t z2zE2846;
          {
            struct zByteSliceFields z2zE2845;
            z2zE2845 = zf.zsource;
            z2zE2846 = z2zE2845.zlen;
          }
          uint64_t z2zE2847;
          z2zE2847 = zf.zcontent_len;
          {    z2zE2849 = (z2zE2846 - z2zE2847);
          }
        }
        z2zE2850 = zslice_byte(z2zE2848, z2zE2849);
      }
      z3zE2680 = (z2zE2850 != UINT64_C(0x00));
    }
    z8zE174 = z3zE2680;
  }
end_function_3092: ;
  return z8zE174;
end_block_exception_3093: ;

  return false;
}

sail_u256 zrlp_ref_word(struct zRlpFieldRef zf)
{
  sail_u256 z8zE175;
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE2836;
  {
    bool z2zE2835;
    z2zE2835 = zf.zis_list;
    bool z3zE2679;
    if (z2zE2835) {  z3zE2679 = true;  } else {  z3zE2679 = (zRLP_WORD_LENGTH_LIMIT < zn);  }
    z2zE2836 = z3zE2679;
  }
  if (z2zE2836) {
    struct zexception z2zE2837;
    CREATE(zexception)(&z2zE2837);
    zInvalidBlock(&z2zE2837, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2837);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:659.8-659.37");
    KILL(zexception)(&z2zE2837);
    goto end_block_exception_3091;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2837);
  } else {
    struct zByteSliceFields z2zE2840;
    z2zE2840 = zf.zsource;
    uint64_t z2zE2841;
    {
      uint64_t z2zE2839;
      {
        struct zByteSliceFields z2zE2838;
        z2zE2838 = zf.zsource;
        z2zE2839 = z2zE2838.zlen;
      }
      {    z2zE2841 = (z2zE2839 - zn);
      }
    }
    z8zE175 = zslice_load_n(z2zE2840, z2zE2841, zn);
  }
end_function_3090: ;
  return z8zE175;
end_block_exception_3091: ;

  return u256_zero();
}

sail_u256 zrlp_ref_uint_word(struct zRlpFieldRef zf)
{
  sail_u256 z8zE176;
  bool z2zE2833;
  z2zE2833 = zrlp_ref_uint_canonical(zf);
  if (z2zE2833) {
    {
      z8zE176 = zrlp_ref_word(zf);
      if (have_exception) {  goto end_block_exception_3089;  }
    }
  } else {
    struct zexception z2zE2834;
    CREATE(zexception)(&z2zE2834);
    zInvalidBlock(&z2zE2834, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2834);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:675.8-675.37");
    KILL(zexception)(&z2zE2834);
    goto end_block_exception_3089;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2834);
  }
end_function_3088: ;
  return z8zE176;
end_block_exception_3089: ;

  return u256_zero();
}

uint64_t zrlp_ref_uint64(struct zRlpFieldRef zf)
{
  uint64_t z8zE177;
  bool z2zE2831;
  {
    bool z2zE2830;
    {
      bool z2zE2828;
      z2zE2828 = zrlp_ref_uint_canonical(zf);
      z2zE2830 = not(z2zE2828);
    }
    bool z3zE2677;
    if (z2zE2830) {  z3zE2677 = true;  } else {
      uint64_t z2zE2829;
      z2zE2829 = zf.zcontent_len;
      z3zE2677 = (zRLP_UINT64_LENGTH_LIMIT < z2zE2829);
    }
    z2zE2831 = z3zE2677;
  }
  unit z3zE2678;
  if (z2zE2831) {
    struct zexception z2zE2832;
    CREATE(zexception)(&z2zE2832);
    zInvalidBlock(&z2zE2832, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2832);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:686.8-686.37");
    KILL(zexception)(&z2zE2832);
    goto end_block_exception_3087;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2832);
  } else {  z3zE2678 = UNIT;  }
  uint64_t zcontent_length;
  zcontent_length = zf.zcontent_len;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zf);
  uint64_t zwidth;
  {    zwidth = (zcontent_length % UINT64_C(9));
  }
  z8zE177 = zrlp_uint64_width(zcontent, zwidth);
end_function_3086: ;
  return z8zE177;
end_block_exception_3087: ;

  return UINT64_C(0xdeadc0de);
}

