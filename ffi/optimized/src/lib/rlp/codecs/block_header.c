/* Optimized canonical block-header RLP and hash construction. */
#include "evmsail/prelude.h"

#include "evmsail/lib/rlp/codecs/block_header.h"
#include "lib/rlp/encoding.h"
#include "evmsail/primitives/chain_config.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"
#include "primitives/hash.h"
#include "primitives/value.h"

#include <stdint.h>
#include <string.h>

static const uint8_t empty_ommer_hash[32] = {
    0x1d, 0xcc, 0x4d, 0xe8, 0xde, 0xc7, 0x5d, 0x7a,
    0xab, 0x85, 0xb5, 0x67, 0xb6, 0xcc, 0xd4, 0x1a,
    0xd3, 0x12, 0x45, 0x1b, 0x94, 0x8a, 0x74, 0x13,
    0xf0, 0xa1, 0x42, 0xfd, 0x40, 0xd4, 0x93, 0x47,
};

static uint8_t *write_rlp_fixed(uint8_t *out, const uint8_t *bytes,
                                uint64_t len) {
  out = rlp_write_string_prefix(out, len, bytes[0]);
  memcpy(out, bytes, len);
  return out + len;
}

static uint64_t block_header_content_size(const struct BlockHeader *header,
                                          uint64_t extra_len,
                                          uint8_t extra_first) {
  uint64_t len = 6 * 33 + 21 + 259 + 1 + 9;
  if (!rlp_add_length(
          &len, rlp_quantity_size_u64(header->number)) ||
      !rlp_add_length(
          &len, rlp_quantity_size_u64(header->gas_limit)) ||
      !rlp_add_length(
          &len, rlp_quantity_size_u64(header->gas_used)) ||
      !rlp_add_length(
          &len, rlp_quantity_size_u64(header->timestamp)))
    return UINT64_MAX;
  const uint64_t extra_size =
      rlp_string_size(extra_len, extra_first);
  if (extra_size == UINT64_MAX ||
      !rlp_add_length(&len, extra_size))
    return UINT64_MAX;
  const uint64_t fork = active_fork();
  if (fork >= EVMSAIL_FORK_LONDON &&
      !rlp_add_length(
          &len, rlp_quantity_size_u256(header->base_fee)))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_SHANGHAI &&
      !rlp_add_length(&len, 33))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_CANCUN &&
      (!rlp_add_length(
           &len, rlp_quantity_size_u64(header->blob_gas_used)) ||
       !rlp_add_length(
           &len, rlp_quantity_size_u64(header->excess_blob_gas)) ||
       !rlp_add_length(&len, 33)))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_PRAGUE &&
      !rlp_add_length(&len, 33))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_AMSTERDAM &&
      (!rlp_add_length(&len, 33) ||
       !rlp_add_length(
           &len, rlp_quantity_size_u64(header->slot_number))))
    return UINT64_MAX;
  return len;
}

Hash32 block_header_hash(
    struct BlockHeader header,
    Hash32 transactions_root, Hash32 withdrawals_root,
    Hash32 requests_hash,
    Hash32 block_access_list_hash) {
  const uint64_t extra_len = header.extra_data.len;
  const uint8_t *extra_bytes = NULL;
  uint8_t extra_first = 0;
  if (extra_len != 0) {
    extra_bytes = stateless_input_ptr(
        header.extra_data.off, extra_len);
    if (!extra_bytes)
      return zero_hash();
    extra_first = extra_bytes[0];
  }
  const uint8_t *logs_bloom_bytes = stateless_input_ptr(
      header.logs_bloom.off, header.logs_bloom.len);
  if (!logs_bloom_bytes ||
      header.logs_bloom.len != 256)
    return zero_hash();
  const uint64_t content_len =
      block_header_content_size(&header, extra_len, extra_first);
  if (content_len == UINT64_MAX || content_len > 749)
    return zero_hash();
  uint64_t total = content_len;
  if (!rlp_add_length(
          &total, rlp_list_prefix_size(content_len)) ||
      total > UINT32_MAX)
    return zero_hash();

  uint8_t *preimage = scratch_borrow(total);
  if (!preimage)
    return zero_hash();
  uint8_t prev_randao[32];
  const uint64_t fork = active_fork();
  sail_word_to_be_bytes(prev_randao, header.prev_randao);
  uint8_t *cursor = rlp_write_list_prefix(preimage, content_len);
  cursor = write_rlp_fixed(
      cursor, hash_bytes_const(&header.parent_hash), 32);
  cursor = write_rlp_fixed(cursor, empty_ommer_hash, 32);
  cursor = write_rlp_fixed(
      cursor, address_bytes_const(&header.fee_recipient), 20);
  cursor = write_rlp_fixed(
      cursor, hash_bytes_const(&header.state_root), 32);
  cursor = write_rlp_fixed(
      cursor, hash_bytes_const(&transactions_root), 32);
  cursor = write_rlp_fixed(
      cursor, hash_bytes_const(&header.receipts_root), 32);
  cursor = write_rlp_fixed(cursor, logs_bloom_bytes, 256);
  cursor = rlp_write_u64(cursor, 0);
  cursor = rlp_write_u64(cursor, header.number);
  cursor = rlp_write_u64(cursor, header.gas_limit);
  cursor = rlp_write_u64(cursor, header.gas_used);
  cursor = rlp_write_u64(cursor, header.timestamp);
  cursor = rlp_write_string_prefix(cursor, extra_len, extra_first);
  if (extra_len != 0)
    memmove(cursor, extra_bytes, extra_len);
  cursor += extra_len;
  cursor = write_rlp_fixed(cursor, prev_randao, 32);
  *cursor++ = 0x88;
  memset(cursor, 0, 8);
  cursor += 8;
  if (fork >= EVMSAIL_FORK_LONDON)
    cursor = rlp_write_u256(cursor, header.base_fee);
  if (fork >= EVMSAIL_FORK_SHANGHAI)
    cursor = write_rlp_fixed(
        cursor, hash_bytes_const(&withdrawals_root), 32);
  if (fork >= EVMSAIL_FORK_CANCUN) {
    cursor = rlp_write_u64(cursor, header.blob_gas_used);
    cursor = rlp_write_u64(cursor, header.excess_blob_gas);
    cursor = write_rlp_fixed(
        cursor,
        hash_bytes_const(&header.parent_beacon_block_root), 32);
  }
  if (fork >= EVMSAIL_FORK_PRAGUE)
    cursor = write_rlp_fixed(
        cursor, hash_bytes_const(&requests_hash), 32);
  if (fork >= EVMSAIL_FORK_AMSTERDAM) {
    cursor = write_rlp_fixed(
        cursor, hash_bytes_const(&block_access_list_hash), 32);
    cursor = rlp_write_u64(cursor, header.slot_number);
  }
  if ((uint64_t)(cursor - preimage) != total)
    return zero_hash();
  return keccak_bytes(preimage, total);
}
