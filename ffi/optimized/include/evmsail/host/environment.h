#ifndef EVMSAIL_OPTIMIZED_HOST_ENVIRONMENT_H
#define EVMSAIL_OPTIMIZED_HOST_ENVIRONMENT_H

#include "evmsail/prelude.h"

void environment_workspace_bind(void);
unit environment_reset(unit ignored);
unit ancestor_hash_write(uint64_t index, Hash32 hash);
Hash32 ancestor_hash_read(uint64_t index);

#endif
