/* Optimized whole-transaction RLP decoding. */
#include "evmsail/prelude.h"

#include "evmsail/lib/rlp/codecs/transaction_signing.h"
#include "evmsail/host/region_access.h"
#include "evmsail/primitives/crypto.h"
#include "evmsail/spec/primitives/crypto.h"
#include "evmsail/spec/primitives/tx.h"
#include "evmsail/spec/exceptions.h"
#include "lib/rlp/decoding.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "evmsail/primitives/word.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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
    {9, 0, 2, 3, 4, 5, 6, 7, 8, TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD, 1, TX_NO_FIELD,
     TX_NO_FIELD, TX_NO_FIELD, false},
    /* AccessListTx */
    {11, 1, 3, 4, 5, 6, 8, 9, 10, 0, 7, TX_NO_FIELD, TX_NO_FIELD, 2, TX_NO_FIELD, TX_NO_FIELD,
     TX_NO_FIELD, false},
    /* FeeMarketTx */
    {12, 1, 4, 5, 6, 7, 9, 10, 11, 0, 8, TX_NO_FIELD, TX_NO_FIELD, TX_NO_FIELD, 2, 3, TX_NO_FIELD,
     false},
    /* BlobTx */
    {14, 1, 4, 5, 6, 7, 11, 12, 13, 0, 8, 10, TX_NO_FIELD, TX_NO_FIELD, 2, 3, 9, false},
    /* SetCodeTx */
    {13, 1, 4, 5, 6, 7, 10, 11, 12, 0, 8, TX_NO_FIELD, 9, TX_NO_FIELD, 2, 3, TX_NO_FIELD, true},
};
#undef TX_NO_FIELD

static const rlp_item *tx_field(const rlp_item *fields, uint8_t index)
{
  return index == TX_FIELD_ABSENT ? NULL : &fields[index];
}

_Noreturn static void decode_failure(enum FatalError reason, const char *location)
{
  (void)location;
  fatal_error(reason);
}

static struct StatelessInputSliceFields content_slice(const rlp_item *item)
{
  struct StatelessInputSliceFields result = {
      .bytes = sail_read_only_bytes(item->content),
      .len = item->content_len,
  };
  return result;
}

static Address low_address(U256 value)
{
  return address_from_word(value);
}

static bool decode_access_list(const rlp_item *field, struct AccessListRef *result)
{
  rlp_cursor entries;
  if (!rlp_item_list(field, &entries)) {
    return false;
  }

  uint32_t address_count = 0;
  uint32_t slot_count = 0;
  while (entries.remaining != 0) {
    rlp_item entry;
    rlp_item entry_fields[2];
    rlp_cursor entry_children;
    rlp_cursor keys;
    U256 ignored;
    if (!rlp_take_item(&entries, &entry) || !rlp_item_list(&entry, &entry_children) ||
        !rlp_take_fields(entry_children, entry_fields, 2) ||
        !rlp_word_raw(&entry_fields[0], &ignored) || !rlp_item_list(&entry_fields[1], &keys)) {
      return false;
    }
    ++address_count;
    while (keys.remaining != 0) {
      rlp_item key;
      if (!rlp_take_item(&keys, &key) || !rlp_word_raw(&key, &ignored)) {
        return false;
      }
      ++slot_count;
    }
  }

  result->encoded = content_slice(field);
  result->address_count = address_count;
  result->slot_count = slot_count;
  return true;
}

static bool decode_blob_hashes(const rlp_item *field, uint8_t blob_limit,
                               struct BlobHashesFields *result, enum FatalError *error)
{
  rlp_cursor hashes;
  if (!rlp_item_list(field, &hashes)) {
    return false;
  }

  uint8_t count = 0;
  while (hashes.remaining != 0) {
    rlp_item hash;
    if (!rlp_take_item(&hashes, &hash) || hash.is_list || hash.source_len != 33 ||
        hash.content_len != 32 || hash.source[0] != 0xa0) {
      return false;
    }
    if (hash.content[0] != 0x01) {
      *error = ExecutionInvalid;
      return false;
    }
    if (count == blob_limit) {
      return false;
    }
    ++count;
  }

  result->bytes = content_slice(field);
  result->count = count;
  return true;
}

static bool validate_authorizations(const rlp_item *field,
                                    struct AuthorizationListRefFields *result)
{
  rlp_cursor tuples;
  if (!rlp_item_list(field, &tuples)) {
    return false;
  }

  uint32_t count = 0;
  while (tuples.remaining != 0) {
    rlp_item tuple;
    rlp_item fields[6];
    rlp_cursor children;
    U256 word;
    uint64_t integer;
    if (!rlp_take_item(&tuples, &tuple) || !rlp_item_list(&tuple, &children) ||
        !rlp_take_fields(children, fields, 6) || !rlp_quantity_word(&fields[0], &word) ||
        !rlp_word_raw(&fields[1], &word) || !rlp_quantity_u64(&fields[2], &integer) ||
        !rlp_quantity_u64(&fields[3], &integer) || !rlp_quantity_word(&fields[4], &word) ||
        !rlp_quantity_word(&fields[5], &word)) {
      return false;
    }
    ++count;
  }

  result->encoded = content_slice(field);
  result->count = count;
  return true;
}

static bool decode_common_fields(struct TransactionFields *result, const rlp_item *nonce,
                                 const rlp_item *gas, const rlp_item *recipient,
                                 const rlp_item *value, const rlp_item *data, const rlp_item *v,
                                 const rlp_item *r, const rlp_item *s, bool nonce_is_u64)
{
  U256 recipient_word;
  uint64_t gas_limit = 0;
  uint64_t nonce_u64 = 0;
  if ((!nonce_is_u64 && !rlp_quantity_word(nonce, &result->nonce)) ||
      (nonce_is_u64 && !rlp_quantity_u64(nonce, &nonce_u64)) ||
      !rlp_quantity_u64(gas, &gas_limit) || !rlp_word_raw(recipient, &recipient_word) ||
      !rlp_quantity_word(value, &result->value) || !rlp_word_raw(v, &result->sig_v) ||
      !rlp_quantity_word(r, &result->sig_r) || !rlp_quantity_word(s, &result->sig_s)) {
    return false;
  }
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

struct TransactionFields decode_transaction(struct StatelessInputSliceFields transaction,
                                            struct StatelessInputSliceFields public_key,
                                            uint8_t blob_limit)
{
  struct TransactionFields result;
  memset(&result, 0, sizeof(result));
  result.raw = transaction;
  result.pubkey = public_key;

  const uint8_t *tx_bytes = transaction.bytes;
  if (transaction.len == 0) {
    decode_failure(RlpDecode, "optimized transaction input");
  }

  const uint8_t first = tx_bytes[0];
  const bool legacy = first >= 0xc0;
  const uint8_t type = (int)legacy ? 0 : first;
  if (type > 4 || (!legacy && type == 0)) {
    decode_failure(RlpDecode, "optimized transaction type");
  }

  const uint32_t payload_delta = (int)legacy ? 0 : 1;
  rlp_cursor fields;
  if (!rlp_exact_list(tx_bytes + payload_delta, transaction.len - payload_delta, &fields)) {
    decode_failure(RlpDecode, "optimized transaction envelope");
  }

  const TxFieldLayout *layout = &TX_FIELD_LAYOUTS[type];
  rlp_item f[14];
  if (!rlp_take_fields(fields, f, layout->field_count)) {
    decode_failure(RlpDecode, "optimized transaction fields");
  }

  const struct StatelessInputSliceFields public_key_body = {
      .bytes = public_key.bytes + 1,
      .len = 64,
  };
  const Hash32 sender_hash = host_keccak_stateless_input(public_key_body);
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

  if (!decode_common_fields(&result, nonce, gas, recipient, value, data, v, r, s,
                            layout->nonce_is_u64 != 0U)) {
    decode_failure(RlpDecode, "optimized transaction scalars");
  }

  if (chain_id && !rlp_quantity_u64(chain_id, &result.chain_id)) {
    decode_failure(RlpDecode, "optimized transaction chain id");
  }

  if (gas_price) {
    if (!rlp_quantity_word(gas_price, &result.max_fee)) {
      decode_failure(RlpDecode, "optimized transaction gas price");
    }
    result.max_priority_fee = result.max_fee;
  } else if (!rlp_quantity_word(max_priority_fee, &result.max_priority_fee) ||
             !rlp_quantity_word(max_fee, &result.max_fee)) {
    decode_failure(RlpDecode, "optimized transaction fees");
  }

  if (max_blob_fee && !rlp_quantity_word(max_blob_fee, &result.max_blob_fee)) {
    decode_failure(RlpDecode, "optimized transaction blob fee");
  }

  if (access_list && !decode_access_list(access_list, &result.access_list)) {
    decode_failure(RlpDecode, "optimized transaction access list");
  }

  enum FatalError nested_error = RlpDecode;
  if (blob_hashes &&
      !decode_blob_hashes(blob_hashes, blob_limit, &result.blob_hashes, &nested_error)) {
    decode_failure(nested_error, "optimized transaction blob hashes");
  }

  if (authorizations && !validate_authorizations(authorizations, &result.authorizations)) {
    decode_failure(RlpDecode, "optimized transaction authorizations");
  }

  /* Both pointers belong to the validated transaction slice and RLP fields
   * are ordered, so this distance is in the 32-bit host-region domain. */
  const uint32_t signing_length = (uint32_t)(v->source - first_signed->source);
  const struct StatelessInputSliceFields signing_fields = {
      .bytes = sail_read_only_bytes(first_signed->source),
      .len = signing_length,
  };
  result.signing_hash = tx_signing_hash(type, signing_fields, result.sig_v);
  return result;
}
