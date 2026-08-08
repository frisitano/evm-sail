#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint64_t unit;
#define UNIT UINT64_C(0)
#define EQUAL(type) eq_ ## type
#define UNDEFINED(type) undefined_ ## type
static inline bool eq_unit(void) { return true; }
static inline bool eq_bool(bool lhs, bool rhs) { return lhs == rhs; }
static inline bool eq_fbits(uint64_t lhs, uint64_t rhs) { return lhs == rhs; }
static inline void undefined_unit(void) {}
static inline bool undefined_bool(void) { return false; }
static inline uint64_t undefined_fbits(void) { return UINT64_C(0); }
static inline uint64_t safe_rshift(uint64_t value, uint64_t amount) { return amount >= UINT64_C(64) ? UINT64_C(0) : value >> amount; }
_Noreturn static inline void sail_match_failure(const char *function) { const int write_status = fprintf(stderr, "Sail match failure in %s\n", function); (void)write_status; abort(); }
