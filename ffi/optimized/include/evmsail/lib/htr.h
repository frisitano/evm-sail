#ifndef EVMSAIL_OPTIMIZED_LIB_HTR_H
#define EVMSAIL_OPTIMIZED_LIB_HTR_H

#include "evmsail/prelude.h"

/*
 * Optimized-C implementation refinement for the explicit Sail
 * htr_new_payload_request equation. This declaration is injected only into
 * optimized generated C; standard C and proof extraction use the Sail body.
 */
struct zStatelessInputRef;
Hash32 htr_new_payload_request(
    struct zStatelessInputRef input_ref);

#endif
