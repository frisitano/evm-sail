/* Optimized CREATE2 address derivation. */
#include "evmsail/prelude.h"

#include "lib/rlp/encoding.h"
#include "evmsail/host/scratch.h"
#include "primitives/hash.h"
#include "primitives/value.h"

#include <stdint.h>
#include <string.h>

bytes20 create2_address(bytes20 sender, u256 salt, bytes32 init_hash)
{
  uint8_t *preimage = scratch_borrow(85);
  if (!preimage) {
    bytes20 zero = {{0}};
    return zero;
  }
  preimage[0] = 0xff;
  memcpy(preimage + 1, bytes20_data(&sender), 20);
  sail_word_to_be_bytes(preimage + 21, salt);
  memcpy(preimage + 53, bytes32_data(&init_hash), 32);
  const bytes32 digest = keccak_bytes(preimage, 85);
  return hash_low_address(&digest);
}
