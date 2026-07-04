/* EVM precompile dispatch over staged byte ranges.
 *
 * CALL-family and tx-to-precompile inputs are snapshotted into this module once.
 * Gas inspection reads the staged bytes, and successful execution writes the
 * result directly into the pending returndata buffer.
 */
#ifndef PRECOMPILES_H
#define PRECOMPILES_H

#include "sail.h"
#include <stdint.h>

unit precompile_stage_memory(uint64_t id, uint64_t off, uint64_t len);
unit precompile_stage_tx(uint64_t id);
uint64_t precompile_input_byte(uint64_t i);
uint64_t precompile_run_staged_to_returndata(const unit u);

uint64_t precompile_run_host_input(uint64_t id);
uint64_t precompile_ok(const unit u);
uint64_t precompile_out(uint64_t i);

#endif
