#ifndef EVMSAIL_OPTIMIZED_HTR_H
#define EVMSAIL_OPTIMIZED_HTR_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

/*
 * Optimized-C implementation refinement for the explicit Sail
 * htr_new_payload_request equation. This declaration is injected only into
 * optimized generated C; standard C and proof extraction use the Sail body.
 */
struct zStatelessInputRef;
sail_fixed_bytes_32 evmsail_htr_new_payload_request(
    struct zStatelessInputRef input_ref);

#endif
