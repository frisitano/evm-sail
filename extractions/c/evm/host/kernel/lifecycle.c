/* Generated from sail/host/kernel/lifecycle.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zk_state_checkpoint(unit z3zE3018)
{
  uint64_t z8zE285;
  z8zE285 = host_state_checkpoint(UNIT);
end_function_3034: ;
  return z8zE285;
end_block_exception_3035: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_set_header(struct zBlockHeader zh)
{
  unit z8zE286;
  zk_header = zh;
  z8zE286 = UNIT;
end_function_3032: ;
  return z8zE286;
end_block_exception_3033: ;

  return UNIT;
}

unit zk_set_tx(struct zTxEnv zenv)
{
  unit z8zE287;
  zk_tx = zenv;
  z8zE287 = UNIT;
end_function_3030: ;
  return z8zE287;
end_block_exception_3031: ;

  return UNIT;
}

unit zk_tx_reset(unit z3zE3012)
{
  unit z8zE288;
  unit z3zE3017;
  z3zE3017 = acct_tx_reset(UNIT);
  unit z3zE3016;
  z3zE3016 = storage_tx_reset(UNIT);
  unit z3zE3015;
  z3zE3015 = warm_reset(UNIT);
  unit z3zE3014;
  z3zE3014 = transient_storage_reset(UNIT);
  unit z3zE3013;
  z3zE3013 = logs_tx_reset(UNIT);
  z8zE288 = host_state_checkpoint_reset(UNIT);
end_function_3028: ;
  return z8zE288;
end_block_exception_3029: ;

  return UNIT;
}

bool zaccount_deleted_at_tx_end(struct zAccount zacc)
{
  bool z8zE289;
  bool z2zE2940;
  z2zE2940 = zacc.zselfdestructed;
  bool z3zE3011;
  if (z2zE2940) {
    bool z2zE2939;
    z2zE2939 = zfork_lt(zk_fork, zCancun);
    bool z3zE3010;
    if (z2zE2939) {  z3zE3010 = true;  } else {  z3zE3010 = zacc.zcreated;  }
    z3zE3011 = z3zE3010;
  } else {  z3zE3011 = false;  }
  z8zE289 = z3zE3011;
end_function_3026: ;
  return z8zE289;
end_block_exception_3027: ;

  return false;
}

unit zk_tx_merge(unit z3zE2978)
{
  unit z8zE290;
  bool zmore;
  zmore = true;
  bool z3zE2993;
  unit z3zE2994;
while_3011: ;
  {
    z3zE2993 = zmore;
    if (!(z3zE2993)) goto wend_3012;
    struct zoptionzIRAcctEntryzK z2zE2875;
    CREATE(zoptionzIRAcctEntryzK)(&z2zE2875);
    acct_tx_pop_ascending(&z2zE2875, UNIT);
    unit z3zE2979;
    {
      if (z2zE2875.kind != Kind_zSomezIRAcctEntryzK) goto case_3015;
      struct zAcctEntry ze;
      ze = z2zE2875.variants.zSomezIRAcctEntryzK;
      struct zAccount zcurr;
      {
        struct zAcctValue z2zE2922;
        z2zE2922 = ze.zvalue;
        zcurr = z2zE2922.zcurr;
      }
      bool zdeleted;
      zdeleted = zaccount_deleted_at_tx_end(zcurr);
      unit z3zE2988;
      if (zdeleted) {
        bool z2zE2876;
        z2zE2876 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE2989;
        if (z2zE2876) {
          zcurr = zaccount_clear_preserving_balance(zcurr);
          z3zE2989 = UNIT;
        } else {
          zcurr = zaccount_delete(zcurr);
          z3zE2989 = UNIT;
        }
        sail_fixed_bytes_20 z2zE2877;
        z2zE2877 = ze.zaddr;
        z3zE2988 = storage_tx_clear(z2zE2877);
      } else {  z3zE2988 = UNIT;  }
      bool z2zE2882;
      {
        bool z3zE2982;
        if (zdeleted) {  z3zE2982 = true;  } else {
          bool z2zE2881;
          z2zE2881 = zcurr.zstorage_cleared;
          bool z3zE2981;
          if (z2zE2881) {
            bool z2zE2880;
            {
              struct zAccount z2zE2879;
              {
                struct zAcctValue z2zE2878;
                z2zE2878 = ze.zvalue;
                z2zE2879 = z2zE2878.zorig;
              }
              z2zE2880 = z2zE2879.zstorage_cleared;
            }
            z3zE2981 = not(z2zE2880);
          } else {  z3zE2981 = false;  }
          z3zE2982 = z3zE2981;
        }
        z2zE2882 = z3zE2982;
      }
      unit z3zE2987;
      if (z2zE2882) {
        sail_fixed_bytes_20 z2zE2883;
        z2zE2883 = ze.zaddr;
        z3zE2987 = storage_block_clear(z2zE2883);
      } else {  z3zE2987 = UNIT;  }
      bool z2zE2890;
      {
        uint64_t z2zE2888;
        {
          struct zAccountInfo z2zE2884;
          z2zE2884 = zcurr.zinfo;
          z2zE2888 = z2zE2884.znonce;
        }
        uint64_t z2zE2889;
        {
          struct zAccountInfo z2zE2887;
          {
            struct zAccount z2zE2886;
            {
              struct zAcctValue z2zE2885;
              z2zE2885 = ze.zvalue;
              z2zE2886 = z2zE2885.zorig;
            }
            z2zE2887 = z2zE2886.zinfo;
          }
          z2zE2889 = z2zE2887.znonce;
        }
        z2zE2890 = (z2zE2888 != z2zE2889);
      }
      unit z3zE2986;
      if (z2zE2890) {
        sail_fixed_bytes_20 z2zE2892;
        z2zE2892 = ze.zaddr;
        uint64_t z2zE2893;
        {
          struct zAccountInfo z2zE2891;
          z2zE2891 = zcurr.zinfo;
          z2zE2893 = z2zE2891.znonce;
        }
        z3zE2986 = bal_note_nonce_change(z2zE2892, z2zE2893);
      } else {  z3zE2986 = UNIT;  }
      bool z2zE2900;
      {
        sail_u256 z2zE2898;
        {
          struct zAccountInfo z2zE2894;
          z2zE2894 = zcurr.zinfo;
          z2zE2898 = z2zE2894.zbalance;
        }
        sail_u256 z2zE2899;
        {
          struct zAccountInfo z2zE2897;
          {
            struct zAccount z2zE2896;
            {
              struct zAcctValue z2zE2895;
              z2zE2895 = ze.zvalue;
              z2zE2896 = z2zE2895.zorig;
            }
            z2zE2897 = z2zE2896.zinfo;
          }
          z2zE2899 = z2zE2897.zbalance;
        }
        z2zE2900 = (!eq_u256(z2zE2898, z2zE2899));
      }
      unit z3zE2985;
      if (z2zE2900) {
        sail_fixed_bytes_20 z2zE2902;
        z2zE2902 = ze.zaddr;
        sail_u256 z2zE2903;
        {
          struct zAccountInfo z2zE2901;
          z2zE2901 = zcurr.zinfo;
          z2zE2903 = z2zE2901.zbalance;
        }
        z3zE2985 = bal_note_balance_change(z2zE2902, z2zE2903);
      } else {  z3zE2985 = UNIT;  }
      bool z2zE2910;
      {
        sail_fixed_bytes_32 z2zE2908;
        {
          struct zAccountInfo z2zE2904;
          z2zE2904 = zcurr.zinfo;
          z2zE2908 = z2zE2904.zcode_hash;
        }
        sail_fixed_bytes_32 z2zE2909;
        {
          struct zAccountInfo z2zE2907;
          {
            struct zAccount z2zE2906;
            {
              struct zAcctValue z2zE2905;
              z2zE2905 = ze.zvalue;
              z2zE2906 = z2zE2905.zorig;
            }
            z2zE2907 = z2zE2906.zinfo;
          }
          z2zE2909 = z2zE2907.zcode_hash;
        }
        z2zE2910 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2908, z2zE2909);
      }
      unit z3zE2984;
      if (z2zE2910) {
        sail_fixed_bytes_20 z2zE2912;
        z2zE2912 = ze.zaddr;
        sail_fixed_bytes_32 z2zE2913;
        {
          struct zAccountInfo z2zE2911;
          z2zE2911 = zcurr.zinfo;
          z2zE2913 = z2zE2911.zcode_hash;
        }
        z3zE2984 = bal_note_code_change(z2zE2912, z2zE2913);
      } else {  z3zE2984 = UNIT;  }
      zcurr.zcreated = false;
      zcurr.zselfdestructed = false;
      unit z3zE2983;
      z3zE2983 = UNIT;
      bool z2zE2916;
      {
        struct zAccount z2zE2915;
        {
          struct zAcctValue z2zE2914;
          z2zE2914 = ze.zvalue;
          z2zE2915 = z2zE2914.zorig;
        }
        z2zE2916 = zaccount_changed(zcurr, z2zE2915);
      }
      if (z2zE2916) {
        struct zAcctEntry z2zE2921;
        {
          sail_fixed_bytes_20 z2zE2917;
          z2zE2917 = ze.zaddr;
          struct zAcctValue z2zE2920;
          {
            struct zAccount z2zE2919;
            {
              struct zAcctValue z2zE2918;
              z2zE2918 = ze.zvalue;
              z2zE2919 = z2zE2918.zorig;
            }
            struct zAcctValue z3zE2991;
            z3zE2991.zcurr = zcurr;
            z3zE2991.zorig = z2zE2919;
            z2zE2920 = z3zE2991;
          }
          struct zAcctEntry z3zE2992;
          z3zE2992.zaddr = z2zE2917;
          z3zE2992.zvalue = z2zE2920;
          z2zE2921 = z3zE2992;
        }
        z3zE2979 = acct_block_write(z2zE2921);
      } else {  z3zE2979 = UNIT;  }
      goto finish_match_3013;
    }
  case_3015: ;
    {
      /* complete */
      zmore = false;
      z3zE2979 = UNIT;
      goto finish_match_3013;
    }
  case_3014: ;
  finish_match_3013: ;
    z3zE2994 = z3zE2979;
    KILL(zoptionzIRAcctEntryzK)(&z2zE2875);
    goto while_3011;
  }
wend_3012: ;
  unit z3zE3008;
  z3zE3008 = UNIT;
  zmore = true;
  unit z3zE3007;
  z3zE3007 = UNIT;
  bool z3zE3003;
  unit z3zE3004;
while_3016: ;
  {
    z3zE3003 = zmore;
    if (!(z3zE3003)) goto wend_3017;
    struct zoptionzIRStorageEntryzK z2zE2923;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE2923);
    storage_tx_pop(&z2zE2923, UNIT);
    unit z3zE2995;
    {
      if (z2zE2923.kind != Kind_zSomezIRStorageEntryzK) goto case_3020;
      struct zStorageEntry z3zE3009;
      z3zE3009 = z2zE2923.variants.zSomezIRStorageEntryzK;
      struct zoptionzIRAccountzK z2zE2926;
      CREATE(zoptionzIRAccountzK)(&z2zE2926);
      {
        sail_fixed_bytes_20 z2zE2925;
        {
          struct zStorageKey z2zE2924;
          z2zE2924 = z3zE3009.zkey;
          z2zE2925 = z2zE2924.zaddr;
        }
        acct_block_get(&z2zE2926, z2zE2925);
      }
      unit z3zE2997;
      {
        if (z2zE2926.kind != Kind_zSomezIRAccountzK) goto case_3023;
        struct zAccount zacc;
        zacc = z2zE2926.variants.zSomezIRAccountzK;
        bool z2zE2932;
        {
          bool z2zE2931;
          z2zE2931 = zacc.zpresent;
          bool z3zE2999;
          if (z2zE2931) {
            sail_u256 z2zE2929;
            {
              struct zStorageValue z2zE2927;
              z2zE2927 = z3zE3009.zvalue;
              z2zE2929 = z2zE2927.zcurr;
            }
            sail_u256 z2zE2930;
            {
              struct zStorageValue z2zE2928;
              z2zE2928 = z3zE3009.zvalue;
              z2zE2930 = z2zE2928.zorig;
            }
            z3zE2999 = (!eq_u256(z2zE2929, z2zE2930));
          } else {  z3zE2999 = false;  }
          z2zE2932 = z3zE2999;
        }
        if (z2zE2932) {
          sail_fixed_bytes_20 z2zE2936;
          {
            struct zStorageKey z2zE2933;
            z2zE2933 = z3zE3009.zkey;
            z2zE2936 = z2zE2933.zaddr;
          }
          sail_u256 z2zE2937;
          {
            struct zStorageKey z2zE2934;
            z2zE2934 = z3zE3009.zkey;
            z2zE2937 = z2zE2934.zslot;
          }
          sail_u256 z2zE2938;
          {
            struct zStorageValue z2zE2935;
            z2zE2935 = z3zE3009.zvalue;
            z2zE2938 = z2zE2935.zcurr;
          }
          unit z3zE3001;
          z3zE3001 = bal_note_storage_change(z2zE2936, z2zE2937, z2zE2938);
          z3zE2997 = storage_block_put(z3zE3009);
        } else {  z3zE2997 = UNIT;  }
        goto finish_match_3021;
      }
    case_3023: ;
      {
        /* complete */
        z3zE2997 = UNIT;
        goto finish_match_3021;
      }
    case_3022: ;
    finish_match_3021: ;
      z3zE2995 = z3zE2997;
      KILL(zoptionzIRAccountzK)(&z2zE2926);
      goto finish_match_3018;
    }
  case_3020: ;
    {
      /* complete */
      zmore = false;
      z3zE2995 = UNIT;
      goto finish_match_3018;
    }
  case_3019: ;
  finish_match_3018: ;
    z3zE3004 = z3zE2995;
    KILL(zoptionzIRStorageEntryzK)(&z2zE2923);
    goto while_3016;
  }
wend_3017: ;
  unit z3zE3006;
  z3zE3006 = UNIT;
  unit z3zE3005;
  z3zE3005 = acct_tx_reset(UNIT);
  z8zE290 = storage_tx_reset(UNIT);
end_function_3024: ;
  return z8zE290;
end_block_exception_3025: ;

  return UNIT;
}

unit zk_revert(uint64_t zcheckpoint)
{
  unit z8zE291;
  z8zE291 = host_state_revert(zcheckpoint);
end_function_3009: ;
  return z8zE291;
end_block_exception_3010: ;

  return UNIT;
}

