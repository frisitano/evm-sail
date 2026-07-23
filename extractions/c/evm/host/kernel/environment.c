/* Generated from sail/host/kernel/environment.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
sail_u256 zk_env(enum zEnvField zf)
{
  sail_u256 z8zE213;
  sail_u256 z3zE3194;
  {
    if ((zF_Number != zf)) goto case_3263;
    uint64_t z2zE3249;
    {
      uint64_t z2zE3248;
      z2zE3248 = zk_header.znumber;
      z2zE3249 = zword_of_block_number(z2zE3248);
    }
    z3zE3194 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3249);
    goto finish_match_3253;
  }
case_3263: ;
  {
    if ((zF_Timestamp != zf)) goto case_3262;
    uint64_t z2zE3251;
    {
      uint64_t z2zE3250;
      z2zE3250 = zk_header.ztimestamp;
      z2zE3251 = zword_of_block_timestamp(z2zE3250);
    }
    z3zE3194 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3251);
    goto finish_match_3253;
  }
case_3262: ;
  {
    if ((zF_Coinbase != zf)) goto case_3261;
    sail_fixed_bytes_20 z2zE3252;
    z2zE3252 = zk_header.zfee_recipient;
    z3zE3194 = zaddress_to_word(z2zE3252);
    goto finish_match_3253;
  }
case_3261: ;
  {
    if ((zF_BaseFee != zf)) goto case_3260;
    z3zE3194 = zk_header.zbase_fee;
    goto finish_match_3253;
  }
case_3260: ;
  {
    if ((zF_ChainId != zf)) goto case_3259;
    uint64_t z2zE3253;
    z2zE3253 = zword_of_chain_identifier(zk_chain_id);
    z3zE3194 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3253);
    goto finish_match_3253;
  }
case_3259: ;
  {
    if ((zF_GasLimit != zf)) goto case_3258;
    uint64_t z2zE3254;
    z2zE3254 = zk_header.zgas_limit;
    z3zE3194 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3254);
    goto finish_match_3253;
  }
case_3258: ;
  {
    if ((zF_PrevRandao != zf)) goto case_3257;
    z3zE3194 = zk_header.zprev_randao;
    goto finish_match_3253;
  }
case_3257: ;
  {
    if ((zF_Origin != zf)) goto case_3256;
    sail_fixed_bytes_20 z2zE3255;
    z2zE3255 = zk_tx.zorigin;
    z3zE3194 = zaddress_to_word(z2zE3255);
    goto finish_match_3253;
  }
case_3256: ;
  {
    if ((zF_GasPrice != zf)) goto case_3255;
    z3zE3194 = zk_tx.zgas_price;
    goto finish_match_3253;
  }
case_3255: ;
  {
    /* complete */
    uint64_t z2zE3257;
    {
      uint64_t z2zE3256;
      z2zE3256 = zk_header.zslot_number;
      z2zE3257 = zword_of_slot_number(z2zE3256);
    }
    z3zE3194 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3257);
    goto finish_match_3253;
  }
case_3254: ;
finish_match_3253: ;
  z8zE213 = z3zE3194;
end_function_3264: ;
  return z8zE213;
end_block_exception_3265: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_coinbase(unit z3zE3193)
{
  sail_fixed_bytes_20 z8zE214;
  z8zE214 = zk_header.zfee_recipient;
end_function_3251: ;
  return z8zE214;
end_block_exception_3252: ;

  return fixed_bytes_20_zero();
}

sail_u256 zblockhash_word_distance(sail_u256 zcurrent, sail_u256 znumber)
{
  sail_u256 z8zE215;
  z8zE215 = u256_sub(zcurrent, znumber);
end_function_3249: ;
  return z8zE215;
end_block_exception_3250: ;

  return u256_zero();
}

sail_fixed_bytes_32 zk_blockhash(sail_u256 znumber_word)
{
  sail_fixed_bytes_32 z8zE216;
  uint64_t zcurrent;
  zcurrent = zk_header.znumber;
  sail_u256 zcurrent_word;
  {
    uint64_t z2zE3247;
    z2zE3247 = zword_of_block_number(zcurrent);
    zcurrent_word = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3247);
  }
  bool z2zE3242;
  z2zE3242 = u256_lt(znumber_word, zcurrent_word);
  if (z2zE3242) {
    sail_u256 z2zE3243;
    z2zE3243 = zblockhash_word_distance(zcurrent_word, znumber_word);
    sail_fixed_bytes_32 z3zE3191;
    {
      sail_u256 zdistance_word;
      zdistance_word = z2zE3243;
      bool z2zE3244;
      z2zE3244 = (!u64_lt_u256(UINT64_C(256), zdistance_word));
      if (z2zE3244) {
        uint64_t zdistance;
        zdistance = u256_to_u64(zdistance_word);
        bool z2zE3245;
        z2zE3245 = (zk_n_headers < zdistance);
        if (z2zE3245) {
          struct zexception z2zE3246;
          CREATE(zexception)(&z2zE3246);
          zInvalidBlock(&z2zE3246, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE3246);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/host/kernel/environment.sail:106.16-106.52");
          KILL(zexception)(&z2zE3246);
          goto end_block_exception_3248;
          /* unreachable after throw */
          KILL(zexception)(&z2zE3246);
        } else {
          uint64_t zindex;
          {    zindex = (zdistance - UINT64_C(1));
          }
          z3zE3191 = ancestor_hash_read(zindex);
        }
      } else {  z3zE3191 = zZERO_HASH;  }
      goto finish_match_3245;
    }
  case_3246: ;
    sail_match_failure("k_blockhash");
  finish_match_3245: ;
    z8zE216 = z3zE3191;
  } else {  z8zE216 = zZERO_HASH;  }
end_function_3247: ;
  return z8zE216;
end_block_exception_3248: ;

  return fixed_bytes_32_zero();
}

sail_u256 zk_blobhash(sail_u256 zindex_word)
{
  sail_u256 z8zE217;
  uint64_t zcount;
  {
    struct zBlobHashes z2zE3241;
    z2zE3241 = zk_tx.zblob_hashes;
    zcount = z2zE3241.zcount;
  }
  bool z2zE3237;
  z2zE3237 = u256_lt_u64(zindex_word, zcount);
  if (z2zE3237) {
    uint64_t zindex;
    zindex = u256_to_u64(zindex_word);
    uint64_t zoffset;
    {
      uint64_t z2zE3240;
      {    z2zE3240 = (UINT64_C(33) * zindex);
      }
      {    zoffset = (z2zE3240 + UINT64_C(1));
      }
    }
    struct zByteSliceFields z2zE3239;
    {
      struct zBlobHashes z2zE3238;
      z2zE3238 = zk_tx.zblob_hashes;
      z2zE3239 = z2zE3238.zbytes;
    }
    z8zE217 = zslice_load_n(z2zE3239, zoffset, zWORD_BYTE_LENGTH);
  } else {  z8zE217 = zZERO_WORD;  }
end_function_3243: ;
  return z8zE217;
end_block_exception_3244: ;

  return u256_zero();
}

sail_fixed_bytes_20 zk_create_addr(sail_fixed_bytes_20 za, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE218;
  z8zE218 = zcreate_address(za, znonce);
end_function_3241: ;
  return z8zE218;
end_block_exception_3242: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zk_create2_addr(sail_fixed_bytes_20 za, sail_u256 zsalt, sail_fixed_bytes_32 zinithash)
{
  sail_fixed_bytes_20 z8zE219;
  z8zE219 = zcreate2_address(za, zsalt, zinithash);
end_function_3239: ;
  return z8zE219;
end_block_exception_3240: ;

  return fixed_bytes_20_zero();
}

