/* Stable dense identities used by the optimized semantic state model. */
#ifndef EVMSAIL_OPTIMIZED_HOST_STATE_PRIMITIVES_H
#define EVMSAIL_OPTIMIZED_HOST_STATE_PRIMITIVES_H

#include <stdint.h>

typedef uint32_t AccountId;
typedef uint32_t StorageId;
typedef uint32_t StorageGeneration;

/* Dense account identifiers start at one. Zero is the permanent result of a
 * failed block-local account lookup and must never index account state. */
enum { ACCOUNT_ID_NONE = 0 };

#endif
