/* Generated from sail/host/kernel/lifecycle.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
uint64_t zk_state_checkpoint(unit z3zE3022)
{
  uint64_t z8zE282;
  z8zE282 = host_state_checkpoint(UNIT);
end_function_3030: ;
  return z8zE282;
end_block_exception_3031: ;

  return UINT64_C(0xdeadc0de);
}

unit zk_set_header(struct zBlockHeader zh)
{
  unit z8zE283;
  zk_header = zh;
  z8zE283 = UNIT;
end_function_3028: ;
  return z8zE283;
end_block_exception_3029: ;

  return UNIT;
}

unit zk_set_tx(struct zTxEnv zenv)
{
  unit z8zE284;
  zk_tx = zenv;
  z8zE284 = UNIT;
end_function_3026: ;
  return z8zE284;
end_block_exception_3027: ;

  return UNIT;
}

unit zk_tx_reset(unit z3zE3016)
{
  unit z8zE285;
  unit z3zE3021;
  z3zE3021 = acct_tx_reset(UNIT);
  unit z3zE3020;
  z3zE3020 = storage_tx_reset(UNIT);
  unit z3zE3019;
  z3zE3019 = warm_reset(UNIT);
  unit z3zE3018;
  z3zE3018 = transient_storage_reset(UNIT);
  unit z3zE3017;
  z3zE3017 = logs_tx_reset(UNIT);
  z8zE285 = host_state_checkpoint_reset(UNIT);
end_function_3024: ;
  return z8zE285;
end_block_exception_3025: ;

  return UNIT;
}

bool zaccount_deleted_at_tx_end(struct zAccount zacc)
{
  bool z8zE286;
  bool z2zE2917;
  z2zE2917 = zacc.zselfdestructed;
  bool z3zE3015;
  if (z2zE2917) {
    bool z2zE2916;
    z2zE2916 = zfork_lt(zk_fork, zCancun);
    bool z3zE3014;
    if (z2zE2916) {  z3zE3014 = true;  } else {  z3zE3014 = zacc.zcreated;  }
    z3zE3015 = z3zE3014;
  } else {  z3zE3015 = false;  }
  z8zE286 = z3zE3015;
end_function_3022: ;
  return z8zE286;
end_block_exception_3023: ;

  return false;
}

unit zk_tx_merge(unit z3zE2982)
{
  unit z8zE287;
  bool zmore;
  zmore = true;
  bool z3zE2997;
  unit z3zE2998;
while_3007: ;
  {
    z3zE2997 = zmore;
    if (!(z3zE2997)) goto wend_3008;
    struct zoptionzIRAcctEntryzK z2zE2852;
    CREATE(zoptionzIRAcctEntryzK)(&z2zE2852);
    acct_tx_pop_ascending(&z2zE2852, UNIT);
    unit z3zE2983;
    {
      if (z2zE2852.kind != Kind_zSomezIRAcctEntryzK) goto case_3011;
      struct zAcctEntry ze;
      ze = z2zE2852.variants.zSomezIRAcctEntryzK;
      struct zAccount zcurr;
      {
        struct zAcctValue z2zE2899;
        z2zE2899 = ze.zvalue;
        zcurr = z2zE2899.zcurr;
      }
      bool zdeleted;
      zdeleted = zaccount_deleted_at_tx_end(zcurr);
      unit z3zE2992;
      if (zdeleted) {
        bool z2zE2853;
        z2zE2853 = zfork_gteq(zk_fork, zAmsterdam);
        unit z3zE2993;
        if (z2zE2853) {
          zcurr = zaccount_clear_preserving_balance(zcurr);
          z3zE2993 = UNIT;
        } else {
          zcurr = zaccount_delete(zcurr);
          z3zE2993 = UNIT;
        }
        sail_fixed_bytes_20 z2zE2854;
        z2zE2854 = ze.zaddr;
        z3zE2992 = storage_tx_clear(z2zE2854);
      } else {  z3zE2992 = UNIT;  }
      bool z2zE2859;
      {
        bool z3zE2986;
        if (zdeleted) {  z3zE2986 = true;  } else {
          bool z2zE2858;
          z2zE2858 = zcurr.zstorage_cleared;
          bool z3zE2985;
          if (z2zE2858) {
            bool z2zE2857;
            {
              struct zAccount z2zE2856;
              {
                struct zAcctValue z2zE2855;
                z2zE2855 = ze.zvalue;
                z2zE2856 = z2zE2855.zorig;
              }
              z2zE2857 = z2zE2856.zstorage_cleared;
            }
            z3zE2985 = not(z2zE2857);
          } else {  z3zE2985 = false;  }
          z3zE2986 = z3zE2985;
        }
        z2zE2859 = z3zE2986;
      }
      unit z3zE2991;
      if (z2zE2859) {
        sail_fixed_bytes_20 z2zE2860;
        z2zE2860 = ze.zaddr;
        z3zE2991 = storage_block_clear(z2zE2860);
      } else {  z3zE2991 = UNIT;  }
      bool z2zE2867;
      {
        uint64_t z2zE2865;
        {
          struct zAccountInfo z2zE2861;
          z2zE2861 = zcurr.zinfo;
          z2zE2865 = z2zE2861.znonce;
        }
        uint64_t z2zE2866;
        {
          struct zAccountInfo z2zE2864;
          {
            struct zAccount z2zE2863;
            {
              struct zAcctValue z2zE2862;
              z2zE2862 = ze.zvalue;
              z2zE2863 = z2zE2862.zorig;
            }
            z2zE2864 = z2zE2863.zinfo;
          }
          z2zE2866 = z2zE2864.znonce;
        }
        z2zE2867 = (z2zE2865 != z2zE2866);
      }
      unit z3zE2990;
      if (z2zE2867) {
        sail_fixed_bytes_20 z2zE2869;
        z2zE2869 = ze.zaddr;
        uint64_t z2zE2870;
        {
          struct zAccountInfo z2zE2868;
          z2zE2868 = zcurr.zinfo;
          z2zE2870 = z2zE2868.znonce;
        }
        z3zE2990 = bal_note_nonce_change(z2zE2869, z2zE2870);
      } else {  z3zE2990 = UNIT;  }
      bool z2zE2877;
      {
        sail_u256 z2zE2875;
        {
          struct zAccountInfo z2zE2871;
          z2zE2871 = zcurr.zinfo;
          z2zE2875 = z2zE2871.zbalance;
        }
        sail_u256 z2zE2876;
        {
          struct zAccountInfo z2zE2874;
          {
            struct zAccount z2zE2873;
            {
              struct zAcctValue z2zE2872;
              z2zE2872 = ze.zvalue;
              z2zE2873 = z2zE2872.zorig;
            }
            z2zE2874 = z2zE2873.zinfo;
          }
          z2zE2876 = z2zE2874.zbalance;
        }
        z2zE2877 = (!eq_u256(z2zE2875, z2zE2876));
      }
      unit z3zE2989;
      if (z2zE2877) {
        sail_fixed_bytes_20 z2zE2879;
        z2zE2879 = ze.zaddr;
        sail_u256 z2zE2880;
        {
          struct zAccountInfo z2zE2878;
          z2zE2878 = zcurr.zinfo;
          z2zE2880 = z2zE2878.zbalance;
        }
        z3zE2989 = bal_note_balance_change(z2zE2879, z2zE2880);
      } else {  z3zE2989 = UNIT;  }
      bool z2zE2887;
      {
        sail_fixed_bytes_32 z2zE2885;
        {
          struct zAccountInfo z2zE2881;
          z2zE2881 = zcurr.zinfo;
          z2zE2885 = z2zE2881.zcode_hash;
        }
        sail_fixed_bytes_32 z2zE2886;
        {
          struct zAccountInfo z2zE2884;
          {
            struct zAccount z2zE2883;
            {
              struct zAcctValue z2zE2882;
              z2zE2882 = ze.zvalue;
              z2zE2883 = z2zE2882.zorig;
            }
            z2zE2884 = z2zE2883.zinfo;
          }
          z2zE2886 = z2zE2884.zcode_hash;
        }
        z2zE2887 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE2885, z2zE2886);
      }
      unit z3zE2988;
      if (z2zE2887) {
        sail_fixed_bytes_20 z2zE2889;
        z2zE2889 = ze.zaddr;
        sail_fixed_bytes_32 z2zE2890;
        {
          struct zAccountInfo z2zE2888;
          z2zE2888 = zcurr.zinfo;
          z2zE2890 = z2zE2888.zcode_hash;
        }
        z3zE2988 = bal_note_code_change(z2zE2889, z2zE2890);
      } else {  z3zE2988 = UNIT;  }
      zcurr.zcreated = false;
      zcurr.zselfdestructed = false;
      unit z3zE2987;
      z3zE2987 = UNIT;
      bool z2zE2893;
      {
        struct zAccount z2zE2892;
        {
          struct zAcctValue z2zE2891;
          z2zE2891 = ze.zvalue;
          z2zE2892 = z2zE2891.zorig;
        }
        z2zE2893 = zaccount_changed(zcurr, z2zE2892);
      }
      if (z2zE2893) {
        struct zAcctEntry z2zE2898;
        {
          sail_fixed_bytes_20 z2zE2894;
          z2zE2894 = ze.zaddr;
          struct zAcctValue z2zE2897;
          {
            struct zAccount z2zE2896;
            {
              struct zAcctValue z2zE2895;
              z2zE2895 = ze.zvalue;
              z2zE2896 = z2zE2895.zorig;
            }
            struct zAcctValue z3zE2995;
            z3zE2995.zcurr = zcurr;
            z3zE2995.zorig = z2zE2896;
            z2zE2897 = z3zE2995;
          }
          struct zAcctEntry z3zE2996;
          z3zE2996.zaddr = z2zE2894;
          z3zE2996.zvalue = z2zE2897;
          z2zE2898 = z3zE2996;
        }
        z3zE2983 = acct_block_write(z2zE2898);
      } else {  z3zE2983 = UNIT;  }
      goto finish_match_3009;
    }
  case_3011: ;
    {
      /* complete */
      zmore = false;
      z3zE2983 = UNIT;
      goto finish_match_3009;
    }
  case_3010: ;
  finish_match_3009: ;
    z3zE2998 = z3zE2983;
    KILL(zoptionzIRAcctEntryzK)(&z2zE2852);
    goto while_3007;
  }
wend_3008: ;
  unit z3zE3012;
  z3zE3012 = UNIT;
  zmore = true;
  unit z3zE3011;
  z3zE3011 = UNIT;
  bool z3zE3007;
  unit z3zE3008;
while_3012: ;
  {
    z3zE3007 = zmore;
    if (!(z3zE3007)) goto wend_3013;
    struct zoptionzIRStorageEntryzK z2zE2900;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE2900);
    storage_tx_pop(&z2zE2900, UNIT);
    unit z3zE2999;
    {
      if (z2zE2900.kind != Kind_zSomezIRStorageEntryzK) goto case_3016;
      struct zStorageEntry z3zE3013;
      z3zE3013 = z2zE2900.variants.zSomezIRStorageEntryzK;
      struct zoptionzIRAccountzK z2zE2903;
      CREATE(zoptionzIRAccountzK)(&z2zE2903);
      {
        sail_fixed_bytes_20 z2zE2902;
        {
          struct zStorageKey z2zE2901;
          z2zE2901 = z3zE3013.zkey;
          z2zE2902 = z2zE2901.zaddr;
        }
        acct_block_get(&z2zE2903, z2zE2902);
      }
      unit z3zE3001;
      {
        if (z2zE2903.kind != Kind_zSomezIRAccountzK) goto case_3019;
        struct zAccount zacc;
        zacc = z2zE2903.variants.zSomezIRAccountzK;
        bool z2zE2909;
        {
          bool z2zE2908;
          z2zE2908 = zacc.zpresent;
          bool z3zE3003;
          if (z2zE2908) {
            sail_u256 z2zE2906;
            {
              struct zStorageValue z2zE2904;
              z2zE2904 = z3zE3013.zvalue;
              z2zE2906 = z2zE2904.zcurr;
            }
            sail_u256 z2zE2907;
            {
              struct zStorageValue z2zE2905;
              z2zE2905 = z3zE3013.zvalue;
              z2zE2907 = z2zE2905.zorig;
            }
            z3zE3003 = (!eq_u256(z2zE2906, z2zE2907));
          } else {  z3zE3003 = false;  }
          z2zE2909 = z3zE3003;
        }
        if (z2zE2909) {
          sail_fixed_bytes_20 z2zE2913;
          {
            struct zStorageKey z2zE2910;
            z2zE2910 = z3zE3013.zkey;
            z2zE2913 = z2zE2910.zaddr;
          }
          sail_u256 z2zE2914;
          {
            struct zStorageKey z2zE2911;
            z2zE2911 = z3zE3013.zkey;
            z2zE2914 = z2zE2911.zslot;
          }
          sail_u256 z2zE2915;
          {
            struct zStorageValue z2zE2912;
            z2zE2912 = z3zE3013.zvalue;
            z2zE2915 = z2zE2912.zcurr;
          }
          unit z3zE3005;
          z3zE3005 = bal_note_storage_change(z2zE2913, z2zE2914, z2zE2915);
          z3zE3001 = storage_block_put(z3zE3013);
        } else {  z3zE3001 = UNIT;  }
        goto finish_match_3017;
      }
    case_3019: ;
      {
        /* complete */
        z3zE3001 = UNIT;
        goto finish_match_3017;
      }
    case_3018: ;
    finish_match_3017: ;
      z3zE2999 = z3zE3001;
      KILL(zoptionzIRAccountzK)(&z2zE2903);
      goto finish_match_3014;
    }
  case_3016: ;
    {
      /* complete */
      zmore = false;
      z3zE2999 = UNIT;
      goto finish_match_3014;
    }
  case_3015: ;
  finish_match_3014: ;
    z3zE3008 = z3zE2999;
    KILL(zoptionzIRStorageEntryzK)(&z2zE2900);
    goto while_3012;
  }
wend_3013: ;
  unit z3zE3010;
  z3zE3010 = UNIT;
  unit z3zE3009;
  z3zE3009 = acct_tx_reset(UNIT);
  z8zE287 = storage_tx_reset(UNIT);
end_function_3020: ;
  return z8zE287;
end_block_exception_3021: ;

  return UNIT;
}

unit zk_revert(uint64_t zcheckpoint)
{
  unit z8zE288;
  z8zE288 = host_state_revert(zcheckpoint);
end_function_3005: ;
  return z8zE288;
end_block_exception_3006: ;

  return UNIT;
}

