/* EVM precompile dispatch over source-backed byte ranges.
 *
 * CALL-family and tx-to-precompile inputs are identified by source, offset, and
 * length. Successful execution writes the result directly into the pending
 * returndata buffer. Fixed-width signing helpers use direct typed entry points
 * so Sail does not expose an EVM-Sail list to C.
 */
#ifndef PRECOMPILES_H
#define PRECOMPILES_H

#include "sail.h"
#include <stdbool.h>
#include <stdint.h>

bool precompile_run_source_to_returndata(uint64_t id, uint64_t source_kind,
                                            uint64_t off, uint64_t len);

bool precompile_secp256k1_verify_hash_sig_pub(const lbits h, const lbits r,
                                              const lbits s, const lbits x,
                                              const lbits y);

void precompile_ecrecover_hash_sig(lbits *rop, const lbits h, uint64_t yparity,
                                   const lbits r, const lbits s);

#endif
