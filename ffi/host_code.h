/* C-backed code buffer + JUMPDEST bitmap (see host_code.c). Declared here so
 * the Sail-generated C call sites are prototyped via `sail -c --c-include`. */
#ifndef HOST_CODE_H
#define HOST_CODE_H
#include "sail.h"
#include <stdbool.h>
unit hj_begin(const unit u);            /* reset for a new code body          */
unit hj_byte(uint64_t b);               /* stream the next code byte          */
bool hj_valid(uint64_t i);              /* is offset i a reachable JUMPDEST?  */
uint64_t hc_byte(uint64_t i);           /* code[i], 0 past the end            */
void hc_word(lbits *rop, uint64_t i, uint64_t n);  /* n-byte immediate at i   */
unit hc_to_mem(uint64_t dst, uint64_t off, uint64_t len);  /* CODECOPY */
#endif
