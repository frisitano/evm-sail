/* Generated from sail/lib/ssz/ssz.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_105(void) {

  uint64_t z3zE116;
  z3zE116 = UINT64_C(4);
  zSSZ_OFF_BYTES = z3zE116;
let_end_242: ;
}
void kill_letbind_105(void) {
}

void create_letbind_106(void) {

  uint64_t z3zE117;
  z3zE117 = UINT64_C(8);
  zSSZ_UINT_BYTES = z3zE117;
let_end_243: ;
}
void kill_letbind_106(void) {
}

uint64_t zsszz_field_offset(uint64_t zbase, uint64_t zdelta)
{
  uint64_t z8zE134;
  {    z8zE134 = (zbase + zdelta);
  }
end_function_3237: ;
  return z8zE134;
end_block_exception_3238: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_u32_at(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE135;
  uint64_t zb0;
  {
    uint64_t z2zE3099;
    z2zE3099 = zslice_byte(zinput, zoffset);
    zb0 = z2zE3099;
  }
  uint64_t zb1;
  {
    uint64_t z2zE3098;
    {
      uint64_t z2zE3097;
      z2zE3097 = zsszz_field_offset(zoffset, UINT64_C(1));
      z2zE3098 = zslice_byte(zinput, z2zE3097);
    }
    zb1 = z2zE3098;
  }
  uint64_t zb2;
  {
    uint64_t z2zE3096;
    {
      uint64_t z2zE3095;
      z2zE3095 = zsszz_field_offset(zoffset, UINT64_C(2));
      z2zE3096 = zslice_byte(zinput, z2zE3095);
    }
    zb2 = z2zE3096;
  }
  uint64_t zb3;
  {
    uint64_t z2zE3094;
    {
      uint64_t z2zE3093;
      z2zE3093 = zsszz_field_offset(zoffset, UINT64_C(3));
      z2zE3094 = zslice_byte(zinput, z2zE3093);
    }
    zb3 = z2zE3094;
  }
  uint64_t z2zE3092;
  {
    uint64_t z2zE3091;
    {
      uint64_t z2zE3089;
      z2zE3089 = ((UINT64_C(8) >= UINT64_C(64)) ? UINT64_C(0) : ((zb1 << UINT64_C(8)) & UINT64_C(0xFFFFFFFF)));
      uint64_t z2zE3090;
      {
        uint64_t z2zE3087;
        z2zE3087 = ((UINT64_C(16) >= UINT64_C(64)) ? UINT64_C(0) : ((zb2 << UINT64_C(16)) & UINT64_C(0xFFFFFFFF)));
        uint64_t z2zE3088;
        z2zE3088 = ((UINT64_C(24) >= UINT64_C(64)) ? UINT64_C(0) : ((zb3 << UINT64_C(24)) & UINT64_C(0xFFFFFFFF)));
        z2zE3090 = (z2zE3087 | z2zE3088);
      }
      z2zE3091 = (z2zE3089 | z2zE3090);
    }
    z2zE3092 = (zb0 | z2zE3091);
  }
  z8zE135 = ((uint64_t) z2zE3092);
end_function_3235: ;
  return z8zE135;
end_block_exception_3236: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_u32(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE136;
  z8zE136 = zsszz_u32_at(zinput, zoffset);
end_function_3233: ;
  return z8zE136;
end_block_exception_3234: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_offset_to_source_pointer(uint64_t zvalue)
{
  uint64_t z8zE137;
  z8zE137 = zvalue;
end_function_3231: ;
  return z8zE137;
end_block_exception_3232: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zdecode_sszz_uint(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE138;
  uint64_t z2zE3085;
  {
    uint64_t z2zE3080;
    {
      uint64_t z2zE3075;
      {
        uint64_t z2zE3070;
        {
          uint64_t z2zE3065;
          {
            uint64_t z2zE3060;
            {
              uint64_t z2zE3055;
              {
                uint64_t z2zE3051;
                z2zE3051 = zslice_byte(zinput, zoffset);
                z2zE3055 = ((uint64_t) z2zE3051);
              }
              uint64_t z2zE3056;
              {
                uint64_t z2zE3054;
                {
                  uint64_t z2zE3053;
                  {
                    uint64_t z2zE3052;
                    z2zE3052 = zsszz_field_offset(zoffset, UINT64_C(1));
                    z2zE3053 = zslice_byte(zinput, z2zE3052);
                  }
                  z2zE3054 = ((uint64_t) z2zE3053);
                }
                {    z2zE3056 = (z2zE3054 * UINT64_C(256));
                }
              }
              {    z2zE3060 = (z2zE3055 + z2zE3056);
              }
            }
            uint64_t z2zE3061;
            {
              uint64_t z2zE3059;
              {
                uint64_t z2zE3058;
                {
                  uint64_t z2zE3057;
                  z2zE3057 = zsszz_field_offset(zoffset, UINT64_C(2));
                  z2zE3058 = zslice_byte(zinput, z2zE3057);
                }
                z2zE3059 = ((uint64_t) z2zE3058);
              }
              {    z2zE3061 = (z2zE3059 * UINT64_C(65536));
              }
            }
            {    z2zE3065 = (z2zE3060 + z2zE3061);
            }
          }
          uint64_t z2zE3066;
          {
            uint64_t z2zE3064;
            {
              uint64_t z2zE3063;
              {
                uint64_t z2zE3062;
                z2zE3062 = zsszz_field_offset(zoffset, UINT64_C(3));
                z2zE3063 = zslice_byte(zinput, z2zE3062);
              }
              z2zE3064 = ((uint64_t) z2zE3063);
            }
            {    z2zE3066 = (z2zE3064 * UINT64_C(16777216));
            }
          }
          {    z2zE3070 = (z2zE3065 + z2zE3066);
          }
        }
        uint64_t z2zE3071;
        {
          uint64_t z2zE3069;
          {
            uint64_t z2zE3068;
            {
              uint64_t z2zE3067;
              z2zE3067 = zsszz_field_offset(zoffset, zSSZ_OFF_BYTES);
              z2zE3068 = zslice_byte(zinput, z2zE3067);
            }
            z2zE3069 = ((uint64_t) z2zE3068);
          }
          {    z2zE3071 = (z2zE3069 * UINT64_C(4294967296));
          }
        }
        {    z2zE3075 = (z2zE3070 + z2zE3071);
        }
      }
      uint64_t z2zE3076;
      {
        uint64_t z2zE3074;
        {
          uint64_t z2zE3073;
          {
            uint64_t z2zE3072;
            z2zE3072 = zsszz_field_offset(zoffset, UINT64_C(5));
            z2zE3073 = zslice_byte(zinput, z2zE3072);
          }
          z2zE3074 = ((uint64_t) z2zE3073);
        }
        {    z2zE3076 = (z2zE3074 * UINT64_C(1099511627776));
        }
      }
      {    z2zE3080 = (z2zE3075 + z2zE3076);
      }
    }
    uint64_t z2zE3081;
    {
      uint64_t z2zE3079;
      {
        uint64_t z2zE3078;
        {
          uint64_t z2zE3077;
          z2zE3077 = zsszz_field_offset(zoffset, UINT64_C(6));
          z2zE3078 = zslice_byte(zinput, z2zE3077);
        }
        z2zE3079 = ((uint64_t) z2zE3078);
      }
      {    z2zE3081 = (z2zE3079 * UINT64_C(281474976710656));
      }
    }
    {    z2zE3085 = (z2zE3080 + z2zE3081);
    }
  }
  uint64_t z2zE3086;
  {
    uint64_t z2zE3084;
    {
      uint64_t z2zE3083;
      {
        uint64_t z2zE3082;
        z2zE3082 = zsszz_field_offset(zoffset, UINT64_C(7));
        z2zE3083 = zslice_byte(zinput, z2zE3082);
      }
      z2zE3084 = ((uint64_t) z2zE3083);
    }
    {    z2zE3086 = (z2zE3084 * UINT64_C(72057594037927936));
    }
  }
  {    z8zE138 = (z2zE3085 + z2zE3086);
  }
end_function_3229: ;
  return z8zE138;
end_block_exception_3230: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zsszz_addr(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_20 z8zE139;
  sail_u256 z2zE3050;
  z2zE3050 = zslice_load_n(zinput, zoffset, zADDRESS_BYTE_LENGTH);
  z8zE139 = evmsail_word_to_address(z2zE3050);
end_function_3227: ;
  return z8zE139;
end_block_exception_3228: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_32 zsszz_bytes32(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_32 z8zE140;
  sail_u256 z2zE3049;
  z2zE3049 = zslice_load(zinput, zoffset);
  z8zE140 = evmsail_word_to_hash(z2zE3049);
end_function_3225: ;
  return z8zE140;
end_block_exception_3226: ;

  return fixed_bytes_32_zero();
}

uint64_t zsszz_logs_bloom_index(uint64_t zindex)
{
  uint64_t z8zE141;
  {    z8zE141 = (UINT64_C(255) - zindex);
  }
end_function_3223: ;
  return z8zE141;
end_block_exception_3224: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_256 zsszz_logs_bloom(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_256 z8zE142;
  sail_fixed_bytes_256 zout;
  {
    zz5vecz8z5bv8z9 z3zE2788;
    CREATE(zz5vecz8z5bv8z9)(&z3zE2788);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE2788, INT64_C(256));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(31), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(32), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(33), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(34), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(35), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(36), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(37), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(38), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(39), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(40), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(41), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(42), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(43), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(44), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(45), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(46), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(47), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(48), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(49), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(50), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(51), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(52), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(53), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(54), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(55), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(56), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(57), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(58), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(59), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(60), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(61), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(62), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(63), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(64), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(65), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(66), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(67), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(68), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(69), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(70), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(71), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(72), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(73), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(74), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(75), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(76), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(77), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(78), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(79), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(80), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(81), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(82), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(83), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(84), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(85), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(86), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(87), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(88), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(89), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(90), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(91), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(92), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(93), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(94), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(95), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(96), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(97), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(98), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(99), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(100), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(101), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(102), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(103), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(104), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(105), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(106), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(107), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(108), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(109), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(110), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(111), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(112), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(113), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(114), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(115), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(116), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(117), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(118), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(119), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(120), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(121), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(122), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(123), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(124), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(125), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(126), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(127), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(128), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(129), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(130), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(131), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(132), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(133), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(134), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(135), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(136), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(137), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(138), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(139), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(140), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(141), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(142), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(143), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(144), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(145), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(146), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(147), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(148), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(149), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(150), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(151), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(152), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(153), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(154), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(155), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(156), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(157), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(158), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(159), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(160), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(161), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(162), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(163), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(164), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(165), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(166), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(167), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(168), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(169), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(170), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(171), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(172), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(173), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(174), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(175), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(176), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(177), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(178), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(179), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(180), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(181), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(182), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(183), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(184), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(185), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(186), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(187), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(188), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(189), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(190), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(191), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(192), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(193), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(194), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(195), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(196), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(197), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(198), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(199), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(200), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(201), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(202), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(203), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(204), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(205), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(206), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(207), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(208), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(209), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(210), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(211), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(212), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(213), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(214), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(215), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(216), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(217), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(218), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(219), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(220), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(221), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(222), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(223), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(224), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(225), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(226), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(227), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(228), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(229), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(230), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(231), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(232), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(233), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(234), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(235), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(236), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(237), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(238), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(239), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(240), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(241), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(242), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(243), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(244), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(245), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(246), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(247), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(248), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(249), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(250), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(251), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(252), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(253), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(254), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE2788, z3zE2788, INT64_C(255), UINT64_C(0x00));
    for (size_t z8zE784 = 0; z8zE784 < 256; ++z8zE784) {
      zout.bytes[z8zE784] = (uint8_t)(z3zE2788.data[z8zE784] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE2788);
  }
  int64_t z3zE2789;
  {    z3zE2789 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2790;
  {    z3zE2790 = (int64_t)(UINT64_C(255));
  }
  int64_t z3zE2791;
  {    z3zE2791 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE2789;
    unit z3zE2792;
  for_start_3219: ;
    {
      if ((z3zE2790 < zk)) goto for_end_3220;
      uint64_t z2zE3047;
      {
        uint64_t z3zE3017;
        z3zE3017 = (uint64_t)(zk);
        z2zE3047 = zsszz_logs_bloom_index(z3zE3017);
      }
      uint64_t z2zE3048;
      {
        uint64_t z2zE3046;
        {
          uint64_t z3zE3016;
          z3zE3016 = (uint64_t)(zk);
          z2zE3046 = zsszz_field_offset(zoffset, z3zE3016);
        }
        z2zE3048 = zslice_byte(zinput, z2zE3046);
      }
      zout = fast_unsigned_vector_update_fixed_bytes_256(zout, z2zE3047, z2zE3048);
      z3zE2792 = UNIT;
      zk = (zk + z3zE2791);
      goto for_start_3219;
    }
  for_end_3220: ;
  }
  unit z3zE2793;
  z3zE2793 = UNIT;
  z8zE142 = zout;
end_function_3221: ;
  return z8zE142;
end_block_exception_3222: ;

  return fixed_bytes_256_zero();
}

uint64_t zsszz_u256_index(uint64_t zindex)
{
  uint64_t z8zE143;
  {    z8zE143 = (UINT64_C(31) - zindex);
  }
end_function_3217: ;
  return z8zE143;
end_block_exception_3218: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zsszz_u256(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_u256 z8zE144;
  sail_u256 zresult;
  zresult = zWORD_ZERO;
  int64_t z3zE2783;
  {    z3zE2783 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE2784;
  {    z3zE2784 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE2785;
  {    z3zE2785 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE2783;
    unit z3zE2786;
  for_start_3213: ;
    {
      if ((z3zE2784 < zk)) goto for_end_3214;
      sail_u256 z2zE3044;
      z2zE3044 = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresult, UINT64_C(256));
      uint64_t z2zE3045;
      {
        uint64_t z2zE3043;
        {
          uint64_t z2zE3042;
          {
            uint64_t z2zE3041;
            {
              uint64_t z3zE3018;
              z3zE3018 = (uint64_t)(zk);
              z2zE3041 = zsszz_u256_index(z3zE3018);
            }
            z2zE3042 = zsszz_field_offset(zoffset, z2zE3041);
          }
          z2zE3043 = zslice_byte(zinput, z2zE3042);
        }
        z2zE3045 = ((uint64_t) z2zE3043);
      }
      zresult = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3044, z2zE3045);
      z3zE2786 = UNIT;
      zk = (zk + z3zE2785);
      goto for_start_3213;
    }
  for_end_3214: ;
  }
  unit z3zE2787;
  z3zE2787 = UNIT;
  z8zE144 = zresult;
end_function_3215: ;
  return z8zE144;
end_block_exception_3216: ;

  return u256_zero();
}

