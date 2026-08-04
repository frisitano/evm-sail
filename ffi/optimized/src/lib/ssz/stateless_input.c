/* Optimized SSZ stateless-input and withdrawal decoding. */
#include "evmsail/prelude.h"

#include "evmsail/lib/ssz/stateless_input.h"
#include "evmsail/exceptions.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"
#include "lib/mpt/trie.h"
#include "lib/rlp/encoding.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

unit index_witness_nodes(struct BoundedSszListRef nodes) {
  return mpt_index_witness_nodes(nodes);
}

unit index_witness_codes(struct BoundedSszListRef codes,
                                 bool amsterdam_or_later) {
  return mpt_index_witness_codes(codes, amsterdam_or_later);
}

static struct StatelessInputRef ssz_input_failure(const char *location) {
  struct StatelessInputRef result;
  memset(&result, 0, sizeof(result));
  throw_invalid_block(InvalidConfig, location);
  return result;
}

static struct Withdrawal ssz_withdrawal_failure(const char *location) {
  struct Withdrawal result;
  memset(&result, 0, sizeof(result));
  throw_invalid_block(InvalidConfig, location);
  return result;
}

static uint32_t load_le_u32(const uint8_t *bytes) {
  return ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) |
         ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static uint64_t load_le_u64(const uint8_t *bytes) {
  uint64_t result = 0;
  for (uint64_t i = 0; i < 8; ++i)
    result |= (uint64_t)bytes[i] << (8 * i);
  return result;
}

U256 ssz_u256(struct StatelessInputSliceFields input,
                   uint32_t offset) {
  if (offset > input.len || UINT64_C(32) > input.len - offset) {
    throw_invalid_block(InvalidConfig, "optimized SSZ uint256");
    return (U256){.limbs = {0, 0, 0, 0}};
  }
  const uint8_t *source = input.bytes + offset;
  return (U256){
      .limbs = {
          load_le_u64(source),
          load_le_u64(source + 8),
          load_le_u64(source + 16),
          load_le_u64(source + 24),
      },
  };
}

static struct StatelessInputSliceFields source_span(
    struct StatelessInputSliceFields container, uint64_t start,
    uint64_t stop) {
  struct StatelessInputSliceFields result = {
      .bytes = container.bytes + start,
      .len = stop - start,
  };
  return result;
}

static bool offsets_ordered(const uint32_t *offsets, uint64_t count,
                            uint64_t first, uint64_t length) {
  if (count == 0 || offsets[0] != first)
    return false;
  for (uint64_t i = 0; i < count; ++i) {
    if (offsets[i] < first || offsets[i] > length)
      return false;
    if (i != 0 && offsets[i] < offsets[i - 1])
      return false;
  }
  return true;
}

static bool variable_list_ref(struct StatelessInputSliceFields bytes,
                              const uint8_t *source, uint64_t maximum_count,
                              uint64_t maximum_item_length,
                              struct BoundedSszListRef *result) {
  uint64_t count = 0;
  if (bytes.len != 0) {
    if (bytes.len < 4)
      return false;
    const uint64_t first_offset = load_le_u32(source);
    if (first_offset == 0 || (first_offset & 3) != 0 ||
        first_offset > bytes.len)
      return false;
    count = first_offset / 4;
  }
  if (count > maximum_count)
    return false;
  result->bytes = bytes;
  result->count = count;
  result->max_item_length = maximum_item_length;
  return true;
}

static bool fixed_list_ref(struct StatelessInputSliceFields bytes,
                           uint64_t width, uint64_t maximum_count,
                           struct BoundedSszListRef *result) {
  if (width == 0 || bytes.len % width != 0)
    return false;
  const uint64_t count = bytes.len / width;
  if (count > maximum_count)
    return false;
  result->bytes = bytes;
  result->count = count;
  result->max_item_length = width;
  return true;
}

struct StatelessInputRef decode_stateless_input_ref(
    struct StatelessInputSliceFields input) {
  enum {
    STATELESS_INPUT_FIXED_LENGTH = 18,
    SSZ_BODY = 2,
    BODY_FIXED_LENGTH = 16,
    NEW_PAYLOAD_REQUEST_FIXED_LENGTH = 44,
    EXECUTION_PAYLOAD_FIXED_LENGTH = 540,
    EXECUTION_REQUESTS_FIXED_LENGTH = 20,
    EXECUTION_WITNESS_FIXED_LENGTH = 12,
    PUBLIC_KEY_LENGTH = 65,
    WITHDRAWAL_LENGTH = 44,
  };
  const uint64_t MAX_TRANSACTIONS = UINT64_C(1) << 20;
  const uint64_t MAX_TRANSACTION_LENGTH = UINT64_C(1) << 30;
  const uint64_t MAX_WITHDRAWALS = UINT64_C(1) << 4;
  const uint64_t MAX_EXTRA_DATA_LENGTH = UINT64_C(1) << 5;
  const uint64_t MAX_BLOCK_ACCESS_LIST_LENGTH = UINT64_C(1) << 30;
  const uint64_t MAX_BLOB_COMMITMENTS = UINT64_C(1) << 12;
  const uint64_t MAX_WITNESS_NODES = UINT64_C(1) << 22;
  const uint64_t MAX_WITNESS_NODE_LENGTH = UINT64_C(1) << 10;
  const uint64_t MAX_WITNESS_CODES = UINT64_C(1) << 18;
  const uint64_t MAX_WITNESS_CODE_LENGTH = UINT64_C(1) << 16;
  const uint64_t MAX_WITNESS_HEADERS = UINT64_C(1) << 8;
  const uint64_t MAX_WITNESS_HEADER_LENGTH = UINT64_C(1) << 10;
  const uint64_t MAX_PUBLIC_KEYS = UINT64_C(1) << 15;

  struct StatelessInputRef result;
  memset(&result, 0, sizeof(result));
  const uint8_t *source = input.bytes;
  if (!source || input.len < STATELESS_INPUT_FIXED_LENGTH ||
      source[1] != 0x01 || source[0] < 0x0a || source[0] > 0x15)
    return ssz_input_failure("optimized stateless input header");
  result.protocol = schema_protocol_profile_forwards(source[0]);

  const struct StatelessInputSliceFields body =
      source_span(input, SSZ_BODY, input.len);
  const uint8_t *body_source = source + SSZ_BODY;
  const uint64_t body_length = body.len;
  const uint32_t body_offsets[4] = {
      load_le_u32(body_source), load_le_u32(body_source + 4),
      load_le_u32(body_source + 8), load_le_u32(body_source + 12)};
  if (!offsets_ordered(body_offsets, 4, BODY_FIXED_LENGTH, body_length))
    return ssz_input_failure("optimized stateless input offsets");

  result.new_payload_request =
      source_span(body, body_offsets[0], body_offsets[1]);
  const struct StatelessInputSliceFields witness =
      source_span(body, body_offsets[1], body_offsets[2]);
  result.chain_config =
      source_span(body, body_offsets[2], body_offsets[3]);
  result.public_keys =
      source_span(body, body_offsets[3], body_length);

  if (result.new_payload_request.len <
      NEW_PAYLOAD_REQUEST_FIXED_LENGTH)
    return ssz_input_failure("optimized new payload request");
  const uint8_t *npr_source = body_source + body_offsets[0];
  const uint32_t npr_offsets[3] = {
      load_le_u32(npr_source), load_le_u32(npr_source + 4),
      load_le_u32(npr_source + 40)};
  if (!offsets_ordered(npr_offsets, 3, NEW_PAYLOAD_REQUEST_FIXED_LENGTH,
                       result.new_payload_request.len))
    return ssz_input_failure("optimized new payload request offsets");
  result.execution_payload =
      source_span(result.new_payload_request, npr_offsets[0],
                  npr_offsets[1]);
  result.versioned_hashes =
      source_span(result.new_payload_request, npr_offsets[1],
                  npr_offsets[2]);
  const struct StatelessInputSliceFields requests =
      source_span(result.new_payload_request, npr_offsets[2],
                  result.new_payload_request.len);

  if (result.execution_payload.len < EXECUTION_PAYLOAD_FIXED_LENGTH)
    return ssz_input_failure("optimized execution payload");
  const uint8_t *payload_source = npr_source + npr_offsets[0];
  const uint32_t payload_offsets[4] = {
      load_le_u32(payload_source + 436),
      load_le_u32(payload_source + 504),
      load_le_u32(payload_source + 508),
      load_le_u32(payload_source + 528)};
  if (!offsets_ordered(payload_offsets, 4, EXECUTION_PAYLOAD_FIXED_LENGTH,
                       result.execution_payload.len))
    return ssz_input_failure("optimized execution payload offsets");
  result.extra_data =
      source_span(result.execution_payload, payload_offsets[0],
                  payload_offsets[1]);
  const struct StatelessInputSliceFields transactions =
      source_span(result.execution_payload, payload_offsets[1],
                  payload_offsets[2]);
  const struct StatelessInputSliceFields withdrawals =
      source_span(result.execution_payload, payload_offsets[2],
                  payload_offsets[3]);
  result.block_access_list =
      source_span(result.execution_payload, payload_offsets[3],
                  result.execution_payload.len);
  if (result.extra_data.len > MAX_EXTRA_DATA_LENGTH ||
      result.block_access_list.len > MAX_BLOCK_ACCESS_LIST_LENGTH ||
      !variable_list_ref(transactions,
                         payload_source + payload_offsets[1],
                         MAX_TRANSACTIONS, MAX_TRANSACTION_LENGTH,
                         &result.transactions) ||
      !fixed_list_ref(withdrawals, WITHDRAWAL_LENGTH, MAX_WITHDRAWALS,
                      &result.withdrawals))
    return ssz_input_failure("optimized execution payload collections");

  if (requests.len < EXECUTION_REQUESTS_FIXED_LENGTH)
    return ssz_input_failure("optimized execution requests");
  const uint8_t *requests_source =
      npr_source + npr_offsets[2];
  const uint32_t request_offsets[5] = {
      load_le_u32(requests_source), load_le_u32(requests_source + 4),
      load_le_u32(requests_source + 8),
      load_le_u32(requests_source + 12),
      load_le_u32(requests_source + 16)};
  if (!offsets_ordered(request_offsets, 5, EXECUTION_REQUESTS_FIXED_LENGTH,
                       requests.len))
    return ssz_input_failure("optimized execution request offsets");
  result.deposits =
      source_span(requests, request_offsets[0], request_offsets[1]);
  result.withdrawal_requests =
      source_span(requests, request_offsets[1], request_offsets[2]);
  result.consolidation_requests =
      source_span(requests, request_offsets[2], request_offsets[3]);
  result.builder_deposit_requests =
      source_span(requests, request_offsets[3], request_offsets[4]);
  result.builder_exit_requests =
      source_span(requests, request_offsets[4], requests.len);

  if (witness.len < EXECUTION_WITNESS_FIXED_LENGTH)
    return ssz_input_failure("optimized execution witness");
  const uint8_t *witness_source = body_source + body_offsets[1];
  const uint32_t witness_offsets[3] = {
      load_le_u32(witness_source), load_le_u32(witness_source + 4),
      load_le_u32(witness_source + 8)};
  if (!offsets_ordered(witness_offsets, 3, EXECUTION_WITNESS_FIXED_LENGTH,
                       witness.len))
    return ssz_input_failure("optimized execution witness offsets");
  const struct StatelessInputSliceFields witness_state =
      source_span(witness, witness_offsets[0], witness_offsets[1]);
  const struct StatelessInputSliceFields witness_codes =
      source_span(witness, witness_offsets[1], witness_offsets[2]);
  const struct StatelessInputSliceFields witness_headers =
      source_span(witness, witness_offsets[2], witness.len);
  if (!variable_list_ref(witness_state,
                         witness_source + witness_offsets[0],
                         MAX_WITNESS_NODES, MAX_WITNESS_NODE_LENGTH,
                         &result.witness_state) ||
      !variable_list_ref(witness_codes,
                         witness_source + witness_offsets[1],
                         MAX_WITNESS_CODES, MAX_WITNESS_CODE_LENGTH,
                         &result.witness_codes) ||
      !variable_list_ref(witness_headers,
                         witness_source + witness_offsets[2],
                         MAX_WITNESS_HEADERS, MAX_WITNESS_HEADER_LENGTH,
                         &result.witness_headers))
    return ssz_input_failure("optimized execution witness collections");

  if (result.public_keys.len % PUBLIC_KEY_LENGTH != 0 ||
      result.public_keys.len / PUBLIC_KEY_LENGTH > MAX_PUBLIC_KEYS) {
    return ssz_input_failure("optimized public keys");
  }
  struct BoundedSszListRef ignored;
  if (!fixed_list_ref(result.versioned_hashes, 32,
                      MAX_BLOB_COMMITMENTS, &ignored))
    return ssz_input_failure("optimized versioned hashes");
  return result;
}

struct Withdrawal decode_withdrawal(
    struct StatelessInputSliceFields withdrawal) {
  struct Withdrawal result;
  memset(&result, 0, sizeof(result));
  const uint8_t *source = withdrawal.bytes;
  if (!source || withdrawal.len != 44)
    return ssz_withdrawal_failure("optimized withdrawal");
  result.index = load_le_u64(source);
  result.validator_index = load_le_u64(source + 8);
  result.address = address_from_be_bytes(source + 16);
  result.amount = load_le_u64(source + 36);
  return result;
}

Hash32 sha256_request_digest(
    uint64_t request_type, struct StatelessInputSliceFields request) {
  const uint64_t request_len = request.len;
  if (request_len >= UINT32_MAX)
    return zero_hash();
  uint8_t *preimage = scratch_borrow(request_len + 1);
  if (!preimage)
    return zero_hash();
  const uint8_t *request_bytes = request.bytes;
  if (!request_bytes)
    return zero_hash();
  preimage[0] = (uint8_t)request_type;
  memmove(preimage + 1, request_bytes, request_len);
  return sha256_bytes(preimage, request_len + 1);
}
