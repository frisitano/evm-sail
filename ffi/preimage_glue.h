#ifndef EVMSAIL_PREIMAGE_GLUE_H
#define EVMSAIL_PREIMAGE_GLUE_H

#include "sail_abi.h"

/*
 * Optimized-C refinements for complete hash-preimage operations. These
 * declarations are injected only into optimized generated C; standard C and
 * proof extraction retain the explicit Sail equations.
 */
enum zTxType;
struct zBlockHeader;
struct zStatelessInputSliceFields;
struct zStatelessInputRef;

EVMSAIL_ADDRESS_RETURN evmsail_optimized_create2_address(
    EVMSAIL_ADDRESS_RESULT(result) sail_address sender, sail_word salt,
    sail_hash init_hash);

EVMSAIL_HASH_RETURN evmsail_optimized_tx_signing_hash(
    EVMSAIL_HASH_RESULT(result) enum zTxType tx_type,
    struct zStatelessInputSliceFields content, sail_word v);

EVMSAIL_HASH_RETURN evmsail_optimized_sha256_request_digest(
    EVMSAIL_HASH_RESULT(result) uint64_t request_type,
    struct zStatelessInputSliceFields request);

EVMSAIL_HASH_RETURN evmsail_optimized_execution_requests_hash(
    EVMSAIL_HASH_RESULT(result) struct zStatelessInputRef input_ref);

EVMSAIL_HASH_RETURN evmsail_optimized_block_header_hash(
    EVMSAIL_HASH_RESULT(result) struct zBlockHeader header,
    sail_hash transactions_root, sail_hash withdrawals_root,
    sail_hash requests_hash, sail_hash block_access_list_hash);

#endif
