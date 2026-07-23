#ifndef EVMSAIL_QUANTITY_ABI_H
#define EVMSAIL_QUANTITY_ABI_H

#include "sail.h"

#include <stdint.h>

/*
 * The canonical model represents byte_quantity as a mathematical natural, so
 * the ordinary Sail C backend exposes it as sail_int. The production splice
 * gives the same nominal type a uint64_t C representation. Keep that generated
 * ABI distinction at this private marshalling boundary; host stores remain
 * fixed-width in both builds.
 */
#ifdef EVMSAIL_STANDARD_ABI
#define EVMSAIL_BYTE_QUANTITY_PARAM(name) sail_int name

static inline uint64_t evmsail_byte_quantity_value(const sail_int value) {
  return convert_mach_uint_of_sail_int(value);
}

static inline void evmsail_byte_quantity_set(sail_int *out, uint64_t value) {
  convert_sail_int_of_mach_uint(out, value);
}
#else
#define EVMSAIL_BYTE_QUANTITY_PARAM(name) uint64_t name

static inline uint64_t evmsail_byte_quantity_value(uint64_t value) {
  return value;
}

static inline void evmsail_byte_quantity_set(uint64_t *out, uint64_t value) {
  *out = value;
}
#endif

/* Generic host-table counts and indices have a semantic 64-bit bound in the
 * model, so standard and optimized executables share this ABI. */
#define EVMSAIL_ITEM_PARAM(name) uint64_t name
#define EVMSAIL_ITEM_RETURN uint64_t
#define EVMSAIL_ITEM_RESULT(name)

static inline uint64_t evmsail_item_value(uint64_t value) { return value; }

#define EVMSAIL_RETURN_ITEM(result, value) return (value)

#endif
