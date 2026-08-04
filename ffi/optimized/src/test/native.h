#ifndef EVMSAIL_OPTIMIZED_TEST_NATIVE_H
#define EVMSAIL_OPTIMIZED_TEST_NATIVE_H

#include <stddef.h>
#include <stdint.h>

/* Native harness output is owned by native.c. Diagnostics borrow it only
 * while serializing an explicitly requested post-run report. */
const uint8_t *native_output_data(size_t *size);

#endif
