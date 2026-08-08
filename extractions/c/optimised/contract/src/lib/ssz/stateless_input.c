/* Optimized SSZ stateless-input and withdrawal decoding. */
#include "evmsail/prelude.h"

#include "evmsail/lib/ssz/stateless_input.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"
#include "evmsail/spec/exceptions.h"
#include "evmsail/spec/lib/ssz/stateless_input.h"
#include "evmsail/spec/main.h"
#include "evmsail/spec/primitives/chain_config.h"
#include "evmsail/spec/primitives/block.h"
#include "lib/mpt/trie.h"
#include "lib/rlp/encoding.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void index_witness_nodes(struct BoundedSszListRef nodes)
{
  mpt_index_witness_nodes(nodes);
}

void index_witness_codes(struct BoundedSszListRef codes, bool amsterdam_or_later)
{
  mpt_index_witness_codes(codes, amsterdam_or_later);
}

static uint32_t load_le_u32(const uint8_t *bytes)
{
  return ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) |
         ((uint32_t)bytes[3] << 24);
}

static uint64_t load_le_u64(const uint8_t *bytes)
{
  uint64_t result = 0;
  for (uint64_t i = 0; i < 8; ++i) {
    result |= (uint64_t)bytes[i] << (8 * i);
  }
  return result;
}

/* The sole call site reads offset 440 inside the execution payload's
 * validated 540-byte fixed section, so no bounds re-check is needed. */
u256 ssz_u256(Bytes input, uint32_t offset)
{
  const uint8_t *source = input.bytes + offset;
  return (u256){
      .limbs =
          {
              load_le_u64(source),
              load_le_u64(source + 8),
              load_le_u64(source + 16),
              load_le_u64(source + 24),
          },
  };
}

static Bytes source_span(Bytes container, uint32_t start, uint32_t stop)
{
  Bytes result = {
      .bytes = container.bytes + start,
      .len = stop - start,
  };
  return result;
}

/* Every caller passes a literal nonzero count and pre-checks the container
 * length against `first`; monotonicity carries `first` forward, so
 * validation starts at index 1. */
static bool offsets_ordered(const uint32_t *offsets, uint32_t count, uint32_t first,
                            uint32_t length)
{
  if (offsets[0] != first) {
    return false;
  }
  for (uint32_t i = 1; i < count; ++i) {
    if (offsets[i] > length || offsets[i] < offsets[i - 1]) {
      return false;
    }
  }
  return true;
}

static bool variable_list_ref(Bytes bytes, uint32_t maximum_count, uint32_t maximum_item_length,
                              struct BoundedSszListRef *result)
{
  uint32_t count = 0;
  if (bytes.len != 0) {
    if (bytes.len < 4) {
      return false;
    }
    const uint32_t first_offset = load_le_u32(bytes.bytes);
    if (first_offset == 0 || (first_offset & 3) != 0 || first_offset > bytes.len) {
      return false;
    }
    count = first_offset / 4;
  }
  if (count > maximum_count) {
    return false;
  }
  result->bytes = bytes;
  result->count = count;
  result->max_item_length = maximum_item_length;
  return true;
}

static bool fixed_list_ref(Bytes bytes, uint32_t width, uint32_t maximum_count,
                           struct BoundedSszListRef *result)
{
  if (bytes.len % width != 0) {
    return false;
  }
  const uint32_t count = bytes.len / width;
  if (count > maximum_count) {
    return false;
  }
  result->bytes = bytes;
  result->count = count;
  result->max_item_length = width;
  return true;
}

struct StatelessInputRef decode_stateless_input_ref(Bytes input)
{
  enum {
    STATELESS_INPUT_FIXED_LENGTH = 18,
    SSZ_BODY = 2,
    BODY_FIXED_LENGTH = 16,
    NEW_PAYLOAD_REQUEST_FIXED_LENGTH = 44,
    EXECUTION_PAYLOAD_FIXED_LENGTH = 540,
    EXECUTION_REQUESTS_FIXED_LENGTH = 20,
    EXECUTION_WITNESS_FIXED_LENGTH = 12,
    SSZ_PUBLIC_KEY_LENGTH = 65,
    WITHDRAWAL_LENGTH = 44,
  };
  const uint32_t MAX_TRANSACTIONS = UINT32_C(1) << 20;
  const uint32_t MAX_TRANSACTION_LENGTH = UINT32_C(1) << 30;
  const uint32_t MAX_WITHDRAWALS = UINT32_C(1) << 4;
  const uint32_t MAX_EXTRA_DATA_LENGTH = UINT32_C(1) << 5;
  const uint32_t MAX_BLOCK_ACCESS_LIST_LENGTH = UINT32_C(1) << 30;
  const uint32_t MAX_BLOB_COMMITMENTS = UINT32_C(1) << 12;
  const uint32_t MAX_WITNESS_NODES = UINT32_C(1) << 22;
  const uint32_t MAX_WITNESS_NODE_LENGTH = UINT32_C(1) << 10;
  const uint32_t MAX_WITNESS_CODES = UINT32_C(1) << 18;
  const uint32_t MAX_WITNESS_CODE_LENGTH = UINT32_C(1) << 16;
  const uint32_t MAX_WITNESS_HEADERS = UINT32_C(1) << 8;
  const uint32_t MAX_WITNESS_HEADER_LENGTH = UINT32_C(1) << 10;
  const uint32_t MAX_PUBLIC_KEYS = UINT32_C(1) << 15;

  struct StatelessInputRef result;
  memset(&result, 0, sizeof(result));
  const uint8_t *source = input.bytes;
  if (!source || input.len < STATELESS_INPUT_FIXED_LENGTH || source[1] != 0x01 ||
      source[0] < 0x0a || source[0] > 0x15) {
    fatal_error(InvalidConfig);
  }
  result.protocol = schema_protocol_profile(source[0]);

  const Bytes body = source_span(input, SSZ_BODY, input.len);
  const uint8_t *body_source = source + SSZ_BODY;
  const uint32_t body_length = body.len;
  const uint32_t body_offsets[4] = {load_le_u32(body_source), load_le_u32(body_source + 4),
                                    load_le_u32(body_source + 8), load_le_u32(body_source + 12)};
  if (!offsets_ordered(body_offsets, 4, BODY_FIXED_LENGTH, body_length)) {
    fatal_error(InvalidConfig);
  }

  result.new_payload_request = source_span(body, body_offsets[0], body_offsets[1]);
  const Bytes witness = source_span(body, body_offsets[1], body_offsets[2]);
  result.chain_config = source_span(body, body_offsets[2], body_offsets[3]);
  result.public_keys = source_span(body, body_offsets[3], body_length);

  if (result.new_payload_request.len < NEW_PAYLOAD_REQUEST_FIXED_LENGTH) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *npr_source = body_source + body_offsets[0];
  const uint32_t npr_offsets[3] = {load_le_u32(npr_source), load_le_u32(npr_source + 4),
                                   load_le_u32(npr_source + 40)};
  if (!offsets_ordered(npr_offsets, 3, NEW_PAYLOAD_REQUEST_FIXED_LENGTH,
                       result.new_payload_request.len)) {
    fatal_error(InvalidConfig);
  }
  result.execution_payload =
      source_span(result.new_payload_request, npr_offsets[0], npr_offsets[1]);
  result.versioned_hashes = source_span(result.new_payload_request, npr_offsets[1], npr_offsets[2]);
  const Bytes requests =
      source_span(result.new_payload_request, npr_offsets[2], result.new_payload_request.len);

  if (result.execution_payload.len < EXECUTION_PAYLOAD_FIXED_LENGTH) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *payload_source = npr_source + npr_offsets[0];
  const uint32_t payload_offsets[4] = {
      load_le_u32(payload_source + 436), load_le_u32(payload_source + 504),
      load_le_u32(payload_source + 508), load_le_u32(payload_source + 528)};
  if (!offsets_ordered(payload_offsets, 4, EXECUTION_PAYLOAD_FIXED_LENGTH,
                       result.execution_payload.len)) {
    fatal_error(InvalidConfig);
  }
  result.extra_data = source_span(result.execution_payload, payload_offsets[0], payload_offsets[1]);
  const Bytes transactions =
      source_span(result.execution_payload, payload_offsets[1], payload_offsets[2]);
  const Bytes withdrawals =
      source_span(result.execution_payload, payload_offsets[2], payload_offsets[3]);
  result.block_access_list =
      source_span(result.execution_payload, payload_offsets[3], result.execution_payload.len);
  if (result.extra_data.len > MAX_EXTRA_DATA_LENGTH ||
      result.block_access_list.len > MAX_BLOCK_ACCESS_LIST_LENGTH ||
      !variable_list_ref(transactions, MAX_TRANSACTIONS, MAX_TRANSACTION_LENGTH,
                         &result.transactions) ||
      !fixed_list_ref(withdrawals, WITHDRAWAL_LENGTH, MAX_WITHDRAWALS, &result.withdrawals)) {
    fatal_error(InvalidConfig);
  }

  if (requests.len < EXECUTION_REQUESTS_FIXED_LENGTH) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *requests_source = npr_source + npr_offsets[2];
  const uint32_t request_offsets[5] = {
      load_le_u32(requests_source), load_le_u32(requests_source + 4),
      load_le_u32(requests_source + 8), load_le_u32(requests_source + 12),
      load_le_u32(requests_source + 16)};
  if (!offsets_ordered(request_offsets, 5, EXECUTION_REQUESTS_FIXED_LENGTH, requests.len)) {
    fatal_error(InvalidConfig);
  }
  result.deposits = source_span(requests, request_offsets[0], request_offsets[1]);
  result.withdrawal_requests = source_span(requests, request_offsets[1], request_offsets[2]);
  result.consolidation_requests = source_span(requests, request_offsets[2], request_offsets[3]);
  result.builder_deposit_requests = source_span(requests, request_offsets[3], request_offsets[4]);
  result.builder_exit_requests = source_span(requests, request_offsets[4], requests.len);

  if (witness.len < EXECUTION_WITNESS_FIXED_LENGTH) {
    fatal_error(InvalidConfig);
  }
  const uint8_t *witness_source = body_source + body_offsets[1];
  const uint32_t witness_offsets[3] = {load_le_u32(witness_source), load_le_u32(witness_source + 4),
                                       load_le_u32(witness_source + 8)};
  if (!offsets_ordered(witness_offsets, 3, EXECUTION_WITNESS_FIXED_LENGTH, witness.len)) {
    fatal_error(InvalidConfig);
  }
  const Bytes witness_state = source_span(witness, witness_offsets[0], witness_offsets[1]);
  const Bytes witness_codes = source_span(witness, witness_offsets[1], witness_offsets[2]);
  const Bytes witness_headers = source_span(witness, witness_offsets[2], witness.len);
  if (!variable_list_ref(witness_state, MAX_WITNESS_NODES, MAX_WITNESS_NODE_LENGTH,
                         &result.witness_state) ||
      !variable_list_ref(witness_codes, MAX_WITNESS_CODES, MAX_WITNESS_CODE_LENGTH,
                         &result.witness_codes) ||
      !variable_list_ref(witness_headers, MAX_WITNESS_HEADERS, MAX_WITNESS_HEADER_LENGTH,
                         &result.witness_headers)) {
    fatal_error(InvalidConfig);
  }

  if (result.public_keys.len % SSZ_PUBLIC_KEY_LENGTH != 0 ||
      result.public_keys.len / SSZ_PUBLIC_KEY_LENGTH > MAX_PUBLIC_KEYS) {
    fatal_error(InvalidConfig);
  }
  struct BoundedSszListRef ignored;
  if (!fixed_list_ref(result.versioned_hashes, 32, MAX_BLOB_COMMITMENTS, &ignored)) {
    fatal_error(InvalidConfig);
  }
  fatal_error_set_input(result);
  return result;
}

/* The parameter is StatelessInputSliceLength(44) by type:
 * ssz_fixed_list_pop slices exactly 44 bytes over a 44-stride list. */
struct Withdrawal decode_withdrawal(Bytes withdrawal)
{
  struct Withdrawal result;
  const uint8_t *source = withdrawal.bytes;
  result.index = load_le_u64(source);
  result.validator_index = load_le_u64(source + 8);
  result.address = address_from_be_bytes(source + 16);
  result.amount = load_le_u64(source + 36);
  return result;
}

bytes32 sha256_request_digest(uint64_t request_type, Bytes request)
{
  const uint64_t request_len = request.len;
  uint8_t *preimage = scratch_borrow(request_len + 1);
  if (!preimage) {
    return zero_hash();
  }
  preimage[0] = (uint8_t)request_type;
  memmove(preimage + 1, request.bytes, request_len);
  return sha256_bytes(preimage, request_len + 1);
}
