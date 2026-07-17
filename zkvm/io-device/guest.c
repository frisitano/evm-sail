/* Spike implementation of the standard zkVM IO interface. The validation
 * device supplies private input and receives public output; the Sail model and
 * its FFI adapters know nothing about this transport. */
#include "io_mmio.h"
#include "zkvm_io.h"

#include <stdlib.h>

static const uint8_t *private_input;
static size_t private_input_size;
static int private_input_ready;

void read_input(const uint8_t **buf_ptr, size_t *buf_size) {
  if (!private_input_ready) {
    volatile uint64_t *io = (volatile uint64_t *)ZKVM_IO_MMIO_BASE;
    uint64_t size = io[ZKVM_IO_R_INPUT_SIZE];
    if (size != 0) {
      uint8_t *buffer = malloc((size_t)size);
      if (!buffer) abort();
      io[ZKVM_IO_R_INPUT_DST] = (uint64_t)(uintptr_t)buffer;
      io[ZKVM_IO_R_INPUT_GO] = 1;
      if (io[ZKVM_IO_R_INPUT_OK] != 1) abort();
      private_input = buffer;
      private_input_size = (size_t)size;
    }
    private_input_ready = 1;
  }
  *buf_ptr = private_input;
  *buf_size = private_input_size;
}

void write_output(const uint8_t *output, size_t size) {
  volatile uint64_t *io = (volatile uint64_t *)ZKVM_IO_MMIO_BASE;
  io[ZKVM_IO_R_OUTPUT_SRC] = (uint64_t)(uintptr_t)output;
  io[ZKVM_IO_R_OUTPUT_SIZE] = (uint64_t)size;
  io[ZKVM_IO_R_OUTPUT_GO] = 1;
  if (io[ZKVM_IO_R_OUTPUT_OK] != 1) abort();
}
