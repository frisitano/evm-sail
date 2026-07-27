/* Generated from sail/evm/interpreter.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
struct zByteSliceFields zinterpret(unit z3zE1566)
{
  struct zByteSliceFields z8zE484;
  z8zE484 = evmsail_interpret(UNIT);
end_function_1964: ;
  return z8zE484;
end_block_exception_1965: ;
  struct zByteSliceFields z8zE884 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE884;
}

bool zframe_succeeded(unit z3zE1560)
{
  bool z8zE485;
  bool z3zE1561;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_1961;
    if (zframe_status.variants.zHalted.kind != Kind_zHaltRevert) goto case_1961;
    z3zE1561 = false;
    goto finish_match_1957;
  }
case_1961: ;
  {
    if (zframe_status.kind != Kind_zHalted) goto case_1960;
    z3zE1561 = true;
    goto finish_match_1957;
  }
case_1960: ;
  {
    if (zframe_status.kind != Kind_zRunning) goto case_1959;
    z3zE1561 = true;
    goto finish_match_1957;
  }
case_1959: ;
  {
    /* complete */
    z3zE1561 = false;
    goto finish_match_1957;
  }
case_1958: ;
finish_match_1957: ;
  z8zE485 = z3zE1561;
end_function_1962: ;
  return z8zE485;
end_block_exception_1963: ;

  return false;
}

unit zresume_call(struct zCallContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE486;
  zreturndata = zoutput;
  unit z3zE1552;
  z3zE1552 = UNIT;
  struct zFrameCheckpoint zcheckpoint;
  CREATE(zFrameCheckpoint)(&zcheckpoint);
  zcheckpoint = zcontinuation.zcheckpoint;
  bool zsucceeded;
  zsucceeded = zframe_succeeded(UNIT);
  uint64_t zchild_left;
  zchild_left = zgas_remaining;
  uint64_t zchild_state_left;
  zchild_state_left = zstate_gas_remaining;
  uint64_t zchild_state_spill;
  zchild_state_spill = zstate_gas_spilled;
  __int128 zchild_refund;
  zchild_refund = zframe_refund;
  unit z3zE1556;
  z3zE1556 = zrestore_frame(zcheckpoint);
  unit z3zE1555;
  {
    z3zE1555 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_1956;
    }
  }
  unit z3zE1554;
  {
    z3zE1554 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_1956;
    }
  }
  uint64_t z2zE1646;
  z2zE1646 = zcontinuation.zreturn_offset;
  uint64_t z2zE1647;
  z2zE1647 = zcontinuation.zreturn_length;
  unit z3zE1553;
  z3zE1553 = zreturndata_copy_prefix(z2zE1646, z2zE1647);
  if (zsucceeded) {
    unit z3zE1559;
    {
      z3zE1559 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1956;
      }
    }
    {
      z8zE486 = zpush_word(zWORD_ONE);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1956;
      }
    }
  } else {
    uint64_t z2zE1648;
    z2zE1648 = zcheckpoint.zstate;
    unit z3zE1558;
    z3zE1558 = zk_revert(z2zE1648);
    bool z2zE1649;
    z2zE1649 = zcontinuation.znew_account_charged;
    unit z3zE1557;
    if (z2zE1649) {
      {
        z3zE1557 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_1956;
        }
      }
    } else {  z3zE1557 = UNIT;  }
    {
      z8zE486 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1956;
      }
    }
  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_1955: ;
  return z8zE486;
end_block_exception_1956: ;

  return UNIT;
}

unit zresume_create(struct zCreateContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE487;
  zreturndata = zoutput;
  unit z3zE1533;
  z3zE1533 = UNIT;
  struct zFrameCheckpoint zcheckpoint;
  CREATE(zFrameCheckpoint)(&zcheckpoint);
  zcheckpoint = zcontinuation.zcheckpoint;
  bool zinitcode_succeeded;
  zinitcode_succeeded = zframe_succeeded(UNIT);
  uint64_t zdeployed_length;
  zdeployed_length = zreturndata_sizze(UNIT);
  uint64_t zdeployed_sizze;
  zdeployed_sizze = zdeployed_length;
  bool zfrontier_empty_deposit;
  zfrontier_empty_deposit = false;
  unit z3zE1534;
  if (zinitcode_succeeded) {
    bool z2zE1637;
    {
      bool z2zE1636;
      {
        bool z2zE1632;
        z2zE1632 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_sizze);
        z2zE1636 = not(z2zE1632);
      }
      bool z3zE1537;
      if (z2zE1636) {  z3zE1537 = true;  } else {
        bool z2zE1635;
        z2zE1635 = zfork_gteq(zk_fork, zLondon);
        bool z3zE1536;
        if (z2zE1635) {
          bool z2zE1634;
          z2zE1634 = (zdeployed_sizze != UINT64_C(0));
          bool z3zE1535;
          if (z2zE1634) {
            uint64_t z2zE1633;
            z2zE1633 = zslice_byte(zreturndata, UINT64_C(0));
            z3zE1535 = (z2zE1633 == UINT64_C(0xEF));
          } else {  z3zE1535 = false;  }
          z3zE1536 = z3zE1535;
        } else {  z3zE1536 = false;  }
        z3zE1537 = z3zE1536;
      }
      z2zE1637 = z3zE1537;
    }
    if (z2zE1637) {
      {
        z3zE1534 = zexc_halt(zOutOfGas);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_1954;
        }
      }
    } else {
      struct zoptionzIU64zK z2zE1638;
      CREATE(zoptionzIU64zK)(&z2zE1638);
      zcode_deployment_execution_cost(&z2zE1638, zdeployed_length, zgas_remaining);
      unit z3zE1538;
      {
        if (z2zE1638.kind != Kind_zSomezIU64zK) goto case_1952;
        uint64_t zexecution_deposit;
        zexecution_deposit = z2zE1638.variants.zSomezIU64zK;
        {
          zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE1638);
            goto end_block_exception_1954;
          }
        }
        unit z3zE1541;
        z3zE1541 = UNIT;
        uint64_t z2zE1639;
        {
          z2zE1639 = zcode_deployment_state_cost(zdeployed_length);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE1638);
            goto end_block_exception_1954;
          }
        }
        {
          z3zE1538 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1639);
          if (have_exception) {
            KILL(zFrameCheckpoint)(&zcheckpoint);
            KILL(zoptionzIU64zK)(&z2zE1638);
            goto end_block_exception_1954;
          }
        }
        goto finish_match_1950;
      }
    case_1952: ;
      {
        /* complete */
        bool z2zE1640;
        z2zE1640 = zfork_lt(zk_fork, zHomestead);
        if (z2zE1640) {
          zgas_remaining = zGAS_ZERO;
          unit z3zE1540;
          z3zE1540 = UNIT;
          zfrontier_empty_deposit = true;
          z3zE1538 = UNIT;
        } else {
          {
            z3zE1538 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zFrameCheckpoint)(&zcheckpoint);
              KILL(zoptionzIU64zK)(&z2zE1638);
              goto end_block_exception_1954;
            }
          }
        }
        goto finish_match_1950;
      }
    case_1951: ;
    finish_match_1950: ;
      z3zE1534 = z3zE1538;
      KILL(zoptionzIU64zK)(&z2zE1638);
    }
  } else {  z3zE1534 = UNIT;  }
  bool zdeploy_succeeds;
  {
    bool z3zE1543;
    if (zinitcode_succeeded) {  z3zE1543 = zframe_succeeded(UNIT);  } else {  z3zE1543 = false;  }
    zdeploy_succeeds = z3zE1543;
  }
  uint64_t zchild_left;
  zchild_left = zgas_remaining;
  uint64_t zchild_state_left;
  zchild_state_left = zstate_gas_remaining;
  uint64_t zchild_state_spill;
  zchild_state_spill = zstate_gas_spilled;
  __int128 zchild_refund;
  zchild_refund = zframe_refund;
  unit z3zE1551;
  z3zE1551 = zrestore_frame(zcheckpoint);
  unit z3zE1550;
  {
    z3zE1550 = zrefund_gas(zchild_left);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_1954;
    }
  }
  unit z3zE1549;
  {
    z3zE1549 = zreturn_child_state_gas(zchild_state_left, zchild_state_spill);
    if (have_exception) {
      KILL(zFrameCheckpoint)(&zcheckpoint);
      goto end_block_exception_1954;
    }
  }
  unit z3zE1544;
  if (zdeploy_succeeds) {
    unit z3zE1547;
    {
      z3zE1547 = zrecord_refund(zchild_refund);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1954;
      }
    }
    struct zByteSliceFields zdeployed_bytes;
    if (zfrontier_empty_deposit) {  zdeployed_bytes = zEMPTY_SLICE;  } else {  zdeployed_bytes = zreturndata;  }
    struct zByteSliceFields zdeployed_code;
    zdeployed_code = zvalidated_code_slice(zdeployed_bytes);
    sail_fixed_bytes_20 z2zE1641;
    z2zE1641 = zcontinuation.zaddress;
    unit z3zE1548;
    {
      z3zE1548 = zk_deploy_code(z2zE1641, zdeployed_code);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1954;
      }
    }
    sail_u256 z2zE1643;
    {
      sail_fixed_bytes_20 z2zE1642;
      z2zE1642 = zcontinuation.zaddress;
      z2zE1643 = evmsail_address_to_word(z2zE1642);
    }
    {
      z3zE1544 = zpush_word(z2zE1643);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1954;
      }
    }
  } else {
    uint64_t z2zE1644;
    z2zE1644 = zcheckpoint.zstate;
    unit z3zE1546;
    z3zE1546 = zk_revert(z2zE1644);
    bool z2zE1645;
    z2zE1645 = zcontinuation.znew_account_charged;
    unit z3zE1545;
    if (z2zE1645) {
      {
        z3zE1545 = zcredit_state_gas_refund(zG_amsterdam_state_new_account);
        if (have_exception) {
          KILL(zFrameCheckpoint)(&zcheckpoint);
          goto end_block_exception_1954;
        }
      }
    } else {  z3zE1545 = UNIT;  }
    {
      z3zE1544 = zpush_word(zWORD_ZERO);
      if (have_exception) {
        KILL(zFrameCheckpoint)(&zcheckpoint);
        goto end_block_exception_1954;
      }
    }
  }
  if (zinitcode_succeeded) {  z8zE487 = zreturndata_clear(UNIT);  } else {  z8zE487 = UNIT;  }
  KILL(zFrameCheckpoint)(&zcheckpoint);
end_function_1953: ;
  return z8zE487;
end_block_exception_1954: ;

  return UNIT;
}

unit zresume_frame(struct zFrameContinuation zcontinuation, struct zByteSliceFields zoutput)
{
  unit z8zE488;
  unit z3zE1529;
  {
    if (zcontinuation.kind != Kind_zEmpty) goto case_1947;
    struct zexception z2zE1631;
    CREATE(zexception)(&z2zE1631);
    zInvalidBlock(&z2zE1631, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1631);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/interpreter.sail:958.19-958.55");
    KILL(zexception)(&z2zE1631);
    goto end_block_exception_1949;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1631);
    goto finish_match_1944;
  }
case_1947: ;
  {
    if (zcontinuation.kind != Kind_zResumeCall) goto case_1946;
    struct zCallContinuation zcall;
    CREATE(zCallContinuation)(&zcall);
    zcall = zcontinuation.variants.zResumeCall;
    {
      z3zE1529 = zresume_call(zcall, zoutput);
      if (have_exception) {
        KILL(zCallContinuation)(&zcall);
        goto end_block_exception_1949;
      }
    }
    KILL(zCallContinuation)(&zcall);
    goto finish_match_1944;
  }
case_1946: ;
  {
    /* complete */
    struct zCreateContinuation zcreate;
    CREATE(zCreateContinuation)(&zcreate);
    zcreate = zcontinuation.variants.zResumeCreate;
    {
      z3zE1529 = zresume_create(zcreate, zoutput);
      if (have_exception) {
        KILL(zCreateContinuation)(&zcreate);
        goto end_block_exception_1949;
      }
    }
    KILL(zCreateContinuation)(&zcreate);
    goto finish_match_1944;
  }
case_1945: ;
finish_match_1944: ;
  z8zE488 = z3zE1529;
end_function_1948: ;
  return z8zE488;
end_block_exception_1949: ;

  return UNIT;
}

