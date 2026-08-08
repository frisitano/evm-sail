#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bytes20 create2_address_(bytes20 sender, u256 salt, bytes32 init_hash)
{
  return create2_address(sender, salt, init_hash);
}

