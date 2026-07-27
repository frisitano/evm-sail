/* Generated from sail/primitives/crypto.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_79(void) {

  sail_fixed_bytes_32 z3zE83;
  z3zE83 = evmsail_word_to_hash(((sail_u256){{UINT64_C(8933690324103308400), UINT64_C(16501389505681565499), UINT64_C(10556012783764702144), UINT64_C(14254532742954296124)}}));
  zKECCAK_EMPTY = z3zE83;
let_end_182: ;
}
void kill_letbind_79(void) {
}

void create_letbind_80(void) {

  sail_fixed_bytes_32 z3zE84;
  z3zE84 = evmsail_word_to_hash(((sail_u256){{UINT64_C(99694600006120481), UINT64_C(6577753664917384640), UINT64_C(18411636558227634286), UINT64_C(6262289465969759654)}}));
  zEMPTY_TRIE_ROOT = z3zE84;
let_end_183: ;
}
void kill_letbind_80(void) {
}

sail_fixed_bytes_32 zkeccak256_slice(struct zByteSliceFields zs)
{
  sail_fixed_bytes_32 z8zE100;
  zz5listz8z5unionz0zzBytesz9 z2zE3185;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3185);
  {
    struct zBytes z2zE3184;
    CREATE(zBytes)(&z2zE3184);
    zBytesSlice(&z2zE3184, zs);
    zconsz3z5unionz0zzBytes(&z2zE3185, z2zE3184, z2zE3185);
    KILL(zBytes)(&z2zE3184);
  }
  z8zE100 = host_keccak_segments(z2zE3185);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3185);
end_function_3341: ;
  return z8zE100;
end_block_exception_3342: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zsha256_slice(struct zByteSliceFields zs)
{
  sail_fixed_bytes_32 z8zE101;
  zz5listz8z5unionz0zzBytesz9 z2zE3183;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3183);
  {
    struct zBytes z2zE3182;
    CREATE(zBytes)(&z2zE3182);
    zBytesSlice(&z2zE3182, zs);
    zconsz3z5unionz0zzBytes(&z2zE3183, z2zE3182, z2zE3183);
    KILL(zBytes)(&z2zE3182);
  }
  z8zE101 = host_sha256_segments(z2zE3183);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3183);
end_function_3339: ;
  return z8zE101;
end_block_exception_3340: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zkeccak256_word(sail_u256 zw)
{
  sail_fixed_bytes_32 z8zE102;
  zz5listz8z5unionz0zzBytesz9 z2zE3181;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3181);
  {
    struct zBytes z2zE3180;
    CREATE(zBytes)(&z2zE3180);
    {
      zz5listz8z5bv8z9 z2zE3179;
      CREATE(zz5listz8z5bv8z9)(&z2zE3179);
      zword_to_bytes32(&z2zE3179, zw);
      zbytes_list(&z2zE3180, z2zE3179, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3179);
    }
    zconsz3z5unionz0zzBytes(&z2zE3181, z2zE3180, z2zE3181);
    KILL(zBytes)(&z2zE3180);
  }
  z8zE102 = host_keccak_segments(z2zE3181);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3181);
end_function_3337: ;
  return z8zE102;
end_block_exception_3338: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zkeccak256_address(sail_fixed_bytes_20 za)
{
  sail_fixed_bytes_32 z8zE103;
  zz5listz8z5unionz0zzBytesz9 z2zE3178;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&z2zE3178);
  {
    struct zBytes z2zE3177;
    CREATE(zBytes)(&z2zE3177);
    {
      zz5listz8z5bv8z9 z2zE3176;
      CREATE(zz5listz8z5bv8z9)(&z2zE3176);
      zaddress_to_bytes(&z2zE3176, za);
      zbytes_list(&z2zE3177, z2zE3176, zADDRESS_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE3176);
    }
    zconsz3z5unionz0zzBytes(&z2zE3178, z2zE3177, z2zE3178);
    KILL(zBytes)(&z2zE3177);
  }
  z8zE103 = host_keccak_segments(z2zE3178);
  KILL(zz5listz8z5unionz0zzBytesz9)(&z2zE3178);
end_function_3335: ;
  return z8zE103;
end_block_exception_3336: ;

  return fixed_bytes_32_zero();
}

void create_letbind_81(void) {

  sail_u256 z3zE85;
  z3zE85 = ((sail_u256){{UINT64_C(13822214165235122497), UINT64_C(13451932020343611451), UINT64_C(18446744073709551614), UINT64_C(18446744073709551615)}});
  zSECP_N_FULL = z3zE85;
let_end_188: ;
}
void kill_letbind_81(void) {
}

void create_letbind_82(void) {

  sail_u256 z3zE86;
  z3zE86 = ((sail_u256){{UINT64_C(16134479119472337056), UINT64_C(6725966010171805725), UINT64_C(18446744073709551615), UINT64_C(9223372036854775807)}});
  zSECP_N_HALF = z3zE86;
let_end_189: ;
}
void kill_letbind_82(void) {
}

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zecrecover_addr(sail_fixed_bytes_32 zh, uint64_t zyparity, sail_u256 zr, sail_u256 zs)
{
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE104;
  struct zAddressResult zrecovered;
  zrecovered = precompile_ecrecover_hash_sig(zh, zyparity, zr, zs);
  bool z2zE3174;
  z2zE3174 = zrecovered.zsuccess;
  sail_fixed_bytes_20 z2zE3175;
  z2zE3175 = zrecovered.zaddress;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE2874;
  z3zE2874.ztup0 = z2zE3174;
  z3zE2874.ztup1 = z2zE3175;
  z8zE104 = z3zE2874;
end_function_3333: ;
  return z8zE104;
end_block_exception_3334: ;
  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z8zE777 = { .ztup0 = false, .ztup1 = fixed_bytes_20_zero() };
  return z8zE777;
}

