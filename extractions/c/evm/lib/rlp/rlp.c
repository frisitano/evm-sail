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
end_function_3623: ;
  return z8zE146;
end_block_exception_3624: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_byte_count(uint64_t zindex)
{
  uint64_t z8zE147;
  {    z8zE147 = (zindex + UINT64_C(1));
  }
end_function_3619: ;
  return z8zE147;
end_block_exception_3620: ;

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
  int64_t z3zE3553;
  {    z3zE3553 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3554;
  {    z3zE3554 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3555;
  {    z3zE3555 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3553;
    unit z3zE3556;
  for_start_3615: ;
    {
      if ((z3zE3554 < zbyte_index)) goto for_end_3616;
      bool z2zE3741;
      z2zE3741 = zword_nonzzero(zremaining);
      if (z2zE3741) {
        uint64_t z2zE3742;
        z2zE3742 = zword_low_byte(zremaining);
        zconsz3z5bv8(&zout, z2zE3742, zout);
        unit z3zE3558;
        z3zE3558 = UNIT;
        zremaining = zword_shift_right(zremaining, UINT64_C(8));
        unit z3zE3557;
        z3zE3557 = UNIT;
        {
          uint64_t z3zE3784;
          z3zE3784 = (uint64_t)(zbyte_index);
          zlen = zword_byte_count(z3zE3784);
        }
        z3zE3556 = UNIT;
      } else {  z3zE3556 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE3555);
      goto for_start_3615;
    }
  for_end_3616: ;
  }
  unit z3zE3559;
  z3zE3559 = UNIT;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3560;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3560);
  COPY(zz5listz8z5bv8z9)(&((&z3zE3560)->ztup0), zout);
  z3zE3560.ztup1 = zlen;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE148)), z3zE3560);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3560);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3617: ;
  goto end_function_4072;
end_block_exception_3618: ;
  goto end_function_4072;
end_function_4072: ;
}

uint64_t zrlp_nat_length_byte(uint64_t zvalue)
{
  uint64_t z8zE149;
  z8zE149 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
end_function_3613: ;
  return z8zE149;
end_block_exception_3614: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_minimal_word_len(sail_u256 zw)
{
  uint64_t z8zE150;
  z8zE150 = zword_byte_length(zw);
end_function_3601: ;
  return z8zE150;
end_block_exception_3602: ;

  return UINT64_C(0xdeadc0de);
}

sail_u128 zrlp_slice_sizze(struct zByteSliceFields zdata)
{
  sail_u128 z8zE151;
  uint64_t zlen;
  zlen = zdata.zlen;
  uint64_t zfirst;
  {
    bool z2zE3710;
    z2zE3710 = (zlen == UINT64_C(0));
    if (z2zE3710) {  zfirst = UINT64_C(0x00);  } else {  zfirst = zslice_byte(zdata, UINT64_C(0));  }
  }
  z8zE151 = zrlp_string_sizzezIreprzGU64zCB8zCR__sail_c_repr_u128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlen, zfirst);
end_function_3590: ;
  return z8zE151;
end_block_exception_3591: ;

  return u128_zero();
}

uint64_t zrlp_uint_word_sizze(sail_u256 zw)
{
  uint64_t z8zE152;
  uint64_t zlen;
  zlen = zrlp_minimal_word_len(zw);
  uint64_t zfirst;
  {
    bool z2zE3709;
    z2zE3709 = (zlen == UINT64_C(1));
    if (z2zE3709) {  zfirst = zword_low_byte(zw);  } else {  zfirst = UINT64_C(0x00);  }
  }
  bool z2zE3708;
  {
    bool z2zE3707;
    z2zE3707 = (zlen == UINT64_C(1));
    bool z3zE3534;
    if (z2zE3707) {
      uint64_t z2zE3706;
      z2zE3706 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3534 = (z2zE3706 == UINT64_C(0b0));
    } else {  z3zE3534 = false;  }
    z2zE3708 = z3zE3534;
  }
  if (z2zE3708) {  z8zE152 = UINT64_C(1);  } else {
    {    z8zE152 = (UINT64_C(1) + zlen);
    }
  }
end_function_3588: ;
  return z8zE152;
end_block_exception_3589: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_nat_sizze(uint64_t zn)
{
  uint64_t z8zE153;
  sail_u256 z2zE3705;
  z2zE3705 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE153 = zrlp_uint_word_sizze(z2zE3705);
end_function_3586: ;
  return z8zE153;
end_block_exception_3587: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_list_sizze(uint64_t zcontent_len)
{
  uint64_t z8zE154;
  uint64_t z2zE3703;
  {
    uint64_t z2zE3702;
    z2zE3702 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
    z2zE3703 = zrlp_scratch_small_length(z2zE3702);
  }
  {
    z8zE154 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE3703);
    if (have_exception) {  goto end_block_exception_3583;  }
  }
end_function_3582: ;
  return z8zE154;
end_block_exception_3583: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_slice_sizze(struct zByteSliceFields zdata)
{
  uint64_t z8zE155;
  uint64_t zlength;
  zlength = zdata.zlen;
  uint64_t zfirst;
  {
    bool z2zE3701;
    z2zE3701 = (zlength == UINT64_C(0));
    if (z2zE3701) {  zfirst = UINT64_C(0x00);  } else {  zfirst = zslice_byte(zdata, UINT64_C(0));  }
  }
  bool z2zE3698;
  {
    bool z2zE3697;
    z2zE3697 = (zlength == UINT64_C(1));
    bool z3zE3533;
    if (z2zE3697) {
      uint64_t z2zE3696;
      z2zE3696 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3533 = (z2zE3696 == UINT64_C(0b0));
    } else {  z3zE3533 = false;  }
    z2zE3698 = z3zE3533;
  }
  if (z2zE3698) {  z8zE155 = UINT64_C(1);  } else {
    uint64_t z2zE3700;
    {
      uint64_t z2zE3699;
      z2zE3699 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE3700 = zrlp_scratch_small_length(z2zE3699);
    }
    {
      z8zE155 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE3700);
      if (have_exception) {  goto end_block_exception_3581;  }
    }
  }
end_function_3580: ;
  return z8zE155;
end_block_exception_3581: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_bytes_sizze(zz5listz8z5bv8z9 zdata, uint64_t zlength)
{
  uint64_t z8zE156;
  uint64_t zfirst;
  {
    uint64_t z3zE3529;
    {
      if ((zdata == NULL)) goto case_3577;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE3529 = zb;
      goto finish_match_3575;
    }
  case_3577: ;
    {
      /* complete */
      z3zE3529 = UINT64_C(0x00);
      goto finish_match_3575;
    }
  case_3576: ;
  finish_match_3575: ;
    zfirst = z3zE3529;
  }
  bool z2zE3693;
  {
    bool z2zE3692;
    z2zE3692 = (zlength == UINT64_C(1));
    bool z3zE3532;
    if (z2zE3692) {
      uint64_t z2zE3691;
      z2zE3691 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3532 = (z2zE3691 == UINT64_C(0b0));
    } else {  z3zE3532 = false;  }
    z2zE3693 = z3zE3532;
  }
  if (z2zE3693) {  z8zE156 = UINT64_C(1);  } else {
    uint64_t z2zE3695;
    {
      uint64_t z2zE3694;
      z2zE3694 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlength);
      z2zE3695 = zrlp_scratch_small_length(z2zE3694);
    }
    {
      z8zE156 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlength, z2zE3695);
      if (have_exception) {  goto end_block_exception_3579;  }
    }
  }
end_function_3578: ;
  return z8zE156;
end_block_exception_3579: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_bytes(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  unit z8zE157;
  uint64_t zfirst;
  {
    uint64_t z3zE3521;
    {
      if ((zdata == NULL)) goto case_3564;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE3521 = zb;
      goto finish_match_3562;
    }
  case_3564: ;
    {
      /* complete */
      z3zE3521 = UINT64_C(0x00);
      goto finish_match_3562;
    }
  case_3563: ;
  finish_match_3562: ;
    zfirst = z3zE3521;
  }
  unit z3zE3524;
  z3zE3524 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlen, zfirst);
  z8zE157 = zscratch_push_bytes(zdata, zlen);
end_function_3565: ;
  return z8zE157;
end_block_exception_3566: ;

  return UNIT;
}

unit zrlp_write_slice(struct zByteSliceFields zdata)
{
  unit z8zE158;
  uint64_t z2zE3687;
  z2zE3687 = zdata.zlen;
  uint64_t z2zE3688;
  {
    bool z2zE3686;
    {
      uint64_t z2zE3685;
      z2zE3685 = zdata.zlen;
      z2zE3686 = (z2zE3685 == UINT64_C(0));
    }
    if (z2zE3686) {  z2zE3688 = UINT64_C(0x00);  } else {  z2zE3688 = zslice_byte(zdata, UINT64_C(0));  }
  }
  unit z3zE3520;
  z3zE3520 = zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3687, z2zE3688);
  z8zE158 = zscratch_push_slice(zdata);
end_function_3560: ;
  return z8zE158;
end_block_exception_3561: ;

  return UNIT;
}

unit zrlp_write_uint_word(sail_u256 zw)
{
  unit z8zE159;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3684;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3684);
  zminimal_word_bytes(&z2zE3684, zw);
  unit z3zE3518;
  {
    zz5listz8z5bv8z9 zbytes;
    CREATE(zz5listz8z5bv8z9)(&zbytes);
    COPY(zz5listz8z5bv8z9)(&zbytes, z2zE3684.ztup0);
    uint64_t zlen;
    zlen = z2zE3684.ztup1;
    z3zE3518 = zrlp_write_bytes(zbytes, zlen);
    KILL(zz5listz8z5bv8z9)(&zbytes);
    goto finish_match_3556;
  }
case_3557: ;
  sail_match_failure("rlp_write_uint_word");
finish_match_3556: ;
  z8zE159 = z3zE3518;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3684);
end_function_3558: ;
  return z8zE159;
end_block_exception_3559: ;

  return UNIT;
}

unit zrlp_write_uint_nat(uint64_t zn)
{
  unit z8zE160;
  sail_u256 z2zE3683;
  z2zE3683 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
  z8zE160 = zrlp_write_uint_word(z2zE3683);
end_function_3554: ;
  return z8zE160;
end_block_exception_3555: ;

  return UNIT;
}

unit zrlp_write_word(sail_u256 zw)
{
  unit z8zE161;
  zz5listz8z5bv8z9 z2zE3682;
  CREATE(zz5listz8z5bv8z9)(&z2zE3682);
  zword_to_bytes32(&z2zE3682, zw);
  z8zE161 = zrlp_write_bytes(z2zE3682, zWORD_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE3682);
end_function_3552: ;
  return z8zE161;
end_block_exception_3553: ;

  return UNIT;
}

unit zrlp_write_addr(sail_fixed_bytes_20 za)
{
  unit z8zE162;
  zz5listz8z5bv8z9 z2zE3681;
  CREATE(zz5listz8z5bv8z9)(&z2zE3681);
  zaddress_to_bytes(&z2zE3681, za);
  z8zE162 = zrlp_write_bytes(z2zE3681, zADDRESS_BYTE_LENGTH);
  KILL(zz5listz8z5bv8z9)(&z2zE3681);
end_function_3550: ;
  return z8zE162;
end_block_exception_3551: ;

  return UNIT;
}

struct zByteSliceFields zrlp_finish(uint64_t zstart)
{
  struct zByteSliceFields z8zE163;
  z8zE163 = zscratch_finish(zstart);
end_function_3548: ;
  return z8zE163;
end_block_exception_3549: ;
  struct zByteSliceFields z8zE835 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE835;
}

uint64_t zrlp_uint64_append(uint64_t z_width, uint64_t zprefix, uint64_t znext)
{
  uint64_t z8zE164;
  uint64_t z2zE3679;
  {    z2zE3679 = (zprefix * UINT64_C(256));
  }
  uint64_t z2zE3680;
  z2zE3680 = ((uint64_t) znext);
  {    z8zE164 = (z2zE3679 + z2zE3680);
  }
end_function_3546: ;
  return z8zE164;
end_block_exception_3547: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint64_width(struct zByteSliceFields zcontent, uint64_t zwidth)
{
  uint64_t z8zE165;
  bool z2zE3674;
  z2zE3674 = (zwidth == UINT64_C(0));
  if (z2zE3674) {  z8zE165 = UINT64_C(0);  } else {
    uint64_t z2zE3677;
    {
      uint64_t z2zE3675;
      {    z2zE3675 = (zwidth - UINT64_C(1));
      }
      z2zE3677 = zrlp_uint64_width(zcontent, z2zE3675);
    }
    uint64_t z2zE3678;
    {
      uint64_t z2zE3676;
      {    z2zE3676 = (zwidth - UINT64_C(1));
      }
      z2zE3678 = zslice_byte(zcontent, z2zE3676);
    }
    z8zE165 = zrlp_uint64_append(zwidth, z2zE3677, z2zE3678);
  }
end_function_3544: ;
  return z8zE165;
end_block_exception_3545: ;

  return UINT64_C(0xdeadc0de);
}

bool zrlp_bytes_equal_at(zz5listz8z5bv8z9 zexpected, struct zByteSliceFields zsource, uint64_t zstart)
{
  bool z8zE166;
  zz5listz8z5bv8z9 zrest;
  CREATE(zz5listz8z5bv8z9)(&zrest);
  COPY(zz5listz8z5bv8z9)(&zrest, zexpected);
  uint64_t zcurrent;
  zcurrent = zstart;
  bool zequal;
  zequal = true;
  int64_t z3zE3504;
  {    z3zE3504 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3505;
  {    z3zE3505 = (int64_t)(UINT64_C(7));
  }
  int64_t z3zE3506;
  {    z3zE3506 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3504;
    unit z3zE3513;
  for_start_3534: ;
    {
      if ((z3zE3505 < zbyte_index)) goto for_end_3535;
      unit z3zE3507;
      {
        if ((zrest == NULL)) goto case_3538;
        uint64_t zb;
        zb = (*zrest).hd;
        zz5listz8z5bv8z9 ztail;
        CREATE(zz5listz8z5bv8z9)(&ztail);
        COPY(zz5listz8z5bv8z9)(&ztail, (*zrest).tl);
        uint64_t zposition;
        zposition = zcurrent;
        uint64_t zsource_length;
        zsource_length = zsource.zlen;
        bool z2zE3672;
        z2zE3672 = (zposition < zsource_length);
        unit z3zE3509;
        if (z2zE3672) {
          bool z3zE3510;
          if (zequal) {
            uint64_t z2zE3673;
            z2zE3673 = zslice_byte(zsource, zcurrent);
            z3zE3510 = (z2zE3673 == zb);
          } else {  z3zE3510 = false;  }
          zequal = z3zE3510;
          unit z3zE3511;
          z3zE3511 = UNIT;
          {    zcurrent = (zposition + UINT64_C(1));
          }
          z3zE3509 = UNIT;
        } else {
          zequal = false;
          z3zE3509 = UNIT;
        }
        COPY(zz5listz8z5bv8z9)(&zrest, ztail);
        z3zE3507 = UNIT;
        KILL(zz5listz8z5bv8z9)(&ztail);
        goto finish_match_3536;
      }
    case_3538: ;
      {
        /* complete */
        z3zE3507 = UNIT;
        goto finish_match_3536;
      }
    case_3537: ;
    finish_match_3536: ;
      z3zE3513 = z3zE3507;
      zbyte_index = (zbyte_index + z3zE3506);
      goto for_start_3534;
    }
  for_end_3535: ;
  }
  unit z3zE3514;
  z3zE3514 = UNIT;
  bool z3zE3515;
  {
    if (!((zrest == NULL))) goto case_3541;
    z3zE3515 = zequal;
    goto finish_match_3539;
  }
case_3541: ;
  {
    z3zE3515 = false;
    goto finish_match_3539;
  }
case_3540: ;
finish_match_3539: ;
  z8zE166 = z3zE3515;
  KILL(zz5listz8z5bv8z9)(&zrest);
end_function_3542: ;
  return z8zE166;
end_block_exception_3543: ;

  return false;
}

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 zrlp_ref_hdr(struct zByteSliceFields zb)
{
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE167;
  uint64_t zsource_len;
  zsource_len = zb.zlen;
  bool z2zE3653;
  z2zE3653 = (zsource_len == UINT64_C(0));
  unit z3zE3494;
  if (z2zE3653) {
    struct zexception z2zE3654;
    CREATE(zexception)(&z2zE3654);
    zInvalidBlock(&z2zE3654, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3654);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:427.8-427.37");
    KILL(zexception)(&z2zE3654);
    goto end_block_exception_3533;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3654);
  } else {  z3zE3494 = UNIT;  }
  uint64_t zh;
  {
    uint64_t z2zE3671;
    z2zE3671 = zslice_byte(zb, UINT64_C(0));
    zh = ((uint64_t) z2zE3671);
  }
  bool z2zE3655;
  z2zE3655 = (zh < UINT64_C(128));
  if (z2zE3655) {
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3501;
    z3zE3501.ztup0 = false;
    z3zE3501.ztup1 = UINT64_C(0);
    z3zE3501.ztup2 = UINT64_C(1);
    z8zE167 = z3zE3501;
  } else {
    bool z2zE3656;
    z2zE3656 = (zh < UINT64_C(184));
    if (z2zE3656) {
      uint64_t z2zE3657;
      {    z2zE3657 = (zh - UINT64_C(128));
      }
      struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3500;
      z3zE3500.ztup0 = false;
      z3zE3500.ztup1 = UINT64_C(1);
      z3zE3500.ztup2 = z2zE3657;
      z8zE167 = z3zE3500;
    } else {
      bool z2zE3658;
      z2zE3658 = (zh < UINT64_C(192));
      if (z2zE3658) {
        uint64_t zlength_width;
        {    zlength_width = (zh - UINT64_C(183));
        }
        bool z2zE3660;
        {
          uint64_t z2zE3659;
          {    z2zE3659 = (zsource_len - UINT64_C(1));
          }
          z2zE3660 = (z2zE3659 < zlength_width);
        }
        unit z3zE3498;
        if (z2zE3660) {
          struct zexception z2zE3661;
          CREATE(zexception)(&z2zE3661);
          zInvalidBlock(&z2zE3661, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3661);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:439.20-439.49");
          KILL(zexception)(&z2zE3661);
          goto end_block_exception_3533;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3661);
        } else {  z3zE3498 = UNIT;  }
        struct zByteSliceFields zlength_bytes;
        zlength_bytes = zsub_slice(zb, UINT64_C(1), zlength_width);
        uint64_t z2zE3662;
        {    z2zE3662 = (UINT64_C(1) + zlength_width);
        }
        uint64_t z2zE3663;
        z2zE3663 = zrlp_uint64_width(zlength_bytes, zlength_width);
        struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3499;
        z3zE3499.ztup0 = false;
        z3zE3499.ztup1 = z2zE3662;
        z3zE3499.ztup2 = z2zE3663;
        z8zE167 = z3zE3499;
      } else {
        bool z2zE3664;
        z2zE3664 = (zh < UINT64_C(248));
        if (z2zE3664) {
          uint64_t z2zE3665;
          {    z2zE3665 = (zh - UINT64_C(192));
          }
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3497;
          z3zE3497.ztup0 = true;
          z3zE3497.ztup1 = UINT64_C(1);
          z3zE3497.ztup2 = z2zE3665;
          z8zE167 = z3zE3497;
        } else {
          uint64_t z3zE3502;
          {    z3zE3502 = (zh - UINT64_C(247));
          }
          bool z2zE3667;
          {
            uint64_t z2zE3666;
            {    z2zE3666 = (zsource_len - UINT64_C(1));
            }
            z2zE3667 = (z2zE3666 < z3zE3502);
          }
          unit z3zE3495;
          if (z2zE3667) {
            struct zexception z2zE3668;
            CREATE(zexception)(&z2zE3668);
            zInvalidBlock(&z2zE3668, zRlpDecode);
            COPY(zexception)(current_exception, z2zE3668);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:453.24-453.53");
            KILL(zexception)(&z2zE3668);
            goto end_block_exception_3533;
            /* unreachable after throw */
            KILL(zexception)(&z2zE3668);
          } else {  z3zE3495 = UNIT;  }
          struct zByteSliceFields z3zE3503;
          z3zE3503 = zsub_slice(zb, UINT64_C(1), z3zE3502);
          uint64_t z2zE3669;
          {    z2zE3669 = (UINT64_C(1) + z3zE3502);
          }
          uint64_t z2zE3670;
          z2zE3670 = zrlp_uint64_width(z3zE3503, z3zE3502);
          struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z3zE3496;
          z3zE3496.ztup0 = true;
          z3zE3496.ztup1 = z2zE3669;
          z3zE3496.ztup2 = z2zE3670;
          z8zE167 = z3zE3496;
        }
      }
    }
  }
end_function_3532: ;
  return z8zE167;
end_block_exception_3533: ;
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z8zE836 = { .ztup0 = false, .ztup1 = UINT64_C(0xdeadc0de), .ztup2 = UINT64_C(0xdeadc0de) };
  return z8zE836;
}

struct zRlpCursor zrlp_ref_cursor(struct zRlpFieldRef zf)
{
  struct zRlpCursor z8zE168;
  bool z2zE3645;
  z2zE3645 = zf.zis_list;
  if (z2zE3645) {
    struct zByteSliceFields zcontent;
    {
      struct zByteSliceFields z2zE3649;
      z2zE3649 = zf.zsource;
      uint64_t z2zE3650;
      {
        uint64_t z2zE3647;
        {
          struct zByteSliceFields z2zE3646;
          z2zE3646 = zf.zsource;
          z2zE3647 = z2zE3646.zlen;
        }
        uint64_t z2zE3648;
        z2zE3648 = zf.zcontent_len;
        {    z2zE3650 = (z2zE3647 - z2zE3648);
        }
      }
      uint64_t z2zE3651;
      z2zE3651 = zf.zcontent_len;
      zcontent = zsub_slice(z2zE3649, z2zE3650, z2zE3651);
    }
    struct zRlpCursor zcursor;
    {
      struct zRlpCursor z3zE3493;
      z3zE3493.zcurrent = UINT64_C(0);
      z3zE3493.zsource = zcontent;
      zcursor = z3zE3493;
    }
    z8zE168 = zcursor;
  } else {
    struct zexception z2zE3652;
    CREATE(zexception)(&z2zE3652);
    zInvalidBlock(&z2zE3652, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3652);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:493.8-493.37");
    KILL(zexception)(&z2zE3652);
    goto end_block_exception_3531;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3652);
  }
end_function_3530: ;
  return z8zE168;
end_block_exception_3531: ;
  struct zByteSliceFields z8zE838 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursor z8zE837 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE838 };
  return z8zE837;
}

bool zrlp_cursor_empty(struct zRlpCursor zcursor)
{
  bool z8zE169;
  uint64_t z2zE3643;
  z2zE3643 = zcursor.zcurrent;
  uint64_t z2zE3644;
  {
    struct zByteSliceFields z2zE3642;
    z2zE3642 = zcursor.zsource;
    z2zE3644 = z2zE3642.zlen;
  }
  z8zE169 = (z2zE3643 == z2zE3644);
end_function_3528: ;
  return z8zE169;
end_block_exception_3529: ;

  return false;
}

struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 zrlp_cursor_pop(struct zRlpCursor zcursor)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z8zE170;
  bool z2zE3622;
  {
    bool z2zE3621;
    {
      uint64_t z2zE3619;
      z2zE3619 = zcursor.zcurrent;
      uint64_t z2zE3620;
      {
        struct zByteSliceFields z2zE3618;
        z2zE3618 = zcursor.zsource;
        z2zE3620 = z2zE3618.zlen;
      }
      z2zE3621 = (z2zE3619 < z2zE3620);
    }
    z2zE3622 = not(z2zE3621);
  }
  unit z3zE3479;
  if (z2zE3622) {
    struct zexception z2zE3623;
    CREATE(zexception)(&z2zE3623);
    zInvalidBlock(&z2zE3623, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3623);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:531.8-531.37");
    KILL(zexception)(&z2zE3623);
    goto end_block_exception_3527;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3623);
  } else {  z3zE3479 = UNIT;  }
  struct zByteSliceFields zremaining;
  {
    struct zByteSliceFields z2zE3640;
    z2zE3640 = zcursor.zsource;
    uint64_t z2zE3641;
    z2zE3641 = zcursor.zcurrent;
    zremaining = zslice_suffix(z2zE3640, z2zE3641);
  }
  struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE3624;
  {
    z2zE3624 = zrlp_ref_hdr(zremaining);
    if (have_exception) {  goto end_block_exception_3527;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z3zE3480;
  {
    bool zis_list;
    zis_list = z2zE3624.ztup0;
    uint64_t zcontent_off;
    zcontent_off = z2zE3624.ztup1;
    uint64_t zcontent_len_value;
    zcontent_len_value = z2zE3624.ztup2;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z3zE3481;
    {
      uint64_t zcontent_len;
      zcontent_len = zcontent_len_value;
      bool z2zE3626;
      {
        uint64_t z2zE3625;
        z2zE3625 = zremaining.zlen;
        z2zE3626 = (z2zE3625 < zcontent_off);
      }
      unit z3zE3483;
      if (z2zE3626) {
        struct zexception z2zE3627;
        CREATE(zexception)(&z2zE3627);
        zInvalidBlock(&z2zE3627, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3627);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:537.8-537.37");
        KILL(zexception)(&z2zE3627);
        goto end_block_exception_3527;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3627);
      } else {  z3zE3483 = UNIT;  }
      bool z2zE3630;
      {
        uint64_t z2zE3629;
        {
          uint64_t z2zE3628;
          z2zE3628 = zremaining.zlen;
          {    z2zE3629 = (z2zE3628 - zcontent_off);
          }
        }
        z2zE3630 = (z2zE3629 < zcontent_len);
      }
      unit z3zE3482;
      if (z2zE3630) {
        struct zexception z2zE3631;
        CREATE(zexception)(&z2zE3631);
        zInvalidBlock(&z2zE3631, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3631);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:540.8-540.37");
        KILL(zexception)(&z2zE3631);
        goto end_block_exception_3527;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3631);
      } else {  z3zE3482 = UNIT;  }
      uint64_t z2zE3632;
      {    z2zE3632 = (zcontent_off + zcontent_len);
      }
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z3zE3484;
      {
        uint64_t zfull_len;
        zfull_len = z2zE3632;
        uint64_t z2zE3634;
        {
          uint64_t z2zE3633;
          z2zE3633 = zcursor.zcurrent;
          {    z2zE3634 = (z2zE3633 + zfull_len);
          }
        }
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z3zE3485;
        {
          uint64_t znext;
          znext = z2zE3634;
          bool z2zE3635;
          z2zE3635 = (UINT64_C(0) < zfull_len);
          if (z2zE3635) {
            struct zByteSliceFields zfield_source;
            {
              struct zByteSliceFields z2zE3637;
              z2zE3637 = zcursor.zsource;
              uint64_t z2zE3638;
              z2zE3638 = zcursor.zcurrent;
              zfield_source = zsub_slice(z2zE3637, z2zE3638, zfull_len);
            }
            struct zRlpFieldRef zfield;
            {
              struct zRlpFieldRef z3zE3487;
              z3zE3487.zcontent_len = zcontent_len;
              z3zE3487.zis_list = zis_list;
              z3zE3487.zsource = zfield_source;
              zfield = z3zE3487;
            }
            struct zRlpCursor zadvanced;
            {
              struct zByteSliceFields z2zE3636;
              z2zE3636 = zcursor.zsource;
              struct zRlpCursor z3zE3488;
              z3zE3488.zcurrent = znext;
              z3zE3488.zsource = z2zE3636;
              zadvanced = z3zE3488;
            }
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z3zE3489;
            z3zE3489.ztup0 = zfield;
            z3zE3489.ztup1 = zadvanced;
            z3zE3485 = z3zE3489;
          } else {
            struct zexception z2zE3639;
            CREATE(zexception)(&z2zE3639);
            zInvalidBlock(&z2zE3639, zRlpDecode);
            COPY(zexception)(current_exception, z2zE3639);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:557.8-557.37");
            KILL(zexception)(&z2zE3639);
            goto end_block_exception_3527;
            /* unreachable after throw */
            KILL(zexception)(&z2zE3639);
          }
          goto finish_match_3524;
        }
      case_3525: ;
        sail_match_failure("rlp_cursor_pop");
      finish_match_3524: ;
        z3zE3484 = z3zE3485;
        goto finish_match_3522;
      }
    case_3523: ;
      sail_match_failure("rlp_cursor_pop");
    finish_match_3522: ;
      z3zE3481 = z3zE3484;
      goto finish_match_3520;
    }
  case_3521: ;
    sail_match_failure("rlp_cursor_pop");
  finish_match_3520: ;
    z3zE3480 = z3zE3481;
    goto finish_match_3518;
  }
case_3519: ;
  sail_match_failure("rlp_cursor_pop");
finish_match_3518: ;
  z8zE170 = z3zE3480;
end_function_3526: ;
  return z8zE170;
end_block_exception_3527: ;
  struct zByteSliceFields z8zE843 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRef z8zE842 = { .zcontent_len = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE843 };
  struct zByteSliceFields z8zE841 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursor z8zE840 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE841 };
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z8zE839 = { .ztup0 = z8zE842, .ztup1 = z8zE840 };
  return z8zE839;
}

unit zrlp_cursor_expect_end(struct zRlpCursor zcursor)
{
  unit z8zE171;
  bool z2zE3616;
  z2zE3616 = zrlp_cursor_empty(zcursor);
  if (z2zE3616) {  z8zE171 = UNIT;  } else {
    struct zexception z2zE3617;
    CREATE(zexception)(&z2zE3617);
    zInvalidBlock(&z2zE3617, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3617);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:570.8-570.37");
    KILL(zexception)(&z2zE3617);
    goto end_block_exception_3517;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3617);
  }
end_function_3516: ;
  return z8zE171;
end_block_exception_3517: ;

  return UNIT;
}

struct zRlpFieldRef zrlp_single_ref(struct zByteSliceFields zitem)
{
  struct zRlpFieldRef z8zE172;
  uint64_t zitem_length;
  zitem_length = zitem.zlen;
  bool z2zE3608;
  z2zE3608 = (zitem_length == UINT64_C(0));
  if (z2zE3608) {
    struct zexception z2zE3609;
    CREATE(zexception)(&z2zE3609);
    zInvalidBlock(&z2zE3609, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3609);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:592.8-592.37");
    KILL(zexception)(&z2zE3609);
    goto end_block_exception_3515;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3609);
  } else {
    struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 z2zE3610;
    {
      z2zE3610 = zrlp_ref_hdr(zitem);
      if (have_exception) {  goto end_block_exception_3515;  }
    }
    struct zRlpFieldRef z3zE3472;
    {
      bool zis_list;
      zis_list = z2zE3610.ztup0;
      uint64_t zcontent_off;
      zcontent_off = z2zE3610.ztup1;
      uint64_t zcontent_len_value;
      zcontent_len_value = z2zE3610.ztup2;
      struct zRlpFieldRef z3zE3473;
      {
        uint64_t zcontent_len;
        zcontent_len = zcontent_len_value;
        bool z2zE3611;
        z2zE3611 = (zitem_length < zcontent_off);
        unit z3zE3475;
        if (z2zE3611) {
          struct zexception z2zE3612;
          CREATE(zexception)(&z2zE3612);
          zInvalidBlock(&z2zE3612, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3612);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:597.12-597.41");
          KILL(zexception)(&z2zE3612);
          goto end_block_exception_3515;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3612);
        } else {  z3zE3475 = UNIT;  }
        bool z2zE3614;
        {
          uint64_t z2zE3613;
          {    z2zE3613 = (zitem_length - zcontent_off);
          }
          z2zE3614 = (zcontent_len != z2zE3613);
        }
        unit z3zE3474;
        if (z2zE3614) {
          struct zexception z2zE3615;
          CREATE(zexception)(&z2zE3615);
          zInvalidBlock(&z2zE3615, zRlpDecode);
          COPY(zexception)(current_exception, z2zE3615);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:600.12-600.41");
          KILL(zexception)(&z2zE3615);
          goto end_block_exception_3515;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3615);
        } else {  z3zE3474 = UNIT;  }
        struct zRlpFieldRef zfield;
        {
          struct zRlpFieldRef z3zE3476;
          z3zE3476.zcontent_len = zcontent_len;
          z3zE3476.zis_list = zis_list;
          z3zE3476.zsource = zitem;
          zfield = z3zE3476;
        }
        z3zE3473 = zfield;
        goto finish_match_3512;
      }
    case_3513: ;
      sail_match_failure("rlp_single_ref");
    finish_match_3512: ;
      z3zE3472 = z3zE3473;
      goto finish_match_3510;
    }
  case_3511: ;
    sail_match_failure("rlp_single_ref");
  finish_match_3510: ;
    z8zE172 = z3zE3472;
  }
end_function_3514: ;
  return z8zE172;
end_block_exception_3515: ;
  struct zByteSliceFields z8zE845 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpFieldRef z8zE844 = { .zcontent_len = UINT64_C(0xdeadc0de), .zis_list = false, .zsource = z8zE845 };
  return z8zE844;
}

struct zRlpCursor zrlp_node_cursor(struct zByteSliceFields znode)
{
  struct zRlpCursor z8zE173;
  struct zRlpFieldRef z2zE3607;
  {
    z2zE3607 = zrlp_single_ref(znode);
    if (have_exception) {  goto end_block_exception_3509;  }
  }
  {
    z8zE173 = zrlp_ref_cursor(z2zE3607);
    if (have_exception) {  goto end_block_exception_3509;  }
  }
end_function_3508: ;
  return z8zE173;
end_block_exception_3509: ;
  struct zByteSliceFields z8zE847 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zRlpCursor z8zE846 = { .zcurrent = UINT64_C(0xdeadc0de), .zsource = z8zE847 };
  return z8zE846;
}

struct zByteSliceFields zrlp_ref_content(struct zRlpFieldRef zf)
{
  struct zByteSliceFields z8zE174;
  struct zByteSliceFields z2zE3604;
  z2zE3604 = zf.zsource;
  uint64_t z2zE3605;
  {
    uint64_t z2zE3602;
    {
      struct zByteSliceFields z2zE3601;
      z2zE3601 = zf.zsource;
      z2zE3602 = z2zE3601.zlen;
    }
    uint64_t z2zE3603;
    z2zE3603 = zf.zcontent_len;
    {    z2zE3605 = (z2zE3602 - z2zE3603);
    }
  }
  uint64_t z2zE3606;
  z2zE3606 = zf.zcontent_len;
  z8zE174 = zsub_slice(z2zE3604, z2zE3605, z2zE3606);
end_function_3506: ;
  return z8zE174;
end_block_exception_3507: ;
  struct zByteSliceFields z8zE848 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE848;
}

bool zrlp_ref_framing_canonical(struct zRlpFieldRef zf)
{
  bool z8zE175;
  uint64_t zn;
  zn = zf.zcontent_len;
  uint64_t zpayload_length;
  zpayload_length = zn;
  uint64_t zfull_length;
  {
    struct zByteSliceFields z2zE3600;
    z2zE3600 = zf.zsource;
    zfull_length = z2zE3600.zlen;
  }
  uint64_t zcontent_offset;
  {    zcontent_offset = (zfull_length - zpayload_length);
  }
  struct zByteSliceFields zsource;
  zsource = zf.zsource;
  uint64_t zsource_length;
  zsource_length = zsource.zlen;
  bool z2zE3549;
  z2zE3549 = (zfull_length == UINT64_C(0));
  if (z2zE3549) {  z8zE175 = false;  } else {
    bool z2zE3550;
    z2zE3550 = zf.zis_list;
    if (z2zE3550) {
      bool z2zE3551;
      z2zE3551 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
      if (z2zE3551) {
        bool z2zE3557;
        {
          uint64_t z2zE3552;
          {    z2zE3552 = (zpayload_length + UINT64_C(1));
          }
          z2zE3557 = (zfull_length == z2zE3552);
        }
        bool z3zE3469;
        if (z2zE3557) {
          uint64_t z2zE3555;
          {
            struct zByteSliceFields z2zE3553;
            z2zE3553 = zf.zsource;
            z2zE3555 = zslice_byte(z2zE3553, UINT64_C(0));
          }
          uint64_t z2zE3556;
          {
            uint64_t z2zE3554;
            z2zE3554 = zrlp_nat_length_byte(zn);
            z2zE3556 = ((UINT64_C(0xC0) + z2zE3554) & UINT64_C(0xFF));
          }
          z3zE3469 = (z2zE3555 == z2zE3556);
        } else {  z3zE3469 = false;  }
        z8zE175 = z3zE3469;
      } else {
        struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3559;
        CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3559);
        {
          sail_u256 z2zE3558;
          z2zE3558 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
          zminimal_word_bytes(&z2zE3559, z2zE3558);
        }
        bool z3zE3465;
        {
          zz5listz8z5bv8z9 zlen_bytes;
          CREATE(zz5listz8z5bv8z9)(&zlen_bytes);
          COPY(zz5listz8z5bv8z9)(&zlen_bytes, z2zE3559.ztup0);
          uint64_t zlength_width;
          zlength_width = z2zE3559.ztup1;
          bool z2zE3569;
          {
            sail_u128 z2zE3561;
            {
              uint64_t z2zE3560;
              {    z2zE3560 = (UINT64_C(1) + zlength_width);
              }
              z2zE3561 = u128_add_u64_u64(z2zE3560, zpayload_length);
            }
            z2zE3569 = u128_eq_u64(z2zE3561, zfull_length);
          }
          bool z3zE3467;
          if (z2zE3569) {
            bool z2zE3568;
            {
              uint64_t z2zE3564;
              {
                struct zByteSliceFields z2zE3562;
                z2zE3562 = zf.zsource;
                z2zE3564 = zslice_byte(z2zE3562, UINT64_C(0));
              }
              uint64_t z2zE3565;
              {
                uint64_t z2zE3563;
                z2zE3563 = zrlp_nat_length_byte(zlength_width);
                z2zE3565 = ((UINT64_C(0xF7) + z2zE3563) & UINT64_C(0xFF));
              }
              z2zE3568 = (z2zE3564 == z2zE3565);
            }
            bool z3zE3466;
            if (z2zE3568) {
              bool z2zE3566;
              z2zE3566 = (UINT64_C(0) < zsource_length);
              struct zByteSliceFields z2zE3567;
              z2zE3567 = zf.zsource;
              z3zE3466 = zrlp_bytes_equal_at(zlen_bytes, z2zE3567, UINT64_C(1));
            } else {  z3zE3466 = false;  }
            z3zE3467 = z3zE3466;
          } else {  z3zE3467 = false;  }
          z3zE3465 = z3zE3467;
          KILL(zz5listz8z5bv8z9)(&zlen_bytes);
          goto finish_match_3502;
        }
      case_3503: ;
        sail_match_failure("rlp_ref_framing_canonical");
      finish_match_3502: ;
        z8zE175 = z3zE3465;
        KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3559);
      }
    } else {
      bool z2zE3570;
      z2zE3570 = (zpayload_length == UINT64_C(0));
      if (z2zE3570) {
        bool z2zE3573;
        z2zE3573 = (zfull_length == UINT64_C(1));
        bool z3zE3464;
        if (z2zE3573) {
          uint64_t z2zE3572;
          {
            struct zByteSliceFields z2zE3571;
            z2zE3571 = zf.zsource;
            z2zE3572 = zslice_byte(z2zE3571, UINT64_C(0));
          }
          z3zE3464 = (z2zE3572 == UINT64_C(0x80));
        } else {  z3zE3464 = false;  }
        z8zE175 = z3zE3464;
      } else {
        uint64_t zfirst;
        {
          struct zByteSliceFields z2zE3599;
          z2zE3599 = zf.zsource;
          zfirst = zslice_byte(z2zE3599, zcontent_offset);
        }
        bool z2zE3576;
        {
          bool z2zE3575;
          z2zE3575 = (zpayload_length == UINT64_C(1));
          bool z3zE3457;
          if (z2zE3575) {
            uint64_t z2zE3574;
            z2zE3574 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
            z3zE3457 = (z2zE3574 == UINT64_C(0b0));
          } else {  z3zE3457 = false;  }
          z2zE3576 = z3zE3457;
        }
        if (z2zE3576) {
          bool z2zE3579;
          z2zE3579 = (zfull_length == UINT64_C(1));
          bool z3zE3463;
          if (z2zE3579) {
            uint64_t z2zE3578;
            {
              struct zByteSliceFields z2zE3577;
              z2zE3577 = zf.zsource;
              z2zE3578 = zslice_byte(z2zE3577, UINT64_C(0));
            }
            z3zE3463 = (z2zE3578 == zfirst);
          } else {  z3zE3463 = false;  }
          z8zE175 = z3zE3463;
        } else {
          bool z2zE3580;
          z2zE3580 = (!(zRLP_SHORT_LENGTH_LIMIT < zn));
          if (z2zE3580) {
            bool z2zE3586;
            {
              uint64_t z2zE3581;
              {    z2zE3581 = (zpayload_length + UINT64_C(1));
              }
              z2zE3586 = (zfull_length == z2zE3581);
            }
            bool z3zE3462;
            if (z2zE3586) {
              uint64_t z2zE3584;
              {
                struct zByteSliceFields z2zE3582;
                z2zE3582 = zf.zsource;
                z2zE3584 = zslice_byte(z2zE3582, UINT64_C(0));
              }
              uint64_t z2zE3585;
              {
                uint64_t z2zE3583;
                z2zE3583 = zrlp_nat_length_byte(zn);
                z2zE3585 = ((UINT64_C(0x80) + z2zE3583) & UINT64_C(0xFF));
              }
              z3zE3462 = (z2zE3584 == z2zE3585);
            } else {  z3zE3462 = false;  }
            z8zE175 = z3zE3462;
          } else {
            struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3588;
            CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3588);
            {
              sail_u256 z2zE3587;
              z2zE3587 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zn);
              zminimal_word_bytes(&z2zE3588, z2zE3587);
            }
            bool z3zE3458;
            {
              zz5listz8z5bv8z9 z3zE3470;
              CREATE(zz5listz8z5bv8z9)(&z3zE3470);
              COPY(zz5listz8z5bv8z9)(&z3zE3470, z2zE3588.ztup0);
              uint64_t z3zE3471;
              z3zE3471 = z2zE3588.ztup1;
              bool z2zE3598;
              {
                sail_u128 z2zE3590;
                {
                  uint64_t z2zE3589;
                  {    z2zE3589 = (UINT64_C(1) + z3zE3471);
                  }
                  z2zE3590 = u128_add_u64_u64(z2zE3589, zpayload_length);
                }
                z2zE3598 = u128_eq_u64(z2zE3590, zfull_length);
              }
              bool z3zE3460;
              if (z2zE3598) {
                bool z2zE3597;
                {
                  uint64_t z2zE3593;
                  {
                    struct zByteSliceFields z2zE3591;
                    z2zE3591 = zf.zsource;
                    z2zE3593 = zslice_byte(z2zE3591, UINT64_C(0));
                  }
                  uint64_t z2zE3594;
                  {
                    uint64_t z2zE3592;
                    z2zE3592 = zrlp_nat_length_byte(z3zE3471);
                    z2zE3594 = ((UINT64_C(0xB7) + z2zE3592) & UINT64_C(0xFF));
                  }
                  z2zE3597 = (z2zE3593 == z2zE3594);
                }
                bool z3zE3459;
                if (z2zE3597) {
                  bool z2zE3595;
                  z2zE3595 = (UINT64_C(0) < zsource_length);
                  struct zByteSliceFields z2zE3596;
                  z2zE3596 = zf.zsource;
                  z3zE3459 = zrlp_bytes_equal_at(z3zE3470, z2zE3596, UINT64_C(1));
                } else {  z3zE3459 = false;  }
                z3zE3460 = z3zE3459;
              } else {  z3zE3460 = false;  }
              z3zE3458 = z3zE3460;
              KILL(zz5listz8z5bv8z9)(&z3zE3470);
              goto finish_match_3500;
            }
          case_3501: ;
            sail_match_failure("rlp_ref_framing_canonical");
          finish_match_3500: ;
            z8zE175 = z3zE3458;
            KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3588);
          }
        }
      }
    }
  }
end_function_3504: ;
  return z8zE175;
end_block_exception_3505: ;

  return false;
}

bool zrlp_ref_bytes_canonical(struct zRlpFieldRef zf)
{
  bool z8zE176;
  bool z2zE3548;
  {
    bool z2zE3547;
    z2zE3547 = zf.zis_list;
    z2zE3548 = not(z2zE3547);
  }
  bool z3zE3456;
  if (z2zE3548) {  z3zE3456 = zrlp_ref_framing_canonical(zf);  } else {  z3zE3456 = false;  }
  z8zE176 = z3zE3456;
end_function_3498: ;
  return z8zE176;
end_block_exception_3499: ;

  return false;
}

bool zrlp_ref_uint_canonical(struct zRlpFieldRef zf)
{
  bool z8zE177;
  bool z2zE3538;
  {
    bool z2zE3537;
    z2zE3537 = zrlp_ref_bytes_canonical(zf);
    z2zE3538 = not(z2zE3537);
  }
  if (z2zE3538) {  z8zE177 = false;  } else {
    bool z2zE3546;
    {
      uint64_t z2zE3539;
      z2zE3539 = zf.zcontent_len;
      z2zE3546 = (z2zE3539 == UINT64_C(0));
    }
    bool z3zE3455;
    if (z2zE3546) {  z3zE3455 = true;  } else {
      uint64_t z2zE3545;
      {
        struct zByteSliceFields z2zE3543;
        z2zE3543 = zf.zsource;
        uint64_t z2zE3544;
        {
          uint64_t z2zE3541;
          {
            struct zByteSliceFields z2zE3540;
            z2zE3540 = zf.zsource;
            z2zE3541 = z2zE3540.zlen;
          }
          uint64_t z2zE3542;
          z2zE3542 = zf.zcontent_len;
          {    z2zE3544 = (z2zE3541 - z2zE3542);
          }
        }
        z2zE3545 = zslice_byte(z2zE3543, z2zE3544);
      }
      z3zE3455 = (z2zE3545 != UINT64_C(0x00));
    }
    z8zE177 = z3zE3455;
  }
end_function_3496: ;
  return z8zE177;
end_block_exception_3497: ;

  return false;
}

sail_u256 zrlp_ref_word(struct zRlpFieldRef zf)
{
  sail_u256 z8zE178;
  uint64_t zn;
  zn = zf.zcontent_len;
  bool z2zE3531;
  {
    bool z2zE3530;
    z2zE3530 = zf.zis_list;
    bool z3zE3454;
    if (z2zE3530) {  z3zE3454 = true;  } else {  z3zE3454 = (zRLP_WORD_LENGTH_LIMIT < zn);  }
    z2zE3531 = z3zE3454;
  }
  if (z2zE3531) {
    struct zexception z2zE3532;
    CREATE(zexception)(&z2zE3532);
    zInvalidBlock(&z2zE3532, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3532);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:708.8-708.37");
    KILL(zexception)(&z2zE3532);
    goto end_block_exception_3495;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3532);
  } else {
    struct zByteSliceFields z2zE3535;
    z2zE3535 = zf.zsource;
    uint64_t z2zE3536;
    {
      uint64_t z2zE3534;
      {
        struct zByteSliceFields z2zE3533;
        z2zE3533 = zf.zsource;
        z2zE3534 = z2zE3533.zlen;
      }
      {    z2zE3536 = (z2zE3534 - zn);
      }
    }
    z8zE178 = zslice_load_n(z2zE3535, z2zE3536, zn);
  }
end_function_3494: ;
  return z8zE178;
end_block_exception_3495: ;

  return u256_zero();
}

sail_u256 zrlp_ref_uint_word(struct zRlpFieldRef zf)
{
  sail_u256 z8zE179;
  bool z2zE3528;
  z2zE3528 = zrlp_ref_uint_canonical(zf);
  if (z2zE3528) {
    {
      z8zE179 = zrlp_ref_word(zf);
      if (have_exception) {  goto end_block_exception_3493;  }
    }
  } else {
    struct zexception z2zE3529;
    CREATE(zexception)(&z2zE3529);
    zInvalidBlock(&z2zE3529, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3529);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:723.8-723.37");
    KILL(zexception)(&z2zE3529);
    goto end_block_exception_3493;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3529);
  }
end_function_3492: ;
  return z8zE179;
end_block_exception_3493: ;

  return u256_zero();
}

uint64_t zrlp_ref_uint64(struct zRlpFieldRef zf)
{
  uint64_t z8zE180;
  bool z2zE3526;
  {
    bool z2zE3525;
    {
      bool z2zE3523;
      z2zE3523 = zrlp_ref_uint_canonical(zf);
      z2zE3525 = not(z2zE3523);
    }
    bool z3zE3452;
    if (z2zE3525) {  z3zE3452 = true;  } else {
      uint64_t z2zE3524;
      z2zE3524 = zf.zcontent_len;
      z3zE3452 = (zRLP_UINT64_LENGTH_LIMIT < z2zE3524);
    }
    z2zE3526 = z3zE3452;
  }
  unit z3zE3453;
  if (z2zE3526) {
    struct zexception z2zE3527;
    CREATE(zexception)(&z2zE3527);
    zInvalidBlock(&z2zE3527, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3527);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/rlp.sail:733.8-733.37");
    KILL(zexception)(&z2zE3527);
    goto end_block_exception_3491;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3527);
  } else {  z3zE3453 = UNIT;  }
  uint64_t zcontent_length;
  zcontent_length = zf.zcontent_len;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zf);
  uint64_t zwidth;
  {    zwidth = (zcontent_length % UINT64_C(9));
  }
  z8zE180 = zrlp_uint64_width(zcontent, zwidth);
end_function_3490: ;
  return z8zE180;
end_block_exception_3491: ;

  return UINT64_C(0xdeadc0de);
}

