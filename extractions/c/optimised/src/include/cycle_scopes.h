#ifndef EVMSAIL_CYCLE_SCOPES_H
#define EVMSAIL_CYCLE_SCOPES_H

#include "sail.h"

#include <stdint.h>

/* Optimized-C-only drill-down scopes. Canonical Sail profiling owns ids
 * 0..51; the threaded interpreter uses these only when its profile build flag
 * is enabled. */
enum evmsail_cycle_scope_id {
    EVMSAIL_SCOPE_EVM_ALU = 52,
    EVMSAIL_SCOPE_EVM_STACK = 53,
    EVMSAIL_SCOPE_EVM_MEMORY = 54,
    EVMSAIL_SCOPE_EVM_FLOW = 55,
    EVMSAIL_SCOPE_EVM_ENVIRONMENT = 56,
    EVMSAIL_SCOPE_EVM_STATE = 57,
    EVMSAIL_SCOPE_EVM_FRAME = 58,
    EVMSAIL_SCOPE_EVM_HALT = 59,
};

unit cycle_scope_start(uint64_t scope);
unit cycle_scope_end(uint64_t scope);

#endif
