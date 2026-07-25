/* Generated from sail/primitives/crypto.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_55(void) {

  sail_fixed_bytes_32 z3zE60;
  zz5vecz8z5bv8z9 z3zE59;
  CREATE(zz5vecz8z5bv8z9)(&z3zE59);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE59, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(0), UINT64_C(0x70));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(1), UINT64_C(0xA4));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(2), UINT64_C(0x85));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(3), UINT64_C(0x5D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(4), UINT64_C(0x04));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(5), UINT64_C(0xD8));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(6), UINT64_C(0xFA));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(7), UINT64_C(0x7B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(8), UINT64_C(0x3B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(9), UINT64_C(0x27));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(10), UINT64_C(0x82));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(11), UINT64_C(0xCA));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(12), UINT64_C(0x53));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(13), UINT64_C(0xB6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(14), UINT64_C(0x00));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(15), UINT64_C(0xE5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(16), UINT64_C(0xC0));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(17), UINT64_C(0x03));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(18), UINT64_C(0xC7));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(19), UINT64_C(0xDC));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(20), UINT64_C(0xB2));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(21), UINT64_C(0x7D));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(22), UINT64_C(0x7E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(23), UINT64_C(0x92));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(24), UINT64_C(0x3C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(25), UINT64_C(0x23));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(26), UINT64_C(0xF7));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(27), UINT64_C(0x86));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(28), UINT64_C(0x01));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(29), UINT64_C(0x46));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(30), UINT64_C(0xD2));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE59, z3zE59, INT64_C(31), UINT64_C(0xC5));
  for (size_t z8zE813 = 0; z8zE813 < 32; ++z8zE813) {
    z3zE60.bytes[z8zE813] = (uint8_t)(z3zE59.data[z8zE813] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE59);
  zKECCAK_EMPTY = z3zE60;
let_end_157: ;
}
void kill_letbind_55(void) {
}

void create_letbind_56(void) {

  sail_fixed_bytes_32 z3zE62;
  zz5vecz8z5bv8z9 z3zE61;
  CREATE(zz5vecz8z5bv8z9)(&z3zE61);
  internal_vector_init_zz5vecz8z5bv8z9(&z3zE61, INT64_C(32));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(0), UINT64_C(0x21));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(1), UINT64_C(0xB4));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(2), UINT64_C(0x63));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(3), UINT64_C(0xE3));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(4), UINT64_C(0xB5));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(5), UINT64_C(0x2F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(6), UINT64_C(0x62));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(7), UINT64_C(0x01));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(8), UINT64_C(0xC0));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(9), UINT64_C(0xAD));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(10), UINT64_C(0x6C));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(11), UINT64_C(0x99));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(12), UINT64_C(0x1B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(13), UINT64_C(0xE0));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(14), UINT64_C(0x48));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(15), UINT64_C(0x5B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(16), UINT64_C(0x6E));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(17), UINT64_C(0xF8));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(18), UINT64_C(0xC0));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(19), UINT64_C(0x92));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(20), UINT64_C(0xE6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(21), UINT64_C(0x45));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(22), UINT64_C(0x83));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(23), UINT64_C(0xFF));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(24), UINT64_C(0xA6));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(25), UINT64_C(0x55));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(26), UINT64_C(0xCC));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(27), UINT64_C(0x1B));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(28), UINT64_C(0x17));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(29), UINT64_C(0x1F));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(30), UINT64_C(0xE8));
  internal_vector_update_zz5vecz8z5bv8z9(&z3zE61, z3zE61, INT64_C(31), UINT64_C(0x56));
  for (size_t z8zE814 = 0; z8zE814 < 32; ++z8zE814) {
    z3zE62.bytes[z8zE814] = (uint8_t)(z3zE61.data[z8zE814] & UINT64_C(0xff));
  }
  KILL(zz5vecz8z5bv8z9)(&z3zE61);
  zEMPTY_TRIE_ROOT = z3zE62;
let_end_158: ;
}
void kill_letbind_56(void) {
}

sail_fixed_bytes_32 zkeccak256_slice(struct zByteSliceFields zs)
{
  sail_fixed_bytes_32 z8zE99;
  zz5listz8z5unionz0zzBytesz9 z2zE3895;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3895);
  {
    struct zBytes z2zE3894;
    CREATE(zBytes)(&z2zE3894);
    zBytesSlice(&z2zE3894, zs);
    zconsz3z5unionz0zzBytes(&z2zE3895, z2zE3894, z2zE3895);
    KILL(zBytes)(&z2zE3894);
  }
  z8zE99 = host_keccak_segments(z2zE3895);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3895);
end_function_3757: ;
  return z8zE99;
end_block_exception_3758: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zsha256_slice(struct zByteSliceFields zs)
{
  sail_fixed_bytes_32 z8zE100;
  zz5listz8z5unionz0zzBytesz9 z2zE3893;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3893);
  {
    struct zBytes z2zE3892;
    CREATE(zBytes)(&z2zE3892);
    zBytesSlice(&z2zE3892, zs);
    zconsz3z5unionz0zzBytes(&z2zE3893, z2zE3892, z2zE3893);
    KILL(zBytes)(&z2zE3892);
  }
  z8zE100 = host_sha256_segments(z2zE3893);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3893);
end_function_3755: ;
  return z8zE100;
end_block_exception_3756: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zkeccak256_word(sail_u256 zw)
{
  sail_fixed_bytes_32 z8zE101;
  zz5listz8z5unionz0zzBytesz9 z2zE3891;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3891);
  {
    struct zBytes z2zE3890;
    CREATE(zBytes)(&z2zE3890);
    {
      zz5listz8z5bv8z9 z2zE3889;
      CREATE(zz5listz8z5bv8z9)(&z2zE3889);
      zword_to_bytes32(&z2zE3889, zw);
      zbytes_list(&z2zE3890, z2zE3889, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3889);
    }
    zconsz3z5unionz0zzBytes(&z2zE3891, z2zE3890, z2zE3891);
    KILL(zBytes)(&z2zE3890);
  }
  z8zE101 = host_keccak_segments(z2zE3891);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3891);
end_function_3753: ;
  return z8zE101;
end_block_exception_3754: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zkeccak256_address(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE102;
  zz5listz8z5unionz0zzBytesz9 z2zE3888;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3888);
  {
    struct zBytes z2zE3887;
    CREATE(zBytes)(&z2zE3887);
    {
      zz5listz8z5bv8z9 z2zE3886;
      CREATE(zz5listz8z5bv8z9)(&z2zE3886);
      zaddress_to_bytes(&z2zE3886, za);
      zbytes_list(&z2zE3887, z2zE3886, zADDRESS_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3886);
    }
    zconsz3z5unionz0zzBytes(&z2zE3888, z2zE3887, z2zE3888);
    KILL(zBytes)(&z2zE3887);
  }
  z8zE102 = host_keccak_segments(z2zE3888);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3888);
end_function_3751: ;
  return z8zE102;
end_block_exception_3752: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zsha256_pair(sail_fixed_bytes_32 za, sail_fixed_bytes_32 zb)
{
  sail_fixed_bytes_32 z8zE103;
  zz5listz8z5unionz0zzBytesz9 z2zE3885;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3885);
  {
    struct zBytes z2zE3883;
    CREATE(zBytes)(&z2zE3883);
    {
      zz5listz8z5bv8z9 z2zE3881;
      CREATE(zz5listz8z5bv8z9)(&z2zE3881);
      zhash_to_bytes32(&z2zE3881, za);
      zbytes_list(&z2zE3883, z2zE3881, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3881);
    }
    struct zBytes z2zE3884;
    CREATE(zBytes)(&z2zE3884);
    {
      zz5listz8z5bv8z9 z2zE3882;
      CREATE(zz5listz8z5bv8z9)(&z2zE3882);
      zhash_to_bytes32(&z2zE3882, zb);
      zbytes_list(&z2zE3884, z2zE3882, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3882);
    }
    zconsz3z5unionz0zzBytes(&z2zE3885, z2zE3884, z2zE3885);
    zconsz3z5unionz0zzBytes(&z2zE3885, z2zE3883, z2zE3885);
    KILL(zBytes)(&z2zE3884);
    KILL(zBytes)(&z2zE3883);
  }
  z8zE103 = host_sha256_segments(z2zE3885);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3885);
end_function_3749: ;
  return z8zE103;
end_block_exception_3750: ;

  return fixed_bytes_32_zero();
}

void create_letbind_57(void) {

  sail_u256 z3zE63;
  z3zE63 = ((sail_u256){{UINT64_C(13822214165235122497), UINT64_C(13451932020343611451), UINT64_C(18446744073709551614), UINT64_C(18446744073709551615)}});
  zSECP_N_FULL = z3zE63;
let_end_164: ;
}
void kill_letbind_57(void) {
}

void create_letbind_58(void) {

  sail_u256 z3zE64;
  z3zE64 = ((sail_u256){{UINT64_C(16134479119472337056), UINT64_C(6725966010171805725), UINT64_C(18446744073709551615), UINT64_C(9223372036854775807)}});
  zSECP_N_HALF = z3zE64;
let_end_165: ;
}
void kill_letbind_58(void) {
}

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zecrecover_addr(sail_fixed_bytes_32 zh, uint64_t zyparity, sail_u256 zr, sail_u256 zs)
{
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE104;
  struct zAddressResult zrecovered;
  zrecovered = precompile_ecrecover_hash_sig(zh, zyparity, zr, zs);
  bool z2zE3879;
  z2zE3879 = zrecovered.zsuccess;
  sail_fixed_bytes_20 z2zE3880;
  z2zE3880 = zrecovered.zaddress;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3653;
  z3zE3653.ztup0 = z2zE3879;
  z3zE3653.ztup1 = z2zE3880;
  z8zE104 = z3zE3653;
end_function_3747: ;
  return z8zE104;
end_block_exception_3748: ;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE815 = { .ztup0 = false, .ztup1 = fixed_bytes_20_zero() };
  return z8zE815;
}

