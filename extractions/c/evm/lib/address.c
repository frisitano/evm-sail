/* Generated from sail/lib/address.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zcreate_address(sail_fixed_bytes_20 zsender, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE190;
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {    zcontent_len = (UINT64_C(21) + znonce_length);
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE3512;
  z3zE3512 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE3511;
  z3zE3511 = zrlp_write_addr(zsender);
  unit z3zE3510;
  z3zE3510 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_20 zaddress;
  {
    sail_u256 z2zE3546;
    {
      sail_fixed_bytes_32 z2zE3545;
      z2zE3545 = zkeccak256_slice(zencoded);
      z2zE3546 = zhash_to_word(z2zE3545);
    }
    zaddress = zword_to_address(z2zE3546);
  }
  unit z3zE3513;
  z3zE3513 = zscratch_rewind(zmark);
  z8zE190 = zaddress;
end_function_3483: ;
  return z8zE190;
end_block_exception_3484: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zcreate2_address(sail_fixed_bytes_20 zsender, sail_u256 zsalt, sail_fixed_bytes_32 zinit_hash)
{
  sail_fixed_bytes_20 z8zE191;
  sail_u256 z2zE3544;
  {
    sail_fixed_bytes_32 z2zE3543;
    {
      zz5listz8z5unionz0zzBytesz9 z2zE3542;
      CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3542);
      {
        struct zBytes z2zE3538;
        CREATE(zBytes)(&z2zE3538);
        {
          zz5listz8z5bv8z9 z3zE3508;
          CREATE(zz5listz8z5bv8z9)(&z3zE3508);
          zconsz3z5bv8(&z3zE3508, UINT64_C(0xFF), z3zE3508);
          zbytes_list(&z2zE3538, z3zE3508, UINT64_C(1));
          KILL(zz5listz8z5bv8z9)(&z3zE3508);
        }
        struct zBytes z2zE3539;
        CREATE(zBytes)(&z2zE3539);
        {
          zz5listz8z5bv8z9 z2zE3535;
          CREATE(zz5listz8z5bv8z9)(&z2zE3535);
          zaddress_to_bytes(&z2zE3535, zsender);
          zbytes_list(&z2zE3539, z2zE3535, zADDRESS_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3535);
        }
        struct zBytes z2zE3540;
        CREATE(zBytes)(&z2zE3540);
        {
          zz5listz8z5bv8z9 z2zE3536;
          CREATE(zz5listz8z5bv8z9)(&z2zE3536);
          zword_to_bytes32(&z2zE3536, zsalt);
          zbytes_list(&z2zE3540, z2zE3536, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3536);
        }
        struct zBytes z2zE3541;
        CREATE(zBytes)(&z2zE3541);
        {
          zz5listz8z5bv8z9 z2zE3537;
          CREATE(zz5listz8z5bv8z9)(&z2zE3537);
          zhash_to_bytes32(&z2zE3537, zinit_hash);
          zbytes_list(&z2zE3541, z2zE3537, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE3537);
        }
        zconsz3z5unionz0zzBytes(&z2zE3542, z2zE3541, z2zE3542);
        zconsz3z5unionz0zzBytes(&z2zE3542, z2zE3540, z2zE3542);
        zconsz3z5unionz0zzBytes(&z2zE3542, z2zE3539, z2zE3542);
        zconsz3z5unionz0zzBytes(&z2zE3542, z2zE3538, z2zE3542);
        KILL(zBytes)(&z2zE3541);
        KILL(zBytes)(&z2zE3540);
        KILL(zBytes)(&z2zE3539);
        KILL(zBytes)(&z2zE3538);
      }
      z2zE3543 = host_keccak_segments(z2zE3542);
      KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3542);
    }
    z2zE3544 = zhash_to_word(z2zE3543);
  }
  z8zE191 = zword_to_address(z2zE3544);
end_function_3481: ;
  return z8zE191;
end_block_exception_3482: ;

  return fixed_bytes_20_zero();
}

