/* Generated from sail/main.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "evm_internal.h"
#endif
unit zmain(unit z3zE446)
{
  unit z8zE717;
  unit z3zE448;
  z3zE448 = UNIT;
  unit z3zE447;
  z3zE447 = zcycle_scope_start(zSCOPE_STATELESS_VALIDATION);
  struct zoptionzIRGuestValidationzK zresult;
  CREATE(zoptionzIRGuestValidationzK)(&zresult);
  {
    { /* try */
      struct zStatelessInputRef zinput_ref;
      {
        struct zByteSliceFields z2zE6;
        z2zE6 = stateless_input(UNIT);
        {
          zinput_ref = zdecode_stateless_input_ref(z2zE6);
          if (have_exception) {  goto end_block_exception_1130;  }
        }
      }
      struct zStatelessValidationResult zvalidation;
      CREATE(zStatelessValidationResult)(&zvalidation);
      {
        zverify_stateless_payload(&zvalidation, zinput_ref);
        if (have_exception) {
          KILL(zStatelessValidationResult)(&zvalidation);
          goto end_block_exception_1130;
        }
      }
      bool zvalid;
      {
        bool z3zE449;
        {
          if (zvalidation.kind != Kind_zStatelessPayloadValid) goto case_1123;
          z3zE449 = true;
          goto finish_match_1121;
        }
      case_1123: ;
        {
          /* complete */
          struct zStatelessValidationFailure zfailure;
          zfailure = zvalidation.variants.zStatelessPayloadInvalid;
          uint64_t z2zE4;
          z2zE4 = zfailure.zscope;
          enum zBlockError z2zE5;
          z2zE5 = zfailure.zreason;
          unit z3zE450;
          z3zE450 = zvalidation_debug_record(z2zE4, z2zE5);
          z3zE449 = false;
          goto finish_match_1121;
        }
      case_1122: ;
      finish_match_1121: ;
        zvalid = z3zE449;
      }
      struct zGuestValidation z3zE453;
      z3zE453.zinput_ref = zinput_ref;
      z3zE453.zvalid = zvalid;
      zSomezIRGuestValidationzK(&zresult, z3zE453);
      KILL(zStatelessValidationResult)(&zvalidation);
    end_block_exception_1130: ;
    }
    if (!(have_exception)) goto post_exception_handlers_1124;
    have_exception = false;
    {
      if ((*current_exception).kind != Kind_zInvalidBlock) goto try_1125;
      enum zBlockError zreason;
      zreason = (*current_exception).variants.zInvalidBlock;
      unit z3zE455;
      z3zE455 = zvalidation_debug_record(zSCOPE_DECODE_INPUT, zreason);
      zNonezIRGuestValidationzK(&zresult, UNIT);
      goto post_exception_handlers_1124;
    }
  try_1125: ;
    have_exception = true;
  post_exception_handlers_1124: ;
  }
  unit z3zE457;
  z3zE457 = zcycle_scope_end(zSCOPE_STATELESS_VALIDATION);
  unit z3zE458;
  {
    if (zresult.kind != Kind_zSomezIRGuestValidationzK) goto case_1128;
    struct zGuestValidation z1zE0;
    z1zE0 = zresult.variants.zSomezIRGuestValidationzK;
    struct zStatelessInputRef z2zE2;
    z2zE2 = z1zE0.zinput_ref;
    bool z2zE3;
    z2zE3 = z1zE0.zvalid;
    {
      z3zE458 = zwrite_validation_result(z2zE2, z2zE3);
      if (have_exception) {
        KILL(zoptionzIRGuestValidationzK)(&zresult);
        goto end_block_exception_1131;
      }
    }
    goto finish_match_1126;
  }
case_1128: ;
  {
    /* complete */
    z3zE458 = zwrite_invalid_result(UNIT);
    goto finish_match_1126;
  }
case_1127: ;
finish_match_1126: ;
  z8zE717 = z3zE458;
  KILL(zoptionzIRGuestValidationzK)(&zresult);
end_function_1129: ;
  return z8zE717;
end_block_exception_1131: ;

  return UNIT;
}

