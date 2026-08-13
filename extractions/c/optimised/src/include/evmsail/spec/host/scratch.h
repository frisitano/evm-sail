#pragma once

#include "evmsail/spec/host/region_access.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bool scratch_reserve_at(uint32_t /* arg_0 */, uint32_t /* arg_1 */);

Bytes scratch_store_byte(uint32_t /* arg_0 */, uint64_t /* arg_1 */);

Bytes scratch_store_stateless_input(uint32_t /* arg_0 */, Bytes /* arg_1 */);

Bytes scratch_store_scratch(uint32_t /* arg_0 */, Bytes /* arg_1 */);

Bytes scratch_store_log_data(uint32_t /* arg_0 */, Bytes /* arg_1 */);

Bytes scratch_store_output(uint32_t /* arg_0 */, Bytes /* arg_1 */);

Bytes scratch_store_address(uint32_t /* arg_0 */, bytes20 /* arg_1 */);

Bytes scratch_store_b256(uint32_t /* arg_0 */, bytes32 /* arg_1 */, uint8_t /* arg_2 */);

Bytes scratch_store_word(uint32_t /* arg_0 */, u256 /* arg_1 */, uint8_t /* arg_2 */);

void scratch_truncate(uint32_t /* arg_0 */);


#ifdef __cplusplus
}
#endif
