/* Generated from sail/host/kernel/accounts.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
bool zaccount_info_changed(struct zAccountInfo zc, struct zAccountInfo zo)
{
  bool z8zE250;
  bool z2zE3087;
  {
    uint64_t z2zE3077;
    z2zE3077 = zc.znonce;
    uint64_t z2zE3078;
    z2zE3078 = zo.znonce;
    z2zE3087 = (z2zE3077 != z2zE3078);
  }
  bool z3zE3063;
  if (z2zE3087) {  z3zE3063 = true;  } else {
    bool z2zE3086;
    {
      sail_u256 z2zE3079;
      z2zE3079 = zc.zbalance;
      sail_u256 z2zE3080;
      z2zE3080 = zo.zbalance;
      z2zE3086 = (!eq_u256(z2zE3079, z2zE3080));
    }
    bool z3zE3062;
    if (z2zE3086) {  z3zE3062 = true;  } else {
      bool z2zE3085;
      {
        sail_fixed_bytes_32 z2zE3081;
        z2zE3081 = zc.zcode_hash;
        sail_fixed_bytes_32 z2zE3082;
        z2zE3082 = zo.zcode_hash;
        z2zE3085 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3081, z2zE3082);
      }
      bool z3zE3061;
      if (z2zE3085) {  z3zE3061 = true;  } else {
        sail_fixed_bytes_32 z2zE3083;
        z2zE3083 = zc.zstorage_root;
        sail_fixed_bytes_32 z2zE3084;
        z2zE3084 = zo.zstorage_root;
        z3zE3061 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3083, z2zE3084);
      }
      z3zE3062 = z3zE3061;
    }
    z3zE3063 = z3zE3062;
  }
  z8zE250 = z3zE3063;
end_function_3106: ;
  return z8zE250;
end_block_exception_3107: ;

  return false;
}

bool zaccount_info_empty(struct zAccountInfo zinfo)
{
  bool z8zE251;
  bool z2zE3076;
  {
    sail_fixed_bytes_32 z2zE3072;
    z2zE3072 = zinfo.zcode_hash;
    z2zE3076 = eq_fixed_bytes_32(z2zE3072, zKECCAK_EMPTY);
  }
  bool z3zE3060;
  if (z2zE3076) {
    bool z2zE3075;
    {
      uint64_t z2zE3073;
      z2zE3073 = zinfo.znonce;
      z2zE3075 = (z2zE3073 == UINT64_C(0));
    }
    bool z3zE3059;
    if (z2zE3075) {
      sail_u256 z2zE3074;
      z2zE3074 = zinfo.zbalance;
      z3zE3059 = zword_is_zzero(z2zE3074);
    } else {  z3zE3059 = false;  }
    z3zE3060 = z3zE3059;
  } else {  z3zE3060 = false;  }
  z8zE251 = z3zE3060;
end_function_3104: ;
  return z8zE251;
end_block_exception_3105: ;

  return false;
}

bool zaccount_changed(struct zAccount zc, struct zAccount zo)
{
  bool z8zE252;
  bool z2zE3071;
  {
    struct zAccountInfo z2zE3064;
    z2zE3064 = zc.zinfo;
    struct zAccountInfo z2zE3065;
    z2zE3065 = zo.zinfo;
    z2zE3071 = zaccount_info_changed(z2zE3064, z2zE3065);
  }
  bool z3zE3058;
  if (z2zE3071) {  z3zE3058 = true;  } else {
    bool z2zE3070;
    {
      bool z2zE3066;
      z2zE3066 = zc.zpresent;
      bool z2zE3067;
      z2zE3067 = zo.zpresent;
      z2zE3070 = zneq_bool(z2zE3066, z2zE3067);
    }
    bool z3zE3057;
    if (z2zE3070) {  z3zE3057 = true;  } else {
      bool z2zE3068;
      z2zE3068 = zc.zstorage_cleared;
      bool z2zE3069;
      z2zE3069 = zo.zstorage_cleared;
      z3zE3057 = zneq_bool(z2zE3068, z2zE3069);
    }
    z3zE3058 = z3zE3057;
  }
  z8zE252 = z3zE3058;
end_function_3102: ;
  return z8zE252;
end_block_exception_3103: ;

  return false;
}

struct zAccount zaccount_set_info(struct zAccount zacc, struct zAccountInfo zinfo)
{
  struct zAccount z8zE253;
  bool z2zE3060;
  z2zE3060 = zaccount_info_empty(zinfo);
  if (z2zE3060) {
    struct zAccountInfo z2zE3063;
    {
      sail_fixed_bytes_32 z2zE3062;
      {
        struct zAccountInfo z2zE3061;
        z2zE3061 = zacc.zinfo;
        z2zE3062 = z2zE3061.zstorage_root;
      }
      struct zAccountInfo z3zE3055;
      z3zE3055 = zEMPTY_ACCOUNT_INFO;
      z3zE3055.zstorage_root = z2zE3062;
      z2zE3063 = z3zE3055;
    }
    struct zAccount z3zE3056;
    z3zE3056 = zacc;
    z3zE3056.zinfo = z2zE3063;
    z3zE3056.zpresent = false;
    z3zE3056.zstorage_cleared = true;
    z8zE253 = z3zE3056;
  } else {
    struct zAccount z3zE3054;
    z3zE3054 = zacc;
    z3zE3054.zinfo = zinfo;
    z3zE3054.zpresent = true;
    z8zE253 = z3zE3054;
  }
end_function_3100: ;
  return z8zE253;
end_block_exception_3101: ;
  struct zAccountInfo z8zE886 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE885 = { .zcreated = false, .zinfo = z8zE886, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE885;
}

struct zAccount zaccount_clear_storage(struct zAccount zacc)
{
  struct zAccount z8zE254;
  struct zAccount z3zE3053;
  z3zE3053 = zacc;
  z3zE3053.zstorage_cleared = true;
  z8zE254 = z3zE3053;
end_function_3098: ;
  return z8zE254;
end_block_exception_3099: ;
  struct zAccountInfo z8zE888 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE887 = { .zcreated = false, .zinfo = z8zE888, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE887;
}

struct zAccount zaccount_delete(struct zAccount zacc)
{
  struct zAccount z8zE255;
  struct zAccountInfo z2zE3059;
  {
    sail_fixed_bytes_32 z2zE3058;
    {
      struct zAccountInfo z2zE3057;
      z2zE3057 = zacc.zinfo;
      z2zE3058 = z2zE3057.zstorage_root;
    }
    struct zAccountInfo z3zE3051;
    z3zE3051 = zEMPTY_ACCOUNT_INFO;
    z3zE3051.zstorage_root = z2zE3058;
    z2zE3059 = z3zE3051;
  }
  struct zAccount z3zE3052;
  z3zE3052 = zacc;
  z3zE3052.zinfo = z2zE3059;
  z3zE3052.zpresent = false;
  z3zE3052.zstorage_cleared = true;
  z8zE255 = z3zE3052;
end_function_3096: ;
  return z8zE255;
end_block_exception_3097: ;
  struct zAccountInfo z8zE890 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE889 = { .zcreated = false, .zinfo = z8zE890, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE889;
}

struct zAccount zaccount_clear_preserving_balance(struct zAccount zacc)
{
  struct zAccount z8zE256;
  struct zAccount z2zE3055;
  z2zE3055 = zaccount_clear_storage(zacc);
  struct zAccountInfo z2zE3056;
  {
    struct zAccountInfo z2zE3054;
    z2zE3054 = zacc.zinfo;
    struct zAccountInfo z3zE3050;
    z3zE3050 = z2zE3054;
    z3zE3050.zcode_hash = zKECCAK_EMPTY;
    z3zE3050.znonce = UINT64_C(0);
    z2zE3056 = z3zE3050;
  }
  z8zE256 = zaccount_set_info(z2zE3055, z2zE3056);
end_function_3094: ;
  return z8zE256;
end_block_exception_3095: ;
  struct zAccountInfo z8zE892 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE891 = { .zcreated = false, .zinfo = z8zE892, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE891;
}

unit zstore_account(sail_fixed_bytes_20 za, struct zAccount zv)
{
  unit z8zE257;
  z8zE257 = acct_tx_update(za, zv);
end_function_3092: ;
  return z8zE257;
end_block_exception_3093: ;

  return UNIT;
}

unit zstore_account_info(sail_fixed_bytes_20 za, struct zAccount zacc, struct zAccountInfo zinfo)
{
  unit z8zE258;
  bool z2zE3021;
  z2zE3021 = zaccount_info_empty(zinfo);
  unit z3zE3045;
  if (z2zE3021) {  z3zE3045 = storage_tx_clear(za);  } else {  z3zE3045 = UNIT;  }
  struct zAccount znext;
  znext = zaccount_set_info(zacc, zinfo);
  bool z2zE3032;
  {
    bool z2zE3031;
    {
      sail_fixed_bytes_32 z2zE3024;
      {
        struct zAccountInfo z2zE3022;
        z2zE3022 = znext.zinfo;
        z2zE3024 = z2zE3022.zstorage_root;
      }
      sail_fixed_bytes_32 z2zE3025;
      {
        struct zAccountInfo z2zE3023;
        z2zE3023 = zacc.zinfo;
        z2zE3025 = z2zE3023.zstorage_root;
      }
      z2zE3031 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3024, z2zE3025);
    }
    bool z3zE3047;
    if (z2zE3031) {  z3zE3047 = true;  } else {
      bool z2zE3030;
      {
        bool z2zE3026;
        z2zE3026 = znext.zpresent;
        bool z2zE3027;
        z2zE3027 = zacc.zpresent;
        z2zE3030 = zneq_bool(z2zE3026, z2zE3027);
      }
      bool z3zE3046;
      if (z2zE3030) {  z3zE3046 = true;  } else {
        bool z2zE3028;
        z2zE3028 = znext.zstorage_cleared;
        bool z2zE3029;
        z2zE3029 = zacc.zstorage_cleared;
        z3zE3046 = zneq_bool(z2zE3028, z2zE3029);
      }
      z3zE3047 = z3zE3046;
    }
    z2zE3032 = z3zE3047;
  }
  if (z2zE3032) {  z8zE258 = zstore_account(za, znext);  } else {
    bool z2zE3037;
    {
      sail_u256 z2zE3035;
      {
        struct zAccountInfo z2zE3033;
        z2zE3033 = znext.zinfo;
        z2zE3035 = z2zE3033.zbalance;
      }
      sail_u256 z2zE3036;
      {
        struct zAccountInfo z2zE3034;
        z2zE3034 = zacc.zinfo;
        z2zE3036 = z2zE3034.zbalance;
      }
      z2zE3037 = (!eq_u256(z2zE3035, z2zE3036));
    }
    unit z3zE3049;
    if (z2zE3037) {
      sail_u256 z2zE3039;
      {
        struct zAccountInfo z2zE3038;
        z2zE3038 = znext.zinfo;
        z2zE3039 = z2zE3038.zbalance;
      }
      z3zE3049 = acct_tx_set_balance(za, z2zE3039);
    } else {  z3zE3049 = UNIT;  }
    bool z2zE3044;
    {
      uint64_t z2zE3042;
      {
        struct zAccountInfo z2zE3040;
        z2zE3040 = znext.zinfo;
        z2zE3042 = z2zE3040.znonce;
      }
      uint64_t z2zE3043;
      {
        struct zAccountInfo z2zE3041;
        z2zE3041 = zacc.zinfo;
        z2zE3043 = z2zE3041.znonce;
      }
      z2zE3044 = (z2zE3042 != z2zE3043);
    }
    unit z3zE3048;
    if (z2zE3044) {
      uint64_t z2zE3046;
      {
        struct zAccountInfo z2zE3045;
        z2zE3045 = znext.zinfo;
        z2zE3046 = z2zE3045.znonce;
      }
      z3zE3048 = acct_tx_set_nonce(za, z2zE3046);
    } else {  z3zE3048 = UNIT;  }
    bool z2zE3051;
    {
      sail_fixed_bytes_32 z2zE3049;
      {
        struct zAccountInfo z2zE3047;
        z2zE3047 = znext.zinfo;
        z2zE3049 = z2zE3047.zcode_hash;
      }
      sail_fixed_bytes_32 z2zE3050;
      {
        struct zAccountInfo z2zE3048;
        z2zE3048 = zacc.zinfo;
        z2zE3050 = z2zE3048.zcode_hash;
      }
      z2zE3051 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3049, z2zE3050);
    }
    if (z2zE3051) {
      sail_fixed_bytes_32 z2zE3053;
      {
        struct zAccountInfo z2zE3052;
        z2zE3052 = znext.zinfo;
        z2zE3053 = z2zE3052.zcode_hash;
      }
      z8zE258 = acct_tx_set_code_hash(za, z2zE3053);
    } else {  z8zE258 = UNIT;  }
  }
end_function_3090: ;
  return z8zE258;
end_block_exception_3091: ;

  return UNIT;
}

sail_u256 zk_get_balance(sail_fixed_bytes_20 za)
{
  sail_u256 z8zE259;
  struct zAccountInfo z2zE3020;
  {
    struct zAccount z2zE3019;
    {
      z2zE3019 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3089;  }
    }
    z2zE3020 = z2zE3019.zinfo;
  }
  z8zE259 = z2zE3020.zbalance;
end_function_3088: ;
  return z8zE259;
end_block_exception_3089: ;

  return u256_zero();
}

uint64_t zk_get_nonce(sail_fixed_bytes_20 za)
{
  uint64_t z8zE260;
  struct zAccountInfo z2zE3018;
  {
    struct zAccount z2zE3017;
    {
      z2zE3017 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3087;  }
    }
    z2zE3018 = z2zE3017.zinfo;
  }
  z8zE260 = z2zE3018.znonce;
end_function_3086: ;
  return z8zE260;
end_block_exception_3087: ;

  return UINT64_C(0xdeadc0de);
}

bool zk_account_exists(sail_fixed_bytes_20 za)
{
  bool z8zE261;
  struct zAccount z2zE3016;
  {
    z2zE3016 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3085;  }
  }
  z8zE261 = z2zE3016.zpresent;
end_function_3084: ;
  return z8zE261;
end_block_exception_3085: ;

  return false;
}

bool zk_account_is_empty(sail_fixed_bytes_20 za)
{
  bool z8zE262;
  struct zAccountInfo z2zE3015;
  {
    struct zAccount z2zE3014;
    {
      z2zE3014 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3083;  }
    }
    z2zE3015 = z2zE3014.zinfo;
  }
  z8zE262 = zaccount_info_empty(z2zE3015);
end_function_3082: ;
  return z8zE262;
end_block_exception_3083: ;

  return false;
}

bool zk_account_occupied(sail_fixed_bytes_20 za)
{
  bool z8zE263;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3081;  }
  }
  struct zAccountInfo zinfo;
  zinfo = zacc.zinfo;
  bool zanchored_storage;
  {
    bool z2zE3013;
    {
      bool z2zE3011;
      z2zE3011 = zacc.zstorage_cleared;
      z2zE3013 = not(z2zE3011);
    }
    bool z3zE3042;
    if (z2zE3013) {
      sail_fixed_bytes_32 z2zE3012;
      z2zE3012 = zinfo.zstorage_root;
      z3zE3042 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3012, zEMPTY_TRIE_ROOT);
    } else {  z3zE3042 = false;  }
    zanchored_storage = z3zE3042;
  }
  bool z2zE3010;
  {
    bool z2zE3009;
    {
      sail_fixed_bytes_32 z2zE3006;
      z2zE3006 = zinfo.zcode_hash;
      z2zE3009 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3006, zKECCAK_EMPTY);
    }
    bool z3zE3044;
    if (z2zE3009) {  z3zE3044 = true;  } else {
      bool z2zE3008;
      {
        uint64_t z2zE3007;
        z2zE3007 = zinfo.znonce;
        z2zE3008 = (z2zE3007 != UINT64_C(0));
      }
      bool z3zE3043;
      if (z2zE3008) {  z3zE3043 = true;  } else {  z3zE3043 = zanchored_storage;  }
      z3zE3044 = z3zE3043;
    }
    z2zE3010 = z3zE3044;
  }
  if (z2zE3010) {  z8zE263 = true;  } else {  z8zE263 = storage_has_writes(za);  }
end_function_3080: ;
  return z8zE263;
end_block_exception_3081: ;

  return false;
}

unit zk_transfer(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE264;
  struct zAccount zsrc_acc;
  {
    zsrc_acc = zk_aload(zsrc);
    if (have_exception) {  goto end_block_exception_3079;  }
  }
  struct zAccount zdst_acc;
  {
    zdst_acc = zk_aload(zdst);
    if (have_exception) {  goto end_block_exception_3079;  }
  }
  bool z2zE2995;
  {
    bool z2zE2994;
    z2zE2994 = zword_is_zzero(zv);
    bool z3zE3037;
    if (z2zE2994) {  z3zE3037 = true;  } else {  z3zE3037 = eq_fixed_bytes_20(zsrc, zdst);  }
    z2zE2995 = z3zE3037;
  }
  if (z2zE2995) {  z8zE264 = UNIT;  } else {
    struct zAccountInfo z2zE3000;
    {
      struct zAccountInfo z2zE2996;
      z2zE2996 = zsrc_acc.zinfo;
      sail_u256 z2zE2999;
      {
        sail_u256 z2zE2998;
        {
          struct zAccountInfo z2zE2997;
          z2zE2997 = zsrc_acc.zinfo;
          z2zE2998 = z2zE2997.zbalance;
        }
        z2zE2999 = zalu_sub(z2zE2998, zv);
      }
      struct zAccountInfo z3zE3038;
      z3zE3038 = z2zE2996;
      z3zE3038.zbalance = z2zE2999;
      z2zE3000 = z3zE3038;
    }
    unit z3zE3041;
    z3zE3041 = zstore_account_info(zsrc, zsrc_acc, z2zE3000);
    struct zAccountInfo z2zE3005;
    {
      struct zAccountInfo z2zE3001;
      z2zE3001 = zdst_acc.zinfo;
      sail_u256 z2zE3004;
      {
        sail_u256 z2zE3003;
        {
          struct zAccountInfo z2zE3002;
          z2zE3002 = zdst_acc.zinfo;
          z2zE3003 = z2zE3002.zbalance;
        }
        z2zE3004 = zalu_add(z2zE3003, zv);
      }
      struct zAccountInfo z3zE3039;
      z3zE3039 = z2zE3001;
      z3zE3039.zbalance = z2zE3004;
      z2zE3005 = z3zE3039;
    }
    unit z3zE3040;
    z3zE3040 = zstore_account_info(zdst, zdst_acc, z2zE3005);
    z8zE264 = zk_emit_transfer_log(zsrc, zdst, zv);
  }
end_function_3078: ;
  return z8zE264;
end_block_exception_3079: ;

  return UNIT;
}

unit zk_bump_nonce(sail_fixed_bytes_20 za)
{
  unit z8zE265;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3077;  }
  }
  struct zAccountInfo z2zE2993;
  {
    struct zAccountInfo z2zE2989;
    z2zE2989 = zcur.zinfo;
    uint64_t z2zE2992;
    {
      uint64_t z2zE2991;
      {
        struct zAccountInfo z2zE2990;
        z2zE2990 = zcur.zinfo;
        z2zE2991 = z2zE2990.znonce;
      }
      z2zE2992 = zaccount_nonce_increment(z2zE2991);
    }
    struct zAccountInfo z3zE3036;
    z3zE3036 = z2zE2989;
    z3zE3036.znonce = z2zE2992;
    z2zE2993 = z3zE3036;
  }
  z8zE265 = zstore_account_info(za, zcur, z2zE2993);
end_function_3076: ;
  return z8zE265;
end_block_exception_3077: ;

  return UNIT;
}

unit zk_add_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE266;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3075;  }
  }
  bool z2zE2983;
  {
    bool z2zE2982;
    z2zE2982 = zword_is_zzero(zv);
    z2zE2983 = not(z2zE2982);
  }
  if (z2zE2983) {
    struct zAccountInfo z2zE2988;
    {
      struct zAccountInfo z2zE2984;
      z2zE2984 = zcur.zinfo;
      sail_u256 z2zE2987;
      {
        sail_u256 z2zE2986;
        {
          struct zAccountInfo z2zE2985;
          z2zE2985 = zcur.zinfo;
          z2zE2986 = z2zE2985.zbalance;
        }
        z2zE2987 = zalu_add(z2zE2986, zv);
      }
      struct zAccountInfo z3zE3035;
      z3zE3035 = z2zE2984;
      z3zE3035.zbalance = z2zE2987;
      z2zE2988 = z3zE3035;
    }
    z8zE266 = zstore_account_info(za, zcur, z2zE2988);
  } else {  z8zE266 = UNIT;  }
end_function_3074: ;
  return z8zE266;
end_block_exception_3075: ;

  return UNIT;
}

unit zk_sub_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE267;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3073;  }
  }
  bool z2zE2976;
  {
    bool z2zE2975;
    z2zE2975 = zword_is_zzero(zv);
    z2zE2976 = not(z2zE2975);
  }
  if (z2zE2976) {
    struct zAccountInfo z2zE2981;
    {
      struct zAccountInfo z2zE2977;
      z2zE2977 = zcur.zinfo;
      sail_u256 z2zE2980;
      {
        sail_u256 z2zE2979;
        {
          struct zAccountInfo z2zE2978;
          z2zE2978 = zcur.zinfo;
          z2zE2979 = z2zE2978.zbalance;
        }
        z2zE2980 = zalu_sub(z2zE2979, zv);
      }
      struct zAccountInfo z3zE3034;
      z3zE3034 = z2zE2977;
      z3zE3034.zbalance = z2zE2980;
      z2zE2981 = z3zE3034;
    }
    z8zE267 = zstore_account_info(za, zcur, z2zE2981);
  } else {  z8zE267 = UNIT;  }
end_function_3072: ;
  return z8zE267;
end_block_exception_3073: ;

  return UNIT;
}

unit zk_clear_storage(sail_fixed_bytes_20 za)
{
  unit z8zE268;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3071;  }
  }
  unit z3zE3033;
  z3zE3033 = storage_tx_clear(za);
  struct zAccount z2zE2974;
  z2zE2974 = zaccount_clear_storage(zcur);
  z8zE268 = zstore_account(za, z2zE2974);
end_function_3070: ;
  return z8zE268;
end_block_exception_3071: ;

  return UNIT;
}

