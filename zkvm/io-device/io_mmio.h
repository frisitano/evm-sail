/* Spike validation transport behind the standard ffi/zkvm_io.h interface.
 * Production zkVMs provide read_input/write_output directly. */
#ifndef EVMSAIL_IO_MMIO_H
#define EVMSAIL_IO_MMIO_H

#define ZKVM_IO_MMIO_BASE 0x40001000UL

enum {
  ZKVM_IO_R_INPUT_SIZE = 0,
  ZKVM_IO_R_INPUT_DST = 1,
  ZKVM_IO_R_INPUT_GO = 2,
  ZKVM_IO_R_INPUT_OK = 3,
  ZKVM_IO_R_OUTPUT_SRC = 4,
  ZKVM_IO_R_OUTPUT_SIZE = 5,
  ZKVM_IO_R_OUTPUT_GO = 6,
  ZKVM_IO_R_OUTPUT_OK = 7,
};

#endif
