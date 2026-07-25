/* Generated from sail/lib/address.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zcreate_address(sail_fixed_bytes_20 zsender, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE181;
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {    zcontent_len = (UINT64_C(21) + znonce_length);
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE3450;
  z3zE3450 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE3449;
  z3zE3449 = zrlp_write_addr(zsender);
  unit z3zE3448;
  z3zE3448 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_20 zaddress;
  {
    sail_u256 z2zE3522;
    {
      sail_fixed_bytes_32 z2zE3521;
      z2zE3521 = zkeccak256_slice(zencoded);
      z2zE3522 = zhash_to_word(z2zE3521);
    }
    zaddress = zword_to_address(z2zE3522);
  }
  unit z3zE3451;
  z3zE3451 = zscratch_rewind(zmark);
  z8zE181 = zaddress;
end_function_3488: ;
  return z8zE181;
end_block_exception_3489: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zcreate2_address(sail_fixed_bytes_20 zsender, sail_u256 zsalt, sail_fixed_bytes_32 zinit_hash)
{
  sail_fixed_bytes_20 z8zE182;
  sail_u256 z2zE3520;
  {
    sail_fixed_bytes_32 z2zE3519;
    {
      zz5listz8z5unionz0zzBytesz9 z2zE3518;
      CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3518);
      {
        struct zBytes z2zE3514;
        CREATE(zBytes)(&z2zE3514);
        {
          zz5listz8z5bv8z9 z3zE3446;
          CREATE(zz5listz8z5bv8z9)(&z3zE3446);
          zconsz3z5bv8(&z3zE3446, UINT64_C(0xFF), z3zE3446);
          zbytes_list(&z2zE3514, z3zE3446, UINT64_C(1));
          KILL(zz5listz8z5bv8z9)(&z3zE3446);
        }
        struct zBytes z2zE3515;
        CREATE(zBytes)(&z2zE3515);
        {
          zz5listz8z5bv8z9 z2zE3511;
          CREATE(zz5listz8z5bv8z9)(&z2zE3511);
          zaddress_to_bytes(&z2zE3511, zsender);
          zbytes_list(&z2zE3515, z2zE3511, zADDRESS_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3511);
        }
        struct zBytes z2zE3516;
        CREATE(zBytes)(&z2zE3516);
        {
          zz5listz8z5bv8z9 z2zE3512;
          CREATE(zz5listz8z5bv8z9)(&z2zE3512);
          zword_to_bytes32(&z2zE3512, zsalt);
          zbytes_list(&z2zE3516, z2zE3512, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3512);
        }
        struct zBytes z2zE3517;
        CREATE(zBytes)(&z2zE3517);
        {
          zz5listz8z5bv8z9 z2zE3513;
          CREATE(zz5listz8z5bv8z9)(&z2zE3513);
          zhash_to_bytes32(&z2zE3513, zinit_hash);
          zbytes_list(&z2zE3517, z2zE3513, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3513);
        }
        zconsz3z5unionz0zzBytes(&z2zE3518, z2zE3517, z2zE3518);
        zconsz3z5unionz0zzBytes(&z2zE3518, z2zE3516, z2zE3518);
        zconsz3z5unionz0zzBytes(&z2zE3518, z2zE3515, z2zE3518);
        zconsz3z5unionz0zzBytes(&z2zE3518, z2zE3514, z2zE3518);
        KILL(zBytes)(&z2zE3517);
        KILL(zBytes)(&z2zE3516);
        KILL(zBytes)(&z2zE3515);
        KILL(zBytes)(&z2zE3514);
      }
      z2zE3519 = host_keccak_segments(z2zE3518);
      KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3518);
    }
    z2zE3520 = zhash_to_word(z2zE3519);
  }
  z8zE182 = zword_to_address(z2zE3520);
end_function_3486: ;
  return z8zE182;
end_block_exception_3487: ;

  return fixed_bytes_20_zero();
}

