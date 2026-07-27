/* Generated from sail/lib/address.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_20 zcreate_address(sail_fixed_bytes_20 zsender, uint64_t znonce)
{
  sail_fixed_bytes_20 z8zE178;
  uint64_t znonce_length;
  znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  uint64_t zcontent_len;
  {    zcontent_len = (UINT64_C(21) + znonce_length);
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE2675;
  z3zE2675 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE2674;
  z3zE2674 = zrlp_write_addr(zsender);
  unit z3zE2673;
  z3zE2673 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(znonce);
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_20 zaddress;
  {
    sail_u256 z2zE2827;
    {
      sail_fixed_bytes_32 z2zE2826;
      z2zE2826 = zkeccak256_slice(zencoded);
      z2zE2827 = evmsail_hash_to_word(z2zE2826);
    }
    zaddress = evmsail_word_to_address(z2zE2827);
  }
  unit z3zE2676;
  z3zE2676 = zscratch_rewind(zmark);
  z8zE178 = zaddress;
end_function_3084: ;
  return z8zE178;
end_block_exception_3085: ;

  return fixed_bytes_20_zero();
}

sail_fixed_bytes_20 zcreate2_address(sail_fixed_bytes_20 zsender, sail_u256 zsalt, sail_fixed_bytes_32 zinit_hash)
{
  sail_fixed_bytes_20 z8zE179;
  sail_u256 z2zE2825;
  {
    sail_fixed_bytes_32 z2zE2824;
    {
      zz5listz8z5unionz0zzBytesz9 z2zE2823;
      CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE2823);
      {
        struct zBytes z2zE2819;
        CREATE(zBytes)(&z2zE2819);
        {
          zz5listz8z5bv8z9 z3zE2671;
          CREATE(zz5listz8z5bv8z9)(&z3zE2671);
          zconsz3z5bv8(&z3zE2671, UINT64_C(0xFF), z3zE2671);
          zbytes_list(&z2zE2819, z3zE2671, UINT64_C(1));
          KILL(zz5listz8z5bv8z9)(&z3zE2671);
        }
        struct zBytes z2zE2820;
        CREATE(zBytes)(&z2zE2820);
        {
          zz5listz8z5bv8z9 z2zE2816;
          CREATE(zz5listz8z5bv8z9)(&z2zE2816);
          zaddress_to_bytes(&z2zE2816, zsender);
          zbytes_list(&z2zE2820, z2zE2816, zADDRESS_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE2816);
        }
        struct zBytes z2zE2821;
        CREATE(zBytes)(&z2zE2821);
        {
          zz5listz8z5bv8z9 z2zE2817;
          CREATE(zz5listz8z5bv8z9)(&z2zE2817);
          zword_to_bytes32(&z2zE2817, zsalt);
          zbytes_list(&z2zE2821, z2zE2817, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE2817);
        }
        struct zBytes z2zE2822;
        CREATE(zBytes)(&z2zE2822);
        {
          zz5listz8z5bv8z9 z2zE2818;
          CREATE(zz5listz8z5bv8z9)(&z2zE2818);
          zhash_to_bytes32(&z2zE2818, zinit_hash);
          zbytes_list(&z2zE2822, z2zE2818, zWORD_BYTE_LENGTH);
          KILL(zz5listz8z5bv8z9)(&z2zE2818);
        }
        zconsz3z5unionz0zzBytes(&z2zE2823, z2zE2822, z2zE2823);
        zconsz3z5unionz0zzBytes(&z2zE2823, z2zE2821, z2zE2823);
        zconsz3z5unionz0zzBytes(&z2zE2823, z2zE2820, z2zE2823);
        zconsz3z5unionz0zzBytes(&z2zE2823, z2zE2819, z2zE2823);
        KILL(zBytes)(&z2zE2822);
        KILL(zBytes)(&z2zE2821);
        KILL(zBytes)(&z2zE2820);
        KILL(zBytes)(&z2zE2819);
      }
      z2zE2824 = host_keccak_segments(z2zE2823);
      KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE2823);
    }
    z2zE2825 = evmsail_hash_to_word(z2zE2824);
  }
  z8zE179 = evmsail_word_to_address(z2zE2825);
end_function_3082: ;
  return z8zE179;
end_block_exception_3083: ;

  return fixed_bytes_20_zero();
}

