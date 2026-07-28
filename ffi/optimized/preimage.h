#ifndef EVMSAIL_OPTIMIZED_PREIMAGE_H
#define EVMSAIL_OPTIMIZED_PREIMAGE_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

/*
 * Optimized-C refinements for complete hash-preimage operations. These
 * declarations are injected only into optimized generated C; standard C and
 * proof extraction retain the explicit Sail equations.
 */
enum zTxType;
struct zBlockHeader;
struct zStatelessInputSliceFields;
struct zStatelessInputRef;

sail_fixed_bytes_20 evmsail_optimized_create2_address(
    sail_fixed_bytes_20 sender, sail_u256 salt,
    sail_fixed_bytes_32 init_hash);

sail_fixed_bytes_32 evmsail_optimized_tx_signing_hash(
    enum zTxType tx_type,
    struct zStatelessInputSliceFields content, sail_u256 v);

sail_fixed_bytes_32 evmsail_optimized_sha256_request_digest(
    uint64_t request_type,
    struct zStatelessInputSliceFields request);

sail_fixed_bytes_32 evmsail_optimized_execution_requests_hash(
    struct zStatelessInputRef input_ref);

sail_fixed_bytes_32 evmsail_optimized_block_header_hash(
    struct zBlockHeader header,
    sail_fixed_bytes_32 transactions_root, sail_fixed_bytes_32 withdrawals_root,
    sail_fixed_bytes_32 requests_hash,
    sail_fixed_bytes_32 block_access_list_hash);

#endif
