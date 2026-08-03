#ifndef EVMSAIL_OPTIMIZED_EXCEPTIONS_H
#define EVMSAIL_OPTIMIZED_EXCEPTIONS_H

#include "evmsail/prelude.h"

/*
 * Raise the generated Sail InvalidBlock exception from an optimized C
 * override. The corresponding Sail extern must carry `$[c_throws]` so the
 * custom compiler marks the external call as throwing and generated callers
 * test have_exception immediately after it, unwinding without consuming its
 * placeholder return value.
 *
 * The shared model ABI header owns the exception and BlockError layouts.
 */
static inline void throw_invalid_block(enum zBlockError reason,
                                               const char *location) {
  if (have_exception) return;
  current_exception->kind = Kind_zInvalidBlock;
  current_exception->variants.zInvalidBlock = reason;
  have_exception = true;
  copy_sail_string(throw_location, location);
}

#endif
