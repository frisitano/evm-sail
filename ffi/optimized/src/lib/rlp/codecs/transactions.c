/* Optimized whole-transaction RLP decoding. */
#include "evmsail/prelude.h"

#include "evmsail/lib/rlp/codecs/transactions.h"
#include "evmsail/lib/rlp/codecs/transaction_signing.h"
#include "evmsail/exceptions.h"
#include "evmsail/host/region_access.h"
#include "evmsail/primitives/crypto.h"
#include "primitives/value.h"
#include "primitives/hash.h"
#include "evmsail/primitives/word.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  const uint8_t *source;
  const uint8_t *content;
  uint64_t source_off;
  uint64_t content_off;
  uint64_t source_len;
  uint64_t content_len;
  bool is_list;
  bool canonical;
} rlp_item;

typedef struct {
  const uint8_t *next;
  uint64_t next_off;
  uint64_t remaining;
} rlp_cursor;

enum { TX_FIELD_ABSENT = UINT8_MAX };

/* Physical field positions for the five canonical transaction envelopes.
 * Sail owns the semantic transaction-type classification. This table is only
 * the optimized decoder's defunctionalized wire-layout data: each byte is an
 * index into the already validated RLP field array, or TX_FIELD_ABSENT. */
typedef struct {
  uint8_t field_count;
  uint8_t nonce;
  uint8_t gas;
  uint8_t recipient;
  uint8_t value;
  uint8_t data;
  uint8_t v;
  uint8_t r;
  uint8_t s;
  uint8_t chain_id;
  uint8_t access_list;
  uint8_t blob_hashes;
  uint8_t authorizations;
  uint8_t gas_price;
  uint8_t max_priority_fee;
  uint8_t max_fee;
  uint8_t max_blob_fee;
  uint8_t nonce_is_u64;
} TxFieldLayout;

#define TX_NO_FIELD TX_FIELD_ABSENT
static const TxFieldLayout TX_FIELD_LAYOUTS[5] = {
    /* LegacyTx */
    {9, 0, 2, 3, 4, 5, 6, 7, 8, TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD,
     TX_NO_FIELD, 1, TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD, false},
    /* AccessListTx */
    {11, 1, 3, 4, 5, 6, 8, 9, 10, 0, 7, TX_NO_FIELD, TX_NO_FIELD, 2,
     TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD, false},
    /* FeeMarketTx */
    {12, 1, 4, 5, 6, 7, 9, 10, 11, 0, 8, TX_NO_FIELD, TX_NO_FIELD,
     TX_NO_FIELD, 2, 3, TX_NO_FIELD, false},
    /* BlobTx */
    {14, 1, 4, 5, 6, 7, 11, 12, 13, 0, 8, 10, TX_NO_FIELD, TX_NO_FIELD,
     2, 3, 9, false},
    /* SetCodeTx */
    {13, 1, 4, 5, 6, 7, 10, 11, 12, 0, 8, TX_NO_FIELD, 9, TX_NO_FIELD,
     2, 3, TX_NO_FIELD, true},
};
#undef TX_NO_FIELD

static const rlp_item *tx_field(const rlp_item *fields, uint8_t index) {
  return index == TX_FIELD_ABSENT ? NULL : &fields[index];
}

static struct TransactionFields decode_failure(enum BlockError reason,
                                                const char *location) {
  struct TransactionFields result;
  memset(&result, 0, sizeof(result));
  throw_invalid_block(reason, location);
  return result;
}

static bool parse_be_length(const uint8_t *source, uint64_t available,
                            uint64_t width, uint64_t *length,
                            bool *minimal) {
  if (width == 0 || width > 8 || available < width)
    return false;
  uint64_t value = 0;
  for (uint64_t i = 0; i < width; ++i)
    value = (value << 8) | source[i];
  *length = value;
  *minimal = source[0] != 0;
  return true;
}

static bool parse_item(const uint8_t *source, uint64_t source_off,
                       uint64_t available, rlp_item *item) {
  if (available == 0)
    return false;

  const uint8_t header = source[0];
  uint64_t content_offset = 0;
  uint64_t content_len = 0;
  bool is_list = false;
  bool canonical = true;

  if (header < 0x80) {
    content_len = 1;
  } else if (header <= 0xb7) {
    content_offset = 1;
    content_len = (uint64_t)header - 0x80;
  } else if (header <= 0xbf) {
    const uint64_t width = (uint64_t)header - 0xb7;
    bool minimal = false;
    content_offset = 1 + width;
    if (!parse_be_length(source + 1, available - 1, width, &content_len,
                         &minimal))
      return false;
    canonical = minimal && content_len > 55;
  } else if (header <= 0xf7) {
    is_list = true;
    content_offset = 1;
    content_len = (uint64_t)header - 0xc0;
  } else {
    const uint64_t width = (uint64_t)header - 0xf7;
    bool minimal = false;
    is_list = true;
    content_offset = 1 + width;
    if (!parse_be_length(source + 1, available - 1, width, &content_len,
                         &minimal))
      return false;
    canonical = minimal && content_len > 55;
  }

  if (content_offset > available ||
      content_len > available - content_offset)
    return false;
  if (!is_list && header >= 0x80 && header <= 0xb7 &&
      content_len == 1 && source[content_offset] < 0x80)
    canonical = false;

  item->source = source;
  item->content = source + content_offset;
  item->source_off = source_off;
  item->content_off = source_off + content_offset;
  item->source_len = content_offset + content_len;
  item->content_len = content_len;
  item->is_list = is_list;
  item->canonical = canonical;
  return true;
}

static bool take_item(rlp_cursor *cursor, rlp_item *item) {
  if (!parse_item(cursor->next, cursor->next_off, cursor->remaining, item))
    return false;
  cursor->next += item->source_len;
  cursor->next_off += item->source_len;
  cursor->remaining -= item->source_len;
  return true;
}

static bool item_list(const rlp_item *item, rlp_cursor *children) {
  if (!item->is_list)
    return false;
  children->next = item->content;
  children->next_off = item->content_off;
  children->remaining = item->content_len;
  return true;
}

static bool exact_list(const uint8_t *source, uint64_t source_off,
                       uint64_t source_len, rlp_cursor *children) {
  rlp_item root;
  return parse_item(source, source_off, source_len, &root) &&
         root.source_len == source_len && item_list(&root, children);
}

static bool take_fields(rlp_cursor children, rlp_item *fields,
                        uint64_t field_count) {
  for (uint64_t i = 0; i < field_count; ++i)
    if (!take_item(&children, &fields[i]))
      return false;
  return children.remaining == 0;
}

static bool word_raw(const rlp_item *item, U256 *value) {
  if (item->is_list || item->content_len > 32)
    return false;
  uint8_t bytes[32] = {0};
  memcpy(bytes + 32 - item->content_len, item->content, item->content_len);
  *value = be_bytes_to_sail_word(bytes);
  return true;
}

static bool quantity_u256(const rlp_item *item, U256 *value) {
  return item->canonical &&
         (item->content_len == 0 || item->content[0] != 0) &&
         word_raw(item, value);
}

static bool quantity_u64(const rlp_item *item, uint64_t *value) {
  if (item->is_list || !item->canonical || item->content_len > 8 ||
      (item->content_len != 0 && item->content[0] == 0))
    return false;
  uint64_t result = 0;
  for (uint64_t i = 0; i < item->content_len; ++i)
    result = (result << 8) | item->content[i];
  *value = result;
  return true;
}

static struct StatelessInputSliceFields content_slice(const rlp_item *item) {
  struct StatelessInputSliceFields result = {
      .off = item->content_off,
      .len = item->content_len,
  };
  return result;
}

static Address low_address(U256 value) {
  return address_from_word(value);
}

static bool decode_access_list(const rlp_item *field,
                               struct AccessListRef *result) {
  rlp_cursor entries;
  if (!item_list(field, &entries))
    return false;

  uint64_t address_count = 0;
  uint64_t slot_count = 0;
  while (entries.remaining != 0) {
    rlp_item entry;
    rlp_item entry_fields[2];
    rlp_cursor entry_children;
    rlp_cursor keys;
    U256 ignored;
    if (!take_item(&entries, &entry) ||
        !item_list(&entry, &entry_children) ||
        !take_fields(entry_children, entry_fields, 2) ||
        !word_raw(&entry_fields[0], &ignored) ||
        !item_list(&entry_fields[1], &keys))
      return false;
    ++address_count;
    while (keys.remaining != 0) {
      rlp_item key;
      if (!take_item(&keys, &key) || !word_raw(&key, &ignored))
        return false;
      ++slot_count;
    }
  }

  result->encoded = content_slice(field);
  result->address_count = address_count;
  result->slot_count = slot_count;
  return true;
}

static bool decode_blob_hashes(const rlp_item *field, uint64_t blob_limit,
                               struct BlobHashesFields *result,
                               enum BlockError *error) {
  rlp_cursor hashes;
  if (!item_list(field, &hashes))
    return false;

  uint64_t count = 0;
  while (hashes.remaining != 0) {
    rlp_item hash;
    if (!take_item(&hashes, &hash) || hash.is_list ||
        hash.source_len != 33 || hash.content_len != 32 ||
        hash.source[0] != 0xa0)
      return false;
    if (hash.content[0] != 0x01) {
      *error = ExecutionInvalid;
      return false;
    }
    if (count == blob_limit)
      return false;
    ++count;
  }

  result->bytes = content_slice(field);
  result->count = count;
  return true;
}

static bool validate_authorizations(
    const rlp_item *field, struct AuthorizationListRefFields *result) {
  rlp_cursor tuples;
  if (!item_list(field, &tuples))
    return false;

  uint64_t count = 0;
  while (tuples.remaining != 0) {
    rlp_item tuple;
    rlp_item fields[6];
    rlp_cursor children;
    U256 word;
    uint64_t integer;
    if (!take_item(&tuples, &tuple) || !item_list(&tuple, &children) ||
        !take_fields(children, fields, 6) ||
        !quantity_u256(&fields[0], &word) ||
        !word_raw(&fields[1], &word) ||
        !quantity_u64(&fields[2], &integer) ||
        !quantity_u64(&fields[3], &integer) ||
        !quantity_u256(&fields[4], &word) ||
        !quantity_u256(&fields[5], &word))
      return false;
    ++count;
  }

  result->encoded = content_slice(field);
  result->count = count;
  return true;
}

static bool decode_common_fields(struct TransactionFields *result,
                                 const rlp_item *nonce,
                                 const rlp_item *gas,
                                 const rlp_item *recipient,
                                 const rlp_item *value,
                                 const rlp_item *data,
                                 const rlp_item *v,
                                 const rlp_item *r,
                                 const rlp_item *s,
                                 bool nonce_is_u64) {
  U256 recipient_word;
  uint64_t gas_limit = 0;
  uint64_t nonce_u64 = 0;
  if ((!nonce_is_u64 && !quantity_u256(nonce, &result->nonce)) ||
      (nonce_is_u64 && !quantity_u64(nonce, &nonce_u64)) ||
      !quantity_u64(gas, &gas_limit) ||
      !word_raw(recipient, &recipient_word) ||
      !quantity_u256(value, &result->value) ||
      !word_raw(v, &result->sig_v) ||
      !quantity_u256(r, &result->sig_r) ||
      !quantity_u256(s, &result->sig_s))
    return false;
  if (nonce_is_u64) {
    const U256 widened = {{nonce_u64, 0, 0, 0}};
    result->nonce = widened;
  }
  result->gas_limit = gas_limit;
  result->is_create = recipient->content_len == 0;
  result->recipient = low_address(recipient_word);
  result->input_src = content_slice(data);
  return true;
}

struct TransactionFields decode_transaction(
    struct StatelessInputSliceFields transaction,
    struct StatelessInputSliceFields public_key, uint8_t blob_limit) {
  struct TransactionFields result;
  memset(&result, 0, sizeof(result));
  result.raw = transaction;
  result.pubkey = public_key;

  const uint8_t *tx_bytes =
      stateless_input_ptr(transaction.off, transaction.len);
  const uint8_t *key_bytes =
      stateless_input_ptr(public_key.off, public_key.len);
  if (!tx_bytes || transaction.len == 0 || !key_bytes ||
      public_key.len != 65)
    return decode_failure(RlpDecode, "optimized transaction input");

  const uint8_t first = tx_bytes[0];
  const bool legacy = first >= 0xc0;
  const uint8_t type = legacy ? 0 : first;
  if (type > 4 || (!legacy && type == 0))
    return decode_failure(RlpDecode, "optimized transaction type");

  const uint64_t payload_delta = legacy ? 0 : 1;
  rlp_cursor fields;
  if (transaction.len < payload_delta ||
      !exact_list(tx_bytes + payload_delta, transaction.off + payload_delta,
                  transaction.len - payload_delta, &fields))
    return decode_failure(RlpDecode, "optimized transaction envelope");

  const TxFieldLayout *layout = &TX_FIELD_LAYOUTS[type];
  rlp_item f[14];
  if (!take_fields(fields, f, layout->field_count))
    return decode_failure(RlpDecode, "optimized transaction fields");

  const struct StatelessInputSliceFields public_key_body = {
      .off = public_key.off + 1,
      .len = 64,
  };
  const Hash32 sender_hash =
      host_keccak_stateless_input(public_key_body);
  result.sender = hash_low_address(&sender_hash);
  result.tx_type = (enum TxType)type;

  const rlp_item *nonce = tx_field(f, layout->nonce);
  const rlp_item *gas = tx_field(f, layout->gas);
  const rlp_item *recipient = tx_field(f, layout->recipient);
  const rlp_item *value = tx_field(f, layout->value);
  const rlp_item *data = tx_field(f, layout->data);
  const rlp_item *v = tx_field(f, layout->v);
  const rlp_item *r = tx_field(f, layout->r);
  const rlp_item *s = tx_field(f, layout->s);
  const rlp_item *first_signed = &f[0];
  const rlp_item *access_list = tx_field(f, layout->access_list);
  const rlp_item *blob_hashes = tx_field(f, layout->blob_hashes);
  const rlp_item *authorizations = tx_field(f, layout->authorizations);
  const rlp_item *gas_price = tx_field(f, layout->gas_price);
  const rlp_item *max_priority_fee = tx_field(f, layout->max_priority_fee);
  const rlp_item *max_fee = tx_field(f, layout->max_fee);
  const rlp_item *max_blob_fee = tx_field(f, layout->max_blob_fee);
  const rlp_item *chain_id = tx_field(f, layout->chain_id);

  if (!decode_common_fields(&result, nonce, gas, recipient, value, data, v, r,
                            s, layout->nonce_is_u64))
    return decode_failure(RlpDecode, "optimized transaction scalars");

  if (chain_id && !quantity_u64(chain_id, &result.chain_id))
    return decode_failure(RlpDecode, "optimized transaction chain id");

  if (gas_price) {
    if (!quantity_u256(gas_price, &result.max_fee))
      return decode_failure(RlpDecode, "optimized transaction gas price");
    result.max_priority_fee = result.max_fee;
  } else if (!quantity_u256(max_priority_fee, &result.max_priority_fee) ||
             !quantity_u256(max_fee, &result.max_fee)) {
    return decode_failure(RlpDecode, "optimized transaction fees");
  }

  if (max_blob_fee &&
      !quantity_u256(max_blob_fee, &result.max_blob_fee))
    return decode_failure(RlpDecode, "optimized transaction blob fee");

  if (access_list && !decode_access_list(access_list, &result.access_list))
    return decode_failure(RlpDecode, "optimized transaction access list");

  enum BlockError nested_error = RlpDecode;
  if (blob_hashes &&
      !decode_blob_hashes(blob_hashes, blob_limit, &result.blob_hashes,
                          &nested_error))
    return decode_failure(nested_error, "optimized transaction blob hashes");

  if (authorizations &&
      !validate_authorizations(authorizations, &result.authorizations))
    return decode_failure(RlpDecode,
                          "optimized transaction authorizations");

  const struct StatelessInputSliceFields signing_fields = {
      .off = first_signed->source_off,
      .len = v->source_off - first_signed->source_off,
  };
  result.signing_hash =
      tx_signing_hash(type, signing_fields, result.sig_v);
  return result;
}
