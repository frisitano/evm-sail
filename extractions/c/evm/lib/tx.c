/* Generated from sail/lib/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_u256 zlegacy_sig_chain_id(sail_u256 zv)
{
  sail_u256 z8zE192;
  sail_u256 z2zE3534;
  z2zE3534 = zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zv, UINT64_C(35));
  z8zE192 = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE3534, UINT64_C(2));
end_function_3479: ;
  return z8zE192;
end_block_exception_3480: ;

  return u256_zero();
}

void create_letbind_90(void) {

  uint64_t z3zE115;
  z3zE115 = UINT64_C(2);
  zLEGACY_SIGNATURE_SUFFIX_LENGTH = z3zE115;
let_end_296: ;
}
void kill_letbind_90(void) {
}

void create_letbind_91(void) {

  uint64_t z3zE116;
  z3zE116 = zDOUBLE_WORD_BYTE_LENGTH;
  zPUBLIC_KEY_BODY_LENGTH = z3zE116;
let_end_297: ;
}
void kill_letbind_91(void) {
}

sail_fixed_bytes_32 ztx_signing_hash(enum zTxType zt, struct zByteSliceFields zcontent_src, sail_u256 zv)
{
  sail_fixed_bytes_32 z8zE193;
  uint64_t ztb;
  ztb = ztx_type_byte(zt);
  bool zeip155;
  {
    bool z2zE3533;
    z2zE3533 = (ztb == UINT64_C(0x00));
    bool z3zE3496;
    if (z2zE3533) {
      z3zE3496 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
    } else {  z3zE3496 = false;  }
    zeip155 = z3zE3496;
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
    bool z2zE3531;
    {
      uint64_t z2zE3530;
      z2zE3530 = zcontent_src.zlen;
      z2zE3531 = (!(UINT64_C(1073741824) < z2zE3530));
    }
    if (z2zE3531) {  zcontent_length = zcontent_src.zlen;  } else {
      struct zexception z2zE3532;
      CREATE(zexception)(&z2zE3532);
      zInvalidBlock(&z2zE3532, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3532);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/tx.sail:46.12-46.41");
      KILL(zexception)(&z2zE3532);
      goto end_block_exception_3478;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3532);
    }
  }
  uint64_t z3zE3507;
  z3zE3507 = zsuffix_len;
  uint64_t zcontent_len;
  {    zcontent_len = (zcontent_length + z3zE3507);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3517;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3517);
  zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE3517, zcontent_len);
  sail_fixed_bytes_32 z3zE3497;
  {
    zz5listz8z5bv8z9 zprefix;
    CREATE(zz5listz8z5bv8z9)(&zprefix);
    COPY(zz5listz8z5bv8z9)(&zprefix, z2zE3517.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE3517.ztup1;
    if (zeip155) {
      uint64_t zmark;
      zmark = zscratch_begin(UNIT);
      unit z3zE3504;
      z3zE3504 = zrlp_write_uint_word(zchain_id);
      zz5listz8z5bv8z9 z3zE3502;
      CREATE(zz5listz8z5bv8z9)(&z3zE3502);
      zconsz3z5bv8(&z3zE3502, UINT64_C(0x80), z3zE3502);
      zconsz3z5bv8(&z3zE3502, UINT64_C(0x80), z3zE3502);
      unit z3zE3503;
      z3zE3503 = zscratch_push_bytes(z3zE3502, zLEGACY_SIGNATURE_SUFFIX_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z3zE3502);
      struct zByteSliceFields zsuffix;
      zsuffix = zrlp_finish(zmark);
      sail_fixed_bytes_32 zsigning_hash;
      {
        zz5listz8z5unionz0zzBytesz9 z2zE3521;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3521);
        {
          struct zBytes z2zE3518;
          CREATE(zBytes)(&z2zE3518);
          zbytes_list(&z2zE3518, zprefix, zprefix_len);
          struct zBytes z2zE3519;
          CREATE(zBytes)(&z2zE3519);
          zBytesSlice(&z2zE3519, zcontent_src);
          struct zBytes z2zE3520;
          CREATE(zBytes)(&z2zE3520);
          zBytesSlice(&z2zE3520, zsuffix);
          zconsz3z5unionz0zzBytes(&z2zE3521, z2zE3520, z2zE3521);
          zconsz3z5unionz0zzBytes(&z2zE3521, z2zE3519, z2zE3521);
          zconsz3z5unionz0zzBytes(&z2zE3521, z2zE3518, z2zE3521);
          KILL(zBytes)(&z2zE3520);
          KILL(zBytes)(&z2zE3519);
          KILL(zBytes)(&z2zE3518);
        }
        zsigning_hash = host_keccak_segments(z2zE3521);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3521);
      }
      unit z3zE3506;
      z3zE3506 = zscratch_rewind(zmark);
      z3zE3497 = zsigning_hash;
    } else {
      bool z2zE3522;
      z2zE3522 = (ztb != UINT64_C(0x00));
      if (z2zE3522) {
        zz5listz8z5unionz0zzBytesz9 z2zE3526;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3526);
        {
          struct zBytes z2zE3523;
          CREATE(zBytes)(&z2zE3523);
          {
            zz5listz8z5bv8z9 z3zE3500;
            CREATE(zz5listz8z5bv8z9)(&z3zE3500);
            zconsz3z5bv8(&z3zE3500, ztb, z3zE3500);
            zbytes_list(&z2zE3523, z3zE3500, UINT64_C(1));
            KILL(zz5listz8z5bv8z9)(&z3zE3500);
          }
          struct zBytes z2zE3524;
          CREATE(zBytes)(&z2zE3524);
          zbytes_list(&z2zE3524, zprefix, zprefix_len);
          struct zBytes z2zE3525;
          CREATE(zBytes)(&z2zE3525);
          zBytesSlice(&z2zE3525, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE3526, z2zE3525, z2zE3526);
          zconsz3z5unionz0zzBytes(&z2zE3526, z2zE3524, z2zE3526);
          zconsz3z5unionz0zzBytes(&z2zE3526, z2zE3523, z2zE3526);
          KILL(zBytes)(&z2zE3525);
          KILL(zBytes)(&z2zE3524);
          KILL(zBytes)(&z2zE3523);
        }
        z3zE3497 = host_keccak_segments(z2zE3526);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3526);
      } else {
        zz5listz8z5unionz0zzBytesz9 z2zE3529;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3529);
        {
          struct zBytes z2zE3527;
          CREATE(zBytes)(&z2zE3527);
          zbytes_list(&z2zE3527, zprefix, zprefix_len);
          struct zBytes z2zE3528;
          CREATE(zBytes)(&z2zE3528);
          zBytesSlice(&z2zE3528, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE3529, z2zE3528, z2zE3529);
          zconsz3z5unionz0zzBytes(&z2zE3529, z2zE3527, z2zE3529);
          KILL(zBytes)(&z2zE3528);
          KILL(zBytes)(&z2zE3527);
        }
        z3zE3497 = host_keccak_segments(z2zE3529);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3529);
      }
    }
    KILL(zz5listz8z5bv8z9)(&zprefix);
    goto finish_match_3475;
  }
case_3476: ;
  sail_match_failure("tx_signing_hash");
finish_match_3475: ;
  z8zE193 = z3zE3497;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3517);
end_function_3477: ;
  return z8zE193;
end_block_exception_3478: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zauth_signing_hash(sail_u256 zchain_id, sail_fixed_bytes_20 zaddr, uint64_t znonce)
{
  sail_fixed_bytes_32 z8zE194;
  uint64_t zchain_id_length;
  zchain_id_length = zrlp_uint_word_sizze(zchain_id);
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {
    uint64_t z2zE3516;
    {    z2zE3516 = (zchain_id_length + UINT64_C(21));
    }
    {    zcontent_len = (z2zE3516 + znonce_length);
    }
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  zz5listz8z5bv8z9 z3zE3489;
  CREATE(zz5listz8z5bv8z9)(&z3zE3489);
  zconsz3z5bv8(&z3zE3489, UINT64_C(0x05), z3zE3489);
  unit z3zE3494;
  z3zE3494 = zscratch_push_bytes(z3zE3489, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE3489);
  unit z3zE3493;
  z3zE3493 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE3492;
  z3zE3492 = zrlp_write_uint_word(zchain_id);
  unit z3zE3491;
  z3zE3491 = zrlp_write_addr(zaddr);
  unit z3zE3490;
  z3zE3490 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_32 zsigning_hash;
  zsigning_hash = zkeccak256_slice(zencoded);
  unit z3zE3495;
  z3zE3495 = zscratch_rewind(zmark);
  z8zE194 = zsigning_hash;
end_function_3473: ;
  return z8zE194;
end_block_exception_3474: ;

  return fixed_bytes_32_zero();
}

bool ztx_sig_v_valid(uint64_t zchain_id, enum zTxType zt, sail_u256 zv)
{
  bool z8zE195;
  bool z3zE3482;
  {
    if ((zLegacyTx != zt)) goto case_3470;
    bool z2zE3514;
    z2zE3514 = u256_eq_u64(zv, UINT64_C(27));
    bool z3zE3487;
    if (z2zE3514) {  z3zE3487 = true;  } else {
      bool z2zE3513;
      z2zE3513 = u256_eq_u64(zv, UINT64_C(28));
      bool z3zE3486;
      if (z2zE3513) {  z3zE3486 = true;  } else {
        bool z2zE3512;
        z2zE3512 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
        bool z3zE3485;
        if (z2zE3512) {
          sail_u256 z2zE3510;
          z2zE3510 = zlegacy_sig_chain_id(zv);
          uint64_t z2zE3511;
          z2zE3511 = zword_of_chain_identifier(zchain_id);
          z3zE3485 = u256_eq_u64(z2zE3510, z2zE3511);
        } else {  z3zE3485 = false;  }
        z3zE3486 = z3zE3485;
      }
      z3zE3487 = z3zE3486;
    }
    z3zE3482 = z3zE3487;
    goto finish_match_3468;
  }
case_3470: ;
  {
    bool z2zE3515;
    z2zE3515 = eq_u256(zv, zWORD_ZERO);
    bool z3zE3483;
    if (z2zE3515) {  z3zE3483 = true;  } else {  z3zE3483 = eq_u256(zv, zWORD_ONE);  }
    z3zE3482 = z3zE3483;
    goto finish_match_3468;
  }
case_3469: ;
finish_match_3468: ;
  z8zE195 = z3zE3482;
end_function_3471: ;
  return z8zE195;
end_block_exception_3472: ;

  return false;
}

uint64_t ztx_y_parity(enum zTxType zt, sail_u256 zv)
{
  uint64_t z8zE196;
  uint64_t z3zE3479;
  {
    if ((zLegacyTx != zt)) goto case_3465;
    bool z2zE3508;
    {
      sail_u256 z2zE3507;
      z2zE3507 = zword_and(zv, zWORD_ONE);
      z2zE3508 = eq_u256(z2zE3507, zWORD_ONE);
    }
    if (z2zE3508) {  z3zE3479 = UINT64_C(0);  } else {  z3zE3479 = UINT64_C(1);  }
    goto finish_match_3463;
  }
case_3465: ;
  {
    bool z2zE3509;
    z2zE3509 = eq_u256(zv, zWORD_ZERO);
    if (z2zE3509) {  z3zE3479 = UINT64_C(0);  } else {  z3zE3479 = UINT64_C(1);  }
    goto finish_match_3463;
  }
case_3464: ;
finish_match_3463: ;
  z8zE196 = z3zE3479;
end_function_3466: ;
  return z8zE196;
end_block_exception_3467: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_auth_valid(sail_fixed_bytes_20 zsender, sail_fixed_bytes_32 zh, uint64_t zparity, sail_u256 zr, sail_u256 zs)
{
  bool z8zE197;
  bool z2zE3505;
  z2zE3505 = zword_ult(zSECP_N_HALF, zs);
  if (z2zE3505) {  z8zE197 = false;  } else {
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3506;
    z2zE3506 = zecrecover_addr(zh, zparity, zr, zs);
    bool z3zE3476;
    {
      bool zrecovered;
      zrecovered = z2zE3506.ztup0;
      sail_fixed_bytes_20 zrecovered_sender;
      zrecovered_sender = z2zE3506.ztup1;
      bool z3zE3477;
      if (zrecovered) {  z3zE3477 = eq_fixed_bytes_20(zrecovered_sender, zsender);  } else {  z3zE3477 = false;  }
      z3zE3476 = z3zE3477;
      goto finish_match_3459;
    }
  case_3460: ;
    sail_match_failure("tx_auth_valid");
  finish_match_3459: ;
    z8zE197 = z3zE3476;
  }
end_function_3461: ;
  return z8zE197;
end_block_exception_3462: ;

  return false;
}

