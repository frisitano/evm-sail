#ifndef EVMSAIL_HTR_GLUE_H
#define EVMSAIL_HTR_GLUE_H

#include "sail_abi.h"

/*
 * Optimized-C implementation refinement for the explicit Sail
 * htr_new_payload_request equation. This declaration is injected only into
 * optimized generated C; standard C and proof extraction use the Sail body.
 */
struct zStatelessInputRef;
EVMSAIL_HASH_RETURN evmsail_htr_new_payload_request(
    EVMSAIL_HASH_RESULT(result) struct zStatelessInputRef input_ref);

#endif
