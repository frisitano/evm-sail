/* Optimized CREATE2 address derivation. */
#include "evmsail/prelude.h"

#include "lib/rlp/encoding.h"
#include "evmsail/host/scratch.h"
#include "primitives/hash.h"
#include "primitives/value.h"

#include <stdint.h>
#include <string.h>

Address create2_address(Address sender, U256 salt, Hash32 init_hash)
{
  uint8_t *preimage = scratch_borrow(85);
  if (!preimage) {
    Address zero = {{0}};
    return zero;
  }
  preimage[0] = 0xff;
  memcpy(preimage + 1, address_bytes_const(&sender), 20);
  sail_word_to_be_bytes(preimage + 21, salt);
  memcpy(preimage + 53, hash_bytes_const(&init_hash), 32);
  const Hash32 digest = keccak_bytes(preimage, 85);
  return hash_low_address(&digest);
}
