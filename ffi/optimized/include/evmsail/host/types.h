#ifndef EVMSAIL_HOST_TYPES_H
#define EVMSAIL_HOST_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Nominal host-region slices retain semantic Sail coordinates, but optimized
 * C lowers every live byte coordinate to a stable pointer. */
typedef struct Bytes {
  uint32_t len;
  const uint8_t *bytes;
} Bytes;

/* Cursor into the transaction-scoped prepared EIP-7702 authorization table.
 * The semantic model represents this as an immutable list; optimized C stores
 * the decoded rows once in its fixed workspace and advances this compact view. */
typedef struct PreparedAuthorizationList {
  uint32_t offset;
  uint16_t count;
} PreparedAuthorizationList;

static inline bool eq_Bytes(Bytes op1, Bytes op2)
{
  return (bool)(op1.len == op2.len && op1.bytes == op2.bytes);
}

static inline Bytes undefined_Bytes(void)
{
  return (Bytes){.len = 0, .bytes = NULL};
}

#endif /* EVMSAIL_HOST_TYPES_H */
