#ifndef EVMSAIL_OPTIMIZED_HOST_DEBUG_H
#define EVMSAIL_OPTIMIZED_HOST_DEBUG_H

#include "evmsail/prelude.h"

/* Native diagnostic hook declared only by sail/host/debug_enabled.sail. */
#ifdef EVMSAIL_NATIVE_DEBUG_AGGREGATES
unit validation_debug_capture_location(unit ignored);
#endif

#endif
