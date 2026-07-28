/* Generated aggregate boundaries implemented by hash_glue.c. */
#ifndef EVMSAIL_HASH_GLUE_H
#define EVMSAIL_HASH_GLUE_H

#include "sail_abi.h"
#include <stdbool.h>
#include <stdint.h>

struct zStatelessInputSliceFields;
struct zScratchSliceFields;
struct zMemorySliceFields;
struct zCodeRegionSliceFields;
struct zOutputSliceFields;
struct zLogDataSliceFields;

EVMSAIL_HASH_RETURN host_keccak_stateless_input(
    EVMSAIL_HASH_RESULT(result) struct zStatelessInputSliceFields input);
EVMSAIL_HASH_RETURN host_keccak_scratch(
    EVMSAIL_HASH_RESULT(result) struct zScratchSliceFields input);
EVMSAIL_HASH_RETURN host_keccak_memory(
    EVMSAIL_HASH_RESULT(result) struct zMemorySliceFields input);
EVMSAIL_HASH_RETURN host_keccak_code(
    EVMSAIL_HASH_RESULT(result) struct zCodeRegionSliceFields input);
EVMSAIL_HASH_RETURN host_keccak_output(
    EVMSAIL_HASH_RESULT(result) struct zOutputSliceFields input);
EVMSAIL_HASH_RETURN host_keccak_log_data(
    EVMSAIL_HASH_RESULT(result) struct zLogDataSliceFields input);
EVMSAIL_HASH_RETURN host_sha256_stateless_input(
    EVMSAIL_HASH_RESULT(result) struct zStatelessInputSliceFields input);
EVMSAIL_HASH_RETURN host_sha256_scratch(
    EVMSAIL_HASH_RESULT(result) struct zScratchSliceFields input);
EVMSAIL_HASH_RETURN host_sha256_memory(
    EVMSAIL_HASH_RESULT(result) struct zMemorySliceFields input);
EVMSAIL_HASH_RETURN host_keccak_word(
    EVMSAIL_HASH_RESULT(result) EVMSAIL_WORD_PARAM(input));
EVMSAIL_HASH_RETURN host_keccak_address(
    EVMSAIL_HASH_RESULT(result) sail_address input);
EVMSAIL_HASH_RETURN host_sha256_pair(
    EVMSAIL_HASH_RESULT(result) sail_hash left, sail_hash right);

#endif
