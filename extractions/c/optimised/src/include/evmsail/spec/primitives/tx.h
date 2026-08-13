#pragma once

#include "evmsail/spec/primitives/account.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum TxType
enum TxType { LegacyTx, AccessListTx, FeeMarketTx, BlobTx, SetCodeTx };

// enum TxSignatureScheme
enum TxSignatureScheme { LegacySignature, TypedSignature };

// struct TxTypeSemantics
struct TxTypeSemantics {
  bool blob;
  uint8_t minimum_fork;
  bool set_code;
  enum TxSignatureScheme signature;
};

// struct tuple_u256_u256
struct tuple_u256_u256 {
  u256 tup0;
  u256 tup1;
};

// struct tuple_u256_u256_u256
struct tuple_u256_u256_u256 {
  u256 tup0;
  u256 tup1;
  u256 tup2;
};

// struct tuple_u256_u256_u256_u256
struct tuple_u256_u256_u256_u256 {
  u256 tup0;
  u256 tup1;
  u256 tup2;
  u256 tup3;
};

// union LogTopics
enum kind_LogTopics { Kind_LogTopics0, Kind_LogTopics1, Kind_LogTopics2, Kind_LogTopics3, Kind_LogTopics4 };

struct LogTopics {
  enum kind_LogTopics kind;
  union {
    struct { unit LogTopics0; };
    struct { u256 LogTopics1; };
    struct { struct tuple_u256_u256 LogTopics2; };
    struct { struct tuple_u256_u256_u256 LogTopics3; };
    struct { struct tuple_u256_u256_u256_u256 LogTopics4; };
  } variants;
};

// struct LogSeriesRef
struct LogSeriesRef {
  uint64_t count;
  uint64_t start;
};

// struct ReceiptFields
struct ReceiptFields {
  uint64_t execution_gas;
  uint64_t gas_used;
  struct LogSeriesRef logs;
  uint64_t state_gas;
  bool success;
  enum TxType tx_type;
};

// struct BlobHashesFields
struct BlobHashesFields {
  Bytes bytes;
  uint8_t count;
};

// struct AuthorizationListRefFields
struct AuthorizationListRefFields {
  uint32_t count;
  Bytes encoded;
};

// struct Authorization
struct Authorization {
  bytes20 address;
  bytes20 authority;
  u256 chain_id;
  uint64_t nonce;
  bool valid_sig;
};

// struct AccessListRef
struct AccessListRef {
  uint32_t address_count;
  Bytes encoded;
  uint32_t slot_count;
};

// struct TransactionFields
struct TransactionFields {
  struct AccessListRef access_list;
  struct AuthorizationListRefFields authorizations;
  struct BlobHashesFields blob_hashes;
  uint64_t chain_id;
  uint64_t gas_limit;
  Bytes input_src;
  bool is_create;
  u256 max_blob_fee;
  u256 max_fee;
  u256 max_priority_fee;
  u256 nonce;
  Bytes pubkey;
  Bytes raw;
  bytes20 recipient;
  bytes20 sender;
  u256 sig_r;
  u256 sig_s;
  u256 sig_v;
  bytes32 signing_hash;
  enum TxType tx_type;
  u256 value;
};

uint64_t tx_envelope_type(enum TxType t);

struct TxTypeSemantics tx_type_semantics(enum TxType t);

void create_letbind_73(void);
void kill_letbind_73(void);


uint64_t log_store_index_increment(uint64_t value);

uint64_t log_store_index_add(uint64_t left, uint64_t right);

struct ReceiptFields receipt_fields(enum TxType tx_type, bool success, uint64_t gas_used, uint64_t execution_gas, uint64_t state_gas, struct LogSeriesRef logs);

struct ReceiptFields receipt_within(enum TxType tx_type, bool success, uint64_t gas_used, uint64_t execution_gas, uint64_t state_gas, struct LogSeriesRef logs);

PreparedAuthorizationList prepare_authorizations(struct AuthorizationListRefFields /* arg_0 */);

struct Authorization prepared_authorization_head(PreparedAuthorizationList /* arg_0 */);

PreparedAuthorizationList prepared_authorization_tail(PreparedAuthorizationList /* arg_0 */, uint16_t /* arg_1 */);

struct TransactionFields decode_transaction(Bytes /* arg_0 */, Bytes /* arg_1 */, uint8_t /* arg_2 */);

struct AuthorizationListRefFields authorization_list_ref(Bytes encoded, uint8_t count);

extern struct BlobHashesFields EMPTY_BLOB_HASHES;



#ifdef __cplusplus
}
#endif
