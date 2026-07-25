/* Generated from sail/primitives/chain_config.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_59(void) {

  struct zBlobSchedule z3zE66;
  struct zBlobSchedule z3zE65;
  z3zE65.zbase_fee_update_fraction = UINT64_C(1);
  z3zE65.zmax = UINT64_C(0);
  z3zE65.ztarget = UINT64_C(0);
  z3zE66 = z3zE65;
  zNO_BLOB_SCHEDULE = z3zE66;
let_end_167: ;
}
void kill_letbind_59(void) {
}

struct zProtocolProfile zprotocol_profile(uint64_t zschema_fork)
{
  struct zProtocolProfile z8zE105;
  bool z2zE3864;
  z2zE3864 = (zschema_fork == UINT64_C(0x0A));
  if (z2zE3864) {
    struct zProtocolProfile z3zE3652;
    z3zE3652.zblob_schedule = zNO_BLOB_SCHEDULE;
    z3zE3652.zfork = zBerlin;
    z8zE105 = z3zE3652;
  } else {
    bool z2zE3867;
    {
      bool z2zE3866;
      z2zE3866 = (zschema_fork == UINT64_C(0x0B));
      bool z3zE3637;
      if (z2zE3866) {  z3zE3637 = true;  } else {
        bool z2zE3865;
        z2zE3865 = (zschema_fork == UINT64_C(0x0C));
        bool z3zE3636;
        if (z2zE3865) {  z3zE3636 = true;  } else {  z3zE3636 = (zschema_fork == UINT64_C(0x0D));  }
        z3zE3637 = z3zE3636;
      }
      z2zE3867 = z3zE3637;
    }
    if (z2zE3867) {
      struct zProtocolProfile z3zE3651;
      z3zE3651.zblob_schedule = zNO_BLOB_SCHEDULE;
      z3zE3651.zfork = zLondon;
      z8zE105 = z3zE3651;
    } else {
      bool z2zE3868;
      z2zE3868 = (zschema_fork == UINT64_C(0x0E));
      if (z2zE3868) {
        struct zProtocolProfile z3zE3650;
        z3zE3650.zblob_schedule = zNO_BLOB_SCHEDULE;
        z3zE3650.zfork = zParis;
        z8zE105 = z3zE3650;
      } else {
        bool z2zE3869;
        z2zE3869 = (zschema_fork == UINT64_C(0x0F));
        if (z2zE3869) {
          struct zProtocolProfile z3zE3649;
          z3zE3649.zblob_schedule = zNO_BLOB_SCHEDULE;
          z3zE3649.zfork = zShanghai;
          z8zE105 = z3zE3649;
        } else {
          bool z2zE3870;
          z2zE3870 = (zschema_fork == UINT64_C(0x10));
          if (z2zE3870) {
            struct zProtocolProfile z3zE3647;
            struct zBlobSchedule z3zE3648;
            z3zE3648.zbase_fee_update_fraction = UINT64_C(3338477);
            z3zE3648.zmax = UINT64_C(6);
            z3zE3648.ztarget = UINT64_C(3);
            z3zE3647.zblob_schedule = z3zE3648;
            z3zE3647.zfork = zCancun;
            z8zE105 = z3zE3647;
          } else {
            bool z2zE3872;
            {
              bool z2zE3871;
              z2zE3871 = (zschema_fork == UINT64_C(0x11));
              bool z3zE3638;
              if (z2zE3871) {  z3zE3638 = true;  } else {  z3zE3638 = (zschema_fork == UINT64_C(0x12));  }
              z2zE3872 = z3zE3638;
            }
            if (z2zE3872) {
              enum zFork z2zE3874;
              {
                bool z2zE3873;
                z2zE3873 = (zschema_fork == UINT64_C(0x11));
                if (z2zE3873) {  z2zE3874 = zPrague;  } else {  z2zE3874 = zOsaka;  }
              }
              struct zProtocolProfile z3zE3645;
              struct zBlobSchedule z3zE3646;
              z3zE3646.zbase_fee_update_fraction = UINT64_C(5007716);
              z3zE3646.zmax = UINT64_C(9);
              z3zE3646.ztarget = UINT64_C(6);
              z3zE3645.zblob_schedule = z3zE3646;
              z3zE3645.zfork = z2zE3874;
              z8zE105 = z3zE3645;
            } else {
              bool z2zE3875;
              z2zE3875 = (zschema_fork == UINT64_C(0x13));
              if (z2zE3875) {
                struct zProtocolProfile z3zE3643;
                struct zBlobSchedule z3zE3644;
                z3zE3644.zbase_fee_update_fraction = UINT64_C(8346193);
                z3zE3644.zmax = UINT64_C(15);
                z3zE3644.ztarget = UINT64_C(10);
                z3zE3643.zblob_schedule = z3zE3644;
                z3zE3643.zfork = zOsaka;
                z8zE105 = z3zE3643;
              } else {
                bool z2zE3876;
                z2zE3876 = (zschema_fork == UINT64_C(0x14));
                if (z2zE3876) {
                  struct zProtocolProfile z3zE3641;
                  struct zBlobSchedule z3zE3642;
                  z3zE3642.zbase_fee_update_fraction = UINT64_C(11684671);
                  z3zE3642.zmax = UINT64_C(21);
                  z3zE3642.ztarget = UINT64_C(14);
                  z3zE3641.zblob_schedule = z3zE3642;
                  z3zE3641.zfork = zOsaka;
                  z8zE105 = z3zE3641;
                } else {
                  bool z2zE3877;
                  z2zE3877 = (zschema_fork == UINT64_C(0x15));
                  if (z2zE3877) {
                    struct zProtocolProfile z3zE3639;
                    struct zBlobSchedule z3zE3640;
                    z3zE3640.zbase_fee_update_fraction = UINT64_C(11684671);
                    z3zE3640.zmax = UINT64_C(21);
                    z3zE3640.ztarget = UINT64_C(14);
                    z3zE3639.zblob_schedule = z3zE3640;
                    z3zE3639.zfork = zAmsterdam;
                    z8zE105 = z3zE3639;
                  } else {
                    struct zexception z2zE3878;
                    CREATE(zexception)(&z2zE3878);
                    zInvalidBlock(&z2zE3878, zInvalidConfig);
                    COPY(zexception)(current_exception, z2zE3878);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/primitives/chain_config.sail:78.40-78.73");
                    KILL(zexception)(&z2zE3878);
                    goto end_block_exception_3746;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE3878);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
end_function_3745: ;
  return z8zE105;
end_block_exception_3746: ;
  struct zBlobSchedule z8zE817 = { .zbase_fee_update_fraction = UINT64_C(0xdeadc0de), .zmax = UINT64_C(0xdeadc0de), .ztarget = UINT64_C(0xdeadc0de) };
  struct zProtocolProfile z8zE816 = { .zblob_schedule = z8zE817, .zfork = ((enum zFork)0) };
  return z8zE816;
}

