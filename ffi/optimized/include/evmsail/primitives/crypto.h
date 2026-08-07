/* Hash host operations implemented by this backend's hash.c. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H

#include "evmsail/prelude.h"
#include "evmsail/host/types.h"
#include <stdbool.h>
#include <stdint.h>

bytes32 host_keccak_code(Bytes input);
bytes32 host_keccak_output(Bytes input);
bytes32 host_keccak_log_data(Bytes input);
bytes32 host_sha256_scratch(Bytes input);
bytes32 host_keccak_word(u256 input);
bytes32 host_keccak_address(bytes20 input);
bytes32 host_sha256_pair(bytes32 left, bytes32 right);

#endif
