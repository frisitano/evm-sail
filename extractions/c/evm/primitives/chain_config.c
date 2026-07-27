/* Generated from sail/primitives/chain_config.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_83(void) {

  struct zBlobSchedule z3zE88;
  struct zBlobSchedule z3zE87;
  z3zE87.zbase_fee_update_fraction = UINT64_C(1);
  z3zE87.zmax = UINT64_C(0);
  z3zE87.ztarget = UINT64_C(0);
  z3zE88 = z3zE87;
  zNO_BLOB_SCHEDULE = z3zE88;
let_end_191: ;
}
void kill_letbind_83(void) {
}

struct zProtocolProfile zprotocol_profile(uint64_t zschema_fork)
{
  struct zProtocolProfile z8zE105;
  bool z2zE3159;
  z2zE3159 = (zschema_fork == UINT64_C(0x0A));
  if (z2zE3159) {
    struct zProtocolProfile z3zE2873;
    z3zE2873.zblob_schedule = zNO_BLOB_SCHEDULE;
    z3zE2873.zfork = zBerlin;
    z8zE105 = z3zE2873;
  } else {
    bool z2zE3162;
    {
      bool z2zE3161;
      z2zE3161 = (zschema_fork == UINT64_C(0x0B));
      bool z3zE2858;
      if (z2zE3161) {  z3zE2858 = true;  } else {
        bool z2zE3160;
        z2zE3160 = (zschema_fork == UINT64_C(0x0C));
        bool z3zE2857;
        if (z2zE3160) {  z3zE2857 = true;  } else {  z3zE2857 = (zschema_fork == UINT64_C(0x0D));  }
        z3zE2858 = z3zE2857;
      }
      z2zE3162 = z3zE2858;
    }
    if (z2zE3162) {
      struct zProtocolProfile z3zE2872;
      z3zE2872.zblob_schedule = zNO_BLOB_SCHEDULE;
      z3zE2872.zfork = zLondon;
      z8zE105 = z3zE2872;
    } else {
      bool z2zE3163;
      z2zE3163 = (zschema_fork == UINT64_C(0x0E));
      if (z2zE3163) {
        struct zProtocolProfile z3zE2871;
        z3zE2871.zblob_schedule = zNO_BLOB_SCHEDULE;
        z3zE2871.zfork = zParis;
        z8zE105 = z3zE2871;
      } else {
        bool z2zE3164;
        z2zE3164 = (zschema_fork == UINT64_C(0x0F));
        if (z2zE3164) {
          struct zProtocolProfile z3zE2870;
          z3zE2870.zblob_schedule = zNO_BLOB_SCHEDULE;
          z3zE2870.zfork = zShanghai;
          z8zE105 = z3zE2870;
        } else {
          bool z2zE3165;
          z2zE3165 = (zschema_fork == UINT64_C(0x10));
          if (z2zE3165) {
            struct zProtocolProfile z3zE2868;
            struct zBlobSchedule z3zE2869;
            z3zE2869.zbase_fee_update_fraction = UINT64_C(3338477);
            z3zE2869.zmax = UINT64_C(6);
            z3zE2869.ztarget = UINT64_C(3);
            z3zE2868.zblob_schedule = z3zE2869;
            z3zE2868.zfork = zCancun;
            z8zE105 = z3zE2868;
          } else {
            bool z2zE3167;
            {
              bool z2zE3166;
              z2zE3166 = (zschema_fork == UINT64_C(0x11));
              bool z3zE2859;
              if (z2zE3166) {  z3zE2859 = true;  } else {  z3zE2859 = (zschema_fork == UINT64_C(0x12));  }
              z2zE3167 = z3zE2859;
            }
            if (z2zE3167) {
              enum zFork z2zE3169;
              {
                bool z2zE3168;
                z2zE3168 = (zschema_fork == UINT64_C(0x11));
                if (z2zE3168) {  z2zE3169 = zPrague;  } else {  z2zE3169 = zOsaka;  }
              }
              struct zProtocolProfile z3zE2866;
              struct zBlobSchedule z3zE2867;
              z3zE2867.zbase_fee_update_fraction = UINT64_C(5007716);
              z3zE2867.zmax = UINT64_C(9);
              z3zE2867.ztarget = UINT64_C(6);
              z3zE2866.zblob_schedule = z3zE2867;
              z3zE2866.zfork = z2zE3169;
              z8zE105 = z3zE2866;
            } else {
              bool z2zE3170;
              z2zE3170 = (zschema_fork == UINT64_C(0x13));
              if (z2zE3170) {
                struct zProtocolProfile z3zE2864;
                struct zBlobSchedule z3zE2865;
                z3zE2865.zbase_fee_update_fraction = UINT64_C(8346193);
                z3zE2865.zmax = UINT64_C(15);
                z3zE2865.ztarget = UINT64_C(10);
                z3zE2864.zblob_schedule = z3zE2865;
                z3zE2864.zfork = zOsaka;
                z8zE105 = z3zE2864;
              } else {
                bool z2zE3171;
                z2zE3171 = (zschema_fork == UINT64_C(0x14));
                if (z2zE3171) {
                  struct zProtocolProfile z3zE2862;
                  struct zBlobSchedule z3zE2863;
                  z3zE2863.zbase_fee_update_fraction = UINT64_C(11684671);
                  z3zE2863.zmax = UINT64_C(21);
                  z3zE2863.ztarget = UINT64_C(14);
                  z3zE2862.zblob_schedule = z3zE2863;
                  z3zE2862.zfork = zOsaka;
                  z8zE105 = z3zE2862;
                } else {
                  bool z2zE3172;
                  z2zE3172 = (zschema_fork == UINT64_C(0x15));
                  if (z2zE3172) {
                    struct zProtocolProfile z3zE2860;
                    struct zBlobSchedule z3zE2861;
                    z3zE2861.zbase_fee_update_fraction = UINT64_C(11684671);
                    z3zE2861.zmax = UINT64_C(21);
                    z3zE2861.ztarget = UINT64_C(14);
                    z3zE2860.zblob_schedule = z3zE2861;
                    z3zE2860.zfork = zAmsterdam;
                    z8zE105 = z3zE2860;
                  } else {
                    struct zexception z2zE3173;
                    CREATE(zexception)(&z2zE3173);
                    zInvalidBlock(&z2zE3173, zInvalidConfig);
                    COPY(zexception)(current_exception, z2zE3173);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/primitives/chain_config.sail:78.40-78.73");
                    KILL(zexception)(&z2zE3173);
                    goto end_block_exception_3332;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE3173);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
end_function_3331: ;
  return z8zE105;
end_block_exception_3332: ;
  struct zBlobSchedule z8zE779 = { .zbase_fee_update_fraction = UINT64_C(0xdeadc0de), .zmax = UINT64_C(0xdeadc0de), .ztarget = UINT64_C(0xdeadc0de) };
  struct zProtocolProfile z8zE778 = { .zblob_schedule = z8zE779, .zfork = ((enum zFork)0) };
  return z8zE778;
}

