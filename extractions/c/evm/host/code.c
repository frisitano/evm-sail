/* Generated from sail/host/code.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
unit zstore_jumpdest_chunk(uint64_t ztable, uint64_t zcode_len, struct zCodeAnalysis zanalysis)
{
  unit z8zE129;
  bool z2zE3877;
  {
    sail_u256 z2zE3876;
    z2zE3876 = zanalysis.zchunk;
    z2zE3877 = (!eq_u256(z2zE3876, zEMPTY_JUMPDEST_CHUNK));
  }
  if (z2zE3877) {
    bool zstored;
    {
      uint64_t z2zE3878;
      z2zE3878 = zanalysis.zchunk_index;
      sail_u256 z2zE3879;
      z2zE3879 = zanalysis.zchunk;
      zstored = jumpdest_table_store_chunk(ztable, zcode_len, z2zE3878, z2zE3879);
    }
    z8zE129 = sail_assert(zstored, "JUMPDEST chunk store");
  } else {  z8zE129 = UNIT;  }
end_function_3683: ;
  return z8zE129;
end_block_exception_3684: ;

  return UNIT;
}

sail_u256 zjumpdest_bit(uint64_t zindex)
{
  sail_u256 z8zE130;
  sail_u256 zchunk;
  zchunk = zEMPTY_JUMPDEST_CHUNK;
  zchunk = u256_update_u64(zchunk, zindex, UINT64_C(0b1));
  unit z3zE3659;
  z3zE3659 = UNIT;
  z8zE130 = zchunk;
end_function_3681: ;
  return z8zE130;
end_block_exception_3682: ;

  return u256_zero();
}

unit zanalyzze_code_from(struct zByteSliceFields zcode, enum zFork zfork, uint64_t ztable, uint64_t zpc, struct zCodeAnalysis zanalysis)
{
  unit z8zE131;
  uint64_t zposition;
  zposition = zpc;
  uint64_t zcode_len;
  zcode_len = zcode.zlen;
  bool z2zE3851;
  z2zE3851 = (zposition < zcode_len);
  if (z2zE3851) {
    sail_u256 zchunk;
    zchunk = zanalysis.zchunk;
    uint64_t zchunk_index;
    zchunk_index = zanalysis.zchunk_index;
    uint64_t zchunk_offset;
    zchunk_offset = zanalysis.zchunk_offset;
    uint64_t zopcode;
    zopcode = zslice_byte(zcode, zposition);
    bool z2zE3852;
    z2zE3852 = (zopcode == UINT64_C(0x5B));
    unit z3zE3649;
    if (z2zE3852) {
      sail_u256 z2zE3853;
      z2zE3853 = zjumpdest_bit(zchunk_offset);
      zchunk = u256_or(zchunk, z2zE3853);
      z3zE3649 = UNIT;
    } else {  z3zE3649 = UNIT;  }
    uint64_t zopcode_value;
    zopcode_value = ((uint64_t) zopcode);
    uint64_t zstep;
    {
      bool z2zE3863;
      {
        bool z2zE3862;
        z2zE3862 = (!(zopcode_value < UINT64_C(96)));
        bool z3zE3650;
        if (z2zE3862) {  z3zE3650 = (!(UINT64_C(127) < zopcode_value));  } else {  z3zE3650 = false;  }
        z2zE3863 = z3zE3650;
      }
      if (z2zE3863) {
        {    zstep = (zopcode_value - UINT64_C(94));
        }
      } else {
        bool z2zE3866;
        {
          bool z2zE3865;
          z2zE3865 = zfork_gteq(zfork, zAmsterdam);
          bool z3zE3652;
          if (z2zE3865) {
            bool z2zE3864;
            z2zE3864 = (zopcode_value == UINT64_C(230));
            bool z3zE3651;
            if (z2zE3864) {  z3zE3651 = true;  } else {  z3zE3651 = (zopcode_value == UINT64_C(231));  }
            z3zE3652 = z3zE3651;
          } else {  z3zE3652 = false;  }
          z2zE3866 = z3zE3652;
        }
        if (z2zE3866) {
          bool z2zE3869;
          {
            uint64_t z2zE3868;
            {
              uint64_t z2zE3867;
              {    z2zE3867 = (zposition + UINT64_C(1));
              }
              z2zE3868 = zslice_byte(zcode, z2zE3867);
            }
            z2zE3869 = zdeep_stack_immediate_valid(z2zE3868);
          }
          if (z2zE3869) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
        } else {
          bool z2zE3871;
          {
            bool z2zE3870;
            z2zE3870 = zfork_gteq(zfork, zAmsterdam);
            bool z3zE3653;
            if (z2zE3870) {  z3zE3653 = (zopcode_value == UINT64_C(232));  } else {  z3zE3653 = false;  }
            z2zE3871 = z3zE3653;
          }
          if (z2zE3871) {
            bool z2zE3874;
            {
              uint64_t z2zE3873;
              {
                uint64_t z2zE3872;
                {    z2zE3872 = (zposition + UINT64_C(1));
                }
                z2zE3873 = zslice_byte(zcode, z2zE3872);
              }
              z2zE3874 = zexchange_immediate_valid(z2zE3873);
            }
            if (z2zE3874) {  zstep = UINT64_C(2);  } else {  zstep = UINT64_C(1);  }
          } else {  zstep = UINT64_C(1);  }
        }
      }
    }
    bool z2zE3855;
    {
      uint64_t z2zE3854;
      {    z2zE3854 = (zcode_len - zposition);
      }
      z2zE3855 = (zstep < z2zE3854);
    }
    if (z2zE3855) {
      uint64_t zadded;
      {    zadded = (zposition + zstep);
      }
      uint64_t zprogressed;
      {    zprogressed = (zchunk_offset + zstep);
      }
      bool z2zE3856;
      z2zE3856 = (zprogressed < UINT64_C(256));
      if (z2zE3856) {
        struct zCodeAnalysis z3zE3658;
        z3zE3658.zchunk = zchunk;
        z3zE3658.zchunk_index = zchunk_index;
        z3zE3658.zchunk_offset = zprogressed;
        z8zE131 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z3zE3658);
      } else {
        uint64_t z2zE3857;
        z2zE3857 = zcode.zlen;
        struct zCodeAnalysis z3zE3655;
        z3zE3655.zchunk = zchunk;
        z3zE3655.zchunk_index = zchunk_index;
        z3zE3655.zchunk_offset = zchunk_offset;
        unit z3zE3656;
        z3zE3656 = zstore_jumpdest_chunk(ztable, z2zE3857, z3zE3655);
        struct zCodeAnalysis z2zE3860;
        {
          uint64_t z2zE3858;
          z2zE3858 = (zadded / UINT64_C(256));
          uint64_t z2zE3859;
          {    z2zE3859 = (zprogressed - UINT64_C(256));
          }
          struct zCodeAnalysis z3zE3657;
          z3zE3657.zchunk = zEMPTY_JUMPDEST_CHUNK;
          z3zE3657.zchunk_index = z2zE3858;
          z3zE3657.zchunk_offset = z2zE3859;
          z2zE3860 = z3zE3657;
        }
        z8zE131 = zanalyzze_code_from(zcode, zfork, ztable, zadded, z2zE3860);
      }
    } else {
      uint64_t z2zE3861;
      z2zE3861 = zcode.zlen;
      struct zCodeAnalysis z3zE3654;
      z3zE3654.zchunk = zchunk;
      z3zE3654.zchunk_index = zchunk_index;
      z3zE3654.zchunk_offset = zchunk_offset;
      z8zE131 = zstore_jumpdest_chunk(ztable, z2zE3861, z3zE3654);
    }
  } else {
    uint64_t z2zE3875;
    z2zE3875 = zcode.zlen;
    z8zE131 = zstore_jumpdest_chunk(ztable, z2zE3875, zanalysis);
  }
end_function_3679: ;
  return z8zE131;
end_block_exception_3680: ;

  return UNIT;
}

uint64_t zanalyzze_code(struct zByteSliceFields zcode, enum zFork zfork)
{
  uint64_t z8zE132;
  bool z2zE3849;
  {
    uint64_t z2zE3848;
    z2zE3848 = zcode.zlen;
    z2zE3849 = (z2zE3848 == UINT64_C(0));
  }
  if (z2zE3849) {  z8zE132 = zEMPTY_JUMPDEST_REF;  } else {
    uint64_t zlength;
    zlength = zcode.zlen;
    uint64_t ztable;
    ztable = jumpdest_table_alloc(zlength);
    bool z2zE3850;
    z2zE3850 = (ztable != zEMPTY_JUMPDEST_REF);
    unit z3zE3648;
    z3zE3648 = sail_assert(z2zE3850, "JUMPDEST table allocation");
    struct zCodeAnalysis z3zE3646;
    z3zE3646.zchunk = zEMPTY_JUMPDEST_CHUNK;
    z3zE3646.zchunk_index = UINT64_C(0);
    z3zE3646.zchunk_offset = UINT64_C(0);
    unit z3zE3647;
    z3zE3647 = zanalyzze_code_from(zcode, zfork, ztable, UINT64_C(0), z3zE3646);
    z8zE132 = ztable;
  }
end_function_3677: ;
  return z8zE132;
end_block_exception_3678: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_32 zcode_db_insert(struct zByteSliceFields zcode, enum zFork zfork)
{
  sail_fixed_bytes_32 z8zE133;
  uint64_t z2zE3847;
  z2zE3847 = zanalyzze_code(zcode, zfork);
  z8zE133 = code_db_store_indexed(zcode, z2zE3847);
end_function_3675: ;
  return z8zE133;
end_block_exception_3676: ;

  return fixed_bytes_32_zero();
}

struct zCode zcode_db_resolve(sail_fixed_bytes_32 zcode_hash)
{
  struct zCode z8zE134;
  bool z2zE3844;
  z2zE3844 = eq_fixed_bytes_32(zcode_hash, zKECCAK_EMPTY);
  if (z2zE3844) {  z8zE134 = zEMPTY_CODE;  } else {
    struct zoptionzIRCodezK z2zE3845;
    CREATE(zoptionzIRCodezK)(&z2zE3845);
    code_db_lookup(&z2zE3845, zcode_hash);
    struct zCode z3zE3643;
    {
      if (z2zE3845.kind != Kind_zSomezIRCodezK) goto case_3672;
      struct zCode zcode;
      zcode = z2zE3845.variants.zSomezIRCodezK;
      z3zE3643 = zcode;
      goto finish_match_3670;
    }
  case_3672: ;
    {
      /* complete */
      struct zexception z2zE3846;
      CREATE(zexception)(&z2zE3846);
      zInvalidBlock(&z2zE3846, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE3846);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/host/code.sail:162.22-162.58");
      KILL(zoptionzIRCodezK)(&z2zE3845);
      KILL(zexception)(&z2zE3846);
      goto end_block_exception_3674;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3846);
      goto finish_match_3670;
    }
  case_3671: ;
  finish_match_3670: ;
    z8zE134 = z3zE3643;
    KILL(zoptionzIRCodezK)(&z2zE3845);
  }
end_function_3673: ;
  return z8zE134;
end_block_exception_3674: ;
  struct zByteSliceFields z8zE836 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zCode z8zE835 = { .zbytes = z8zE836, .zjumpdests = UINT64_C(0xdeadc0de) };
  return z8zE835;
}

