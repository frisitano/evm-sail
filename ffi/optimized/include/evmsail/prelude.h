#ifndef EVMSAIL_OPTIMIZED_PRELUDE_H
#define EVMSAIL_OPTIMIZED_PRELUDE_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

#include <stdint.h>

/*
 * Semantic names for the fixed representations selected by the optimized Sail
 * backend. Generated names stop at this boundary: host modules speak in EVM
 * values, not in backend lowering details.
 */
typedef bytes20 Address;
typedef bytes32 Hash32;
typedef bytes256 LogsBloom;
typedef u256 U256;

#endif
