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
  bool z2zE3903;
  z2zE3903 = (zschema_fork == UINT64_C(0x0A));
  if (z2zE3903) {
    struct zProtocolProfile z3zE3723;
    z3zE3723.zblob_schedule = zNO_BLOB_SCHEDULE;
    z3zE3723.zfork = zBerlin;
    z8zE105 = z3zE3723;
  } else {
    bool z2zE3906;
    {
      bool z2zE3905;
      z2zE3905 = (zschema_fork == UINT64_C(0x0B));
      bool z3zE3708;
      if (z2zE3905) {  z3zE3708 = true;  } else {
        bool z2zE3904;
        z2zE3904 = (zschema_fork == UINT64_C(0x0C));
        bool z3zE3707;
        if (z2zE3904) {  z3zE3707 = true;  } else {  z3zE3707 = (zschema_fork == UINT64_C(0x0D));  }
        z3zE3708 = z3zE3707;
      }
      z2zE3906 = z3zE3708;
    }
    if (z2zE3906) {
      struct zProtocolProfile z3zE3722;
      z3zE3722.zblob_schedule = zNO_BLOB_SCHEDULE;
      z3zE3722.zfork = zLondon;
      z8zE105 = z3zE3722;
    } else {
      bool z2zE3907;
      z2zE3907 = (zschema_fork == UINT64_C(0x0E));
      if (z2zE3907) {
        struct zProtocolProfile z3zE3721;
        z3zE3721.zblob_schedule = zNO_BLOB_SCHEDULE;
        z3zE3721.zfork = zParis;
        z8zE105 = z3zE3721;
      } else {
        bool z2zE3908;
        z2zE3908 = (zschema_fork == UINT64_C(0x0F));
        if (z2zE3908) {
          struct zProtocolProfile z3zE3720;
          z3zE3720.zblob_schedule = zNO_BLOB_SCHEDULE;
          z3zE3720.zfork = zShanghai;
          z8zE105 = z3zE3720;
        } else {
          bool z2zE3909;
          z2zE3909 = (zschema_fork == UINT64_C(0x10));
          if (z2zE3909) {
            struct zProtocolProfile z3zE3718;
            struct zBlobSchedule z3zE3719;
            z3zE3719.zbase_fee_update_fraction = UINT64_C(3338477);
            z3zE3719.zmax = UINT64_C(6);
            z3zE3719.ztarget = UINT64_C(3);
            z3zE3718.zblob_schedule = z3zE3719;
            z3zE3718.zfork = zCancun;
            z8zE105 = z3zE3718;
          } else {
            bool z2zE3911;
            {
              bool z2zE3910;
              z2zE3910 = (zschema_fork == UINT64_C(0x11));
              bool z3zE3709;
              if (z2zE3910) {  z3zE3709 = true;  } else {  z3zE3709 = (zschema_fork == UINT64_C(0x12));  }
              z2zE3911 = z3zE3709;
            }
            if (z2zE3911) {
              enum zFork z2zE3913;
              {
                bool z2zE3912;
                z2zE3912 = (zschema_fork == UINT64_C(0x11));
                if (z2zE3912) {  z2zE3913 = zPrague;  } else {  z2zE3913 = zOsaka;  }
              }
              struct zProtocolProfile z3zE3716;
              struct zBlobSchedule z3zE3717;
              z3zE3717.zbase_fee_update_fraction = UINT64_C(5007716);
              z3zE3717.zmax = UINT64_C(9);
              z3zE3717.ztarget = UINT64_C(6);
              z3zE3716.zblob_schedule = z3zE3717;
              z3zE3716.zfork = z2zE3913;
              z8zE105 = z3zE3716;
            } else {
              bool z2zE3914;
              z2zE3914 = (zschema_fork == UINT64_C(0x13));
              if (z2zE3914) {
                struct zProtocolProfile z3zE3714;
                struct zBlobSchedule z3zE3715;
                z3zE3715.zbase_fee_update_fraction = UINT64_C(8346193);
                z3zE3715.zmax = UINT64_C(15);
                z3zE3715.ztarget = UINT64_C(10);
                z3zE3714.zblob_schedule = z3zE3715;
                z3zE3714.zfork = zOsaka;
                z8zE105 = z3zE3714;
              } else {
                bool z2zE3915;
                z2zE3915 = (zschema_fork == UINT64_C(0x14));
                if (z2zE3915) {
                  struct zProtocolProfile z3zE3712;
                  struct zBlobSchedule z3zE3713;
                  z3zE3713.zbase_fee_update_fraction = UINT64_C(11684671);
                  z3zE3713.zmax = UINT64_C(21);
                  z3zE3713.ztarget = UINT64_C(14);
                  z3zE3712.zblob_schedule = z3zE3713;
                  z3zE3712.zfork = zOsaka;
                  z8zE105 = z3zE3712;
                } else {
                  bool z2zE3916;
                  z2zE3916 = (zschema_fork == UINT64_C(0x15));
                  if (z2zE3916) {
                    struct zProtocolProfile z3zE3710;
                    struct zBlobSchedule z3zE3711;
                    z3zE3711.zbase_fee_update_fraction = UINT64_C(11684671);
                    z3zE3711.zmax = UINT64_C(21);
                    z3zE3711.ztarget = UINT64_C(14);
                    z3zE3710.zblob_schedule = z3zE3711;
                    z3zE3710.zfork = zAmsterdam;
                    z8zE105 = z3zE3710;
                  } else {
                    struct zexception z2zE3917;
                    CREATE(zexception)(&z2zE3917);
                    zInvalidBlock(&z2zE3917, zInvalidConfig);
                    COPY(zexception)(current_exception, z2zE3917);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/primitives/chain_config.sail:78.40-78.73");
                    KILL(zexception)(&z2zE3917);
                    goto end_block_exception_3765;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE3917);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
end_function_3764: ;
  return z8zE105;
end_block_exception_3765: ;
  struct zBlobSchedule z8zE820 = { .zbase_fee_update_fraction = UINT64_C(0xdeadc0de), .zmax = UINT64_C(0xdeadc0de), .ztarget = UINT64_C(0xdeadc0de) };
  struct zProtocolProfile z8zE819 = { .zblob_schedule = z8zE820, .zfork = ((enum zFork)0) };
  return z8zE819;
}

