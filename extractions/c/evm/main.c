/* Generated from sail/main.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "evm_internal.h"
#endif
unit zmain(unit z3zE410)
{
  unit z8zE698;
  unit z3zE412;
  z3zE412 = UNIT;
  unit z3zE411;
  z3zE411 = zcycle_scope_start(zSCOPE_STATELESS_VALIDATION);
  struct zoptionzIRGuestValidationzK zresult;
  CREATE(zoptionzIRGuestValidationzK)(&zresult);
  {
    { /* try */
      struct zStatelessInputRef zinput_ref;
      {
        struct zByteSliceFields z2zE5;
        z2zE5 = stateless_input(UNIT);
        {
          zinput_ref = zdecode_stateless_input_ref(z2zE5);
          if (have_exception) {  goto end_block_exception_1122;  }
        }
      }
      struct zStatelessValidationResult zvalidation;
      CREATE(zStatelessValidationResult)(&zvalidation);
      {
        zverify_stateless_payload(&zvalidation, zinput_ref);
        if (have_exception) {
          KILL(zStatelessValidationResult)(&zvalidation);
          goto end_block_exception_1122;
        }
      }
      bool zvalid;
      {
        bool z3zE413;
        {
          if (zvalidation.kind != Kind_zStatelessPayloadValid) goto case_1115;
          z3zE413 = true;
          goto finish_match_1113;
        }
      case_1115: ;
        {
          /* complete */
          struct zStatelessValidationFailure zfailure;
          zfailure = zvalidation.variants.zStatelessPayloadInvalid;
          uint64_t z2zE3;
          z2zE3 = zfailure.zscope;
          enum zBlockError z2zE4;
          z2zE4 = zfailure.zreason;
          unit z3zE414;
          z3zE414 = zvalidation_debug_record(z2zE3, z2zE4);
          z3zE413 = false;
          goto finish_match_1113;
        }
      case_1114: ;
      finish_match_1113: ;
        zvalid = z3zE413;
      }
      struct zGuestValidation z3zE417;
      z3zE417.zinput_ref = zinput_ref;
      z3zE417.zvalid = zvalid;
      zSomezIRGuestValidationzK(&zresult, z3zE417);
      KILL(zStatelessValidationResult)(&zvalidation);
    end_block_exception_1122: ;
    }
    if (!(have_exception)) goto post_exception_handlers_1116;
    have_exception = false;
    {
      if ((*current_exception).kind != Kind_zInvalidBlock) goto try_1117;
      enum zBlockError zreason;
      zreason = (*current_exception).variants.zInvalidBlock;
      unit z3zE419;
      z3zE419 = zvalidation_debug_record(zSCOPE_DECODE_INPUT, zreason);
      zNonezIRGuestValidationzK(&zresult, UNIT);
      goto post_exception_handlers_1116;
    }
  try_1117: ;
    have_exception = true;
  post_exception_handlers_1116: ;
  }
  unit z3zE421;
  z3zE421 = zcycle_scope_end(zSCOPE_STATELESS_VALIDATION);
  unit z3zE422;
  {
    if (zresult.kind != Kind_zSomezIRGuestValidationzK) goto case_1120;
    struct zGuestValidation z1zE0;
    z1zE0 = zresult.variants.zSomezIRGuestValidationzK;
    struct zStatelessInputRef z2zE1;
    z2zE1 = z1zE0.zinput_ref;
    bool z2zE2;
    z2zE2 = z1zE0.zvalid;
    z3zE422 = zwrite_validation_result(z2zE1, z2zE2);
    goto finish_match_1118;
  }
case_1120: ;
  {
    /* complete */
    z3zE422 = zwrite_invalid_result(UNIT);
    goto finish_match_1118;
  }
case_1119: ;
finish_match_1118: ;
  z8zE698 = z3zE422;
  KILL(zoptionzIRGuestValidationzK)(&zresult);
end_function_1121: ;
  return z8zE698;
end_block_exception_1123: ;

  return UNIT;
}

