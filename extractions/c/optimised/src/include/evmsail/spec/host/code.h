#pragma once

#include "evmsail/spec/primitives/stateless_input.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

Bytes code_region_from_input(Bytes /* arg_0 */);

Bytes code_region_from_memory(Bytes /* arg_0 */);

Bytes code_region_from_output(Bytes /* arg_0 */);

Bytes code_region_from_delegation(bytes20 /* arg_0 */);

bytes32 code_db_store_indexed(struct CodeFields /* arg_0 */);

bool jumpdest_ref_contains(uint8_t * /* arg_0 */, uint32_t /* arg_1 */, uint32_t /* arg_2 */);

struct AddressResult code_db_read_delegation(bytes32 /* arg_0 */);

uint8_t * analyze_code(Bytes code, uint8_t fork);

Bytes code_db_intern_input(Bytes bytes);

Bytes code_db_intern_memory(Bytes bytes);

Bytes code_db_intern_output(Bytes bytes);

struct CodeFields code_db_resolve(bytes32 code_hash);

bytes32 code_db_insert(Bytes code, uint8_t fork);


#ifdef __cplusplus
}
#endif
