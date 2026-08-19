#pragma once

#include "sail.h"
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

// type abbreviation y_parity
typedef uint8_t zy_parity;

// type abbreviation word_bit_count
typedef uint16_t zword_bit_count;


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif


// type abbreviation word
typedef u256 zword;

// type abbreviation withdrawal_index
typedef uint64_t zwithdrawal_index;

// type abbreviation withdrawal_amount
typedef uint64_t zwithdrawal_amount;

// type abbreviation validator_index
typedef uint64_t zvalidator_index;

// type abbreviation validation_stage
typedef uint8_t zvalidation_stage;

// type abbreviation trie_path_len
typedef uint8_t ztrie_path_len;

// type abbreviation trie_path_cursor
typedef uint8_t ztrie_path_cursor;


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif


// type abbreviation transaction_state_gas_used
typedef u128 ztransaction_state_gas_used;

// type abbreviation transaction_state_gas_delta
typedef __int128 ztransaction_state_gas_delta;

// type abbreviation transaction_item_count
typedef uint32_t ztransaction_item_count;

// type abbreviation transaction_gas
typedef uint64_t ztransaction_gas;

// type abbreviation transaction_count
typedef uint32_t ztransaction_count;

// type abbreviation transaction_byte_length
typedef uint32_t ztransaction_byte_length;

// type abbreviation transaction_blob_gas
typedef uint32_t ztransaction_blob_gas;

// type abbreviation transaction_blob_count
typedef sail_int ztransaction_blob_count;

// type abbreviation stateless_input_pointer
typedef uint32_t zstateless_input_pointer;

// type abbreviation state_gas_spill
typedef uint32_t zstate_gas_spill;

// type abbreviation state_gas_delta
typedef __int128 zstate_gas_delta;

// type abbreviation state_gas
typedef uint64_t zstate_gas;

// type abbreviation stack_slot_count
typedef uint16_t zstack_slot_count;

// type abbreviation stack_operation_index
typedef uint8_t zstack_operation_index;

// type abbreviation stack_index
typedef uint16_t zstack_index;

// type abbreviation ssz_uint
typedef uint64_t zsszz_uint;

// type abbreviation ssz_offset_index
typedef uint32_t zsszz_offset_index;

// type abbreviation ssz_offset
typedef uint32_t zsszz_offset;

// type abbreviation source_pointer
typedef uint32_t zsource_pointer;

// type abbreviation source_length
typedef uint32_t zsource_length;

// type abbreviation slot_number
typedef uint64_t zslot_number;

// type abbreviation rlp_scratch_length
typedef uint32_t zrlp_scratch_length;

// type abbreviation rlp_index
typedef uint32_t zrlp_index;

// type abbreviation push_width
typedef uint8_t zpush_width;

// type abbreviation profile_excess_blob_gas_limit
typedef sail_int zprofile_excess_blob_gas_limit;

// type abbreviation prepared_authorization_count
typedef uint16_t zprepared_authorizzation_count;

// type abbreviation parent_header_field_cursor
typedef uint8_t zparent_header_field_cursor;

// type abbreviation pairing_check_result
typedef uint8_t zpairing_check_result;

// type abbreviation operand_stack_height
typedef uint16_t zoperand_stack_height;

// type abbreviation opcode
typedef uint8_t zopcode;

// type abbreviation nibble
typedef uint64_t znibble;

// type abbreviation modexp_product
typedef u256 zmodexp_product;

// type abbreviation modexp_pre_osaka_extra
typedef uint64_t zmodexp_pre_osaka_extra;

// type abbreviation modexp_osaka_extra
typedef uint64_t zmodexp_osaka_extra;

// type abbreviation modexp_factor
typedef u128 zmodexp_factor;

// type abbreviation memory_required_endpoint
typedef sail_int zmemory_required_endpoint;

// type abbreviation memory_length
typedef uint32_t zmemory_length;

// type abbreviation memory_height
typedef uint32_t zmemory_height;

// type abbreviation memory_base
typedef uint32_t zmemory_base;

// type abbreviation log_topic_count
typedef uint8_t zlog_topic_count;

// type abbreviation log_store_index
typedef uint64_t zlog_store_index;

// type abbreviation linear_gas_variable_product
typedef u128 zlinear_gas_variable_product;

// type abbreviation linear_gas_product
typedef u128 zlinear_gas_product;

// type abbreviation jump_table_index
typedef uint64_t zjump_table_index;

// type abbreviation htr_leaf_count
typedef uint32_t zhtr_leaf_count;

// type abbreviation htr_depth
typedef uint8_t zhtr_depth;

// type abbreviation hex_prefix_cursor
typedef uint8_t zhex_prefix_cursor;

#ifndef SAIL_FIXED_BYTES_32_DEFINED
#define SAIL_FIXED_BYTES_32_DEFINED
typedef struct { uint8_t bytes[32]; } fixed_bytes_32;
#endif

// type abbreviation hash
typedef fixed_bytes_32 zhash;

// type abbreviation gas_refund_delta
typedef int16_t zgas_refund_delta;

// type abbreviation gas_refund
typedef __int128 zgas_refund;

// type abbreviation gas_cost
typedef uint64_t zgas_cost;

// type abbreviation gas_constant
typedef uint16_t zgas_constant;

// type abbreviation gas
typedef uint64_t zgas;

// type abbreviation frame_state_gas_delta
typedef __int128 zframe_state_gas_delta;

// type abbreviation frame_depth
typedef uint16_t zframe_depth;

// type abbreviation excess_blob_gas
typedef uint32_t zexcess_blob_gas;

// union exception
enum kind_zexception { Kind_z__dummy_exnz3 };

struct zexception {
  enum kind_zexception kind;
  union {struct { unit z__dummy_exnz3; };} variants;
};

// type abbreviation code_scan_position
typedef uint32_t zcode_scan_position;

// type abbreviation code_pointer
typedef uint32_t zcode_pointer;

// type abbreviation code_length
typedef uint32_t zcode_length;

// type abbreviation chain_identifier
typedef uint64_t zchain_identifier;

// type abbreviation call_tree_steps
typedef u128 zcall_tree_steps;

// type abbreviation byte
typedef uint64_t zbyte;

// type abbreviation branch_mask
typedef uint64_t zbranch_mask;

// type abbreviation branch_content_length
typedef uint16_t zbranch_content_length;

// type abbreviation bls_msm_tail_product
typedef uint64_t zbls_msm_tail_product;

// type abbreviation bls_msm_product
typedef u128 zbls_msm_product;

// type abbreviation bls_msm_coefficient
typedef uint32_t zbls_msm_coefficient;

// type abbreviation bls_discount
typedef uint16_t zbls_discount;

// type abbreviation bloom_bit_index
typedef uint16_t zbloom_bit_index;

// type abbreviation block_timestamp
typedef uint64_t zblock_timestamp;

// type abbreviation block_number
typedef uint64_t zblock_number;

// type abbreviation block_gas_limit
typedef uint64_t zblock_gas_limit;

// type abbreviation block_gas
typedef uint64_t zblock_gas;

// type abbreviation block_access_index
typedef uint32_t zblock_access_index;

// type abbreviation blob_gas_used
typedef uint32_t zblob_gas_used;

// type abbreviation b256_index
typedef uint8_t zb256_index;

// type abbreviation b256
typedef fixed_bytes_32 zb256;

// type abbreviation authorization_refund
typedef uint64_t zauthorizzation_refund;

// type abbreviation authorization_item_refund
typedef uint16_t zauthorizzation_item_refund;

// struct tuple_(%u8, %struct z__sail_c_repr_u256)
struct ztuple_z8z5u8zCz0z5structz0zz__sail_c_repr_u256z9 {
  uint8_t ztup0;
  u256 ztup1;
};

// union ast
enum kind_zast { Kind_zADD, Kind_zADDMOD, Kind_zADDRESS, Kind_zAND, Kind_zBALANCE, Kind_zBASEFEE, Kind_zBLOBBASEFEE, Kind_zBLOBHASH, Kind_zBLOCKHASH, Kind_zBYTE, Kind_zCALL, Kind_zCALLCODE, Kind_zCALLDATACOPY, Kind_zCALLDATALOAD, Kind_zCALLDATASIZE, Kind_zCALLER, Kind_zCALLVALUE, Kind_zCHAINID, Kind_zCLZ, Kind_zCODECOPY, Kind_zCODESIZE, Kind_zCOINBASE, Kind_zCREATE2, Kind_zDELEGATECALL, Kind_zDIV, Kind_zDUP, Kind_zDUPN, Kind_zEQ, Kind_zEXCHANGE, Kind_zEXP, Kind_zEXTCODECOPY, Kind_zEXTCODEHASH, Kind_zEXTCODESIZE, Kind_zGAS, Kind_zGASLIMIT, Kind_zGASPRICE, Kind_zGT, Kind_zINVALID, Kind_zISZERO, Kind_zJUMP, Kind_zJUMPDEST, Kind_zJUMPI, Kind_zKECCAK256, Kind_zLOG, Kind_zLT, Kind_zMCOPY, Kind_zMLOAD, Kind_zMOD, Kind_zMSIZE, Kind_zMSTORE, Kind_zMSTORE8, Kind_zMUL, Kind_zMULMOD, Kind_zNOT, Kind_zNUMBER, Kind_zOR, Kind_zORIGIN, Kind_zPC, Kind_zPOP, Kind_zPREVRANDAO, Kind_zPUSH, Kind_zRETURN, Kind_zRETURNDATACOPY, Kind_zRETURNDATASIZE, Kind_zREVERT, Kind_zSAR, Kind_zSDIV, Kind_zSELFBALANCE, Kind_zSELFDESTRUCT, Kind_zSGT, Kind_zSHL, Kind_zSHR, Kind_zSIGNEXTEND, Kind_zSLOAD, Kind_zSLOTNUM, Kind_zSLT, Kind_zSMOD, Kind_zSSTORE, Kind_zSTATICCALL, Kind_zSTOP, Kind_zSUB, Kind_zSWAP, Kind_zSWAPN, Kind_zTIMESTAMP, Kind_zTLOAD, Kind_zTSTORE, Kind_zXOR, Kind_zopcode_CREATE };

struct zast {
  enum kind_zast kind;
  union {
    struct { unit zADD; };
    struct { unit zADDMOD; };
    struct { unit zADDRESS; };
    struct { unit zAND; };
    struct { unit zBALANCE; };
    struct { unit zBASEFEE; };
    struct { unit zBLOBBASEFEE; };
    struct { unit zBLOBHASH; };
    struct { unit zBLOCKHASH; };
    struct { unit zBYTE; };
    struct { unit zCALL; };
    struct { unit zCALLCODE; };
    struct { unit zCALLDATACOPY; };
    struct { unit zCALLDATALOAD; };
    struct { unit zCALLDATASIZE; };
    struct { unit zCALLER; };
    struct { unit zCALLVALUE; };
    struct { unit zCHAINID; };
    struct { unit zCLZ; };
    struct { unit zCODECOPY; };
    struct { unit zCODESIZE; };
    struct { unit zCOINBASE; };
    struct { unit zCREATE2; };
    struct { unit zDELEGATECALL; };
    struct { unit zDIV; };
    struct { uint8_t zDUP; };
    struct { uint64_t zDUPN; };
    struct { unit zEQ; };
    struct { uint64_t zEXCHANGE; };
    struct { unit zEXP; };
    struct { unit zEXTCODECOPY; };
    struct { unit zEXTCODEHASH; };
    struct { unit zEXTCODESIZE; };
    struct { unit zGAS; };
    struct { unit zGASLIMIT; };
    struct { unit zGASPRICE; };
    struct { unit zGT; };
    struct { unit zINVALID; };
    struct { unit zISZERO; };
    struct { unit zJUMP; };
    struct { unit zJUMPDEST; };
    struct { unit zJUMPI; };
    struct { unit zKECCAK256; };
    struct { uint8_t zLOG; };
    struct { unit zLT; };
    struct { unit zMCOPY; };
    struct { unit zMLOAD; };
    struct { unit zMOD; };
    struct { unit zMSIZE; };
    struct { unit zMSTORE; };
    struct { unit zMSTORE8; };
    struct { unit zMUL; };
    struct { unit zMULMOD; };
    struct { unit zNOT; };
    struct { unit zNUMBER; };
    struct { unit zOR; };
    struct { unit zORIGIN; };
    struct { unit zPC; };
    struct { unit zPOP; };
    struct { unit zPREVRANDAO; };
    struct { struct ztuple_z8z5u8zCz0z5structz0zz__sail_c_repr_u256z9 zPUSH; };
    struct { unit zRETURN; };
    struct { unit zRETURNDATACOPY; };
    struct { unit zRETURNDATASIZE; };
    struct { unit zREVERT; };
    struct { unit zSAR; };
    struct { unit zSDIV; };
    struct { unit zSELFBALANCE; };
    struct { unit zSELFDESTRUCT; };
    struct { unit zSGT; };
    struct { unit zSHL; };
    struct { unit zSHR; };
    struct { unit zSIGNEXTEND; };
    struct { unit zSLOAD; };
    struct { unit zSLOTNUM; };
    struct { unit zSLT; };
    struct { unit zSMOD; };
    struct { unit zSSTORE; };
    struct { unit zSTATICCALL; };
    struct { unit zSTOP; };
    struct { unit zSUB; };
    struct { uint8_t zSWAP; };
    struct { uint64_t zSWAPN; };
    struct { unit zTIMESTAMP; };
    struct { unit zTLOAD; };
    struct { unit zTSTORE; };
    struct { unit zXOR; };
    struct { unit zopcode_CREATE; };
  } variants;
};

// type abbreviation ancestor_index
typedef uint8_t zancestor_index;

// type abbreviation ancestor_hash_count
typedef uint16_t zancestor_hash_count;

#ifndef SAIL_FIXED_BYTES_20_DEFINED
#define SAIL_FIXED_BYTES_20_DEFINED
typedef struct { uint8_t bytes[20]; } fixed_bytes_20;
#endif

// type abbreviation address
typedef fixed_bytes_20 zaddress;

// type abbreviation account_nonce
typedef uint64_t zaccount_nonce;

// struct WitnessContext
struct zWitnessContext {
  u256 zparent_base_fee_per_gas;
  uint32_t zparent_blob_gas_used;
  uint32_t zparent_excess_blob_gas;
  fixed_bytes_32 zparent_hash;
  fixed_bytes_32 zparent_state_root;
};

// struct Withdrawal
struct zWithdrawal {
  fixed_bytes_20 zaddress;
  uint64_t zamount;
  uint64_t zindex;
  uint64_t zvalidator_index;
};

// struct TxUpfrontResult
struct zTxUpfrontResult {
  uint64_t zauthorizzation_refund;
  bool zcreate_target_prestate_empty;
};

// enum TxType
enum zTxType { zLegacyTx, zAccessListTx, zFeeMarketTx, zBlobTx, zSetCodeTx };

// enum TxSignatureScheme
enum zTxSignatureScheme { zLegacySignature, zTypedSignature };

// struct TxTypeSemantics
struct zTxTypeSemantics {
  bool zblob;
  uint8_t zminimum_fork;
  bool zset_code;
  enum zTxSignatureScheme zsignature;
};

// struct TxFrameGasSnapshotFields
struct zTxFrameGasSnapshotFields {
  uint64_t zadmitted_limit;
  uint64_t zcalldata_floor;
  uint64_t zregular_limit;
  uint64_t zremaining;
  uint64_t zstate_used;
};

// struct TxFrameResultFields
struct zTxFrameResultFields {
  struct zTxFrameGasSnapshotFields zgas;
  __int128 zrefund;
  bool zsuccess;
};

// type abbreviation TxFrameGasSnapshotForLimits
typedef struct zTxFrameGasSnapshotFields zTxFrameGasSnapshotForLimits;

// union TrieUpdateSource
enum kind_zTrieUpdateSource { Kind_zChangedAccountTrieUpdates, Kind_zStorageTrieUpdates };

struct zTrieUpdateSource {
  enum kind_zTrieUpdateSource kind;
  union {
    struct { unit zChangedAccountTrieUpdates; };
    struct { fixed_bytes_20 zStorageTrieUpdates; };
  } variants;
};

// struct TrieRootResult
struct zTrieRootResult {
  bool zchanged;
  fixed_bytes_32 zroot;
};

// struct TriePath
struct zTriePath {
  fixed_bytes_32 zdata;
  uint8_t zlen;
};

// union TrieUpdateRelation
enum kind_zTrieUpdateRelation { Kind_zUpdateBeyondPrefix, Kind_zUpdateSourceExhausted, Kind_zUpdateUnderPrefix };

struct zTrieUpdateRelation {
  enum kind_zTrieUpdateRelation kind;
  union {
    struct { uint8_t zUpdateBeyondPrefix; };
    struct { unit zUpdateSourceExhausted; };
    struct { struct zTriePath zUpdateUnderPrefix; };
  } variants;
};

// struct TransactionPreparation
struct zTransactionPreparation {
  bool zdelegated;
  bool zready;
};

// struct TransactionMergeSemantics
struct zTransactionMergeSemantics {
  bool zdelete_only_created;
  bool zpreserve_selfdestruct_balance;
};

// struct TransactionInitialGasFields
struct zTransactionInitialGasFields {
  uint64_t zadmitted_limit;
  uint64_t zcalldata_floor;
  uint64_t zexecution_remaining;
  uint64_t zintrinsic_execution;
  uint64_t zintrinsic_state;
  uint64_t zregular_limit;
  uint64_t zstate_remaining;
};

// struct TxValidityFields
struct zTxValidityFields {
  u256 zblob_fee;
  struct zTransactionInitialGasFields zgas;
  u256 zgas_price;
  uint64_t znonce_before;
  u256 zpriority_fee;
  fixed_bytes_20 zsender;
};

// type abbreviation TxValidity
typedef struct zTxValidityFields zTxValidity;

// struct TransactionGasAllowanceFields
struct zTransactionGasAllowanceFields {
  uint64_t zregular;
  uint64_t ztotal;
};

// struct TransactionCosts
struct zTransactionCosts {
  u256 zblob_fee;
  uint32_t zblob_gas;
  uint64_t zcalldata_floor;
  uint64_t zintrinsic_execution;
  uint64_t zintrinsic_state;
  u256 zupfront;
};

// struct StorageValue
struct zStorageValue {
  u256 zcurr;
  u256 zorig;
};

// union StorageTxLookup
enum kind_zStorageTxLookup { Kind_zStorageTxCleared, Kind_zStorageTxHit, Kind_zStorageTxMiss };

struct zStorageTxLookup {
  enum kind_zStorageTxLookup kind;
  union {
    struct { unit zStorageTxCleared; };
    struct { struct zStorageValue zStorageTxHit; };
    struct { unit zStorageTxMiss; };
  } variants;
};

// struct StorageKey
struct zStorageKey {
  fixed_bytes_20 zaddr;
  u256 zslot;
};

// struct StorageEntry
struct zStorageEntry {
  struct zStorageKey zkey;
  struct zStorageValue zvalue;
};

// union StorageTxPopResult
enum kind_zStorageTxPopResult { Kind_zStorageTxPopExhausted, Kind_zStorageTxPopRow };

struct zStorageTxPopResult {
  enum kind_zStorageTxPopResult kind;
  union {
    struct { unit zStorageTxPopExhausted; };
    struct { struct zStorageEntry zStorageTxPopRow; };
  } variants;
};

// struct StorageTrieEntry
struct zStorageTrieEntry {
  fixed_bytes_32 zaddress_hash;
  struct zStorageEntry zentry;
  fixed_bytes_32 zslot_hash;
};

// struct StorageBlockRow
struct zStorageBlockRow {
  bool zfound;
  struct zStorageValue zvalue;
};

// union StorageBlockIterResult
enum kind_zStorageBlockIterResult { Kind_zStorageBlockIterExhausted, Kind_zStorageBlockIterRow };

struct zStorageBlockIterResult {
  enum kind_zStorageBlockIterResult kind;
  union {
    struct { unit zStorageBlockIterExhausted; };
    struct { struct zStorageTrieEntry zStorageBlockIterRow; };
  } variants;
};

// struct StatelessInputSliceFields
struct zStatelessInputSliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// type abbreviation TransactionInputSlice
typedef struct zStatelessInputSliceFields zTransactionInputSlice;

// type abbreviation StatelessInputSliceLength
typedef struct zStatelessInputSliceFields zStatelessInputSliceLength;

// type abbreviation StatelessInputSliceAtMost
typedef struct zStatelessInputSliceFields zStatelessInputSliceAtMost;

// type abbreviation StatelessInputSliceAtLeast
typedef struct zStatelessInputSliceFields zStatelessInputSliceAtLeast;

// type abbreviation StatelessInputSlice
typedef struct zStatelessInputSliceFields zStatelessInputSlice;

// enum StackValidation
enum zStackValidation { zStackValid, zStackUnderflowFailure, zStackOverflowFailure };

// struct StackPointer
struct zStackPointer {
  uint16_t zheight;
  uint64_t zstorage;
};

// struct SszContainerCursor
struct zSszzContainerCursor {
  struct zStatelessInputSliceFields zbytes;
  uint32_t zcurrent;
};

// struct SstoreCosts
struct zSstoreCosts {
  uint64_t zexecution;
  __int128 zrefund;
  uint64_t zstate_charge;
  uint32_t zstate_credit;
};

// struct ScratchSliceFields
struct zScratchSliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// union TrieLeafValue
enum kind_zTrieLeafValue { Kind_zInputTrieLeaf, Kind_zScratchTrieLeaf };

struct zTrieLeafValue {
  enum kind_zTrieLeafValue kind;
  union {
    struct { struct zStatelessInputSliceFields zInputTrieLeaf; };
    struct { struct zScratchSliceFields zScratchTrieLeaf; };
  } variants;
};

// union TrieChange
enum kind_zTrieChange { Kind_zTrieDelete, Kind_zTriePut };

struct zTrieChange {
  enum kind_zTrieChange kind;
  union {
    struct { unit zTrieDelete; };
    struct { struct zScratchSliceFields zTriePut; };
  } variants;
};

// struct TrieUpdate
struct zTrieUpdate {
  struct zTrieChange zchange;
  struct zTriePath zkey;
};

// struct TrieUpdateFetch
struct zTrieUpdateFetch {
  bool zavailable;
  struct zTrieUpdate zupdate;
};

// struct TrieUpdateCursor
struct zTrieUpdateCursor {
  struct zTrieUpdate zcurrent;
  struct zTrieUpdateRelation zrelation;
  struct zTrieUpdateSource zsource;
};

// type abbreviation ScratchSliceAtLeast
typedef struct zScratchSliceFields zScratchSliceAtLeast;

// type abbreviation ScratchSlice
typedef struct zScratchSliceFields zScratchSlice;

// struct ScratchRlpFieldRef
struct zScratchRlpFieldRef {
  uint32_t zcontent_len;
  bool zis_list;
  struct zScratchSliceFields zsource;
};

// type abbreviation ScratchRlpCursor
typedef struct zScratchSliceFields zScratchRlpCursor;

// union RlpResult<o>
enum kind_zRlpResultzIozK { Kind_zRlpInvalidValuezIozK, Kind_zRlpOkzIozK };

struct zRlpResultzIozK {
  enum kind_zRlpResultzIozK kind;
  union {
    struct { unit zRlpInvalidValuezIozK; };
    struct { bool zRlpOkzIozK; };
  } variants;
};

// struct RlpIndexItem
struct zRlpIndexItem {
  uint32_t zindex;
  struct zTriePath zkey;
};

// struct RlpIndexCursor
struct zRlpIndexCursor {
  uint32_t zcount;
  struct zRlpIndexItem zcurrent;
  uint32_t zposition;
};

// struct RlpFieldRef
struct zRlpFieldRef {
  uint32_t zcontent_len;
  bool zis_list;
  struct zStatelessInputSliceFields zsource;
};

// struct RlpEncoder
struct zRlpEncoder {
  uint32_t zexpected_len;
  uint32_t zstart;
};

// type abbreviation RlpCursor
typedef struct zStatelessInputSliceFields zRlpCursor;

// struct ReceiptRecordsRef
struct zReceiptRecordsRef {
  struct zScratchSliceFields zbytes;
  uint32_t zcount;
};

// enum PrecompileId
enum zPrecompileId { zNotPrecompile, zEcrecover, zSha256, zRipemd160, zIdentity, zModexp, zBn254Add, zBn254Mul, zBn254Pairing, zBlake2f, zKzzgPointEvaluation, zBlsG1Add, zBlsG1Msm, zBlsG2Add, zBlsG2Msm, zBlsPairing, zBlsMapFpToG1, zBlsMapFp2ToG2, zP256Verify };

// type abbreviation precompile_id
typedef enum zPrecompileId zprecompile_id;

// struct ParentHeaderFields
struct zParentHeaderFields {
  u256 zbase_fee;
  uint32_t zblob_gas_used;
  uint32_t zexcess_blob_gas;
  bool zhave_base_fee;
  bool zhave_blob_gas;
  bool zhave_excess_blob_gas;
  bool zhave_parent;
  bool zhave_state;
  fixed_bytes_32 zparent_hash;
  fixed_bytes_32 zstate_root;
};

// struct OutputSliceFields
struct zOutputSliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// struct PrecompileResult
struct zPrecompileResult {
  struct zOutputSliceFields zoutput;
  bool zsuccess;
};

// type abbreviation OutputSlice
typedef struct zOutputSliceFields zOutputSlice;

// struct Message
struct zMessage {
  fixed_bytes_20 zaddress;
  fixed_bytes_20 zcaller;
  fixed_bytes_20 zcode_address;
  uint16_t zdepth;
  bool zis_static;
  uint64_t zstate_gas_reservoir;
  u256 zvalue;
};

// struct MemoryRangeFields
struct zMemoryRangeFields {
  uint32_t zlen;
  uint32_t zoff;
};

// type abbreviation MemoryRange
typedef struct zMemoryRangeFields zMemoryRange;

// struct MemoryAccessFields
struct zMemoryAccessFields {
  struct zMemoryRangeFields zrange;
  uint32_t zrequested_height;
};

// type abbreviation LogsBloomRef
typedef struct zStatelessInputSliceFields zLogsBloomRef;

#ifndef SAIL_FIXED_BYTES_256_DEFINED
#define SAIL_FIXED_BYTES_256_DEFINED
typedef struct { uint8_t bytes[256]; } fixed_bytes_256;
#endif

// type abbreviation LogsBloom
typedef fixed_bytes_256 zLogsBloom;

// struct tuple_(%struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256)
struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 {
  u256 ztup0;
  u256 ztup1;
};

// struct tuple_(%struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256)
struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 {
  u256 ztup0;
  u256 ztup1;
  u256 ztup2;
};

// struct tuple_(%struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256)
struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 {
  u256 ztup0;
  u256 ztup1;
  u256 ztup2;
  u256 ztup3;
};

// union LogTopics
enum kind_zLogTopics { Kind_zLogTopics0, Kind_zLogTopics1, Kind_zLogTopics2, Kind_zLogTopics3, Kind_zLogTopics4 };

struct zLogTopics {
  enum kind_zLogTopics kind;
  union {
    struct { unit zLogTopics0; };
    struct { u256 zLogTopics1; };
    struct { struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zLogTopics2; };
    struct { struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zLogTopics3; };
    struct { struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zLogTopics4; };
  } variants;
};

// struct LogSeriesRef
struct zLogSeriesRef {
  uint64_t zcount;
  uint64_t zstart;
};

// struct ReceiptFields
struct zReceiptFields {
  uint64_t zexecution_gas;
  uint64_t zgas_used;
  struct zLogSeriesRef zlogs;
  uint64_t zstate_gas;
  bool zsuccess;
  enum zTxType ztx_type;
};

// type abbreviation Receipt
typedef struct zReceiptFields zReceipt;

// struct LogDataSliceFields
struct zLogDataSliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// type abbreviation LogDataSliceLength
typedef struct zLogDataSliceFields zLogDataSliceLength;

// type abbreviation LogDataSlice
typedef struct zLogDataSliceFields zLogDataSlice;

// struct IntrinsicGasCost
struct zIntrinsicGasCost {
  uint64_t zcalldata_floor;
  uint64_t zexecution;
  uint64_t zstate;
};

// struct InlineNode
struct zInlineNode {
  fixed_bytes_32 zdata;
  uint8_t zlen;
};

// union NodeRef
enum kind_zNodeRef { Kind_zEmptyRef, Kind_zHashRef, Kind_zInputInlineRef, Kind_zScratchInlineRef };

struct zNodeRef {
  enum kind_zNodeRef kind;
  union {
    struct { unit zEmptyRef; };
    struct { fixed_bytes_32 zHashRef; };
    struct { struct zStatelessInputSliceFields zInputInlineRef; };
    struct { struct zInlineNode zScratchInlineRef; };
  } variants;
};

// union TrieItemValue
enum kind_zTrieItemValue { Kind_zBranchItem, Kind_zEmptySubtree, Kind_zLeafItem, Kind_zSubtreeItem };

struct zTrieItemValue {
  enum kind_zTrieItemValue kind;
  union {
    struct { struct zNodeRef zBranchItem; };
    struct { unit zEmptySubtree; };
    struct { struct zTrieLeafValue zLeafItem; };
    struct { struct zNodeRef zSubtreeItem; };
  } variants;
};

// struct TrieItem
struct zTrieItem {
  struct zTriePath zpath;
  struct zTrieItemValue zvalue;
};

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ816ZCZ0Z5UNIONZ0ZZNODEREFZ9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ816ZCZ0Z5UNIONZ0ZZNODEREFZ9_DEFINED
typedef struct zz5fvecz816zCz0z5unionz0zzNodeRefz9 {
  size_t len;
  struct zNodeRef data[16];
} zz5fvecz816zCz0z5unionz0zzNodeRefz9;
#endif

// struct TrieChildren
struct zTrieChildren {
  zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren;
  uint8_t zcount;
  uint64_t zmask;
  struct zTrieItem zonly;
};

// struct tuple_(%struct zTriePath, %union zNodeRef)
struct ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9 {
  struct zTriePath ztup0;
  struct zNodeRef ztup1;
};

// struct tuple_(%struct zTriePath, %struct zScratchSliceFields)
struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzScratchSliceFieldsz9 {
  struct zTriePath ztup0;
  struct zScratchSliceFields ztup1;
};

// struct tuple_(%fvec(16, %union zNodeRef), %struct zScratchSliceFields)
struct ztuple_z8z5fvecz816zCz0z5unionz0zzNodeRefz9zCz0z5structz0zzScratchSliceFieldsz9 {
  zz5fvecz816zCz0z5unionz0zzNodeRefz9 ztup0;
  struct zScratchSliceFields ztup1;
};

// union ScratchTrieNode
enum kind_zScratchTrieNode { Kind_zScratchBranchNode, Kind_zScratchExtensionNode, Kind_zScratchLeafNode };

struct zScratchTrieNode {
  enum kind_zScratchTrieNode kind;
  union {
    struct { struct ztuple_z8z5fvecz816zCz0z5unionz0zzNodeRefz9zCz0z5structz0zzScratchSliceFieldsz9 zScratchBranchNode; };
    struct { struct ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9 zScratchExtensionNode; };
    struct { struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzScratchSliceFieldsz9 zScratchLeafNode; };
  } variants;
};

// struct tuple_(%struct zTriePath, %struct zStatelessInputSliceFields)
struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzStatelessInputSliceFieldsz9 {
  struct zTriePath ztup0;
  struct zStatelessInputSliceFields ztup1;
};

// struct tuple_(%fvec(16, %union zNodeRef), %struct zStatelessInputSliceFields)
struct ztuple_z8z5fvecz816zCz0z5unionz0zzNodeRefz9zCz0z5structz0zzStatelessInputSliceFieldsz9 {
  zz5fvecz816zCz0z5unionz0zzNodeRefz9 ztup0;
  struct zStatelessInputSliceFields ztup1;
};

// union InputTrieNode
enum kind_zInputTrieNode { Kind_zInputBranchNode, Kind_zInputExtensionNode, Kind_zInputLeafNode };

struct zInputTrieNode {
  enum kind_zInputTrieNode kind;
  union {
    struct { struct ztuple_z8z5fvecz816zCz0z5unionz0zzNodeRefz9zCz0z5structz0zzStatelessInputSliceFieldsz9 zInputBranchNode; };
    struct { struct ztuple_z8z5structz0zzTriePathzCz0z5unionz0zzNodeRefz9 zInputExtensionNode; };
    struct { struct ztuple_z8z5structz0zzTriePathzCz0z5structz0zzStatelessInputSliceFieldsz9 zInputLeafNode; };
  } variants;
};

// struct IndexedTrieCursor
struct zIndexedTrieCursor {
  struct zRlpIndexCursor zkeys;
  struct zScratchSliceFields zreceipt_remaining;
  struct zScratchSliceFields zreceipt_zzero;
};

// enum HtrRequestKind
enum zHtrRequestKind { zHtrDeposit, zHtrWithdrawalRequest, zHtrConsolidationRequest, zHtrBuilderDepositRequest, zHtrBuilderExitRequest };

// union HaltKind
enum kind_zHaltKind { Kind_zHaltReturn, Kind_zHaltRevert, Kind_zHaltSelfDestruct, Kind_zHaltStop };

struct zHaltKind {
  enum kind_zHaltKind kind;
  union {
    struct { struct zOutputSliceFields zHaltReturn; };
    struct { struct zOutputSliceFields zHaltRevert; };
    struct { unit zHaltSelfDestruct; };
    struct { unit zHaltStop; };
  } variants;
};

// struct GasLimitsFields
struct zGasLimitsFields {
  uint64_t zblock_limit;
  uint32_t zsystem_regular_limit;
  uint8_t zsystem_state_limit;
  uint64_t ztransaction_regular_limit;
  uint64_t ztransaction_total_limit;
};

// struct GasCharge
struct zGasCharge {
  bool zaffordable;
  uint64_t zcost;
};

// type abbreviation Fork
typedef uint8_t zFork;

// enum FatalError
enum zFatalError { zInvalidConfig, zHeaderChainBroken, zRlpDecode, zInvalidSignature, zInvalidGasLimit, zGasUsedExceedsLimit, zBlobGasLimitExceeded, zExecutionInvalid, zInvalidGasUsed, zInvalidBlobGasUsed, zInvalidExcessBlobGas, zInvalidStateRoot, zInvalidReceiptsRoot, zInvalidLogsBloom, zInvalidBlockHash, zInvalidParentHash, zBlockAccessListTooLarge, zInvalidBlockAccessList, zInvalidExecutionRequests, zWitnessDeficient, zNumericOverflow };

// enum ExceptionKind
enum zExceptionKind { zStackUnderflow, zStackOverflow, zOutOfGas, zInvalidOpcode, zInvalidJump, zStaticViolation, zCallDepthExceeded, zInsufficientBalance, zWriteProtection, zInitCodeTooLarge, zNonceOverflow, zAddressCollision };

// union OpcodeOutcome
enum kind_zOpcodeOutcome { Kind_zContinue, Kind_zFailed };

struct zOpcodeOutcome {
  enum kind_zOpcodeOutcome kind;
  union {
    struct { unit zContinue; };
    struct { enum zExceptionKind zFailed; };
  } variants;
};

// union FrameStatus
enum kind_zFrameStatus { Kind_zExceptional, Kind_zHalted, Kind_zRunning };

struct zFrameStatus {
  enum kind_zFrameStatus kind;
  union {
    struct { enum zExceptionKind zExceptional; };
    struct { struct zHaltKind zHalted; };
    struct { unit zRunning; };
  } variants;
};

// struct ExceptionalStateTransition
struct zExceptionalStateTransition {
  uint64_t zstate_gas_remaining;
  uint32_t zstate_gas_spilled;
  struct zFrameStatus zstatus;
};

// struct EvmMemorySliceFields
struct zEvmMemorySliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// union LogData
enum kind_zLogData { Kind_zLogDataMemory, Kind_zLogDataWord };

struct zLogData {
  enum kind_zLogData kind;
  union {
    struct { struct zEvmMemorySliceFields zLogDataMemory; };
    struct { u256 zLogDataWord; };
  } variants;
};

// type abbreviation EvmMemorySlice
typedef struct zEvmMemorySliceFields zEvmMemorySlice;

// enum EnvField
enum zEnvField { zF_Number, zF_Timestamp, zF_Coinbase, zF_BaseFee, zF_ChainId, zF_GasLimit, zF_PrevRandao, zF_Origin, zF_GasPrice, zF_SlotNumber };

// enum DeepStackOperation
enum zDeepStackOperation { zDeepStackDuplicate, zDeepStackSwap, zDeepStackExchange, zNotDeepStackOperation };

// struct CreateSemantics
struct zCreateSemantics {bool zuses_salt;};

// enum CreateKind
enum zCreateKind { zCreateByNonce, zCreateBySalt };

// struct CodeRegionSliceFields
struct zCodeRegionSliceFields {
  uint32_t zbytes;
  uint32_t zlen;
};

// type abbreviation CodeSlice
typedef struct zCodeRegionSliceFields zCodeSlice;

// type abbreviation CodeRegionSlice
typedef struct zCodeRegionSliceFields zCodeRegionSlice;

// struct CodeFields
struct zCodeFields {
  uint32_t zbytes;
  uint64_t zjumpdests;
  uint32_t zlen;
};

// type abbreviation Code
typedef struct zCodeFields zCode;

// struct ChainConfig
struct zChainConfig {uint64_t zchain_id;};

// union CalldataSlice
enum kind_zCalldataSlice { Kind_zInputCalldata, Kind_zMemoryCalldata };

struct zCalldataSlice {
  enum kind_zCalldataSlice kind;
  union {
    struct { struct zStatelessInputSliceFields zInputCalldata; };
    struct { struct zEvmMemorySliceFields zMemoryCalldata; };
  } variants;
};

// struct FrameTransition
struct zFrameTransition {
  struct zCalldataSlice zcalldata;
  struct zCodeFields zcode;
  uint64_t zgas_remaining;
  uint32_t zmemory_base;
  uint32_t zmemory_height;
  struct zMessage zmessage;
  uint32_t zpc;
  __int128 zrefund;
  struct zOutputSliceFields zreturndata;
  struct zStackPointer zstack_top;
  uint64_t zstate_gas_remaining;
  uint32_t zstate_gas_spilled;
  struct zFrameStatus zstatus;
};

// struct FrameCheckpoint
struct zFrameCheckpoint {
  struct zCalldataSlice zcalldata;
  struct zCodeFields zcode;
  uint64_t zgas_remaining;
  uint32_t zmemory_height;
  struct zMessage zmessage;
  uint32_t zpc;
  __int128 zrefund;
  struct zStackPointer zstack_top;
  uint64_t zstate_gas_remaining;
  uint32_t zstate_gas_spilled;
  struct zFrameStatus zstatus;
};

// struct CreateContinuation
struct zCreateContinuation {
  fixed_bytes_20 zaddress;
  struct zFrameCheckpoint zcheckpoint;
  bool znew_account_charged;
};

// struct CallSemantics
struct zCallSemantics {
  bool zenters_static_context;
  bool zinherits_caller_and_value;
  bool ztakes_value;
  bool ztransfers_value;
  bool zuses_target_address;
};

// enum CallKind
enum zCallKind { zCall, zCallCode, zDelegateCall, zStaticCall };

// struct CallContinuation
struct zCallContinuation {
  struct zFrameCheckpoint zcheckpoint;
  bool znew_account_charged;
  uint32_t zreturn_length;
  uint32_t zreturn_offset;
};

// union FrameContinuation
enum kind_zFrameContinuation { Kind_zEmpty, Kind_zResumeCall, Kind_zResumeCreate };

struct zFrameContinuation {
  enum kind_zFrameContinuation kind;
  union {
    struct { unit zEmpty; };
    struct { struct zCallContinuation zResumeCall; };
    struct { struct zCreateContinuation zResumeCreate; };
  } variants;
};

// type abbreviation BranchRefs
typedef zz5fvecz816zCz0z5unionz0zzNodeRefz9 zBranchRefs;

// struct BoundedSszListRef
struct zBoundedSszzListRef {
  struct zStatelessInputSliceFields zbytes;
  uint32_t zcount;
  uint32_t zmax_item_length;
};

// type abbreviation WitnessNodeListRef
typedef struct zBoundedSszzListRef zWitnessNodeListRef;

// type abbreviation WitnessHeaderListRef
typedef struct zBoundedSszzListRef zWitnessHeaderListRef;

// type abbreviation WitnessCodeListRef
typedef struct zBoundedSszzListRef zWitnessCodeListRef;

// type abbreviation WithdrawalListRef
typedef struct zBoundedSszzListRef zWithdrawalListRef;

// type abbreviation TransactionListRef
typedef struct zBoundedSszzListRef zTransactionListRef;

// union IndexedTrieSource
enum kind_zIndexedTrieSource { Kind_zIndexedReceipts, Kind_zIndexedTransactions, Kind_zIndexedWithdrawals };

struct zIndexedTrieSource {
  enum kind_zIndexedTrieSource kind;
  union {
    struct { struct zReceiptRecordsRef zIndexedReceipts; };
    struct { struct zBoundedSszzListRef zIndexedTransactions; };
    struct { struct zBoundedSszzListRef zIndexedWithdrawals; };
  } variants;
};

// struct BoundedSszListCursor
struct zBoundedSszzListCursor {
  uint32_t zcurrent;
  uint32_t zindex;
  struct zBoundedSszzListRef zitems;
};

// type abbreviation WitnessNodeListCursor
typedef struct zBoundedSszzListCursor zWitnessNodeListCursor;

// type abbreviation WitnessHeaderListCursor
typedef struct zBoundedSszzListCursor zWitnessHeaderListCursor;

// struct WitnessHeaderIndex
struct zWitnessHeaderIndex {
  struct zBoundedSszzListCursor zcursor;
  u256 zparent_base_fee_per_gas;
  uint32_t zparent_blob_gas_used;
  uint32_t zparent_excess_blob_gas;
  bool zparent_fields_valid;
  fixed_bytes_32 zparent_state_root;
  fixed_bytes_32 zprevious_hash;
  bool zvalid;
};

// type abbreviation WitnessCodeListCursor
typedef struct zBoundedSszzListCursor zWitnessCodeListCursor;

// struct BlockHeader
struct zBlockHeader {
  u256 zbase_fee;
  uint32_t zblob_gas_used;
  uint32_t zexcess_blob_gas;
  struct zStatelessInputSliceFields zextra_data;
  fixed_bytes_20 zfee_recipient;
  uint64_t zgas_limit;
  uint64_t zgas_used;
  struct zStatelessInputSliceFields zlogs_bloom;
  uint64_t znumber;
  fixed_bytes_32 zparent_beacon_block_root;
  fixed_bytes_32 zparent_hash;
  u256 zprev_randao;
  fixed_bytes_32 zreceipts_root;
  uint64_t zslot_number;
  fixed_bytes_32 zstate_root;
  uint64_t ztimestamp;
};

// struct BlockGasUsageFields
struct zBlockGasUsageFields {
  uint64_t zexecution;
  u128 zreceipts;
  uint64_t zstate;
};

// type abbreviation BlockGasUsageFor
typedef struct zBlockGasUsageFields zBlockGasUsageFor;

// struct BlockExecutionResult
struct zBlockExecutionResult {
  uint32_t zblob_gas_used;
  uint64_t zexecution_gas_used;
  fixed_bytes_20 zfirst_tx_recipient;
  uint64_t zheader_gas_used;
  struct zLogSeriesRef zlogs;
  fixed_bytes_32 zreceipts_root;
  uint64_t zstate_gas_used;
};

// struct BlockBody
struct zBlockBody {
  struct zStatelessInputSliceFields zblock_access_list;
  struct zBoundedSszzListRef ztransactions;
  struct zBoundedSszzListRef zwithdrawals;
};

// struct Block
struct zBlock {
  struct zBlockBody zbody;
  struct zBlockHeader zheader;
};

// struct ExecutionPayload
struct zExecutionPayload {
  struct zBlock zblock;
  fixed_bytes_32 zexpected_block_hash;
};

// struct StatelessInput
struct zStatelessInput {
  struct zChainConfig zchain_config;
  struct zExecutionPayload zpayload;
};

// struct BlobScheduleFields
struct zBlobScheduleFields {
  uint32_t zbase_fee_update_fraction;
  uint8_t zmax;
  uint8_t ztarget;
};

// struct ProtocolProfileFields
struct zProtocolProfileFields {
  struct zBlobScheduleFields zblob_schedule;
  uint32_t zdeployed_code_sizze_limit;
  uint32_t zexcess_blob_gas_limit;
  uint8_t zfork;
  uint32_t zinitcode_sizze_limit;
  uint8_t zrefund_divisor;
  uint8_t ztransaction_blob_limit;
  uint64_t ztransaction_regular_gas_limit;
  uint64_t ztransaction_total_gas_limit;
};

// struct StatelessInputRef
struct zStatelessInputRef {
  struct zStatelessInputSliceFields zblock_access_list;
  struct zStatelessInputSliceFields zbuilder_deposit_requests;
  struct zStatelessInputSliceFields zbuilder_exit_requests;
  struct zStatelessInputSliceFields zchain_config;
  struct zStatelessInputSliceFields zconsolidation_requests;
  struct zStatelessInputSliceFields zdeposits;
  struct zStatelessInputSliceFields zexecution_payload;
  struct zStatelessInputSliceFields zextra_data;
  struct zStatelessInputSliceFields znew_payload_request;
  struct zProtocolProfileFields zprotocol;
  struct zStatelessInputSliceFields zpublic_keys;
  struct zBoundedSszzListRef ztransactions;
  struct zStatelessInputSliceFields zversioned_hashes;
  struct zStatelessInputSliceFields zwithdrawal_requests;
  struct zBoundedSszzListRef zwithdrawals;
  struct zBoundedSszzListRef zwitness_codes;
  struct zBoundedSszzListRef zwitness_headers;
  struct zBoundedSszzListRef zwitness_state;
};

// type abbreviation ProtocolProfile
typedef struct zProtocolProfileFields zProtocolProfile;

// struct ExecutionProfileFields
struct zExecutionProfileFields {
  struct zGasLimitsFields zgas;
  struct zProtocolProfileFields zprotocol;
};

// type abbreviation ExecutionProfile
typedef struct zExecutionProfileFields zExecutionProfile;

// struct BlobHashesFields
struct zBlobHashesFields {
  struct zStatelessInputSliceFields zbytes;
  uint8_t zcount;
};

// struct TxEnvFields
struct zTxEnvFields {
  struct zBlobHashesFields zblob_hashes;
  u256 zgas_price;
  fixed_bytes_20 zorigin;
};

// type abbreviation TxEnv
typedef struct zTxEnvFields zTxEnv;

// struct BalStorageChangeEntry
struct zBalStorageChangeEntry {
  uint32_t zindex;
  u256 zslot;
  u256 zvalue;
};

// struct BalNonceChangeEntry
struct zBalNonceChangeEntry {
  uint32_t zindex;
  uint64_t zvalue;
};

// struct BalCodeChangeEntry
struct zBalCodeChangeEntry {
  fixed_bytes_32 zcode_hash;
  uint32_t zindex;
};

// struct BalBalanceChangeEntry
struct zBalBalanceChangeEntry {
  uint32_t zindex;
  u256 zvalue;
};

// union BalIterEntry
enum kind_zBalIterEntry { Kind_zBalAccount, Kind_zBalAccountEnd, Kind_zBalBalanceChange, Kind_zBalCodeChange, Kind_zBalEmpty, Kind_zBalNonceChange, Kind_zBalStorageChange, Kind_zBalStorageRead };

struct zBalIterEntry {
  enum kind_zBalIterEntry kind;
  union {
    struct { fixed_bytes_20 zBalAccount; };
    struct { unit zBalAccountEnd; };
    struct { struct zBalBalanceChangeEntry zBalBalanceChange; };
    struct { struct zBalCodeChangeEntry zBalCodeChange; };
    struct { unit zBalEmpty; };
    struct { struct zBalNonceChangeEntry zBalNonceChange; };
    struct { struct zBalStorageChangeEntry zBalStorageChange; };
    struct { u256 zBalStorageRead; };
  } variants;
};

// struct AuthorizationListRefFields
struct zAuthorizzationListRefFields {
  uint32_t zcount;
  struct zStatelessInputSliceFields zencoded;
};

// type abbreviation AuthorizationListRef
typedef struct zAuthorizzationListRefFields zAuthorizzationListRef;

// struct Authorization
struct zAuthorizzation {
  fixed_bytes_20 zaddress;
  fixed_bytes_20 zauthority;
  u256 zchain_id;
  uint64_t znonce;
  bool zvalid_sig;
};

struct node_zz5listz8z5structz0zzAuthorizzzzationz9 {
  unsigned int rc;
  struct zAuthorizzation hd;
  struct node_zz5listz8z5structz0zzAuthorizzzzationz9 *tl;
};
typedef struct node_zz5listz8z5structz0zzAuthorizzzzationz9 *zz5listz8z5structz0zzAuthorizzzzationz9;

// struct PreparedAuthorizationList
struct zPreparedAuthorizzationList {
  uint16_t zcount;
  zz5listz8z5structz0zzAuthorizzzzationz9 zentries;
};

// struct AddressResult
struct zAddressResult {
  fixed_bytes_20 zaddress;
  bool zsuccess;
};

// struct AccountInfo
struct zAccountInfo {
  u256 zbalance;
  fixed_bytes_32 zcode_hash;
  uint64_t znonce;
  fixed_bytes_32 zstorage_root;
};

// struct AccountExecutionContext
struct zAccountExecutionContext {fixed_bytes_20 zaddress;};

// struct Account
struct zAccount {
  bool zcreated;
  struct zAccountInfo zinfo;
  bool zpresent;
  bool zselfdestructed;
  bool zstorage_cleared;
};

// struct AcctValue
struct zAcctValue {
  struct zAccount zcurr;
  struct zAccount zorig;
};

// struct AcctEntry
struct zAcctEntry {
  fixed_bytes_20 zaddr;
  struct zAcctValue zvalue;
};

// union AcctTxPopResult
enum kind_zAcctTxPopResult { Kind_zAcctTxPopExhausted, Kind_zAcctTxPopRow };

struct zAcctTxPopResult {
  enum kind_zAcctTxPopResult kind;
  union {
    struct { unit zAcctTxPopExhausted; };
    struct { struct zAcctEntry zAcctTxPopRow; };
  } variants;
};

// struct AcctTrieEntry
struct zAcctTrieEntry {
  fixed_bytes_32 zaddress_hash;
  struct zAcctEntry zentry;
};

// union AcctBlockIterResult
enum kind_zAcctBlockIterResult { Kind_zAcctBlockIterExhausted, Kind_zAcctBlockIterRow };

struct zAcctBlockIterResult {
  enum kind_zAcctBlockIterResult kind;
  union {
    struct { unit zAcctBlockIterExhausted; };
    struct { struct zAcctTrieEntry zAcctBlockIterRow; };
  } variants;
};

// struct AccountRow
struct zAccountRow {
  struct zAccount zaccount;
  bool zfound;
};

// struct AccessListRef
struct zAccessListRef {
  uint32_t zaddress_count;
  struct zStatelessInputSliceFields zencoded;
  uint32_t zslot_count;
};

// struct TransactionFields
struct zTransactionFields {
  struct zAccessListRef zaccess_list;
  struct zAuthorizzationListRefFields zauthorizzations;
  struct zBlobHashesFields zblob_hashes;
  uint64_t zchain_id;
  uint64_t zgas_limit;
  struct zStatelessInputSliceFields zinput_src;
  bool zis_create;
  u256 zmax_blob_fee;
  u256 zmax_fee;
  u256 zmax_priority_fee;
  u256 znonce;
  struct zStatelessInputSliceFields zpubkey;
  struct zStatelessInputSliceFields zraw;
  fixed_bytes_20 zrecipient;
  fixed_bytes_20 zsender;
  u256 zsig_r;
  u256 zsig_s;
  u256 zsig_v;
  fixed_bytes_32 zsigning_hash;
  enum zTxType ztx_type;
  u256 zvalue;
};

// type abbreviation Transaction
typedef struct zTransactionFields zTransaction;

// struct AccessListDecode
struct zAccessListDecode {
  uint32_t zaddress_count;
  uint32_t zslot_count;
};

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ832ZCZ0Z5BV8Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ832ZCZ0Z5BV8Z9_DEFINED
typedef struct zz5fvecz832zCz0z5bv8z9 {
  size_t len;
  uint64_t data[32];
} zz5fvecz832zCz0z5bv8z9;
#endif

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ820ZCZ0Z5BV8Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ820ZCZ0Z5BV8Z9_DEFINED
typedef struct zz5fvecz820zCz0z5bv8z9 {
  size_t len;
  uint64_t data[20];
} zz5fvecz820zCz0z5bv8z9;
#endif


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } u256;
#endif

#ifndef SAIL_U320_DEFINED
#define SAIL_U320_DEFINED
typedef struct { uint64_t limbs[5]; } u320;
#endif


#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ8256ZCZ0Z5BV8Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ8256ZCZ0Z5BV8Z9_DEFINED
typedef struct zz5fvecz8256zCz0z5bv8z9 {
  size_t len;
  uint64_t data[256];
} zz5fvecz8256zCz0z5bv8z9;
#endif

// struct tuple_(%bool, %u32, %u64)
struct ztuple_z8z5boolzCz0z5u32zCz0z5u64z9 {
  bool ztup0;
  uint32_t ztup1;
  uint64_t ztup2;
};

// struct tuple_(%bool, %u8, %u8)
struct ztuple_z8z5boolzCz0z5u8zCz0z5u8z9 {
  bool ztup0;
  uint8_t ztup1;
  uint8_t ztup2;
};

// struct tuple_(%bool, %u8, %u64)
struct ztuple_z8z5boolzCz0z5u8zCz0z5u64z9 {
  bool ztup0;
  uint8_t ztup1;
  uint64_t ztup2;
};

// struct tuple_(%bool, %struct z__sail_c_repr_fixed_bytes(20))
struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 {
  bool ztup0;
  fixed_bytes_20 ztup1;
};

// struct tuple_(%u64, %u64, %u32)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
};

// struct tuple_(%u64, %u64, %u8)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u8z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint8_t ztup2;
};

// struct tuple_(%u64, %u32)
struct ztuple_z8z5u64zCz0z5u32z9 {
  uint64_t ztup0;
  uint32_t ztup1;
};

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ8128ZCZ0Z5U16Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ8128ZCZ0Z5U16Z9_DEFINED
typedef struct zz5fvecz8128zCz0z5u16z9 {
  size_t len;
  uint16_t data[128];
} zz5fvecz8128zCz0z5u16z9;
#endif

// struct tuple_(%u16, %u16)
struct ztuple_z8z5u16zCz0z5u16z9 {
  uint16_t ztup0;
  uint16_t ztup1;
};

// struct tuple_(%u8, %u8)
struct ztuple_z8z5u8zCz0z5u8z9 {
  uint8_t ztup0;
  uint8_t ztup1;
};

// struct tuple_(%u64, %union zOpcodeOutcome)
struct ztuple_z8z5u64zCz0z5unionz0zzOpcodeOutcomez9 {
  uint64_t ztup0;
  struct zOpcodeOutcome ztup1;
};

// struct tuple_(%u8, %union zOpcodeOutcome)
struct ztuple_z8z5u8zCz0z5unionz0zzOpcodeOutcomez9 {
  uint8_t ztup0;
  struct zOpcodeOutcome ztup1;
};

// struct tuple_(%u32, %u64, %union zOpcodeOutcome)
struct ztuple_z8z5u32zCz0z5u64zCz0z5unionz0zzOpcodeOutcomez9 {
  uint32_t ztup0;
  uint64_t ztup1;
  struct zOpcodeOutcome ztup2;
};

// struct tuple_(%u32, %u8, %union zOpcodeOutcome)
struct ztuple_z8z5u32zCz0z5u8zCz0z5unionz0zzOpcodeOutcomez9 {
  uint32_t ztup0;
  uint8_t ztup1;
  struct zOpcodeOutcome ztup2;
};

// struct tuple_(%union zLogTopics, %struct zStackPointer)
struct ztuple_z8z5unionz0zzLogTopicszCz0z5structz0zzStackPointerz9 {
  struct zLogTopics ztup0;
  struct zStackPointer ztup1;
};

// struct tuple_(%u64, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint64_t ztup0;
  struct zStackPointer ztup1;
  struct zOpcodeOutcome ztup2;
};

// struct tuple_(%u8, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u8zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint8_t ztup0;
  struct zStackPointer ztup1;
  struct zOpcodeOutcome ztup2;
};

// struct tuple_(%u64, %struct zStackPointer, %u32, %union zOpcodeOutcome)
struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 {
  uint64_t ztup0;
  struct zStackPointer ztup1;
  uint32_t ztup2;
  struct zOpcodeOutcome ztup3;
};

// struct tuple_(%u8, %struct zStackPointer, %u32, %union zOpcodeOutcome)
struct ztuple_z8z5u8zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 {
  uint8_t ztup0;
  struct zStackPointer ztup1;
  uint32_t ztup2;
  struct zOpcodeOutcome ztup3;
};

// struct tuple_(%u64, %u64, %u32, %i128, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  struct zOpcodeOutcome ztup5;
};

// struct tuple_(%bool, %u64, %u64, %u32)
struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 {
  bool ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
};

// struct tuple_(%u8, %u64, %u32, %i128, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u8zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint8_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  struct zOpcodeOutcome ztup5;
};

// struct tuple_(%u32, %u64, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u32zCz0z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint32_t ztup0;
  uint64_t ztup1;
  struct zStackPointer ztup2;
  struct zOpcodeOutcome ztup3;
};

// struct tuple_(%u32, %u8, %struct zStackPointer, %union zOpcodeOutcome)
struct ztuple_z8z5u32zCz0z5u8zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 {
  uint32_t ztup0;
  uint8_t ztup1;
  struct zStackPointer ztup2;
  struct zOpcodeOutcome ztup3;
};

// struct tuple_(%u64, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint64_t ztup0;
  struct zStackPointer ztup1;
  uint32_t ztup2;
  struct zFrameStatus ztup3;
};

// struct tuple_(%u8, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u8zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint8_t ztup0;
  struct zStackPointer ztup1;
  uint32_t ztup2;
  struct zFrameStatus ztup3;
};

// struct tuple_(%u64, %u64, %u32, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  struct zStackPointer ztup3;
  uint32_t ztup4;
  struct zFrameStatus ztup5;
};

// struct tuple_(%u8, %u64, %u32, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u8zCz0z5u64zCz0z5u32zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint8_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  struct zStackPointer ztup3;
  uint32_t ztup4;
  struct zFrameStatus ztup5;
};

// struct tuple_(%u64, %u64, %u32, %i128, %struct zStackPointer, %union zFrameStatus)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzFrameStatusz9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  struct zFrameStatus ztup5;
};

// struct tuple_(%u8, %u64, %u32, %i128, %struct zStackPointer, %union zFrameStatus)
struct ztuple_z8z5u8zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzFrameStatusz9 {
  uint8_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  struct zFrameStatus ztup5;
};

// struct tuple_(%bool, %u8, %u64, %u32)
struct ztuple_z8z5boolzCz0z5u8zCz0z5u64zCz0z5u32z9 {
  bool ztup0;
  uint8_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
};

// struct tuple_(%u64, %u64, %u32, %i128, %struct zStackPointer, %u32, %u32)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5u32z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  uint32_t ztup5;
  uint32_t ztup6;
};

// struct tuple_(%u64, %u64, %u8, %i128, %struct zStackPointer, %u32, %u32)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u8zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5u32z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint8_t ztup2;
  __int128 ztup3;
  struct zStackPointer ztup4;
  uint32_t ztup5;
  uint32_t ztup6;
};

// struct tuple_(%u64, %u64, %u32, %i128, %union zFrameStatus, %struct zOutputSliceFields)
struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint32_t ztup2;
  __int128 ztup3;
  struct zFrameStatus ztup4;
  struct zOutputSliceFields ztup5;
};

// struct tuple_(%struct zTransactionPreparation, %u64, %u64, %u32, %struct z__sail_c_repr_fixed_bytes(20), %struct z__sail_c_repr_fixed_bytes(20), %struct zCodeFields, %union zCalldataSlice)
struct ztuple_z8z5structz0zzTransactionPreparationzCz0z5u64zCz0z5u64zCz0z5u32zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zzCodeFieldszCz0z5unionz0zzCalldataSlicez9 {
  struct zTransactionPreparation ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
  fixed_bytes_20 ztup4;
  fixed_bytes_20 ztup5;
  struct zCodeFields ztup6;
  struct zCalldataSlice ztup7;
};

// struct tuple_(%bool, %struct zTriePath)
struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 {
  bool ztup0;
  struct zTriePath ztup1;
};

// struct tuple_(%struct zTrieUpdate, %struct zTrieUpdateCursor)
struct ztuple_z8z5structz0zzTrieUpdatezCz0z5structz0zzTrieUpdateCursorz9 {
  struct zTrieUpdate ztup0;
  struct zTrieUpdateCursor ztup1;
};

// struct tuple_(%struct zRlpIndexItem, %struct zRlpIndexCursor)
struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 {
  struct zRlpIndexItem ztup0;
  struct zRlpIndexCursor ztup1;
};

// struct tuple_(%struct zTrieItem, %struct zTrieUpdateCursor)
struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 {
  struct zTrieItem ztup0;
  struct zTrieUpdateCursor ztup1;
};

// struct tuple_(%struct zTrieUpdate, %bool)
struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 {
  struct zTrieUpdate ztup0;
  bool ztup1;
};

// struct tuple_(%struct zStatelessInputSliceFields, %struct zBoundedSszzListCursor)
struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 {
  struct zStatelessInputSliceFields ztup0;
  struct zBoundedSszzListCursor ztup1;
};

// struct tuple_(%struct zStatelessInputSliceFields, %struct zSszzContainerCursor)
struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 {
  struct zStatelessInputSliceFields ztup0;
  struct zSszzContainerCursor ztup1;
};

// struct tuple_(%struct zScratchSliceFields, %struct zScratchSliceFields)
struct ztuple_z8z5structz0zzScratchSliceFieldszCz0z5structz0zzScratchSliceFieldsz9 {
  struct zScratchSliceFields ztup0;
  struct zScratchSliceFields ztup1;
};

// struct tuple_(%struct zStatelessInputSliceFields, %struct zBoundedSszzListRef)
struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 {
  struct zStatelessInputSliceFields ztup0;
  struct zBoundedSszzListRef ztup1;
};

// struct tuple_(%struct zTrieItem, %struct zIndexedTrieCursor)
struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzIndexedTrieCursorz9 {
  struct zTrieItem ztup0;
  struct zIndexedTrieCursor ztup1;
};

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ84ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ84ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
typedef struct zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 data[4];
} zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ819ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ819ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
typedef struct zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 data[19];
} zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ85ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ85ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
typedef struct zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 data[5];
} zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ83ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ83ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
typedef struct zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 data[3];
} zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

#ifndef SAIL_FIXED_VECTOR_ZZ5FVECZ82ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_FIXED_VECTOR_ZZ5FVECZ82ZCZ0Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
typedef struct zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 data[2];
} zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

// struct tuple_(%bool, %u64, %u8, %u32)
struct ztuple_z8z5boolzCz0z5u64zCz0z5u8zCz0z5u32z9 {
  bool ztup0;
  uint64_t ztup1;
  uint8_t ztup2;
  uint32_t ztup3;
};

// struct tuple_(%u32, %bv8)
struct ztuple_z8z5u32zCz0z5bv8z9 {
  uint32_t ztup0;
  uint64_t ztup1;
};

// struct tuple_(%u32, %struct z__sail_c_repr_u256)
struct ztuple_z8z5u32zCz0z5structz0zz__sail_c_repr_u256z9 {
  uint32_t ztup0;
  u256 ztup1;
};

// struct tuple_(%u32, %union zast)
struct ztuple_z8z5u32zCz0z5unionz0zzastz9 {
  uint32_t ztup0;
  struct zast ztup1;
};

// struct tuple_(%u32, %u8, %u64, %u32, %i128, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u32zCz0z5u8zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint32_t ztup0;
  uint8_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
  __int128 ztup4;
  struct zStackPointer ztup5;
  uint32_t ztup6;
  struct zFrameStatus ztup7;
};

// struct tuple_(%u32, %u64, %u64, %u32, %i128, %struct zStackPointer, %u32, %union zFrameStatus)
struct ztuple_z8z5u32zCz0z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 {
  uint32_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
  __int128 ztup4;
  struct zStackPointer ztup5;
  uint32_t ztup6;
  struct zFrameStatus ztup7;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5STRUCTZ0ZZ__SAIL_C_REPR_FIXED_BYTESZ832Z9Z9_DEFINED
struct zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  size_t len;
  fixed_bytes_32 *data;
};
typedef struct zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;
#endif

// struct tuple_(%struct zTransactionPreparation, %u8, %u64, %u32, %struct z__sail_c_repr_fixed_bytes(20), %struct z__sail_c_repr_fixed_bytes(20), %struct zCodeFields, %union zCalldataSlice)
struct ztuple_z8z5structz0zzTransactionPreparationzCz0z5u8zCz0z5u64zCz0z5u32zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zzCodeFieldszCz0z5unionz0zzCalldataSlicez9 {
  struct zTransactionPreparation ztup0;
  uint8_t ztup1;
  uint64_t ztup2;
  uint32_t ztup3;
  fixed_bytes_20 ztup4;
  fixed_bytes_20 ztup5;
  struct zCodeFields ztup6;
  struct zCalldataSlice ztup7;
};

// struct tuple_(%struct z__sail_c_repr_u256, %struct zStackPointer)
struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zzStackPointerz9 {
  u256 ztup0;
  struct zStackPointer ztup1;
};

// struct tuple_(%struct zFrameCheckpoint, %struct zStackPointer, %u32, %u32)
struct ztuple_z8z5structz0zzFrameCheckpointzCz0z5structz0zzStackPointerzCz0z5u32zCz0z5u32z9 {
  struct zFrameCheckpoint ztup0;
  struct zStackPointer ztup1;
  uint32_t ztup2;
  uint32_t ztup3;
};

bool zneq_bool(bool zx, bool zy);

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(fixed_bytes_32 zx, fixed_bytes_32 zy);

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(fixed_bytes_20 zx, fixed_bytes_20 zy);

bool zneq_anythingzIEPrecompileIdz5zK(enum zPrecompileId zx, enum zPrecompileId zy);

u256 zu256(u256 zvalue);

fixed_bytes_32 zB256(fixed_bytes_32 zbytes);

u256 zhash_to_word(fixed_bytes_32 zbytes);

fixed_bytes_32 zword_to_hash(u256 zvalue);

fixed_bytes_20 zword_to_address(u256 zvalue);

void create_letbind_0(void);
void kill_letbind_0(void);


void create_letbind_1(void);
void kill_letbind_1(void);


void create_letbind_2(void);
void kill_letbind_2(void);


void create_letbind_3(void);
void kill_letbind_3(void);


void create_letbind_4(void);
void kill_letbind_4(void);


void create_letbind_5(void);
void kill_letbind_5(void);


void create_letbind_6(void);
void kill_letbind_6(void);


u256 zword_add_word(u256 zleft, u256 zright);

u256 zword_sub_word(u256 zleft, u256 zright);

u256 zword_and(u256 zleft, u256 zright);

u256 zword_or(u256 zleft, u256 zright);

u256 zword_xor(u256 zleft, u256 zright);

u256 zword_not(u256 zvalue);

uint64_t zword_bit(u256 zvalue, uint8_t zindex);

uint64_t zword_low_byte(u256 zvalue);

u256 zword_shift_right_one(u256 zvalue);

u256 zword_of_bool(bool zb);

bool zword_is_zzero(u256 zw);

bool zword_nonzzero(u256 zw);

bool zword_ult(u256 za, u256 zb);

bool zword_ule(u256 za, u256 zb);

uint8_t zu64_bit_length(uint64_t zvalue);

uint16_t zword_bit_length(u256 zvalue);

u256 zword_mul_word(u256 za, u256 zb);

u256 zword_div_word(u256 zdividend, u256 zdivisor);

u256 zword_mod_word(u256 zdividend, u256 zdivisor);

bool zword_greater_than_word(u256 zleft, u256 zright);

u256 zword_shift_left(u256 zvalue, uint16_t zamount);

u256 zword_shift_right(u256 zvalue, uint16_t zamount);

uint8_t zword_byte_length(u256 zvalue);

u256 zword_arithmetic_shift_right(u256 zvalue, uint16_t zamount);

u256 zaddress_to_word(fixed_bytes_20 zbytes);

u256 zword_negate(u256 zvalue);

u256 zword_abs(u256 zvalue);

bool zword_slt(u256 za, u256 zb);

u256 zalu_add(u256 za, u256 zb);

u256 zalu_sub(u256 za, u256 zb);

u256 zalu_mul(u256 za, u256 zb);

u256 zalu_div(u256 za, u256 zb);

u256 zalu_mod(u256 za, u256 zb);

u256 zalu_sdiv(u256 za, u256 zb);

u256 zalu_smod(u256 za, u256 zb);

u256 zalu_addmod(u256 za, u256 zb, u256 zn);

u256 zalu_mulmod(u256 za, u256 zb, u256 zn);

u256 zalu_exp(u256 zbase, u256 zexponent);

u256 zalu_signextend(u256 zbyte_index, u256 zvalue);

u256 zalu_lt(u256 za, u256 zb);

u256 zalu_gt(u256 za, u256 zb);

u256 zalu_slt(u256 za, u256 zb);

u256 zalu_sgt(u256 za, u256 zb);

u256 zalu_eq(u256 za, u256 zb);

u256 zalu_iszzero(u256 za);

u256 zalu_and(u256 za, u256 zb);

u256 zalu_or(u256 za, u256 zb);

u256 zalu_xor(u256 za, u256 zb);

u256 zalu_not(u256 za);

u256 zalu_byte(u256 zi, u256 zx);

u256 zalu_shl(u256 zshift_amt, u256 zv);

u256 zalu_shr(u256 zshift_amt, u256 zv);

u256 zalu_sar(u256 zshift_amt, u256 zv);

u256 zalu_clzz(u256 zx);

uint64_t zword_of_account_nonce(uint64_t zvalue);

uint64_t zword_of_slot_number(uint64_t zvalue);

uint64_t zword_of_block_number(uint64_t zvalue);

uint64_t zword_of_block_timestamp(uint64_t zvalue);

uint64_t zword_of_chain_identifier(uint64_t zvalue);

struct zMemoryRangeFields zmemory_range(uint32_t zoff, uint32_t zlen);

void create_letbind_7(void);
void kill_letbind_7(void);


void create_letbind_8(void);
void kill_letbind_8(void);


u256 zword_of_source_byte_count(uint32_t zvalue);

void create_letbind_9(void);
void kill_letbind_9(void);


void create_letbind_10(void);
void kill_letbind_10(void);


void create_letbind_11(void);
void kill_letbind_11(void);


struct zGasCharge zgas_charge(uint64_t zcost);

void create_letbind_12(void);
void kill_letbind_12(void);


void create_letbind_13(void);
void kill_letbind_13(void);


void create_letbind_14(void);
void kill_letbind_14(void);


void create_letbind_15(void);
void kill_letbind_15(void);


void create_letbind_16(void);
void kill_letbind_16(void);


void create_letbind_17(void);
void kill_letbind_17(void);


void create_letbind_18(void);
void kill_letbind_18(void);


void create_letbind_19(void);
void kill_letbind_19(void);


void create_letbind_20(void);
void kill_letbind_20(void);


void create_letbind_21(void);
void kill_letbind_21(void);


uint32_t zcalldata_slice_length(struct zCalldataSlice zs);

uint32_t zstateless_input_slice_length(struct zStatelessInputSliceFields zs);

void create_letbind_22(void);
void kill_letbind_22(void);


void create_letbind_23(void);
void kill_letbind_23(void);


void create_letbind_24(void);
void kill_letbind_24(void);


void create_letbind_25(void);
void kill_letbind_25(void);


struct zStatelessInputSliceFields zstateless_input_slice(uint32_t zoff, uint32_t zlen);

struct zEvmMemorySliceFields zevm_memory_slice(uint32_t zoff, uint32_t zlen);

struct zLogDataSliceFields zlog_data_slice(uint32_t zoff, uint32_t zlen);

void create_letbind_26(void);
void kill_letbind_26(void);


void create_letbind_27(void);
void kill_letbind_27(void);


void create_letbind_28(void);
void kill_letbind_28(void);


void create_letbind_29(void);
void kill_letbind_29(void);


void create_letbind_30(void);
void kill_letbind_30(void);


void create_letbind_31(void);
void kill_letbind_31(void);


void create_letbind_32(void);
void kill_letbind_32(void);


struct zStatelessInputSliceFields zstateless_input_sub_slice(struct zStatelessInputSliceFields zs, uint32_t zoff, uint32_t zlen);

struct zScratchSliceFields zscratch_sub_slice(struct zScratchSliceFields zs, uint32_t zoff, uint32_t zlen);

struct zEvmMemorySliceFields zmemory_sub_slice(struct zEvmMemorySliceFields zs, uint32_t zoff, uint32_t zlen);

struct zStatelessInputSliceFields zstateless_input_slice_suffix(struct zStatelessInputSliceFields zs, uint32_t zoff);

struct zScratchSliceFields zscratch_slice_suffix(struct zScratchSliceFields zs, uint32_t zoff);

void create_letbind_33(void);
void kill_letbind_33(void);


struct zCodeRegionSliceFields zcode_slice(struct zCodeRegionSliceFields zbytes);

struct zCodeRegionSliceFields zvalidated_code_slice(struct zCodeRegionSliceFields zbytes);

void create_letbind_34(void);
void kill_letbind_34(void);


enum zDeepStackOperation zdeep_stack_operation(uint8_t zopcode);

bool zdeep_stack_immediate_valid(uint64_t zimmediate);

bool zexchange_immediate_valid(uint64_t zimmediate);

bool zdeep_stack_operation_immediate_valid(enum zDeepStackOperation zoperation, uint64_t zimmediate);

struct zCodeFields zanalyzzed_code(struct zCodeRegionSliceFields zbytes, uint64_t zjumpdests);

struct zCodeRegionSliceFields zcode_bytes(struct zCodeFields zcode);

void create_letbind_35(void);
void kill_letbind_35(void);


uint64_t zstateless_input_slice_byte(struct zStatelessInputSliceFields zs, uint32_t zoff);

uint64_t zmemory_slice_byte(struct zEvmMemorySliceFields zs, uint32_t zoff);

uint64_t zcode_slice_byte(struct zCodeRegionSliceFields zs, uint32_t zoff);

uint64_t zscratch_byte(struct zScratchSliceFields zs, uint32_t zoff);

uint64_t zcalldata_slice_byte(struct zCalldataSlice zs, uint32_t zoff);

uint32_t zslice_count_nonzzero(struct zStatelessInputSliceFields zs);

u256 zstateless_input_slice_load(struct zStatelessInputSliceFields zs, uint32_t zoff);

u256 zmemory_slice_load(struct zEvmMemorySliceFields zs, uint32_t zoff);

u256 zcalldata_slice_load(struct zCalldataSlice zs, uint32_t zoff);

u256 zstateless_input_slice_load_word_offset(struct zStatelessInputSliceFields zs, u256 zoff);

u256 zmemory_slice_load_word_offset(struct zEvmMemorySliceFields zs, u256 zoff);

u256 zcalldata_slice_load_word_offset(struct zCalldataSlice zs, u256 zoff);

u256 zstateless_input_slice_load_n(struct zStatelessInputSliceFields zs, uint32_t zoff, uint8_t zn);

u256 zcode_slice_load_n(struct zCodeRegionSliceFields zs, uint32_t zoff, uint8_t zn);

u256 zscratch_slice_load_n(struct zScratchSliceFields zs, uint32_t zoff, uint8_t zn);

unit zstateless_input_slice_copy(struct zStatelessInputSliceFields zs, uint32_t zdst, uint32_t zoff, uint32_t zlen);

unit zmemory_slice_copy(struct zEvmMemorySliceFields zs, uint32_t zdst, uint32_t zoff, uint32_t zlen);

unit zcode_slice_copy(struct zCodeRegionSliceFields zs, uint32_t zdst, uint32_t zoff, uint32_t zlen);

unit zoutput_slice_copy(struct zOutputSliceFields zs, uint32_t zdst, uint32_t zoff, uint32_t zlen);

unit zstateless_input_slice_copy_word_offset(struct zStatelessInputSliceFields zs, uint32_t zdst, u256 zoff, uint32_t zlen);

unit zmemory_slice_copy_word_offset(struct zEvmMemorySliceFields zs, uint32_t zdst, u256 zoff, uint32_t zlen);

unit zcode_slice_copy_word_offset(struct zCodeRegionSliceFields zs, uint32_t zdst, u256 zoff, uint32_t zlen);

unit zcalldata_slice_copy_word_offset(struct zCalldataSlice zs, uint32_t zdst, u256 zoff, uint32_t zlen);

uint32_t zscratch_begin(unit z3zE5100);

uint32_t zscratch_reserve(uint32_t zlen);

unit zscratch_push_byte(uint64_t zdata);

unit zstateless_input_scratch_push_slice(struct zStatelessInputSliceFields zdata);

unit zscratch_scratch_push_slice(struct zScratchSliceFields zdata);

unit zlog_data_scratch_push_slice(struct zLogDataSliceFields zdata);

unit zoutput_scratch_push_slice(struct zOutputSliceFields zdata);

unit zscratch_push_address(fixed_bytes_20 zdata);

unit zscratch_push_b256(fixed_bytes_32 zdata, uint8_t zlen);

unit zscratch_push_fixed_bytes_256(fixed_bytes_256 zdata);

unit zscratch_push_word_be(u256 zdata, uint8_t zlen);

struct zScratchSliceFields zscratch_finish(uint32_t zstart);

unit zscratch_rewind(uint32_t zmark);

unit zscratch_reset(unit z3zE5095);

void create_letbind_36(void);
void kill_letbind_36(void);


void create_letbind_37(void);
void kill_letbind_37(void);


fixed_bytes_32 zcalldata_sha256(struct zCalldataSlice zinput);

void create_letbind_38(void);
void kill_letbind_38(void);


void create_letbind_39(void);
void kill_letbind_39(void);


struct zAddressResult zecrecover_addr(fixed_bytes_32 zh, uint8_t zyparity, u256 zr, u256 zs);

void create_letbind_40(void);
void kill_letbind_40(void);


void create_letbind_41(void);
void kill_letbind_41(void);


void create_letbind_42(void);
void kill_letbind_42(void);


void create_letbind_43(void);
void kill_letbind_43(void);


void create_letbind_44(void);
void kill_letbind_44(void);


void create_letbind_45(void);
void kill_letbind_45(void);


void create_letbind_46(void);
void kill_letbind_46(void);


void create_letbind_47(void);
void kill_letbind_47(void);


void create_letbind_48(void);
void kill_letbind_48(void);


void create_letbind_49(void);
void kill_letbind_49(void);


void create_letbind_50(void);
void kill_letbind_50(void);


void create_letbind_51(void);
void kill_letbind_51(void);


void create_letbind_52(void);
void kill_letbind_52(void);


void create_letbind_53(void);
void kill_letbind_53(void);


void create_letbind_54(void);
void kill_letbind_54(void);


void create_letbind_55(void);
void kill_letbind_55(void);


void create_letbind_56(void);
void kill_letbind_56(void);


struct zProtocolProfileFields zpack_protocol_profile(struct zProtocolProfileFields zprofile);

struct zGasLimitsFields zgas_limits_for(struct zProtocolProfileFields zprofile, uint64_t zblock_limit);

struct zExecutionProfileFields zexecution_profile_for(struct zProtocolProfileFields zprotocol, uint64_t zblock_limit);

struct zProtocolProfileFields zschema_protocol_profile(uint64_t zschema_fork);

bool zschema_protocol_profile_forwards_matches(uint64_t zschema_fork);

void create_letbind_57(void);
void kill_letbind_57(void);


void create_letbind_58(void);
void kill_letbind_58(void);


void create_letbind_59(void);
void kill_letbind_59(void);


void create_letbind_60(void);
void kill_letbind_60(void);


void create_letbind_61(void);
void kill_letbind_61(void);


void create_letbind_62(void);
void kill_letbind_62(void);


void create_letbind_63(void);
void kill_letbind_63(void);


void create_letbind_64(void);
void kill_letbind_64(void);


void create_letbind_65(void);
void kill_letbind_65(void);


void create_letbind_66(void);
void kill_letbind_66(void);


void create_letbind_67(void);
void kill_letbind_67(void);


void create_letbind_68(void);
void kill_letbind_68(void);


void create_letbind_69(void);
void kill_letbind_69(void);


void create_letbind_70(void);
void kill_letbind_70(void);


void create_letbind_71(void);
void kill_letbind_71(void);


void create_letbind_72(void);
void kill_letbind_72(void);


struct zAccount zaccount_from_info(struct zAccountInfo zinfo);

uint64_t ztx_envelope_type(enum zTxType zt);

struct zTxTypeSemantics ztx_type_semantics(enum zTxType zt);

void create_letbind_73(void);
void kill_letbind_73(void);


void create_letbind_74(void);
void kill_letbind_74(void);


struct zAuthorizzationListRefFields zauthorizzation_list_ref(struct zStatelessInputSliceFields zencoded, uint32_t zcount);

void create_letbind_75(void);
void kill_letbind_75(void);


struct zTransactionFields zpack_transaction(struct zTransactionFields ztx);

uint64_t zlog_store_index_increment(uint64_t zvalue);

uint64_t zlog_store_index_add(uint64_t zleft, uint64_t zright);

struct zReceiptFields zreceipt_fields(uint64_t z_limit, uint64_t z_regular_limit, enum zTxType ztx_type, bool zsuccess, uint64_t zgas_used, uint64_t zexecution_gas, uint64_t zstate_gas, struct zLogSeriesRef zlogs);

struct zReceiptFields zreceipt_within(uint64_t zlimit, uint64_t zregular_limit, enum zTxType ztx_type, bool zsuccess, uint64_t zgas_used, uint64_t zexecution_gas, uint64_t zstate_gas, struct zLogSeriesRef zlogs);

void create_letbind_76(void);
void kill_letbind_76(void);


bool zlogs_bloom_equal(fixed_bytes_256 za, fixed_bytes_256 zb);

fixed_bytes_256 zlogs_bloom_from_ref(struct zStatelessInputSliceFields zreference);

bool zlogs_bloom_matches_ref(fixed_bytes_256 zcomputed, struct zStatelessInputSliceFields zreference);

void create_letbind_77(void);
void kill_letbind_77(void);


struct zTxEnvFields ztx_env(fixed_bytes_20 zorigin, u256 zgas_price, struct zBlobHashesFields zblob_hashes);

struct zTransactionInitialGasFields ztransaction_initial_gas_fields(uint64_t ztotal, uint64_t zregular, uint64_t zintrinsic_execution, uint64_t zintrinsic_state, uint64_t zcalldata_floor, uint64_t zexecution, uint64_t zstate);

struct zTxValidityFields ztx_validity_fields(fixed_bytes_20 zsender, uint64_t znonce_before, struct zTransactionInitialGasFields zgas, u256 zblob_fee, u256 zgas_price, u256 zpriority_fee);

struct zTxFrameGasSnapshotFields ztx_frame_gas_snapshot_fields(uint64_t zlimit, uint64_t zregular, uint64_t zcalldata_floor, uint64_t zremaining, uint64_t zstate_used);

void create_letbind_78(void);
void kill_letbind_78(void);


struct zCodeRegionSliceFields zcode_db_intern_input(struct zStatelessInputSliceFields zbytes);

struct zCodeRegionSliceFields zcode_db_intern_memory(struct zEvmMemorySliceFields zbytes);

struct zCodeRegionSliceFields zcode_db_intern_output(struct zOutputSliceFields zbytes);

struct zCodeFields zcode_db_resolve(fixed_bytes_32 zcode_hash);

void create_letbind_79(void);
void kill_letbind_79(void);


void create_letbind_80(void);
void kill_letbind_80(void);


uint32_t zsszz_u32_at(struct zStatelessInputSliceFields zinput, uint32_t zoffset);

uint32_t zsszz_u32_in_slice(struct zStatelessInputSliceFields zinput, uint32_t zoffset);

uint32_t zsszz_offset_to_source_pointer(uint32_t zvalue);

uint64_t zdecode_sszz_uint(struct zStatelessInputSliceFields zinput, uint32_t zoffset);

uint8_t zsszz_u256_index(uint8_t zindex);

void create_letbind_81(void);
void kill_letbind_81(void);


void create_letbind_82(void);
void kill_letbind_82(void);


void create_letbind_83(void);
void kill_letbind_83(void);


void create_letbind_84(void);
void kill_letbind_84(void);


void create_letbind_85(void);
void kill_letbind_85(void);


uint32_t zrlp_scratch_small_length(uint8_t zvalue);

uint64_t zrlp_nat_length_byte(uint8_t zvalue);

uint8_t zrlp_minimal_word_len(u256 zw);

uint32_t zrlp_materializzed_slice_sizze(uint32_t zlength, uint64_t zfirst);

uint32_t zrlp_scratch_region_sizze(struct zScratchSliceFields zdata);

uint32_t zrlp_log_data_sizze(struct zLogDataSliceFields zdata);

uint8_t zrlp_uint_word_sizze(u256 zw);

uint32_t zrlp_scratch_list_sizze(uint32_t zcontent_len);

uint32_t zrlp_input_scratch_slice_sizze(struct zStatelessInputSliceFields zdata);

uint32_t zrlp_scratch_scratch_slice_sizze(struct zScratchSliceFields zdata);

uint32_t zrlp_log_scratch_slice_sizze(struct zLogDataSliceFields zdata);

unit zrlp_write_input_slice(struct zStatelessInputSliceFields zdata);

unit zrlp_write_scratch_slice(struct zScratchSliceFields zdata);

unit zrlp_write_log_data_slice(struct zLogDataSliceFields zdata);

unit zrlp_write_uint_word(u256 zw);

unit zrlp_write_word(u256 zw);

unit zrlp_write_addr(fixed_bytes_20 za);

struct zRlpEncoder zrlp_encoder_begin(uint32_t zexpected_len);

struct zScratchSliceFields zrlp_encoder_finish(struct zRlpEncoder zencoder);

unit zrlp_encoder_rewind(struct zRlpEncoder zencoder);

uint64_t zrlp_uint64_append(uint8_t z_width, uint64_t zprefix, uint64_t znext);

struct ztuple_z8z5boolzCz0z5u32zCz0z5u64z9 zrlp_ref_hdr(struct zStatelessInputSliceFields zb);

struct zStatelessInputSliceFields zrlp_decode_list(struct zRlpFieldRef zf);

struct zRlpFieldRef zrlp_decode_item(struct zStatelessInputSliceFields zcursor);

struct zStatelessInputSliceFields zrlp_cursor_advance(struct zStatelessInputSliceFields zcursor, uint32_t zconsumed);

unit zrlp_cursor_expect_end(struct zStatelessInputSliceFields zcursor);

struct zRlpFieldRef zrlp_single_ref(struct zStatelessInputSliceFields zitem);

struct zStatelessInputSliceFields zrlp_node_cursor(struct zStatelessInputSliceFields znode);

struct zStatelessInputSliceFields zrlp_item_content(struct zRlpFieldRef zf);

struct ztuple_z8z5boolzCz0z5u32zCz0z5u64z9 zscratch_rlp_ref_hdr(struct zScratchSliceFields zb);

struct zScratchSliceFields zscratch_rlp_decode_list(struct zScratchRlpFieldRef zf);

struct zScratchRlpFieldRef zscratch_rlp_decode_item(struct zScratchSliceFields zcursor);

struct zScratchSliceFields zscratch_rlp_cursor_advance(struct zScratchSliceFields zcursor, uint32_t zconsumed);

unit zscratch_rlp_cursor_expect_end(struct zScratchSliceFields zcursor);

struct zScratchRlpFieldRef zscratch_rlp_single_ref(struct zScratchSliceFields zitem);

struct zScratchSliceFields zscratch_rlp_node_cursor(struct zScratchSliceFields znode);

struct zScratchSliceFields zscratch_rlp_item_content(struct zScratchRlpFieldRef zf);

u256 zscratch_rlp_decode_word(struct zScratchRlpFieldRef zf);

bool zrlp_ref_framing_canonical(struct zRlpFieldRef zf);

bool zrlp_ref_bytes_canonical(struct zRlpFieldRef zf);

bool zrlp_item_uint_canonical(struct zRlpFieldRef zf);

u256 zrlp_decode_word(struct zRlpFieldRef zf);

u256 zrlp_decode_u256(struct zRlpFieldRef zf);

uint64_t zrlp_decode_uint64(struct zRlpFieldRef zf);

void zrlp_decode_bool(struct zRlpResultzIozK *rop, struct zRlpFieldRef zfield);

fixed_bytes_20 zcreate_address(fixed_bytes_20 zsender, uint64_t znonce);

fixed_bytes_20 zcreate2_address(fixed_bytes_20 zsender, u256 zsalt, fixed_bytes_32 zinit_hash);

u256 zlegacy_sig_chain_id(u256 zv);

void create_letbind_86(void);
void kill_letbind_86(void);


void create_letbind_87(void);
void kill_letbind_87(void);


fixed_bytes_32 ztx_signing_hash(enum zTxType zt, struct zStatelessInputSliceFields zcontent_src, u256 zv);

fixed_bytes_32 zauth_signing_hash(u256 zchain_id, fixed_bytes_20 zaddr, uint64_t znonce);

uint8_t ztx_signature_parity(uint64_t zchain_id, enum zTxSignatureScheme zscheme, u256 zv);

bool ztx_auth_valid(fixed_bytes_20 zsender, fixed_bytes_32 zh, uint8_t zparity, u256 zr, u256 zs);

void create_letbind_88(void);
void kill_letbind_88(void);


struct zStatelessInputSliceFields ztransaction_rlp_content(struct zRlpFieldRef zf);

struct zAccessListDecode zdecode_access_list_keys(struct zStatelessInputSliceFields zcursor, fixed_bytes_20 zaddr, struct zAccessListDecode ztail);

struct zAccessListDecode zdecode_access_list_entries(struct zStatelessInputSliceFields zcursor);

struct zAccessListRef zdecode_access_list(struct zRlpFieldRef zf);

void create_letbind_89(void);
void kill_letbind_89(void);


void create_letbind_90(void);
void kill_letbind_90(void);


struct zBlobHashesFields zdecode_blob_hashes(struct zRlpFieldRef zf, uint8_t zlimit);

struct zAuthorizzationListRefFields zdecode_auth_list(struct zRlpFieldRef zf);

struct zAuthorizzation zdecode_authorizzation(struct zRlpFieldRef ztuple);

void zprepare_authorizzation_entries(zz5listz8z5structz0zzAuthorizzzzationz9 *rop, struct zStatelessInputSliceFields zcursor, uint16_t zcount);

void zprepare_authorizzations(struct zPreparedAuthorizzationList *rop, struct zAuthorizzationListRefFields zauthorizzations);

struct zAuthorizzation zprepared_authorizzation_head(struct zPreparedAuthorizzationList zauthorizzations);

struct zStatelessInputSliceFields ztx_input_span(struct zRlpFieldRef zdata);

struct zStatelessInputSliceFields ztx_sig_span(struct zRlpFieldRef zfirst, struct zRlpFieldRef zsignature);

uint64_t zrlp_decode_gas(struct zRlpFieldRef zf);

struct zTransactionFields zdecode_legacy_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, fixed_bytes_20 zsender, struct zStatelessInputSliceFields zfields);

struct zTransactionFields zdecode_access_list_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, fixed_bytes_20 zsender, struct zStatelessInputSliceFields zfields);

struct zTransactionFields zdecode_fee_market_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, fixed_bytes_20 zsender, struct zStatelessInputSliceFields zfields);

struct zTransactionFields zdecode_blob_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, uint8_t zblob_limit, fixed_bytes_20 zsender, struct zStatelessInputSliceFields zfields);

struct zTransactionFields zdecode_set_code_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, fixed_bytes_20 zsender, struct zStatelessInputSliceFields zfields);

struct zTransactionFields zrlp_decode_tx(struct zStatelessInputSliceFields ztx, struct zStatelessInputSliceFields zpubkey, uint8_t zblob_limit);

struct zStackPointer zstack_reset(unit z3zE4747);

struct zStackPointer zoperand_stack_push_empty_frame(unit z3zE4745);

uint16_t zstack_top_height(struct zStackPointer ztop);

u256 zstack_slot_read(struct zStackPointer ztop, uint16_t zindex);

unit zstack_slot_write(struct zStackPointer ztop, uint16_t zindex, u256 zvalue);

struct zOutputSliceFields zfreezze_memory_output(struct zEvmMemorySliceFields zdata);

struct zOutputSliceFields zfreezze_input_output(struct zStatelessInputSliceFields zdata);

struct zOutputSliceFields zfreezze_calldata_output(struct zCalldataSlice zdata);

struct zOutputSliceFields zoutput_buffer_word(u256 zvalue);

struct zOutputSliceFields zoutput_buffer_words(u256 zfirst, u256 zsecond);

u256 zk_env(enum zEnvField zf);

fixed_bytes_20 zk_coinbase(unit z3zE4727);

u256 zblockhash_word_distance(u256 zcurrent, u256 znumber);

fixed_bytes_32 zk_blockhash(u256 znumber_word);

u256 zk_blobhash(u256 zindex_word);

fixed_bytes_20 zk_create_addr(fixed_bytes_20 za, uint64_t znonce);

fixed_bytes_20 zk_create2_addr(fixed_bytes_20 za, u256 zsalt, fixed_bytes_32 zinithash);

struct zAccount zk_aload(fixed_bytes_20 za);

u256 zstateless_storage_by_key(fixed_bytes_32 zroot, fixed_bytes_32 zslot_hash);

enum zPrecompileId zprecompile_id_for_address(fixed_bytes_20 zbytes);

struct zStorageKey zstorage_key(fixed_bytes_20 za, u256 zs);

bool zk_account_is_warm(fixed_bytes_20 za);

unit zk_account_mark_warm(fixed_bytes_20 za);

bool zk_slot_is_warm(fixed_bytes_20 za, u256 zs);

unit zk_slot_mark_warm(fixed_bytes_20 za, u256 zs);

unit zk_prewarm_slot(fixed_bytes_20 za, u256 zs);

struct zStorageValue zk_sload(fixed_bytes_20 za, u256 zs);

unit zk_sstore(fixed_bytes_20 za, u256 zs, struct zStorageValue zv);

u256 zk_tload(fixed_bytes_20 za, u256 zs);

unit zk_tstore(fixed_bytes_20 za, u256 zs, u256 zv);

unit zk_log_topics(struct zLogTopics ztopics);

unit zk_log_data(struct zLogData zdata);

unit zk_log(fixed_bytes_20 za, struct zLogTopics ztopics, struct zLogData zdata);

struct zLogSeriesRef zread_logs(unit z3zE4686);

struct zLogDataSliceFields zread_log_data(uint64_t zindex);

void create_letbind_91(void);
void kill_letbind_91(void);


uint64_t zbloom_bit_mask(uint8_t zbit_to_set);

fixed_bytes_256 zbloom_set_bit(fixed_bytes_256 zbloom, uint16_t zbit_to_set);

fixed_bytes_256 zbloom_add_entry_hash(fixed_bytes_256 zbloom, fixed_bytes_32 zh);

fixed_bytes_256 zbloom_add_log_at(fixed_bytes_256 zbloom, uint64_t zindex);

fixed_bytes_256 zbloom_add_logs(fixed_bytes_256 zbloom, struct zLogSeriesRef zlogs);

fixed_bytes_256 zlogs_bloom_for_logs(struct zLogSeriesRef zlogs);

unit zk_emit_transfer_log(fixed_bytes_20 zsrc, fixed_bytes_20 zdst, u256 zv);

struct zAccount zstateless_account_by_key(fixed_bytes_32 zroot, fixed_bytes_32 zaddress_hash);

bool zaccount_info_changed(struct zAccountInfo zc, struct zAccountInfo zo);

bool zaccount_info_empty(struct zAccountInfo zinfo);

bool zaccount_changed(struct zAccount zc, struct zAccount zo);

struct zAccount zaccount_set_info(struct zAccount zacc, struct zAccountInfo zinfo);

struct zAccount zaccount_clear_storage(struct zAccount zacc);

struct zAccount zaccount_delete(struct zAccount zacc);

struct zAccount zaccount_clear_preserving_balance(struct zAccount zacc);

unit zstore_account(fixed_bytes_20 za, struct zAccount zv);

unit zstore_account_info(fixed_bytes_20 za, struct zAccount zacc, struct zAccountInfo zinfo);

u256 zk_get_balance(fixed_bytes_20 za);

uint64_t zk_get_nonce(fixed_bytes_20 za);

bool zk_account_exists(fixed_bytes_20 za);

bool zk_account_is_empty(fixed_bytes_20 za);

bool zk_account_occupied(fixed_bytes_20 za);

unit zk_transfer(fixed_bytes_20 zsrc, fixed_bytes_20 zdst, u256 zv);

unit zk_bump_nonce(fixed_bytes_20 za);

unit zk_add_balance(fixed_bytes_20 za, u256 zv);

unit zk_sub_balance(fixed_bytes_20 za, u256 zv);

unit zk_clear_storage(fixed_bytes_20 za);

fixed_bytes_32 zk_code_key(fixed_bytes_20 za);

fixed_bytes_32 zk_get_codehash(fixed_bytes_20 za);

unit zk_deploy_code(fixed_bytes_20 za, struct zCodeRegionSliceFields zcode);

unit zk_set_delegation(fixed_bytes_20 za, fixed_bytes_20 ztarget);

unit zk_clear_code(fixed_bytes_20 za);

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zk_deleg_target(fixed_bytes_20 za);

uint32_t zk_get_code_sizze(fixed_bytes_20 za);

unit zk_code_copy(fixed_bytes_20 za, uint32_t zdst, u256 zoff, uint32_t zlen);

unit zk_selfdestruct(fixed_bytes_20 za);

bool zk_is_selfdestructed(fixed_bytes_20 za);

unit zk_mark_created(fixed_bytes_20 za);

bool zk_was_created(fixed_bytes_20 za);

unit zk_zzero_balance(fixed_bytes_20 za);

unit zk_journal_checkpoint(unit z3zE4620);

unit zk_set_header(struct zBlockHeader zh);

unit zk_set_tx(struct zTxEnvFields zenv);

unit zk_tx_reset(unit z3zE4614);

struct zTransactionMergeSemantics ztransaction_merge_semantics(uint8_t zfork);

bool zaccount_deleted_at_tx_end(struct zTransactionMergeSemantics zsemantics, struct zAccount zacc);

unit zk_tx_merge(unit z3zE4580);

unit zk_journal_revert(unit z3zE4579);

unit zk_journal_commit(unit z3zE4578);

__int128 zvalidated_refund_add(__int128 zleft, __int128 zright);

__int128 zrecord_refund(__int128 zrefund, __int128 zdelta);

uint32_t zframe_code_len(struct zCodeFields zframe_code);

bool zframe_jumpdest_valid(struct zCodeFields zframe_code, uint32_t zdest);

void create_letbind_92(void);
void kill_letbind_92(void);


uint64_t zconserved_gas_add(uint64_t zavailable, uint64_t zcredit);

__int128 zframe_state_gas_used(uint64_t zstate_gas_reservoir, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled);

void zexceptional_state(struct zExceptionalStateTransition *rop, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled, uint64_t zstate_gas_reservoir, enum zExceptionKind zk);

uint16_t zstack_height(struct zStackPointer ztop);

u256 zread_stack_word(struct zStackPointer zsp);

unit zwrite_stack_word(struct zStackPointer zsp, u256 zvalue);

unit zstack_set(struct zStackPointer ztop, uint16_t zn, u256 zw);

bool zis_running(struct zFrameStatus zframe_status);

struct zOutputSliceFields zreturndata_clear(unit z3zE4571);

uint32_t zreturndata_sizze(struct zOutputSliceFields zreturndata);

unit zreturndata_copy(struct zOutputSliceFields zreturndata, uint32_t zdst, uint32_t zoff, uint32_t zlen);

unit zreturndata_copy_prefix(struct zOutputSliceFields zreturndata, uint32_t zdst, uint32_t zwant);

uint32_t zreturndata_remaining(uint32_t zavailable, uint32_t zoffset);

uint32_t zmemory_high_water(uint32_t zheight);

void create_letbind_93(void);
void kill_letbind_93(void);


void create_letbind_94(void);
void kill_letbind_94(void);


uint32_t zmemory_absolute(uint32_t zbase, uint32_t zrelative);

uint32_t zmemory_parent_base(uint32_t zchild_base, uint32_t zparent_height);

uint32_t zexpand_memory(uint32_t zbase, uint32_t zheight, uint32_t zrequested_height);

struct zEvmMemorySliceFields zactive_memory_slice(uint32_t zbase, uint32_t zmem, uint32_t zoff, uint32_t zlen);

struct zCodeRegionSliceFields zmemory_code_slice(uint32_t zbase, uint32_t zmem, uint32_t zoff, uint32_t zlen);

unit zmem_set_byte(uint32_t zbase, uint32_t zoff, uint64_t zv);

u256 zmem_load(uint32_t zbase, uint32_t zoff);

unit zmem_store(uint32_t zbase, uint32_t zoff, u256 zw);

unit zmem_store_byte(uint32_t zbase, uint32_t zoff, u256 zw);

unit zmem_mcopy(uint32_t zbase, uint32_t zdst, uint32_t zsrc, uint32_t zlen);

u256 zmem_keccak(uint32_t zbase, uint32_t zmem, struct zMemoryRangeFields zrange);

void create_letbind_95(void);
void kill_letbind_95(void);


u256 zprotocol_word(u256 zvalue);

u256 zfake_exponential_word(struct zBlobScheduleFields zschedule, uint32_t znumerator);

u256 zblob_base_fee(uint8_t zfork, struct zBlobScheduleFields zschedule, uint32_t zlimit, uint32_t zexcess_blob_gas);

uint32_t zblock_blob_gas_add(uint8_t zmaximum_count, uint32_t zaccumulated, uint32_t ztransaction);

uint32_t znext_excess_blob_gas(struct zProtocolProfileFields zprofile, uint32_t zparent_excess_blob_gas, uint32_t zparent_blob_gas_used, u256 zparent_base_fee_per_gas);

void create_letbind_96(void);
void kill_letbind_96(void);


void create_letbind_97(void);
void kill_letbind_97(void);


void create_letbind_98(void);
void kill_letbind_98(void);


void create_letbind_99(void);
void kill_letbind_99(void);


void create_letbind_100(void);
void kill_letbind_100(void);


void create_letbind_101(void);
void kill_letbind_101(void);


void create_letbind_102(void);
void kill_letbind_102(void);


void create_letbind_103(void);
void kill_letbind_103(void);


void create_letbind_104(void);
void kill_letbind_104(void);


void create_letbind_105(void);
void kill_letbind_105(void);


void create_letbind_106(void);
void kill_letbind_106(void);


void create_letbind_107(void);
void kill_letbind_107(void);


void create_letbind_108(void);
void kill_letbind_108(void);


void create_letbind_109(void);
void kill_letbind_109(void);


void create_letbind_110(void);
void kill_letbind_110(void);


void create_letbind_111(void);
void kill_letbind_111(void);


void create_letbind_112(void);
void kill_letbind_112(void);


void create_letbind_113(void);
void kill_letbind_113(void);


void create_letbind_114(void);
void kill_letbind_114(void);


void create_letbind_115(void);
void kill_letbind_115(void);


void create_letbind_116(void);
void kill_letbind_116(void);


void create_letbind_117(void);
void kill_letbind_117(void);


void create_letbind_118(void);
void kill_letbind_118(void);


void create_letbind_119(void);
void kill_letbind_119(void);


void create_letbind_120(void);
void kill_letbind_120(void);


void create_letbind_121(void);
void kill_letbind_121(void);


void create_letbind_122(void);
void kill_letbind_122(void);


void create_letbind_123(void);
void kill_letbind_123(void);


void create_letbind_124(void);
void kill_letbind_124(void);


void create_letbind_125(void);
void kill_letbind_125(void);


void create_letbind_126(void);
void kill_letbind_126(void);


void create_letbind_127(void);
void kill_letbind_127(void);


void create_letbind_128(void);
void kill_letbind_128(void);


void create_letbind_129(void);
void kill_letbind_129(void);


void create_letbind_130(void);
void kill_letbind_130(void);


void create_letbind_131(void);
void kill_letbind_131(void);


void create_letbind_132(void);
void kill_letbind_132(void);


void create_letbind_133(void);
void kill_letbind_133(void);


void create_letbind_134(void);
void kill_letbind_134(void);


void create_letbind_135(void);
void kill_letbind_135(void);


void create_letbind_136(void);
void kill_letbind_136(void);


void create_letbind_137(void);
void kill_letbind_137(void);


void create_letbind_138(void);
void kill_letbind_138(void);


uint16_t zsstore_clear_refund(unit z3zE4551);

uint32_t zstate_gas_spill_room(uint32_t zleft);

struct ztuple_z8z5u64zCz0z5u64zCz0z5u32z9 zcredit_state_gas_refund(uint64_t zg, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled, uint32_t zamount);

struct ztuple_z8z5u64zCz0z5u32z9 zreturn_child_state_gas(uint64_t zparent_remaining, uint32_t zparent_spilled, uint64_t zchild_remaining, uint32_t zchild_spilled);

uint64_t zrefund_gas(uint64_t zg, uint64_t zamount);

uint64_t zgas_sub(uint64_t zleft, uint64_t zright);

u256 zmemory_word_count_word(u256 zbyte_len);

struct zMemoryAccessFields zmemory_access(u256 zstart, u256 zsizze);

uint16_t zaccount_cost(bool zwarm);

uint16_t zexternal_code_read_cost(unit z3zE4531);

uint16_t zsload_cost(bool zwarm);

uint16_t zcall_value_cost(unit z3zE4530);

uint16_t zcreate_access_cost(unit z3zE4529);

struct zGasCharge zcode_deployment_execution_cost(uint32_t zbyte_len, uint64_t zavailable);

uint64_t zcode_deployment_state_cost(uint32_t zbyte_len);

u256 zpc_word(struct zCalldataSlice zinput, uint32_t zstart, uint8_t zbyte_count);

u256 zpc_word_after_declared_field(struct zCalldataSlice zinput, uint32_t zprefix, u256 zdeclared_length, uint8_t zbyte_count);

uint32_t zpc_blake2_rounds(struct zCalldataSlice zinput);

struct zGasCharge zmodexp_gas(struct zCalldataSlice zinput, uint64_t zavailable);

void create_letbind_139(void);
void kill_letbind_139(void);


void create_letbind_140(void);
void kill_letbind_140(void);


struct zGasCharge zprecompile_gas(enum zPrecompileId znum, struct zCalldataSlice zinput, uint64_t zavailable);

uint16_t zamsterdam_storage_access_cost(bool zcold);

uint64_t zsstore_sentry_cost(bool zcold);

struct zSstoreCosts zlegacy_sstore_costs(u256 zoriginal, u256 zcurrent, u256 znew_value, bool zcold);

struct zSstoreCosts zamsterdam_sstore_costs(u256 zoriginal, u256 zcurrent, u256 znew_value, bool zcold);

struct zSstoreCosts zsstore_costs(u256 zoriginal, u256 zcurrent, u256 znew_value, bool zcold);

struct zGasCharge zword_scaled_gas_cost(uint16_t zper_unit, u256 zunits, uint64_t zavailable);

struct zGasCharge zmemory_word_gas_cost(uint16_t zbase, uint16_t zper_word, u256 zsizze, uint64_t zavailable);

struct zGasCharge zkeccak_gas_cost(u256 zsizze, uint64_t zavailable);

struct zGasCharge zcopy_gas_cost(u256 zsizze, uint64_t zavailable);

struct zGasCharge zlog_gas_cost(uint8_t znum_topics, u256 zsizze, uint64_t zavailable);

uint64_t zexp_gas(u256 zexponent);

uint32_t ztransaction_initcode_gas(uint32_t zbyte_len);

uint64_t zcall_gas_cap_word(uint64_t zavailable, u256 zrequested);

void create_letbind_141(void);
void kill_letbind_141(void);


void create_letbind_142(void);
void kill_letbind_142(void);


void create_letbind_143(void);
void kill_letbind_143(void);


void create_letbind_144(void);
void kill_letbind_144(void);


void create_letbind_145(void);
void kill_letbind_145(void);


void create_letbind_146(void);
void kill_letbind_146(void);


void create_letbind_147(void);
void kill_letbind_147(void);


void create_letbind_148(void);
void kill_letbind_148(void);


void create_letbind_149(void);
void kill_letbind_149(void);


void create_letbind_150(void);
void kill_letbind_150(void);


void create_letbind_151(void);
void kill_letbind_151(void);


void create_letbind_152(void);
void kill_letbind_152(void);


void create_letbind_153(void);
void kill_letbind_153(void);


void create_letbind_154(void);
void kill_letbind_154(void);


void create_letbind_155(void);
void kill_letbind_155(void);


void create_letbind_156(void);
void kill_letbind_156(void);


void create_letbind_157(void);
void kill_letbind_157(void);


void create_letbind_158(void);
void kill_letbind_158(void);


void create_letbind_159(void);
void kill_letbind_159(void);


void create_letbind_160(void);
void kill_letbind_160(void);


void create_letbind_161(void);
void kill_letbind_161(void);


void create_letbind_162(void);
void kill_letbind_162(void);


void create_letbind_163(void);
void kill_letbind_163(void);


void create_letbind_164(void);
void kill_letbind_164(void);


void create_letbind_165(void);
void kill_letbind_165(void);


void create_letbind_166(void);
void kill_letbind_166(void);


void create_letbind_167(void);
void kill_letbind_167(void);


void create_letbind_168(void);
void kill_letbind_168(void);


void create_letbind_169(void);
void kill_letbind_169(void);


void create_letbind_170(void);
void kill_letbind_170(void);


void create_letbind_171(void);
void kill_letbind_171(void);


void create_letbind_172(void);
void kill_letbind_172(void);


void create_letbind_173(void);
void kill_letbind_173(void);


void create_letbind_174(void);
void kill_letbind_174(void);


void create_letbind_175(void);
void kill_letbind_175(void);


void create_letbind_176(void);
void kill_letbind_176(void);


void create_letbind_177(void);
void kill_letbind_177(void);


void create_letbind_178(void);
void kill_letbind_178(void);


void create_letbind_179(void);
void kill_letbind_179(void);


void create_letbind_180(void);
void kill_letbind_180(void);


void create_letbind_181(void);
void kill_letbind_181(void);


void create_letbind_182(void);
void kill_letbind_182(void);


void create_letbind_183(void);
void kill_letbind_183(void);


void create_letbind_184(void);
void kill_letbind_184(void);


void create_letbind_185(void);
void kill_letbind_185(void);


void create_letbind_186(void);
void kill_letbind_186(void);


struct zPrecompileResult zprecompile_success(struct zOutputSliceFields zoutput);

struct zPrecompileResult zprecompile_failure(unit z3zE4440);

struct zPrecompileResult zcopied_result(struct zCalldataSlice zdata);

struct zPrecompileResult zboolean_result(bool zvalue);

bool zprecompile_active_at_fork(enum zPrecompileId zn);

enum zPrecompileId zprecompile_id_if_active(enum zPrecompileId zcandidate);

struct zPrecompileResult zrun_ecrecover(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_sha256(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_ripemd160(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_modexp(struct zCalldataSlice zinput);

struct zPrecompileResult zpairing_result(uint8_t zresult);

struct zPrecompileResult zrun_blake2f(struct zCalldataSlice zinput);

bool zkzzg_versioned_hash_matches(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_kzzg_point_evaluation(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_g1_add(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_g1_msm(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_g2_add(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_g2_msm(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_pairing(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_map_fp_to_g1(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_bls_map_fp2_to_g2(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_p256_verify(struct zCalldataSlice zinput);

struct zPrecompileResult zrun_precompile_slice(enum zPrecompileId znum, struct zCalldataSlice zinput);

uint8_t zdecode_single_stack_index(uint64_t zimmediate);

struct ztuple_z8z5u16zCz0z5u16z9 zdecode_exchange_stack_indices(uint64_t zimmediate);

void zopcode_frame_status(struct zFrameStatus *rop, struct zOpcodeOutcome zresult);

void zguard_static(struct ztuple_z8z5u64zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zg, bool zis_static);

void zdo_jump(struct ztuple_z8z5u32zCz0z5u64zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zpc_in, uint64_t zg, struct zCodeFields zframe_code, u256 zdestination_value);

void zguard_stack(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint16_t zinputs, uint16_t zoutputs);

void zpop_log_topics(struct ztuple_z8z5unionz0zzLogTopicszCz0z5structz0zzStackPointerz9 *rop, uint8_t zcount, struct zStackPointer zsp_in);

void zexecute_add(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_mul(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_sub(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_div(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_sdiv(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_mod(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_smod(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_addmod(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_mulmod(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_exp(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_signextend(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_lt(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_gt(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_slt(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_sgt(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_eq(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_iszzero(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_and(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_or(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_xor(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_not(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_byte(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_shl(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_shr(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_sar(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_clzz(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_keccak256(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_address(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_address, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_origin(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_caller(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_caller, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_callvalue(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, u256 zcarried_value, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_gasprice(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_calldatasizze(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCalldataSlice zcarried_calldata, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_calldataload(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCalldataSlice zcarried_calldata, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_calldatacopy(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCalldataSlice zcarried_calldata, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_codesizze(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCodeFields zcarried_code, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_codecopy(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCodeFields zcarried_code, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_balance(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_selfbalance(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_address, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_extcodesizze(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_extcodecopy(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_extcodehash(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_returndatasizze(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zOutputSliceFields zcarried_returndata, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_returndatacopy(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zOutputSliceFields zcarried_returndata, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_blockhash(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_coinbase(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_timestamp(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_number(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_slotnum(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_prevrandao(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_gaslimit(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_chainid(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_basefee(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_blobbasefee(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, u256 zblob_fee, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_blobhash(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_pop(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_mload(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_mstore(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_mstore8(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_msizze(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_mcopy(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

struct zAccountExecutionContext zaccount_execution_context(fixed_bytes_20 zaddress);

struct zAccountExecutionContext zrefresh_account_execution_context(struct zAccountExecutionContext zcontext, fixed_bytes_20 zprevious_address, fixed_bytes_20 znext_address);

void zexecute_sload(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zAccountExecutionContext zcontext, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_sstore(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zAccountExecutionContext zcontext, uint8_t zfork, bool zcarried_is_static, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_sp);

void zexecute_tload(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_address, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_tstore(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_address, bool zcarried_is_static, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_jump(struct ztuple_z8z5u32zCz0z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCodeFields zcarried_code, uint32_t zcarried_pc, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_jumpi(struct ztuple_z8z5u32zCz0z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, struct zCodeFields zcarried_code, uint32_t zcarried_pc, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_pc(struct ztuple_z8z5u32zCz0z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint32_t zcarried_pc, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_gas(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp);

void zexecute_jumpdest(struct ztuple_z8z5u64zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas);

void zexecute_push(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint8_t zn, u256 zv);

void zexecute_dupn(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint64_t zimmediate);

void zexecute_swapn(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint64_t zimmediate);

void zexecute_exchange(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint64_t zimmediate);

void zexecute_stop(struct zFrameStatus *rop, unit z3zE3293);

void zexecute_return(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 *rop, uint32_t zmemory_base, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_revert(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzFrameStatusz9 *rop, uint64_t zcarried_state_gas_reservoir, uint32_t zmemory_base, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_invalid(struct ztuple_z8z5u64zCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas);

void zexecute_selfdestruct(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5unionz0zzFrameStatusz9 *rop, fixed_bytes_20 zcarried_address, uint8_t zfork, bool zcarried_is_static, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_sp);

bool zopcode_available(uint8_t zopcode, uint8_t zfork);

void zresume_frame(struct zFrameTransition *rop, struct zFrameContinuation zcontinuation, struct zOutputSliceFields zoutput, uint32_t zchild_memory_base, uint64_t zchild_gas, uint64_t zchild_state_gas, uint32_t zchild_state_spill, __int128 zchild_refund, struct zFrameStatus zchild_status, uint64_t zchild_state_gas_reservoir);

struct zOutputSliceFields zframe_output(struct zFrameStatus zframe_status);

bool zframe_succeeded(struct zFrameStatus zframe_status);

struct zCodeFields zexecutable_code(fixed_bytes_20 ztarget, bool zdele, fixed_bytes_20 zdtgt);

struct zCallSemantics zcall_semantics(enum zCallKind zkind);

uint16_t zcall_stack_inputs(enum zCallKind zkind);

struct zCreateSemantics zcreate_semantics(enum zCreateKind zkind);

uint16_t zcreate_stack_inputs(enum zCreateKind zkind);

void zresume_call(struct zFrameTransition *rop, struct zCallContinuation zcontinuation, struct zOutputSliceFields zoutput, uint32_t zchild_memory_base, uint64_t zchild_gas, uint64_t zchild_state_gas, uint32_t zchild_state_spill, __int128 zchild_refund, struct zFrameStatus zchild_status);

void zresume_create(struct zFrameTransition *rop, struct zCreateContinuation zcontinuation, struct zOutputSliceFields zoutput, uint32_t zchild_memory_base, uint64_t zchild_gas, uint64_t zchild_state_gas, uint32_t zchild_state_spill, __int128 zchild_refund, struct zFrameStatus zchild_status, uint64_t zchild_state_gas_reservoir);

void create_letbind_187(void);
void kill_letbind_187(void);


void create_letbind_188(void);
void kill_letbind_188(void);


void create_letbind_189(void);
void kill_letbind_189(void);


void create_letbind_190(void);
void kill_letbind_190(void);


void create_letbind_191(void);
void kill_letbind_191(void);


void create_letbind_192(void);
void kill_letbind_192(void);


void create_letbind_193(void);
void kill_letbind_193(void);


void create_letbind_194(void);
void kill_letbind_194(void);


void create_letbind_195(void);
void kill_letbind_195(void);


void create_letbind_196(void);
void kill_letbind_196(void);


void create_letbind_197(void);
void kill_letbind_197(void);


void create_letbind_198(void);
void kill_letbind_198(void);


void create_letbind_199(void);
void kill_letbind_199(void);


void create_letbind_200(void);
void kill_letbind_200(void);


void create_letbind_201(void);
void kill_letbind_201(void);


void create_letbind_202(void);
void kill_letbind_202(void);


void create_letbind_203(void);
void kill_letbind_203(void);


void create_letbind_204(void);
void kill_letbind_204(void);


void create_letbind_205(void);
void kill_letbind_205(void);


struct zCodeRegionSliceFields ztransaction_initcode_slice(struct zStatelessInputSliceFields zinput);

uint64_t zcalldata_cost(struct zStatelessInputSliceFields zinput);

uint64_t zlegacy_intrinsic_gas(struct zTransactionFields ztx);

uint64_t zlegacy_calldata_floor(struct zStatelessInputSliceFields zinput);

uint16_t zamsterdam_recipient_execution_cost(struct zTransactionFields ztx);

struct zIntrinsicGasCost zintrinsic_gas(struct zTransactionFields ztx);

u320 ztransaction_blob_fee(u256 zblob_price, uint32_t zblob_gas);

void ztransaction_upfront_cost(sail_int *rop, u256 zmax_fee, uint64_t zgas_limit, u256 zvalue, u256 zmax_blob_fee, uint32_t zblob_gas);

struct zTransactionCosts ztransaction_costs(struct zProtocolProfileFields zprofile, struct zTransactionFields ztx, uint64_t zgas_limit, uint32_t zexcess_blob_gas);

struct zTxFrameGasSnapshotFields ztx_frame_gas_snapshot(struct zTransactionInitialGasFields zinitial, uint64_t zexecution, uint64_t zstate, __int128 zstate_delta);

struct zTransactionGasAllowanceFields ztransaction_gas_allowance_fields(uint64_t zvalue, uint64_t z_total_limit, uint64_t zregular_limit);

uint16_t zprocess_auth(struct zAuthorizzation zau);

uint64_t zauthorizzation_refund_add(uint16_t zitem, uint64_t zaccumulated);

uint64_t zprocess_auth_cursor(struct zPreparedAuthorizzationList zauthorizzations, uint16_t zcount);

uint64_t zprocess_auth_list(struct zPreparedAuthorizzationList zauthorizzations);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 zprocess_amsterdam_auth(struct zAuthorizzation zau, fixed_bytes_20 zsender, fixed_bytes_20 zcurrent_target, bool ztransfers_value, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 zprocess_amsterdam_auth_cursor(struct zPreparedAuthorizzationList zauthorizzations, uint16_t zcount, fixed_bytes_20 zsender, fixed_bytes_20 zcurrent_target, bool ztransfers_value, uint64_t zgas, uint64_t zstate_gas, uint32_t zstate_spill);

unit zwarm_access_list_keys(struct zStatelessInputSliceFields zcursor, fixed_bytes_20 zaddr);

unit zwarm_access_list(struct zStatelessInputSliceFields zcursor);

unit zprewarm(struct zTransactionFields ztx);

struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zeff_gas_price_for(u256 zbase_fee, u256 zmax_fee, u256 zmax_priority_fee);

struct zTxValidityFields zcheck_transaction_validity(struct zTransactionFields ztx, struct zTransactionGasAllowanceFields zallowance);

struct zTxUpfrontResult zapply_transaction_upfront_effects(struct zTransactionFields ztx, struct zTxValidityFields zv, struct zPreparedAuthorizzationList zauthorizzations);

struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5u32z9 zenter_transaction_frame(struct zTxValidityFields zv);

void zrun_create_transaction_frame(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 *rop, struct zTransactionFields ztx, fixed_bytes_20 zsender, uint64_t znonce_before, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_stack, uint32_t zcarried_memory_base, uint32_t zcarried_memory_height, struct zCodeFields zcarried_code, struct zCalldataSlice zcarried_calldata, uint64_t zstate_gas_reservoir);

void zrun_call_transaction_frame(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 *rop, struct zTransactionFields ztx, fixed_bytes_20 zsender, bool zdelegated, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_stack, uint32_t zcarried_memory_base, uint32_t zcarried_memory_height, fixed_bytes_20 zcarried_code_address, struct zCodeFields zcarried_code, struct zCalldataSlice zcarried_calldata, uint64_t zstate_gas_reservoir);

struct zTxFrameResultFields zrun_legacy_transaction_frame(struct zTransactionFields ztx, struct zTxValidityFields zv);

struct zTxFrameResultFields zrun_amsterdam_transaction_frame(struct zTransactionFields ztx, struct zTxValidityFields zv, struct zTxUpfrontResult zupfront, struct zPreparedAuthorizzationList zauthorizzations);

struct zTxFrameResultFields zrun_transaction_frame(struct zTransactionFields ztx, struct zTxValidityFields zv, struct zTxUpfrontResult zupfront, struct zPreparedAuthorizzationList zauthorizzations);

uint64_t zremaining_gas_after_refund(uint64_t z_limit, __int128 ztotal, uint64_t zremaining, uint64_t zcap);

struct zReceiptFields zsettle_transaction(struct zTransactionFields ztx, struct zTxValidityFields zv, uint64_t zauthorizzation_refund, struct zTxFrameResultFields zfr);

struct zReceiptFields zprocess_transaction(struct zTransactionFields ztx, struct zTransactionGasAllowanceFields zallowance);

void create_letbind_206(void);
void kill_letbind_206(void);


uint8_t zpath_len(struct zTriePath zpath);

struct zTriePath zpath_new(fixed_bytes_32 zdata, uint8_t zlen);

uint64_t zpath_nibble(struct zTriePath zpath, uint8_t zi);

struct zTriePath zpath_append_nibble(struct zTriePath zpath, uint64_t zvalue);

struct zTriePath zpath_append_byte(struct zTriePath zpath, uint64_t zvalue);

struct zTriePath zpath_single(uint64_t zn);

struct zTriePath zpath_concat(struct zTriePath za, struct zTriePath zb);

struct zTriePath zpath_drop(struct zTriePath zpath, uint8_t zn);

bool zpath_eq(struct zTriePath za, struct zTriePath zb);

bool zpath_prefix_of(struct zTriePath zprefix, struct zTriePath zpath);

uint8_t zcommon_prefix_length(struct zTriePath za, struct zTriePath zb);

uint8_t zhex_prefix_encoded_length(struct zTriePath zpath);

uint64_t zhex_prefix_first_byte(struct zTriePath zpath, bool zis_leaf);

struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zhex_prefix_decode_ref(struct zRlpFieldRef zf);

struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zscratch_hex_prefix_decode_ref(struct zScratchRlpFieldRef zf);

void create_letbind_207(void);
void kill_letbind_207(void);


struct zInlineNode zinline_node_from_scratch_slice(struct zScratchSliceFields zbytes);

struct zScratchSliceFields zinline_node_slice(struct zInlineNode znode);

fixed_bytes_32 zinline_node_hash(struct zInlineNode znode);

uint8_t znode_ref_sizze(struct zNodeRef zr);

unit zrlp_write_node_ref(struct zNodeRef zr);

uint8_t zrlp_hex_prefix_sizze(struct zTriePath zpath, bool zis_leaf);

unit zrlp_write_hex_prefix(struct zTriePath zpath, bool zis_leaf);

void zchild_ref(struct zNodeRef *rop, struct zScratchSliceFields zencoded);

uint64_t zbranch_mask_for(uint64_t zindex);

bool zbranch_mask_has(uint64_t zmask, uint64_t zindex);

uint64_t zbranch_mask_set(uint64_t zmask, uint64_t zindex);

void zinput_leaf_child_ref(struct zNodeRef *rop, struct zTriePath zkey, struct zStatelessInputSliceFields zvalue);

void zscratch_leaf_child_ref(struct zNodeRef *rop, struct zTriePath zkey, struct zScratchSliceFields zvalue);

void zleaf_child_ref(struct zNodeRef *rop, struct zTriePath zkey, struct zTrieLeafValue zvalue);

void zextension_child_ref(struct zNodeRef *rop, struct zTriePath zkey, struct zNodeRef zchildref);

void zbranch_child_ref(struct zNodeRef *rop, uint64_t zmask, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren);

fixed_bytes_32 ztrie_ref_to_root(struct zNodeRef zr);

void zinput_node_to_ref(struct zNodeRef *rop, struct zStatelessInputSliceFields znode);

struct zStatelessInputSliceFields znode_db_lookup(fixed_bytes_32 zh);

void zbranch_refs_get(struct zNodeRef *rop, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren, uint64_t zindex);

void zinput_field_to_ref(struct zNodeRef *rop, struct zRlpFieldRef zf);

void zscratch_field_to_ref(struct zNodeRef *rop, struct zScratchRlpFieldRef zf);

void zdecode_input_trie_node(struct zInputTrieNode *rop, struct zStatelessInputSliceFields znode);

void zdecode_scratch_trie_node(struct zScratchTrieNode *rop, struct zScratchSliceFields znode);

struct zStatelessInputSliceFields zresolve_witness_ref(struct zNodeRef zr);

void zmerge_ext_node(struct zNodeRef *rop, struct zTriePath zprefix, struct zStatelessInputSliceFields zchildnode);

void zmerge_ext_ref(struct zNodeRef *rop, struct zTriePath zprefix, struct zNodeRef zchildref);

void create_letbind_208(void);
void kill_letbind_208(void);


void ztrie_update_source_next(struct zTrieUpdateFetch *rop, struct zTrieUpdateSource zsource);

void ztrie_updates_begin(struct zTrieUpdateCursor *rop, struct zTrieUpdateSource zsource);

bool zupdates_empty(struct zTrieUpdateCursor zupdates);

void ztrie_updates_pop(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates);

void ztrie_updates_rebase(struct zTrieUpdateCursor *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix);

void ztrie_updates_descend(struct zTrieUpdateCursor *rop, struct zTrieUpdateCursor zupdates);

void ztrie_empty_subtree(struct zTrieItem *rop, unit z3zE1325);

void ztrie_leaf(struct zTrieItem *rop, struct zTriePath zpath, struct zTrieLeafValue zvalue);

void ztrie_input_leaf(struct zTrieItem *rop, struct zTriePath zpath, struct zStatelessInputSliceFields zvalue);

void ztrie_scratch_leaf(struct zTrieItem *rop, struct zTriePath zpath, struct zScratchSliceFields zvalue);

void ztrie_branch(struct zTrieItem *rop, struct zTriePath zpath, struct zNodeRef zchildref);

void ztrie_subtree(struct zTrieItem *rop, struct zTriePath zpath, struct zNodeRef zchildref);

void ztrie_children_empty(struct zTrieChildren *rop, unit z3zE1311);

void ztrie_children_add(struct zTrieChildren *rop, struct zTrieChildren zchildren, struct zTriePath zprefix, uint64_t zindex, struct zTrieItem zchild);

void ztrie_children_finish(struct zTrieItem *rop, struct zTriePath zprefix, struct zTrieChildren zchildren);

fixed_bytes_32 ztrie_subtree_root(struct zTrieItem zsubtree);

uint8_t zrlp_index_encoded_width(uint32_t zvalue);

struct zTriePath ztrie_index_key(uint32_t zindex);

uint32_t zrlp_index_at_position(struct zRlpIndexCursor zcursor);

struct zRlpIndexCursor zrlp_index_cursor(uint32_t zcount);

bool zrlp_index_cursor_empty(struct zRlpIndexCursor zcursor);

struct zRlpIndexItem zrlp_index_cursor_peek(struct zRlpIndexCursor zcursor);

struct zRlpIndexCursor zrlp_index_cursor_advance(struct zRlpIndexCursor zcursor);

struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 zrlp_index_cursor_pop(struct zRlpIndexCursor zcursor);

bool zupdate_under_current_prefix(struct zTrieUpdateCursor zupdates);

uint64_t zupdate_child_nibble(struct zTrieUpdateCursor zupdates);

uint8_t zupdate_child_ranges_remaining(struct zTrieUpdateCursor zupdates);

uint8_t zoverlay_child_ranges_remaining(struct zTrieUpdateCursor zupdates, bool zexisting_pending, uint64_t zexisting_nibble);

void zupdates_subtree(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zwitness_subtree(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zStatelessInputSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, uint8_t zcursor);

struct zTrieRootResult ztrie_root_cursor(fixed_bytes_32 zbase_root, struct zTrieUpdateCursor zupdates);

struct zTrieRootResult ztrie_root(fixed_bytes_32 zbase_root, struct zTrieUpdateSource zsource);

struct zStatelessInputSliceFields ztrie_lookup(fixed_bytes_32 zroot, struct zTriePath zkey);

struct zAccountInfo zdecode_state_account(struct zStatelessInputSliceFields zvalue);

struct zScratchSliceFields zencode_storage_value(u256 zvalue);

struct zScratchSliceFields zencode_state_account(struct zAccountInfo zinfo, fixed_bytes_32 zstorage_root);

bool zstorage_value_changed(struct zStorageValue zvalue);

bool zaccount_value_changed(struct zAcctValue zvalue);

void zstorage_update(struct zTrieUpdate *rop, struct zStorageTrieEntry ztrie_entry);

void zaccount_update(struct zTrieUpdate *rop, struct zAcctTrieEntry ztrie_entry, fixed_bytes_32 zstorage_root);

void znext_storage_trie_update(struct zTrieUpdateFetch *rop, fixed_bytes_20 zaddr);

void zaccount_trie_update(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *rop, struct zAcctTrieEntry ztrie_entry);

void znext_changed_account_trie_update(struct zTrieUpdateFetch *rop, unit z3zE1079);

fixed_bytes_32 zcompute_state_root(unit z3zE1074);

unit zvalidation_debug_capture_block_gas(uint64_t z_actual, uint64_t z_expected, uint64_t z_execution, uint64_t z_state);

void create_letbind_209(void);
void kill_letbind_209(void);


void create_letbind_210(void);
void kill_letbind_210(void);


void create_letbind_211(void);
void kill_letbind_211(void);


void create_letbind_212(void);
void kill_letbind_212(void);


void create_letbind_213(void);
void kill_letbind_213(void);


void create_letbind_214(void);
void kill_letbind_214(void);


void create_letbind_215(void);
void kill_letbind_215(void);


void create_letbind_216(void);
void kill_letbind_216(void);


void create_letbind_217(void);
void kill_letbind_217(void);


void create_letbind_218(void);
void kill_letbind_218(void);


void create_letbind_219(void);
void kill_letbind_219(void);


void create_letbind_220(void);
void kill_letbind_220(void);


void create_letbind_221(void);
void kill_letbind_221(void);


void create_letbind_222(void);
void kill_letbind_222(void);


void create_letbind_223(void);
void kill_letbind_223(void);


void create_letbind_224(void);
void kill_letbind_224(void);


void create_letbind_225(void);
void kill_letbind_225(void);


void create_letbind_226(void);
void kill_letbind_226(void);


void create_letbind_227(void);
void kill_letbind_227(void);


void create_letbind_228(void);
void kill_letbind_228(void);


void create_letbind_229(void);
void kill_letbind_229(void);


void create_letbind_230(void);
void kill_letbind_230(void);


void create_letbind_231(void);
void kill_letbind_231(void);


void create_letbind_232(void);
void kill_letbind_232(void);


void create_letbind_233(void);
void kill_letbind_233(void);


void create_letbind_234(void);
void kill_letbind_234(void);


void create_letbind_235(void);
void kill_letbind_235(void);


void create_letbind_236(void);
void kill_letbind_236(void);


void create_letbind_237(void);
void kill_letbind_237(void);


void create_letbind_238(void);
void kill_letbind_238(void);


void create_letbind_239(void);
void kill_letbind_239(void);


void create_letbind_240(void);
void kill_letbind_240(void);


void create_letbind_241(void);
void kill_letbind_241(void);


void create_letbind_242(void);
void kill_letbind_242(void);


void create_letbind_243(void);
void kill_letbind_243(void);


void create_letbind_244(void);
void kill_letbind_244(void);


void create_letbind_245(void);
void kill_letbind_245(void);


void create_letbind_246(void);
void kill_letbind_246(void);


void create_letbind_247(void);
void kill_letbind_247(void);


void create_letbind_248(void);
void kill_letbind_248(void);


void create_letbind_249(void);
void kill_letbind_249(void);


void create_letbind_250(void);
void kill_letbind_250(void);


void create_letbind_251(void);
void kill_letbind_251(void);


void create_letbind_252(void);
void kill_letbind_252(void);


void create_letbind_253(void);
void kill_letbind_253(void);


void create_letbind_254(void);
void kill_letbind_254(void);


void create_letbind_255(void);
void kill_letbind_255(void);


void create_letbind_256(void);
void kill_letbind_256(void);


void create_letbind_257(void);
void kill_letbind_257(void);


void create_letbind_258(void);
void kill_letbind_258(void);


void create_letbind_259(void);
void kill_letbind_259(void);


void create_letbind_260(void);
void kill_letbind_260(void);


void create_letbind_261(void);
void kill_letbind_261(void);


void create_letbind_262(void);
void kill_letbind_262(void);


void create_letbind_263(void);
void kill_letbind_263(void);


void create_letbind_264(void);
void kill_letbind_264(void);


void create_letbind_265(void);
void kill_letbind_265(void);


void create_letbind_266(void);
void kill_letbind_266(void);


void create_letbind_267(void);
void kill_letbind_267(void);


void create_letbind_268(void);
void kill_letbind_268(void);


void create_letbind_269(void);
void kill_letbind_269(void);


void create_letbind_270(void);
void kill_letbind_270(void);


void create_letbind_271(void);
void kill_letbind_271(void);


void create_letbind_272(void);
void kill_letbind_272(void);


void create_letbind_273(void);
void kill_letbind_273(void);


void create_letbind_274(void);
void kill_letbind_274(void);


void create_letbind_275(void);
void kill_letbind_275(void);


uint32_t zsszz_offset_table_position(uint32_t zindex);

struct zBoundedSszzListCursor zsszz_list_cursor(struct zBoundedSszzListRef zitems);

bool zsszz_list_cursor_empty(struct zBoundedSszzListCursor zcursor);

struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 zsszz_list_pop(struct zBoundedSszzListCursor zcursor);

struct zStatelessInputSliceFields zsszz_list_at(struct zBoundedSszzListRef zitems, sail_int zindex);

struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 zsszz_take(struct zSszzContainerCursor zcursor, uint32_t zstop);

struct zStatelessInputSliceFields zsszz_finish(struct zSszzContainerCursor zcursor);

struct zBoundedSszzListRef zsszz_bounded_variable_list_ref(struct zStatelessInputSliceFields zbytes, uint32_t zmaximum_count, uint32_t zmaximum_item_length);

struct zStatelessInputRef zdecode_stateless_input_ref(struct zStatelessInputSliceFields zinput);

unit zindex_witness_nodes_cursor(struct zBoundedSszzListCursor zcursor);

unit zindex_witness_nodes(struct zBoundedSszzListRef znodes);

unit zindex_witness_codes_cursor(struct zBoundedSszzListCursor zcursor);

unit zindex_witness_codes(struct zBoundedSszzListRef zcodes);

void create_letbind_276(void);
void kill_letbind_276(void);


uint8_t znext_parent_header_field(uint8_t zindex);

struct zWitnessHeaderIndex zindex_witness_header_cursor(struct zWitnessHeaderIndex zstate);

struct zWitnessContext zindex_witness_headers(struct zBoundedSszzListRef zheaders);

uint32_t zdecode_payload_blob_gas_used(struct zStatelessInputSliceFields zpayload, struct zProtocolProfileFields zprofile);

uint32_t zdecode_payload_excess_blob_gas(struct zStatelessInputSliceFields zpayload, struct zProtocolProfileFields zprofile);

struct zBlockHeader zdecode_block_header_sszz(struct zStatelessInputRef zinput_ref);

struct zWithdrawal zdecode_withdrawal(struct zStatelessInputSliceFields zwithdrawal);

struct zChainConfig zdecode_chain_config(struct zStatelessInputSliceFields zcc, uint64_t znumber, uint64_t ztimestamp);

struct zStatelessInput zdecode_stateless_input(struct zStatelessInputRef zinput_ref);

struct zWitnessContext zindex_execution_witness(struct zStatelessInputRef zinput_ref);

struct zTransactionFields zdecode_transaction(struct zStatelessInputSliceFields ztransaction, struct zStatelessInputSliceFields zpublic_key);

struct zScratchSliceFields zwithdrawal_rlp(struct zStatelessInputSliceFields zwithdrawal);

uint32_t ztopics_rlp_content_sizze(uint64_t zindex);

uint32_t ztopics_rlp_sizze(uint64_t zindex);

uint32_t zlog_entry_rlp_content_sizze(uint64_t zindex);

uint32_t zlog_entry_rlp_sizze(uint64_t zindex);

uint32_t zlogs_rlp_content_sizze(struct zLogSeriesRef zlogs);

uint32_t zlogs_rlp_sizze(struct zLogSeriesRef zlogs);

unit zrlp_write_topics(uint64_t zindex);

unit zrlp_write_log_entry(uint64_t zindex);

unit zrlp_write_logs(struct zLogSeriesRef zlogs);

unit zrlp_write_logs_bloom(fixed_bytes_256 zbloom);

unit zreceipt_write_logs_bloom(struct zReceiptFields zreceipt);

uint32_t zreceipt_payload_content_sizze(struct zReceiptFields zr, u128 zcumulative_gas_used);

uint32_t zreceipt_encoded_length(struct zReceiptFields zr, u128 zcumulative_gas_used);

unit zreceipt_write_encoded(struct zReceiptFields zr, u128 zcumulative_gas_used);

unit zreceipt_record_append(struct zReceiptFields zr, u128 zcumulative_gas_used);

fixed_bytes_32 zindexed_receipt_trie_root(struct zReceiptRecordsRef zreceipts);

uint32_t zreceipt_store_begin(unit z3zE894);

struct ztuple_z8z5structz0zzScratchSliceFieldszCz0z5structz0zzScratchSliceFieldsz9 zreceipt_record_pop(struct zScratchSliceFields zrecords);

unit zreceipt_store_append(struct zReceiptFields zreceipt, u128 zcumulative_gas_used, uint32_t z_index);

fixed_bytes_32 zreceipt_store_root(uint32_t zrecords_start, uint32_t zcount);

bool zblock_logs_bloom_matches(struct zLogSeriesRef zlogs, struct zStatelessInputSliceFields zreference);

void create_letbind_277(void);
void kill_letbind_277(void);


void create_letbind_278(void);
void kill_letbind_278(void);


void create_letbind_279(void);
void kill_letbind_279(void);


void create_letbind_280(void);
void kill_letbind_280(void);


void create_letbind_281(void);
void kill_letbind_281(void);


void create_letbind_282(void);
void kill_letbind_282(void);


void create_letbind_283(void);
void kill_letbind_283(void);


void create_letbind_284(void);
void kill_letbind_284(void);


void create_letbind_285(void);
void kill_letbind_285(void);


void create_letbind_286(void);
void kill_letbind_286(void);


void create_letbind_287(void);
void kill_letbind_287(void);


void create_letbind_288(void);
void kill_letbind_288(void);


void create_letbind_289(void);
void kill_letbind_289(void);


void create_letbind_290(void);
void kill_letbind_290(void);


void create_letbind_291(void);
void kill_letbind_291(void);


void create_letbind_292(void);
void kill_letbind_292(void);


void create_letbind_293(void);
void kill_letbind_293(void);


void create_letbind_294(void);
void kill_letbind_294(void);


void create_letbind_295(void);
void kill_letbind_295(void);


void create_letbind_296(void);
void kill_letbind_296(void);


void create_letbind_297(void);
void kill_letbind_297(void);


void create_letbind_298(void);
void kill_letbind_298(void);


void create_letbind_299(void);
void kill_letbind_299(void);


void create_letbind_300(void);
void kill_letbind_300(void);


void create_letbind_301(void);
void kill_letbind_301(void);


void create_letbind_302(void);
void kill_letbind_302(void);


void create_letbind_303(void);
void kill_letbind_303(void);


void create_letbind_304(void);
void kill_letbind_304(void);


void zrun_system_call_frame(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 *rop, fixed_bytes_20 ztgt, struct zCodeFields zcode, struct zCalldataSlice zinput, uint32_t zmemory_base);

unit zsystem_call(fixed_bytes_20 ztgt, fixed_bytes_32 zinput);

struct zScratchSliceFields zsystem_call_checked(fixed_bytes_20 ztgt);

bool zdeposit_log_matches(uint64_t zindex);

struct zStatelessInputSliceFields zauthenticate_deposit_request(struct zLogDataSliceFields zdata, struct zStatelessInputSliceFields zexpected);

struct zStatelessInputSliceFields zauthenticate_deposit_logs(struct zLogSeriesRef zlogs, struct zStatelessInputSliceFields zexpected);

unit zvalidate_request_stream(fixed_bytes_20 ztgt, struct zStatelessInputSliceFields zexpected);

unit zvalidate_execution_requests(struct zStatelessInputRef zinput_ref);

struct zStatelessInputSliceFields zbal_ref_cursor(struct zRlpFieldRef zf);

struct zStatelessInputSliceFields zbal_ref_bytes(struct zRlpFieldRef zf);

u256 zbal_ref_word(struct zRlpFieldRef zf);

uint64_t zbal_ref_uint64(struct zRlpFieldRef zf);

unit zbal_expect_end(struct zStatelessInputSliceFields zcursor);

unit zbal_compare_index_word(struct zRlpFieldRef zpair, uint32_t zindex, u256 zvalue);

unit zbal_compare_index_nonce(struct zRlpFieldRef zpair, uint32_t zindex, uint64_t zvalue);

unit zbal_compare_index_code(struct zRlpFieldRef zpair, uint32_t zindex, fixed_bytes_32 zcode_hash);

unit zbal_validate_storage_change_values(struct zStatelessInputSliceFields zcursor, u256 zslot);

uint32_t zbal_validate_storage_changes(struct zStatelessInputSliceFields zcursor);

uint32_t zbal_validate_storage_reads(struct zStatelessInputSliceFields zcursor);

unit zbal_validate_balance_changes(struct zStatelessInputSliceFields zcursor);

unit zbal_validate_nonce_changes(struct zStatelessInputSliceFields zcursor);

unit zbal_validate_code_changes(struct zStatelessInputSliceFields zcursor);

uint32_t zbal_validate_accounts(struct zStatelessInputSliceFields zcursor);

unit zvalidate_block_access_list(struct zStatelessInputSliceFields zbytes, uint64_t zblock_gas_limit);

struct zBlockGasUsageFields zblock_gas_usage_empty(uint64_t z_limit);

void create_letbind_305(void);
void kill_letbind_305(void);


unit zrun_block_start_system_calls(unit z3zE795);

struct zBlockExecutionResult zexecute_block_transactions(struct zBoundedSszzListRef ztransactions, struct zStatelessInputSliceFields zpublic_keys, struct zStatelessInputSliceFields zexpected_deposits);

unit zapply_withdrawals(struct zBoundedSszzListRef zwithdrawals);

unit zapply_block_end_state(struct zBlockBody zbody);

struct zBlockExecutionResult zexecute_block_body(struct zBlockBody zbody, struct zStatelessInputRef zinput_ref);

fixed_bytes_32 zblock_header_hash(struct zBlockHeader zheader, fixed_bytes_32 ztransactions_root, fixed_bytes_32 zwithdrawals_root, fixed_bytes_32 zrequests_hash, fixed_bytes_32 zblock_access_list_hash);

struct ztuple_z8z5structz0zzScratchSliceFieldszCz0z5structz0zzScratchSliceFieldsz9 zindexed_receipt_parts(struct zIndexedTrieSource zsource);

struct zIndexedTrieCursor zindexed_trie_begin(struct zIndexedTrieSource zsource);

bool zindexed_trie_cursor_empty(struct zIndexedTrieCursor zcursor);

struct zTriePath zindexed_trie_cursor_key(struct zIndexedTrieCursor zcursor);

bool zindexed_trie_next_under(struct zIndexedTrieCursor zcursor, struct zTriePath zprefix);

void zindexed_trie_pop(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzIndexedTrieCursorz9 *rop, struct zIndexedTrieSource zsource, struct zIndexedTrieCursor zcursor);

void zindexed_trie_subtree(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzIndexedTrieCursorz9 *rop, struct zIndexedTrieSource zsource, struct zIndexedTrieCursor zcursor, struct zTriePath zprefix);

fixed_bytes_32 zindexed_trie_root(struct zIndexedTrieSource zsource);

fixed_bytes_32 ztransaction_trie_root(struct zBoundedSszzListRef ztxs);

fixed_bytes_32 zwithdrawals_trie_root(struct zBoundedSszzListRef zwds);

uint32_t zexpected_payload_excess_blob_gas(struct zWitnessContext zwitness);

fixed_bytes_32 zexecution_requests_hash(struct zStatelessInputRef zinput_ref);

unit zvalidate_execution_payload(struct zStatelessInput zinput, struct zStatelessInputRef zinput_ref, struct zWitnessContext zwitness);

unit zvalidate_executed_block(struct zBlock zblock, struct zBlockExecutionResult zresult);

void create_letbind_306(void);
void kill_letbind_306(void);


void create_letbind_307(void);
void kill_letbind_307(void);


void create_letbind_308(void);
void kill_letbind_308(void);


void create_letbind_309(void);
void kill_letbind_309(void);


void create_letbind_310(void);
void kill_letbind_310(void);


unit zverify_stateless_payload(struct zStatelessInputRef zinput_ref);

void create_letbind_311(void);
void kill_letbind_311(void);


void create_letbind_312(void);
void kill_letbind_312(void);


void create_letbind_313(void);
void kill_letbind_313(void);


void create_letbind_314(void);
void kill_letbind_314(void);


void create_letbind_315(void);
void kill_letbind_315(void);


void create_letbind_316(void);
void kill_letbind_316(void);


void create_letbind_317(void);
void kill_letbind_317(void);


void create_letbind_318(void);
void kill_letbind_318(void);


void create_letbind_319(void);
void kill_letbind_319(void);


void create_letbind_320(void);
void kill_letbind_320(void);


void create_letbind_321(void);
void kill_letbind_321(void);


void create_letbind_322(void);
void kill_letbind_322(void);


void create_letbind_323(void);
void kill_letbind_323(void);


void create_letbind_324(void);
void kill_letbind_324(void);


void create_letbind_325(void);
void kill_letbind_325(void);


void create_letbind_326(void);
void kill_letbind_326(void);


void create_letbind_327(void);
void kill_letbind_327(void);


void create_letbind_328(void);
void kill_letbind_328(void);


void create_letbind_329(void);
void kill_letbind_329(void);


void create_letbind_330(void);
void kill_letbind_330(void);


void create_letbind_331(void);
void kill_letbind_331(void);


void create_letbind_332(void);
void kill_letbind_332(void);


void create_letbind_333(void);
void kill_letbind_333(void);


void create_letbind_334(void);
void kill_letbind_334(void);


void create_letbind_335(void);
void kill_letbind_335(void);


void create_letbind_336(void);
void kill_letbind_336(void);


void create_letbind_337(void);
void kill_letbind_337(void);


void create_letbind_338(void);
void kill_letbind_338(void);


void create_letbind_339(void);
void kill_letbind_339(void);


void create_letbind_340(void);
void kill_letbind_340(void);


void create_letbind_341(void);
void kill_letbind_341(void);


void create_letbind_342(void);
void kill_letbind_342(void);


void create_letbind_343(void);
void kill_letbind_343(void);


void create_letbind_344(void);
void kill_letbind_344(void);


void create_letbind_345(void);
void kill_letbind_345(void);


void create_letbind_346(void);
void kill_letbind_346(void);


void create_letbind_347(void);
void kill_letbind_347(void);


void create_letbind_348(void);
void kill_letbind_348(void);


void create_letbind_349(void);
void kill_letbind_349(void);


void create_letbind_350(void);
void kill_letbind_350(void);


void create_letbind_351(void);
void kill_letbind_351(void);


void create_letbind_352(void);
void kill_letbind_352(void);


void create_letbind_353(void);
void kill_letbind_353(void);


void create_letbind_354(void);
void kill_letbind_354(void);


void create_letbind_355(void);
void kill_letbind_355(void);


void create_letbind_356(void);
void kill_letbind_356(void);


void create_letbind_357(void);
void kill_letbind_357(void);


void create_letbind_358(void);
void kill_letbind_358(void);


void create_letbind_359(void);
void kill_letbind_359(void);


fixed_bytes_32 zsszz_zzero_hash(uint8_t zlevel);

uint32_t zhtr_leaf_capacity(uint8_t zdepth);

fixed_bytes_32 zhtr_uint(uint64_t zv);

fixed_bytes_32 zhtr_u256(u256 zvalue);

fixed_bytes_32 zhtr_bytes32(fixed_bytes_32 zb);

fixed_bytes_32 zhtr_addr(fixed_bytes_20 zaddress_bytes);

fixed_bytes_32 zhtr_length_chunk(uint32_t zlen);

fixed_bytes_32 zmix_in_length(fixed_bytes_32 zroot, uint32_t zlen);

uint8_t zclog2(uint32_t zn);

fixed_bytes_32 zhtr_chunk(struct zStatelessInputSliceFields zbytes, sail_int zchunk_index);

uint32_t zhtr_chunk_count(uint32_t zbyte_len);

fixed_bytes_32 zhtr_bytes_root(struct zStatelessInputSliceFields zbytes, uint8_t zdepth);

fixed_bytes_32 zhtr_bytevector(struct zStatelessInputSliceFields zbytes);

fixed_bytes_32 zhtr_withdrawal(struct zStatelessInputSliceFields zwithdrawal);

fixed_bytes_32 zhtr_transactions(struct zBoundedSszzListRef ztransactions);

fixed_bytes_32 zhtr_withdrawals(struct zBoundedSszzListRef zwithdrawals);

fixed_bytes_32 zhtr_execution_payload(struct zStatelessInputRef zinput_ref);

fixed_bytes_32 zhtr_versioned_hashes(struct zStatelessInputSliceFields zversioned_hashes);

fixed_bytes_32 zhtr_deposit(struct zStatelessInputSliceFields zdeposit);

fixed_bytes_32 zhtr_withdrawal_request(struct zStatelessInputSliceFields zrequest);

fixed_bytes_32 zhtr_consolidation_request(struct zStatelessInputSliceFields zrequest);

fixed_bytes_32 zhtr_builder_deposit_request(struct zStatelessInputSliceFields zrequest);

fixed_bytes_32 zhtr_builder_exit_request(struct zStatelessInputSliceFields zrequest);

fixed_bytes_32 zhtr_request_leaf(struct zStatelessInputSliceFields zrequests, sail_int zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_deposits(struct zStatelessInputSliceFields zdeposits);

fixed_bytes_32 zhtr_withdrawal_requests(struct zStatelessInputSliceFields zrequests);

fixed_bytes_32 zhtr_consolidation_requests(struct zStatelessInputSliceFields zrequests);

fixed_bytes_32 zhtr_builder_deposit_requests(struct zStatelessInputSliceFields zrequests);

fixed_bytes_32 zhtr_builder_exit_requests(struct zStatelessInputSliceFields zrequests);

fixed_bytes_32 zhtr_execution_requests(struct zStatelessInputRef zinput_ref);

fixed_bytes_32 zhtr_new_payload_request(struct zStatelessInputRef zinput_ref);

void create_letbind_360(void);
void kill_letbind_360(void);


unit zwrite_prefix(fixed_bytes_32 zroot, bool zsuccess);

unit zcommit_validation_result(fixed_bytes_32 zroot, bool zsuccess, struct zStatelessInputSliceFields zchain_config);

unit zwrite_validation_result(struct zStatelessInputRef zinput_ref, bool zsuccess);

unit zmain(unit z3zE444);

unit zinitializze_registers(unit z3zE443);

struct zPrecompileResult zaccelerator_result_bool_uint16_t_to_struct_zzPrecompileResult(bool zsuccess, uint16_t zoutput_len);

struct zPrecompileResult zaccelerator_result_bool_uint32_t_to_struct_zzPrecompileResult(bool zsuccess, uint32_t zoutput_len);

struct zPrecompileResult zaccelerator_result_bool_uint8_t_to_struct_zzPrecompileResult(bool zsuccess, uint8_t zoutput_len);

struct zPrecompileResult zaccelerator_result_bool_uint8_t_to_struct_zzPrecompileResult_variant_2(bool zsuccess, uint8_t zoutput_len);

struct zPrecompileResult zaccelerator_result_bool_uint8_t_to_struct_zzPrecompileResult_variant_3(bool zsuccess, uint8_t zoutput_len);

u256 zalu_add_u256_u128_to_u256(u256 za, u128 zb);

u256 zalu_add_u256_uint64_t_to_u256(u256 za, uint64_t zb);

uint64_t zanalyzze_code_struct_zzCodeRegionSliceFields_uint8_t_to_uint64_t(struct zCodeRegionSliceFields zcode, uint8_t zfork);

uint64_t zanalyzze_code_struct_zzCodeRegionSliceFields_uint8_t_to_uint64_t_variant_2(struct zCodeRegionSliceFields zcode, uint8_t zfork);

unit zanalyzze_code_from(struct zCodeRegionSliceFields zcode, uint8_t zfork, uint64_t ztable, uint8_t zpc);

struct zAuthorizzationListRefFields zauthorizzation_list_ref_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzAuthorizzzzationListRefFields(struct zStatelessInputSliceFields zencoded, uint8_t zcount);

uint32_t zblock_blob_gas_add_uint8_t_uint32_t_uint32_t_to_uint32_t(uint8_t zmaximum_count, uint32_t zaccumulated, uint32_t ztransaction);

struct zBlockGasUsageFields zblock_gas_usage_add_struct_zzBlockGasUsageFields_uint64_t_uint64_t_uint64_t_to_struct_zzBlockGasUsageFields(struct zBlockGasUsageFields zusage, uint64_t zadd_execution, uint64_t zadd_state, uint64_t zadd_receipt);

struct zBlockGasUsageFields zblock_gas_usage_add_struct_zzBlockGasUsageFields_uint64_t_uint8_t_uint64_t_to_struct_zzBlockGasUsageFields(struct zBlockGasUsageFields zusage, uint64_t zadd_execution, uint8_t zadd_state, uint64_t zadd_receipt);

bool zbls_g1_padding_struct_zzCalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct zCalldataSlice zinput, uint8_t zbase, uint16_t zstride, uint32_t zcount);

bool zbls_g1_padding_struct_zzCalldataSlice_uint8_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zinput, uint8_t zbase, uint8_t zstride, uint32_t zcount);

bool zbls_g1_padding_struct_zzCalldataSlice_uint8_t_uint8_t_uint8_t_to_bool(struct zCalldataSlice zinput, uint8_t zbase, uint8_t zstride, uint8_t zcount);

bool zbls_g2_padding_struct_zzCalldataSlice_uint8_t_uint16_t_uint32_t_to_bool(struct zCalldataSlice zinput, uint8_t zbase, uint16_t zstride, uint32_t zcount);

bool zbls_g2_padding_struct_zzCalldataSlice_uint8_t_uint16_t_uint32_t_to_bool_variant_2(struct zCalldataSlice zinput, uint8_t zbase, uint16_t zstride, uint32_t zcount);

bool zbls_g2_padding_struct_zzCalldataSlice_uint8_t_uint16_t_uint8_t_to_bool(struct zCalldataSlice zinput, uint8_t zbase, uint16_t zstride, uint8_t zcount);

struct zGasCharge zbls_msm_gas_zzzz5fveczz8128zzCzz0zz5u16zz9_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_zzGasCharge(zz5fvecz8128zCz0z5u16z9 ztable, uint16_t zbase, uint16_t zmaxd, uint32_t zk, uint64_t zavailable);

struct zGasCharge zbls_msm_gas_zzzz5fveczz8128zzCzz0zz5u16zz9_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_zzGasCharge_variant_2(zz5fvecz8128zCz0z5u16z9 ztable, uint16_t zbase, uint16_t zmaxd, uint32_t zk, uint64_t zavailable);

uint16_t zbranch_content_length_add(uint16_t zcurrent, uint8_t zaddition);

uint64_t zcalldata_slice_byte_struct_zzCalldataSlice_uint8_t_to_uint64_t(struct zCalldataSlice zs, uint8_t zoff);

u256 zcalldata_slice_load_struct_zzCalldataSlice_uint8_t_to_u256(struct zCalldataSlice zs, uint8_t zoff);

u256 zcalldata_slice_load_struct_zzCalldataSlice_uint8_t_to_u256_variant_2(struct zCalldataSlice zs, uint8_t zoff);

u256 zcalldata_slice_load_struct_zzCalldataSlice_uint8_t_to_u256_variant_3(struct zCalldataSlice zs, uint8_t zoff);

void zcalldata_sub_slice(struct zCalldataSlice *rop, struct zCalldataSlice zs, uint8_t zoff, uint8_t zlen);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 zcharge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_zztuple_zz8zz5boolzzCzz0zz5u64zzCzz0zz5u64zzCzz0zz5u32zz9(uint64_t zg, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled, uint32_t zamount);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 zcharge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_zztuple_zz8zz5boolzzCzz0zz5u64zzCzz0zz5u64zzCzz0zz5u32zz9_variant_2(uint64_t zg, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled, uint32_t zamount);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64zCz0z5u32z9 zcharge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_zztuple_zz8zz5boolzzCzz0zz5u64zzCzz0zz5u64zzCzz0zz5u32zz9(uint64_t zg, uint64_t zstate_gas_remaining, uint32_t zstate_gas_spilled, uint64_t zamount);

fixed_bytes_32 zcode_db_insert_struct_zzCodeRegionSliceFields_uint8_t_to_fixed_bytes_32(struct zCodeRegionSliceFields zcode, uint8_t zfork);

fixed_bytes_32 zcode_db_insert_struct_zzCodeRegionSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zCodeRegionSliceFields zcode, uint8_t zfork);

uint32_t zcompute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t(uint8_t zfork, uint8_t ztarget, uint8_t zmaximum, uint32_t zdenominator);

uint32_t zcompute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint32_t_to_uint32_t_variant_2(uint8_t zfork, uint8_t ztarget, uint8_t zmaximum, uint32_t zdenominator);

uint8_t zcompute_profile_excess_blob_gas_limit_uint8_t_uint8_t_uint8_t_uint8_t_to_uint8_t(uint8_t zfork, uint8_t ztarget, uint8_t zmaximum, uint8_t zdenominator);

uint64_t zconserved_gas_add_uint64_t_uint32_t_to_uint64_t(uint64_t zavailable, uint32_t zcredit);

struct zGasCharge zcopy_gas_cost_u256_uint64_t_to_struct_zzGasCharge(u256 zsizze, uint64_t zavailable);

uint8_t zdecode_blob_hash_items_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_uint8_t(struct zStatelessInputSliceFields zcursor, uint8_t zlimit, uint8_t zcount);

uint8_t zdecode_blob_hash_items_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_uint8_t_variant_2(struct zStatelessInputSliceFields zcursor, uint8_t zlimit, uint8_t zcount);

struct ztuple_z8z5u32zCz0z5bv8z9 zdecode_deep_immediate(struct zCodeFields zframe_code, uint32_t zimmediate_offset, enum zDeepStackOperation zoperation);

void zdecode_input_branch_node_struct_zzStatelessInputSliceFields_uint8_t_zzzz5fveczz816zzCzz0zz5unionzz0zzzzNodeRefzz9_to_struct_zzInputTrieNode(struct zInputTrieNode *rop, struct zStatelessInputSliceFields zcursor, uint8_t zindex, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren);

void zdecode_input_branch_node_struct_zzStatelessInputSliceFields_uint8_t_zzzz5fveczz816zzCzz0zz5unionzz0zzzzNodeRefzz9_to_struct_zzInputTrieNode_variant_2(struct zInputTrieNode *rop, struct zStatelessInputSliceFields zcursor, uint8_t zindex, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren);

struct zParentHeaderFields zdecode_parent_header_fields_struct_zzStatelessInputSliceFields_uint8_t_struct_zzParentHeaderFields_to_struct_zzParentHeaderFields(struct zStatelessInputSliceFields zcursor, uint8_t zfield_index, struct zParentHeaderFields zfields);

struct zParentHeaderFields zdecode_parent_header_fields_struct_zzStatelessInputSliceFields_uint8_t_struct_zzParentHeaderFields_to_struct_zzParentHeaderFields_variant_2(struct zStatelessInputSliceFields zcursor, uint8_t zfield_index, struct zParentHeaderFields zfields);

struct ztuple_z8z5u32zCz0z5structz0zz__sail_c_repr_u256z9 zdecode_push_immediate(struct zCodeFields zframe_code, uint32_t zimmediate_offset, uint8_t zwidth);

void zdecode_scratch_branch_node_struct_zzScratchSliceFields_uint8_t_zzzz5fveczz816zzCzz0zz5unionzz0zzzzNodeRefzz9_to_struct_zzScratchTrieNode(struct zScratchTrieNode *rop, struct zScratchSliceFields zcursor, uint8_t zindex, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren);

void zdecode_scratch_branch_node_struct_zzScratchSliceFields_uint8_t_zzzz5fveczz816zzCzz0zz5unionzz0zzzzNodeRefzz9_to_struct_zzScratchTrieNode_variant_2(struct zScratchTrieNode *rop, struct zScratchSliceFields zcursor, uint8_t zindex, zz5fvecz816zCz0z5unionz0zzNodeRefz9 zchildren);

uint64_t zdecode_scratch_uint(struct zScratchSliceFields zinput, uint8_t zoffset);

void zdecode_simple(struct zast *rop, uint8_t zopcode, uint8_t zfork);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_2(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_3(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_4(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_5(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_6(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t_variant_7(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_2(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_3(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_4(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_5(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint64_t zdecode_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_6(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

bool zdeployed_code_sizze_allowed(uint32_t zsizze);

void zexecute_dup(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint8_t zn);

void zexecute_log(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5u32zCz0z5unionz0zzOpcodeOutcomez9 *rop, fixed_bytes_20 zcarried_address, bool zcarried_is_static, uint32_t zmemory_base, uint8_t zn, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_height);

void zexecute_swap(struct ztuple_z8z5u64zCz0z5structz0zzStackPointerzCz0z5unionz0zzOpcodeOutcomez9 *rop, uint64_t zcarried_gas, struct zStackPointer zcarried_sp, uint8_t zn);

struct zExecutionProfileFields zexecution_profile_for_struct_zzProtocolProfileFields_uint8_t_to_struct_zzExecutionProfileFields(struct zProtocolProfileFields zprotocol, uint8_t zblock_limit);

void zfetch(struct ztuple_z8z5u32zCz0z5unionz0zzastz9 *rop, struct zCodeFields zframe_code, uint32_t zcurrent, uint8_t zfork);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_2(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_3(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_4(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_5(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_6(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_7(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint16_t_uint64_t_to_struct_zzGasCharge_variant_8(uint16_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint32_t_uint64_t_to_struct_zzGasCharge(uint32_t zcost, uint64_t zavailable);

struct zGasCharge zfixed_precompile_gas_uint8_t_uint64_t_to_struct_zzGasCharge(uint8_t zcost, uint64_t zavailable);

struct zGasLimitsFields zgas_limits_for_struct_zzProtocolProfileFields_uint8_t_to_struct_zzGasLimitsFields(struct zProtocolProfileFields zprofile, uint8_t zblock_limit);

uint64_t zgas_sub_uint64_t_uint16_t_to_uint64_t(uint64_t zleft, uint16_t zright);

uint64_t zgas_sub_uint64_t_uint32_t_to_uint64_t(uint64_t zleft, uint32_t zright);

void zguard_stack_struct_zzStackPointer_uint16_t_uint8_t_to_struct_zzOpcodeOutcome(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint16_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint16_t_uint8_t_to_struct_zzOpcodeOutcome_variant_2(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint16_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_2(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_3(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_4(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_5(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_6(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_7(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_8(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_9(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_10(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_11(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_12(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_13(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_14(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_15(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

void zguard_stack_struct_zzStackPointer_uint8_t_uint8_t_to_struct_zzOpcodeOutcome_variant_16(struct zOpcodeOutcome *rop, struct zStackPointer zcarried_sp, uint8_t zinputs, uint8_t zoutputs);

fixed_bytes_32 zhtr_bytelist_struct_zzStatelessInputSliceFields_uint32_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint32_t zlimit_bytes);

fixed_bytes_32 zhtr_bytelist_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zlimit_bytes);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u128_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u256_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u320_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_bytes_subtree_struct_zzStatelessInputSliceFields_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_u128_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u128 zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_u256_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u256 zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_u320_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, u320 zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_uint32_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint32_t zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_uint32_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint32_t zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_uint64_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint64_t zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zchunk_index);

fixed_bytes_32 zhtr_chunk_struct_zzStatelessInputSliceFields_sail_int_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, sail_int zchunk_index);

uint32_t zhtr_chunk_count_uint32_t_to_uint32_t(uint32_t zbyte_len);

uint32_t zhtr_chunk_count_uint8_t_to_uint32_t(uint8_t zbyte_len);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_2(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_3(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_4(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_5(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_6(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_7(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_8(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_9(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_10(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_11(uint8_t zdepth);

uint32_t zhtr_leaf_capacity_uint8_t_to_uint32_t_variant_12(uint8_t zdepth);

fixed_bytes_32 zhtr_length_chunk_uint16_t_to_fixed_bytes_32(uint16_t zlen);

fixed_bytes_32 zhtr_length_chunk_uint8_t_to_fixed_bytes_32(uint8_t zlen);

fixed_bytes_32 zhtr_length_chunk_uint8_t_to_fixed_bytes_32_variant_2(uint8_t zlen);

fixed_bytes_32 zhtr_length_chunk_uint8_t_to_fixed_bytes_32_variant_3(uint8_t zlen);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u128_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, u128 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u128_enum_zzHtrRequestKind_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, u128 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u256_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, u256 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u256_enum_zzHtrRequestKind_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, u256 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u320_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, u320 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_u320_enum_zzHtrRequestKind_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, u320 zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_uint32_t_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, uint32_t zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_uint64_t_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, uint64_t zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_uint64_t_enum_zzHtrRequestKind_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, uint64_t zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_request_leaf_struct_zzStatelessInputSliceFields_uint8_t_enum_zzHtrRequestKind_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, uint8_t zindex, enum zHtrRequestKind zkind);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u128_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u256_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u320_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_6(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_requests_subtree_struct_zzStatelessInputSliceFields_enum_zzHtrRequestKind_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zrequests, enum zHtrRequestKind zkind, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_sszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_bytevector_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zoffset, uint16_t zlen);

fixed_bytes_32 zhtr_sszz_bytevector_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zoffset, uint8_t zlen);

fixed_bytes_32 zhtr_sszz_bytevector_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zoffset, uint8_t zlen);

fixed_bytes_32 zhtr_sszz_bytevector_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zbytes, uint8_t zoffset, uint8_t zlen);

fixed_bytes_32 zhtr_sszz_bytevector_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zbytes, uint8_t zoffset, uint8_t zlen);

fixed_bytes_32 zhtr_sszz_u256(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_6(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_7(struct zStatelessInputSliceFields zbytes, uint16_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_sszz_uint_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_6(struct zStatelessInputSliceFields zbytes, uint8_t zoffset);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u128_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u256_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u320_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(struct zBoundedSszzListRef ztransactions, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef ztransactions, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_transactions_subtree_struct_zzBoundedSszzzzListRef_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef ztransactions, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u128_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u256_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u320_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zversioned_hashes, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zversioned_hashes, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_versioned_hashes_subtree_struct_zzStatelessInputSliceFields_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zversioned_hashes, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u128_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u256_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u320_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(struct zBoundedSszzListRef zwithdrawals, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(struct zBoundedSszzListRef zwithdrawals, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zhtr_withdrawals_subtree_struct_zzBoundedSszzzzListRef_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(struct zBoundedSszzListRef zwithdrawals, sail_int zstart, uint32_t zcount, uint8_t zdepth);

bool zinitcode_sizze_allowed(uint32_t zsizze);

void zinterpret_uint32_t_uint8_t_uint8_t___int128_struct_zzStackPointer_uint32_t_uint32_t_fixed_bytes_20_fixed_bytes_20_fixed_bytes_20_u256_uint8_t_bool_uint8_t_struct_zzCodeFields_struct_zzCalldataSlice_to_struct_zztuple_zz8zz5u64zzCzz0zz5u64zzCzz0zz5u32zzCzz0zz5i128zzCzz0zz5unionzz0zzzzFrameStatuszzCzz0zz5structzz0zzzzOutputSliceFieldszz9(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 *rop, uint32_t zinitial_gas, uint8_t zinitial_state_gas, uint8_t zinitial_state_spill, __int128 zinitial_refund, struct zStackPointer zinitial_sp, uint32_t zinitial_memory_base, uint32_t zinitial_memory_height, fixed_bytes_20 zinitial_caller, fixed_bytes_20 zinitial_address, fixed_bytes_20 zinitial_code_address, u256 zinitial_value, uint8_t zinitial_state_gas_reservoir, bool zinitial_is_static, uint8_t zinitial_depth, struct zCodeFields zinitial_code, struct zCalldataSlice zinitial_calldata);

void zinterpret_uint64_t_uint64_t_uint32_t___int128_struct_zzStackPointer_uint32_t_uint32_t_fixed_bytes_20_fixed_bytes_20_fixed_bytes_20_u256_uint64_t_bool_uint8_t_struct_zzCodeFields_struct_zzCalldataSlice_to_struct_zztuple_zz8zz5u64zzCzz0zz5u64zzCzz0zz5u32zzCzz0zz5i128zzCzz0zz5unionzz0zzzzFrameStatuszzCzz0zz5structzz0zzzzOutputSliceFieldszz9(struct ztuple_z8z5u64zCz0z5u64zCz0z5u32zCz0z5i128zCz0z5unionz0zzFrameStatuszCz0z5structz0zzOutputSliceFieldsz9 *rop, uint64_t zinitial_gas, uint64_t zinitial_state_gas, uint32_t zinitial_state_spill, __int128 zinitial_refund, struct zStackPointer zinitial_sp, uint32_t zinitial_memory_base, uint32_t zinitial_memory_height, fixed_bytes_20 zinitial_caller, fixed_bytes_20 zinitial_address, fixed_bytes_20 zinitial_code_address, u256 zinitial_value, uint64_t zinitial_state_gas_reservoir, bool zinitial_is_static, uint8_t zinitial_depth, struct zCodeFields zinitial_code, struct zCalldataSlice zinitial_calldata);

unit zk_add_balance_fixed_bytes_20_u128_to_unit(fixed_bytes_20 za, u128 zv);

unit zk_add_balance_fixed_bytes_20_uint64_t_to_unit(fixed_bytes_20 za, uint64_t zv);

struct zGasCharge zlinear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_zzGasCharge(uint16_t zbase, uint16_t zper_unit, uint32_t zunits, uint64_t zavailable);

struct zGasCharge zlinear_gas_uint16_t_uint16_t_uint32_t_uint64_t_to_struct_zzGasCharge_variant_2(uint16_t zbase, uint16_t zper_unit, uint32_t zunits, uint64_t zavailable);

struct zGasCharge zlinear_gas_uint16_t_uint8_t_uint32_t_uint64_t_to_struct_zzGasCharge(uint16_t zbase, uint8_t zper_unit, uint32_t zunits, uint64_t zavailable);

struct zGasCharge zlinear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_zzGasCharge(uint8_t zbase, uint8_t zper_unit, uint32_t zunits, uint64_t zavailable);

struct zGasCharge zlinear_gas_uint8_t_uint8_t_uint32_t_uint64_t_to_struct_zzGasCharge_variant_2(uint8_t zbase, uint8_t zper_unit, uint32_t zunits, uint64_t zavailable);

uint64_t zlog_data_byte(struct zLogDataSliceFields zs, uint8_t zoff);

u256 zlog_data_slice_load_struct_zzLogDataSliceFields_uint16_t_to_u256(struct zLogDataSliceFields zs, uint16_t zoff);

u256 zlog_data_slice_load_struct_zzLogDataSliceFields_uint8_t_to_u256(struct zLogDataSliceFields zs, uint8_t zoff);

struct zLogDataSliceFields zlog_data_sub_slice_struct_zzLogDataSliceFields_uint16_t_uint8_t_to_struct_zzLogDataSliceFields(struct zLogDataSliceFields zs, uint16_t zoff, uint8_t zlen);

struct zLogDataSliceFields zlog_data_sub_slice_struct_zzLogDataSliceFields_uint8_t_uint8_t_to_struct_zzLogDataSliceFields(struct zLogDataSliceFields zs, uint8_t zoff, uint8_t zlen);

uint64_t zmem_cost(uint32_t zwords);

struct zMemoryAccessFields zmemory_access_u256_u256_to_struct_zzMemoryAccessFields(u256 zstart, u256 zsizze);

struct zMemoryAccessFields zmemory_access_u256_u256_to_struct_zzMemoryAccessFields_variant_2(u256 zstart, u256 zsizze);

struct zGasCharge zmemory_expansion_gas_cost(uint32_t zmem, u320 zrequested_height, uint64_t zavailable);

struct zMemoryRangeFields zmemory_range_uint8_t_uint8_t_to_struct_zzMemoryRangeFields(uint8_t zoff, uint8_t zlen);

u320 zmemory_requested_height_u256_u256_to_u320(u256 zstart, u256 zsizze);

u320 zmemory_requested_height_u256_u256_to_u320_variant_2(u256 zstart, u256 zsizze);

u320 zmemory_requested_height_u256_u256_to_u320_variant_3(u256 zstart, u256 zsizze);

uint64_t zmemory_slice_byte_struct_zzEvmMemorySliceFields_uint8_t_to_uint64_t(struct zEvmMemorySliceFields zs, uint8_t zoff);

u256 zmemory_slice_load_struct_zzEvmMemorySliceFields_uint8_t_to_u256(struct zEvmMemorySliceFields zs, uint8_t zoff);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(struct zEvmMemorySliceFields zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(struct zEvmMemorySliceFields zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(struct zEvmMemorySliceFields zs, uint32_t zstart, uint32_t zstride, uint8_t zwidth, uint32_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(struct zEvmMemorySliceFields zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(struct zEvmMemorySliceFields zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(struct zEvmMemorySliceFields zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(struct zEvmMemorySliceFields zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(struct zEvmMemorySliceFields zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zmemory_slice_strided_zzero_value_struct_zzEvmMemorySliceFields_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(struct zEvmMemorySliceFields zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

struct zEvmMemorySliceFields zmemory_sub_slice_struct_zzEvmMemorySliceFields_uint8_t_uint8_t_to_struct_zzEvmMemorySliceFields(struct zEvmMemorySliceFields zs, uint8_t zoff, uint8_t zlen);

uint16_t zmemory_word_count_uint32_t_to_uint16_t(uint32_t zbyte_len);

uint32_t zmemory_word_count_uint32_t_to_uint32_t(uint32_t zbyte_len);

u256 zmemory_word_count_word_u256_to_u256(u256 zbyte_len);

struct zGasCharge zmemory_word_gas_cost_uint16_t_uint16_t_u256_uint64_t_to_struct_zzGasCharge(uint16_t zbase, uint16_t zper_word, u256 zsizze, uint64_t zavailable);

struct zGasCharge zmemory_word_gas_cost_uint16_t_uint8_t_u256_uint64_t_to_struct_zzGasCharge(uint16_t zbase, uint8_t zper_word, u256 zsizze, uint64_t zavailable);

fixed_bytes_32 zmerkleizze_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz819zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz819zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz82zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz82zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz83zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz83zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz84zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz84zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_3(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5fveczz85zzCzz0zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5fvecz85zCz0z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u128_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u128 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u256_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u256 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_u320_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, u320 zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint32_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint32_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint64_t_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint64_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_uint8_t_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, uint8_t zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmerkleizze_vector_subtree_zzzz5veczz8zz5structzz0zzzz__sail_c_repr_fixed_byteszz832zz9zz9_sail_int_uint32_t_uint8_t_to_fixed_bytes_32_variant_2(zz5vecz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 zleaves, sail_int zstart, uint32_t zcount, uint8_t zdepth);

fixed_bytes_32 zmix_in_length_fixed_bytes_32_uint16_t_to_fixed_bytes_32(fixed_bytes_32 zroot, uint16_t zlen);

fixed_bytes_32 zmix_in_length_fixed_bytes_32_uint8_t_to_fixed_bytes_32(fixed_bytes_32 zroot, uint8_t zlen);

fixed_bytes_32 zmix_in_length_fixed_bytes_32_uint8_t_to_fixed_bytes_32_variant_2(fixed_bytes_32 zroot, uint8_t zlen);

fixed_bytes_32 zmix_in_length_fixed_bytes_32_uint8_t_to_fixed_bytes_32_variant_3(fixed_bytes_32 zroot, uint8_t zlen);

uint8_t znext_parent_header_field_uint8_t_to_uint8_t(uint8_t zindex);

struct zOutputSliceFields zoutput_buffer_slice_uint16_t_to_struct_zzOutputSliceFields(uint16_t zlen);

struct zOutputSliceFields zoutput_buffer_slice_uint32_t_to_struct_zzOutputSliceFields(uint32_t zlen);

struct zOutputSliceFields zoutput_buffer_slice_uint32_t_to_struct_zzOutputSliceFields_variant_2(uint32_t zlen);

struct zOutputSliceFields zoutput_buffer_slice_uint8_t_to_struct_zzOutputSliceFields(uint8_t zlen);

struct zOutputSliceFields zoutput_buffer_slice_uint8_t_to_struct_zzOutputSliceFields_variant_2(uint8_t zlen);

struct zOutputSliceFields zoutput_buffer_slice_uint8_t_to_struct_zzOutputSliceFields_variant_3(uint8_t zlen);

uint64_t zoutput_byte(struct zOutputSliceFields zs, uint8_t zoff);

struct zOutputSliceFields zoutput_slice_uint8_t_uint16_t_to_struct_zzOutputSliceFields(uint8_t zoff, uint16_t zlen);

struct zOutputSliceFields zoutput_slice_uint8_t_uint32_t_to_struct_zzOutputSliceFields(uint8_t zoff, uint32_t zlen);

struct zOutputSliceFields zoutput_slice_uint8_t_uint8_t_to_struct_zzOutputSliceFields(uint8_t zoff, uint8_t zlen);

unit zoutput_slice_copy_struct_zzOutputSliceFields_uint32_t_uint8_t_uint32_t_to_unit(struct zOutputSliceFields zs, uint32_t zdst, uint8_t zoff, uint32_t zlen);

void zoverlay_extension_subtree_struct_zzNodeRef_struct_zzTriePath_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zNodeRef zchildref, struct zTriePath zchild_prefix, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zoverlay_extension_subtree_struct_zzNodeRef_struct_zzTriePath_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_2(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zNodeRef zchildref, struct zTriePath zchild_prefix, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zoverlay_extension_subtree_struct_zzNodeRef_struct_zzTriePath_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_3(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zNodeRef zchildref, struct zTriePath zchild_prefix, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zoverlay_leaf_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_struct_zzTriePath_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zStatelessInputSliceFields zvalue, uint8_t zcursor);

void zoverlay_leaf_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_struct_zzTriePath_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_2(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zStatelessInputSliceFields zvalue, uint8_t zcursor);

void zoverlay_leaf_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_struct_zzTriePath_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_3(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zStatelessInputSliceFields zvalue, uint8_t zcursor);

struct zTriePath zpath_append_byte_struct_zzTriePath_uint64_t_to_struct_zzTriePath(struct zTriePath zpath, uint64_t zvalue);

uint8_t zpath_byte_index(uint8_t zi);

struct zTriePath zpath_drop_struct_zzTriePath_uint8_t_to_struct_zzTriePath(struct zTriePath zpath, uint8_t zn);

struct zTriePath zpath_drop_struct_zzTriePath_uint8_t_to_struct_zzTriePath_variant_2(struct zTriePath zpath, uint8_t zn);

struct zTriePath zpath_drop_struct_zzTriePath_uint8_t_to_struct_zzTriePath_variant_3(struct zTriePath zpath, uint8_t zn);

bool zpath_matches_struct_zzTriePath_uint8_t_struct_zzTriePath_to_bool(struct zTriePath zkey, uint8_t zpos, struct zTriePath zseg);

bool zpath_matches_struct_zzTriePath_uint8_t_struct_zzTriePath_to_bool_variant_2(struct zTriePath zkey, uint8_t zpos, struct zTriePath zseg);

uint64_t zpath_nibble_struct_zzTriePath_uint8_t_to_uint64_t(struct zTriePath zpath, uint8_t zi);

u256 zpc_word_struct_zzCalldataSlice_uint8_t_uint8_t_to_u256(struct zCalldataSlice zinput, uint8_t zstart, uint8_t zbyte_count);

u256 zpc_word_struct_zzCalldataSlice_uint8_t_uint8_t_to_u256_variant_2(struct zCalldataSlice zinput, uint8_t zstart, uint8_t zbyte_count);

u256 zpc_word_struct_zzCalldataSlice_uint8_t_uint8_t_to_u256_variant_3(struct zCalldataSlice zinput, uint8_t zstart, uint8_t zbyte_count);

u256 zpc_word_struct_zzCalldataSlice_uint8_t_uint8_t_to_u256_variant_4(struct zCalldataSlice zinput, uint8_t zstart, uint8_t zbyte_count);

u256 zpc_word_after_declared_field_struct_zzCalldataSlice_uint8_t_u256_uint8_t_to_u256(struct zCalldataSlice zinput, uint8_t zprefix, u256 zdeclared_length, uint8_t zbyte_count);

void zprepare_amsterdam_transaction_dispatch(struct ztuple_z8z5structz0zzTransactionPreparationzCz0z5u64zCz0z5u64zCz0z5u32zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9zCz0z5structz0zzCodeFieldszCz0z5unionz0zzCalldataSlicez9 *rop, struct zTransactionFields ztx, struct zTxValidityFields zv, struct zTxUpfrontResult zupfront, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill);

void zprepared_authorizzation_tail(struct zPreparedAuthorizzationList *rop, struct zPreparedAuthorizzationList zauthorizzations, uint16_t zcount);

u256 zread_push(struct zCodeRegionSliceFields zcode, uint32_t zoffset, uint8_t zn);

unit zreceipt_record_write_length(uint32_t zvalue);

__int128 zrecord_refund___int128_uint16_t_to___int128(__int128 zrefund, uint16_t zdelta);

struct zRlpEncoder zrlp_encoder_begin_uint16_t_to_struct_zzRlpEncoder(uint16_t zexpected_len);

struct zRlpEncoder zrlp_encoder_begin_uint8_t_to_struct_zzRlpEncoder(uint8_t zexpected_len);

struct zRlpEncoder zrlp_encoder_begin_uint8_t_to_struct_zzRlpEncoder_variant_2(uint8_t zexpected_len);

struct zRlpEncoder zrlp_encoder_begin_uint8_t_to_struct_zzRlpEncoder_variant_3(uint8_t zexpected_len);

uint8_t zrlp_input_slice_sizze(struct zStatelessInputSliceFields zdata);

uint8_t zrlp_length_prefix_len_uint16_t_to_uint8_t(uint16_t zlen);

uint8_t zrlp_length_prefix_len_uint16_t_to_uint8_t_variant_2(uint16_t zlen);

uint8_t zrlp_length_prefix_len_uint32_t_to_uint8_t(uint32_t zlen);

uint8_t zrlp_length_prefix_len_uint32_t_to_uint8_t_variant_2(uint32_t zlen);

uint8_t zrlp_length_prefix_len_uint8_t_to_uint8_t(uint8_t zlen);

uint8_t zrlp_length_prefix_len_uint8_t_to_uint8_t_variant_2(uint8_t zlen);

uint8_t zrlp_length_prefix_len_uint8_t_to_uint8_t_variant_3(uint8_t zlen);

uint8_t zrlp_length_prefix_len_uint8_t_to_uint8_t_variant_4(uint8_t zlen);

uint8_t zrlp_length_prefix_len_uint8_t_to_uint8_t_variant_5(uint8_t zlen);

u256 zrlp_length_word_uint16_t_to_u256(uint16_t zvalue);

u256 zrlp_length_word_uint16_t_to_u256_variant_2(uint16_t zvalue);

u256 zrlp_length_word_uint16_t_to_u256_variant_3(uint16_t zvalue);

u256 zrlp_length_word_uint32_t_to_u256(uint32_t zvalue);

u256 zrlp_length_word_uint32_t_to_u256_variant_2(uint32_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_2(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_3(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_4(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_5(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_6(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_7(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_8(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_9(uint8_t zvalue);

u256 zrlp_length_word_uint8_t_to_u256_variant_10(uint8_t zvalue);

uint16_t zrlp_list_sizze_uint16_t_to_uint16_t(uint16_t zcontent_len);

uint8_t zrlp_list_sizze_uint8_t_to_uint8_t(uint8_t zcontent_len);

uint8_t zrlp_list_sizze_uint8_t_to_uint8_t_variant_2(uint8_t zcontent_len);

uint8_t zrlp_list_sizze_uint8_t_to_uint8_t_variant_3(uint8_t zcontent_len);

uint8_t zrlp_list_sizze_uint8_t_to_uint8_t_variant_4(uint8_t zcontent_len);

uint8_t zrlp_list_sizze_uint8_t_to_uint8_t_variant_5(uint8_t zcontent_len);

uint8_t zrlp_minimal_word_len_u128_to_uint8_t(u128 zw);

uint8_t zrlp_minimal_word_len_uint32_t_to_uint8_t(uint32_t zw);

uint8_t zrlp_minimal_word_len_uint64_t_to_uint8_t(uint64_t zw);

uint8_t zrlp_minimal_word_len_uint8_t_to_uint8_t(uint8_t zw);

uint8_t zrlp_minimal_word_len_uint8_t_to_uint8_t_variant_2(uint8_t zw);

uint32_t zrlp_scratch_length_add_uint16_t_uint32_t_to_uint32_t(uint16_t zleft, uint32_t zright);

uint32_t zrlp_scratch_length_add_uint16_t_uint8_t_to_uint32_t(uint16_t zleft, uint8_t zright);

uint32_t zrlp_scratch_length_add_uint32_t_uint16_t_to_uint32_t(uint32_t zleft, uint16_t zright);

uint32_t zrlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t(uint32_t zleft, uint32_t zright);

uint32_t zrlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_2(uint32_t zleft, uint32_t zright);

uint32_t zrlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(uint32_t zleft, uint32_t zright);

uint32_t zrlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(uint32_t zleft, uint8_t zright);

uint32_t zrlp_scratch_length_add_uint8_t_uint32_t_to_uint32_t(uint8_t zleft, uint32_t zright);

uint64_t zrlp_string_sizze_uint32_t_uint64_t_to_uint64_t(uint32_t zlen, uint64_t zfirst);

uint8_t zrlp_string_sizze_uint32_t_uint64_t_to_uint8_t(uint32_t zlen, uint64_t zfirst);

uint64_t zrlp_uint64_width_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t(struct zStatelessInputSliceFields zcontent, uint8_t zwidth);

uint64_t zrlp_uint64_width_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_2(struct zStatelessInputSliceFields zcontent, uint8_t zwidth);

uint64_t zrlp_uint64_width_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_3(struct zStatelessInputSliceFields zcontent, uint8_t zwidth);

uint8_t zrlp_uint_word_sizze_uint32_t_to_uint8_t(uint32_t zw);

uint8_t zrlp_uint_word_sizze_uint64_t_to_uint8_t(uint64_t zw);

uint8_t zrlp_uint_word_sizze_uint8_t_to_uint8_t(uint8_t zw);

unit zrlp_write_list_prefix_uint16_t_to_unit(uint16_t zcontent_len);

unit zrlp_write_list_prefix_uint16_t_to_unit_variant_2(uint16_t zcontent_len);

unit zrlp_write_list_prefix_uint32_t_to_unit(uint32_t zcontent_len);

unit zrlp_write_list_prefix_uint32_t_to_unit_variant_2(uint32_t zcontent_len);

unit zrlp_write_list_prefix_uint8_t_to_unit(uint8_t zcontent_len);

unit zrlp_write_list_prefix_uint8_t_to_unit_variant_2(uint8_t zcontent_len);

unit zrlp_write_list_prefix_uint8_t_to_unit_variant_3(uint8_t zcontent_len);

unit zrlp_write_list_prefix_uint8_t_to_unit_variant_4(uint8_t zcontent_len);

unit zrlp_write_list_prefix_uint8_t_to_unit_variant_5(uint8_t zcontent_len);

unit zrlp_write_string_prefix_uint16_t_uint64_t_to_unit(uint16_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint32_t_uint64_t_to_unit(uint32_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint8_t_uint64_t_to_unit(uint8_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(uint8_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_3(uint8_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_4(uint8_t zlen, uint64_t zfirst);

unit zrlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_5(uint8_t zlen, uint64_t zfirst);

unit zrlp_write_uint_word_u128_to_unit(u128 zw);

unit zrlp_write_uint_word_uint32_t_to_unit(uint32_t zw);

unit zrlp_write_uint_word_uint64_t_to_unit(uint64_t zw);

unit zrlp_write_uint_word_uint8_t_to_unit(uint8_t zw);

unit zrlp_write_uint_word_uint8_t_to_unit_variant_2(uint8_t zw);

void zrun_call(struct zFrameTransition *rop, uint32_t zcarried_pc, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_base, uint32_t zcarried_memory_height, fixed_bytes_20 zcarried_caller, fixed_bytes_20 zcarried_address, fixed_bytes_20 zcarried_code_address, u256 zcarried_value, uint64_t zcarried_state_gas_reservoir, bool zcarried_is_static, uint16_t zcarried_depth, struct zCodeFields zcarried_code, struct zCalldataSlice zcarried_calldata, struct zOutputSliceFields zcarried_returndata, enum zCallKind zkind);

void zrun_create(struct zFrameTransition *rop, uint32_t zcarried_pc, uint64_t zcarried_gas, uint64_t zcarried_state_gas, uint32_t zcarried_state_spill, __int128 zcarried_refund, struct zStackPointer zcarried_sp, uint32_t zcarried_memory_base, uint32_t zcarried_memory_height, fixed_bytes_20 zcarried_caller, fixed_bytes_20 zcarried_address, fixed_bytes_20 zcarried_code_address, u256 zcarried_value, uint64_t zcarried_state_gas_reservoir, bool zcarried_is_static, uint16_t zcarried_depth, struct zCodeFields zcarried_code, struct zCalldataSlice zcarried_calldata, struct zOutputSliceFields zcarried_returndata, enum zCreateKind zkind);

uint64_t zscratch_byte_struct_zzScratchSliceFields_uint8_t_to_uint64_t(struct zScratchSliceFields zs, uint8_t zoff);

uint32_t zscratch_field_offset_uint32_t_uint8_t_to_uint32_t(uint32_t zbase, uint8_t zdelta);

uint32_t zscratch_field_offset_uint8_t_uint8_t_to_uint32_t(uint8_t zbase, uint8_t zdelta);

uint32_t zscratch_length_add_uint8_t_uint32_t_to_uint32_t(uint8_t zleft, uint32_t zright);

uint32_t zscratch_length_add_uint8_t_uint32_t_to_uint32_t_variant_2(uint8_t zleft, uint32_t zright);

uint32_t zscratch_length_add_uint8_t_uint32_t_to_uint32_t_variant_3(uint8_t zleft, uint32_t zright);

unit zscratch_push_b256_fixed_bytes_32_uint8_t_to_unit(fixed_bytes_32 zdata, uint8_t zlen);

unit zscratch_push_word_be_u256_uint8_t_to_unit(u256 zdata, uint8_t zlen);

uint32_t zscratch_reserve_uint16_t_to_uint32_t(uint16_t zlen);

uint32_t zscratch_reserve_uint8_t_to_uint32_t(uint8_t zlen);

uint64_t zscratch_rlp_uint64_width_struct_zzScratchSliceFields_uint8_t_to_uint64_t(struct zScratchSliceFields zcontent, uint8_t zwidth);

uint64_t zscratch_rlp_uint64_width_struct_zzScratchSliceFields_uint8_t_to_uint64_t_variant_2(struct zScratchSliceFields zcontent, uint8_t zwidth);

u256 zscratch_slice_load(struct zScratchSliceFields zs, uint8_t zoff);

struct zScratchSliceFields zscratch_slice_suffix_struct_zzScratchSliceFields_uint8_t_to_struct_zzScratchSliceFields(struct zScratchSliceFields zs, uint8_t zoff);

struct zScratchSliceFields zscratch_slice_suffix_struct_zzScratchSliceFields_uint8_t_to_struct_zzScratchSliceFields_variant_2(struct zScratchSliceFields zs, uint8_t zoff);

struct zScratchSliceFields zscratch_sub_slice_struct_zzScratchSliceFields_uint8_t_uint32_t_to_struct_zzScratchSliceFields(struct zScratchSliceFields zs, uint8_t zoff, uint32_t zlen);

struct zScratchSliceFields zscratch_sub_slice_struct_zzScratchSliceFields_uint8_t_uint8_t_to_struct_zzScratchSliceFields(struct zScratchSliceFields zs, uint8_t zoff, uint8_t zlen);

fixed_bytes_32 zsha256_request_digest_uint64_t_struct_zzStatelessInputSliceFields_to_fixed_bytes_32(uint64_t zrequest_type, struct zStatelessInputSliceFields zs);

fixed_bytes_32 zsha256_request_digest_uint64_t_struct_zzStatelessInputSliceFields_to_fixed_bytes_32_variant_2(uint64_t zrequest_type, struct zStatelessInputSliceFields zs);

fixed_bytes_32 zsha256_request_digest_uint64_t_struct_zzStatelessInputSliceFields_to_fixed_bytes_32_variant_3(uint64_t zrequest_type, struct zStatelessInputSliceFields zs);

fixed_bytes_32 zsha256_request_digest_uint64_t_struct_zzStatelessInputSliceFields_to_fixed_bytes_32_variant_4(uint64_t zrequest_type, struct zStatelessInputSliceFields zs);

fixed_bytes_32 zsha256_request_digest_uint64_t_struct_zzStatelessInputSliceFields_to_fixed_bytes_32_variant_5(uint64_t zrequest_type, struct zStatelessInputSliceFields zs);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool_variant_2(struct zCalldataSlice zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint16_t_uint8_t_uint32_t_to_bool_variant_3(struct zCalldataSlice zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(struct zCalldataSlice zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zs, uint32_t zstart, uint32_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(struct zCalldataSlice zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool_variant_2(struct zCalldataSlice zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint16_t_uint8_t_uint32_t_to_bool_variant_3(struct zCalldataSlice zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(struct zCalldataSlice zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(struct zCalldataSlice zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(struct zCalldataSlice zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool_variant_2(struct zCalldataSlice zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

bool zslice_strided_zzero_struct_zzCalldataSlice_uint8_t_uint8_t_uint8_t_uint8_t_to_bool_variant_3(struct zCalldataSlice zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

fixed_bytes_20 zsszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_20(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_20 zsszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_20_variant_2(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_20 zsszz_addr_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_20_variant_3(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_to_struct_zzBoundedSszzzzListRef(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint16_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_to_struct_zzBoundedSszzzzListRef_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint16_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzBoundedSszzzzListRef(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint8_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzBoundedSszzzzListRef_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint8_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzBoundedSszzzzListRef_variant_3(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint8_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzBoundedSszzzzListRef_variant_4(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint8_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzBoundedSszzzzListRef_variant_5(struct zStatelessInputSliceFields zbytes, uint8_t zitem_sizze, uint8_t zmaximum_count);

struct zBoundedSszzListRef zsszz_bounded_variable_list_ref_struct_zzStatelessInputSliceFields_uint16_t_uint16_t_to_struct_zzBoundedSszzzzListRef(struct zStatelessInputSliceFields zbytes, uint16_t zmaximum_count, uint16_t zmaximum_item_length);

struct zBoundedSszzListRef zsszz_bounded_variable_list_ref_struct_zzStatelessInputSliceFields_uint32_t_uint16_t_to_struct_zzBoundedSszzzzListRef(struct zStatelessInputSliceFields zbytes, uint32_t zmaximum_count, uint16_t zmaximum_item_length);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint16_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_2(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_3(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_4(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_32 zsszz_bytes32_struct_zzStatelessInputSliceFields_uint8_t_to_fixed_bytes_32_variant_5(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

struct zStatelessInputSliceFields zsszz_container_bytes_struct_zzStatelessInputSliceFields_uint16_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zbytes, uint16_t zminimum);

struct zStatelessInputSliceFields zsszz_container_bytes_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zbytes, uint8_t zminimum);

struct zStatelessInputSliceFields zsszz_container_bytes_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zStatelessInputSliceFields zbytes, uint8_t zminimum);

struct zSszzContainerCursor zsszz_container_cursor_struct_zzStatelessInputSliceFields_uint16_t_to_struct_zzSszzzzContainerCursor(struct zStatelessInputSliceFields zbytes, uint16_t zfixed_length);

struct zSszzContainerCursor zsszz_container_cursor_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzSszzzzContainerCursor(struct zStatelessInputSliceFields zbytes, uint8_t zfixed_length);

uint32_t zsszz_field_offset_uint16_t_uint8_t_to_uint32_t(uint16_t zbase, uint8_t zdelta);

uint32_t zsszz_field_offset_uint32_t_uint8_t_to_uint32_t(uint32_t zbase, uint8_t zdelta);

uint16_t zsszz_field_offset_uint8_t_uint16_t_to_uint16_t(uint8_t zbase, uint16_t zdelta);

uint32_t zsszz_field_offset_uint8_t_uint8_t_to_uint32_t(uint8_t zbase, uint8_t zdelta);

uint8_t zsszz_field_offset_uint8_t_uint8_t_to_uint8_t(uint8_t zbase, uint8_t zdelta);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u128_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u128 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u128_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zBoundedSszzListRef zitems, u128 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u256_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u256 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u256_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zBoundedSszzListRef zitems, u256 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u320_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u320 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_u320_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zBoundedSszzListRef zitems, u320 zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_uint32_t_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint32_t zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_uint32_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zBoundedSszzListRef zitems, uint32_t zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_uint64_t_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint64_t zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_uint64_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zBoundedSszzListRef zitems, uint64_t zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint8_t zindex, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_fixed_list_at_struct_zzBoundedSszzzzListRef_sail_int_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, sail_int zindex, uint8_t zitem_sizze);

struct ztuple_z8z5structz0zzStatelessInputSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 zsszz_fixed_list_pop(struct zBoundedSszzListRef zitems, uint8_t zitem_sizze);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_u128_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u128 zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_u256_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u256 zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_u320_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, u320 zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_uint32_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint32_t zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_uint64_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint64_t zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_uint8_t_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, uint8_t zindex);

struct zStatelessInputSliceFields zsszz_list_at_struct_zzBoundedSszzzzListRef_sail_int_to_struct_zzStatelessInputSliceFields(struct zBoundedSszzListRef zitems, sail_int zindex);

u256 zsszz_u256(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_2(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_3(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_4(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_2(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_3(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_4(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_5(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_6(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_7(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_2(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_3(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint16_t_to_uint32_t_variant_4(struct zStatelessInputSliceFields zinput, uint16_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_2(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_3(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_4(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_5(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_6(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

uint32_t zsszz_u32_at_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t_variant_7(struct zStatelessInputSliceFields zinput, uint8_t zoffset);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_2(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_3(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_4(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_5(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_6(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_7(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_8(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_9(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_10(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_11(uint8_t zlevel);

fixed_bytes_32 zsszz_zzero_hash_uint8_t_to_fixed_bytes_32_variant_12(uint8_t zlevel);

unit zstack_set_struct_zzStackPointer_uint8_t_u256_to_unit(struct zStackPointer ztop, uint8_t zn, u256 zw);

unit zstack_set_struct_zzStackPointer_uint8_t_u256_to_unit_variant_2(struct zStackPointer ztop, uint8_t zn, u256 zw);

unit zstack_set_struct_zzStackPointer_uint8_t_u256_to_unit_variant_3(struct zStackPointer ztop, uint8_t zn, u256 zw);

u256 zstack_slot_read_struct_zzStackPointer_uint8_t_to_u256(struct zStackPointer ztop, uint8_t zindex);

u256 zstack_slot_read_struct_zzStackPointer_uint8_t_to_u256_variant_2(struct zStackPointer ztop, uint8_t zindex);

u256 zstack_slot_read_struct_zzStackPointer_uint8_t_to_u256_variant_3(struct zStackPointer ztop, uint8_t zindex);

u256 zstack_slot_read_struct_zzStackPointer_uint8_t_to_u256_variant_4(struct zStackPointer ztop, uint8_t zindex);

u256 zstack_slot_read_struct_zzStackPointer_uint8_t_to_u256_variant_5(struct zStackPointer ztop, uint8_t zindex);

unit zstack_slot_write_struct_zzStackPointer_uint8_t_u256_to_unit(struct zStackPointer ztop, uint8_t zindex, u256 zvalue);

struct zStackPointer zstack_top_advance(struct zStackPointer ztop, uint8_t zcount);

struct zStackPointer zstack_top_retreat(struct zStackPointer ztop, uint8_t zcount);

uint32_t zstate_gas_spill_add_uint32_t___int128_to_uint32_t(uint32_t zleft, __int128 zright);

uint32_t zstate_gas_spill_add_uint32_t_int64_t_to_uint32_t(uint32_t zleft, int64_t zright);

uint32_t zstate_gas_spill_add_uint32_t_int64_t_to_uint32_t_variant_2(uint32_t zleft, int64_t zright);

uint32_t zstate_gas_spill_add_uint32_t_uint32_t_to_uint32_t(uint32_t zleft, uint32_t zright);

uint32_t zstate_gas_spill_room_uint32_t_to_uint32_t(uint32_t zleft);

uint64_t zstateless_input_slice_byte_struct_zzStatelessInputSliceFields_uint16_t_to_uint64_t(struct zStatelessInputSliceFields zs, uint16_t zoff);

uint64_t zstateless_input_slice_byte_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t(struct zStatelessInputSliceFields zs, uint8_t zoff);

uint64_t zstateless_input_slice_byte_struct_zzStatelessInputSliceFields_uint8_t_to_uint64_t_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff);

u256 zstateless_input_slice_load_struct_zzStatelessInputSliceFields_uint16_t_to_u256(struct zStatelessInputSliceFields zs, uint16_t zoff);

u256 zstateless_input_slice_load_struct_zzStatelessInputSliceFields_uint8_t_to_u256(struct zStatelessInputSliceFields zs, uint8_t zoff);

u256 zstateless_input_slice_load_struct_zzStatelessInputSliceFields_uint8_t_to_u256_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff);

u256 zstateless_input_slice_load_n_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_u256(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zn);

u256 zstateless_input_slice_load_n_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_u256_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zn);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint32_t_uint16_t_uint8_t_uint32_t_to_bool(struct zStatelessInputSliceFields zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint32_t_uint16_t_uint8_t_uint8_t_to_bool(struct zStatelessInputSliceFields zs, uint32_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint32_t_uint32_t_uint8_t_uint32_t_to_bool(struct zStatelessInputSliceFields zs, uint32_t zstart, uint32_t zstride, uint8_t zwidth, uint32_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint32_t_uint8_t_uint8_t_uint32_t_to_bool(struct zStatelessInputSliceFields zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint32_t_uint8_t_uint8_t_uint8_t_to_bool(struct zStatelessInputSliceFields zs, uint32_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_uint8_t_uint32_t_to_bool(struct zStatelessInputSliceFields zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint32_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_uint8_t_uint8_t_to_bool(struct zStatelessInputSliceFields zs, uint8_t zstart, uint16_t zstride, uint8_t zwidth, uint8_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_uint8_t_uint32_t_to_bool(struct zStatelessInputSliceFields zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint32_t zcount);

bool zstateless_input_slice_strided_zzero_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_uint8_t_uint8_t_to_bool(struct zStatelessInputSliceFields zs, uint8_t zstart, uint8_t zstride, uint8_t zwidth, uint8_t zcount);

struct zStatelessInputSliceFields zstateless_input_slice_suffix_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint8_t zoff);

struct zStatelessInputSliceFields zstateless_input_slice_suffix_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff);

struct zStatelessInputSliceFields zstateless_input_slice_suffix_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields_variant_3(struct zStatelessInputSliceFields zs, uint8_t zoff);

struct zStatelessInputSliceFields zstateless_input_slice_suffix_struct_zzStatelessInputSliceFields_uint8_t_to_struct_zzStatelessInputSliceFields_variant_4(struct zStatelessInputSliceFields zs, uint8_t zoff);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint16_t_uint32_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint16_t zoff, uint32_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint16_t_uint8_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint16_t zoff, uint8_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint16_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint8_t zoff, uint16_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint32_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint8_t zoff, uint32_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint32_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff, uint32_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_3(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_4(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zlen);

struct zStatelessInputSliceFields zstateless_input_sub_slice_struct_zzStatelessInputSliceFields_uint8_t_uint8_t_to_struct_zzStatelessInputSliceFields_variant_5(struct zStatelessInputSliceFields zs, uint8_t zoff, uint8_t zlen);

void zsuspend_frame(struct ztuple_z8z5structz0zzFrameCheckpointzCz0z5structz0zzStackPointerzCz0z5u32zCz0z5u32z9 *rop, uint32_t zpc, uint64_t zgas_remaining, struct zStackPointer zstack_top, uint32_t zmemory_base, uint32_t zmemory_height, uint8_t zstate_gas_remaining, uint32_t zstate_gas_spilled, __int128 zframe_refund, struct zFrameStatus zframe_status, struct zMessage zmessage, struct zCodeFields zframe_code, struct zCalldataSlice zcalldata);

struct zTransactionGasAllowanceFields ztransaction_gas_allowance(uint64_t zvalue, uint64_t ztotal_limit, uint64_t zregular_limit);

struct zTransactionInitialGasFields ztransaction_initial_gas(struct zTransactionGasAllowanceFields zallowance, uint64_t zintrinsic_execution, uint64_t zintrinsic_state, uint64_t zcalldata_floor);

struct zTransactionInitialGasFields ztransaction_initial_gas_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_zzTransactionInitialGasFields(uint64_t ztotal, uint64_t zregular, uint64_t zintrinsic_execution, uint64_t zintrinsic_state, uint64_t zcalldata_floor, uint64_t zexecution, uint8_t zstate);

void ztrie_child_ref_struct_zzTrieItem_uint8_t_to_struct_zzNodeRef(struct zNodeRef *rop, struct zTrieItem zit, uint8_t zdepth);

void ztrie_child_ref_struct_zzTrieItem_uint8_t_to_struct_zzNodeRef_variant_2(struct zNodeRef *rop, struct zTrieItem zit, uint8_t zdepth);

struct zStatelessInputSliceFields ztrie_walk_struct_zzStatelessInputSliceFields_struct_zzTriePath_uint8_t_to_struct_zzStatelessInputSliceFields(struct zStatelessInputSliceFields znode, struct zTriePath zkey, uint8_t zpos);

struct zStatelessInputSliceFields ztrie_walk_struct_zzStatelessInputSliceFields_struct_zzTriePath_uint8_t_to_struct_zzStatelessInputSliceFields_variant_2(struct zStatelessInputSliceFields znode, struct zTriePath zkey, uint8_t zpos);

struct zTxFrameGasSnapshotFields ztx_frame_gas_snapshot_struct_zzTransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_zzTxFrameGasSnapshotFields(struct zTransactionInitialGasFields zinitial, uint64_t zexecution, uint64_t zstate, __int128 zstate_delta);

struct zTxFrameGasSnapshotFields ztx_frame_gas_snapshot_struct_zzTransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_zzTxFrameGasSnapshotFields(struct zTransactionInitialGasFields zinitial, uint8_t zexecution, uint8_t zstate, __int128 zstate_delta);

struct zTxFrameGasSnapshotFields ztx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_zzTxFrameGasSnapshotFields(uint64_t zlimit, uint64_t zregular, uint64_t zcalldata_floor, uint64_t zremaining, uint8_t zstate_used);

u256 zu256_u128_to_u256(u128 zvalue);

u256 zu256_uint16_t_to_u256(uint16_t zvalue);

u256 zu256_uint32_t_to_u256(uint32_t zvalue);

u256 zu256_uint64_t_to_u256(uint64_t zvalue);

u256 zu256_uint8_t_to_u256(uint8_t zvalue);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_2(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_3(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_4(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_5(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

void zupdates_subtree_struct_zzTrieUpdateCursor_struct_zzTriePath_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_6(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, uint8_t zcursor);

uint32_t zvalidate_auth_tuples_struct_zzStatelessInputSliceFields_uint32_t_to_uint32_t(struct zStatelessInputSliceFields zcursor, uint32_t zcount);

uint32_t zvalidate_auth_tuples_struct_zzStatelessInputSliceFields_uint32_t_to_uint32_t_variant_2(struct zStatelessInputSliceFields zcursor, uint32_t zcount);

uint32_t zvalidate_auth_tuples_struct_zzStatelessInputSliceFields_uint8_t_to_uint32_t(struct zStatelessInputSliceFields zcursor, uint8_t zcount);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint16_t_uint16_t_to_enum_zzStackValidation(struct zStackPointer ztop, uint16_t zinputs, uint16_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint16_t_uint8_t_to_enum_zzStackValidation(struct zStackPointer ztop, uint16_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation_variant_2(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation_variant_3(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation_variant_4(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation_variant_5(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

enum zStackValidation zvalidate_stack_struct_zzStackPointer_uint8_t_uint8_t_to_enum_zzStackValidation_variant_6(struct zStackPointer ztop, uint8_t zinputs, uint8_t zoutputs);

__int128 zvalidated_refund_add___int128_uint16_t_to___int128(__int128 zleft, uint16_t zright);

u256 zvalidated_word_product(u256 zvalue, uint64_t zfactor);

void zwitness_subtree_struct_zzStatelessInputSliceFields_struct_zzTriePath_struct_zzTrieUpdateCursor_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zStatelessInputSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, uint8_t zcursor);

void zwitness_subtree_struct_zzStatelessInputSliceFields_struct_zzTriePath_struct_zzTrieUpdateCursor_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_2(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zStatelessInputSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, uint8_t zcursor);

void zwitness_subtree_struct_zzStatelessInputSliceFields_struct_zzTriePath_struct_zzTrieUpdateCursor_uint8_t_to_struct_zztuple_zz8zz5structzz0zzzzTrieItemzzCzz0zz5structzz0zzzzTrieUpdateCursorzz9_variant_3(struct ztuple_z8z5structz0zzTrieItemzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zStatelessInputSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, uint8_t zcursor);

u256 zword_add_word_u256_u128_to_u256(u256 zleft, u128 zright);

u256 zword_add_word_u256_uint64_t_to_u256(u256 zleft, uint64_t zright);

u256 zword_add_word_u256_uint8_t_to_u256(u256 zleft, uint8_t zright);

uint16_t zword_bit_length_u128_to_uint16_t(u128 zvalue);

uint16_t zword_bit_length_uint32_t_to_uint16_t(uint32_t zvalue);

uint16_t zword_bit_length_uint64_t_to_uint16_t(uint64_t zvalue);

uint16_t zword_bit_length_uint8_t_to_uint16_t(uint8_t zvalue);

uint16_t zword_bit_length_uint8_t_to_uint16_t_variant_2(uint8_t zvalue);

uint8_t zword_byte_length_u128_to_uint8_t(u128 zvalue);

uint8_t zword_byte_length_uint32_t_to_uint8_t(uint32_t zvalue);

uint8_t zword_byte_length_uint64_t_to_uint8_t(uint64_t zvalue);

uint8_t zword_byte_length_uint8_t_to_uint8_t(uint8_t zvalue);

uint8_t zword_byte_length_uint8_t_to_uint8_t_variant_2(uint8_t zvalue);

u256 zword_div_word_u256_u256_to_u256(u256 zdividend, u256 zdivisor);

u256 zword_div_word_u256_u256_to_u256_variant_2(u256 zdividend, u256 zdivisor);

u256 zword_div_word_u256_uint8_t_to_u256(u256 zdividend, uint8_t zdivisor);

bool zword_is_zzero_u128_to_bool(u128 zw);

bool zword_is_zzero_uint64_t_to_bool(uint64_t zw);

uint64_t zword_low_byte_u128_to_uint64_t(u128 zvalue);

uint64_t zword_low_byte_uint32_t_to_uint64_t(uint32_t zvalue);

uint64_t zword_low_byte_uint64_t_to_uint64_t(uint64_t zvalue);

uint64_t zword_low_byte_uint8_t_to_uint64_t(uint8_t zvalue);

u256 zword_mod_word_u256_u256_to_u256(u256 zdividend, u256 zdivisor);

u256 zword_mod_word_u256_u256_to_u256_variant_2(u256 zdividend, u256 zdivisor);

u256 zword_mul_word_u256_uint16_t_to_u256(u256 za, uint16_t zb);

u256 zword_of_nat_byte_count_u128_to_u256(u128 zvalue);

u256 zword_of_nat_byte_count_uint16_t_to_u256(uint16_t zvalue);

u256 zword_of_nat_byte_count_uint16_t_to_u256_variant_2(uint16_t zvalue);

u256 zword_of_nat_byte_count_uint16_t_to_u256_variant_3(uint16_t zvalue);

u256 zword_of_nat_byte_count_uint16_t_to_u256_variant_4(uint16_t zvalue);

u256 zword_of_nat_byte_count_uint32_t_to_u256(uint32_t zvalue);

u256 zword_of_nat_byte_count_uint32_t_to_u256_variant_2(uint32_t zvalue);

u256 zword_of_nat_byte_count_uint32_t_to_u256_variant_3(uint32_t zvalue);

u256 zword_of_nat_byte_count_uint64_t_to_u256(uint64_t zvalue);

u256 zword_of_nat_byte_count_uint64_t_to_u256_variant_2(uint64_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_2(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_3(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_4(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_5(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_6(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_7(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_8(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_9(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_10(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_11(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_12(uint8_t zvalue);

u256 zword_of_nat_byte_count_uint8_t_to_u256_variant_13(uint8_t zvalue);

u256 zword_of_source_byte_count_uint16_t_to_u256(uint16_t zvalue);

u256 zword_of_source_byte_count_uint8_t_to_u256(uint8_t zvalue);

u256 zword_of_source_byte_count_uint8_t_to_u256_variant_2(uint8_t zvalue);

u256 zword_of_source_byte_count_uint8_t_to_u256_variant_3(uint8_t zvalue);

struct zGasCharge zword_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_zzGasCharge(uint8_t zper_unit, u256 zunits, uint64_t zavailable);

struct zGasCharge zword_scaled_gas_cost_uint8_t_u256_uint64_t_to_struct_zzGasCharge_variant_2(uint8_t zper_unit, u256 zunits, uint64_t zavailable);

u256 zword_shift_left_u256_uint8_t_to_u256(u256 zvalue, uint8_t zamount);

u256 zword_shift_right_u256_uint8_t_to_u256(u256 zvalue, uint8_t zamount);

u256 zword_sub_word_u256_uint8_t_to_u256(u256 zleft, uint8_t zright);

bool zword_ule_u256_u256_to_bool(u256 za, u256 zb);

bool zword_ule_uint8_t_u256_to_bool(uint8_t za, u256 zb);

bool zword_ult_u256_uint8_t_to_bool(u256 za, uint8_t zb);

extern struct zexception *current_exception;

extern bool have_exception;

extern sail_string *throw_location;

// register zscratch_arena
extern struct zScratchSliceFields zscratch_arena;

// register zk_parent_state_root
extern fixed_bytes_32 zk_parent_state_root;

// register zk_n_headers
extern uint16_t zk_n_headers;

// register zk_chain_id
extern uint64_t zk_chain_id;

// register zk_execution_profile
extern struct zExecutionProfileFields zk_execution_profile;

// register zk_header
extern struct zBlockHeader zk_header;

// register zk_tx
extern struct zTxEnvFields zk_tx;

// register zk_current_transaction_epoch
extern uint32_t zk_current_transaction_epoch;



#ifdef __cplusplus
}
#endif
