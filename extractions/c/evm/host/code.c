/* Generated from sail/host/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zstore_jumpdest_chunk(uint64_t ztable, uint64_t zcode_len, struct zCodeAnalysis zanalysis)
{
  unit z8zE129;
  bool z2zE3838;
  {
    sail_u256 z2zE3837;
    z2zE3837 = zanalysis.zchunk;
    z2zE3838 = (!eq_u256(z2zE3837, zEMPTY_JUMPDEST_CHUNK));
  }
  if (z2zE3838) {
    bool zstored;
    {
      uint64_t z2zE3839;
      z2zE3839 = zanalysis.zchunk_index;
      sail_u256 z2zE3840;
      z2zE3840 = zanalysis.zchunk;
      zstored = jumpdest_table_store_chunk(ztable, zcode_len, z2zE3839, z2zE3840);
    }
    z8zE129 = sail_assert(zstored, "JUMPDEST chunk store");
  } else {  z8zE129 = UNIT;  }
end_function_3664: ;
  return z8zE129;
end_block_exception_3665: ;

  return UNIT;
}

sail_u256 zjumpdest_bit(uint64_t zindex)
{
  sail_u256 z8zE130;
  sail_u256 zchunk;
  zchunk = zEMPTY_JUMPDEST_CHUNK;
  zchunk = u256_update_u64(zchunk, zindex, UINT64_C(0b1));
  unit z3zE3588;
  z3zE3588 = UNIT;
  z8zE130 = zchunk;
end_function_3662: ;
  return z8zE130;
end_block_exception_3663: ;

  return u256_zero();
}

unit zanalyzze_code_from(struct zByteSliceFields zcode, enum zFork zfork, uint64_t ztable, uint64_t zpc, struct zCodeAnalysis zanalysis)
{
  unit z8zE131;
  uint64_t zposition;
  zposition = zpc;
  uint64_t zcode_len;
  zcode_len = zcode.zlen;
  bool z2zE3812;
  z2zE3812 = (zposition < zcode_len);
  if (z2zE3812) {
    sail_u256 zchunk;
    zchunk = zanalysis.zchunk;
    uint64_t zchunk_index;
    zchunk_index = zanalysis.zchunk_index;
    uint64_t zchunk_offset;
    zchunk_offset = zanalysis.zchunk_offset;
    uint64_t zopcode;
    zopcode = zslice_byte(zcode, zposition);
    bool z2zE3813;
    z2zE3813 = (zopcode == UINT64_C(0x5B));
    unit z3zE3578;
    if (z2zE3813) {
      sail_u256 z2zE3814;
      z2zE3814 = zjumpdest_bit(zchunk_offset);
      zchunk = u256_or(zchunk, z2zE3814);
      z3zE3578 = UNIT;
    } else {  z3zE3578 = UNIT;  }
    uint64_t zopcode_value;
    zopcode_value = ((uint64_t) zopcode);
    uint64_t zstep;
    {
      bool z2zE3824;
      {
        bool z2zE3823;
        z2zE3823 = (!(zopcode_value < UINT64_C(96)));
        bool z3zE3579;
        if (z2zE3823) {  z3zE3579 = (!(UINT64_C(127) < zopcode_value));  } else {  z3zE3579 = false;  }
        z2zE3824 = z3zE3579;
      }
      if (z2zE3824) {
        {    zstep = (zopcode_value - UINT64_C(94));
        }
      } else {
        bool z2zE3827;
        {
          bool z2zE3826;
          z2zE3826 = zfork_gteq(zfork, zAmsterdam);
          bool z3zE3581;
          if (z2zE3826) {
            bool z2zE3825;
            z2zE3825 = (zopcode_value == UINT64_C(230));
            bool z3zE3580;
            if (z2zE3825) {  z3zE3580 = true;  } else {  z3zE3580 = (zopcode_value == UINT64_C(231));  }
            z3zE3581 = z3zE3580;
          } else {  z3zE3581 = false;  }
          z2zE3827 = z3zE3581;
        }
        if (z2zE3827) {
          bool z2zE3830;
          {
            uint64_t z2zE3829;
            {
              uint64_t z2zE3828;
              {    z2zE3828 = (zposition + UINT64_C(1));
              }
              z2zE3829 = zslice_byte(zcode, z2zE3828);
            }
            z2zE3830 = zdeep_stack_immediate_valid(z2zE3829);
          }
          if (z2zE3830) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
        } else {
          bool z2zE3832;
          {
            bool z2zE3831;
            z2zE3831 = zfork_gteq(zfork, zAmsterdam);
            bool z3zE3582;
            if (z2zE3831) {  z3zE3582 = (zopcode_value == UINT64_C(232));  } else {  z3zE3582 = false;  }
            z2zE3832 = z3zE3582;
          }
          if (z2zE3832) {
            bool z2zE3835;
            {
              uint64_t z2zE3834;
              {
                uint64_t z2zE3833;
                {    z2zE3833 = (zposition + UINT64_C(1));
                }
                z2zE3834 = zslice_byte(zcode, z2zE3833);
              }
              z2zE3835 = zexchange_immediate_valid(z2zE3834);
            }
            if (z2zE3835) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
          } else {  zstep = UINT64_C(1);  }
        }
      }
    }
    bool z2zE3816;
    {
      uint64_t z2zE3815;
      {    z2zE3815 = (zcode_len - zposition);
      }
      z2zE3816 = (zstep < z2zE3815);
    }
    if (z2zE3816) {
      uint64_t zadded;
      {    zadded = (zposition + zstep);
      }
      uint64_t zprogressed;
      {    zprogressed = (zchunk_offset + zstep);
      }
      bool z2zE3817;
      z2zE3817 = (zprogressed < UINT64_C(256));
      if (z2zE3817) {
        struct zCodeAnalysis z3zE3587;
        z3zE3587.zchunk = zchunk;
        z3zE3587.zchunk_index = zchunk_index;
        z3zE3587.zchunk_offset = zprogressed;
        z8zE131 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z3zE3587);
      } else {
        uint64_t z2zE3818;
        z2zE3818 = zcode.zlen;
        struct zCodeAnalysis z3zE3584;
        z3zE3584.zchunk = zchunk;
        z3zE3584.zchunk_index = zchunk_index;
        z3zE3584.zchunk_offset = zchunk_offset;
        unit z3zE3585;
        z3zE3585 = zstore_jumpdest_chunk(ztable, z2zE3818, z3zE3584);
        struct zCodeAnalysis z2zE3821;
        {
          uint64_t z2zE3819;
          z2zE3819 = (zadded / UINT64_C(256));
          uint64_t z2zE3820;
          {    z2zE3820 = (zprogressed - UINT64_C(256));
          }
          struct zCodeAnalysis z3zE3586;
          z3zE3586.zchunk = zEMPTY_JUMPDEST_CHUNK;
          z3zE3586.zchunk_index = z2zE3819;
          z3zE3586.zchunk_offset = z2zE3820;
          z2zE3821 = z3zE3586;
        }
        z8zE131 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z2zE3821);
      }
    } else {
      uint64_t z2zE3822;
      z2zE3822 = zcode.zlen;
      struct zCodeAnalysis z3zE3583;
      z3zE3583.zchunk = zchunk;
      z3zE3583.zchunk_index = zchunk_index;
      z3zE3583.zchunk_offset = zchunk_offset;
      z8zE131 = zstore_jumpdest_chunk(ztable, z2zE3822, z3zE3583);
    }
  } else {
    uint64_t z2zE3836;
    z2zE3836 = zcode.zlen;
    z8zE131 = zstore_jumpdest_chunk(ztable, z2zE3836, zanalysis);
  }
end_function_3660: ;
  return z8zE131;
end_block_exception_3661: ;

  return UNIT;
}

uint64_t zanalyzze_code(struct zByteSliceFields zcode, enum zFork zfork)
{
  uint64_t z8zE132;
  bool z2zE3810;
  {
    uint64_t z2zE3809;
    z2zE3809 = zcode.zlen;
    z2zE3810 = (z2zE3809 == UINT64_C(0));
  }
  if (z2zE3810) {  z8zE132 = zEMPTY_JUMPDEST_REF;  } else {
    uint64_t zlength;
    zlength = zcode.zlen;
    uint64_t ztable;
    ztable = jumpdest_table_alloc(zlength);
    bool z2zE3811;
    z2zE3811 = (ztable != zEMPTY_JUMPDEST_REF);
    unit z3zE3577;
    z3zE3577 = sail_assert(z2zE3811, "JUMPDEST table allocation");
    struct zCodeAnalysis z3zE3575;
    z3zE3575.zchunk = zEMPTY_JUMPDEST_CHUNK;
    z3zE3575.zchunk_index = UINT64_C(0);
    z3zE3575.zchunk_offset = UINT64_C(0);
    unit z3zE3576;
    z3zE3576 = zanalyzze_code_from(zcode, zfork, ztable, UINT64_C(0), z3zE3575);
    z8zE132 = ztable;
  }
end_function_3658: ;
  return z8zE132;
end_block_exception_3659: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_32 zcode_db_insert(struct zByteSliceFields zcode, enum zFork zfork)
{
  sail_fixed_bytes_32 z8zE133;
  uint64_t z2zE3808;
  z2zE3808 = zanalyzze_code(zcode, zfork);
  z8zE133 = code_db_store_indexed(zcode, z2zE3808);
end_function_3656: ;
  return z8zE133;
end_block_exception_3657: ;

  return fixed_bytes_32_zero();
}

struct zCode zcode_db_resolve(sail_fixed_bytes_32 zcode_hash)
{
  struct zCode z8zE134;
  bool z2zE3805;
  z2zE3805 = eq_fixed_bytes_32(zcode_hash, zKECCAK_EMPTY);
  if (z2zE3805) {  z8zE134 = zEMPTY_CODE;  } else {
    struct zoptionzIRCodezK z2zE3806;
    CREATE(zoptionzIRCodezK)(&z2zE3806);
    code_db_lookup(&z2zE3806, zcode_hash);
    struct zCode z3zE3572;
    {
      if (z2zE3806.kind != Kind_zSomezIRCodezK) goto case_3653;
      struct zCode zcode;
      zcode = z2zE3806.variants.zSomezIRCodezK;
      z3zE3572 = zcode;
      goto finish_match_3651;
    }
  case_3653: ;
    {
      /* complete */
      struct zexception z2zE3807;
      CREATE(zexception)(&z2zE3807);
      zInvalidBlock(&z2zE3807, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3807);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/host/code.sail:162.22-162.58");
      KILL(zoptionzIRCodezK)(&z2zE3806);
      KILL(zexception)(&z2zE3807);
      goto end_block_exception_3655;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3807);
      goto finish_match_3651;
    }
  case_3652: ;
  finish_match_3651: ;
    z8zE134 = z3zE3572;
    KILL(zoptionzIRCodezK)(&z2zE3806);
  }
end_function_3654: ;
  return z8zE134;
end_block_exception_3655: ;
  struct zByteSliceFields z8zE833 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE832 = { .zbytes = z8zE833, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE832;
}

