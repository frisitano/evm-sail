/* C-backed RETURNDATA buffers, one adopted slot per EVM call frame plus one
 * pending child/precompile output buffer.
 */
#ifndef RETURNDATA_H
#define RETURNDATA_H

#include "sail.h"
#include <stdint.h>

unit returndata_reset_all(const unit u);
unit returndata_clear_current(const unit u);
unit returndata_discard_pending(const unit u);
unit returndata_capture_memory(uint64_t off, uint64_t len);
unit returndata_adopt_pending(const unit u);
uint64_t returndata_current_len(const unit u);
uint64_t returndata_pending_len(const unit u);
uint64_t returndata_pending_byte(uint64_t i);
unit returndata_copy_current_to_memory(uint64_t dst, uint64_t off, uint64_t len);
unit returndata_copy_current_prefix_to_memory(uint64_t dst, uint64_t want);

uint8_t *returndata_prepare_pending(uint64_t cap);
unit returndata_set_pending_len(uint64_t len);

#endif
