/* Optimized transaction signing-hash construction. */
#include "evmsail/prelude.h"

#include "evmsail/lib/rlp/codecs/transaction_signing.h"
#include "lib/rlp/encoding.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"
#include "workspace.h"

#include <stdint.h>
#include <string.h>

static bool u256_gte_small(u256 value, uint64_t small)
{
  return (value.limbs[1] != 0 || value.limbs[2] != 0 || value.limbs[3] != 0 ||
          value.limbs[0] >= small) != 0;
}

static u256 legacy_chain_id(u256 v)
{
  u256 result = v;
  const uint64_t low = result.limbs[0];
  result.limbs[0] = low - 35;
  uint64_t borrow = low < 35;
  for (unsigned i = 1; i < 4 && borrow; ++i) {
    const uint64_t limb = result.limbs[i];
    result.limbs[i] = limb - 1;
    borrow = limb == 0;
  }
  for (unsigned i = 0; i < 3; ++i) {
    result.limbs[i] = (result.limbs[i] >> 1) | (result.limbs[i + 1] << 63);
  }
  result.limbs[3] >>= 1;
  return result;
}

/* `content.len` is bounded by the stateless-input region type (< 2^32) and
 * the suffix is at most 35 bytes, so the exact width fits in uint64 without
 * overflow checks; scratch_borrow fails closed on exhaustion. The final
 * width test realizes rlp_encoder_finish's Sail assert. */
bytes32 tx_signing_hash(uint8_t envelope_type, Bytes content, u256 v)
{
  const bool typed = envelope_type != 0;
  const bool eip155 = (!typed && u256_gte_small(v, 35)) != 0;
  const u256 zero_word = {{0}};
  const u256 chain_id = (int)eip155 ? legacy_chain_id(v) : zero_word;
  const uint64_t suffix_len = (int)eip155 ? rlp_quantity_size_u256(chain_id) + 2 : 0;
  const uint64_t content_len = content.len;
  const uint64_t list_content_len = content_len + suffix_len;
  const uint64_t total =
      ((int)typed ? 1 : 0) + rlp_list_prefix_size(list_content_len) + list_content_len;

  uint8_t *preimage = scratch_borrow(total);
  if (!preimage) {
    return zero_hash();
  }

  uint8_t *cursor = preimage;
  if (typed) {
    *cursor++ = envelope_type;
  }
  cursor = rlp_write_list_prefix(cursor, list_content_len);
  memmove(cursor, content.bytes, content_len);
  cursor += content_len;
  if (eip155) {
    cursor = rlp_write_u256(cursor, chain_id);
    *cursor++ = 0x80;
    *cursor++ = 0x80;
  }
  if ((uint64_t)(cursor - preimage) != total) {
    GUEST_ABORT();
  }
  return keccak_bytes(preimage, total);
}
