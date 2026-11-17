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
  uint64_t gas_limit;
  uint64_t gas_used;
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
  uint64_t parent_gas_limit;
  uint64_t parent_gas_used;
  bool parent_has_base_fee;
  bytes32 parent_state_root;
  bytes32 previous_hash;
  bool valid;
};

// struct StatelessInputRef
struct StatelessInputRef {
  Bytes block_access_list;
  Bytes builder_deposit_requests;
  Bytes builder_exit_requests;
  uint64_t chain_id;
  Bytes consolidation_requests;
  Bytes deposits;
  Bytes execution_payload;
  Bytes extra_data;
  Bytes new_payload_request;
  struct ProtocolProfileFields protocol;
  uint64_t schema_id;
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

struct BoundedSszListCursor ssz_list_pop(struct BoundedSszListCursor cursor,
                                         Bytes *restrict statelessinputslicefields_8_1560);

struct StatelessInputRef decode_stateless_input_ref(Bytes /* arg_0 */,
                                                    struct ProtocolProfileFields /* arg_1 */);

struct StatelessInputRef decode_stateless_input_ref_(Bytes input);

void index_witness_nodes_(struct BoundedSszListRef nodes);

void index_witness_codes_(struct BoundedSszListRef codes);

void create_letbind_269(void);
void kill_letbind_269(void);

uint8_t next_parent_header_field(uint8_t index);

struct WitnessHeaderIndex index_witness_header_cursor(struct WitnessHeaderIndex state);

struct WitnessContext index_witness_headers(struct BoundedSszListRef headers);

uint32_t decode_payload_blob_gas_used(Bytes payload, struct ProtocolProfileFields profile);

uint32_t decode_payload_excess_blob_gas(Bytes payload, struct ProtocolProfileFields profile);

struct BlockHeader decode_block_header_ssz(struct StatelessInputRef input_ref);

struct Withdrawal decode_withdrawal_(Bytes withdrawal);

struct StatelessInput decode_stateless_input(struct StatelessInputRef input_ref);

struct WitnessContext index_execution_witness(struct StatelessInputRef input_ref);

struct TransactionFields recover_transaction_sender(struct DecodedTransactionFields tx);

struct TransactionFields decode_transaction_(Bytes transaction);

bytes32 execution_requests_hash(struct StatelessInputRef /* arg_0 */);

bytes32 htr_new_payload_request(struct StatelessInputRef /* arg_0 */);

struct ParentHeaderFields
decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields(
    Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields);

struct ParentHeaderFields
decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields_variant_2(
    Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields);

uint8_t next_parent_header_field_uint8_t_to_uint8_t(uint8_t index);

struct BoundedSszListRef ssz_fixed_list_pop(struct BoundedSszListRef items, uint8_t item_size,
                                            Bytes *restrict statelessinputslicefields_8_1612);

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

extern const uint8_t WD_INDEX;

extern const uint8_t WD_VALIDATOR_INDEX;

extern const uint8_t WD_ADDRESS;

extern const uint8_t WD_AMOUNT;

extern struct ParentHeaderFields EMPTY_PARENT_HEADER_FIELDS;

#ifdef __cplusplus
}
#endif
