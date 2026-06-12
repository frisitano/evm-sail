/* Freestanding <assert.h> shim. Asserts are compiled out (NDEBUG-style), which
 * is the conventional production configuration for mini-gmp. */
#ifndef ZKVM_FREESTANDING_ASSERT_H
#define ZKVM_FREESTANDING_ASSERT_H
#define assert(expr) ((void)0)
#endif
