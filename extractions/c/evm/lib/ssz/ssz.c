/* Generated from sail/lib/ssz/ssz.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_83(void) {

  uint64_t z3zE108;
  z3zE108 = UINT64_C(4);
  zSSZ_OFF_BYTES = z3zE108;
let_end_221: ;
}
void kill_letbind_83(void) {
}

void create_letbind_84(void) {

  uint64_t z3zE109;
  z3zE109 = UINT64_C(8);
  zSSZ_UINT_BYTES = z3zE109;
let_end_222: ;
}
void kill_letbind_84(void) {
}

uint64_t zsszz_field_offset(uint64_t zbase, uint64_t zdelta)
{
  uint64_t z8zE135;
  {    z8zE135 = (zbase + zdelta);
  }
end_function_3668: ;
  return z8zE135;
end_block_exception_3669: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_u32_at(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE136;
  uint64_t zb0;
  {
    uint64_t z2zE3843;
    z2zE3843 = zslice_byte(zinput, zoffset);
    zb0 = z2zE3843;
  }
  uint64_t zb1;
  {
    uint64_t z2zE3842;
    {
      uint64_t z2zE3841;
      z2zE3841 = zsszz_field_offset(zoffset, UINT64_C(1));
      z2zE3842 = zslice_byte(zinput, z2zE3841);
    }
    zb1 = z2zE3842;
  }
  uint64_t zb2;
  {
    uint64_t z2zE3840;
    {
      uint64_t z2zE3839;
      z2zE3839 = zsszz_field_offset(zoffset, UINT64_C(2));
      z2zE3840 = zslice_byte(zinput, z2zE3839);
    }
    zb2 = z2zE3840;
  }
  uint64_t zb3;
  {
    uint64_t z2zE3838;
    {
      uint64_t z2zE3837;
      z2zE3837 = zsszz_field_offset(zoffset, UINT64_C(3));
      z2zE3838 = zslice_byte(zinput, z2zE3837);
    }
    zb3 = z2zE3838;
  }
  uint64_t z2zE3836;
  {
    uint64_t z2zE3835;
    {
      uint64_t z2zE3833;
      z2zE3833 = ((UINT64_C(8) >= UINT64_C(64)) ? UINT64_C(0) : ((zb1 << UINT64_C(8)) & UINT64_C(0xFFFFFFFF)));
      uint64_t z2zE3834;
      {
        uint64_t z2zE3831;
        z2zE3831 = ((UINT64_C(16) >= UINT64_C(64)) ? UINT64_C(0) : ((zb2 << UINT64_C(16)) & UINT64_C(0xFFFFFFFF)));
        uint64_t z2zE3832;
        z2zE3832 = ((UINT64_C(24) >= UINT64_C(64)) ? UINT64_C(0) : ((zb3 << UINT64_C(24)) & UINT64_C(0xFFFFFFFF)));
        z2zE3834 = (z2zE3831 | z2zE3832);
      }
      z2zE3835 = (z2zE3833 | z2zE3834);
    }
    z2zE3836 = (zb0 | z2zE3835);
  }
  z8zE136 = ((uint64_t) z2zE3836);
end_function_3666: ;
  return z8zE136;
end_block_exception_3667: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_u32(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE137;
  z8zE137 = zsszz_u32_at(zinput, zoffset);
end_function_3664: ;
  return z8zE137;
end_block_exception_3665: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zsszz_offset_to_source_pointer(uint64_t zvalue)
{
  uint64_t z8zE138;
  z8zE138 = zvalue;
end_function_3662: ;
  return z8zE138;
end_block_exception_3663: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zdecode_sszz_uint(struct zByteSliceFields zinput, uint64_t zoffset)
{
  uint64_t z8zE139;
  uint64_t z2zE3829;
  {
    uint64_t z2zE3824;
    {
      uint64_t z2zE3819;
      {
        uint64_t z2zE3814;
        {
          uint64_t z2zE3809;
          {
            uint64_t z2zE3804;
            {
              uint64_t z2zE3799;
              {
                uint64_t z2zE3795;
                z2zE3795 = zslice_byte(zinput, zoffset);
                z2zE3799 = ((uint64_t) z2zE3795);
              }
              uint64_t z2zE3800;
              {
                uint64_t z2zE3798;
                {
                  uint64_t z2zE3797;
                  {
                    uint64_t z2zE3796;
                    z2zE3796 = zsszz_field_offset(zoffset, UINT64_C(1));
                    z2zE3797 = zslice_byte(zinput, z2zE3796);
                  }
                  z2zE3798 = ((uint64_t) z2zE3797);
                }
                {    z2zE3800 = (z2zE3798 * UINT64_C(256));
                }
              }
              {    z2zE3804 = (z2zE3799 + z2zE3800);
              }
            }
            uint64_t z2zE3805;
            {
              uint64_t z2zE3803;
              {
                uint64_t z2zE3802;
                {
                  uint64_t z2zE3801;
                  z2zE3801 = zsszz_field_offset(zoffset, UINT64_C(2));
                  z2zE3802 = zslice_byte(zinput, z2zE3801);
                }
                z2zE3803 = ((uint64_t) z2zE3802);
              }
              {    z2zE3805 = (z2zE3803 * UINT64_C(65536));
              }
            }
            {    z2zE3809 = (z2zE3804 + z2zE3805);
            }
          }
          uint64_t z2zE3810;
          {
            uint64_t z2zE3808;
            {
              uint64_t z2zE3807;
              {
                uint64_t z2zE3806;
                z2zE3806 = zsszz_field_offset(zoffset, UINT64_C(3));
                z2zE3807 = zslice_byte(zinput, z2zE3806);
              }
              z2zE3808 = ((uint64_t) z2zE3807);
            }
            {    z2zE3810 = (z2zE3808 * UINT64_C(16777216));
            }
          }
          {    z2zE3814 = (z2zE3809 + z2zE3810);
          }
        }
        uint64_t z2zE3815;
        {
          uint64_t z2zE3813;
          {
            uint64_t z2zE3812;
            {
              uint64_t z2zE3811;
              z2zE3811 = zsszz_field_offset(zoffset, zSSZ_OFF_BYTES);
              z2zE3812 = zslice_byte(zinput, z2zE3811);
            }
            z2zE3813 = ((uint64_t) z2zE3812);
          }
          {    z2zE3815 = (z2zE3813 * UINT64_C(4294967296));
          }
        }
        {    z2zE3819 = (z2zE3814 + z2zE3815);
        }
      }
      uint64_t z2zE3820;
      {
        uint64_t z2zE3818;
        {
          uint64_t z2zE3817;
          {
            uint64_t z2zE3816;
            z2zE3816 = zsszz_field_offset(zoffset, UINT64_C(5));
            z2zE3817 = zslice_byte(zinput, z2zE3816);
          }
          z2zE3818 = ((uint64_t) z2zE3817);
        }
        {    z2zE3820 = (z2zE3818 * UINT64_C(1099511627776));
        }
      }
      {    z2zE3824 = (z2zE3819 + z2zE3820);
      }
    }
    uint64_t z2zE3825;
    {
      uint64_t z2zE3823;
      {
        uint64_t z2zE3822;
        {
          uint64_t z2zE3821;
          z2zE3821 = zsszz_field_offset(zoffset, UINT64_C(6));
          z2zE3822 = zslice_byte(zinput, z2zE3821);
        }
        z2zE3823 = ((uint64_t) z2zE3822);
      }
      {    z2zE3825 = (z2zE3823 * UINT64_C(281474976710656));
      }
    }
    {    z2zE3829 = (z2zE3824 + z2zE3825);
    }
  }
  uint64_t z2zE3830;
  {
    uint64_t z2zE3828;
    {
      uint64_t z2zE3827;
      {
        uint64_t z2zE3826;
        z2zE3826 = zsszz_field_offset(zoffset, UINT64_C(7));
        z2zE3827 = zslice_byte(zinput, z2zE3826);
      }
      z2zE3828 = ((uint64_t) z2zE3827);
    }
    {    z2zE3830 = (z2zE3828 * UINT64_C(72057594037927936));
    }
  }
  {    z8zE139 = (z2zE3829 + z2zE3830);
  }
end_function_3660: ;
  return z8zE139;
end_block_exception_3661: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zsszz_addr(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_20 z8zE140;
  sail_u256 z2zE3794;
  z2zE3794 = zslice_load_n(zinput, zoffset, zADDRESS_BYTE_LENGTH);
  z8zE140 = zword_to_address(z2zE3794);
end_function_3658: ;
  return z8zE140;
end_block_exception_3659: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_32 zsszz_bytes32(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_32 z8zE141;
  sail_u256 z2zE3793;
  z2zE3793 = zslice_load(zinput, zoffset);
  z8zE141 = zword_to_hash(z2zE3793);
end_function_3656: ;
  return z8zE141;
end_block_exception_3657: ;

  return fixed_bytes_32_zero();
}

uint64_t zsszz_logs_bloom_index(uint64_t zindex)
{
  uint64_t z8zE142;
  {    z8zE142 = (UINT64_C(255) - zindex);
  }
end_function_3654: ;
  return z8zE142;
end_block_exception_3655: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_256 zsszz_logs_bloom(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_fixed_bytes_256 z8zE143;
  sail_fixed_bytes_256 zout;
  {
    zz5vecz8z5bv8z9 z3zE3637;
    CREATE(zz5vecz8z5bv8z9)(&z3zE3637);
    internal_vector_init_zz5vecz8z5bv8z9(&z3zE3637, INT64_C(256));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(0), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(1), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(2), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(3), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(4), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(5), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(6), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(7), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(8), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(9), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(10), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(11), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(12), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(13), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(14), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(15), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(16), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(17), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(18), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(19), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(20), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(21), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(22), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(23), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(24), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(25), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(26), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(27), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(28), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(29), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(30), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(31), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(32), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(33), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(34), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(35), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(36), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(37), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(38), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(39), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(40), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(41), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(42), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(43), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(44), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(45), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(46), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(47), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(48), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(49), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(50), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(51), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(52), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(53), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(54), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(55), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(56), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(57), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(58), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(59), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(60), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(61), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(62), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(63), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(64), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(65), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(66), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(67), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(68), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(69), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(70), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(71), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(72), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(73), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(74), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(75), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(76), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(77), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(78), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(79), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(80), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(81), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(82), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(83), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(84), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(85), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(86), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(87), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(88), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(89), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(90), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(91), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(92), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(93), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(94), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(95), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(96), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(97), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(98), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(99), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(100), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(101), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(102), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(103), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(104), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(105), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(106), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(107), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(108), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(109), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(110), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(111), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(112), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(113), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(114), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(115), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(116), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(117), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(118), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(119), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(120), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(121), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(122), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(123), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(124), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(125), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(126), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(127), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(128), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(129), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(130), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(131), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(132), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(133), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(134), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(135), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(136), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(137), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(138), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(139), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(140), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(141), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(142), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(143), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(144), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(145), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(146), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(147), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(148), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(149), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(150), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(151), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(152), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(153), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(154), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(155), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(156), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(157), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(158), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(159), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(160), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(161), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(162), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(163), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(164), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(165), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(166), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(167), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(168), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(169), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(170), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(171), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(172), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(173), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(174), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(175), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(176), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(177), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(178), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(179), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(180), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(181), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(182), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(183), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(184), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(185), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(186), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(187), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(188), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(189), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(190), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(191), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(192), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(193), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(194), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(195), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(196), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(197), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(198), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(199), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(200), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(201), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(202), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(203), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(204), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(205), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(206), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(207), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(208), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(209), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(210), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(211), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(212), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(213), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(214), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(215), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(216), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(217), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(218), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(219), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(220), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(221), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(222), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(223), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(224), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(225), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(226), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(227), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(228), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(229), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(230), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(231), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(232), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(233), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(234), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(235), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(236), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(237), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(238), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(239), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(240), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(241), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(242), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(243), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(244), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(245), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(246), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(247), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(248), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(249), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(250), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(251), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(252), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(253), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(254), UINT64_C(0x00));
    internal_vector_update_zz5vecz8z5bv8z9(&z3zE3637, z3zE3637, INT64_C(255), UINT64_C(0x00));
    for (size_t z8zE837 = 0; z8zE837 < 256; ++z8zE837) {
      zout.bytes[z8zE837] = (uint8_t)(z3zE3637.data[z8zE837] & UINT64_C(0xff));
    }
    KILL(zz5vecz8z5bv8z9)(&z3zE3637);
  }
  int64_t z3zE3638;
  {    z3zE3638 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3639;
  {    z3zE3639 = (int64_t)(UINT64_C(255));
  }
  int64_t z3zE3640;
  {    z3zE3640 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3638;
    unit z3zE3641;
  for_start_3650: ;
    {
      if ((z3zE3639 < zk)) goto for_end_3651;
      uint64_t z2zE3791;
      {
        uint64_t z3zE3853;
        z3zE3853 = (uint64_t)(zk);
        z2zE3791 = zsszz_logs_bloom_index(z3zE3853);
      }
      uint64_t z2zE3792;
      {
        uint64_t z2zE3790;
        {
          uint64_t z3zE3852;
          z3zE3852 = (uint64_t)(zk);
          z2zE3790 = zsszz_field_offset(zoffset, z3zE3852);
        }
        z2zE3792 = zslice_byte(zinput, z2zE3790);
      }
      zout = fast_unsigned_vector_update_fixed_bytes_256(zout, z2zE3791, z2zE3792);
      z3zE3641 = UNIT;
      zk = (zk + z3zE3640);
      goto for_start_3650;
    }
  for_end_3651: ;
  }
  unit z3zE3642;
  z3zE3642 = UNIT;
  z8zE143 = zout;
end_function_3652: ;
  return z8zE143;
end_block_exception_3653: ;

  return fixed_bytes_256_zero();
}

uint64_t zsszz_u256_index(uint64_t zindex)
{
  uint64_t z8zE144;
  {    z8zE144 = (UINT64_C(31) - zindex);
  }
end_function_3648: ;
  return z8zE144;
end_block_exception_3649: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zsszz_u256(struct zByteSliceFields zinput, uint64_t zoffset)
{
  sail_u256 z8zE145;
  sail_u256 zresult;
  zresult = zWORD_ZERO;
  int64_t z3zE3632;
  {    z3zE3632 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3633;
  {    z3zE3633 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3634;
  {    z3zE3634 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zk;
    zk = z3zE3632;
    unit z3zE3635;
  for_start_3644: ;
    {
      if ((z3zE3633 < zk)) goto for_end_3645;
      sail_u256 z2zE3788;
      z2zE3788 = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zresult, UINT64_C(256));
      uint64_t z2zE3789;
      {
        uint64_t z2zE3787;
        {
          uint64_t z2zE3786;
          {
            uint64_t z2zE3785;
            {
              uint64_t z3zE3854;
              z3zE3854 = (uint64_t)(zk);
              z2zE3785 = zsszz_u256_index(z3zE3854);
            }
            z2zE3786 = zsszz_field_offset(zoffset, z2zE3785);
          }
          z2zE3787 = zslice_byte(zinput, z2zE3786);
        }
        z2zE3789 = ((uint64_t) z2zE3787);
      }
      zresult = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3788, z2zE3789);
      z3zE3635 = UNIT;
      zk = (zk + z3zE3634);
      goto for_start_3644;
    }
  for_end_3645: ;
  }
  unit z3zE3636;
  z3zE3636 = UNIT;
  z8zE145 = zresult;
end_function_3646: ;
  return z8zE145;
end_block_exception_3647: ;

  return u256_zero();
}

