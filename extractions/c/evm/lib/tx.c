/* Generated from sail/lib/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_u256 zlegacy_sig_chain_id(sail_u256 zv)
{
  sail_u256 z8zE183;
  sail_u256 z2zE3510;
  z2zE3510 = zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zv, UINT64_C(35));
  z8zE183 = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3510, UINT64_C(2));
end_function_3484: ;
  return z8zE183;
end_block_exception_3485: ;

  return u256_zero();
}

void create_letbind_90(void) {

  uint64_t z3zE115;
  z3zE115 = UINT64_C(2);
  zLEGACY_SIGNATURE_SUFFIX_LENGTH = z3zE115;
let_end_287: ;
}
void kill_letbind_90(void) {
}

void create_letbind_91(void) {

  uint64_t z3zE116;
  z3zE116 = zDOUBLE_WORD_BYTE_LENGTH;
  zPUBLIC_KEY_BODY_LENGTH = z3zE116;
let_end_288: ;
}
void kill_letbind_91(void) {
}

sail_fixed_bytes_32 ztx_signing_hash(enum zTxType zt, struct zByteSliceFields zcontent_src, sail_u256 zv)
{
  sail_fixed_bytes_32 z8zE184;
  uint64_t ztb;
  ztb = ztx_type_byte(zt);
  bool zeip155;
  {
    bool z2zE3509;
    z2zE3509 = (ztb == UINT64_C(0x00));
    bool z3zE3434;
    if (z2zE3509) {
      z3zE3434 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
    } else {  z3zE3434 = false;  }
    zeip155 = z3zE3434;
  }
  sail_u256 zchain_id;
  if (zeip155) {  zchain_id = zlegacy_sig_chain_id(zv);  } else {  zchain_id = zZERO_WORD;  }
  uint64_t zsuffix_len;
  if (zeip155) {
    uint64_t zchain_id_length;
    zchain_id_length = zrlp_uint_word_sizze(zchain_id);
    uint64_t zsuffix_length;
    zsuffix_length = zLEGACY_SIGNATURE_SUFFIX_LENGTH;
    {    zsuffix_len = (zchain_id_length + zsuffix_length);
    }
  } else {  zsuffix_len = UINT64_C(0);  }
  uint64_t zcontent_length;
  {
    bool z2zE3507;
    {
      uint64_t z2zE3506;
      z2zE3506 = zcontent_src.zlen;
      z2zE3507 = (!(UINT64_C(1073741824) < z2zE3506));
    }
    if (z2zE3507) {  zcontent_length = zcontent_src.zlen;  } else {
      struct zexception z2zE3508;
      CREATE(zexception)(&z2zE3508);
      zInvalidBlock(&z2zE3508, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3508);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/tx.sail:46.12-46.41");
      KILL(zexception)(&z2zE3508);
      goto end_block_exception_3483;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3508);
    }
  }
  uint64_t z3zE3445;
  z3zE3445 = zsuffix_len;
  uint64_t zcontent_len;
  {    zcontent_len = (zcontent_length + z3zE3445);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3493;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3493);
  zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE3493, zcontent_len);
  sail_fixed_bytes_32 z3zE3435;
  {
    zz5listz8z5bv8z9 zprefix;
    CREATE(zz5listz8z5bv8z9)(&zprefix);
    COPY(zz5listz8z5bv8z9)(&zprefix, z2zE3493.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE3493.ztup1;
    if (zeip155) {
      uint64_t zmark;
      zmark = zscratch_begin(UNIT);
      unit z3zE3442;
      z3zE3442 = zrlp_write_uint_word(zchain_id);
      zz5listz8z5bv8z9 z3zE3440;
      CREATE(zz5listz8z5bv8z9)(&z3zE3440);
      zconsz3z5bv8(&z3zE3440, UINT64_C(0x80), z3zE3440);
      zconsz3z5bv8(&z3zE3440, UINT64_C(0x80), z3zE3440);
      unit z3zE3441;
      z3zE3441 = zscratch_push_bytes(z3zE3440, zLEGACY_SIGNATURE_SUFFIX_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z3zE3440);
      struct zByteSliceFields zsuffix;
      zsuffix = zrlp_finish(zmark);
      sail_fixed_bytes_32 zsigning_hash;
      {
        zz5listz8z5unionz0zzBytesz9 z2zE3497;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3497);
        {
          struct zBytes z2zE3494;
          CREATE(zBytes)(&z2zE3494);
          zbytes_list(&z2zE3494, zprefix, zprefix_len);
          struct zBytes z2zE3495;
          CREATE(zBytes)(&z2zE3495);
          zBytesSlice(&z2zE3495, zcontent_src);
          struct zBytes z2zE3496;
          CREATE(zBytes)(&z2zE3496);
          zBytesSlice(&z2zE3496, zsuffix);
          zconsz3z5unionz0zzBytes(&z2zE3497, z2zE3496, z2zE3497);
          zconsz3z5unionz0zzBytes(&z2zE3497, z2zE3495, z2zE3497);
          zconsz3z5unionz0zzBytes(&z2zE3497, z2zE3494, z2zE3497);
          KILL(zBytes)(&z2zE3496);
          KILL(zBytes)(&z2zE3495);
          KILL(zBytes)(&z2zE3494);
        }
        zsigning_hash = host_keccak_segments(z2zE3497);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3497);
      }
      unit z3zE3444;
      z3zE3444 = zscratch_rewind(zmark);
      z3zE3435 = zsigning_hash;
    } else {
      bool z2zE3498;
      z2zE3498 = (ztb != UINT64_C(0x00));
      if (z2zE3498) {
        zz5listz8z5unionz0zzBytesz9 z2zE3502;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3502);
        {
          struct zBytes z2zE3499;
          CREATE(zBytes)(&z2zE3499);
          {
            zz5listz8z5bv8z9 z3zE3438;
            CREATE(zz5listz8z5bv8z9)(&z3zE3438);
            zconsz3z5bv8(&z3zE3438, ztb, z3zE3438);
            zbytes_list(&z2zE3499, z3zE3438, UINT64_C(1));
            KILL(zz5listz8z5bv8z9)(&z3zE3438);
          }
          struct zBytes z2zE3500;
          CREATE(zBytes)(&z2zE3500);
          zbytes_list(&z2zE3500, zprefix, zprefix_len);
          struct zBytes z2zE3501;
          CREATE(zBytes)(&z2zE3501);
          zBytesSlice(&z2zE3501, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE3502, z2zE3501, z2zE3502);
          zconsz3z5unionz0zzBytes(&z2zE3502, z2zE3500, z2zE3502);
          zconsz3z5unionz0zzBytes(&z2zE3502, z2zE3499, z2zE3502);
          KILL(zBytes)(&z2zE3501);
          KILL(zBytes)(&z2zE3500);
          KILL(zBytes)(&z2zE3499);
        }
        z3zE3435 = host_keccak_segments(z2zE3502);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3502);
      } else {
        zz5listz8z5unionz0zzBytesz9 z2zE3505;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3505);
        {
          struct zBytes z2zE3503;
          CREATE(zBytes)(&z2zE3503);
          zbytes_list(&z2zE3503, zprefix, zprefix_len);
          struct zBytes z2zE3504;
          CREATE(zBytes)(&z2zE3504);
          zBytesSlice(&z2zE3504, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE3505, z2zE3504, z2zE3505);
          zconsz3z5unionz0zzBytes(&z2zE3505, z2zE3503, z2zE3505);
          KILL(zBytes)(&z2zE3504);
          KILL(zBytes)(&z2zE3503);
        }
        z3zE3435 = host_keccak_segments(z2zE3505);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3505);
      }
    }
    KILL(zz5listz8z5bv8z9)(&zprefix);
    goto finish_match_3480;
  }
case_3481: ;
  sail_match_failure("tx_signing_hash");
finish_match_3480: ;
  z8zE184 = z3zE3435;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3493);
end_function_3482: ;
  return z8zE184;
end_block_exception_3483: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zauth_signing_hash(sail_u256 zchain_id, sail_fixed_bytes_20 zaddr, uint64_t znonce)
{
  sail_fixed_bytes_32 z8zE185;
  uint64_t zchain_id_length;
  zchain_id_length = zrlp_uint_word_sizze(zchain_id);
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {
    uint64_t z2zE3492;
    {    z2zE3492 = (zchain_id_length + UINT64_C(21));
    }
    {    zcontent_len = (z2zE3492 + znonce_length);
    }
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  zz5listz8z5bv8z9 z3zE3427;
  CREATE(zz5listz8z5bv8z9)(&z3zE3427);
  zconsz3z5bv8(&z3zE3427, UINT64_C(0x05), z3zE3427);
  unit z3zE3432;
  z3zE3432 = zscratch_push_bytes(z3zE3427, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE3427);
  unit z3zE3431;
  z3zE3431 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE3430;
  z3zE3430 = zrlp_write_uint_word(zchain_id);
  unit z3zE3429;
  z3zE3429 = zrlp_write_addr(zaddr);
  unit z3zE3428;
  z3zE3428 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_32 zsigning_hash;
  zsigning_hash = zkeccak256_slice(zencoded);
  unit z3zE3433;
  z3zE3433 = zscratch_rewind(zmark);
  z8zE185 = zsigning_hash;
end_function_3478: ;
  return z8zE185;
end_block_exception_3479: ;

  return fixed_bytes_32_zero();
}

bool ztx_sig_v_valid(uint64_t zchain_id, enum zTxType zt, sail_u256 zv)
{
  bool z8zE186;
  bool z3zE3420;
  {
    if ((zLegacyTx != zt)) goto case_3475;
    bool z2zE3490;
    z2zE3490 = u256_eq_u64(zv, UINT64_C(27));
    bool z3zE3425;
    if (z2zE3490) {  z3zE3425 = true;  } else {
      bool z2zE3489;
      z2zE3489 = u256_eq_u64(zv, UINT64_C(28));
      bool z3zE3424;
      if (z2zE3489) {  z3zE3424 = true;  } else {
        bool z2zE3488;
        z2zE3488 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
        bool z3zE3423;
        if (z2zE3488) {
          sail_u256 z2zE3486;
          z2zE3486 = zlegacy_sig_chain_id(zv);
          uint64_t z2zE3487;
          z2zE3487 = zword_of_chain_identifier(zchain_id);
          z3zE3423 = u256_eq_u64(z2zE3486, z2zE3487);
        } else {  z3zE3423 = false;  }
        z3zE3424 = z3zE3423;
      }
      z3zE3425 = z3zE3424;
    }
    z3zE3420 = z3zE3425;
    goto finish_match_3473;
  }
case_3475: ;
  {
    bool z2zE3491;
    z2zE3491 = eq_u256(zv, zWORD_ZERO);
    bool z3zE3421;
    if (z2zE3491) {  z3zE3421 = true;  } else {  z3zE3421 = eq_u256(zv, zWORD_ONE);  }
    z3zE3420 = z3zE3421;
    goto finish_match_3473;
  }
case_3474: ;
finish_match_3473: ;
  z8zE186 = z3zE3420;
end_function_3476: ;
  return z8zE186;
end_block_exception_3477: ;

  return false;
}

uint64_t ztx_y_parity(enum zTxType zt, sail_u256 zv)
{
  uint64_t z8zE187;
  uint64_t z3zE3417;
  {
    if ((zLegacyTx != zt)) goto case_3470;
    bool z2zE3484;
    {
      sail_u256 z2zE3483;
      z2zE3483 = zword_and(zv, zWORD_ONE);
      z2zE3484 = eq_u256(z2zE3483, zWORD_ONE);
    }
    if (z2zE3484) {  z3zE3417 = UINT64_C(0);  } else {  z3zE3417 = UINT64_C(1);  }
    goto finish_match_3468;
  }
case_3470: ;
  {
    bool z2zE3485;
    z2zE3485 = eq_u256(zv, zWORD_ZERO);
    if (z2zE3485) {  z3zE3417 = UINT64_C(0);  } else {  z3zE3417 = UINT64_C(1);  }
    goto finish_match_3468;
  }
case_3469: ;
finish_match_3468: ;
  z8zE187 = z3zE3417;
end_function_3471: ;
  return z8zE187;
end_block_exception_3472: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_auth_valid(sail_fixed_bytes_20 zsender, sail_fixed_bytes_32 zh, uint64_t zparity, sail_u256 zr, sail_u256 zs)
{
  bool z8zE188;
  bool z2zE3481;
  z2zE3481 = zword_ult(zSECP_N_HALF, zs);
  if (z2zE3481) {  z8zE188 = false;  } else {
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3482;
    z2zE3482 = zecrecover_addr(zh, zparity, zr, zs);
    bool z3zE3414;
    {
      bool zrecovered;
      zrecovered = z2zE3482.ztup0;
      sail_fixed_bytes_20 zrecovered_sender;
      zrecovered_sender = z2zE3482.ztup1;
      bool z3zE3415;
      if (zrecovered) {  z3zE3415 = eq_fixed_bytes_20(zrecovered_sender, zsender);  } else {  z3zE3415 = false;  }
      z3zE3414 = z3zE3415;
      goto finish_match_3464;
    }
  case_3465: ;
    sail_match_failure("tx_auth_valid");
  finish_match_3464: ;
    z8zE188 = z3zE3414;
  }
end_function_3466: ;
  return z8zE188;
end_block_exception_3467: ;

  return false;
}

