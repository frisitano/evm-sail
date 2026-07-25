/* Generated from sail/host/kernel/accounts.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
bool zaccount_info_changed(struct zAccountInfo zc, struct zAccountInfo zo)
{
  bool z8zE247;
  bool z2zE3064;
  {
    uint64_t z2zE3054;
    z2zE3054 = zc.znonce;
    uint64_t z2zE3055;
    z2zE3055 = zo.znonce;
    z2zE3064 = (z2zE3054 != z2zE3055);
  }
  bool z3zE3067;
  if (z2zE3064) {  z3zE3067 = true;  } else {
    bool z2zE3063;
    {
      sail_u256 z2zE3056;
      z2zE3056 = zc.zbalance;
      sail_u256 z2zE3057;
      z2zE3057 = zo.zbalance;
      z2zE3063 = (!eq_u256(z2zE3056, z2zE3057));
    }
    bool z3zE3066;
    if (z2zE3063) {  z3zE3066 = true;  } else {
      bool z2zE3062;
      {
        sail_fixed_bytes_32 z2zE3058;
        z2zE3058 = zc.zcode_hash;
        sail_fixed_bytes_32 z2zE3059;
        z2zE3059 = zo.zcode_hash;
        z2zE3062 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3058, z2zE3059);
      }
      bool z3zE3065;
      if (z2zE3062) {  z3zE3065 = true;  } else {
        sail_fixed_bytes_32 z2zE3060;
        z2zE3060 = zc.zstorage_root;
        sail_fixed_bytes_32 z2zE3061;
        z2zE3061 = zo.zstorage_root;
        z3zE3065 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3060, z2zE3061);
      }
      z3zE3066 = z3zE3065;
    }
    z3zE3067 = z3zE3066;
  }
  z8zE247 = z3zE3067;
end_function_3102: ;
  return z8zE247;
end_block_exception_3103: ;

  return false;
}

bool zaccount_info_empty(struct zAccountInfo zinfo)
{
  bool z8zE248;
  bool z2zE3053;
  {
    sail_fixed_bytes_32 z2zE3049;
    z2zE3049 = zinfo.zcode_hash;
    z2zE3053 = eq_fixed_bytes_32(z2zE3049, zKECCAK_EMPTY);
  }
  bool z3zE3064;
  if (z2zE3053) {
    bool z2zE3052;
    {
      uint64_t z2zE3050;
      z2zE3050 = zinfo.znonce;
      z2zE3052 = (z2zE3050 == UINT64_C(0));
    }
    bool z3zE3063;
    if (z2zE3052) {
      sail_u256 z2zE3051;
      z2zE3051 = zinfo.zbalance;
      z3zE3063 = zword_is_zzero(z2zE3051);
    } else {  z3zE3063 = false;  }
    z3zE3064 = z3zE3063;
  } else {  z3zE3064 = false;  }
  z8zE248 = z3zE3064;
end_function_3100: ;
  return z8zE248;
end_block_exception_3101: ;

  return false;
}

bool zaccount_changed(struct zAccount zc, struct zAccount zo)
{
  bool z8zE249;
  bool z2zE3048;
  {
    struct zAccountInfo z2zE3041;
    z2zE3041 = zc.zinfo;
    struct zAccountInfo z2zE3042;
    z2zE3042 = zo.zinfo;
    z2zE3048 = zaccount_info_changed(z2zE3041, z2zE3042);
  }
  bool z3zE3062;
  if (z2zE3048) {  z3zE3062 = true;  } else {
    bool z2zE3047;
    {
      bool z2zE3043;
      z2zE3043 = zc.zpresent;
      bool z2zE3044;
      z2zE3044 = zo.zpresent;
      z2zE3047 = zneq_bool(z2zE3043, z2zE3044);
    }
    bool z3zE3061;
    if (z2zE3047) {  z3zE3061 = true;  } else {
      bool z2zE3045;
      z2zE3045 = zc.zstorage_cleared;
      bool z2zE3046;
      z2zE3046 = zo.zstorage_cleared;
      z3zE3061 = zneq_bool(z2zE3045, z2zE3046);
    }
    z3zE3062 = z3zE3061;
  }
  z8zE249 = z3zE3062;
end_function_3098: ;
  return z8zE249;
end_block_exception_3099: ;

  return false;
}

struct zAccount zaccount_set_info(struct zAccount zacc, struct zAccountInfo zinfo)
{
  struct zAccount z8zE250;
  bool z2zE3037;
  z2zE3037 = zaccount_info_empty(zinfo);
  if (z2zE3037) {
    struct zAccountInfo z2zE3040;
    {
      sail_fixed_bytes_32 z2zE3039;
      {
        struct zAccountInfo z2zE3038;
        z2zE3038 = zacc.zinfo;
        z2zE3039 = z2zE3038.zstorage_root;
      }
      struct zAccountInfo z3zE3059;
      z3zE3059 = zEMPTY_ACCOUNT_INFO;
      z3zE3059.zstorage_root = z2zE3039;
      z2zE3040 = z3zE3059;
    }
    struct zAccount z3zE3060;
    z3zE3060 = zacc;
    z3zE3060.zinfo = z2zE3040;
    z3zE3060.zpresent = false;
    z3zE3060.zstorage_cleared = true;
    z8zE250 = z3zE3060;
  } else {
    struct zAccount z3zE3058;
    z3zE3058 = zacc;
    z3zE3058.zinfo = zinfo;
    z3zE3058.zpresent = true;
    z8zE250 = z3zE3058;
  }
end_function_3096: ;
  return z8zE250;
end_block_exception_3097: ;
  struct zAccountInfo z8zE874 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE873 = { .zcreated = false, .zinfo = z8zE874, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE873;
}

struct zAccount zaccount_clear_storage(struct zAccount zacc)
{
  struct zAccount z8zE251;
  struct zAccount z3zE3057;
  z3zE3057 = zacc;
  z3zE3057.zstorage_cleared = true;
  z8zE251 = z3zE3057;
end_function_3094: ;
  return z8zE251;
end_block_exception_3095: ;
  struct zAccountInfo z8zE876 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE875 = { .zcreated = false, .zinfo = z8zE876, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE875;
}

struct zAccount zaccount_delete(struct zAccount zacc)
{
  struct zAccount z8zE252;
  struct zAccountInfo z2zE3036;
  {
    sail_fixed_bytes_32 z2zE3035;
    {
      struct zAccountInfo z2zE3034;
      z2zE3034 = zacc.zinfo;
      z2zE3035 = z2zE3034.zstorage_root;
    }
    struct zAccountInfo z3zE3055;
    z3zE3055 = zEMPTY_ACCOUNT_INFO;
    z3zE3055.zstorage_root = z2zE3035;
    z2zE3036 = z3zE3055;
  }
  struct zAccount z3zE3056;
  z3zE3056 = zacc;
  z3zE3056.zinfo = z2zE3036;
  z3zE3056.zpresent = false;
  z3zE3056.zstorage_cleared = true;
  z8zE252 = z3zE3056;
end_function_3092: ;
  return z8zE252;
end_block_exception_3093: ;
  struct zAccountInfo z8zE878 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE877 = { .zcreated = false, .zinfo = z8zE878, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE877;
}

struct zAccount zaccount_clear_preserving_balance(struct zAccount zacc)
{
  struct zAccount z8zE253;
  struct zAccount z2zE3032;
  z2zE3032 = zaccount_clear_storage(zacc);
  struct zAccountInfo z2zE3033;
  {
    struct zAccountInfo z2zE3031;
    z2zE3031 = zacc.zinfo;
    struct zAccountInfo z3zE3054;
    z3zE3054 = z2zE3031;
    z3zE3054.zcode_hash = zKECCAK_EMPTY;
    z3zE3054.znonce = UINT64_C(0);
    z2zE3033 = z3zE3054;
  }
  z8zE253 = zaccount_set_info(z2zE3032, z2zE3033);
end_function_3090: ;
  return z8zE253;
end_block_exception_3091: ;
  struct zAccountInfo z8zE880 = { .zbalance = u256_zero(), .zcode_hash = fixed_bytes_32_zero(), .znonce = UINT64_C(0xdeadc0de), .zstorage_root = fixed_bytes_32_zero() };
  struct zAccount z8zE879 = { .zcreated = false, .zinfo = z8zE880, .zpresent = false, .zselfdestructed = false, .zstorage_cleared = false };
  return z8zE879;
}

unit zstore_account(sail_fixed_bytes_20 za, struct zAccount zv)
{
  unit z8zE254;
  z8zE254 = acct_tx_update(za, zv);
end_function_3088: ;
  return z8zE254;
end_block_exception_3089: ;

  return UNIT;
}

unit zstore_account_info(sail_fixed_bytes_20 za, struct zAccount zacc, struct zAccountInfo zinfo)
{
  unit z8zE255;
  bool z2zE2998;
  z2zE2998 = zaccount_info_empty(zinfo);
  unit z3zE3049;
  if (z2zE2998) {  z3zE3049 = storage_tx_clear(za);  } else {  z3zE3049 = UNIT;  }
  struct zAccount znext;
  znext = zaccount_set_info(zacc, zinfo);
  bool z2zE3009;
  {
    bool z2zE3008;
    {
      sail_fixed_bytes_32 z2zE3001;
      {
        struct zAccountInfo z2zE2999;
        z2zE2999 = znext.zinfo;
        z2zE3001 = z2zE2999.zstorage_root;
      }
      sail_fixed_bytes_32 z2zE3002;
      {
        struct zAccountInfo z2zE3000;
        z2zE3000 = zacc.zinfo;
        z2zE3002 = z2zE3000.zstorage_root;
      }
      z2zE3008 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3001, z2zE3002);
    }
    bool z3zE3051;
    if (z2zE3008) {  z3zE3051 = true;  } else {
      bool z2zE3007;
      {
        bool z2zE3003;
        z2zE3003 = znext.zpresent;
        bool z2zE3004;
        z2zE3004 = zacc.zpresent;
        z2zE3007 = zneq_bool(z2zE3003, z2zE3004);
      }
      bool z3zE3050;
      if (z2zE3007) {  z3zE3050 = true;  } else {
        bool z2zE3005;
        z2zE3005 = znext.zstorage_cleared;
        bool z2zE3006;
        z2zE3006 = zacc.zstorage_cleared;
        z3zE3050 = zneq_bool(z2zE3005, z2zE3006);
      }
      z3zE3051 = z3zE3050;
    }
    z2zE3009 = z3zE3051;
  }
  if (z2zE3009) {  z8zE255 = zstore_account(za, znext);  } else {
    bool z2zE3014;
    {
      sail_u256 z2zE3012;
      {
        struct zAccountInfo z2zE3010;
        z2zE3010 = znext.zinfo;
        z2zE3012 = z2zE3010.zbalance;
      }
      sail_u256 z2zE3013;
      {
        struct zAccountInfo z2zE3011;
        z2zE3011 = zacc.zinfo;
        z2zE3013 = z2zE3011.zbalance;
      }
      z2zE3014 = (!eq_u256(z2zE3012, z2zE3013));
    }
    unit z3zE3053;
    if (z2zE3014) {
      sail_u256 z2zE3016;
      {
        struct zAccountInfo z2zE3015;
        z2zE3015 = znext.zinfo;
        z2zE3016 = z2zE3015.zbalance;
      }
      z3zE3053 = acct_tx_set_balance(za, z2zE3016);
    } else {  z3zE3053 = UNIT;  }
    bool z2zE3021;
    {
      uint64_t z2zE3019;
      {
        struct zAccountInfo z2zE3017;
        z2zE3017 = znext.zinfo;
        z2zE3019 = z2zE3017.znonce;
      }
      uint64_t z2zE3020;
      {
        struct zAccountInfo z2zE3018;
        z2zE3018 = zacc.zinfo;
        z2zE3020 = z2zE3018.znonce;
      }
      z2zE3021 = (z2zE3019 != z2zE3020);
    }
    unit z3zE3052;
    if (z2zE3021) {
      uint64_t z2zE3023;
      {
        struct zAccountInfo z2zE3022;
        z2zE3022 = znext.zinfo;
        z2zE3023 = z2zE3022.znonce;
      }
      z3zE3052 = acct_tx_set_nonce(za, z2zE3023);
    } else {  z3zE3052 = UNIT;  }
    bool z2zE3028;
    {
      sail_fixed_bytes_32 z2zE3026;
      {
        struct zAccountInfo z2zE3024;
        z2zE3024 = znext.zinfo;
        z2zE3026 = z2zE3024.zcode_hash;
      }
      sail_fixed_bytes_32 z2zE3027;
      {
        struct zAccountInfo z2zE3025;
        z2zE3025 = zacc.zinfo;
        z2zE3027 = z2zE3025.zcode_hash;
      }
      z2zE3028 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE3026, z2zE3027);
    }
    if (z2zE3028) {
      sail_fixed_bytes_32 z2zE3030;
      {
        struct zAccountInfo z2zE3029;
        z2zE3029 = znext.zinfo;
        z2zE3030 = z2zE3029.zcode_hash;
      }
      z8zE255 = acct_tx_set_code_hash(za, z2zE3030);
    } else {  z8zE255 = UNIT;  }
  }
end_function_3086: ;
  return z8zE255;
end_block_exception_3087: ;

  return UNIT;
}

sail_u256 zk_get_balance(sail_fixed_bytes_20 za)
{
  sail_u256 z8zE256;
  struct zAccountInfo z2zE2997;
  {
    struct zAccount z2zE2996;
    {
      z2zE2996 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3085;  }
    }
    z2zE2997 = z2zE2996.zinfo;
  }
  z8zE256 = z2zE2997.zbalance;
end_function_3084: ;
  return z8zE256;
end_block_exception_3085: ;

  return u256_zero();
}

uint64_t zk_get_nonce(sail_fixed_bytes_20 za)
{
  uint64_t z8zE257;
  struct zAccountInfo z2zE2995;
  {
    struct zAccount z2zE2994;
    {
      z2zE2994 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3083;  }
    }
    z2zE2995 = z2zE2994.zinfo;
  }
  z8zE257 = z2zE2995.znonce;
end_function_3082: ;
  return z8zE257;
end_block_exception_3083: ;

  return UINT64_C(0xdeadc0de);
}

bool zk_account_exists(sail_fixed_bytes_20 za)
{
  bool z8zE258;
  struct zAccount z2zE2993;
  {
    z2zE2993 = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3081;  }
  }
  z8zE258 = z2zE2993.zpresent;
end_function_3080: ;
  return z8zE258;
end_block_exception_3081: ;

  return false;
}

bool zk_account_is_empty(sail_fixed_bytes_20 za)
{
  bool z8zE259;
  struct zAccountInfo z2zE2992;
  {
    struct zAccount z2zE2991;
    {
      z2zE2991 = zk_aload(za);
      if (have_exception) {  goto end_block_exception_3079;  }
    }
    z2zE2992 = z2zE2991.zinfo;
  }
  z8zE259 = zaccount_info_empty(z2zE2992);
end_function_3078: ;
  return z8zE259;
end_block_exception_3079: ;

  return false;
}

bool zk_account_occupied(sail_fixed_bytes_20 za)
{
  bool z8zE260;
  struct zAccount zacc;
  {
    zacc = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3077;  }
  }
  struct zAccountInfo zinfo;
  zinfo = zacc.zinfo;
  bool zanchored_storage;
  {
    bool z2zE2990;
    {
      bool z2zE2988;
      z2zE2988 = zacc.zstorage_cleared;
      z2zE2990 = not(z2zE2988);
    }
    bool z3zE3046;
    if (z2zE2990) {
      sail_fixed_bytes_32 z2zE2989;
      z2zE2989 = zinfo.zstorage_root;
      z3zE3046 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2989, zEMPTY_TRIE_ROOT);
    } else {  z3zE3046 = false;  }
    zanchored_storage = z3zE3046;
  }
  bool z2zE2987;
  {
    bool z2zE2986;
    {
      sail_fixed_bytes_32 z2zE2983;
      z2zE2983 = zinfo.zcode_hash;
      z2zE2986 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2983, zKECCAK_EMPTY);
    }
    bool z3zE3048;
    if (z2zE2986) {  z3zE3048 = true;  } else {
      bool z2zE2985;
      {
        uint64_t z2zE2984;
        z2zE2984 = zinfo.znonce;
        z2zE2985 = (z2zE2984 != UINT64_C(0));
      }
      bool z3zE3047;
      if (z2zE2985) {  z3zE3047 = true;  } else {  z3zE3047 = zanchored_storage;  }
      z3zE3048 = z3zE3047;
    }
    z2zE2987 = z3zE3048;
  }
  if (z2zE2987) {  z8zE260 = true;  } else {  z8zE260 = storage_has_writes(za);  }
end_function_3076: ;
  return z8zE260;
end_block_exception_3077: ;

  return false;
}

unit zk_transfer(sail_fixed_bytes_20 zsrc, sail_fixed_bytes_20 zdst, sail_u256 zv)
{
  unit z8zE261;
  struct zAccount zsrc_acc;
  {
    zsrc_acc = zk_aload(zsrc);
    if (have_exception) {  goto end_block_exception_3075;  }
  }
  struct zAccount zdst_acc;
  {
    zdst_acc = zk_aload(zdst);
    if (have_exception) {  goto end_block_exception_3075;  }
  }
  bool z2zE2972;
  {
    bool z2zE2971;
    z2zE2971 = zword_is_zzero(zv);
    bool z3zE3041;
    if (z2zE2971) {  z3zE3041 = true;  } else {  z3zE3041 = eq_fixed_bytes_20(zsrc, zdst);  }
    z2zE2972 = z3zE3041;
  }
  if (z2zE2972) {  z8zE261 = UNIT;  } else {
    struct zAccountInfo z2zE2977;
    {
      struct zAccountInfo z2zE2973;
      z2zE2973 = zsrc_acc.zinfo;
      sail_u256 z2zE2976;
      {
        sail_u256 z2zE2975;
        {
          struct zAccountInfo z2zE2974;
          z2zE2974 = zsrc_acc.zinfo;
          z2zE2975 = z2zE2974.zbalance;
        }
        z2zE2976 = zalu_sub(z2zE2975, zv);
      }
      struct zAccountInfo z3zE3042;
      z3zE3042 = z2zE2973;
      z3zE3042.zbalance = z2zE2976;
      z2zE2977 = z3zE3042;
    }
    unit z3zE3045;
    z3zE3045 = zstore_account_info(zsrc, zsrc_acc, z2zE2977);
    struct zAccountInfo z2zE2982;
    {
      struct zAccountInfo z2zE2978;
      z2zE2978 = zdst_acc.zinfo;
      sail_u256 z2zE2981;
      {
        sail_u256 z2zE2980;
        {
          struct zAccountInfo z2zE2979;
          z2zE2979 = zdst_acc.zinfo;
          z2zE2980 = z2zE2979.zbalance;
        }
        z2zE2981 = zalu_add(z2zE2980, zv);
      }
      struct zAccountInfo z3zE3043;
      z3zE3043 = z2zE2978;
      z3zE3043.zbalance = z2zE2981;
      z2zE2982 = z3zE3043;
    }
    unit z3zE3044;
    z3zE3044 = zstore_account_info(zdst, zdst_acc, z2zE2982);
    z8zE261 = zk_emit_transfer_log(zsrc, zdst, zv);
  }
end_function_3074: ;
  return z8zE261;
end_block_exception_3075: ;

  return UNIT;
}

unit zk_bump_nonce(sail_fixed_bytes_20 za)
{
  unit z8zE262;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3073;  }
  }
  struct zAccountInfo z2zE2970;
  {
    struct zAccountInfo z2zE2966;
    z2zE2966 = zcur.zinfo;
    uint64_t z2zE2969;
    {
      uint64_t z2zE2968;
      {
        struct zAccountInfo z2zE2967;
        z2zE2967 = zcur.zinfo;
        z2zE2968 = z2zE2967.znonce;
      }
      z2zE2969 = zaccount_nonce_increment(z2zE2968);
    }
    struct zAccountInfo z3zE3040;
    z3zE3040 = z2zE2966;
    z3zE3040.znonce = z2zE2969;
    z2zE2970 = z3zE3040;
  }
  z8zE262 = zstore_account_info(za, zcur, z2zE2970);
end_function_3072: ;
  return z8zE262;
end_block_exception_3073: ;

  return UNIT;
}

unit zk_add_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE263;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3071;  }
  }
  bool z2zE2960;
  {
    bool z2zE2959;
    z2zE2959 = zword_is_zzero(zv);
    z2zE2960 = not(z2zE2959);
  }
  if (z2zE2960) {
    struct zAccountInfo z2zE2965;
    {
      struct zAccountInfo z2zE2961;
      z2zE2961 = zcur.zinfo;
      sail_u256 z2zE2964;
      {
        sail_u256 z2zE2963;
        {
          struct zAccountInfo z2zE2962;
          z2zE2962 = zcur.zinfo;
          z2zE2963 = z2zE2962.zbalance;
        }
        z2zE2964 = zalu_add(z2zE2963, zv);
      }
      struct zAccountInfo z3zE3039;
      z3zE3039 = z2zE2961;
      z3zE3039.zbalance = z2zE2964;
      z2zE2965 = z3zE3039;
    }
    z8zE263 = zstore_account_info(za, zcur, z2zE2965);
  } else {  z8zE263 = UNIT;  }
end_function_3070: ;
  return z8zE263;
end_block_exception_3071: ;

  return UNIT;
}

unit zk_sub_balance(sail_fixed_bytes_20 za, sail_u256 zv)
{
  unit z8zE264;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3069;  }
  }
  bool z2zE2953;
  {
    bool z2zE2952;
    z2zE2952 = zword_is_zzero(zv);
    z2zE2953 = not(z2zE2952);
  }
  if (z2zE2953) {
    struct zAccountInfo z2zE2958;
    {
      struct zAccountInfo z2zE2954;
      z2zE2954 = zcur.zinfo;
      sail_u256 z2zE2957;
      {
        sail_u256 z2zE2956;
        {
          struct zAccountInfo z2zE2955;
          z2zE2955 = zcur.zinfo;
          z2zE2956 = z2zE2955.zbalance;
        }
        z2zE2957 = zalu_sub(z2zE2956, zv);
      }
      struct zAccountInfo z3zE3038;
      z3zE3038 = z2zE2954;
      z3zE3038.zbalance = z2zE2957;
      z2zE2958 = z3zE3038;
    }
    z8zE264 = zstore_account_info(za, zcur, z2zE2958);
  } else {  z8zE264 = UNIT;  }
end_function_3068: ;
  return z8zE264;
end_block_exception_3069: ;

  return UNIT;
}

unit zk_clear_storage(sail_fixed_bytes_20 za)
{
  unit z8zE265;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3067;  }
  }
  unit z3zE3037;
  z3zE3037 = storage_tx_clear(za);
  struct zAccount z2zE2951;
  z2zE2951 = zaccount_clear_storage(zcur);
  z8zE265 = zstore_account(za, z2zE2951);
end_function_3066: ;
  return z8zE265;
end_block_exception_3067: ;

  return UNIT;
}

