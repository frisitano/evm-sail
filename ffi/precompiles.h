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

uint64_t precompile_secp256k1_verify_hash_sig_pub(
    uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
    uint64_t r3, uint64_t r2, uint64_t r1, uint64_t r0,
    uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0,
    uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0,
    uint64_t y3, uint64_t y2, uint64_t y1, uint64_t y0);

void precompile_ecrecover_hash_sig(
    lbits *rop,
    uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
    uint64_t yparity,
    uint64_t r3, uint64_t r2, uint64_t r1, uint64_t r0,
    uint64_t s3, uint64_t s2, uint64_t s1, uint64_t s0);

#endif
