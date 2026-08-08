#ifndef EVMSAIL_OPTIMIZED_EXCEPTIONS_H
#define EVMSAIL_OPTIMIZED_EXCEPTIONS_H

#include "evmsail/prelude.h"

/*
 * Terminal validation boundary. The implementation writes the public invalid
 * result and terminates only the current guest run, so native tests remain
 * reusable.
 */
void fatal_error_reset(void);

#endif
