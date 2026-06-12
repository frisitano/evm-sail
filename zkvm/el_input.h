/* zkVM binding of the SSZ byte-source (evm/ssz.sail's ssz_src_byte/ssz_src_len)
 * to the private input (read_input's buffer). Pulls in keccak too so a single
 * `sail -c --c-include el_input.h` covers all guest externs. */
#ifndef EL_INPUT_H
#define EL_INPUT_H
#include "acc_shim.h"                 /* -> sail.h + zkvm-standards accelerator shim (keccak/sha256/precompiles) */
void     ssz_src_len(sail_int rop, const unit u);    /* unit -> int  (byte count) */
uint64_t ssz_src_byte(sail_int idx);                 /* int  -> bits(8) */
uint64_t ssz_src_le(sail_int off, sail_int n);       /* (int,int) -> bits(64): n<=8 LE */
uint64_t ssz_src_be(sail_int off, sail_int n);       /* (int,int) -> bits(64): n<=8 BE */
unit     el_emit_out(uint64_t b);                    /* bits(8) -> unit: write_output one byte */
#endif
