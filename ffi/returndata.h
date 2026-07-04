/* C-backed RETURNDATA buffers, one adopted slot per EVM call frame plus one
 * pending child/precompile output buffer.
 */
#ifndef RETURNDATA_H
#define RETURNDATA_H

#include "sail.h"
#include <stdint.h>

unit hr_reset(const unit u);
unit hr_clear(const unit u);
unit hr_discard(const unit u);
unit hr_capture(uint64_t off, uint64_t len);
unit hr_adopt(const unit u);
uint64_t hr_len(const unit u);
uint64_t hr_pending_len(const unit u);
uint64_t hr_pending_byte(uint64_t i);
unit hr_to_mem(uint64_t dst, uint64_t off, uint64_t len);
unit hr_out_region(uint64_t dst, uint64_t want);

uint8_t *returndata_prepare_pending(uint64_t cap);
unit returndata_set_pending_len(uint64_t len);

#endif
