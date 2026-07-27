/* Generated from sail/host/kernel/environment.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_u256 zk_env(enum zEnvField zf)
{
  sail_u256 z8zE206;
  sail_u256 z3zE2417;
  {
    if ((zF_Number != zf)) goto case_2854;
    uint64_t z2zE2526;
    {
      uint64_t z2zE2525;
      z2zE2525 = zk_header.znumber;
      z2zE2526 = zword_of_block_number(z2zE2525);
    }
    z3zE2417 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2526);
    goto finish_match_2844;
  }
case_2854: ;
  {
    if ((zF_Timestamp != zf)) goto case_2853;
    uint64_t z2zE2528;
    {
      uint64_t z2zE2527;
      z2zE2527 = zk_header.ztimestamp;
      z2zE2528 = zword_of_block_timestamp(z2zE2527);
    }
    z3zE2417 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2528);
    goto finish_match_2844;
  }
case_2853: ;
  {
    if ((zF_Coinbase != zf)) goto case_2852;
    sail_fixed_bytes_20 z2zE2529;
    z2zE2529 = zk_header.zfee_recipient;
    z3zE2417 = evmsail_address_to_word(z2zE2529);
    goto finish_match_2844;
  }
case_2852: ;
  {
    if ((zF_BaseFee != zf)) goto case_2851;
    z3zE2417 = zk_header.zbase_fee;
    goto finish_match_2844;
  }
case_2851: ;
  {
    if ((zF_ChainId != zf)) goto case_2850;
    uint64_t z2zE2530;
    z2zE2530 = zword_of_chain_identifier(zk_chain_id);
    z3zE2417 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2530);
    goto finish_match_2844;
  }
case_2850: ;
  {
    if ((zF_GasLimit != zf)) goto case_2849;
    uint64_t z2zE2531;
    z2zE2531 = zk_header.zgas_limit;
    z3zE2417 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2531);
    goto finish_match_2844;
  }
case_2849: ;
  {
    if ((zF_PrevRandao != zf)) goto case_2848;
    z3zE2417 = zk_header.zprev_randao;
    goto finish_match_2844;
  }
case_2848: ;
  {
    if ((zF_Origin != zf)) goto case_2847;
    sail_fixed_bytes_20 z2zE2532;
    z2zE2532 = zk_tx.zorigin;
    z3zE2417 = evmsail_address_to_word(z2zE2532);
    goto finish_match_2844;
  }
case_2847: ;
  {
    if ((zF_GasPrice != zf)) goto case_2846;
    z3zE2417 = zk_tx.zgas_price;
    goto finish_match_2844;
  }
case_2846: ;
  {
    /* complete */
    uint64_t z2zE2534;
    {
      uint64_t z2zE2533;
      z2zE2533 = zk_header.zslot_number;
      z2zE2534 = zword_of_slot_number(z2zE2533);
    }
    z3zE2417 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2534);
    goto finish_match_2844;
  }
case_2845: ;
finish_match_2844: ;
  z8zE206 = z3zE2417;
end_function_2855: ;
  return z8zE206;
end_block_exception_2856: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_coinbase(unit z3zE2416)
{
  sail_fixed_bytes_20 z8zE207;
  z8zE207 = zk_header.zfee_recipient;
end_function_2842: ;
  return z8zE207;
end_block_exception_2843: ;

  return fixed_bytes_20_zero();
}

sail_u256 zblockhash_word_distance(sail_u256 zcurrent, sail_u256 znumber)
{
  sail_u256 z8zE208;
  z8zE208 = u256_sub(zcurrent, znumber);
end_function_2840: ;
  return z8zE208;
end_block_exception_2841: ;

  return u256_zero();
}

sail_fixed_bytes_32 zk_blockhash(sail_u256 znumber_word)
{
  sail_fixed_bytes_32 z8zE209;
  uint64_t zcurrent;
  zcurrent = zk_header.znumber;
  sail_u256 zcurrent_word;
  {
    uint64_t z2zE2524;
    z2zE2524 = zword_of_block_number(zcurrent);
    zcurrent_word = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2524);
  }
  bool z2zE2519;
  z2zE2519 = u256_lt(znumber_word, zcurrent_word);
  if (z2zE2519) {
    sail_u256 z2zE2520;
    z2zE2520 = zblockhash_word_distance(zcurrent_word, znumber_word);
    sail_fixed_bytes_32 z3zE2414;
    {
      sail_u256 zdistance_word;
      zdistance_word = z2zE2520;
      bool z2zE2521;
      z2zE2521 = (!u64_lt_u256(UINT64_C(256), zdistance_word));
      if (z2zE2521) {
        uint64_t zdistance;
        zdistance = u256_to_u64(zdistance_word);
        bool z2zE2522;
        z2zE2522 = (zk_n_headers < zdistance);
        if (z2zE2522) {
          struct zexception z2zE2523;
          CREATE(zexception)(&z2zE2523);
          zInvalidBlock(&z2zE2523, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE2523);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/host/kernel/environment.sail:109.16-109.52");
          KILL(zexception)(&z2zE2523);
          goto end_block_exception_2839;
          /* unreachable after throw */
          KILL(zexception)(&z2zE2523);
        } else {
          uint64_t zindex;
          {    zindex = (zdistance - UINT64_C(1));
          }
          z3zE2414 = ancestor_hash_read(zindex);
        }
      } else {  z3zE2414 = zZERO_HASH;  }
      goto finish_match_2836;
    }
  case_2837: ;
    sail_match_failure("k_blockhash");
  finish_match_2836: ;
    z8zE209 = z3zE2414;
  } else {  z8zE209 = zZERO_HASH;  }
end_function_2838: ;
  return z8zE209;
end_block_exception_2839: ;

  return fixed_bytes_32_zero();
}

sail_u256 zk_blobhash(sail_u256 zindex_word)
{
  sail_u256 z8zE210;
  uint64_t zcount;
  {
    struct zBlobHashes z2zE2518;
    z2zE2518 = zk_tx.zblob_hashes;
    zcount = z2zE2518.zcount;
  }
  bool z2zE2514;
  z2zE2514 = u256_lt_u64(zindex_word, zcount);
  if (z2zE2514) {
    uint64_t zindex;
    zindex = u256_to_u64(zindex_word);
    uint64_t zoffset;
    {
      uint64_t z2zE2517;
      {    z2zE2517 = (UINT64_C(33) * zindex);
      }
      {    zoffset = (z2zE2517 + UINT64_C(1));
      }
    }
    struct zByteSliceFields z2zE2516;
    {
      struct zBlobHashes z2zE2515;
      z2zE2515 = zk_tx.zblob_hashes;
      z2zE2516 = z2zE2515.zbytes;
    }
    z8zE210 = zslice_load_n(z2zE2516, zoffset, zWORD_BYTE_LENGTH);
  } else {  z8zE210 = zZERO_WORD;  }
end_function_2834: ;
  return z8zE210;
end_block_exception_2835: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_create_addr(sail_fixed_bytes_20 za, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE211;
  z8zE211 = zcreate_address(za, znonce);
end_function_2832: ;
  return z8zE211;
end_block_exception_2833: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zk_create2_addr(sail_fixed_bytes_20 za, sail_u256 zsalt, sail_fixed_bytes_32 zinithash)
{
  sail_fixed_bytes_20 z8zE212;
  z8zE212 = zcreate2_address(za, zsalt, zinithash);
end_function_2830: ;
  return z8zE212;
end_block_exception_2831: ;

  return fixed_bytes_20_zero();
}

