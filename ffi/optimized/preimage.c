/*
 * Optimized whole-operation hash-preimage assembly.
 *
 * Each operation computes its exact encoded length, borrows that many bytes
 * from the shared scratch arena, writes the preimage once in forward order,
 * and invokes the accelerator once. The borrow does not change Sail's visible
 * scratch cursor. The standard build retains the explicit Sail equations.
 */
#include EVMSAIL_MODEL_H

#include "preimage.h"
#include "protocol_profile.h"
#include "region_access.h"
#include "scratch.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const uint8_t preimage_empty;

static sail_fixed_bytes_32 zero_hash(void) {
  sail_fixed_bytes_32 result = {{0}};
  return result;
}

static sail_fixed_bytes_32 keccak_bytes(const uint8_t *bytes, uint64_t len) {
  zkvm_keccak256_hash digest = {{0}};
  if (len > UINT32_MAX ||
      zkvm_keccak256(len ? bytes : &preimage_empty, (size_t)len, &digest) !=
          ZKVM_EOK)
    return zero_hash();
  sail_fixed_bytes_32 result;
  evmsail_hash_set_be_bytes(&result, digest.data);
  return result;
}

static sail_fixed_bytes_32 sha256_bytes(const uint8_t *bytes, uint64_t len) {
  zkvm_sha256_hash digest = {{0}};
  if (len > UINT32_MAX ||
      zkvm_sha256(len ? bytes : &preimage_empty, (size_t)len, &digest) !=
          ZKVM_EOK)
    return zero_hash();
  sail_fixed_bytes_32 result;
  evmsail_hash_set_be_bytes(&result, digest.data);
  return result;
}

static bool add_length(uint64_t *total, uint64_t amount) {
  if (amount > UINT64_MAX - *total)
    return false;
  *total += amount;
  return true;
}

static unsigned be_u64_width(uint64_t value) {
  unsigned width = 0;
  while (value != 0) {
    ++width;
    value >>= 8;
  }
  return width;
}

static unsigned u256_width(sail_u256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  unsigned first = 0;
  while (first < sizeof(bytes) && bytes[first] == 0)
    ++first;
  return (unsigned)sizeof(bytes) - first;
}

static uint64_t rlp_quantity_size_u64(uint64_t value) {
  if (value == 0 || value < 0x80)
    return 1;
  return 1 + be_u64_width(value);
}

static uint64_t rlp_quantity_size_u256(sail_u256 value) {
  const unsigned width = u256_width(value);
  if (width == 0)
    return 1;
  if (width == 1 && value.limbs[0] < 0x80)
    return 1;
  return 1 + width;
}

static unsigned rlp_length_width(uint64_t len) {
  return be_u64_width(len);
}

static uint64_t rlp_string_prefix_size(uint64_t len, uint8_t first) {
  if (len == 1 && first < 0x80)
    return 0;
  if (len <= 55)
    return 1;
  return 1 + rlp_length_width(len);
}

static uint64_t rlp_string_size(uint64_t len, uint8_t first) {
  uint64_t result = len;
  if (!add_length(&result, rlp_string_prefix_size(len, first)))
    return UINT64_MAX;
  return result;
}

static uint64_t rlp_list_prefix_size(uint64_t content_len) {
  return content_len <= 55 ? 1 : 1 + rlp_length_width(content_len);
}

static uint8_t *write_be_length(uint8_t *out, uint64_t value,
                                unsigned width) {
  while (width-- != 0)
    *out++ = (uint8_t)(value >> (width * 8));
  return out;
}

static uint8_t *write_rlp_string_prefix(uint8_t *out, uint64_t len,
                                        uint8_t first) {
  if (len == 1 && first < 0x80)
    return out;
  if (len <= 55) {
    *out++ = (uint8_t)(0x80 + len);
    return out;
  }
  const unsigned width = rlp_length_width(len);
  *out++ = (uint8_t)(0xb7 + width);
  return write_be_length(out, len, width);
}

static uint8_t *write_rlp_list_prefix(uint8_t *out, uint64_t content_len) {
  if (content_len <= 55) {
    *out++ = (uint8_t)(0xc0 + content_len);
    return out;
  }
  const unsigned width = rlp_length_width(content_len);
  *out++ = (uint8_t)(0xf7 + width);
  return write_be_length(out, content_len, width);
}

static uint8_t *write_rlp_u64(uint8_t *out, uint64_t value) {
  if (value == 0) {
    *out++ = 0x80;
    return out;
  }
  const unsigned width = be_u64_width(value);
  if (width == 1 && value < 0x80) {
    *out++ = (uint8_t)value;
    return out;
  }
  *out++ = (uint8_t)(0x80 + width);
  return write_be_length(out, value, width);
}

static uint8_t *write_rlp_u256(uint8_t *out, sail_u256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  const unsigned width = u256_width(value);
  if (width == 0) {
    *out++ = 0x80;
    return out;
  }
  const uint8_t *first = bytes + sizeof(bytes) - width;
  if (width == 1 && first[0] < 0x80) {
    *out++ = first[0];
    return out;
  }
  *out++ = (uint8_t)(0x80 + width);
  memcpy(out, first, width);
  return out + width;
}

static bool u256_gte_small(sail_u256 value, uint64_t small) {
  return value.limbs[1] != 0 || value.limbs[2] != 0 ||
         value.limbs[3] != 0 || value.limbs[0] >= small;
}

static sail_u256 legacy_chain_id(sail_u256 v) {
  sail_u256 result = v;
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

static uint8_t tx_type_byte(enum zTxType tx_type) {
  switch (tx_type) {
  case zLegacyTx:
    return 0;
  case zAccessListTx:
    return 1;
  case zFeeMarketTx:
    return 2;
  case zBlobTx:
    return 3;
  case zSetCodeTx:
    return 4;
  }
  return 0xff;
}

sail_fixed_bytes_20 evmsail_optimized_create2_address(
    sail_fixed_bytes_20 sender, sail_u256 salt,
    sail_fixed_bytes_32 init_hash) {
  uint8_t *preimage = scratch_borrow(85);
  if (!preimage) {
    sail_fixed_bytes_20 zero = {{0}};
    return (zero);
  }
  preimage[0] = 0xff;
  memcpy(preimage + 1, sender.bytes, sizeof(sender.bytes));
  sail_word_to_be_bytes(preimage + 21, salt);
  memcpy(preimage + 53, init_hash.bytes, sizeof(init_hash.bytes));
  const sail_fixed_bytes_32 digest = keccak_bytes(preimage, 85);
  sail_fixed_bytes_20 address;
  memcpy(address.bytes, digest.bytes + 12, sizeof(address.bytes));
  return (address);
}

sail_fixed_bytes_32 evmsail_optimized_tx_signing_hash(
    enum zTxType tx_type,
    struct zStatelessInputSliceFields content, sail_u256 v) {
  const uint8_t type = tx_type_byte(tx_type);
  const bool typed = type != 0;
  const bool eip155 = !typed && u256_gte_small(v, 35);
  const sail_u256 zero_word = {{0}};
  const sail_u256 chain_id = eip155 ? legacy_chain_id(v) : zero_word;
  const uint64_t suffix_len =
      eip155 ? rlp_quantity_size_u256(chain_id) + 2 : 0;
  const uint64_t content_len = evmsail_byte_quantity_value(content.zlen);
  uint64_t list_content_len = content_len;
  uint64_t total = typed ? 1 : 0;
  if (type > 4 || content_len > (UINT64_C(1) << 30) ||
      !add_length(&list_content_len, suffix_len) ||
      !add_length(&total, rlp_list_prefix_size(list_content_len)) ||
      !add_length(&total, list_content_len) || total > UINT32_MAX)
    return (zero_hash());

  uint8_t *preimage = scratch_borrow(total);
  if (!preimage)
    return (zero_hash());
  const uint64_t content_off = evmsail_byte_quantity_value(content.zoff);
  const uint8_t *content_bytes =
      evmsail_stateless_input_ptr(content_off, content_len);
  if (!content_bytes)
    return (zero_hash());

  uint8_t *cursor = preimage;
  if (typed)
    *cursor++ = type;
  cursor = write_rlp_list_prefix(cursor, list_content_len);
  memmove(cursor, content_bytes, content_len);
  cursor += content_len;
  if (eip155) {
    cursor = write_rlp_u256(cursor, chain_id);
    *cursor++ = 0x80;
    *cursor++ = 0x80;
  }
  if ((uint64_t)(cursor - preimage) != total)
    return (zero_hash());
  return (keccak_bytes(preimage, total));
}

sail_fixed_bytes_32 evmsail_optimized_sha256_request_digest(
    uint64_t request_type,
    struct zStatelessInputSliceFields request) {
  const uint64_t request_len = evmsail_byte_quantity_value(request.zlen);
  if (request_len >= UINT32_MAX)
    return (zero_hash());
  uint8_t *preimage = scratch_borrow(request_len + 1);
  if (!preimage)
    return (zero_hash());
  const uint64_t request_off = evmsail_byte_quantity_value(request.zoff);
  const uint8_t *request_bytes =
      evmsail_stateless_input_ptr(request_off, request_len);
  if (!request_bytes)
    return (zero_hash());
  preimage[0] = (uint8_t)request_type;
  memmove(preimage + 1, request_bytes, request_len);
  return (sha256_bytes(preimage, request_len + 1));
}

static bool request_digest(uint8_t type,
                           struct zStatelessInputSliceFields request,
                           sail_fixed_bytes_32 *digest) {
  const uint64_t len = evmsail_byte_quantity_value(request.zlen);
  if (len == 0)
    return false;
  *digest = evmsail_optimized_sha256_request_digest(type, request);
  return true;
}

sail_fixed_bytes_32 evmsail_optimized_execution_requests_hash(
    struct zStatelessInputRef input_ref) {
  sail_fixed_bytes_32 digests[5];
  uint64_t count = 0;
  if (request_digest(0, input_ref.zdeposits, &digests[count]))
    ++count;
  if (request_digest(1, input_ref.zwithdrawal_requests, &digests[count]))
    ++count;
  if (request_digest(2, input_ref.zconsolidation_requests, &digests[count]))
    ++count;
  if (request_digest(3, input_ref.zbuilder_deposit_requests, &digests[count]))
    ++count;
  if (request_digest(4, input_ref.zbuilder_exit_requests, &digests[count]))
    ++count;
  return (sha256_bytes((const uint8_t *)digests, count * sizeof(*digests)));
}

static const uint8_t empty_ommer_hash[32] = {
    0x1d, 0xcc, 0x4d, 0xe8, 0xde, 0xc7, 0x5d, 0x7a,
    0xab, 0x85, 0xb5, 0x67, 0xb6, 0xcc, 0xd4, 0x1a,
    0xd3, 0x12, 0x45, 0x1b, 0x94, 0x8a, 0x74, 0x13,
    0xf0, 0xa1, 0x42, 0xfd, 0x40, 0xd4, 0x93, 0x47,
};

static uint8_t *write_rlp_fixed(uint8_t *out, const uint8_t *bytes,
                                uint64_t len) {
  out = write_rlp_string_prefix(out, len, bytes[0]);
  memcpy(out, bytes, len);
  return out + len;
}

static uint64_t block_header_content_size(const struct zBlockHeader *header,
                                          uint64_t extra_len,
                                          uint8_t extra_first) {
  uint64_t len = 6 * 33 + 21 + 259 + 1 + 9;
  if (!add_length(&len, rlp_quantity_size_u64(header->znumber)) ||
      !add_length(&len, rlp_quantity_size_u64(header->zgas_limit)) ||
      !add_length(&len, rlp_quantity_size_u64(header->zgas_used)) ||
      !add_length(&len, rlp_quantity_size_u64(header->ztimestamp)))
    return UINT64_MAX;
  const uint64_t extra_size = rlp_string_size(extra_len, extra_first);
  if (extra_size == UINT64_MAX || !add_length(&len, extra_size))
    return UINT64_MAX;
  const uint64_t fork = evmsail_active_fork();
  if (fork >= EVMSAIL_FORK_LONDON &&
      !add_length(&len, rlp_quantity_size_u256(header->zbase_fee)))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_SHANGHAI && !add_length(&len, 33))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_CANCUN &&
      (!add_length(&len, rlp_quantity_size_u64(header->zblob_gas_used)) ||
       !add_length(&len, rlp_quantity_size_u64(header->zexcess_blob_gas)) ||
       !add_length(&len, 33)))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_PRAGUE && !add_length(&len, 33))
    return UINT64_MAX;
  if (fork >= EVMSAIL_FORK_AMSTERDAM &&
      (!add_length(&len, 33) ||
       !add_length(&len, rlp_quantity_size_u64(header->zslot_number))))
    return UINT64_MAX;
  return len;
}

sail_fixed_bytes_32 evmsail_optimized_block_header_hash(
    struct zBlockHeader header,
    sail_fixed_bytes_32 transactions_root, sail_fixed_bytes_32 withdrawals_root,
    sail_fixed_bytes_32 requests_hash,
    sail_fixed_bytes_32 block_access_list_hash) {
  const uint64_t extra_len = evmsail_byte_quantity_value(header.zextra_data.zlen);
  const uint8_t *extra_bytes = NULL;
  uint8_t extra_first = 0;
  if (extra_len != 0) {
    extra_bytes = evmsail_stateless_input_ptr(
        evmsail_byte_quantity_value(header.zextra_data.zoff), extra_len);
    if (!extra_bytes)
      return (zero_hash());
    extra_first = extra_bytes[0];
  }
  const uint8_t *logs_bloom_bytes = evmsail_stateless_input_ptr(
      evmsail_byte_quantity_value(header.zlogs_bloom.zoff),
      evmsail_byte_quantity_value(header.zlogs_bloom.zlen));
  if (!logs_bloom_bytes ||
      evmsail_byte_quantity_value(header.zlogs_bloom.zlen) != 256)
    return (zero_hash());
  const uint64_t content_len =
      block_header_content_size(&header, extra_len, extra_first);
  if (content_len == UINT64_MAX || content_len > 749)
    return (zero_hash());
  uint64_t total = content_len;
  if (!add_length(&total, rlp_list_prefix_size(content_len)) ||
      total > UINT32_MAX)
    return (zero_hash());

  uint8_t *preimage = scratch_borrow(total);
  if (!preimage)
    return (zero_hash());
  uint8_t prev_randao[32];
  const uint64_t fork = evmsail_active_fork();
  sail_word_to_be_bytes(prev_randao, header.zprev_randao);
  uint8_t *cursor = write_rlp_list_prefix(preimage, content_len);
  cursor = write_rlp_fixed(cursor, header.zparent_hash.bytes, 32);
  cursor = write_rlp_fixed(cursor, empty_ommer_hash, 32);
  cursor = write_rlp_fixed(cursor, header.zfee_recipient.bytes, 20);
  cursor = write_rlp_fixed(cursor, header.zstate_root.bytes, 32);
  cursor = write_rlp_fixed(cursor, transactions_root.bytes, 32);
  cursor = write_rlp_fixed(cursor, header.zreceipts_root.bytes, 32);
  cursor = write_rlp_fixed(cursor, logs_bloom_bytes, 256);
  cursor = write_rlp_u64(cursor, 0);
  cursor = write_rlp_u64(cursor, header.znumber);
  cursor = write_rlp_u64(cursor, header.zgas_limit);
  cursor = write_rlp_u64(cursor, header.zgas_used);
  cursor = write_rlp_u64(cursor, header.ztimestamp);
  cursor = write_rlp_string_prefix(cursor, extra_len, extra_first);
  if (extra_len != 0)
    memmove(cursor, extra_bytes, extra_len);
  cursor += extra_len;
  cursor = write_rlp_fixed(cursor, prev_randao, 32);
  *cursor++ = 0x88;
  memset(cursor, 0, 8);
  cursor += 8;
  if (fork >= EVMSAIL_FORK_LONDON)
    cursor = write_rlp_u256(cursor, header.zbase_fee);
  if (fork >= EVMSAIL_FORK_SHANGHAI)
    cursor = write_rlp_fixed(cursor, withdrawals_root.bytes, 32);
  if (fork >= EVMSAIL_FORK_CANCUN) {
    cursor = write_rlp_u64(cursor, header.zblob_gas_used);
    cursor = write_rlp_u64(cursor, header.zexcess_blob_gas);
    cursor =
        write_rlp_fixed(cursor, header.zparent_beacon_block_root.bytes, 32);
  }
  if (fork >= EVMSAIL_FORK_PRAGUE)
    cursor = write_rlp_fixed(cursor, requests_hash.bytes, 32);
  if (fork >= EVMSAIL_FORK_AMSTERDAM) {
    cursor = write_rlp_fixed(cursor, block_access_list_hash.bytes, 32);
    cursor = write_rlp_u64(cursor, header.zslot_number);
  }
  if ((uint64_t)(cursor - preimage) != total)
    return (zero_hash());
  return (keccak_bytes(preimage, total));
}
