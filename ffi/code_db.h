/* C-backed code_db + per-frame code descriptors (see code_db.c).
 * Declared here so the Sail-generated C call sites are prototyped via
 * `sail -c --c-include`. */
#ifndef CODE_DB_H
#define CODE_DB_H
#include "sail.h"
#include <stdbool.h>
unit cs_reset(const unit u);                       /* drop the code_db        */
uint64_t cs_begin(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0); /* code_db insert by hash; 1=new */
unit cs_byte(uint64_t b);                          /* stream the next byte    */
unit cs_view_hash_input(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                        uint64_t off, uint64_t len);  /* copy witness code into code_db */
uint64_t fc_set_hash(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0); /* frame := store entry */
uint64_t fc_pend_mem(uint64_t off, uint64_t len);  /* NEXT frame := mem copy */
uint64_t fc_set_txd(const unit u);                 /* frame := tx input       */
unit fc_set_empty(const unit u);
uint64_t hc_byte(uint64_t i);                      /* code[i], 0 past the end */
bool hj_valid(uint64_t i);                         /* JUMPDEST bitmap test    */
unit hc_to_mem(uint64_t dst, uint64_t off, uint64_t len);     /* CODECOPY     */
void hc_word(lbits *rop, uint64_t i, uint64_t n);  /* n-byte PUSH immediate   */
void cd_word(lbits *rop, uint64_t i);              /* CALLDATALOAD            */
uint64_t hc_len(const unit u);                     /* current frame code len  */
uint64_t cs_len(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0);   /* EXTCODESIZE */
unit cs_to_mem(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
               uint64_t dst, uint64_t off, uint64_t len);     /* EXTCODECOPY */
void cs_deleg(lbits *rop, uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0); /* 7702 */
#endif
