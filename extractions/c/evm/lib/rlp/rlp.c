/* Generated from sail/lib/rlp/rlp.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_85(void) {

  uint64_t z3zE110;
  z3zE110 = UINT64_C(55);
  zRLP_SHORT_LENGTH_LIMIT = z3zE110;
let_end_234: ;
}
void kill_letbind_85(void) {
}

void create_letbind_86(void) {

  uint64_t z3zE111;
  z3zE111 = zWORD_BYTE_LENGTH;
  zRLP_WORD_LENGTH_LIMIT = z3zE111;
let_end_235: ;
}
void kill_letbind_86(void) {
}

void create_letbind_87(void) {

  uint64_t z3zE112;
  z3zE112 = zEIGHT_BYTE_LENGTH;
  zRLP_UINT64_LENGTH_LIMIT = z3zE112;
let_end_236: ;
}
void kill_letbind_87(void) {
}

void create_letbind_88(void) {

  uint64_t z3zE113;
  z3zE113 = UINT64_C(33);
  zRLP_ENCODED_WORD_LENGTH = z3zE113;
let_end_237: ;
}
void kill_letbind_88(void) {
}

void create_letbind_89(void) {

  uint64_t z3zE114;
  z3zE114 = UINT64_C(21);
  zRLP_ENCODED_ADDRESS_LENGTH = z3zE114;
let_end_238: ;
}
void kill_letbind_89(void) {
}

uint64_t zrlp_scratch_small_length(uint64_t zvalue)
{
  uint64_t z8zE146;
  z8zE146 = zvalue;
end_function_3642: ;
  return z8zE146;
end_block_exception_3643: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_byte_count(uint64_t zindex)
{
  uint64_t z8zE147;
  {    z8zE147 = (zindex + UINT64_C(1));
  }
end_function_3638: ;
  return z8zE147;
end_block_exception_3639: ;

  return UINT64_C(0xdeadc0de);
}

void zminimal_word_bytes(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE148, sail_u256 zw)
{
  sail_u256 zremaining;
  zremaining = zw;
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  uint64_t zlen;
  zlen = UINT64_C(0);
  int64_t z3zE3624;
  {    z3zE3624 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3625;
  {    z3zE3625 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3626;
  {    z3zE3626 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3624;
    unit z3zE3627;
  for_start_3634: ;
    {
      if ((z3zE3625 < zbyte_index)) goto for_end_3635;
      bool z2zE3780;
      z2zE3780 = zword_nonzzero(zremaining);
      if (z2zE3780) {
        uint64_t z2zE3781;
        z2zE3781 = zword_low_byte(zremaining);
        zconsz3z5bv8(&zout, z2zE3781, zout);
        unit z3zE3629;
        z3zE3629 = UNIT;
        zremaining = zword_shift_right(zremaining, UINT64_C(8));
        unit z3zE3628;
        z3zE3628 = UNIT;
        {
          uint64_t z3zE3855;
          z3zE3855 = (uint64_t)(zbyte_index);
          zlen = zword_byte_count(z3zE3855);
        }
        z3zE3627 = UNIT;
      } else {  z3zE3627 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE3626);
      goto for_start_3634;
    }
  for_end_3635: ;
  }
  unit z3zE3630;
  z3zE3630 = UNIT;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3631;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3631);
  COPY(zz5listz8z5bv8z9)(&((&z3zE3631)->ztup0), zout);
  z3zE3631.ztup1 = zlen;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE148)), z3zE3631);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3631);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3636: ;
  goto end_function_4084;
end_block_exception_3637: ;
  goto end_function_4084;
end_function_4084: ;
}

uint64_t zrlp_nat_length_byte(uint64_t zvalue)
{
  uint64_t z8zE149;
  z8zE149 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
end_function_3632: ;
  return z8zE149;
end_block_exception_3633: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_minimal_word_len(sail_u256 zw)
{
  uint64_t z8zE150;
  z8zE150 = zword_byte_length(zw);
end_function_3620: ;
  return z8zE150;
end_block_exception_3621: ;

  return UINT64_C(0xdeadc0de);
}

sail_u128 zrlp_slice_sizze(struct zByteSliceFields zdata)
{
  sail_u128 z8zE151;
  uint64_t zlen;
  zlen = zdata.zlen;
  uint64_t zfirst;
  {
    bool z2zE3749;
    z2zE3749 = (zlen == UINT64_C(0));
    if (z2zE3749) {  zfirst = UINT64_C(0x00);  } else {  zfirst = zslice_byte(zdata, UINT64_C(0));  }
  }
  z8zE151 = zrlp_string_sizzezIreprzGU64zCB8zCR__sail_c_repr_u128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlen, zfirst);
end_function_3609: ;
  return z8zE151;
end_block_exception_3610: ;

  return u128_zero();
}

uint64_t zrlp_uint_word_sizze(sail_u256 zw)
{
  uint64_t z8zE152;
  uint64_t zlen;
  zlen = zrlp_minimal_word_len(zw);
  uint64_t zfirst;
  {
    bool z2zE3748;
    z2zE3748 = (zlen == UINT64_C(1));
    if (z2zE3748) {  zfirst = zword_low_byte(zw);  } else {  zfirst = UINT64_C(0x00);  }
  }
  bool z2zE3747;
  {
    bool z2zE3746;
    z2zE3746 = (zlen == UINT64_C(1));
    bool z3zE3605;
    if (z2zE3746) {
      uint64_t z2zE3745;
      z2zE3745 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3605 = (z2zE3745 == UINT64_C(0b0));
    } else {  z3zE3605 = false;  }
    z2zE3747 = z3zE3605;
  }
  if (z2zE3747) {  z8zE152 = UINT64_C(1);  } else {
    {    z8zE152 = (UINT64_C(1) + zlen);
    }
  }
end_function_3607: ;
  return z8zE152;
end_block_exception_3608: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_nat_sizze(uint64_t zn)
{
  uint64_t z8zE153;
  sail_u256 z2zE3744;
  z2zE3744 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE153 = zrlp_uint_word_sizze(z2zE3744);
end_function_3605: ;
  return z8zE153;
end_block_exception_3606: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_list_sizze(uint64_t zcontent_len)
{
  uint64_t z8zE154;
  uint64_t z2zE3742;
  {
    uint64_t z2zE3741;
    z2zE3741 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    z2zE3742 = zrlp_scratch_small_length(z2zE3741);
  }
  {
    z8zE154 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE3742);
    if (have_exception) {  goto end_block_exception_3602;  }
  }
end_function_3601: ;
  return z8zE154;
end_block_exception_3602: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_slice_sizze(struct zByteSliceFields zdata)
{
  uint64_t z8zE155;
  uint64_t zlength;
  zlength = zdata.zlen;
  uint64_t zfirst;
  {
    bool z2zE3740;
    z2zE3740 = (zlength == UINT64_C(0));
    if (z2zE3740) {  zfirst = UINT64_C(0x00);  } else {  zfirst = zslice_byte(zdata, UINT64_C(0));  }
  }
  bool z2zE3737;
  {
    bool z2zE3736;
    z2zE3736 = (zlength == UINT64_C(1));
    bool z3zE3604;
    if (z2zE3736) {
      uint64_t z2zE3735;
      z2zE3735 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3604 = (z2zE3735 == UINT64_C(0b0));
    } else {  z3zE3604 = false;  }
    z2zE3737 = z3zE3604;
  }
  if (z2zE3737) {  z8zE155 = UINT64_C(1);  } else {
    uint64_t z2zE3739;
    {
      uint64_t z2zE3738;
      z2zE3738 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE3739 = zrlp_scratch_small_length(z2zE3738);
    }
    {
      z8zE155 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE3739);
      if (have_exception) {  goto end_block_exception_3600;  }
    }
  }
end_function_3599: ;
  return z8zE155;
end_block_exception_3600: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_bytes_sizze(zz5listz8z5bv8z9 zdata, uint64_t zlength)
{
  uint64_t z8zE156;
  uint64_t zfirst;
  {
    uint64_t z3zE3600;
    {
      if ((zdata == NULL)) goto case_3596;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE3600 = zb;
      goto finish_match_3594;
    }
  case_3596: ;
    {
      /* complete */
      z3zE3600 = UINT64_C(0x00);
      goto finish_match_3594;
    }
  case_3595: ;
  finish_match_3594: ;
    zfirst = z3zE3600;
  }
  bool z2zE3732;
  {
    bool z2zE3731;
    z2zE3731 = (zlength == UINT64_C(1));
    bool z3zE3603;
    if (z2zE3731) {
      uint64_t z2zE3730;
      z2zE3730 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3603 = (z2zE3730 == UINT64_C(0b0));
    } else {  z3zE3603 = false;  }
    z2zE3732 = z3zE3603;
  }
  if (z2zE3732) {  z8zE156 = UINT64_C(1);  } else {
    uint64_t z2zE3734;
    {
      uint64_t z2zE3733;
      z2zE3733 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE3734 = zrlp_scratch_small_length(z2zE3733);
    }
    {
      z8zE156 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE3734);
      if (have_exception) {  goto end_block_exception_3598;  }
    }
  }
end_function_3597: ;
  return z8zE156;
end_block_exception_3598: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE157;
  uint64_t zfirst;
  {
    uint64_t z3zE3592;
    {
      if ((zdata == NULL)) goto case_3583;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE3592 = zb;
      goto finish_match_3581;
    }
  case_3583: ;
    {
      /* complete */
      z3zE3592 = UINT64_C(0x00);
      goto finish_match_3581;
    }
  case_3582: ;
  finish_match_3581: ;
    zfirst = z3zE3592;
  }
  unit z3zE3595;
  z3zE3595 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlen, zfirst);
  z8zE157 = zscratch_push_bytes(zdata, zlen);
end_function_3584: ;
  return z8zE157;
end_block_exception_3585: ;

  return UNIT;
}

unit zrlp_write_slice(struct zByteSliceFields zdata)
{
  unit z8zE158;
  uint64_t z2zE3726;
  z2zE3726 = zdata.zlen;
  uint64_t z2zE3727;
  {
    bool z2zE3725;
    {
      uint64_t z2zE3724;
      z2zE3724 = zdata.zlen;
      z2zE3725 = (z2zE3724 == UINT64_C(0));
    }
    if (z2zE3725) {  z2zE3727 = UINT64_C(0x00);  } else {  z2zE3727 = zslice_byte(zdata, UINT64_C(0));  }
  }
  unit z3zE3591;
  z3zE3591 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3726, z2zE3727);
  z8zE158 = zscratch_push_slice(zdata);
end_function_3579: ;
  return z8zE158;
end_block_exception_3580: ;

  return UNIT;
}

unit zrlp_write_uint_word(sail_u256 zw)
{
  unit z8zE159;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3723;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3723);
  zminimal_word_bytes(&z2zE3723, zw);
  unit z3zE3589;
  {
    zz5listz8z5bv8z9 zbytes;
    CREATE(zz5listz8z5bv8z9)(&zbytes);
    COPY(zz5listz8z5bv8z9)(&zbytes, z2zE3723.ztup0);
    uint64_t zlen;
    zlen = z2zE3723.ztup1;
    z3zE3589 = zrlp_write_bytes(zbytes, zlen);
    KILL(zz5listz8z5bv8z9)(&zbytes);
    goto finish_match_3575;
  }
case_3576: ;
  sail_match_failure("rlp_write_uint_word");
finish_match_3575: ;
  z8zE159 = z3zE3589;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3723);
end_function_3577: ;
  return z8zE159;
end_block_exception_3578: ;

  return UNIT;
}

unit zrlp_write_uint_nat(uint64_t zn)
{
  unit z8zE160;
  sail_u256 z2zE3722;
  z2zE3722 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE160 = zrlp_write_uint_word(z2zE3722);
end_function_3573: ;
  return z8zE160;
end_block_exception_3574: ;

  return UNIT;
}

unit zrlp_write_word(sail_u256 zw)
{
  unit z8zE161;
  zz5listz8z5bv8z9 z2zE3721;
  CREATE(zz5listz8z5bv8z9)(&z2zE3721);
  zword_to_bytes32(&z2zE3721, zw);
  z8zE161 = zrlp_write_bytes(z2zE3721, zWORD_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE3721);
end_function_3571: ;
  return z8zE161;
end_block_exception_3572: ;

  return UNIT;
}

unit zrlp_write_addr(sail_fixed_bytes_20 za)
{
  unit z8zE162;
  zz5listz8z5bv8z9 z2zE3720;
  CREATE(zz5listz8z5bv8z9)(&z2zE3720);
  zaddress_to_bytes(&z2zE3720, za);
  z8zE162 = zrlp_write_bytes(z2zE3720, zADDRESS_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE3720);
end_function_3569: ;
  return z8zE162;
end_block_exception_3570: ;

  return UNIT;
}

struct zByteSliceFields zrlp_finish(uint64_t zstart)
{
  struct zByteSliceFields z8zE163;
  z8zE163 = zscratch_finish(zstart);
end_function_3567: ;
  return z8zE163;
end_block_exception_3568: ;
  struct zByteSliceFields z8zE838 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE838;
}

uint64_t zrlp_uint64_append(uint64_t z_width, uint64_t zprefix, uint64_t znext)
{
  uint64_t z8zE164;
  uint64_t z2zE3718;
  {    z2zE3718 = (zprefix * UINT64_C(256));
  }
  uint64_t z2zE3719;
  z2zE3719 = ((uint64_t) znext);
  {    z8zE164 = (z2zE3718 + z2zE3719);
  }
end_function_3565: ;
  return z8zE164;
end_block_exception_3566: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint64_width(struct zByteSliceFields zcontent, uint64_t zwidth)
{
  uint64_t z8zE165;
  bool z2zE3713;
  z2zE3713 = (zwidth == UINT64_C(0));
  if (z2zE3713) {  z8zE165 = UINT64_C(0);  } else {
    uint64_t z2zE3716;
    {
      uint64_t z2zE3714;
      {    z2zE3714 = (zwidth - UINT64_C(1));
      }
      z2zE3716 = zrlp_uint64_width(zcontent, z2zE3714);
    }
    uint64_t z2zE3717;
    {
      uint64_t z2zE3715;
      {    z2zE3715 = (zwidth - UINT64_C(1));
      }
      z2zE3717 = zslice_byte(zcontent, z2zE3715);
    }
    z8zE165 = zrlp_uint64_append(zwidth, z2zE3716, z2zE3717);
  }
end_function_3563: ;
  return z8zE165;
end_block_exception_3564: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_ref_be_length(struct zByteSliceFields zsource, uint64_t zstart, uint64_t zstop, uint64_t zcount)
{
  uint64_t z8zE166;
  uint64_t zsource_length;
  zsource_length = zsource.zlen;
  uint64_t z3zE3585;
  {
    uint64_t zstart_value;
    zstart_value = zstart;
    uint64_t z3zE3586;
    {
      uint64_t zstop_value;
      zstop_value = zstop;
      bool z2zE3706;
      z2zE3706 = (zstop_value < zstart_value);
      if (z2zE3706) {
        struct zexception z2zE3707;
        CREATE(zexception)(&z2zE3707);
        zInvalidBlock(&z2zE3707, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3707);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:418.8-418.37");
        KILL(zexception)(&z2zE3707);
        goto end_block_exception_3562;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3707);
      } else {
        bool z2zE3708;
        z2zE3708 = (zsource_length < zstop_value);
        if (z2zE3708) {
          struct zexception z2zE3709;
          CREATE(zexception)(&z2zE3709);
          zInvalidBlock(&z2zE3709, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3709);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:421.12-421.41");
          KILL(zexception)(&z2zE3709);
          goto end_block_exception_3562;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3709);
        } else {
          bool z2zE3711;
          {
            uint64_t z2zE3710;
            {    z2zE3710 = (zstop_value - zstart_value);
            }
            z2zE3711 = (z2zE3710 < zcount);
          }
          if (z2zE3711) {
            struct zexception z2zE3712;
            CREATE(zexception)(&z2zE3712);
            zInvalidBlock(&z2zE3712, zRlpDecode);
            COPY(zexception)(current_exception, z2zE3712);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:424.16-424.45");
            KILL(zexception)(&z2zE3712);
            goto end_block_exception_3562;
            /* unreachable after throw */
            KILL(zexception)(&z2zE3712);
          } else {
            struct zByteSliceFields zcontent;
            zcontent = zsub_slice(zsource, zstart_value, zcount);
            z3zE3586 = zrlp_uint64_width(zcontent, zcount);
          }
        }
      }
      goto finish_match_3559;
    }
  case_3560: ;
    sail_match_failure("rlp_ref_be_length");
  finish_match_3559: ;
    z3zE3585 = z3zE3586;
    goto finish_match_3557;
  }
case_3558: ;
  sail_match_failure("rlp_ref_be_length");
finish_match_3557: ;
  z8zE166 = z3zE3585;
end_function_3561: ;
  return z8zE166;
end_block_exception_3562: ;

  return UINT64_C(0xdeadc0de);
}

bool zrlp_bytes_equal_at(zz5listz8z5bv8z9 zexpected, struct zByteSliceFields zsource, uint64_t zstart)
{
  bool z8zE167;
  zz5listz8z5bv8z9 zrest;
  CREATE(zz5listz8z5bv8z9)(&zrest);
  COPY(zz5listz8z5bv8z9)(&zrest, zexpected);
  uint64_t zcurrent;
  zcurrent = zstart;
  bool zequal;
  zequal = true;
  int64_t z3zE3571;
  {    z3zE3571 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3572;
  {    z3zE3572 = (int64_t)(UINT64_C(7));
  }
  int64_t z3zE3573;
  {    z3zE3573 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3571;
    unit z3zE3580;
  for_start_3547: ;
    {
      if ((z3zE3572 < zbyte_index)) goto for_end_3548;
      unit z3zE3574;
      {
        if ((zrest == NULL)) goto case_3551;
        uint64_t zb;
        zb = (*zrest).hd;
        zz5listz8z5bv8z9 ztail;
        CREATE(zz5listz8z5bv8z9)(&ztail);
        COPY(zz5listz8z5bv8z9)(&ztail, (*zrest).tl);
        uint64_t zposition;
        zposition = zcurrent;
        uint64_t zsource_length;
        zsource_length = zsource.zlen;
        bool z2zE3704;
        z2zE3704 = (zposition < zsource_length);
        unit z3zE3576;
        if (z2zE3704) {
          bool z3zE3577;
          if (zequal) {
            uint64_t z2zE3705;
            z2zE3705 = zslice_byte(zsource, zcurrent);
            z3zE3577 = (z2zE3705 == zb);
          } else {  z3zE3577 = false;  }
          zequal = z3zE3577;
          unit z3zE3578;
          z3zE3578 = UNIT;
          {    zcurrent = (zposition + UINT64_C(1));
          }
          z3zE3576 = UNIT;
        } else {
          zequal = false;
          z3zE3576 = UNIT;
        }
        COPY(zz5listz8z5bv8z9)(&zrest, ztail);
        z3zE3574 = UNIT;
        KILL(zz5listz8z5bv8z9)(&ztail);
        goto finish_match_3549;
      }
    case_3551: ;
      {
        /* complete */
        z3zE3574 = UNIT;
        goto finish_match_3549;
      }
    case_3550: ;
    finish_match_3549: ;
      z3zE3580 = z3zE3574;
      zbyte_index = (zbyte_index + z3zE3573);
      goto for_start_3547;
    }
  for_end_3548: ;
  }
  unit z3zE3581;
  z3zE3581 = UNIT;
  bool z3zE3582;
  {
    if (!((zrest == NULL))) goto case_3554;
    z3zE3582 = zequal;
    goto finish_match_3552;
  }
case_3554: ;
  {
    z3zE3582 = false;
    goto finish_match_3552;
  }
case_3553: ;
finish_match_3552: ;
  z8zE167 = z3zE3582;
  KILL(zz5listz8z5bv8z9)(&zrest);
end_function_3555: ;
  return z8zE167;
end_block_exception_3556: ;

  return false;
}

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 zrlp_ref_hdr(struct zByteSliceFields zb, uint64_t zpos, uint64_t zstop)
{
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE168;
  bool z2zE3681;
  {
    bool z2zE3680;
    z2zE3680 = (zpos < zstop);
    z2zE3681 = not(z2zE3680);
  }
  unit z3zE3560;
  if (z2zE3681) {
    struct zexception z2zE3682;
    CREATE(zexception)(&z2zE3682);
    zInvalidBlock(&z2zE3682, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3682);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:471.8-471.37");
    KILL(zexception)(&z2zE3682);
    goto end_block_exception_3546;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3682);
  } else {  z3zE3560 = UNIT;  }
  uint64_t zh;
  {
    uint64_t z2zE3703;
    z2zE3703 = zslice_byte(zb, zpos);
    zh = ((uint64_t) z2zE3703);
  }
  bool z2zE3683;
  z2zE3683 = (zh < UINT64_C(128));
  if (z2zE3683) {
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3568;
    z3zE3568.ztup0 = false;
    z3zE3568.ztup1 = zpos;
    z3zE3568.ztup2 = UINT64_C(1);
    z8zE168 = z3zE3568;
  } else {
    uint64_t zposition;
    zposition = zpos;
    uint64_t zstop_position;
    zstop_position = zstop;
    bool z2zE3684;
    z2zE3684 = (!(zposition < zstop_position));
    unit z3zE3561;
    z3zE3561 = UNIT;
    uint64_t zlength_start;
    {    zlength_start = (zposition + UINT64_C(1));
    }
    bool z2zE3686;
    z2zE3686 = (zh < UINT64_C(184));
    if (z2zE3686) {
      uint64_t z2zE3687;
      {    z2zE3687 = (zh - UINT64_C(128));
      }
      struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3567;
      z3zE3567.ztup0 = false;
      z3zE3567.ztup1 = zlength_start;
      z3zE3567.ztup2 = z2zE3687;
      z8zE168 = z3zE3567;
    } else {
      bool z2zE3688;
      z2zE3688 = (zh < UINT64_C(192));
      if (z2zE3688) {
        uint64_t zlength_width;
        {    zlength_width = (zh - UINT64_C(183));
        }
        bool z2zE3691;
        {
          uint64_t z2zE3690;
          {
            uint64_t z2zE3689;
            {    z2zE3689 = (zposition + UINT64_C(1));
            }
            {    z2zE3690 = (zstop_position - z2zE3689);
            }
          }
          z2zE3691 = (z2zE3690 < zlength_width);
        }
        unit z3zE3565;
        if (z2zE3691) {
          struct zexception z2zE3692;
          CREATE(zexception)(&z2zE3692);
          zInvalidBlock(&z2zE3692, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3692);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:489.20-489.49");
          KILL(zexception)(&z2zE3692);
          goto end_block_exception_3546;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3692);
        } else {  z3zE3565 = UNIT;  }
        uint64_t zcontent;
        {
          uint64_t z2zE3694;
          {    z2zE3694 = (zposition + UINT64_C(1));
          }
          {    zcontent = (z2zE3694 + zlength_width);
          }
        }
        uint64_t z2zE3693;
        {
          z2zE3693 = zrlp_ref_be_length(zb, zlength_start, zcontent, zlength_width);
          if (have_exception) {  goto end_block_exception_3546;  }
        }
        struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3566;
        z3zE3566.ztup0 = false;
        z3zE3566.ztup1 = zcontent;
        z3zE3566.ztup2 = z2zE3693;
        z8zE168 = z3zE3566;
      } else {
        bool z2zE3695;
        z2zE3695 = (zh < UINT64_C(248));
        if (z2zE3695) {
          uint64_t z2zE3696;
          {    z2zE3696 = (zh - UINT64_C(192));
          }
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3564;
          z3zE3564.ztup0 = true;
          z3zE3564.ztup1 = zlength_start;
          z3zE3564.ztup2 = z2zE3696;
          z8zE168 = z3zE3564;
        } else {
          uint64_t z3zE3569;
          {    z3zE3569 = (zh - UINT64_C(247));
          }
          bool z2zE3699;
          {
            uint64_t z2zE3698;
            {
              uint64_t z2zE3697;
              {    z2zE3697 = (zposition + UINT64_C(1));
              }
              {    z2zE3698 = (zstop_position - z2zE3697);
              }
            }
            z2zE3699 = (z2zE3698 < z3zE3569);
          }
          unit z3zE3562;
          if (z2zE3699) {
            struct zexception z2zE3700;
            CREATE(zexception)(&z2zE3700);
            zInvalidBlock(&z2zE3700, zRlpDecode);
            COPY(zexception)(current_exception, z2zE3700);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:499.24-499.53");
            KILL(zexception)(&z2zE3700);
            goto end_block_exception_3546;
            /* unreachable after throw */
            KILL(zexception)(&z2zE3700);
          } else {  z3zE3562 = UNIT;  }
          uint64_t z3zE3570;
          {
            uint64_t z2zE3702;
            {    z2zE3702 = (zposition + UINT64_C(1));
            }
            {    z3zE3570 = (z2zE3702 + z3zE3569);
            }
          }
          uint64_t z2zE3701;
          {
            z2zE3701 = zrlp_ref_be_length(zb, zlength_start, z3zE3570, z3zE3569);
            if (have_exception) {  goto end_block_exception_3546;  }
          }
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3563;
          z3zE3563.ztup0 = true;
          z3zE3563.ztup1 = z3zE3570;
          z3zE3563.ztup2 = z2zE3701;
          z8zE168 = z3zE3563;
        }
      }
    }
  }
end_function_3545: ;
  return z8zE168;
end_block_exception_3546: ;
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE839 = { .ztup0 = false, .ztup1 = UINT64_C(0xdeadc0de), .ztup2 = UINT64_C(0xdeadc0de) };
  return z8zE839;
}

struct zRlpFieldRefFields zrlp_field_ref(struct zByteSliceFields zsource, bool zis_list, uint64_t zfull_off, uint64_t zfull_len, uint64_t zcontent_off, uint64_t zcontent_len)
{
  struct zRlpFieldRefFields z8zE169;
  struct zRlpFieldRefFields zfields;
  {
    struct zRlpFieldRefFields z3zE3559;
    z3zE3559.zcontent_len = zcontent_len;
    z3zE3559.zcontent_off = zcontent_off;
    z3zE3559.zfull_len = zfull_len;
    z3zE3559.zfull_off = zfull_off;
    z3zE3559.zis_list = zis_list;
    z3zE3559.zsource = zsource;
    zfields = z3zE3559;
  }
  z8zE169 = zfields;
end_function_3543: ;
  return z8zE169;
end_block_exception_3544: ;
  struct zByteSliceFields z8zE841 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRefFields z8zE840 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcontent_off = UINT64_C(0xdeadc0de), .zfull_len = UINT64_C(0xdeadc0de), .zfull_off = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE841 };
  return z8zE840;
}

struct zRlpCursorFields zrlp_cursor(struct zByteSliceFields zsource, uint64_t zcurrent, uint64_t zstop, bool zvalid)
{
  struct zRlpCursorFields z8zE170;
  struct zRlpCursorFields zfields;
  {
    struct zRlpCursorFields z3zE3558;
    z3zE3558.zcurrent = zcurrent;
    z3zE3558.zsource = zsource;
    z3zE3558.zstop = zstop;
    z3zE3558.zvalid = zvalid;
    zfields = z3zE3558;
  }
  z8zE170 = zfields;
end_function_3541: ;
  return z8zE170;
end_block_exception_3542: ;
  struct zByteSliceFields z8zE843 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursorFields z8zE842 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE843, .zstop = UINT64_C(0xdeadc0de), .zvalid = false };
  return z8zE842;
}

struct zRlpCursorFields zrlp_invalid_cursor(struct zByteSliceFields zsource)
{
  struct zRlpCursorFields z8zE171;
  z8zE171 = zrlp_cursor(zsource, UINT64_C(0), UINT64_C(0), false);
end_function_3539: ;
  return z8zE171;
end_block_exception_3540: ;
  struct zByteSliceFields z8zE845 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursorFields z8zE844 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE845, .zstop = UINT64_C(0xdeadc0de), .zvalid = false };
  return z8zE844;
}

struct zRlpCursorFields zrlp_node_cursor(struct zByteSliceFields znode)
{
  struct zRlpCursorFields z8zE172;
  uint64_t znode_length;
  znode_length = znode.zlen;
  bool z2zE3674;
  z2zE3674 = (znode_length == UINT64_C(0));
  if (z2zE3674) {  z8zE172 = zrlp_invalid_cursor(znode);  } else {
    uint64_t znode_stop;
    znode_stop = znode_length;
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE3675;
    {
      z2zE3675 = zrlp_ref_hdr(znode, UINT64_C(0), znode_stop);
      if (have_exception) {  goto end_block_exception_3538;  }
    }
    struct zRlpCursorFields z3zE3556;
    {
      bool zis_list;
      zis_list = z2zE3675.ztup0;
      uint64_t zcontent;
      zcontent = z2zE3675.ztup1;
      uint64_t zcontent_len;
      zcontent_len = z2zE3675.ztup2;
      uint64_t zcontent_offset;
      zcontent_offset = zcontent;
      uint64_t zpayload_length;
      zpayload_length = zcontent_len;
      bool z2zE3676;
      z2zE3676 = (!(znode_length < zcontent_offset));
      if (z2zE3676) {
        bool z2zE3678;
        {
          uint64_t z2zE3677;
          {    z2zE3677 = (znode_length - zcontent_offset);
          }
          z2zE3678 = (!(z2zE3677 < zpayload_length));
        }
        if (z2zE3678) {
          uint64_t zcontent_stop;
          {    zcontent_stop = (zcontent_offset + zpayload_length);
          }
          if (zis_list) {
            bool z2zE3679;
            z2zE3679 = (zcontent_stop == znode_stop);
            if (z2zE3679) {  z3zE3556 = zrlp_cursor(znode, zcontent, zcontent_stop, true);  } else {
              z3zE3556 = zrlp_invalid_cursor(znode);
            }
          } else {  z3zE3556 = zrlp_invalid_cursor(znode);  }
        } else {  z3zE3556 = zrlp_invalid_cursor(znode);  }
      } else {  z3zE3556 = zrlp_invalid_cursor(znode);  }
      goto finish_match_3535;
    }
  case_3536: ;
    sail_match_failure("rlp_node_cursor");
  finish_match_3535: ;
    z8zE172 = z3zE3556;
  }
end_function_3537: ;
  return z8zE172;
end_block_exception_3538: ;
  struct zByteSliceFields z8zE847 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursorFields z8zE846 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE847, .zstop = UINT64_C(0xdeadc0de), .zvalid = false };
  return z8zE846;
}

struct zRlpCursorFields zrlp_ref_cursor(struct zRlpFieldRefFields zf)
{
  struct zRlpCursorFields z8zE173;
  bool z2zE3661;
  z2zE3661 = zf.zis_list;
  if (z2zE3661) {
    uint64_t zcontent_offset;
    zcontent_offset = zf.zcontent_off;
    uint64_t zcontent_length;
    zcontent_length = zf.zcontent_len;
    uint64_t zsource_length;
    {
      struct zByteSliceFields z2zE3672;
      z2zE3672 = zf.zsource;
      zsource_length = z2zE3672.zlen;
    }
    bool z2zE3662;
    z2zE3662 = (!(zsource_length < zcontent_offset));
    bool z2zE3664;
    {
      uint64_t z2zE3663;
      {    z2zE3663 = (zsource_length - zcontent_offset);
      }
      z2zE3664 = (!(z2zE3663 < zcontent_length));
    }
    uint64_t zstop;
    {    zstop = (zcontent_offset + zcontent_length);
    }
    struct zRlpCursorFields zcursor;
    {
      struct zByteSliceFields z2zE3669;
      z2zE3669 = zf.zsource;
      zcursor = zrlp_cursor(z2zE3669, zcontent_offset, zstop, true);
    }
    uint64_t zcursor_span;
    {
      uint64_t z2zE3667;
      z2zE3667 = zcursor.zstop;
      uint64_t z2zE3668;
      z2zE3668 = zcursor.zcurrent;
      {    zcursor_span = (z2zE3667 - z2zE3668);
      }
    }
    uint64_t zfull_length;
    zfull_length = zf.zfull_len;
    bool z2zE3665;
    z2zE3665 = (!(zfull_length < zcursor_span));
    if (z2zE3665) {  z8zE173 = zcursor;  } else {
      struct zexception z2zE3666;
      CREATE(zexception)(&z2zE3666);
      zInvalidBlock(&z2zE3666, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3666);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:651.20-651.49");
      KILL(zexception)(&z2zE3666);
      goto end_block_exception_3534;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3666);
    }
  } else {
    struct zexception z2zE3673;
    CREATE(zexception)(&z2zE3673);
    zInvalidBlock(&z2zE3673, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3673);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:660.8-660.37");
    KILL(zexception)(&z2zE3673);
    goto end_block_exception_3534;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3673);
  }
end_function_3533: ;
  return z8zE173;
end_block_exception_3534: ;
  struct zByteSliceFields z8zE849 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursorFields z8zE848 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE849, .zstop = UINT64_C(0xdeadc0de), .zvalid = false };
  return z8zE848;
}

bool zrlp_cursor_empty(struct zRlpCursorFields zcursor)
{
  bool z8zE174;
  bool z2zE3660;
  z2zE3660 = zcursor.zvalid;
  bool z3zE3555;
  if (z2zE3660) {
    uint64_t z2zE3658;
    z2zE3658 = zcursor.zcurrent;
    uint64_t z2zE3659;
    z2zE3659 = zcursor.zstop;
    z3zE3555 = (z2zE3658 == z2zE3659);
  } else {  z3zE3555 = false;  }
  z8zE174 = z3zE3555;
end_function_3531: ;
  return z8zE174;
end_block_exception_3532: ;

  return false;
}

struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 zrlp_cursor_pop(struct zRlpCursorFields zcursor)
{
  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z8zE175;
  uint64_t zcurrent;
  zcurrent = zcursor.zcurrent;
  uint64_t zstop;
  zstop = zcursor.zstop;
  bool z2zE3643;
  {
    bool z2zE3642;
    {
      bool z2zE3640;
      z2zE3640 = zcursor.zvalid;
      z2zE3642 = not(z2zE3640);
    }
    bool z3zE3538;
    if (z2zE3642) {  z3zE3538 = true;  } else {
      bool z2zE3641;
      z2zE3641 = (zcurrent < zstop);
      z3zE3538 = not(z2zE3641);
    }
    z2zE3643 = z3zE3538;
  }
  unit z3zE3539;
  if (z2zE3643) {
    struct zexception z2zE3644;
    CREATE(zexception)(&z2zE3644);
    zInvalidBlock(&z2zE3644, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3644);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:710.8-710.37");
    KILL(zexception)(&z2zE3644);
    goto end_block_exception_3530;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3644);
  } else {  z3zE3539 = UNIT;  }
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE3646;
  {
    struct zByteSliceFields z2zE3645;
    z2zE3645 = zcursor.zsource;
    {
      z2zE3646 = zrlp_ref_hdr(z2zE3645, zcurrent, zstop);
      if (have_exception) {  goto end_block_exception_3530;  }
    }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3540;
  {
    bool zis_list;
    zis_list = z2zE3646.ztup0;
    uint64_t zcontent_value;
    zcontent_value = z2zE3646.ztup1;
    uint64_t zcontent_len_value;
    zcontent_len_value = z2zE3646.ztup2;
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3541;
    {
      uint64_t zcontent;
      zcontent = zcontent_value;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3542;
      {
        uint64_t zcontent_len;
        zcontent_len = zcontent_len_value;
        uint64_t zcontent_offset;
        zcontent_offset = zcontent;
        uint64_t zpayload_length;
        zpayload_length = zcontent_len;
        uint64_t zstop_offset;
        zstop_offset = zstop;
        bool z2zE3647;
        z2zE3647 = (zstop_offset < zcontent_offset);
        unit z3zE3544;
        if (z2zE3647) {
          struct zexception z2zE3648;
          CREATE(zexception)(&z2zE3648);
          zInvalidBlock(&z2zE3648, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3648);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:719.8-719.37");
          KILL(zexception)(&z2zE3648);
          goto end_block_exception_3530;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3648);
        } else {  z3zE3544 = UNIT;  }
        bool z2zE3650;
        {
          uint64_t z2zE3649;
          {    z2zE3649 = (zstop_offset - zcontent_offset);
          }
          z2zE3650 = (z2zE3649 < zpayload_length);
        }
        unit z3zE3543;
        if (z2zE3650) {
          struct zexception z2zE3651;
          CREATE(zexception)(&z2zE3651);
          zInvalidBlock(&z2zE3651, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3651);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:722.8-722.37");
          KILL(zexception)(&z2zE3651);
          goto end_block_exception_3530;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3651);
        } else {  z3zE3543 = UNIT;  }
        uint64_t z2zE3652;
        {    z2zE3652 = (zcontent_offset + zpayload_length);
        }
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3545;
        {
          uint64_t znext;
          znext = z2zE3652;
          uint64_t zcurrent_offset;
          zcurrent_offset = zcurrent;
          uint64_t znext_offset;
          znext_offset = znext;
          bool z2zE3653;
          z2zE3653 = (zcurrent_offset < znext_offset);
          if (z2zE3653) {
            uint64_t z2zE3654;
            {    z2zE3654 = (znext_offset - zcurrent_offset);
            }
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3547;
            {
              uint64_t zfull_len;
              zfull_len = z2zE3654;
              struct zRlpFieldRefFields zfield;
              {
                struct zByteSliceFields z2zE3656;
                z2zE3656 = zcursor.zsource;
                struct zRlpFieldRefFields z3zE3548;
                z3zE3548.zcontent_len = zcontent_len;
                z3zE3548.zcontent_off = zcontent;
                z3zE3548.zfull_len = zfull_len;
                z3zE3548.zfull_off = zcurrent;
                z3zE3548.zis_list = zis_list;
                z3zE3548.zsource = z2zE3656;
                zfield = z3zE3548;
              }
              struct zRlpCursorFields zadvanced;
              {
                struct zByteSliceFields z2zE3655;
                z2zE3655 = zcursor.zsource;
                struct zRlpCursorFields z3zE3549;
                z3zE3549.zcurrent = znext;
                z3zE3549.zsource = z2zE3655;
                z3zE3549.zstop = zstop;
                z3zE3549.zvalid = true;
                zadvanced = z3zE3549;
              }
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z3zE3550;
              z3zE3550.ztup0 = zfield;
              z3zE3550.ztup1 = zadvanced;
              z3zE3547 = z3zE3550;
              goto finish_match_3527;
            }
          case_3528: ;
            sail_match_failure("rlp_cursor_pop");
          finish_match_3527: ;
            z3zE3545 = z3zE3547;
          } else {
            struct zexception z2zE3657;
            CREATE(zexception)(&z2zE3657);
            zInvalidBlock(&z2zE3657, zRlpDecode);
            COPY(zexception)(current_exception, z2zE3657);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:745.8-745.37");
            KILL(zexception)(&z2zE3657);
            goto end_block_exception_3530;
            /* unreachable after throw */
            KILL(zexception)(&z2zE3657);
          }
          goto finish_match_3525;
        }
      case_3526: ;
        sail_match_failure("rlp_cursor_pop");
      finish_match_3525: ;
        z3zE3542 = z3zE3545;
        goto finish_match_3523;
      }
    case_3524: ;
      sail_match_failure("rlp_cursor_pop");
    finish_match_3523: ;
      z3zE3541 = z3zE3542;
      goto finish_match_3521;
    }
  case_3522: ;
    sail_match_failure("rlp_cursor_pop");
  finish_match_3521: ;
    z3zE3540 = z3zE3541;
    goto finish_match_3519;
  }
case_3520: ;
  sail_match_failure("rlp_cursor_pop");
finish_match_3519: ;
  z8zE175 = z3zE3540;
end_function_3529: ;
  return z8zE175;
end_block_exception_3530: ;
  struct zByteSliceFields z8zE854 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRefFields z8zE853 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcontent_off = UINT64_C(0xdeadc0de), .zfull_len = UINT64_C(0xdeadc0de), .zfull_off = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE854 };
  struct zByteSliceFields z8zE852 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursorFields z8zE851 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE852, .zstop = UINT64_C(0xdeadc0de), .zvalid = false };
  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z8zE850 = { .ztup0 = z8zE853, .ztup1 = z8zE851 };
  return z8zE850;
}

unit zrlp_cursor_expect_end(struct zRlpCursorFields zcursor)
{
  unit z8zE176;
  bool z2zE3638;
  z2zE3638 = zrlp_cursor_empty(zcursor);
  if (z2zE3638) {  z8zE176 = UNIT;  } else {
    struct zexception z2zE3639;
    CREATE(zexception)(&z2zE3639);
    zInvalidBlock(&z2zE3639, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3639);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:755.8-755.37");
    KILL(zexception)(&z2zE3639);
    goto end_block_exception_3518;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3639);
  }
end_function_3517: ;
  return z8zE176;
end_block_exception_3518: ;

  return UNIT;
}

struct zRlpFieldRefFields zrlp_single_ref(struct zByteSliceFields zitem)
{
  struct zRlpFieldRefFields z8zE177;
  uint64_t zitem_length;
  zitem_length = zitem.zlen;
  bool z2zE3629;
  z2zE3629 = (zitem_length == UINT64_C(0));
  if (z2zE3629) {
    struct zexception z2zE3630;
    CREATE(zexception)(&z2zE3630);
    zInvalidBlock(&z2zE3630, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3630);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:762.8-762.37");
    KILL(zexception)(&z2zE3630);
    goto end_block_exception_3516;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3630);
  } else {
    uint64_t zitem_stop;
    zitem_stop = zitem_length;
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE3631;
    {
      z2zE3631 = zrlp_ref_hdr(zitem, UINT64_C(0), zitem_stop);
      if (have_exception) {  goto end_block_exception_3516;  }
    }
    struct zRlpFieldRefFields z3zE3534;
    {
      bool zisl;
      zisl = z2zE3631.ztup0;
      uint64_t zcs;
      zcs = z2zE3631.ztup1;
      uint64_t zcl;
      zcl = z2zE3631.ztup2;
      uint64_t zcontent_offset;
      zcontent_offset = zcs;
      uint64_t zcontent_length;
      zcontent_length = zcl;
      bool z2zE3632;
      z2zE3632 = (zitem_length < zcontent_offset);
      unit z3zE3536;
      if (z2zE3632) {
        struct zexception z2zE3633;
        CREATE(zexception)(&z2zE3633);
        zInvalidBlock(&z2zE3633, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3633);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:769.12-769.41");
        KILL(zexception)(&z2zE3633);
        goto end_block_exception_3516;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3633);
      } else {  z3zE3536 = UNIT;  }
      bool z2zE3635;
      {
        uint64_t z2zE3634;
        {    z2zE3634 = (zitem_length - zcontent_offset);
        }
        z2zE3635 = (zcontent_length != z2zE3634);
      }
      unit z3zE3535;
      if (z2zE3635) {
        struct zexception z2zE3636;
        CREATE(zexception)(&z2zE3636);
        zInvalidBlock(&z2zE3636, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3636);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:772.12-772.41");
        KILL(zexception)(&z2zE3636);
        goto end_block_exception_3516;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3636);
      } else {  z3zE3535 = UNIT;  }
      uint64_t z2zE3637;
      z2zE3637 = zitem.zlen;
      z3zE3534 = zrlp_field_ref(zitem, zisl, UINT64_C(0), z2zE3637, zcs, zcl);
      goto finish_match_3513;
    }
  case_3514: ;
    sail_match_failure("rlp_single_ref");
  finish_match_3513: ;
    z8zE177 = z3zE3534;
  }
end_function_3515: ;
  return z8zE177;
end_block_exception_3516: ;
  struct zByteSliceFields z8zE856 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRefFields z8zE855 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcontent_off = UINT64_C(0xdeadc0de), .zfull_len = UINT64_C(0xdeadc0de), .zfull_off = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE856 };
  return z8zE855;
}

struct zByteSliceFields zrlp_ref_content(struct zRlpFieldRefFields zf)
{
  struct zByteSliceFields z8zE178;
  struct zByteSliceFields z2zE3626;
  z2zE3626 = zf.zsource;
  uint64_t z2zE3627;
  z2zE3627 = zf.zcontent_off;
  uint64_t z2zE3628;
  z2zE3628 = zf.zcontent_len;
  z8zE178 = zsub_slice(z2zE3626, z2zE3627, z2zE3628);
end_function_3511: ;
  return z8zE178;
end_block_exception_3512: ;
  struct zByteSliceFields z8zE857 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE857;
}

struct zByteSliceFields zrlp_ref_full(struct zRlpFieldRefFields zf)
{
  struct zByteSliceFields z8zE179;
  struct zByteSliceFields z2zE3623;
  z2zE3623 = zf.zsource;
  uint64_t z2zE3624;
  z2zE3624 = zf.zfull_off;
  uint64_t z2zE3625;
  z2zE3625 = zf.zfull_len;
  z8zE179 = zsub_slice(z2zE3623, z2zE3624, z2zE3625);
end_function_3509: ;
  return z8zE179;
end_block_exception_3510: ;
  struct zByteSliceFields z8zE858 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE858;
}

bool zrlp_ref_framing_canonical(struct zRlpFieldRefFields zf)
{
  bool z8zE180;
  uint64_t zn;
  zn = zf.zcontent_len;
  uint64_t zpayload_length;
  zpayload_length = zn;
  uint64_t zfull_length;
  zfull_length = zf.zfull_len;
  uint64_t zfo;
  zfo = zf.zfull_off;
  uint64_t zfull_offset;
  zfull_offset = zfo;
  uint64_t zco;
  zco = zf.zcontent_off;
  struct zByteSliceFields zsource;
  zsource = zf.zsource;
  uint64_t zsource_length;
  zsource_length = zsource.zlen;
  bool z2zE3570;
  z2zE3570 = (zfull_length == UINT64_C(0));
  if (z2zE3570) {  z8zE180 = false;  } else {
    bool z2zE3571;
    z2zE3571 = zf.zis_list;
    if (z2zE3571) {
      bool z2zE3572;
      z2zE3572 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
      if (z2zE3572) {
        bool z2zE3578;
        {
          uint64_t z2zE3573;
          {    z2zE3573 = (zpayload_length + UINT64_C(1));
          }
          z2zE3578 = (zfull_length == z2zE3573);
        }
        bool z3zE3531;
        if (z2zE3578) {
          uint64_t z2zE3576;
          {
            struct zByteSliceFields z2zE3574;
            z2zE3574 = zf.zsource;
            z2zE3576 = zslice_byte(z2zE3574, zfo);
          }
          uint64_t z2zE3577;
          {
            uint64_t z2zE3575;
            z2zE3575 = zrlp_nat_length_byte(zn);
            z2zE3577 = ((UINT64_C(0xC0) + z2zE3575) & UINT64_C(0xFF));
          }
          z3zE3531 = (z2zE3576 == z2zE3577);
        } else {  z3zE3531 = false;  }
        z8zE180 = z3zE3531;
      } else {
        struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3580;
        CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3580);
        {
          sail_u256 z2zE3579;
          z2zE3579 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
          zminimal_word_bytes(&z2zE3580, z2zE3579);
        }
        bool z3zE3527;
        {
          zz5listz8z5bv8z9 zlen_bytes;
          CREATE(zz5listz8z5bv8z9)(&zlen_bytes);
          COPY(zz5listz8z5bv8z9)(&zlen_bytes, z2zE3580.ztup0);
          uint64_t zlength_width;
          zlength_width = z2zE3580.ztup1;
          bool z2zE3591;
          {
            sail_u128 z2zE3582;
            {
              uint64_t z2zE3581;
              {    z2zE3581 = (UINT64_C(1) + zlength_width);
              }
              z2zE3582 = u128_add_u64_u64(z2zE3581, zpayload_length);
            }
            z2zE3591 = u128_eq_u64(z2zE3582, zfull_length);
          }
          bool z3zE3529;
          if (z2zE3591) {
            bool z2zE3590;
            {
              uint64_t z2zE3585;
              {
                struct zByteSliceFields z2zE3583;
                z2zE3583 = zf.zsource;
                z2zE3585 = zslice_byte(z2zE3583, zfo);
              }
              uint64_t z2zE3586;
              {
                uint64_t z2zE3584;
                z2zE3584 = zrlp_nat_length_byte(zlength_width);
                z2zE3586 = ((UINT64_C(0xF7) + z2zE3584) & UINT64_C(0xFF));
              }
              z2zE3590 = (z2zE3585 == z2zE3586);
            }
            bool z3zE3528;
            if (z2zE3590) {
              bool z2zE3587;
              z2zE3587 = (zfull_offset < zsource_length);
              struct zByteSliceFields z2zE3588;
              z2zE3588 = zf.zsource;
              uint64_t z2zE3589;
              {    z2zE3589 = (zfull_offset + UINT64_C(1));
              }
              z3zE3528 = zrlp_bytes_equal_at(zlen_bytes, z2zE3588, z2zE3589);
            } else {  z3zE3528 = false;  }
            z3zE3529 = z3zE3528;
          } else {  z3zE3529 = false;  }
          z3zE3527 = z3zE3529;
          KILL(zz5listz8z5bv8z9)(&zlen_bytes);
          goto finish_match_3505;
        }
      case_3506: ;
        sail_match_failure("rlp_ref_framing_canonical");
      finish_match_3505: ;
        z8zE180 = z3zE3527;
        KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3580);
      }
    } else {
      bool z2zE3592;
      z2zE3592 = (zpayload_length == UINT64_C(0));
      if (z2zE3592) {
        bool z2zE3595;
        z2zE3595 = (zfull_length == UINT64_C(1));
        bool z3zE3526;
        if (z2zE3595) {
          uint64_t z2zE3594;
          {
            struct zByteSliceFields z2zE3593;
            z2zE3593 = zf.zsource;
            z2zE3594 = zslice_byte(z2zE3593, zfo);
          }
          z3zE3526 = (z2zE3594 == UINT64_C(0x80));
        } else {  z3zE3526 = false;  }
        z8zE180 = z3zE3526;
      } else {
        uint64_t zfirst;
        {
          struct zByteSliceFields z2zE3622;
          z2zE3622 = zf.zsource;
          zfirst = zslice_byte(z2zE3622, zco);
        }
        bool z2zE3598;
        {
          bool z2zE3597;
          z2zE3597 = (zpayload_length == UINT64_C(1));
          bool z3zE3519;
          if (z2zE3597) {
            uint64_t z2zE3596;
            z2zE3596 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
            z3zE3519 = (z2zE3596 == UINT64_C(0b0));
          } else {  z3zE3519 = false;  }
          z2zE3598 = z3zE3519;
        }
        if (z2zE3598) {
          bool z2zE3601;
          z2zE3601 = (zfull_length == UINT64_C(1));
          bool z3zE3525;
          if (z2zE3601) {
            uint64_t z2zE3600;
            {
              struct zByteSliceFields z2zE3599;
              z2zE3599 = zf.zsource;
              z2zE3600 = zslice_byte(z2zE3599, zfo);
            }
            z3zE3525 = (z2zE3600 == zfirst);
          } else {  z3zE3525 = false;  }
          z8zE180 = z3zE3525;
        } else {
          bool z2zE3602;
          z2zE3602 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
          if (z2zE3602) {
            bool z2zE3608;
            {
              uint64_t z2zE3603;
              {    z2zE3603 = (zpayload_length + UINT64_C(1));
              }
              z2zE3608 = (zfull_length == z2zE3603);
            }
            bool z3zE3524;
            if (z2zE3608) {
              uint64_t z2zE3606;
              {
                struct zByteSliceFields z2zE3604;
                z2zE3604 = zf.zsource;
                z2zE3606 = zslice_byte(z2zE3604, zfo);
              }
              uint64_t z2zE3607;
              {
                uint64_t z2zE3605;
                z2zE3605 = zrlp_nat_length_byte(zn);
                z2zE3607 = ((UINT64_C(0x80) + z2zE3605) & UINT64_C(0xFF));
              }
              z3zE3524 = (z2zE3606 == z2zE3607);
            } else {  z3zE3524 = false;  }
            z8zE180 = z3zE3524;
          } else {
            struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3610;
            CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3610);
            {
              sail_u256 z2zE3609;
              z2zE3609 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
              zminimal_word_bytes(&z2zE3610, z2zE3609);
            }
            bool z3zE3520;
            {
              zz5listz8z5bv8z9 z3zE3532;
              CREATE(zz5listz8z5bv8z9)(&z3zE3532);
              COPY(zz5listz8z5bv8z9)(&z3zE3532, z2zE3610.ztup0);
              uint64_t z3zE3533;
              z3zE3533 = z2zE3610.ztup1;
              bool z2zE3621;
              {
                sail_u128 z2zE3612;
                {
                  uint64_t z2zE3611;
                  {    z2zE3611 = (UINT64_C(1) + z3zE3533);
                  }
                  z2zE3612 = u128_add_u64_u64(z2zE3611, zpayload_length);
                }
                z2zE3621 = u128_eq_u64(z2zE3612, zfull_length);
              }
              bool z3zE3522;
              if (z2zE3621) {
                bool z2zE3620;
                {
                  uint64_t z2zE3615;
                  {
                    struct zByteSliceFields z2zE3613;
                    z2zE3613 = zf.zsource;
                    z2zE3615 = zslice_byte(z2zE3613, zfo);
                  }
                  uint64_t z2zE3616;
                  {
                    uint64_t z2zE3614;
                    z2zE3614 = zrlp_nat_length_byte(z3zE3533);
                    z2zE3616 = ((UINT64_C(0xB7) + z2zE3614) & UINT64_C(0xFF));
                  }
                  z2zE3620 = (z2zE3615 == z2zE3616);
                }
                bool z3zE3521;
                if (z2zE3620) {
                  bool z2zE3617;
                  z2zE3617 = (zfull_offset < zsource_length);
                  struct zByteSliceFields z2zE3618;
                  z2zE3618 = zf.zsource;
                  uint64_t z2zE3619;
                  {    z2zE3619 = (zfull_offset + UINT64_C(1));
                  }
                  z3zE3521 = zrlp_bytes_equal_at(z3zE3532, z2zE3618, z2zE3619);
                } else {  z3zE3521 = false;  }
                z3zE3522 = z3zE3521;
              } else {  z3zE3522 = false;  }
              z3zE3520 = z3zE3522;
              KILL(zz5listz8z5bv8z9)(&z3zE3532);
              goto finish_match_3503;
            }
          case_3504: ;
            sail_match_failure("rlp_ref_framing_canonical");
          finish_match_3503: ;
            z8zE180 = z3zE3520;
            KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3610);
          }
        }
      }
    }
  }
end_function_3507: ;
  return z8zE180;
end_block_exception_3508: ;

  return false;
}

bool zrlp_ref_bytes_canonical(struct zRlpFieldRefFields zf)
{
  bool z8zE181;
  bool z2zE3569;
  {
    bool z2zE3568;
    z2zE3568 = zf.zis_list;
    z2zE3569 = not(z2zE3568);
  }
  bool z3zE3518;
  if (z2zE3569) {  z3zE3518 = zrlp_ref_framing_canonical(zf);  } else {  z3zE3518 = false;  }
  z8zE181 = z3zE3518;
end_function_3501: ;
  return z8zE181;
end_block_exception_3502: ;

  return false;
}

bool zrlp_ref_uint_canonical(struct zRlpFieldRefFields zf)
{
  bool z8zE182;
  bool z2zE3562;
  {
    bool z2zE3561;
    z2zE3561 = zrlp_ref_bytes_canonical(zf);
    z2zE3562 = not(z2zE3561);
  }
  if (z2zE3562) {  z8zE182 = false;  } else {
    bool z2zE3567;
    {
      uint64_t z2zE3563;
      z2zE3563 = zf.zcontent_len;
      z2zE3567 = (z2zE3563 == UINT64_C(0));
    }
    bool z3zE3517;
    if (z2zE3567) {  z3zE3517 = true;  } else {
      uint64_t z2zE3566;
      {
        struct zByteSliceFields z2zE3564;
        z2zE3564 = zf.zsource;
        uint64_t z2zE3565;
        z2zE3565 = zf.zcontent_off;
        z2zE3566 = zslice_byte(z2zE3564, z2zE3565);
      }
      z3zE3517 = (z2zE3566 != UINT64_C(0x00));
    }
    z8zE182 = z3zE3517;
  }
end_function_3499: ;
  return z8zE182;
end_block_exception_3500: ;

  return false;
}

sail_u256 zrlp_ref_word(struct zRlpFieldRefFields zf)
{
  sail_u256 z8zE183;
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE3557;
  {
    bool z2zE3556;
    z2zE3556 = zf.zis_list;
    bool z3zE3516;
    if (z2zE3556) {  z3zE3516 = true;  } else {  z3zE3516 = (zRLP_WORD_LENGTH_LIMIT < zn);  }
    z2zE3557 = z3zE3516;
  }
  if (z2zE3557) {
    struct zexception z2zE3558;
    CREATE(zexception)(&z2zE3558);
    zInvalidBlock(&z2zE3558, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3558);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:887.8-887.37");
    KILL(zexception)(&z2zE3558);
    goto end_block_exception_3498;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3558);
  } else {
    struct zByteSliceFields z2zE3559;
    z2zE3559 = zf.zsource;
    uint64_t z2zE3560;
    z2zE3560 = zf.zcontent_off;
    z8zE183 = zslice_load_n(z2zE3559, z2zE3560, zn);
  }
end_function_3497: ;
  return z8zE183;
end_block_exception_3498: ;

  return u256_zero();
}

sail_u256 zrlp_ref_uint_word(struct zRlpFieldRefFields zf)
{
  sail_u256 z8zE184;
  bool z2zE3554;
  z2zE3554 = zrlp_ref_uint_canonical(zf);
  if (z2zE3554) {
    {
      z8zE184 = zrlp_ref_word(zf);
      if (have_exception) {  goto end_block_exception_3496;  }
    }
  } else {
    struct zexception z2zE3555;
    CREATE(zexception)(&z2zE3555);
    zInvalidBlock(&z2zE3555, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3555);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:899.8-899.37");
    KILL(zexception)(&z2zE3555);
    goto end_block_exception_3496;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3555);
  }
end_function_3495: ;
  return z8zE184;
end_block_exception_3496: ;

  return u256_zero();
}

uint64_t zrlp_ref_uint64(struct zRlpFieldRefFields zf)
{
  uint64_t z8zE185;
  bool z2zE3552;
  {
    bool z2zE3551;
    {
      bool z2zE3549;
      z2zE3549 = zrlp_ref_uint_canonical(zf);
      z2zE3551 = not(z2zE3549);
    }
    bool z3zE3514;
    if (z2zE3551) {  z3zE3514 = true;  } else {
      uint64_t z2zE3550;
      z2zE3550 = zf.zcontent_len;
      z3zE3514 = (zRLP_UINT64_LENGTH_LIMIT < z2zE3550);
    }
    z2zE3552 = z3zE3514;
  }
  unit z3zE3515;
  if (z2zE3552) {
    struct zexception z2zE3553;
    CREATE(zexception)(&z2zE3553);
    zInvalidBlock(&z2zE3553, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3553);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:906.8-906.37");
    KILL(zexception)(&z2zE3553);
    goto end_block_exception_3494;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3553);
  } else {  z3zE3515 = UNIT;  }
  uint64_t zcontent_length;
  zcontent_length = zf.zcontent_len;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zf);
  uint64_t zwidth;
  {    zwidth = (zcontent_length % UINT64_C(9));
  }
  z8zE185 = zrlp_uint64_width(zcontent, zwidth);
end_function_3493: ;
  return z8zE185;
end_block_exception_3494: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_ref_account_nonce(struct zRlpFieldRefFields zf)
{
  uint64_t z8zE186;
  {
    z8zE186 = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3492;  }
  }
end_function_3491: ;
  return z8zE186;
end_block_exception_3492: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_ref_blob_gas_used(struct zRlpFieldRefFields zf)
{
  uint64_t z8zE187;
  uint64_t zvalue;
  {
    zvalue = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3490;  }
  }
  bool z2zE3547;
  z2zE3547 = (!(UINT64_C(2752512) < zvalue));
  if (z2zE3547) {  z8zE187 = zvalue;  } else {
    struct zexception z2zE3548;
    CREATE(zexception)(&z2zE3548);
    zInvalidBlock(&z2zE3548, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3548);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:947.8-947.37");
    KILL(zexception)(&z2zE3548);
    goto end_block_exception_3490;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3548);
  }
end_function_3489: ;
  return z8zE187;
end_block_exception_3490: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_ref_excess_blob_gas(struct zRlpFieldRefFields zf)
{
  uint64_t z8zE188;
  {
    z8zE188 = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3488;  }
  }
end_function_3487: ;
  return z8zE188;
end_block_exception_3488: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_ref_chain_identifier(struct zRlpFieldRefFields zf)
{
  uint64_t z8zE189;
  {
    z8zE189 = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3486;  }
  }
end_function_3485: ;
  return z8zE189;
end_block_exception_3486: ;

  return UINT64_C(0xdeadc0de);
}

