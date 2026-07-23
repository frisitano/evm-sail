/* Generated from sail/primitives/evm.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_80(void) {

  struct zMessage z3zE103;
  struct zMessage z3zE102;
  z3zE102.zaddress = zZERO_ADDRESS;
  z3zE102.zcaller = zZERO_ADDRESS;
  z3zE102.zcode_address = zZERO_ADDRESS;
  z3zE102.zdepth = UINT64_C(0);
  z3zE102.zis_static = false;
  z3zE102.zstate_gas_reservoir = zGAS_ZERO;
  z3zE102.zvalue = zZERO_WORD;
  z3zE103 = z3zE102;
  zDEFAULT_MESSAGE = z3zE103;
let_end_197: ;
}
void kill_letbind_80(void) {
}

void create_letbind_81(void) {    CREATE(zFrameCheckpoint)(&zDEFAULT_FRAME_CHECKPOINT);

  struct zFrameCheckpoint z3zE105;
  CREATE(zFrameCheckpoint)(&z3zE105);
  struct zFrameStatus z2zE1;
  CREATE(zFrameStatus)(&z2zE1);
  zRunning(&z2zE1, UNIT);
  struct zFrameCheckpoint z3zE104;
  CREATE(zFrameCheckpoint)(&z3zE104);
  z3zE104.zcall_depth = UINT64_C(0);
  z3zE104.zcalldata = zEMPTY_SLICE;
  z3zE104.zcode = zEMPTY_CODE;
  z3zE104.zgas_remaining = zGAS_ZERO;
  z3zE104.zmemory = zEMPTY_SLICE;
  z3zE104.zmessage = zDEFAULT_MESSAGE;
  z3zE104.zpc = UINT64_C(0);
  z3zE104.zrefund = zGAS_REFUND_ZERO;
  z3zE104.zstate = UINT64_C(0);
  z3zE104.zstate_gas_remaining = zGAS_ZERO;
  z3zE104.zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  COPY(zFrameStatus)(&((&z3zE104)->zstatus), z2zE1);
  COPY(zFrameCheckpoint)(&z3zE105, z3zE104);
  KILL(zFrameCheckpoint)(&z3zE104);
  KILL(zFrameStatus)(&z2zE1);
  COPY(zFrameCheckpoint)(&zDEFAULT_FRAME_CHECKPOINT, z3zE105);
  KILL(zFrameCheckpoint)(&z3zE105);
let_end_198: ;
}
void kill_letbind_81(void) {    KILL(zFrameCheckpoint)(&zDEFAULT_FRAME_CHECKPOINT);
}

void create_letbind_82(void) {    CREATE(zFrameContinuation)(&zDEFAULT_FRAME_CONTINUATION);

  struct zFrameContinuation z3zE107;
  CREATE(zFrameContinuation)(&z3zE107);
  struct zCallContinuation z3zE106;
  CREATE(zCallContinuation)(&z3zE106);
  COPY(zFrameCheckpoint)(&((&z3zE106)->zcheckpoint), zDEFAULT_FRAME_CHECKPOINT);
  z3zE106.znew_account_charged = false;
  z3zE106.zreturn_length = UINT64_C(0);
  z3zE106.zreturn_offset = UINT64_C(0);
  zResumeCall(&z3zE107, z3zE106);
  KILL(zCallContinuation)(&z3zE106);
  COPY(zFrameContinuation)(&zDEFAULT_FRAME_CONTINUATION, z3zE107);
  KILL(zFrameContinuation)(&z3zE107);
let_end_199: ;
}
void kill_letbind_82(void) {    KILL(zFrameContinuation)(&zDEFAULT_FRAME_CONTINUATION);
}

