/* Generated from sail/lib/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_u256 zlegacy_sig_chain_id(sail_u256 zv)
{
  sail_u256 z8zE180;
  sail_u256 z2zE2815;
  z2zE2815 = zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zv, UINT64_C(35));
  z8zE180 = zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2815, UINT64_C(2));
end_function_3080: ;
  return z8zE180;
end_block_exception_3081: ;

  return u256_zero();
}

void create_letbind_112(void) {

  uint64_t z3zE123;
  z3zE123 = UINT64_C(2);
  zLEGACY_SIGNATURE_SUFFIX_LENGTH = z3zE123;
let_end_305: ;
}
void kill_letbind_112(void) {
}

void create_letbind_113(void) {

  uint64_t z3zE124;
  z3zE124 = zDOUBLE_WORD_BYTE_LENGTH;
  zPUBLIC_KEY_BODY_LENGTH = z3zE124;
let_end_306: ;
}
void kill_letbind_113(void) {
}

sail_fixed_bytes_32 ztx_signing_hash(enum zTxType zt, struct zByteSliceFields zcontent_src, sail_u256 zv)
{
  sail_fixed_bytes_32 z8zE181;
  uint64_t ztb;
  ztb = ztx_type_byte(zt);
  bool zeip155;
  {
    bool z2zE2814;
    z2zE2814 = (ztb == UINT64_C(0x00));
    bool z3zE2659;
    if (z2zE2814) {
      z3zE2659 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
    } else {  z3zE2659 = false;  }
    zeip155 = z3zE2659;
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
    bool z2zE2812;
    {
      uint64_t z2zE2811;
      z2zE2811 = zcontent_src.zlen;
      z2zE2812 = (!(UINT64_C(1073741824) < z2zE2811));
    }
    if (z2zE2812) {  zcontent_length = zcontent_src.zlen;  } else {
      struct zexception z2zE2813;
      CREATE(zexception)(&z2zE2813);
      zInvalidBlock(&z2zE2813, zRlpDecode);
      COPY(zexception)(current_exception, z2zE2813);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/tx.sail:46.12-46.41");
      KILL(zexception)(&z2zE2813);
      goto end_block_exception_3079;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2813);
    }
  }
  uint64_t z3zE2670;
  z3zE2670 = zsuffix_len;
  uint64_t zcontent_len;
  {    zcontent_len = (zcontent_length + z3zE2670);
  }
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE2798;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2798);
  zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE2798, zcontent_len);
  sail_fixed_bytes_32 z3zE2660;
  {
    zz5listz8z5bv8z9 zprefix;
    CREATE(zz5listz8z5bv8z9)(&zprefix);
    COPY(zz5listz8z5bv8z9)(&zprefix, z2zE2798.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE2798.ztup1;
    if (zeip155) {
      uint64_t zmark;
      zmark = zscratch_begin(UNIT);
      unit z3zE2667;
      z3zE2667 = zrlp_write_uint_word(zchain_id);
      zz5listz8z5bv8z9 z3zE2665;
      CREATE(zz5listz8z5bv8z9)(&z3zE2665);
      zconsz3z5bv8(&z3zE2665, UINT64_C(0x80), z3zE2665);
      zconsz3z5bv8(&z3zE2665, UINT64_C(0x80), z3zE2665);
      unit z3zE2666;
      z3zE2666 = zscratch_push_bytes(z3zE2665, zLEGACY_SIGNATURE_SUFFIX_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z3zE2665);
      struct zByteSliceFields zsuffix;
      zsuffix = zrlp_finish(zmark);
      sail_fixed_bytes_32 zsigning_hash;
      {
        zz5listz8z5unionz0zzBytesz9 z2zE2802;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE2802);
        {
          struct zBytes z2zE2799;
          CREATE(zBytes)(&z2zE2799);
          zbytes_list(&z2zE2799, zprefix, zprefix_len);
          struct zBytes z2zE2800;
          CREATE(zBytes)(&z2zE2800);
          zBytesSlice(&z2zE2800, zcontent_src);
          struct zBytes z2zE2801;
          CREATE(zBytes)(&z2zE2801);
          zBytesSlice(&z2zE2801, zsuffix);
          zconsz3z5unionz0zzBytes(&z2zE2802, z2zE2801, z2zE2802);
          zconsz3z5unionz0zzBytes(&z2zE2802, z2zE2800, z2zE2802);
          zconsz3z5unionz0zzBytes(&z2zE2802, z2zE2799, z2zE2802);
          KILL(zBytes)(&z2zE2801);
          KILL(zBytes)(&z2zE2800);
          KILL(zBytes)(&z2zE2799);
        }
        zsigning_hash = host_keccak_segments(z2zE2802);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE2802);
      }
      unit z3zE2669;
      z3zE2669 = zscratch_rewind(zmark);
      z3zE2660 = zsigning_hash;
    } else {
      bool z2zE2803;
      z2zE2803 = (ztb != UINT64_C(0x00));
      if (z2zE2803) {
        zz5listz8z5unionz0zzBytesz9 z2zE2807;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE2807);
        {
          struct zBytes z2zE2804;
          CREATE(zBytes)(&z2zE2804);
          {
            zz5listz8z5bv8z9 z3zE2663;
            CREATE(zz5listz8z5bv8z9)(&z3zE2663);
            zconsz3z5bv8(&z3zE2663, ztb, z3zE2663);
            zbytes_list(&z2zE2804, z3zE2663, UINT64_C(1));
            KILL(zz5listz8z5bv8z9)(&z3zE2663);
          }
          struct zBytes z2zE2805;
          CREATE(zBytes)(&z2zE2805);
          zbytes_list(&z2zE2805, zprefix, zprefix_len);
          struct zBytes z2zE2806;
          CREATE(zBytes)(&z2zE2806);
          zBytesSlice(&z2zE2806, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE2807, z2zE2806, z2zE2807);
          zconsz3z5unionz0zzBytes(&z2zE2807, z2zE2805, z2zE2807);
          zconsz3z5unionz0zzBytes(&z2zE2807, z2zE2804, z2zE2807);
          KILL(zBytes)(&z2zE2806);
          KILL(zBytes)(&z2zE2805);
          KILL(zBytes)(&z2zE2804);
        }
        z3zE2660 = host_keccak_segments(z2zE2807);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE2807);
      } else {
        zz5listz8z5unionz0zzBytesz9 z2zE2810;
        CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE2810);
        {
          struct zBytes z2zE2808;
          CREATE(zBytes)(&z2zE2808);
          zbytes_list(&z2zE2808, zprefix, zprefix_len);
          struct zBytes z2zE2809;
          CREATE(zBytes)(&z2zE2809);
          zBytesSlice(&z2zE2809, zcontent_src);
          zconsz3z5unionz0zzBytes(&z2zE2810, z2zE2809, z2zE2810);
          zconsz3z5unionz0zzBytes(&z2zE2810, z2zE2808, z2zE2810);
          KILL(zBytes)(&z2zE2809);
          KILL(zBytes)(&z2zE2808);
        }
        z3zE2660 = host_keccak_segments(z2zE2810);
        KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE2810);
      }
    }
    KILL(zz5listz8z5bv8z9)(&zprefix);
    goto finish_match_3076;
  }
case_3077: ;
  sail_match_failure("tx_signing_hash");
finish_match_3076: ;
  z8zE181 = z3zE2660;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE2798);
end_function_3078: ;
  return z8zE181;
end_block_exception_3079: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zauth_signing_hash(sail_u256 zchain_id, sail_fixed_bytes_20 zaddr, uint64_t znonce)
{
  sail_fixed_bytes_32 z8zE182;
  uint64_t zchain_id_length;
  zchain_id_length = zrlp_uint_word_sizze(zchain_id);
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {
    uint64_t z2zE2797;
    {    z2zE2797 = (zchain_id_length + UINT64_C(21));
    }
    {    zcontent_len = (z2zE2797 + znonce_length);
    }
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  zz5listz8z5bv8z9 z3zE2652;
  CREATE(zz5listz8z5bv8z9)(&z3zE2652);
  zconsz3z5bv8(&z3zE2652, UINT64_C(0x05), z3zE2652);
  unit z3zE2657;
  z3zE2657 = zscratch_push_bytes(z3zE2652, UINT64_C(1));
  KILL(zz5listz8z5bv8z9)(&z3zE2652);
  unit z3zE2656;
  z3zE2656 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE2655;
  z3zE2655 = zrlp_write_uint_word(zchain_id);
  unit z3zE2654;
  z3zE2654 = zrlp_write_addr(zaddr);
  unit z3zE2653;
  z3zE2653 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_32 zsigning_hash;
  zsigning_hash = zkeccak256_slice(zencoded);
  unit z3zE2658;
  z3zE2658 = zscratch_rewind(zmark);
  z8zE182 = zsigning_hash;
end_function_3074: ;
  return z8zE182;
end_block_exception_3075: ;

  return fixed_bytes_32_zero();
}

bool ztx_sig_v_valid(uint64_t zchain_id, enum zTxType zt, sail_u256 zv)
{
  bool z8zE183;
  bool z3zE2645;
  {
    if ((zLegacyTx != zt)) goto case_3071;
    bool z2zE2795;
    z2zE2795 = u256_eq_u64(zv, UINT64_C(27));
    bool z3zE2650;
    if (z2zE2795) {  z3zE2650 = true;  } else {
      bool z2zE2794;
      z2zE2794 = u256_eq_u64(zv, UINT64_C(28));
      bool z3zE2649;
      if (z2zE2794) {  z3zE2649 = true;  } else {
        bool z2zE2793;
        z2zE2793 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(35), zv);
        bool z3zE2648;
        if (z2zE2793) {
          sail_u256 z2zE2791;
          z2zE2791 = zlegacy_sig_chain_id(zv);
          uint64_t z2zE2792;
          z2zE2792 = zword_of_chain_identifier(zchain_id);
          z3zE2648 = u256_eq_u64(z2zE2791, z2zE2792);
        } else {  z3zE2648 = false;  }
        z3zE2649 = z3zE2648;
      }
      z3zE2650 = z3zE2649;
    }
    z3zE2645 = z3zE2650;
    goto finish_match_3069;
  }
case_3071: ;
  {
    bool z2zE2796;
    z2zE2796 = eq_u256(zv, zWORD_ZERO);
    bool z3zE2646;
    if (z2zE2796) {  z3zE2646 = true;  } else {  z3zE2646 = eq_u256(zv, zWORD_ONE);  }
    z3zE2645 = z3zE2646;
    goto finish_match_3069;
  }
case_3070: ;
finish_match_3069: ;
  z8zE183 = z3zE2645;
end_function_3072: ;
  return z8zE183;
end_block_exception_3073: ;

  return false;
}

uint64_t ztx_y_parity(enum zTxType zt, sail_u256 zv)
{
  uint64_t z8zE184;
  uint64_t z3zE2642;
  {
    if ((zLegacyTx != zt)) goto case_3066;
    bool z2zE2789;
    {
      sail_u256 z2zE2788;
      z2zE2788 = zword_and(zv, zWORD_ONE);
      z2zE2789 = eq_u256(z2zE2788, zWORD_ONE);
    }
    if (z2zE2789) {  z3zE2642 = UINT64_C(0);  } else {  z3zE2642 = UINT64_C(1);  }
    goto finish_match_3064;
  }
case_3066: ;
  {
    bool z2zE2790;
    z2zE2790 = eq_u256(zv, zWORD_ZERO);
    if (z2zE2790) {  z3zE2642 = UINT64_C(0);  } else {  z3zE2642 = UINT64_C(1);  }
    goto finish_match_3064;
  }
case_3065: ;
finish_match_3064: ;
  z8zE184 = z3zE2642;
end_function_3067: ;
  return z8zE184;
end_block_exception_3068: ;

  return UINT64_C(0xdeadc0de);
}

bool ztx_auth_valid(sail_fixed_bytes_20 zsender, sail_fixed_bytes_32 zh, uint64_t zparity, sail_u256 zr, sail_u256 zs)
{
  bool z8zE185;
  bool z2zE2786;
  z2zE2786 = zword_ult(zSECP_N_HALF, zs);
  if (z2zE2786) {  z8zE185 = false;  } else {
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2787;
    z2zE2787 = zecrecover_addr(zh, zparity, zr, zs);
    bool z3zE2639;
    {
      bool zrecovered;
      zrecovered = z2zE2787.ztup0;
      sail_fixed_bytes_20 zrecovered_sender;
      zrecovered_sender = z2zE2787.ztup1;
      bool z3zE2640;
      if (zrecovered) {  z3zE2640 = eq_fixed_bytes_20(zrecovered_sender, zsender);  } else {  z3zE2640 = false;  }
      z3zE2639 = z3zE2640;
      goto finish_match_3060;
    }
  case_3061: ;
    sail_match_failure("tx_auth_valid");
  finish_match_3060: ;
    z8zE185 = z3zE2639;
  }
end_function_3062: ;
  return z8zE185;
end_block_exception_3063: ;

  return false;
}

