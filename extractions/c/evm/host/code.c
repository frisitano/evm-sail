/* Generated from sail/host/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zstore_jumpdest_chunk(uint64_t ztable, uint64_t zcode_len, struct zCodeAnalysis zanalysis)
{
  unit z8zE128;
  bool z2zE3133;
  {
    sail_u256 z2zE3132;
    z2zE3132 = zanalysis.zchunk;
    z2zE3133 = (!eq_u256(z2zE3132, zEMPTY_JUMPDEST_CHUNK));
  }
  if (z2zE3133) {
    bool zstored;
    {
      uint64_t z2zE3134;
      z2zE3134 = zanalysis.zchunk_index;
      sail_u256 z2zE3135;
      z2zE3135 = zanalysis.zchunk;
      zstored = jumpdest_table_store_chunk(ztable, zcode_len, z2zE3134, z2zE3135);
    }
    z8zE128 = sail_assert(zstored, "JUMPDEST chunk store");
  } else {  z8zE128 = UNIT;  }
end_function_3252: ;
  return z8zE128;
end_block_exception_3253: ;

  return UNIT;
}

sail_u256 zjumpdest_bit(uint64_t zindex)
{
  sail_u256 z8zE129;
  sail_u256 zchunk;
  zchunk = zEMPTY_JUMPDEST_CHUNK;
  zchunk = u256_update_u64(zchunk, zindex, UINT64_C(0b1));
  unit z3zE2810;
  z3zE2810 = UNIT;
  z8zE129 = zchunk;
end_function_3250: ;
  return z8zE129;
end_block_exception_3251: ;

  return u256_zero();
}

unit zanalyzze_code_from(struct zByteSliceFields zcode, enum zFork zfork, uint64_t ztable, uint64_t zpc, struct zCodeAnalysis zanalysis)
{
  unit z8zE130;
  uint64_t zposition;
  zposition = zpc;
  uint64_t zcode_len;
  zcode_len = zcode.zlen;
  bool z2zE3107;
  z2zE3107 = (zposition < zcode_len);
  if (z2zE3107) {
    sail_u256 zchunk;
    zchunk = zanalysis.zchunk;
    uint64_t zchunk_index;
    zchunk_index = zanalysis.zchunk_index;
    uint64_t zchunk_offset;
    zchunk_offset = zanalysis.zchunk_offset;
    uint64_t zopcode;
    zopcode = zslice_byte(zcode, zposition);
    bool z2zE3108;
    z2zE3108 = (zopcode == UINT64_C(0x5B));
    unit z3zE2800;
    if (z2zE3108) {
      sail_u256 z2zE3109;
      z2zE3109 = zjumpdest_bit(zchunk_offset);
      zchunk = u256_or(zchunk, z2zE3109);
      z3zE2800 = UNIT;
    } else {  z3zE2800 = UNIT;  }
    uint64_t zopcode_value;
    zopcode_value = ((uint64_t) zopcode);
    uint64_t zstep;
    {
      bool z2zE3119;
      {
        bool z2zE3118;
        z2zE3118 = (!(zopcode_value < UINT64_C(96)));
        bool z3zE2801;
        if (z2zE3118) {  z3zE2801 = (!(UINT64_C(127) < zopcode_value));  } else {  z3zE2801 = false;  }
        z2zE3119 = z3zE2801;
      }
      if (z2zE3119) {
        {    zstep = (zopcode_value - UINT64_C(94));
        }
      } else {
        bool z2zE3122;
        {
          bool z2zE3121;
          z2zE3121 = zfork_gteq(zfork, zAmsterdam);
          bool z3zE2803;
          if (z2zE3121) {
            bool z2zE3120;
            z2zE3120 = (zopcode_value == UINT64_C(230));
            bool z3zE2802;
            if (z2zE3120) {  z3zE2802 = true;  } else {  z3zE2802 = (zopcode_value == UINT64_C(231));  }
            z3zE2803 = z3zE2802;
          } else {  z3zE2803 = false;  }
          z2zE3122 = z3zE2803;
        }
        if (z2zE3122) {
          bool z2zE3125;
          {
            uint64_t z2zE3124;
            {
              uint64_t z2zE3123;
              {    z2zE3123 = (zposition + UINT64_C(1));
              }
              z2zE3124 = zslice_byte(zcode, z2zE3123);
            }
            z2zE3125 = zdeep_stack_immediate_valid(z2zE3124);
          }
          if (z2zE3125) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
        } else {
          bool z2zE3127;
          {
            bool z2zE3126;
            z2zE3126 = zfork_gteq(zfork, zAmsterdam);
            bool z3zE2804;
            if (z2zE3126) {  z3zE2804 = (zopcode_value == UINT64_C(232));  } else {  z3zE2804 = false;  }
            z2zE3127 = z3zE2804;
          }
          if (z2zE3127) {
            bool z2zE3130;
            {
              uint64_t z2zE3129;
              {
                uint64_t z2zE3128;
                {    z2zE3128 = (zposition + UINT64_C(1));
                }
                z2zE3129 = zslice_byte(zcode, z2zE3128);
              }
              z2zE3130 = zexchange_immediate_valid(z2zE3129);
            }
            if (z2zE3130) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
          } else {  zstep = UINT64_C(1);  }
        }
      }
    }
    bool z2zE3111;
    {
      uint64_t z2zE3110;
      {    z2zE3110 = (zcode_len - zposition);
      }
      z2zE3111 = (zstep < z2zE3110);
    }
    if (z2zE3111) {
      uint64_t zadded;
      {    zadded = (zposition + zstep);
      }
      uint64_t zprogressed;
      {    zprogressed = (zchunk_offset + zstep);
      }
      bool z2zE3112;
      z2zE3112 = (zprogressed < UINT64_C(256));
      if (z2zE3112) {
        struct zCodeAnalysis z3zE2809;
        z3zE2809.zchunk = zchunk;
        z3zE2809.zchunk_index = zchunk_index;
        z3zE2809.zchunk_offset = zprogressed;
        z8zE130 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z3zE2809);
      } else {
        uint64_t z2zE3113;
        z2zE3113 = zcode.zlen;
        struct zCodeAnalysis z3zE2806;
        z3zE2806.zchunk = zchunk;
        z3zE2806.zchunk_index = zchunk_index;
        z3zE2806.zchunk_offset = zchunk_offset;
        unit z3zE2807;
        z3zE2807 = zstore_jumpdest_chunk(ztable, z2zE3113, z3zE2806);
        struct zCodeAnalysis z2zE3116;
        {
          uint64_t z2zE3114;
          z2zE3114 = (zadded / UINT64_C(256));
          uint64_t z2zE3115;
          {    z2zE3115 = (zprogressed - UINT64_C(256));
          }
          struct zCodeAnalysis z3zE2808;
          z3zE2808.zchunk = zEMPTY_JUMPDEST_CHUNK;
          z3zE2808.zchunk_index = z2zE3114;
          z3zE2808.zchunk_offset = z2zE3115;
          z2zE3116 = z3zE2808;
        }
        z8zE130 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z2zE3116);
      }
    } else {
      uint64_t z2zE3117;
      z2zE3117 = zcode.zlen;
      struct zCodeAnalysis z3zE2805;
      z3zE2805.zchunk = zchunk;
      z3zE2805.zchunk_index = zchunk_index;
      z3zE2805.zchunk_offset = zchunk_offset;
      z8zE130 = zstore_jumpdest_chunk(ztable, z2zE3117, z3zE2805);
    }
  } else {
    uint64_t z2zE3131;
    z2zE3131 = zcode.zlen;
    z8zE130 = zstore_jumpdest_chunk(ztable, z2zE3131, zanalysis);
  }
end_function_3248: ;
  return z8zE130;
end_block_exception_3249: ;

  return UNIT;
}

uint64_t zanalyzze_code(struct zByteSliceFields zcode, enum zFork zfork)
{
  uint64_t z8zE131;
  bool z2zE3105;
  {
    uint64_t z2zE3104;
    z2zE3104 = zcode.zlen;
    z2zE3105 = (z2zE3104 == UINT64_C(0));
  }
  if (z2zE3105) {  z8zE131 = zEMPTY_JUMPDEST_REF;  } else {
    uint64_t zlength;
    zlength = zcode.zlen;
    uint64_t ztable;
    ztable = jumpdest_table_alloc(zlength);
    bool z2zE3106;
    z2zE3106 = (ztable != zEMPTY_JUMPDEST_REF);
    unit z3zE2799;
    z3zE2799 = sail_assert(z2zE3106, "JUMPDEST table allocation");
    struct zCodeAnalysis z3zE2797;
    z3zE2797.zchunk = zEMPTY_JUMPDEST_CHUNK;
    z3zE2797.zchunk_index = UINT64_C(0);
    z3zE2797.zchunk_offset = UINT64_C(0);
    unit z3zE2798;
    z3zE2798 = zanalyzze_code_from(zcode, zfork, ztable, UINT64_C(0), z3zE2797);
    z8zE131 = ztable;
  }
end_function_3246: ;
  return z8zE131;
end_block_exception_3247: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_32 zcode_db_insert(struct zByteSliceFields zcode, enum zFork zfork)
{
  sail_fixed_bytes_32 z8zE132;
  uint64_t z2zE3103;
  z2zE3103 = zanalyzze_code(zcode, zfork);
  z8zE132 = code_db_store_indexed(zcode, z2zE3103);
end_function_3244: ;
  return z8zE132;
end_block_exception_3245: ;

  return fixed_bytes_32_zero();
}

struct zCode zcode_db_resolve(sail_fixed_bytes_32 zcode_hash)
{
  struct zCode z8zE133;
  bool z2zE3100;
  z2zE3100 = eq_fixed_bytes_32(zcode_hash, zKECCAK_EMPTY);
  if (z2zE3100) {  z8zE133 = zEMPTY_CODE;  } else {
    struct zoptionzIRCodezK z2zE3101;
    CREATE(zoptionzIRCodezK)(&z2zE3101);
    code_db_lookup(&z2zE3101, zcode_hash);
    struct zCode z3zE2794;
    {
      if (z2zE3101.kind != Kind_zSomezIRCodezK) goto case_3241;
      struct zCode zcode;
      zcode = z2zE3101.variants.zSomezIRCodezK;
      z3zE2794 = zcode;
      goto finish_match_3239;
    }
  case_3241: ;
    {
      /* complete */
      struct zexception z2zE3102;
      CREATE(zexception)(&z2zE3102);
      zInvalidBlock(&z2zE3102, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3102);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/host/code.sail:162.22-162.58");
      KILL(zoptionzIRCodezK)(&z2zE3101);
      KILL(zexception)(&z2zE3102);
      goto end_block_exception_3243;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3102);
      goto finish_match_3239;
    }
  case_3240: ;
  finish_match_3239: ;
    z8zE133 = z3zE2794;
    KILL(zoptionzIRCodezK)(&z2zE3101);
  }
end_function_3242: ;
  return z8zE133;
end_block_exception_3243: ;
  struct zByteSliceFields z8zE783 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE782 = { .zbytes = z8zE783, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE782;
}

