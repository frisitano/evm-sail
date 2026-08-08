#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void validate_block_access_list_(Bytes bytes, uint64_t block_gas_limit)
{
  validate_block_access_list(bytes, block_gas_limit);
}

