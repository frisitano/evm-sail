#pragma once

#include "evmsail/spec/host/debug_disabled.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct ParentHeaderFields
struct ParentHeaderFields {
  u256 base_fee;
  uint32_t blob_gas_used;
  uint32_t excess_blob_gas;
  bool have_base_fee;
  bool have_blob_gas;
  bool have_excess_blob_gas;
  bool have_parent;
  bool have_state;
  bytes32 parent_hash;
  bytes32 state_root;
};

// struct WitnessHeaderIndex
struct WitnessHeaderIndex {
  struct BoundedSszListCursor cursor;
  u256 parent_base_fee_per_gas;
  uint32_t parent_blob_gas_used;
  uint32_t parent_excess_blob_gas;
  bool parent_fields_valid;
  bytes32 parent_state_root;
  bytes32 previous_hash;
  bool valid;
};

// struct StatelessInputRef
struct StatelessInputRef {
  Bytes block_access_list;
  Bytes builder_deposit_requests;
  Bytes builder_exit_requests;
  Bytes chain_config;
  Bytes consolidation_requests;
  Bytes deposits;
  Bytes execution_payload;
  Bytes extra_data;
  Bytes new_payload_request;
  struct ProtocolProfileFields protocol;
  Bytes public_keys;
  struct BoundedSszListRef transactions;
  Bytes versioned_hashes;
  Bytes withdrawal_requests;
  struct BoundedSszListRef withdrawals;
  struct BoundedSszListRef witness_codes;
  struct BoundedSszListRef witness_headers;
  struct BoundedSszListRef witness_state;
};

uint32_t ssz_offset_table_position(uint32_t index);

struct BoundedSszListCursor ssz_list_cursor(struct BoundedSszListRef items);

bool ssz_list_cursor_empty(struct BoundedSszListCursor cursor);

struct tuple_Bytes_BoundedSszListCursor ssz_list_pop(struct BoundedSszListCursor cursor);

struct StatelessInputRef decode_stateless_input_ref(Bytes /* arg_0 */);

struct StatelessInputRef decode_stateless_input_ref_(Bytes input);

void index_witness_nodes_(struct BoundedSszListRef nodes);

void index_witness_codes_(struct BoundedSszListRef codes);

void create_letbind_276(void);
void kill_letbind_276(void);


uint8_t next_parent_header_field(uint8_t index);

struct WitnessHeaderIndex index_witness_header_cursor(struct WitnessHeaderIndex state);

struct WitnessContext index_witness_headers(struct BoundedSszListRef headers);

uint32_t decode_payload_blob_gas_used(Bytes payload, struct ProtocolProfileFields profile);

uint32_t decode_payload_excess_blob_gas(Bytes payload, struct ProtocolProfileFields profile);

struct BlockHeader decode_block_header_ssz(struct StatelessInputRef input_ref);

struct Withdrawal decode_withdrawal_(Bytes withdrawal);

struct ChainConfig decode_chain_config(Bytes cc, uint64_t number, uint64_t timestamp);

struct StatelessInput decode_stateless_input(struct StatelessInputRef input_ref);

struct WitnessContext index_execution_witness(struct StatelessInputRef input_ref);

struct TransactionFields decode_transaction_(Bytes transaction, Bytes public_key);

bytes32 execution_requests_hash(struct StatelessInputRef /* arg_0 */);

bytes32 htr_new_payload_request(struct StatelessInputRef /* arg_0 */);

struct ParentHeaderFields decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields(Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields);

struct ParentHeaderFields decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields_variant_2(Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields);

uint8_t next_parent_header_field_uint8_t_to_uint8_t(uint8_t index);

struct tuple_Bytes_BoundedSszListRef ssz_fixed_list_pop(struct BoundedSszListRef items, uint8_t item_size);

extern const uint8_t NPR_BEACON_ROOT;


extern const uint8_t PL_FEE_RECIPIENT;


extern const uint8_t PL_STATE_ROOT;


extern const uint8_t PL_RECEIPTS_ROOT;


extern const uint8_t PL_LOGS_BLOOM;


extern const uint16_t PL_PREV_RANDAO;


extern const uint16_t PL_BLOCK_NUMBER;


extern const uint16_t PL_GAS_LIMIT;


extern const uint16_t PL_GAS_USED;


extern const uint16_t PL_TIMESTAMP;


extern const uint16_t PL_BASE_FEE;


extern const uint16_t PL_BLOCK_HASH;


extern const uint16_t PL_BLOB_GAS_USED;


extern const uint16_t PL_EXCESS_BLOB_GAS;


extern const uint16_t PL_SLOT_NUMBER;


extern const uint8_t WD_SIZE;


extern const uint8_t CC_CHAIN_ID;


extern const uint8_t CC_ACTIVE_FORK_OFF;


extern const uint8_t FC_ACTIVATION_OFF;


extern const uint8_t FA_BLOCK_NUMBER_OFF;


extern const uint8_t FA_TIMESTAMP_OFF;


extern const uint8_t PUBLIC_KEY_LENGTH;


extern const uint8_t CHAIN_CONFIG_HEADER_LENGTH;


extern const uint8_t CHAIN_CONFIG_MIN_LENGTH;


extern struct ParentHeaderFields EMPTY_PARENT_HEADER_FIELDS;



#ifdef __cplusplus
}
#endif
