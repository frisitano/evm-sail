/* Optimized transaction signing-hash construction. */
#include "evmsail/prelude.h"

#include "evmsail/lib/rlp/codecs/transaction_signing.h"
#include "lib/rlp/encoding.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"

#include <stdint.h>
#include <string.h>

static bool u256_gte_small(U256 value, uint64_t small) {
  return value.limbs[1] != 0 || value.limbs[2] != 0 ||
         value.limbs[3] != 0 || value.limbs[0] >= small;
}

static U256 legacy_chain_id(U256 v) {
  U256 result = v;
  const uint64_t low = result.limbs[0];
  result.limbs[0] = low - 35;
  uint64_t borrow = low < 35;
  for (unsigned i = 1; i < 4 && borrow; ++i) {
    const uint64_t limb = result.limbs[i];
    result.limbs[i] = limb - 1;
    borrow = limb == 0;
  }
  for (unsigned i = 0; i < 3; ++i)
    result.limbs[i] =
        (result.limbs[i] >> 1) | (result.limbs[i + 1] << 63);
  result.limbs[3] >>= 1;
  return result;
}

Hash32 tx_signing_hash(
    uint8_t envelope_type,
    struct zStatelessInputSliceFields content, U256 v) {
  const bool typed = envelope_type != 0;
  const bool eip155 = !typed && u256_gte_small(v, 35);
  const U256 zero_word = {{0}};
  const U256 chain_id = eip155 ? legacy_chain_id(v) : zero_word;
  const uint64_t suffix_len =
      eip155 ? rlp_quantity_size_u256(chain_id) + 2 : 0;
  const uint64_t content_len = content.zlen;
  uint64_t list_content_len = content_len;
  uint64_t total = typed ? 1 : 0;
  if (envelope_type > 4 || content_len > (UINT64_C(1) << 30) ||
      !rlp_add_length(&list_content_len, suffix_len) ||
      !rlp_add_length(
          &total, rlp_list_prefix_size(list_content_len)) ||
      !rlp_add_length(&total, list_content_len) ||
      total > UINT32_MAX)
    return zero_hash();

  uint8_t *preimage = scratch_borrow(total);
  if (!preimage)
    return zero_hash();
  const uint64_t content_off = content.zoff;
  const uint8_t *content_bytes =
      stateless_input_ptr(content_off, content_len);
  if (!content_bytes)
    return zero_hash();

  uint8_t *cursor = preimage;
  if (typed)
    *cursor++ = envelope_type;
  cursor = rlp_write_list_prefix(cursor, list_content_len);
  memmove(cursor, content_bytes, content_len);
  cursor += content_len;
  if (eip155) {
    cursor = rlp_write_u256(cursor, chain_id);
    *cursor++ = 0x80;
    *cursor++ = 0x80;
  }
  if ((uint64_t)(cursor - preimage) != total)
    return zero_hash();
  return keccak_bytes(preimage, total);
}
