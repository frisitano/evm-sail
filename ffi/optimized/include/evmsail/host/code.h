#ifndef EVMSAIL_OPTIMIZED_HOST_CODE_H
#define EVMSAIL_OPTIMIZED_HOST_CODE_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

uint8_t *jumpdest_table_alloc(Bytes code);

#endif
