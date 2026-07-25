/* Generated from sail/host/kernel/environment.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_u256 zk_env(enum zEnvField zf)
{
  sail_u256 z8zE209;
  sail_u256 z3zE3192;
  {
    if ((zF_Number != zf)) goto case_3258;
    uint64_t z2zE3225;
    {
      uint64_t z2zE3224;
      z2zE3224 = zk_header.znumber;
      z2zE3225 = zword_of_block_number(z2zE3224);
    }
    z3zE3192 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3225);
    goto finish_match_3248;
  }
case_3258: ;
  {
    if ((zF_Timestamp != zf)) goto case_3257;
    uint64_t z2zE3227;
    {
      uint64_t z2zE3226;
      z2zE3226 = zk_header.ztimestamp;
      z2zE3227 = zword_of_block_timestamp(z2zE3226);
    }
    z3zE3192 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3227);
    goto finish_match_3248;
  }
case_3257: ;
  {
    if ((zF_Coinbase != zf)) goto case_3256;
    sail_fixed_bytes_20 z2zE3228;
    z2zE3228 = zk_header.zfee_recipient;
    z3zE3192 = zaddress_to_word(z2zE3228);
    goto finish_match_3248;
  }
case_3256: ;
  {
    if ((zF_BaseFee != zf)) goto case_3255;
    z3zE3192 = zk_header.zbase_fee;
    goto finish_match_3248;
  }
case_3255: ;
  {
    if ((zF_ChainId != zf)) goto case_3254;
    uint64_t z2zE3229;
    z2zE3229 = zword_of_chain_identifier(zk_chain_id);
    z3zE3192 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3229);
    goto finish_match_3248;
  }
case_3254: ;
  {
    if ((zF_GasLimit != zf)) goto case_3253;
    uint64_t z2zE3230;
    z2zE3230 = zk_header.zgas_limit;
    z3zE3192 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3230);
    goto finish_match_3248;
  }
case_3253: ;
  {
    if ((zF_PrevRandao != zf)) goto case_3252;
    z3zE3192 = zk_header.zprev_randao;
    goto finish_match_3248;
  }
case_3252: ;
  {
    if ((zF_Origin != zf)) goto case_3251;
    sail_fixed_bytes_20 z2zE3231;
    z2zE3231 = zk_tx.zorigin;
    z3zE3192 = zaddress_to_word(z2zE3231);
    goto finish_match_3248;
  }
case_3251: ;
  {
    if ((zF_GasPrice != zf)) goto case_3250;
    z3zE3192 = zk_tx.zgas_price;
    goto finish_match_3248;
  }
case_3250: ;
  {
    /* complete */
    uint64_t z2zE3233;
    {
      uint64_t z2zE3232;
      z2zE3232 = zk_header.zslot_number;
      z2zE3233 = zword_of_slot_number(z2zE3232);
    }
    z3zE3192 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3233);
    goto finish_match_3248;
  }
case_3249: ;
finish_match_3248: ;
  z8zE209 = z3zE3192;
end_function_3259: ;
  return z8zE209;
end_block_exception_3260: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_coinbase(unit z3zE3191)
{
  sail_fixed_bytes_20 z8zE210;
  z8zE210 = zk_header.zfee_recipient;
end_function_3246: ;
  return z8zE210;
end_block_exception_3247: ;

  return fixed_bytes_20_zero();
}

sail_u256 zblockhash_word_distance(sail_u256 zcurrent, sail_u256 znumber)
{
  sail_u256 z8zE211;
  z8zE211 = u256_sub(zcurrent, znumber);
end_function_3244: ;
  return z8zE211;
end_block_exception_3245: ;

  return u256_zero();
}

sail_fixed_bytes_32 zk_blockhash(sail_u256 znumber_word)
{
  sail_fixed_bytes_32 z8zE212;
  uint64_t zcurrent;
  zcurrent = zk_header.znumber;
  sail_u256 zcurrent_word;
  {
    uint64_t z2zE3223;
    z2zE3223 = zword_of_block_number(zcurrent);
    zcurrent_word = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3223);
  }
  bool z2zE3218;
  z2zE3218 = u256_lt(znumber_word, zcurrent_word);
  if (z2zE3218) {
    sail_u256 z2zE3219;
    z2zE3219 = zblockhash_word_distance(zcurrent_word, znumber_word);
    sail_fixed_bytes_32 z3zE3189;
    {
      sail_u256 zdistance_word;
      zdistance_word = z2zE3219;
      bool z2zE3220;
      z2zE3220 = (!u64_lt_u256(UINT64_C(256), zdistance_word));
      if (z2zE3220) {
        uint64_t zdistance;
        zdistance = u256_to_u64(zdistance_word);
        bool z2zE3221;
        z2zE3221 = (zk_n_headers < zdistance);
        if (z2zE3221) {
          struct zexception z2zE3222;
          CREATE(zexception)(&z2zE3222);
          zInvalidBlock(&z2zE3222, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE3222);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/host/kernel/environment.sail:106.16-106.52");
          KILL(zexception)(&z2zE3222);
          goto end_block_exception_3243;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3222);
        } else {
          uint64_t zindex;
          {    zindex = (zdistance - UINT64_C(1));
          }
          z3zE3189 = ancestor_hash_read(zindex);
        }
      } else {  z3zE3189 = zZERO_HASH;  }
      goto finish_match_3240;
    }
  case_3241: ;
    sail_match_failure("k_blockhash");
  finish_match_3240: ;
    z8zE212 = z3zE3189;
  } else {  z8zE212 = zZERO_HASH;  }
end_function_3242: ;
  return z8zE212;
end_block_exception_3243: ;

  return fixed_bytes_32_zero();
}

sail_u256 zk_blobhash(sail_u256 zindex_word)
{
  sail_u256 z8zE213;
  uint64_t zcount;
  {
    struct zBlobHashes z2zE3217;
    z2zE3217 = zk_tx.zblob_hashes;
    zcount = z2zE3217.zcount;
  }
  bool z2zE3213;
  z2zE3213 = u256_lt_u64(zindex_word, zcount);
  if (z2zE3213) {
    uint64_t zindex;
    zindex = u256_to_u64(zindex_word);
    uint64_t zoffset;
    {
      uint64_t z2zE3216;
      {    z2zE3216 = (UINT64_C(33) * zindex);
      }
      {    zoffset = (z2zE3216 + UINT64_C(1));
      }
    }
    struct zByteSliceFields z2zE3215;
    {
      struct zBlobHashes z2zE3214;
      z2zE3214 = zk_tx.zblob_hashes;
      z2zE3215 = z2zE3214.zbytes;
    }
    z8zE213 = zslice_load_n(z2zE3215, zoffset, zWORD_BYTE_LENGTH);
  } else {  z8zE213 = zZERO_WORD;  }
end_function_3238: ;
  return z8zE213;
end_block_exception_3239: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_create_addr(sail_fixed_bytes_20 za, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE214;
  z8zE214 = zcreate_address(za, znonce);
end_function_3236: ;
  return z8zE214;
end_block_exception_3237: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zk_create2_addr(sail_fixed_bytes_20 za, sail_u256 zsalt, sail_fixed_bytes_32 zinithash)
{
  sail_fixed_bytes_20 z8zE215;
  z8zE215 = zcreate2_address(za, zsalt, zinithash);
end_function_3234: ;
  return z8zE215;
end_block_exception_3235: ;

  return fixed_bytes_20_zero();
}

