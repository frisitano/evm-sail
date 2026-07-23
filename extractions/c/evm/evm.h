#pragma once

#include "sail.h"
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

// type abbreviation y_parity
typedef uint64_t zy_parity;

// type abbreviation word_bit_count
typedef uint64_t zword_bit_count;


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } sail_u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } sail_u256;
#endif


// type abbreviation word
typedef sail_u256 zword;

// type abbreviation withdrawal_index
typedef uint64_t zwithdrawal_index;

// type abbreviation withdrawal_amount
typedef uint64_t zwithdrawal_amount;

// type abbreviation validator_index
typedef uint64_t zvalidator_index;

// type abbreviation trie_path_len
typedef uint64_t ztrie_path_len;

// type abbreviation trie_path_cursor
typedef uint64_t ztrie_path_cursor;

// type abbreviation trie_depth
typedef uint64_t ztrie_depth;

// type abbreviation transaction_state_gas_delta
typedef __int128 ztransaction_state_gas_delta;

// type abbreviation transaction_item_count
typedef uint64_t ztransaction_item_count;

// type abbreviation transaction_initcode_cost
typedef uint64_t ztransaction_initcode_cost;

// type abbreviation transaction_gas
typedef uint64_t ztransaction_gas;

// type abbreviation transaction_count
typedef uint64_t ztransaction_count;

// type abbreviation transaction_calldata_floor_cost
typedef uint64_t ztransaction_calldata_floor_cost;

// type abbreviation transaction_calldata_cost
typedef uint64_t ztransaction_calldata_cost;

// type abbreviation transaction_byte_length
typedef uint64_t ztransaction_byte_length;

// type abbreviation transaction_blob_gas
typedef uint64_t ztransaction_blob_gas;

// type abbreviation transaction_blob_count
typedef uint64_t ztransaction_blob_count;

// type abbreviation state_gas_spill
typedef uint64_t zstate_gas_spill;

// type abbreviation state_gas_delta
typedef __int128 zstate_gas_delta;

// type abbreviation stack_operation_index
typedef uint64_t zstack_operation_index;

// type abbreviation stack_index
typedef uint64_t zstack_index;

// type abbreviation ssz_uint
typedef uint64_t zsszz_uint;

// type abbreviation ssz_offset_index
typedef uint64_t zsszz_offset_index;

// type abbreviation ssz_offset
typedef uint64_t zsszz_offset;

// type abbreviation source_pointer
typedef uint64_t zsource_pointer;

// type abbreviation source_length
typedef uint64_t zsource_length;

// type abbreviation slot_number
typedef uint64_t zslot_number;

// type abbreviation rlp_scratch_length
typedef uint64_t zrlp_scratch_length;

// type abbreviation rlp_natural_size
typedef uint64_t zrlp_natural_sizze;

// type abbreviation rlp_index_byte_width
typedef uint64_t zrlp_index_byte_width;

// type abbreviation push_width
typedef uint64_t zpush_width;

// type abbreviation protocol_fork_index
typedef uint64_t zprotocol_fork_index;

// type abbreviation precompile_selector
typedef uint64_t zprecompile_selector;

// type abbreviation precompile_id
typedef uint64_t zprecompile_id;

// type abbreviation parent_header_field_cursor
typedef uint64_t zparent_header_field_cursor;

// union option<U64>
enum kind_zoptionzIU64zK { Kind_zNonezIU64zK, Kind_zSomezIU64zK };

struct zoptionzIU64zK {
  enum kind_zoptionzIU64zK kind;
  union {
    struct { unit zNonezIU64zK; };
    struct { uint64_t zSomezIU64zK; };
  } variants;
};

// union option<R__sail_c_repr_u256>
enum kind_zoptionzIR__sail_c_repr_u256zK { Kind_zNonezIR__sail_c_repr_u256zK, Kind_zSomezIR__sail_c_repr_u256zK };

struct zoptionzIR__sail_c_repr_u256zK {
  enum kind_zoptionzIR__sail_c_repr_u256zK kind;
  union {
    struct { unit zNonezIR__sail_c_repr_u256zK; };
    struct { sail_u256 zSomezIR__sail_c_repr_u256zK; };
  } variants;
};

// type abbreviation operand_stack_height
typedef uint64_t zoperand_stack_height;

// type abbreviation opcode
typedef uint64_t zopcode;

// type abbreviation nibble
typedef uint64_t znibble;

// type abbreviation modexp_product
typedef sail_u256 zmodexp_product;

// type abbreviation modexp_pre_osaka_extra
typedef uint64_t zmodexp_pre_osaka_extra;

// type abbreviation modexp_osaka_extra
typedef uint64_t zmodexp_osaka_extra;


#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } sail_u128;
#endif


// type abbreviation modexp_factor
typedef sail_u128 zmodexp_factor;

// type abbreviation memory_pointer
typedef uint64_t zmemory_pointer;

// type abbreviation memory_length
typedef uint64_t zmemory_length;

// type abbreviation log_topic_count
typedef uint64_t zlog_topic_count;

// type abbreviation linear_gas_variable_product
typedef sail_u128 zlinear_gas_variable_product;

// type abbreviation linear_gas_product
typedef sail_u128 zlinear_gas_product;

// type abbreviation journal_checkpoint
typedef uint64_t zjournal_checkpoint;

// type abbreviation item_index
typedef uint64_t zitem_index;

// type abbreviation item_count
typedef uint64_t zitem_count;

// type abbreviation htr_leaf_count
typedef uint64_t zhtr_leaf_count;

// type abbreviation htr_depth
typedef uint64_t zhtr_depth;

// type abbreviation host_access
typedef uint64_t zhost_access;

// type abbreviation hex_prefix_cursor
typedef uint64_t zhex_prefix_cursor;

#ifndef SAIL_FIXED_BYTES_32_DEFINED
#define SAIL_FIXED_BYTES_32_DEFINED
typedef struct { uint8_t bytes[32]; } sail_fixed_bytes_32;
#endif

// type abbreviation hash
typedef sail_fixed_bytes_32 zhash;

// type abbreviation gas_refund
typedef __int128 zgas_refund;

// type abbreviation gas_divisor
typedef uint64_t zgas_divisor;

// type abbreviation gas_cost
typedef uint64_t zgas_cost;

// type abbreviation gas_constant
typedef uint64_t zgas_constant;

// type abbreviation gas
typedef uint64_t zgas;

// type abbreviation frame_state_gas_delta
typedef __int128 zframe_state_gas_delta;

// type abbreviation frame_depth
typedef uint64_t zframe_depth;

// type abbreviation fake_exponential_index
typedef uint64_t zfake_exponential_index;

// type abbreviation excess_blob_gas
typedef uint64_t zexcess_blob_gas;

// type abbreviation deep_stack_index
typedef uint64_t zdeep_stack_index;

// type abbreviation code_pointer
typedef uint64_t zcode_pointer;

// type abbreviation code_length
typedef uint64_t zcode_length;

// type abbreviation code_chunk_index
typedef uint64_t zcode_chunk_index;

// type abbreviation chain_identifier
typedef uint64_t zchain_identifier;

// type abbreviation byte
typedef uint64_t zbyte;

// type abbreviation branch_content_length
typedef uint64_t zbranch_content_length;

// type abbreviation bls_msm_tail_product
typedef uint64_t zbls_msm_tail_product;

// type abbreviation bls_msm_product
typedef sail_u128 zbls_msm_product;

// type abbreviation bls_msm_coefficient
typedef uint64_t zbls_msm_coefficient;

// type abbreviation bls_discount
typedef uint64_t zbls_discount;

// type abbreviation bloom_bit_index
typedef uint64_t zbloom_bit_index;

// type abbreviation block_timestamp
typedef uint64_t zblock_timestamp;

// type abbreviation block_number
typedef uint64_t zblock_number;

// type abbreviation block_gas_limit
typedef uint64_t zblock_gas_limit;

// type abbreviation block_gas
typedef uint64_t zblock_gas;

// type abbreviation blob_target_count
typedef uint64_t zblob_target_count;

// type abbreviation blob_gas_used
typedef uint64_t zblob_gas_used;

// type abbreviation blob_fee_update_fraction
typedef uint64_t zblob_fee_update_fraction;

// type abbreviation blob_fee_remainder
typedef uint64_t zblob_fee_remainder;

// type abbreviation blob_count
typedef uint64_t zblob_count;

// type abbreviation blake2_rounds
typedef uint64_t zblake2_rounds;

// type abbreviation bit
typedef uint64_t zbit;

// type abbreviation bal_rlp_length
typedef uint64_t zbal_rlp_length;

// type abbreviation b256_index
typedef uint64_t zb256_index;

// type abbreviation b256
typedef sail_fixed_bytes_32 zb256;

// struct tuple_(%u64, %struct z__sail_c_repr_u256)
struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 {
  uint64_t ztup0;
  sail_u256 ztup1;
};

// union ast
enum kind_zast { Kind_zADD, Kind_zADDMOD, Kind_zADDRESS, Kind_zAND, Kind_zBALANCE, Kind_zBASEFEE, Kind_zBLOBBASEFEE, Kind_zBLOBHASH, Kind_zBLOCKHASH, Kind_zBYTE, Kind_zCALL, Kind_zCALLCODE, Kind_zCALLDATACOPY, Kind_zCALLDATALOAD, Kind_zCALLDATASIZE, Kind_zCALLER, Kind_zCALLVALUE, Kind_zCHAINID, Kind_zCLZ, Kind_zCODECOPY, Kind_zCODESIZE, Kind_zCOINBASE, Kind_zCREATE, Kind_zCREATE2, Kind_zDELEGATECALL, Kind_zDIV, Kind_zDUP, Kind_zDUPN, Kind_zEQ, Kind_zEXCHANGE, Kind_zEXP, Kind_zEXTCODECOPY, Kind_zEXTCODEHASH, Kind_zEXTCODESIZE, Kind_zGAS, Kind_zGASLIMIT, Kind_zGASPRICE, Kind_zGT, Kind_zINVALID, Kind_zISZERO, Kind_zJUMP, Kind_zJUMPDEST, Kind_zJUMPI, Kind_zKECCAK256, Kind_zLOG, Kind_zLT, Kind_zMCOPY, Kind_zMLOAD, Kind_zMOD, Kind_zMSIZE, Kind_zMSTORE, Kind_zMSTORE8, Kind_zMUL, Kind_zMULMOD, Kind_zNOT, Kind_zNUMBER, Kind_zOR, Kind_zORIGIN, Kind_zPC, Kind_zPOP, Kind_zPREVRANDAO, Kind_zPUSH, Kind_zRETURN, Kind_zRETURNDATACOPY, Kind_zRETURNDATASIZE, Kind_zREVERT, Kind_zSAR, Kind_zSDIV, Kind_zSELFBALANCE, Kind_zSELFDESTRUCT, Kind_zSGT, Kind_zSHL, Kind_zSHR, Kind_zSIGNEXTEND, Kind_zSLOAD, Kind_zSLOTNUM, Kind_zSLT, Kind_zSMOD, Kind_zSSTORE, Kind_zSTATICCALL, Kind_zSTOP, Kind_zSUB, Kind_zSWAP, Kind_zSWAPN, Kind_zTIMESTAMP, Kind_zTLOAD, Kind_zTSTORE, Kind_zXOR };

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
    struct { unit zCREATE; };
    struct { unit zCREATE2; };
    struct { unit zDELEGATECALL; };
    struct { unit zDIV; };
    struct { uint64_t zDUP; };
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
    struct { uint64_t zLOG; };
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
    struct { struct ztuple_z8z5u64zCz0z5structz0zz__sail_c_repr_u256z9 zPUSH; };
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
    struct { uint64_t zSWAP; };
    struct { uint64_t zSWAPN; };
    struct { unit zTIMESTAMP; };
    struct { unit zTLOAD; };
    struct { unit zTSTORE; };
    struct { unit zXOR; };
  } variants;
};

// type abbreviation ancestor_index
typedef uint64_t zancestor_index;

// type abbreviation amsterdam_recipient_cost
typedef uint64_t zamsterdam_recipient_cost;

#ifndef SAIL_FIXED_BYTES_20_DEFINED
#define SAIL_FIXED_BYTES_20_DEFINED
typedef struct { uint8_t bytes[20]; } sail_fixed_bytes_20;
#endif

// type abbreviation address
typedef sail_fixed_bytes_20 zaddress;

// type abbreviation account_nonce
typedef uint64_t zaccount_nonce;

// struct WitnessContext
struct zWitnessContext {
  sail_u256 zparent_base_fee_per_gas;
  uint64_t zparent_blob_gas_used;
  uint64_t zparent_excess_blob_gas;
  sail_fixed_bytes_32 zparent_hash;
  sail_fixed_bytes_32 zparent_state_root;
};

// struct Withdrawal
struct zWithdrawal {
  sail_fixed_bytes_20 zaddress;
  uint64_t zamount;
  uint64_t zindex;
  uint64_t zvalidator_index;
};

// struct TxValidity
struct zTxValidity {
  sail_u256 zblob_fee;
  uint64_t zcalldata_floor;
  uint64_t zgas_limit;
  sail_u256 zgas_price;
  uint64_t zintrinsic_execution_gas;
  uint64_t zintrinsic_state_gas;
  uint64_t znonce_before;
  sail_u256 zpriority_fee;
  sail_fixed_bytes_20 zsender;
};

// struct TxUpfrontResult
struct zTxUpfrontResult {
  __int128 zauthorizzation_refund;
  bool zcreate_target_prestate_empty;
};

// enum TxType
enum zTxType { zLegacyTx, zAccessListTx, zFeeMarketTx, zBlobTx, zSetCodeTx };

// struct TxFrameResult
struct zTxFrameResult {
  uint64_t zexecution_gas_remaining;
  __int128 zrefund;
  uint64_t zstate_gas_remaining;
  __int128 zstate_gas_used;
  bool zsuccess;
};

// union TrieUpdateSource
enum kind_zTrieUpdateSource { Kind_zCachedAccountTrieUpdates, Kind_zChangedAccountTrieUpdates, Kind_zStorageTrieUpdates };

struct zTrieUpdateSource {
  enum kind_zTrieUpdateSource kind;
  union {
    struct { unit zCachedAccountTrieUpdates; };
    struct { unit zChangedAccountTrieUpdates; };
    struct { sail_fixed_bytes_20 zStorageTrieUpdates; };
  } variants;
};

// struct TrieRootResult
struct zTrieRootResult {
  bool zchanged;
  sail_fixed_bytes_32 zroot;
};

// struct TriePath
struct zTriePath {
  sail_fixed_bytes_32 zdata;
  uint64_t zlen;
};

// union option<RTriePath>
enum kind_zoptionzIRTriePathzK { Kind_zNonezIRTriePathzK, Kind_zSomezIRTriePathzK };

struct zoptionzIRTriePathzK {
  enum kind_zoptionzIRTriePathzK kind;
  union {
    struct { unit zNonezIRTriePathzK; };
    struct { struct zTriePath zSomezIRTriePathzK; };
  } variants;
};

// struct TransactionCosts
struct zTransactionCosts {
  sail_u256 zblob_fee;
  uint64_t zblob_gas;
  uint64_t zcalldata_floor;
  uint64_t zintrinsic_execution;
  uint64_t zintrinsic_state;
  sail_u256 zupfront;
};

// struct StorageValue
struct zStorageValue {
  sail_u256 zcurr;
  sail_u256 zorig;
};

// union option<RStorageValue>
enum kind_zoptionzIRStorageValuezK { Kind_zNonezIRStorageValuezK, Kind_zSomezIRStorageValuezK };

struct zoptionzIRStorageValuezK {
  enum kind_zoptionzIRStorageValuezK kind;
  union {
    struct { unit zNonezIRStorageValuezK; };
    struct { struct zStorageValue zSomezIRStorageValuezK; };
  } variants;
};

// struct StorageKey
struct zStorageKey {
  sail_fixed_bytes_20 zaddr;
  sail_u256 zslot;
};

// struct StorageEntry
struct zStorageEntry {
  struct zStorageKey zkey;
  struct zStorageValue zvalue;
};

// union option<RStorageEntry>
enum kind_zoptionzIRStorageEntryzK { Kind_zNonezIRStorageEntryzK, Kind_zSomezIRStorageEntryzK };

struct zoptionzIRStorageEntryzK {
  enum kind_zoptionzIRStorageEntryzK kind;
  union {
    struct { unit zNonezIRStorageEntryzK; };
    struct { struct zStorageEntry zSomezIRStorageEntryzK; };
  } variants;
};

// struct SstoreCosts
struct zSstoreCosts {
  uint64_t zexecution;
  __int128 zrefund;
  uint64_t zstate_charge;
  uint64_t zstate_credit;
};

// struct ScaledBlobValue
struct zScaledBlobValue {
  uint64_t zremainder;
  sail_u256 zwhole;
};

// struct RlpIndexItem
struct zRlpIndexItem {
  uint64_t zindex;
  struct zTriePath zkey;
  struct zoptionzIRTriePathzK znext_key;
};

// struct RlpIndexCursor
struct zRlpIndexCursor {
  uint64_t zcount;
  uint64_t zposition;
};

// struct ParentHeaderFields
struct zParentHeaderFields {
  sail_u256 zbase_fee;
  uint64_t zblob_gas_used;
  uint64_t zexcess_blob_gas;
  bool zhave_base_fee;
  bool zhave_blob_gas;
  bool zhave_excess_blob_gas;
  bool zhave_parent;
  bool zhave_state;
  sail_fixed_bytes_32 zparent_hash;
  sail_fixed_bytes_32 zstate_root;
};

// struct Message
struct zMessage {
  sail_fixed_bytes_20 zaddress;
  sail_fixed_bytes_20 zcaller;
  sail_fixed_bytes_20 zcode_address;
  uint64_t zdepth;
  bool zis_static;
  uint64_t zstate_gas_reservoir;
  sail_u256 zvalue;
};

// union MerkleSlot
enum kind_zMerkleSlot { Kind_zEmptyMerkleSlot, Kind_zOccupiedMerkleSlot };

struct zMerkleSlot {
  enum kind_zMerkleSlot kind;
  union {
    struct { unit zEmptyMerkleSlot; };
    struct { sail_fixed_bytes_32 zOccupiedMerkleSlot; };
  } variants;
};

struct node_zz5listz8z5unionz0zzMerkleSlotz9 {
  unsigned int rc;
  struct zMerkleSlot hd;
  struct node_zz5listz8z5unionz0zzMerkleSlotz9 *tl;
};
typedef struct node_zz5listz8z5unionz0zzMerkleSlotz9 *zz5listz8z5unionz0zzMerkleSlotz9;

// struct MerkleAccumulator
struct zMerkleAccumulator {
  uint64_t zcount;
  uint64_t zdepth;
  zz5listz8z5unionz0zzMerkleSlotz9 zfrontier;
};

// struct MemoryRangeFields
struct zMemoryRangeFields {
  uint64_t zlen;
  uint64_t zoff;
};

// type abbreviation MemoryRange
typedef struct zMemoryRangeFields zMemoryRange;

// struct MemoryPairExpansion
struct zMemoryPairExpansion {
  uint64_t zcost;
  struct zMemoryRangeFields zleft;
  uint64_t zrequired_sizze;
  struct zMemoryRangeFields zright;
};

// struct MemoryExpansion
struct zMemoryExpansion {
  uint64_t zcost;
  struct zMemoryRangeFields zrange;
  uint64_t zrequired_sizze;
};

struct node_zz5listz8z5bv8z9 {
  unsigned int rc;
  uint64_t hd;
  struct node_zz5listz8z5bv8z9 *tl;
};
typedef struct node_zz5listz8z5bv8z9 *zz5listz8z5bv8z9;

// struct MaterializedBytes
struct zMaterializzedBytes {
  zz5listz8z5bv8z9 zdata;
  uint64_t zlen;
};

#ifndef SAIL_FIXED_BYTES_256_DEFINED
#define SAIL_FIXED_BYTES_256_DEFINED
typedef struct { uint8_t bytes[256]; } sail_fixed_bytes_256;
#endif

// type abbreviation LogsBloom
typedef sail_fixed_bytes_256 zLogsBloom;

// type abbreviation JumpdestRef
typedef uint64_t zJumpdestRef;

// type abbreviation JumpdestChunk
typedef sail_u256 zJumpdestChunk;

// struct IntrinsicGasCost
struct zIntrinsicGasCost {
  uint64_t zcalldata_floor;
  uint64_t zexecution;
  uint64_t zstate;
};

// struct InlineNode
struct zInlineNode {
  sail_fixed_bytes_32 zdata;
  uint64_t zlen;
};

// union NodeRef
enum kind_zNodeRef { Kind_zEmptyRef, Kind_zHashRef, Kind_zInlineRef };

struct zNodeRef {
  enum kind_zNodeRef kind;
  union {
    struct { unit zEmptyRef; };
    struct { sail_fixed_bytes_32 zHashRef; };
    struct { struct zInlineNode zInlineRef; };
  } variants;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5UNIONZ0ZZNODEREFZ9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5UNIONZ0ZZNODEREFZ9_DEFINED
struct zz5vecz8z5unionz0zzNodeRefz9 {
  size_t len;
  struct zNodeRef *data;
};
typedef struct zz5vecz8z5unionz0zzNodeRefz9 zz5vecz8z5unionz0zzNodeRefz9;
#endif

// struct TrieBranchFrame
struct zTrieBranchFrame {
  zz5vecz8z5unionz0zzNodeRefz9 zchildren;
  uint64_t zdepth;
  uint64_t zmask;
};

struct node_zz5listz8z5structz0zzTrieBranchFramez9 {
  unsigned int rc;
  struct zTrieBranchFrame hd;
  struct node_zz5listz8z5structz0zzTrieBranchFramez9 *tl;
};
typedef struct node_zz5listz8z5structz0zzTrieBranchFramez9 *zz5listz8z5structz0zzTrieBranchFramez9;

// struct TrieBuilder
struct zTrieBuilder {
  bool zcomplete;
  zz5listz8z5structz0zzTrieBranchFramez9 zframes;
  struct zNodeRef zroot;
};

// enum Fork
enum zFork { zFrontier, zHomestead, zByzzantium, zConstantinople, zIstanbul, zBerlin, zLondon, zParis, zShanghai, zCancun, zPrague, zOsaka, zAmsterdam };

// struct FixedBytes32
struct zFixedBytes32 {
  sail_fixed_bytes_32 zdata;
  uint64_t zlen;
};

// enum ExceptionKind
enum zExceptionKind { zStackUnderflow, zStackOverflow, zOutOfGas, zInvalidOpcode, zInvalidJump, zStaticViolation, zCallDepthExceeded, zInsufficientBalance, zWriteProtection, zInitCodeTooLarge, zNonceOverflow, zAddressCollision };

// enum EnvField
enum zEnvField { zF_Number, zF_Timestamp, zF_Coinbase, zF_BaseFee, zF_ChainId, zF_GasLimit, zF_PrevRandao, zF_Origin, zF_GasPrice, zF_SlotNumber };

// struct CodeAnalysis
struct zCodeAnalysis {
  sail_u256 zchunk;
  uint64_t zchunk_index;
  uint64_t zchunk_offset;
};

// struct ChainConfig
struct zChainConfig {uint64_t zchain_id;};

// enum CallKind
enum zCallKind { zCall, zCallCode, zDelegateCall, zStaticCall };

// enum ByteSource
enum zByteSource { zStatelessInputSource, zEvmMemorySource, zCodeSource, zLogDataSource, zOutputSource, zScratchSource };

// struct ByteSliceFields
struct zByteSliceFields {
  uint64_t zlen;
  uint64_t zoff;
  enum zByteSource zsource;
};

// union TrieItemValue
enum kind_zTrieItemValue { Kind_zBranchItem, Kind_zLeafItem, Kind_zSubtreeItem };

struct zTrieItemValue {
  enum kind_zTrieItemValue kind;
  union {
    struct { struct zNodeRef zBranchItem; };
    struct { struct zByteSliceFields zLeafItem; };
    struct { struct zNodeRef zSubtreeItem; };
  } variants;
};

// struct TrieItem
struct zTrieItem {
  struct zTriePath zpath;
  struct zTrieItemValue zvalue;
};

// union option<RTrieItem>
enum kind_zoptionzIRTrieItemzK { Kind_zNonezIRTrieItemzK, Kind_zSomezIRTrieItemzK };

struct zoptionzIRTrieItemzK {
  enum kind_zoptionzIRTrieItemzK kind;
  union {
    struct { unit zNonezIRTrieItemzK; };
    struct { struct zTrieItem zSomezIRTrieItemzK; };
  } variants;
};

// struct TrieItemSink
struct zTrieItemSink {
  struct zTrieBuilder zbuilder;
  struct zoptionzIRTrieItemzK zpending;
};

// union TrieChange
enum kind_zTrieChange { Kind_zTrieDelete, Kind_zTriePut };

struct zTrieChange {
  enum kind_zTrieChange kind;
  union {
    struct { unit zTrieDelete; };
    struct { struct zByteSliceFields zTriePut; };
  } variants;
};

// struct TrieUpdate
struct zTrieUpdate {
  struct zTrieChange zchange;
  struct zTriePath zkey;
};

// union option<RTrieUpdate>
enum kind_zoptionzIRTrieUpdatezK { Kind_zNonezIRTrieUpdatezK, Kind_zSomezIRTrieUpdatezK };

struct zoptionzIRTrieUpdatezK {
  enum kind_zoptionzIRTrieUpdatezK kind;
  union {
    struct { unit zNonezIRTrieUpdatezK; };
    struct { struct zTrieUpdate zSomezIRTrieUpdatezK; };
  } variants;
};

// struct TrieUpdateCursor
struct zTrieUpdateCursor {
  struct zoptionzIRTrieUpdatezK zpending;
  struct zTrieUpdateSource zsource;
};

// type abbreviation TransactionByteSlice
typedef struct zByteSliceFields zTransactionByteSlice;

// struct SszContainerCursor
struct zSszzContainerCursor {
  struct zByteSliceFields zbytes;
  uint64_t zcurrent;
};

// struct RlpFieldRefFields
struct zRlpFieldRefFields {
  uint64_t zcontent_len;
  uint64_t zcontent_off;
  uint64_t zfull_len;
  uint64_t zfull_off;
  bool zis_list;
  struct zByteSliceFields zsource;
};

// type abbreviation RlpFieldRef
typedef struct zRlpFieldRefFields zRlpFieldRef;

// struct LeafNodeData
struct zLeafNodeData {
  struct zTriePath zpath;
  struct zRlpFieldRefFields zvalue;
};

// struct ExtensionNodeData
struct zExtensionNodeData {
  struct zRlpFieldRefFields zchild;
  struct zTriePath zpath;
};

// struct RlpCursorFields
struct zRlpCursorFields {
  uint64_t zcurrent;
  struct zByteSliceFields zsource;
  uint64_t zstop;
  bool zvalid;
};

// type abbreviation RlpCursor
typedef struct zRlpCursorFields zRlpCursor;

// struct PrecompileResult
struct zPrecompileResult {
  struct zByteSliceFields zoutput;
  bool zsuccess;
};

struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9 {
  unsigned int rc;
  sail_u256 hd;
  struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9 *tl;
};
typedef struct node_zz5listz8z5structz0zz__sail_c_repr_u256z9 *zz5listz8z5structz0zz__sail_c_repr_u256z9;

// struct LogEntry
struct zLogEntry {
  sail_fixed_bytes_20 zaddress;
  struct zByteSliceFields zdata;
  zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics;
};

struct node_zz5listz8z5structz0zzLogEntryz9 {
  unsigned int rc;
  struct zLogEntry hd;
  struct node_zz5listz8z5structz0zzLogEntryz9 *tl;
};
typedef struct node_zz5listz8z5structz0zzLogEntryz9 *zz5listz8z5structz0zzLogEntryz9;

// struct Receipt
struct zReceipt {
  uint64_t zexecution_gas;
  uint64_t zgas_used;
  zz5listz8z5structz0zzLogEntryz9 zlogs;
  uint64_t zstate_gas;
  bool zsuccess;
  enum zTxType ztx_type;
};

// struct PendingReceipt
struct zPendingReceipt {
  uint64_t zcumulative_gas_used;
  uint64_t zindex;
  struct zReceipt zreceipt;
};

// union option<RPendingReceipt>
enum kind_zoptionzIRPendingReceiptzK { Kind_zNonezIRPendingReceiptzK, Kind_zSomezIRPendingReceiptzK };

struct zoptionzIRPendingReceiptzK {
  enum kind_zoptionzIRPendingReceiptzK kind;
  union {
    struct { unit zNonezIRPendingReceiptzK; };
    struct { struct zPendingReceipt zSomezIRPendingReceiptzK; };
  } variants;
};

// struct ReceiptAccumulator
struct zReceiptAccumulator {
  sail_fixed_bytes_256 zbloom;
  struct zTrieBuilder zbuilder;
  uint64_t zcount;
  uint64_t zcumulative_gas_used;
  struct zoptionzIRPendingReceiptzK zfirst;
  struct zoptionzIRPendingReceiptzK zpending;
};

// union HaltKind
enum kind_zHaltKind { Kind_zHaltReturn, Kind_zHaltRevert, Kind_zHaltSelfDestruct, Kind_zHaltStop };

struct zHaltKind {
  enum kind_zHaltKind kind;
  union {
    struct { struct zByteSliceFields zHaltReturn; };
    struct { struct zByteSliceFields zHaltRevert; };
    struct { unit zHaltSelfDestruct; };
    struct { unit zHaltStop; };
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

// struct ExecutionRequests
struct zExecutionRequests {
  struct zByteSliceFields zbuilder_deposits;
  struct zByteSliceFields zbuilder_exits;
  struct zByteSliceFields zconsolidations;
  struct zByteSliceFields zdeposits;
  struct zByteSliceFields zwithdrawals;
};

// struct EncodedBlockAccessList
struct zEncodedBlockAccessList {
  struct zByteSliceFields zbytes;
  uint64_t zitem_count;
};

// type abbreviation CodeSlice
typedef struct zByteSliceFields zCodeSlice;

// struct Code
struct zCode {
  struct zByteSliceFields zbytes;
  uint64_t zjumpdests;
};

// union option<RCode>
enum kind_zoptionzIRCodezK { Kind_zNonezIRCodezK, Kind_zSomezIRCodezK };

struct zoptionzIRCodezK {
  enum kind_zoptionzIRCodezK kind;
  union {
    struct { unit zNonezIRCodezK; };
    struct { struct zCode zSomezIRCodezK; };
  } variants;
};

// struct FrameCheckpoint
struct zFrameCheckpoint {
  uint64_t zcall_depth;
  struct zByteSliceFields zcalldata;
  struct zCode zcode;
  uint64_t zgas_remaining;
  struct zByteSliceFields zmemory;
  struct zMessage zmessage;
  uint64_t zpc;
  __int128 zrefund;
  uint64_t zstate;
  uint64_t zstate_gas_remaining;
  uint64_t zstate_gas_spilled;
  struct zFrameStatus zstatus;
};

// struct CreateContinuation
struct zCreateContinuation {
  sail_fixed_bytes_20 zaddress;
  struct zFrameCheckpoint zcheckpoint;
  bool znew_account_charged;
};

// struct CallContinuation
struct zCallContinuation {
  struct zFrameCheckpoint zcheckpoint;
  bool znew_account_charged;
  uint64_t zreturn_length;
  uint64_t zreturn_offset;
};

// union FrameContinuation
enum kind_zFrameContinuation { Kind_zResumeCall, Kind_zResumeCreate };

struct zFrameContinuation {
  enum kind_zFrameContinuation kind;
  union {
    struct { struct zCallContinuation zResumeCall; };
    struct { struct zCreateContinuation zResumeCreate; };
  } variants;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5UNIONZ0ZZFRAMECONTINUATIONZ9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5UNIONZ0ZZFRAMECONTINUATIONZ9_DEFINED
struct zz5vecz8z5unionz0zzFrameContinuationz9 {
  size_t len;
  struct zFrameContinuation *data;
};
typedef struct zz5vecz8z5unionz0zzFrameContinuationz9 zz5vecz8z5unionz0zzFrameContinuationz9;
#endif

// type abbreviation FrameStack
typedef zz5vecz8z5unionz0zzFrameContinuationz9 zFrameStack;

// union Bytes
enum kind_zBytes { Kind_zBytesFixed32, Kind_zBytesList, Kind_zBytesSlice };

struct zBytes {
  enum kind_zBytes kind;
  union {
    struct { struct zFixedBytes32 zBytesFixed32; };
    struct { struct zMaterializzedBytes zBytesList; };
    struct { struct zByteSliceFields zBytesSlice; };
  } variants;
};

// type abbreviation ByteSliceLength
typedef struct zByteSliceFields zByteSliceLength;

// type abbreviation ByteSliceAtLeast
typedef struct zByteSliceFields zByteSliceAtLeast;

// type abbreviation ByteSlice
typedef struct zByteSliceFields zByteSlice;

// union ByteRegionResult
enum kind_zByteRegionResult { Kind_zByteRegionFailed, Kind_zByteRegionReady };

struct zByteRegionResult {
  enum kind_zByteRegionResult kind;
  union {
    struct { unit zByteRegionFailed; };
    struct { struct zByteSliceFields zByteRegionReady; };
  } variants;
};

// type abbreviation BranchRefs
typedef zz5vecz8z5unionz0zzNodeRefz9 zBranchRefs;

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5STRUCTZ0ZZRLPFIELDREFFIELDSZ9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5STRUCTZ0ZZRLPFIELDREFFIELDSZ9_DEFINED
struct zz5vecz8z5structz0zzRlpFieldRefFieldsz9 {
  size_t len;
  struct zRlpFieldRefFields *data;
};
typedef struct zz5vecz8z5structz0zzRlpFieldRefFieldsz9 zz5vecz8z5structz0zzRlpFieldRefFieldsz9;
#endif

// struct BranchNodeData
struct zBranchNodeData {
  zz5vecz8z5structz0zzRlpFieldRefFieldsz9 zchildren;
  struct zRlpFieldRefFields zvalue;
};

// union TrieNode
enum kind_zTrieNode { Kind_zBranchNode, Kind_zExtensionNode, Kind_zInvalidNode, Kind_zLeafNode };

struct zTrieNode {
  enum kind_zTrieNode kind;
  union {
    struct { struct zBranchNodeData zBranchNode; };
    struct { struct zExtensionNodeData zExtensionNode; };
    struct { unit zInvalidNode; };
    struct { struct zLeafNodeData zLeafNode; };
  } variants;
};

// type abbreviation BranchChildren
typedef zz5vecz8z5structz0zzRlpFieldRefFieldsz9 zBranchChildren;

// struct BoundedSszListRef
struct zBoundedSszzListRef {
  struct zByteSliceFields zbytes;
  uint64_t zcount;
  uint64_t zmax_item_length;
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

// struct BoundedSszListCursor
struct zBoundedSszzListCursor {
  uint64_t zcurrent;
  uint64_t zindex;
  struct zBoundedSszzListRef zitems;
};

// type abbreviation WitnessNodeListCursor
typedef struct zBoundedSszzListCursor zWitnessNodeListCursor;

// type abbreviation WitnessHeaderListCursor
typedef struct zBoundedSszzListCursor zWitnessHeaderListCursor;

// struct WitnessHeaderIndex
struct zWitnessHeaderIndex {
  struct zBoundedSszzListCursor zcursor;
  sail_u256 zparent_base_fee_per_gas;
  uint64_t zparent_blob_gas_used;
  uint64_t zparent_excess_blob_gas;
  bool zparent_fields_valid;
  sail_fixed_bytes_32 zparent_state_root;
  sail_fixed_bytes_32 zprevious_hash;
  bool zvalid;
};

// type abbreviation WitnessCodeListCursor
typedef struct zBoundedSszzListCursor zWitnessCodeListCursor;

// struct BlockHeader
struct zBlockHeader {
  sail_u256 zbase_fee;
  uint64_t zblob_gas_used;
  uint64_t zexcess_blob_gas;
  struct zByteSliceFields zextra_data;
  sail_fixed_bytes_20 zfee_recipient;
  uint64_t zgas_limit;
  uint64_t zgas_used;
  sail_fixed_bytes_256 zlogs_bloom;
  uint64_t znumber;
  sail_fixed_bytes_32 zparent_beacon_block_root;
  sail_fixed_bytes_32 zparent_hash;
  sail_u256 zprev_randao;
  sail_fixed_bytes_32 zreceipts_root;
  uint64_t zslot_number;
  sail_fixed_bytes_32 zstate_root;
  uint64_t ztimestamp;
};

// struct BlockExecutionResult
struct zBlockExecutionResult {
  uint64_t zblob_gas_used;
  struct zByteSliceFields zdeposits;
  uint64_t zexecution_gas_used;
  sail_fixed_bytes_20 zfirst_tx_recipient;
  uint64_t zheader_gas_used;
  sail_fixed_bytes_256 zlogs_bloom;
  sail_fixed_bytes_32 zreceipts_root;
  struct zExecutionRequests zrequests;
  uint64_t zstate_gas_used;
};

// enum BlockError
enum zBlockError { zInvalidConfig, zHeaderChainBroken, zRlpDecode, zInvalidSignature, zInvalidGasLimit, zGasUsedExceedsLimit, zBlobGasLimitExceeded, zExecutionInvalid, zInvalidGasUsed, zInvalidBlobGasUsed, zInvalidExcessBlobGas, zInvalidStateRoot, zInvalidReceiptsRoot, zInvalidLogsBloom, zInvalidBlockHash, zInvalidParentHash, zBlockAccessListTooLarge, zInvalidBlockAccessList, zInvalidExecutionRequests, zWitnessDeficient };

// union exception
enum kind_zexception { Kind_zInvalidBlock };

struct zexception {
  enum kind_zexception kind;
  union {struct { enum zBlockError zInvalidBlock; };} variants;
};

// struct StatelessValidationFailure
struct zStatelessValidationFailure {
  enum zBlockError zreason;
  uint64_t zscope;
};

// union StatelessValidationResult
enum kind_zStatelessValidationResult { Kind_zStatelessPayloadInvalid, Kind_zStatelessPayloadValid };

struct zStatelessValidationResult {
  enum kind_zStatelessValidationResult kind;
  union {
    struct { struct zStatelessValidationFailure zStatelessPayloadInvalid; };
    struct { unit zStatelessPayloadValid; };
  } variants;
};

// struct BlockBody
struct zBlockBody {
  struct zByteSliceFields zblock_access_list;
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
  sail_fixed_bytes_32 zexpected_block_hash;
};

// struct StatelessInput
struct zStatelessInput {
  struct zChainConfig zchain_config;
  struct zExecutionPayload zpayload;
};

// struct BlobSchedule
struct zBlobSchedule {
  uint64_t zbase_fee_update_fraction;
  uint64_t zmax;
  uint64_t ztarget;
};

// struct ProtocolProfile
struct zProtocolProfile {
  struct zBlobSchedule zblob_schedule;
  enum zFork zfork;
};

// struct StatelessInputRef
struct zStatelessInputRef {
  struct zByteSliceFields zblock_access_list;
  struct zByteSliceFields zbuilder_deposit_requests;
  struct zByteSliceFields zbuilder_exit_requests;
  struct zByteSliceFields zchain_config;
  struct zByteSliceFields zconsolidation_requests;
  struct zByteSliceFields zdeposits;
  struct zByteSliceFields zexecution_payload;
  struct zByteSliceFields zextra_data;
  struct zByteSliceFields znew_payload_request;
  struct zProtocolProfile zprotocol;
  struct zByteSliceFields zpublic_keys;
  struct zBoundedSszzListRef ztransactions;
  struct zByteSliceFields zversioned_hashes;
  struct zByteSliceFields zwithdrawal_requests;
  struct zBoundedSszzListRef zwithdrawals;
  struct zBoundedSszzListRef zwitness_codes;
  struct zBoundedSszzListRef zwitness_headers;
  struct zBoundedSszzListRef zwitness_state;
};

// struct GuestValidation
struct zGuestValidation {
  struct zStatelessInputRef zinput_ref;
  bool zvalid;
};

// union option<RGuestValidation>
enum kind_zoptionzIRGuestValidationzK { Kind_zNonezIRGuestValidationzK, Kind_zSomezIRGuestValidationzK };

struct zoptionzIRGuestValidationzK {
  enum kind_zoptionzIRGuestValidationzK kind;
  union {
    struct { unit zNonezIRGuestValidationzK; };
    struct { struct zGuestValidation zSomezIRGuestValidationzK; };
  } variants;
};

// struct BlobProductDivMod
struct zBlobProductDivMod {
  sail_u256 zquotient;
  sail_u256 zremainder;
};

// struct BlobHashes
struct zBlobHashes {
  struct zByteSliceFields zbytes;
  uint64_t zcount;
};

// struct TxEnv
struct zTxEnv {
  struct zBlobHashes zblob_hashes;
  sail_u256 zgas_price;
  sail_fixed_bytes_20 zorigin;
};

// struct BalNonceRun
struct zBalNonceRun {
  uint64_t zcursor;
  uint64_t zmaximum;
};

// struct BalContentCursor
struct zBalContentCursor {
  uint64_t zcontent_len;
  uint64_t zcursor;
};

// struct BalContentCount
struct zBalContentCount {
  uint64_t zcontent_len;
  uint64_t zcount;
};

// struct BalAccountSize
struct zBalAccountSizze {
  uint64_t zencoded_len;
  uint64_t zitem_count;
};

// struct Authorization
struct zAuthorizzation {
  sail_fixed_bytes_20 zaddress;
  sail_fixed_bytes_20 zauthority;
  sail_u256 zchain_id;
  uint64_t znonce;
  bool zvalid_sig;
};

struct node_zz5listz8z5structz0zzAuthorizzzzationz9 {
  unsigned int rc;
  struct zAuthorizzation hd;
  struct node_zz5listz8z5structz0zzAuthorizzzzationz9 *tl;
};
typedef struct node_zz5listz8z5structz0zzAuthorizzzzationz9 *zz5listz8z5structz0zzAuthorizzzzationz9;

struct node_zz5listz8z5structz0zzStorageKeyz9 {
  unsigned int rc;
  struct zStorageKey hd;
  struct node_zz5listz8z5structz0zzStorageKeyz9 *tl;
};
typedef struct node_zz5listz8z5structz0zzStorageKeyz9 *zz5listz8z5structz0zzStorageKeyz9;

struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 {
  unsigned int rc;
  sail_fixed_bytes_20 hd;
  struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *tl;
};
typedef struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 *zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9;

// struct Transaction
struct zTransaction {
  uint64_t zaccess_list_address_count;
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zaccess_list_addresses;
  uint64_t zaccess_list_slot_count;
  zz5listz8z5structz0zzStorageKeyz9 zaccess_list_slots;
  uint64_t zauthorizzation_count;
  zz5listz8z5structz0zzAuthorizzzzationz9 zauthorizzations;
  struct zBlobHashes zblob_hashes;
  uint64_t zchain_id;
  uint64_t zgas_limit;
  struct zByteSliceFields zinput_src;
  bool zis_create;
  sail_u256 zmax_blob_fee;
  sail_u256 zmax_fee;
  sail_u256 zmax_priority_fee;
  sail_u256 znonce;
  struct zByteSliceFields zpubkey;
  struct zByteSliceFields zraw;
  sail_fixed_bytes_20 zrecipient;
  sail_fixed_bytes_20 zsender;
  sail_u256 zsig_r;
  sail_u256 zsig_s;
  sail_u256 zsig_v;
  sail_fixed_bytes_32 zsigning_hash;
  enum zTxType ztx_type;
  sail_u256 zvalue;
};

// struct AuthorizationDecode
struct zAuthorizzationDecode {
  zz5listz8z5structz0zzAuthorizzzzationz9 zauthorizzations;
  uint64_t zcount;
};

// struct AmsterdamAuthorizationState
struct zAmsterdamAuthorizzationState {
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zdelegation_set_for;
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zoriginally_delegated;
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zseen_valid_authorities;
};

// struct AddressResult
struct zAddressResult {
  sail_fixed_bytes_20 zaddress;
  bool zsuccess;
};

// struct AccountInfo
struct zAccountInfo {
  sail_u256 zbalance;
  sail_fixed_bytes_32 zcode_hash;
  uint64_t znonce;
  sail_fixed_bytes_32 zstorage_root;
};

// union option<RAccountInfo>
enum kind_zoptionzIRAccountInfozK { Kind_zNonezIRAccountInfozK, Kind_zSomezIRAccountInfozK };

struct zoptionzIRAccountInfozK {
  enum kind_zoptionzIRAccountInfozK kind;
  union {
    struct { unit zNonezIRAccountInfozK; };
    struct { struct zAccountInfo zSomezIRAccountInfozK; };
  } variants;
};

// struct Account
struct zAccount {
  bool zcreated;
  struct zAccountInfo zinfo;
  bool zpresent;
  bool zselfdestructed;
  bool zstorage_cleared;
};

// union option<RAccount>
enum kind_zoptionzIRAccountzK { Kind_zNonezIRAccountzK, Kind_zSomezIRAccountzK };

struct zoptionzIRAccountzK {
  enum kind_zoptionzIRAccountzK kind;
  union {
    struct { unit zNonezIRAccountzK; };
    struct { struct zAccount zSomezIRAccountzK; };
  } variants;
};

// struct AcctValue
struct zAcctValue {
  struct zAccount zcurr;
  struct zAccount zorig;
};

// struct AcctEntry
struct zAcctEntry {
  sail_fixed_bytes_20 zaddr;
  struct zAcctValue zvalue;
};

// union option<RAcctEntry>
enum kind_zoptionzIRAcctEntryzK { Kind_zNonezIRAcctEntryzK, Kind_zSomezIRAcctEntryzK };

struct zoptionzIRAcctEntryzK {
  enum kind_zoptionzIRAcctEntryzK kind;
  union {
    struct { unit zNonezIRAcctEntryzK; };
    struct { struct zAcctEntry zSomezIRAcctEntryzK; };
  } variants;
};

// struct AccessListDecode
struct zAccessListDecode {
  uint64_t zaddress_count;
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zaddresses;
  uint64_t zslot_count;
  zz5listz8z5structz0zzStorageKeyz9 zstorage_slots;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5BV8Z9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5BV8Z9_DEFINED
struct zz5vecz8z5bv8z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5bv8z9 zz5vecz8z5bv8z9;
#endif

struct node_zz5listz8z5unionz0zzBytesz9 {
  unsigned int rc;
  struct zBytes hd;
  struct node_zz5listz8z5unionz0zzBytesz9 *tl;
};
typedef struct node_zz5listz8z5unionz0zzBytesz9 *zz5listz8z5unionz0zzBytesz9;

// struct tuple_(%bool, %struct z__sail_c_repr_fixed_bytes(20))
struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 {
  bool ztup0;
  sail_fixed_bytes_20 ztup1;
};

// struct tuple_(%list(%bv8), %u64)
struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 {
  zz5listz8z5bv8z9 ztup0;
  uint64_t ztup1;
};

// struct tuple_(%bool, %u64, %u64)
struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 {
  bool ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
};

// struct tuple_(%struct zRlpFieldRefFields, %struct zRlpCursorFields)
struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 {
  struct zRlpFieldRefFields ztup0;
  struct zRlpCursorFields ztup1;
};

// struct tuple_(%list(%struct z__sail_c_repr_fixed_bytes(20)), %list(%struct zStorageKey), %u64, %u64)
struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 {
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 ztup0;
  zz5listz8z5structz0zzStorageKeyz9 ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
};

// struct tuple_(%list(%struct zAuthorizzation), %u64)
struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 {
  zz5listz8z5structz0zzAuthorizzzzationz9 ztup0;
  uint64_t ztup1;
};

// struct tuple_(%bool, %struct zTriePath)
struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 {
  bool ztup0;
  struct zTriePath ztup1;
};

// struct tuple_(%struct zMemoryRangeFields, %struct zMemoryRangeFields)
struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 {
  struct zMemoryRangeFields ztup0;
  struct zMemoryRangeFields ztup1;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5BV16Z9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5BV16Z9_DEFINED
struct zz5vecz8z5bv16z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5bv16z9 zz5vecz8z5bv16z9;
#endif

// struct tuple_(%u64, %u64)
struct ztuple_z8z5u64zCz0z5u64z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

// struct tuple_(%u64, %union zast)
struct ztuple_z8z5u64zCz0z5unionz0zzastz9 {
  uint64_t ztup0;
  struct zast ztup1;
};

#ifndef SAIL_VECTOR_ZZ5VECZ8Z5U64Z9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5U64Z9_DEFINED
struct zz5vecz8z5u64z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5u64z9 zz5vecz8z5u64z9;
#endif

// struct tuple_(%struct z__sail_c_repr_u256, %struct z__sail_c_repr_u256)
struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 {
  sail_u256 ztup0;
  sail_u256 ztup1;
};

// struct tuple_(%struct zTrieUpdate, %bool)
struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 {
  struct zTrieUpdate ztup0;
  bool ztup1;
};

// struct tuple_(%struct zTrieBranchFrame, %struct zTrieBuilder)
struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 {
  struct zTrieBranchFrame ztup0;
  struct zTrieBuilder ztup1;
};

// struct tuple_(%struct zRlpIndexItem, %struct zRlpIndexCursor)
struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 {
  struct zRlpIndexItem ztup0;
  struct zRlpIndexCursor ztup1;
};

// struct tuple_(%struct zTrieItemSink, %struct zTrieUpdateCursor)
struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 {
  struct zTrieItemSink ztup0;
  struct zTrieUpdateCursor ztup1;
};

// struct tuple_(%struct zByteSliceFields, %struct zBoundedSszzListCursor)
struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 {
  struct zByteSliceFields ztup0;
  struct zBoundedSszzListCursor ztup1;
};

// struct tuple_(%struct zByteSliceFields, %struct zBoundedSszzListRef)
struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 {
  struct zByteSliceFields ztup0;
  struct zBoundedSszzListRef ztup1;
};

// struct tuple_(%struct zByteSliceFields, %struct zSszzContainerCursor)
struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 {
  struct zByteSliceFields ztup0;
  struct zSszzContainerCursor ztup1;
};

struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 {
  unsigned int rc;
  sail_fixed_bytes_32 hd;
  struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *tl;
};
typedef struct node_zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9 *zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9;

bool zneq_bool(bool, bool);

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(sail_fixed_bytes_32, sail_fixed_bytes_32);

bool zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(sail_fixed_bytes_20, sail_fixed_bytes_20);

uint64_t zfork_index(enum zFork);

bool zfork_lt(enum zFork, enum zFork);

bool zfork_gteq(enum zFork, enum zFork);

sail_u256 zU256(sail_u256);

sail_fixed_bytes_20 zAddress(sail_fixed_bytes_20);

sail_fixed_bytes_32 zB256(sail_fixed_bytes_32);

sail_u256 zhash_to_word(sail_fixed_bytes_32);

sail_fixed_bytes_32 zword_to_hash(sail_u256);

bool zhash_lt(sail_fixed_bytes_32, sail_fixed_bytes_32);

sail_fixed_bytes_20 zword_to_address(sail_u256);

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


sail_u256 zword_add_word(sail_u256, sail_u256);

sail_u256 zword_sub_word(sail_u256, sail_u256);

sail_u256 zword_and(sail_u256, sail_u256);

sail_u256 zword_or(sail_u256, sail_u256);

sail_u256 zword_xor(sail_u256, sail_u256);

sail_u256 zword_not(sail_u256);

uint64_t zword_bit(sail_u256, uint64_t);

uint64_t zword_low_byte(sail_u256);

sail_u256 zword_shift_right_one(sail_u256);

sail_u256 zword_of_bool(bool);

bool zword_is_zzero(sail_u256);

bool zword_nonzzero(sail_u256);

bool zword_ult(sail_u256, sail_u256);

bool zword_ule(sail_u256, sail_u256);

uint64_t zbyte_bit_length(uint64_t);

uint64_t zu16_bit_length(uint64_t);

uint64_t zu32_bit_length(uint64_t);

uint64_t zu64_bit_length(uint64_t);

uint64_t zword_bit_length(sail_u256);

sail_u256 zword_mul_word(sail_u256, sail_u256);

sail_u256 zword_div_word(sail_u256, sail_u256);

sail_u256 zword_mod_word(sail_u256, sail_u256);

sail_u256 zword_shift_left(sail_u256, uint64_t);

sail_u256 zword_shift_right(sail_u256, uint64_t);

uint64_t zword_byte_length(sail_u256);

sail_u256 zword_arithmetic_shift_right(sail_u256, uint64_t);

sail_u256 zaddress_to_word(sail_fixed_bytes_20);

sail_u256 zword_negate(sail_u256);

sail_u256 zword_abs(sail_u256);

bool zword_slt(sail_u256, sail_u256);

sail_u256 zalu_add(sail_u256, sail_u256);

sail_u256 zalu_sub(sail_u256, sail_u256);

sail_u256 zalu_mul(sail_u256, sail_u256);

sail_u256 zalu_div(sail_u256, sail_u256);

sail_u256 zalu_mod(sail_u256, sail_u256);

sail_u256 zalu_sdiv(sail_u256, sail_u256);

sail_u256 zalu_smod(sail_u256, sail_u256);

sail_u256 zalu_addmod(sail_u256, sail_u256, sail_u256);

sail_u256 zalu_mulmod(sail_u256, sail_u256, sail_u256);

sail_u256 zalu_exp(sail_u256, sail_u256);

sail_u256 zalu_signextend(sail_u256, sail_u256);

sail_u256 zalu_lt(sail_u256, sail_u256);

sail_u256 zalu_gt(sail_u256, sail_u256);

sail_u256 zalu_slt(sail_u256, sail_u256);

sail_u256 zalu_sgt(sail_u256, sail_u256);

sail_u256 zalu_eq(sail_u256, sail_u256);

sail_u256 zalu_iszzero(sail_u256);

sail_u256 zalu_and(sail_u256, sail_u256);

sail_u256 zalu_or(sail_u256, sail_u256);

sail_u256 zalu_xor(sail_u256, sail_u256);

sail_u256 zalu_not(sail_u256);

sail_u256 zalu_byte(sail_u256, sail_u256);

sail_u256 zalu_shl(sail_u256, sail_u256);

sail_u256 zalu_shr(sail_u256, sail_u256);

sail_u256 zalu_sar(sail_u256, sail_u256);

sail_u256 zalu_clzz(sail_u256);

void create_letbind_7(void);
void kill_letbind_7(void);


void create_letbind_8(void);
void kill_letbind_8(void);


void create_letbind_9(void);
void kill_letbind_9(void);


void create_letbind_10(void);
void kill_letbind_10(void);


void create_letbind_11(void);
void kill_letbind_11(void);


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


void create_letbind_22(void);
void kill_letbind_22(void);


void create_letbind_23(void);
void kill_letbind_23(void);


void create_letbind_24(void);
void kill_letbind_24(void);


void create_letbind_25(void);
void kill_letbind_25(void);


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


void create_letbind_33(void);
void kill_letbind_33(void);


void create_letbind_34(void);
void kill_letbind_34(void);


unit zcycle_scope_start(uint64_t);

unit zcycle_scope_end(uint64_t);

void zword_to_account_nonce(struct zoptionzIU64zK *rop, sail_u256);

uint64_t zword_of_account_nonce(uint64_t);

uint64_t zword_of_withdrawal_amount(uint64_t);

uint64_t zword_of_slot_number(uint64_t);

uint64_t zword_of_block_number(uint64_t);

uint64_t zword_of_block_timestamp(uint64_t);

uint64_t zword_of_chain_identifier(uint64_t);

sail_fixed_bytes_20 zprecompile_id_to_address(uint64_t);

uint64_t zaccount_nonce_increment(uint64_t);

uint64_t zframe_depth_increment(uint64_t);

uint64_t ztransaction_blob_count_decrement(uint64_t);

struct zMemoryRangeFields zmemory_range(uint64_t, uint64_t);

void create_letbind_35(void);
void kill_letbind_35(void);


sail_u256 zword_of_source_byte_count(uint64_t);

void create_letbind_36(void);
void kill_letbind_36(void);


void create_letbind_37(void);
void kill_letbind_37(void);


void create_letbind_38(void);
void kill_letbind_38(void);


void create_letbind_39(void);
void kill_letbind_39(void);


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


struct zByteSliceFields zbyte_slice(enum zByteSource, uint64_t, uint64_t);

void create_letbind_50(void);
void kill_letbind_50(void);


struct zByteSliceFields zsub_slice(struct zByteSliceFields, uint64_t, uint64_t);

struct zByteSliceFields zslice_suffix(struct zByteSliceFields, uint64_t);

void zmaterializzed_bytes(struct zMaterializzedBytes *rop, zz5listz8z5bv8z9, uint64_t);

void zbytes_list(struct zBytes *rop, zz5listz8z5bv8z9, uint64_t);

void zbytes_fixed32(struct zBytes *rop, sail_fixed_bytes_32, uint64_t);

void create_letbind_51(void);
void kill_letbind_51(void);


void create_letbind_52(void);
void kill_letbind_52(void);


struct zByteSliceFields zcode_slice(struct zByteSliceFields);

struct zByteSliceFields zvalidated_code_slice(struct zByteSliceFields);

void create_letbind_53(void);
void kill_letbind_53(void);


bool zdeep_stack_immediate_valid(uint64_t);

bool zexchange_immediate_valid(uint64_t);

void create_letbind_54(void);
void kill_letbind_54(void);


void zword_to_bytes32(zz5listz8z5bv8z9 *rop, sail_u256);

void zhash_to_bytes32(zz5listz8z5bv8z9 *rop, sail_fixed_bytes_32);

void zaddress_to_bytes(zz5listz8z5bv8z9 *rop, sail_fixed_bytes_20);

void create_letbind_55(void);
void kill_letbind_55(void);


void create_letbind_56(void);
void kill_letbind_56(void);


sail_fixed_bytes_32 zkeccak256_slice(struct zByteSliceFields);

sail_fixed_bytes_32 zsha256_slice(struct zByteSliceFields);

sail_fixed_bytes_32 zkeccak256_word(sail_u256);

sail_fixed_bytes_32 zkeccak256_address(sail_fixed_bytes_20);

sail_fixed_bytes_32 zsha256_pair(sail_fixed_bytes_32, sail_fixed_bytes_32);

void create_letbind_57(void);
void kill_letbind_57(void);


void create_letbind_58(void);
void kill_letbind_58(void);


struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zecrecover_addr(sail_fixed_bytes_32, uint64_t, sail_u256, sail_u256);

void create_letbind_59(void);
void kill_letbind_59(void);


struct zProtocolProfile zprotocol_profile(uint64_t);

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


void create_letbind_73(void);
void kill_letbind_73(void);


struct zAccount zaccount_from_info(struct zAccountInfo);

void create_letbind_74(void);
void kill_letbind_74(void);


void create_letbind_75(void);
void kill_letbind_75(void);


void create_letbind_76(void);
void kill_letbind_76(void);


uint64_t ztx_type_byte(enum zTxType);

bool ztx_is_access_list(enum zTxType);

bool ztx_is_dynamic_fee(enum zTxType);

bool ztx_is_blob(enum zTxType);

bool ztx_is_set_code(enum zTxType);

void create_letbind_77(void);
void kill_letbind_77(void);


bool zlogs_bloom_equal(sail_fixed_bytes_256, sail_fixed_bytes_256);

void zlogs_bloom_bytes(zz5listz8z5bv8z9 *rop, sail_fixed_bytes_256);

void create_letbind_78(void);
void kill_letbind_78(void);


void create_letbind_79(void);
void kill_letbind_79(void);


void create_letbind_80(void);
void kill_letbind_80(void);


void create_letbind_81(void);
void kill_letbind_81(void);


void create_letbind_82(void);
void kill_letbind_82(void);


uint64_t zslice_byte(struct zByteSliceFields, uint64_t);

uint64_t zslice_count_nonzzero(struct zByteSliceFields);

bool zslice_strided_zzero(struct zByteSliceFields, uint64_t, uint64_t, uint64_t, uint64_t);

sail_u256 zslice_load(struct zByteSliceFields, uint64_t);

sail_u256 zslice_load_word_offset(struct zByteSliceFields, sail_u256);

sail_u256 zslice_load_n(struct zByteSliceFields, uint64_t, uint64_t);

unit zslice_copy(struct zByteSliceFields, uint64_t, uint64_t, uint64_t);

unit zslice_copy_word_offset(struct zByteSliceFields, uint64_t, sail_u256, uint64_t);

uint64_t zscratch_begin(unit);

unit zscratch_push_bytes(zz5listz8z5bv8z9, uint64_t);

unit zscratch_push_slice(struct zByteSliceFields);

unit zscratch_push_b256(sail_fixed_bytes_32, uint64_t);

struct zByteSliceFields zscratch_finish(uint64_t);

unit zscratch_rewind(uint64_t);

unit zscratch_reset(unit);

unit zstore_jumpdest_chunk(uint64_t, uint64_t, struct zCodeAnalysis);

sail_u256 zjumpdest_bit(uint64_t);

unit zanalyzze_code_from(struct zByteSliceFields, enum zFork, uint64_t, uint64_t, struct zCodeAnalysis);

uint64_t zanalyzze_code(struct zByteSliceFields, enum zFork);

sail_fixed_bytes_32 zcode_db_insert(struct zByteSliceFields, enum zFork);

struct zCode zcode_db_resolve(sail_fixed_bytes_32);

void create_letbind_83(void);
void kill_letbind_83(void);


void create_letbind_84(void);
void kill_letbind_84(void);


uint64_t zsszz_field_offset(uint64_t, uint64_t);

uint64_t zsszz_u32_at(struct zByteSliceFields, uint64_t);

uint64_t zsszz_u32(struct zByteSliceFields, uint64_t);

uint64_t zsszz_offset_to_source_pointer(uint64_t);

uint64_t zdecode_sszz_uint(struct zByteSliceFields, uint64_t);

sail_fixed_bytes_20 zsszz_addr(struct zByteSliceFields, uint64_t);

sail_fixed_bytes_32 zsszz_bytes32(struct zByteSliceFields, uint64_t);

uint64_t zsszz_logs_bloom_index(uint64_t);

sail_fixed_bytes_256 zsszz_logs_bloom(struct zByteSliceFields, uint64_t);

uint64_t zsszz_u256_index(uint64_t);

sail_u256 zsszz_u256(struct zByteSliceFields, uint64_t);

void create_letbind_85(void);
void kill_letbind_85(void);


void create_letbind_86(void);
void kill_letbind_86(void);


void create_letbind_87(void);
void kill_letbind_87(void);


void create_letbind_88(void);
void kill_letbind_88(void);


void create_letbind_89(void);
void kill_letbind_89(void);


uint64_t zrlp_scratch_small_length(uint64_t);

uint64_t zword_byte_count(uint64_t);

void zminimal_word_bytes(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, sail_u256);

uint64_t zrlp_nat_length_byte(uint64_t);

uint64_t zrlp_minimal_word_len(sail_u256);

sail_u128 zrlp_slice_sizze(struct zByteSliceFields);

uint64_t zrlp_uint_word_sizze(sail_u256);

uint64_t zrlp_uint_nat_sizze(uint64_t);

uint64_t zrlp_scratch_list_sizze(uint64_t);

uint64_t zrlp_scratch_slice_sizze(struct zByteSliceFields);

uint64_t zrlp_scratch_bytes_sizze(zz5listz8z5bv8z9, uint64_t);

unit zrlp_write_bytes(zz5listz8z5bv8z9, uint64_t);

unit zrlp_write_slice(struct zByteSliceFields);

unit zrlp_write_uint_word(sail_u256);

unit zrlp_write_uint_nat(uint64_t);

unit zrlp_write_word(sail_u256);

unit zrlp_write_addr(sail_fixed_bytes_20);

struct zByteSliceFields zrlp_finish(uint64_t);

uint64_t zrlp_uint64_append(uint64_t, uint64_t, uint64_t);

uint64_t zrlp_uint64_width(struct zByteSliceFields, uint64_t);

uint64_t zrlp_ref_be_length(struct zByteSliceFields, uint64_t, uint64_t, uint64_t);

bool zrlp_bytes_equal_at(zz5listz8z5bv8z9, struct zByteSliceFields, uint64_t);

struct ztuple_z8z5boolzCz0z5u64zCz0z5u64z9 zrlp_ref_hdr(struct zByteSliceFields, uint64_t, uint64_t);

struct zRlpFieldRefFields zrlp_field_ref(struct zByteSliceFields, bool, uint64_t, uint64_t, uint64_t, uint64_t);

struct zRlpCursorFields zrlp_cursor(struct zByteSliceFields, uint64_t, uint64_t, bool);

struct zRlpCursorFields zrlp_invalid_cursor(struct zByteSliceFields);

struct zRlpCursorFields zrlp_node_cursor(struct zByteSliceFields);

struct zRlpCursorFields zrlp_ref_cursor(struct zRlpFieldRefFields);

bool zrlp_cursor_empty(struct zRlpCursorFields);

struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 zrlp_cursor_pop(struct zRlpCursorFields);

unit zrlp_cursor_expect_end(struct zRlpCursorFields);

struct zRlpFieldRefFields zrlp_single_ref(struct zByteSliceFields);

struct zByteSliceFields zrlp_ref_content(struct zRlpFieldRefFields);

struct zByteSliceFields zrlp_ref_full(struct zRlpFieldRefFields);

bool zrlp_ref_framing_canonical(struct zRlpFieldRefFields);

bool zrlp_ref_bytes_canonical(struct zRlpFieldRefFields);

bool zrlp_ref_uint_canonical(struct zRlpFieldRefFields);

sail_u256 zrlp_ref_word(struct zRlpFieldRefFields);

sail_u256 zrlp_ref_uint_word(struct zRlpFieldRefFields);

uint64_t zrlp_ref_uint64(struct zRlpFieldRefFields);

uint64_t zrlp_ref_account_nonce(struct zRlpFieldRefFields);

uint64_t zrlp_ref_blob_gas_used(struct zRlpFieldRefFields);

uint64_t zrlp_ref_excess_blob_gas(struct zRlpFieldRefFields);

uint64_t zrlp_ref_chain_identifier(struct zRlpFieldRefFields);

sail_fixed_bytes_20 zcreate_address(sail_fixed_bytes_20, uint64_t);

sail_fixed_bytes_20 zcreate2_address(sail_fixed_bytes_20, sail_u256, sail_fixed_bytes_32);

sail_u256 zlegacy_sig_chain_id(sail_u256);

void create_letbind_90(void);
void kill_letbind_90(void);


void create_letbind_91(void);
void kill_letbind_91(void);


sail_fixed_bytes_32 ztx_signing_hash(enum zTxType, struct zByteSliceFields, sail_u256);

sail_fixed_bytes_32 zauth_signing_hash(sail_u256, sail_fixed_bytes_20, uint64_t);

bool ztx_sig_v_valid(uint64_t, enum zTxType, sail_u256);

uint64_t ztx_y_parity(enum zTxType, sail_u256);

bool ztx_auth_valid(sail_fixed_bytes_20, sail_fixed_bytes_32, uint64_t, sail_u256, sail_u256);

void create_letbind_92(void);
void kill_letbind_92(void);


void zdecode_access_list_keys(struct zAccessListDecode *rop, struct zRlpCursorFields, sail_fixed_bytes_20, struct zAccessListDecode);

void zdecode_access_list_entries(struct zAccessListDecode *rop, struct zRlpCursorFields);

void zdecode_access_list(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *rop, struct zRlpFieldRefFields);

void create_letbind_93(void);
void kill_letbind_93(void);


void create_letbind_94(void);
void kill_letbind_94(void);


uint64_t zdecode_blob_hash_items(struct zRlpCursorFields, uint64_t);

struct zBlobHashes zdecode_blob_hashes(struct zRlpFieldRefFields);

void create_letbind_95(void);
void kill_letbind_95(void);


void zdecode_auth_tuples(struct zAuthorizzationDecode *rop, struct zRlpCursorFields);

void zdecode_auth_list(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *rop, struct zRlpFieldRefFields);

struct zByteSliceFields ztx_input_span(struct zRlpFieldRefFields);

struct zByteSliceFields ztx_sig_span(struct zRlpFieldRefFields, struct zRlpFieldRefFields);

uint64_t zrlp_ref_gas(struct zRlpFieldRefFields, enum zFork);

void zrlp_decode_tx(struct zTransaction *rop, struct zByteSliceFields, struct zByteSliceFields, enum zFork);

struct zByteSliceFields zoutput_buffer_slice(uint64_t);

struct zByteSliceFields zfreezze_output(struct zByteSliceFields);

struct zByteSliceFields zoutput_buffer_word(sail_u256);

struct zByteSliceFields zoutput_buffer_words(sail_u256, sail_u256);

sail_u256 zk_env(enum zEnvField);

sail_fixed_bytes_20 zk_coinbase(unit);

sail_u256 zblockhash_word_distance(sail_u256, sail_u256);

sail_fixed_bytes_32 zk_blockhash(sail_u256);

sail_u256 zk_blobhash(sail_u256);

sail_fixed_bytes_20 zk_create_addr(sail_fixed_bytes_20, uint64_t);

sail_fixed_bytes_20 zk_create2_addr(sail_fixed_bytes_20, sail_u256, sail_fixed_bytes_32);

struct zStorageKey zstorage_key(sail_fixed_bytes_20, sail_u256);

bool zk_access_account(sail_fixed_bytes_20);

bool zk_slot_is_warm(sail_fixed_bytes_20, sail_u256);

struct zAccountInfo zdecode_state_account(struct zByteSliceFields);

struct zTriePath zpath_new(sail_fixed_bytes_32, uint64_t);

struct zTriePath zpath_from_hash(sail_fixed_bytes_32);

struct zByteSliceFields znode_db_lookup(sail_fixed_bytes_32);

struct zRlpFieldRefFields zbranch_children_get(zz5vecz8z5structz0zzRlpFieldRefFieldsz9, uint64_t);

void create_letbind_96(void);
void kill_letbind_96(void);


struct ztuple_z8z5boolzCz0z5structz0zzTriePathz9 zhex_prefix_decode_ref(struct zRlpFieldRefFields);

uint64_t zpath_len(struct zTriePath);

void zdecode_trie_node(struct zTrieNode *rop, struct zByteSliceFields);

void create_letbind_97(void);
void kill_letbind_97(void);


struct zInlineNode zinline_node_from_slice(struct zByteSliceFields);

void zfield_to_ref(struct zNodeRef *rop, struct zRlpFieldRefFields);

uint64_t zpath_byte_index(uint64_t);

uint64_t zpath_nibble(struct zTriePath, uint64_t);

uint64_t ztrie_path_len_increment(uint64_t);

bool zpath_matches(struct zTriePath, uint64_t, struct zTriePath);

struct zByteSliceFields zinline_node_slice(struct zInlineNode);

struct zByteSliceFields zresolve_ref(struct zNodeRef);

struct zByteSliceFields ztrie_walk(struct zByteSliceFields, struct zTriePath, uint64_t);

struct zByteSliceFields ztrie_lookup(sail_fixed_bytes_32, struct zTriePath);

void zstateless_account(struct zoptionzIRAccountInfozK *rop, sail_fixed_bytes_32, sail_fixed_bytes_20);

struct zAccount zk_aload(sail_fixed_bytes_20);

sail_u256 zstateless_storage(sail_fixed_bytes_32, sail_u256);

struct zStorageValue zk_sload(sail_fixed_bytes_20, sail_u256);

unit zk_sstore(sail_fixed_bytes_20, sail_u256, struct zStorageValue);

sail_u256 zk_tload(sail_fixed_bytes_20, sail_u256);

unit zk_tstore(sail_fixed_bytes_20, sail_u256, sail_u256);

unit zk_log(sail_fixed_bytes_20, zz5listz8z5structz0zz__sail_c_repr_u256z9, struct zBytes);

unit zk_emit_transfer_log(sail_fixed_bytes_20, sail_fixed_bytes_20, sail_u256);

bool zaccount_info_changed(struct zAccountInfo, struct zAccountInfo);

bool zaccount_info_empty(struct zAccountInfo);

bool zaccount_changed(struct zAccount, struct zAccount);

struct zAccount zaccount_set_info(struct zAccount, struct zAccountInfo);

struct zAccount zaccount_clear_storage(struct zAccount);

struct zAccount zaccount_delete(struct zAccount);

struct zAccount zaccount_clear_preserving_balance(struct zAccount);

unit zstore_account(sail_fixed_bytes_20, struct zAccount);

unit zstore_account_info(sail_fixed_bytes_20, struct zAccount, struct zAccountInfo);

sail_u256 zk_get_balance(sail_fixed_bytes_20);

uint64_t zk_get_nonce(sail_fixed_bytes_20);

bool zk_account_exists(sail_fixed_bytes_20);

bool zk_account_is_empty(sail_fixed_bytes_20);

bool zk_account_occupied(sail_fixed_bytes_20);

unit zk_transfer(sail_fixed_bytes_20, sail_fixed_bytes_20, sail_u256);

unit zk_bump_nonce(sail_fixed_bytes_20);

unit zk_add_balance(sail_fixed_bytes_20, sail_u256);

unit zk_sub_balance(sail_fixed_bytes_20, sail_u256);

unit zk_clear_storage(sail_fixed_bytes_20);

sail_fixed_bytes_32 zk_code_key(sail_fixed_bytes_20);

sail_fixed_bytes_32 zk_get_codehash(sail_fixed_bytes_20);

unit zk_deploy_code(sail_fixed_bytes_20, struct zByteSliceFields);

uint64_t zdelegation_address_index(uint64_t);

uint64_t zdelegation_code_index(uint64_t);

sail_u256 zdelegation_jumpdest_chunk(sail_fixed_bytes_20);

unit zk_set_delegation(sail_fixed_bytes_20, sail_fixed_bytes_20);

unit zk_clear_code(sail_fixed_bytes_20);

struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zk_deleg_target(sail_fixed_bytes_20);

uint64_t zk_get_code_sizze(sail_fixed_bytes_20);

unit zk_code_copy(sail_fixed_bytes_20, uint64_t, sail_u256, uint64_t);

unit zk_selfdestruct(sail_fixed_bytes_20);

bool zk_is_selfdestructed(sail_fixed_bytes_20);

unit zk_mark_created(sail_fixed_bytes_20);

bool zk_was_created(sail_fixed_bytes_20);

unit zk_zzero_balance(sail_fixed_bytes_20);

uint64_t zk_state_checkpoint(unit);

unit zk_set_header(struct zBlockHeader);

unit zk_set_tx(struct zTxEnv);

unit zk_tx_reset(unit);

bool zaccount_deleted_at_tx_end(struct zAccount);

unit zk_tx_merge(unit);

unit zk_revert(uint64_t);

void create_letbind_98(void);
void kill_letbind_98(void);


unit zframe_stack_reset(unit);

bool zframe_stack_is_empty(unit);

unit zframe_stack_push(struct zFrameContinuation);

void zframe_stack_pop(struct zFrameContinuation *rop, unit);

__int128 zvalidated_refund_add(__int128, __int128);

unit zrecord_refund(__int128);

uint64_t zframe_code_len(unit);

bool zframe_jumpdest_valid(uint64_t);

void create_letbind_99(void);
void kill_letbind_99(void);


uint64_t zconserved_gas_add(uint64_t, uint64_t);

unit zrefill_frame_state_gas(unit);

__int128 zframe_state_gas_used(unit);

unit zexc_halt(enum zExceptionKind);

uint64_t zstack_height(unit);

sail_u256 zpeek(uint64_t);

unit zpush_word(sail_u256);

unit zpush_gas(uint64_t);

sail_u256 zpop(unit);

unit zstack_set(uint64_t, sail_u256);

bool zis_running(unit);

unit zcalldata_install(struct zByteSliceFields);

unit zreturndata_clear(unit);

uint64_t zreturndata_sizze(unit);

unit zreturndata_copy(uint64_t, uint64_t, uint64_t);

unit zreturndata_copy_prefix(uint64_t, uint64_t);

uint64_t zreturndata_remaining(uint64_t, uint64_t);

unit zvalidated_returndata_copy(uint64_t, sail_u256, sail_u256);

unit zreturndata_copy_words(uint64_t, sail_u256, sail_u256);

uint64_t zevm_memory_high_water(unit);

unit zmemory_reset(unit);

struct zByteSliceFields zmemory_expand_to(uint64_t);

struct zByteSliceFields zmemory_byte_slice(uint64_t, uint64_t);

struct zByteSliceFields zmemory_code_slice(uint64_t, uint64_t);

struct zByteSliceFields zmemory_frame_enter(unit);

unit zmemory_frame_leave(struct zByteSliceFields);

void zsuspend_frame(struct zFrameCheckpoint *rop, unit);

unit zrestore_frame(struct zFrameCheckpoint);

unit zmem_set_byte(uint64_t, uint64_t);

sail_u256 zmem_load(uint64_t);

unit zmem_store(uint64_t, sail_u256);

unit zmem_store_byte(uint64_t, sail_u256);

unit zmem_mcopy(uint64_t, uint64_t, uint64_t);

sail_u256 zmem_keccak(struct zMemoryRangeFields);

void create_letbind_100(void);
void kill_letbind_100(void);


void create_letbind_101(void);
void kill_letbind_101(void);


void create_letbind_102(void);
void kill_letbind_102(void);


uint64_t zfake_exponential_index_increment(uint64_t);

sail_u256 zblob_word_add(sail_u256, sail_u256);

struct zScaledBlobValue zscaled_blob_add(struct zScaledBlobValue, struct zScaledBlobValue, uint64_t);

struct zBlobProductDivMod zblob_product_divmod(sail_u256, uint64_t, sail_u256, uint64_t, uint64_t);

struct zScaledBlobValue zscaled_blob_next(struct zScaledBlobValue, uint64_t, uint64_t, uint64_t);

sail_u256 zfake_exponential(sail_u256, uint64_t, uint64_t);

sail_u256 zblob_base_fee(uint64_t);

bool zblob_reserve_price_active(sail_u256, sail_u256);

uint64_t zblob_schedule_target(unit);

uint64_t zblob_schedule_max(unit);

uint64_t zblob_gas_for_count(uint64_t);

uint64_t zblob_target_gas_per_block(unit);

uint64_t zblob_max_gas_per_block(unit);

uint64_t zchecked_block_blob_gas_add(uint64_t, uint64_t);

uint64_t znext_excess_blob_gas(uint64_t, uint64_t, sail_u256);

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


void create_letbind_139(void);
void kill_letbind_139(void);


void create_letbind_140(void);
void kill_letbind_140(void);


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


uint64_t zsstore_clear_refund(unit);

uint64_t zstate_gas_spill_room(uint64_t);

unit zcredit_state_gas_refund(uint64_t);

unit zreturn_child_state_gas(uint64_t, uint64_t);

uint64_t zgas_sub_or_oog(uint64_t, uint64_t);

unit zrefund_gas(uint64_t);

sail_u256 zmemory_word_count_word(sail_u256);

sail_u128 zbounded_mem_cost(uint64_t);

uint64_t zmemory_expansion_cost(uint64_t, uint64_t);

struct zMemoryExpansion zmemory_expansion(sail_u256, sail_u256, uint64_t);

struct zMemoryPairExpansion zmemory_pair_expansion(sail_u256, sail_u256, sail_u256, sail_u256, uint64_t);

unit zexpand_memory(uint64_t);

struct zMemoryRangeFields zapply_memory_expansion(struct zMemoryExpansion);

struct zMemoryRangeFields zcharge_memory_range(sail_u256, sail_u256);

struct ztuple_z8z5structz0zzMemoryRangeFieldszCz0z5structz0zzMemoryRangeFieldsz9 zapply_memory_pair_expansion(struct zMemoryPairExpansion);

uint64_t zaccount_cost(bool);

uint64_t zexternal_code_read_cost(unit);

uint64_t zsload_cost(bool);

uint64_t zcall_value_cost(unit);

uint64_t zcreate_access_cost(unit);

void zcode_deployment_execution_cost(struct zoptionzIU64zK *rop, uint64_t, uint64_t);

uint64_t zcode_deployment_state_cost(uint64_t);

sail_u256 zpc_word(struct zByteSliceFields, uint64_t, uint64_t);

sail_u256 zpc_word_after_declared_field(struct zByteSliceFields, uint64_t, sail_u256, uint64_t);

uint64_t zpc_blake2_rounds(struct zByteSliceFields);

void zmodexp_gas(struct zoptionzIU64zK *rop, struct zByteSliceFields, uint64_t);

void create_letbind_150(void);
void kill_letbind_150(void);


void create_letbind_151(void);
void kill_letbind_151(void);


void zbls_msm_gas(struct zoptionzIU64zK *rop, zz5vecz8z5bv16z9, uint64_t, uint64_t, uint64_t, uint64_t);

void zlinear_gas(struct zoptionzIU64zK *rop, uint64_t, uint64_t, uint64_t, uint64_t);

void zprecompile_gas(struct zoptionzIU64zK *rop, uint64_t, struct zByteSliceFields, uint64_t);

uint64_t zamsterdam_storage_access_cost(bool);

struct zSstoreCosts zlegacy_sstore_costs(sail_u256, sail_u256, sail_u256, bool);

struct zSstoreCosts zamsterdam_sstore_costs(sail_u256, sail_u256, sail_u256, bool);

struct zSstoreCosts zsstore_costs(sail_u256, sail_u256, sail_u256, bool);

unit zcharge_word_scaled_gas(uint64_t, sail_u256);

unit zcharge_memory_word_gas(uint64_t, uint64_t, sail_u256);

unit zcharge_keccak_gas(sail_u256);

unit zcharge_copy_gas(sail_u256);

unit zcharge_log_gas(uint64_t, sail_u256);

uint64_t zexp_gas(sail_u256);

uint64_t ztransaction_initcode_gas(uint64_t);

uint64_t zcall_gas_cap_word(uint64_t, sail_u256);

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


struct zPrecompileResult zprecompile_success(struct zByteSliceFields);

struct zPrecompileResult zprecompile_failure(unit);

struct zPrecompileResult zaccelerator_result(bool, uint64_t);

struct zPrecompileResult zcopied_result(struct zByteSliceFields);

struct zPrecompileResult zboolean_result(bool);

bool zprecompile_active_at_fork(uint64_t);

uint64_t zprecompile_number(sail_fixed_bytes_20);

struct zPrecompileResult zrun_ecrecover(struct zByteSliceFields);

struct zPrecompileResult zrun_sha256(struct zByteSliceFields);

struct zPrecompileResult zrun_ripemd160(struct zByteSliceFields);

struct zPrecompileResult zrun_modexp(struct zByteSliceFields);

struct zPrecompileResult zpairing_result(uint64_t);

struct zPrecompileResult zrun_blake2f(struct zByteSliceFields);

bool zkzzg_versioned_hash_matches(struct zByteSliceFields);

struct zPrecompileResult zrun_kzzg_point_evaluation(struct zByteSliceFields);

bool zbls_g1_padding(struct zByteSliceFields, uint64_t, uint64_t, uint64_t);

bool zbls_g2_padding(struct zByteSliceFields, uint64_t, uint64_t, uint64_t);

struct zPrecompileResult zrun_bls_g1_add(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_g1_msm(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_g2_add(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_g2_msm(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_pairing(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_map_fp_to_g1(struct zByteSliceFields);

struct zPrecompileResult zrun_bls_map_fp2_to_g2(struct zByteSliceFields);

struct zPrecompileResult zrun_p256_verify(struct zByteSliceFields);

struct zPrecompileResult zrun_precompile_slice(uint64_t, struct zByteSliceFields);

uint64_t zdecode_single_stack_index(uint64_t);

struct ztuple_z8z5u64zCz0z5u64z9 zdecode_exchange_stack_indices(uint64_t);

sail_fixed_bytes_20 zself_addr(unit);

bool zguard_static(unit);

unit zdo_jump(sail_u256);

void zpop_log_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 *rop, uint64_t);

unit zexecute_arithmetic(struct zast);

unit zexecute_environment(struct zast);

unit zexecute_block(struct zast);

unit zexecute_memory(struct zast);

unit zexecute_storage(struct zast);

unit zexecute_control(struct zast);

unit zexecute_log(struct zast);

unit zexecute_halt(struct zast);

unit zexecute_opcode(struct zast);

bool zcall_is_delegate(enum zCallKind);

bool zcall_is_static(enum zCallKind);

bool zcall_takes_value(enum zCallKind);

bool zcall_transfers_value(enum zCallKind);

bool zcall_uses_target_address(enum zCallKind);

struct zCode zexecutable_code(sail_fixed_bytes_20, bool, sail_fixed_bytes_20);

unit zrun_call(enum zCallKind);

unit zrun_create(bool);

unit zexecute(struct zast);

sail_u256 zread_push(struct zByteSliceFields, uint64_t, uint64_t);

void zdecode_simple(struct zast *rop, uint64_t);

void zfetch(struct zast *rop, unit);

struct zByteSliceFields zframe_output(unit);

bool zframe_succeeded(unit);

unit zresume_call(struct zCallContinuation, struct zByteSliceFields);

unit zresume_create(struct zCreateContinuation, struct zByteSliceFields);

unit zresume_frame(struct zFrameContinuation, struct zByteSliceFields);

struct zByteSliceFields zinterpret(unit);

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


void create_letbind_206(void);
void kill_letbind_206(void);


void create_letbind_207(void);
void kill_letbind_207(void);


void create_letbind_208(void);
void kill_letbind_208(void);


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


struct zByteSliceFields ztransaction_initcode_slice(struct zByteSliceFields);

void create_letbind_218(void);
void kill_letbind_218(void);


uint64_t zcalldata_cost(struct zByteSliceFields);

uint64_t zvalidate_blob_hash_version_at(struct zBlobHashes, uint64_t, uint64_t);

unit zvalidate_blob_hash_versions(struct zBlobHashes);

uint64_t zlegacy_intrinsic_gas(struct zTransaction);

uint64_t zlegacy_calldata_floor(struct zByteSliceFields);

uint64_t zamsterdam_recipient_execution_cost(struct zTransaction);

struct zIntrinsicGasCost zintrinsic_gas(struct zTransaction);

uint64_t zmax_blobs_per_transaction(unit);

uint64_t ztransaction_blob_gas_for_count(uint64_t);

struct zTransactionCosts ztransaction_costs(struct zTransaction, uint64_t, sail_u256);

uint64_t zvalidated_gas_add(uint64_t, uint64_t);

uint64_t zadmitted_transaction_gas_limit(uint64_t, uint64_t);

__int128 zprocess_auth(struct zAuthorizzation);

__int128 zprocess_auth_list(zz5listz8z5structz0zzAuthorizzzzationz9);

bool zauthorizzation_address_seen(sail_fixed_bytes_20, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9);

void zprocess_amsterdam_auth(struct zAmsterdamAuthorizzationState *rop, struct zAuthorizzation, sail_fixed_bytes_20, sail_fixed_bytes_20, bool, struct zAmsterdamAuthorizzationState);

void zprocess_amsterdam_auth_list(struct zAmsterdamAuthorizzationState *rop, zz5listz8z5structz0zzAuthorizzzzationz9, sail_fixed_bytes_20, sail_fixed_bytes_20, bool, struct zAmsterdamAuthorizzationState);

unit zwarm_access_list_addresses(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9);

unit zwarm_access_list_slots(zz5listz8z5structz0zzStorageKeyz9);

unit zprewarm(struct zTransaction);

struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zeff_gas_price_for(sail_u256, sail_u256, sail_u256);

struct zTxValidity zcheck_transaction_validity(struct zTransaction);

struct zTxUpfrontResult zapply_transaction_upfront_effects(struct zTransaction, struct zTxValidity);

unit zenter_transaction_frame(uint64_t, uint64_t, uint64_t);

bool zprepare_amsterdam_transaction_dispatch(struct zTransaction, struct zTxValidity, struct zTxUpfrontResult);

unit zrun_create_transaction_frame(struct zTransaction, sail_fixed_bytes_20, uint64_t);

unit zrun_call_transaction_frame(struct zTransaction, sail_fixed_bytes_20, bool);

struct zTxFrameResult zrun_legacy_transaction_frame(struct zTransaction, struct zTxValidity);

struct zTxFrameResult zrun_amsterdam_transaction_frame(struct zTransaction, struct zTxValidity, struct zTxUpfrontResult);

struct zTxFrameResult zrun_transaction_frame(struct zTransaction, struct zTxValidity, struct zTxUpfrontResult);

uint64_t zadmitted_transaction_state_gas(__int128);

void zsettle_transaction(struct zReceipt *rop, struct zTransaction, struct zTxValidity, __int128, struct zTxFrameResult);

void zprocess_transaction(struct zReceipt *rop, struct zTransaction);

uint64_t zto_trie_depth(uint64_t);

struct zTriePath zpath_append_nibble(struct zTriePath, uint64_t);

struct zTriePath zpath_append_byte(struct zTriePath, uint64_t);

struct zTriePath zpath_single(uint64_t);

struct zTriePath zpath_concat(struct zTriePath, struct zTriePath);

struct zTriePath zpath_take(struct zTriePath, uint64_t);

struct zTriePath zpath_drop(struct zTriePath, uint64_t);

bool zpath_eq(struct zTriePath, struct zTriePath);

bool zpath_lt(struct zTriePath, struct zTriePath);

bool zpath_prefix_of(struct zTriePath, struct zTriePath);

uint64_t zcommon_prefix_from(struct zTriePath, struct zTriePath, uint64_t);

void zhex_prefix_pairs(zz5listz8z5bv8z9 *rop, struct zTriePath, uint64_t);

void zhex_prefix_compact(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, struct zTriePath, bool);

sail_fixed_bytes_32 zinline_node_hash(struct zInlineNode);

uint64_t zbranch_content_length_add(uint64_t, uint64_t);

uint64_t znode_ref_sizze(struct zNodeRef);

unit zrlp_write_node_ref(struct zNodeRef);

void zchild_ref(struct zNodeRef *rop, struct zByteSliceFields);

uint64_t zbranch_mask_for(uint64_t);

bool zbranch_mask_has(uint64_t, uint64_t);

uint64_t zbranch_mask_set(uint64_t, uint64_t);

void zleaf_child_ref(struct zNodeRef *rop, struct zTriePath, struct zByteSliceFields);

void zextension_child_ref(struct zNodeRef *rop, struct zTriePath, struct zNodeRef);

void zbranch_child_ref(struct zNodeRef *rop, uint64_t, zz5vecz8z5unionz0zzNodeRefz9);

sail_fixed_bytes_32 ztrie_ref_to_root(struct zNodeRef);

void znode_to_ref(struct zNodeRef *rop, struct zByteSliceFields);

void zmerge_ext_node(struct zNodeRef *rop, struct zTriePath, struct zByteSliceFields);

void zmerge_ext_ref(struct zNodeRef *rop, struct zTriePath, struct zNodeRef);

void zcached_account_trie_update_next(struct zoptionzIRTrieUpdatezK *rop, unit);

struct zByteSliceFields zencode_state_account(struct zAccountInfo, sail_fixed_bytes_32);

void zaccount_update(struct zTrieUpdate *rop, struct zAcctEntry, sail_fixed_bytes_32);

bool zaccount_value_changed(struct zAcctValue);

bool zstorage_value_changed(struct zStorageValue);

void znext_changed_storage_entry(struct zoptionzIRStorageEntryzK *rop, sail_fixed_bytes_20);

void zaccount_trie_update(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *rop, struct zAcctEntry);

void znext_changed_account_trie_update(struct zoptionzIRTrieUpdatezK *rop, unit);

struct zByteSliceFields zencode_storage_value(sail_u256);

void zstorage_update(struct zTrieUpdate *rop, struct zStorageEntry);

void znext_storage_trie_update(struct zoptionzIRTrieUpdatezK *rop, sail_fixed_bytes_20);

void ztrie_update_source_next(struct zoptionzIRTrieUpdatezK *rop, struct zTrieUpdateSource);

void ztrie_updates_begin(struct zTrieUpdateCursor *rop, struct zTrieUpdateSource);

bool zupdates_empty(struct zTrieUpdateCursor);

void ztrie_updates_advance(struct zTrieUpdateCursor *rop, struct zTrieUpdateCursor);

bool znext_update_under(struct zTrieUpdateCursor, struct zTriePath);

void zitem_leaf(struct zTrieItem *rop, struct zTriePath, struct zByteSliceFields);

void zitem_branch(struct zTrieItem *rop, struct zTriePath, struct zNodeRef);

void zitem_subtree(struct zTrieItem *rop, struct zTriePath, struct zNodeRef);

void zitem_ref(struct zNodeRef *rop, struct zTrieItem, uint64_t);

void zempty_trie_branch_frame(struct zTrieBranchFrame *rop, uint64_t);

void ztrie_builder_empty(struct zTrieBuilder *rop, unit);

void ztrie_builder_push(struct zTrieBuilder *rop, struct zTrieBuilder, uint64_t);

void ztrie_builder_attach(struct zTrieBuilder *rop, struct zTrieBuilder, struct zTriePath, struct zNodeRef);

void ztrie_builder_pop(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *rop, struct zTrieBuilder);

void ztrie_builder_wrap_branch(struct zNodeRef *rop, struct zTriePath, uint64_t, uint64_t, struct zNodeRef);

void ztrie_builder_close(struct zTrieBuilder *rop, struct zTrieBuilder, struct zTriePath, struct zoptionzIU64zK, uint64_t);

void ztrie_item_next_common(struct zoptionzIU64zK *rop, struct zTrieItem, struct zoptionzIRTriePathzK);

void ztrie_insert_item(struct zTrieBuilder *rop, struct zTrieBuilder, struct zTrieItem, struct zoptionzIRTriePathzK);

void ztrie_sink_empty(struct zTrieItemSink *rop, unit);

void ztrie_sink_emit(struct zTrieItemSink *rop, struct zTrieItemSink, struct zTrieItem);

void ztrie_sink_finish(struct zTrieItemSink *rop, struct zTrieItemSink);

sail_fixed_bytes_32 ztrie_builder_root(struct zTrieBuilder);

sail_fixed_bytes_32 ztrie_sink_root(struct zTrieItemSink);

uint64_t zrlp_index_byte_width_decrement(uint64_t);

uint64_t zrlp_index_encoded_width(uint64_t);

struct zTriePath ztrie_index_key(uint64_t);

struct zRlpIndexCursor zrlp_index_cursor(uint64_t);

bool zrlp_index_cursor_empty(struct zRlpIndexCursor);

uint64_t zrlp_index_at_position(struct zRlpIndexCursor);

void zrlp_index_cursor_pop(struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 *rop, struct zRlpIndexCursor);

void zemit_live_updates_under(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieItemSink, struct zTrieUpdateCursor, struct zTriePath);

void zemit_updates_before_child(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieItemSink, struct zTrieUpdateCursor, struct zTriePath, struct zTriePath);

void zemit_leaf_overlay(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zTrieItemSink, struct zTrieUpdateCursor, struct zTriePath, struct zTriePath, struct zByteSliceFields);

void zwitness_emit(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *rop, struct zByteSliceFields, struct zTriePath, struct zTrieUpdateCursor, struct zTrieItemSink, uint64_t);

struct zTrieRootResult ztrie_root_cursor(sail_fixed_bytes_32, struct zTrieUpdateCursor);

struct zTrieRootResult ztrie_root(sail_fixed_bytes_32, struct zTrieUpdateSource);

unit zprepare_account_post_storage_root(struct zAcctEntry);

unit zprepare_changed_account_post_storage_roots(unit);

sail_fixed_bytes_32 zcompute_state_root(unit);

unit zvalidation_debug_record(uint64_t, enum zBlockError);

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


void create_letbind_276(void);
void kill_letbind_276(void);


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


uint64_t zsszz_offset_table_position(uint64_t);

struct zBoundedSszzListCursor zsszz_list_cursor(struct zBoundedSszzListRef);

bool zsszz_list_cursor_empty(struct zBoundedSszzListCursor);

struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListCursorz9 zsszz_list_pop(struct zBoundedSszzListCursor);

struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzBoundedSszzzzListRefz9 zsszz_fixed_list_pop(struct zBoundedSszzListRef, uint64_t);

struct zSszzContainerCursor zsszz_container_cursor(struct zByteSliceFields, uint64_t);

struct ztuple_z8z5structz0zzByteSliceFieldszCz0z5structz0zzSszzzzContainerCursorz9 zsszz_take(struct zSszzContainerCursor, uint64_t);

struct zByteSliceFields zsszz_finish(struct zSszzContainerCursor);

struct zBoundedSszzListRef zsszz_bounded_variable_list_ref(struct zByteSliceFields, uint64_t, uint64_t);

struct zBoundedSszzListRef zsszz_bounded_fixed_list_ref(struct zByteSliceFields, uint64_t, uint64_t);

struct zStatelessInputRef zdecode_stateless_input_ref(struct zByteSliceFields);

sail_fixed_bytes_32 zsha256_request_digest(uint64_t, struct zByteSliceFields);

unit zindex_witness_nodes_cursor(struct zBoundedSszzListCursor);

unit zindex_witness_nodes(struct zBoundedSszzListRef);

unit zindex_witness_codes_cursor(struct zBoundedSszzListCursor);

unit zindex_witness_codes(struct zBoundedSszzListRef);

void create_letbind_286(void);
void kill_letbind_286(void);


uint64_t znext_parent_header_field(uint64_t);

struct zParentHeaderFields zdecode_parent_header_fields(struct zRlpCursorFields, uint64_t, struct zParentHeaderFields);

struct zWitnessHeaderIndex zindex_witness_header_cursor(struct zWitnessHeaderIndex);

struct zWitnessContext zindex_witness_headers(struct zBoundedSszzListRef);

uint64_t zdecode_payload_blob_gas_used(struct zByteSliceFields);

struct zBlockHeader zdecode_block_header_sszz(struct zStatelessInputRef);

struct zWithdrawal zdecode_withdrawal(struct zByteSliceFields);

struct zChainConfig zdecode_chain_config(struct zByteSliceFields, uint64_t, uint64_t);

struct zStatelessInput zdecode_stateless_input(struct zStatelessInputRef);

struct zWitnessContext zindex_execution_witness(struct zStatelessInputRef);

void zdecode_transaction(struct zTransaction *rop, struct zByteSliceFields, struct zByteSliceFields);

void create_letbind_287(void);
void kill_letbind_287(void);


uint64_t zbloom_bit_mask(uint64_t);

sail_fixed_bytes_256 zbloom_set_bit(sail_fixed_bytes_256, uint64_t);

sail_fixed_bytes_256 zbloom_add_entry_hash(sail_fixed_bytes_256, sail_fixed_bytes_32);

sail_fixed_bytes_256 zbloom_add_topics(sail_fixed_bytes_256, zz5listz8z5structz0zz__sail_c_repr_u256z9);

sail_fixed_bytes_256 zbloom_add_log(sail_fixed_bytes_256, struct zLogEntry);

sail_fixed_bytes_256 zbloom_add_logs(sail_fixed_bytes_256, zz5listz8z5structz0zzLogEntryz9);

sail_fixed_bytes_256 zlogs_bloom_for_logs(zz5listz8z5structz0zzLogEntryz9);

uint64_t ztopics_rlp_content_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9);

uint64_t ztopics_rlp_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9);

uint64_t zlog_entry_rlp_content_sizze(struct zLogEntry);

uint64_t zlog_entry_rlp_sizze(struct zLogEntry);

uint64_t zlogs_rlp_content_sizze(zz5listz8z5structz0zzLogEntryz9);

uint64_t zlogs_rlp_sizze(zz5listz8z5structz0zzLogEntryz9);

unit zrlp_write_topics_content(zz5listz8z5structz0zz__sail_c_repr_u256z9);

unit zrlp_write_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9);

unit zrlp_write_log_entry(struct zLogEntry);

unit zrlp_write_logs_content(zz5listz8z5structz0zzLogEntryz9);

unit zrlp_write_logs(zz5listz8z5structz0zzLogEntryz9);

uint64_t zreceipt_payload_content_sizze(struct zReceipt, uint64_t);

struct zByteSliceFields zreceipt_encoded(struct zReceipt, uint64_t);

void zreceipt_accumulator_empty(struct zReceiptAccumulator *rop, unit);

void zreceipt_insert(struct zTrieBuilder *rop, struct zTrieBuilder, struct zPendingReceipt, struct zoptionzIRTriePathzK);

void zreceipt_accumulator_push(struct zReceiptAccumulator *rop, struct zReceiptAccumulator, struct zReceipt, uint64_t);

sail_fixed_bytes_32 zreceipt_accumulator_root(struct zReceiptAccumulator);

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


void create_letbind_305(void);
void kill_letbind_305(void);


void create_letbind_306(void);
void kill_letbind_306(void);


void create_letbind_307(void);
void kill_letbind_307(void);


void create_letbind_308(void);
void kill_letbind_308(void);


void create_letbind_309(void);
void kill_letbind_309(void);


uint64_t zenter_system_call_frame(sail_fixed_bytes_20, struct zByteSliceFields);

unit zsystem_call(sail_fixed_bytes_20, sail_fixed_bytes_32);

struct zByteSliceFields zsystem_call_checked(sail_fixed_bytes_20);

bool zdeposit_log_matches(struct zLogEntry);

unit zappend_deposit_request(struct zByteSliceFields);

unit zappend_deposit_logs(zz5listz8z5structz0zzLogEntryz9);

struct zExecutionRequests zcollect_execution_requests(struct zByteSliceFields);

void create_letbind_310(void);
void kill_letbind_310(void);


uint64_t zbal_rlp_length_to_byte_length(uint64_t);

uint64_t zbal_rlp_list_sizze(uint64_t);

uint64_t zbal_previous_index(uint64_t);

uint64_t zbal_index_word_content_sizze(uint64_t, sail_u256);

uint64_t zbal_index_word_sizze(uint64_t, sail_u256);

unit zbal_write_index_word(uint64_t, sail_u256);

uint64_t zbal_index_nonce_content_sizze(uint64_t, uint64_t);

uint64_t zbal_index_nonce_sizze(uint64_t, uint64_t);

unit zbal_write_index_nonce(uint64_t, uint64_t);

uint64_t zbal_index_code_content_sizze(uint64_t, sail_fixed_bytes_32);

uint64_t zbal_index_code_sizze(uint64_t, sail_fixed_bytes_32);

unit zbal_write_index_code(uint64_t, sail_fixed_bytes_32);

uint64_t zbal_storage_change_run_end(uint64_t, uint64_t, sail_u256, uint64_t, uint64_t);

struct zBalContentCursor zbal_storage_slot_changes_sizze(uint64_t, uint64_t, sail_u256, uint64_t, uint64_t);

uint64_t zbal_write_storage_slot_changes(uint64_t, uint64_t, sail_u256, uint64_t);

struct zBalContentCount zbal_storage_change_groups_sizze(uint64_t, uint64_t, uint64_t, struct zBalContentCount);

unit zbal_write_storage_change_groups(uint64_t, uint64_t, uint64_t);

struct zBalContentCount zbal_storage_changes_sizze(uint64_t);

unit zbal_write_storage_changes(uint64_t, struct zBalContentCount);

uint64_t zbal_storage_read_run_end(uint64_t, uint64_t, sail_u256, uint64_t);

uint64_t zbal_storage_change_seek(uint64_t, uint64_t, sail_u256, uint64_t);

struct zBalContentCount zbal_storage_read_groups_sizze(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, struct zBalContentCount);

unit zbal_write_storage_read_groups(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

struct zBalContentCount zbal_storage_reads_sizze(uint64_t);

unit zbal_write_storage_reads(uint64_t, struct zBalContentCount);

uint64_t zbal_balance_run_end(uint64_t, uint64_t, uint64_t, uint64_t);

uint64_t zbal_balance_groups_sizze(uint64_t, uint64_t, uint64_t, uint64_t);

unit zbal_write_balance_groups(uint64_t, uint64_t, uint64_t);

uint64_t zbal_balance_changes_sizze(uint64_t);

unit zbal_write_balance_changes(uint64_t, uint64_t);

struct zBalNonceRun zbal_nonce_run(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

uint64_t zbal_nonce_groups_sizze(uint64_t, uint64_t, uint64_t, uint64_t);

unit zbal_write_nonce_groups(uint64_t, uint64_t, uint64_t);

uint64_t zbal_nonce_changes_sizze(uint64_t);

unit zbal_write_nonce_changes(uint64_t, uint64_t);

uint64_t zbal_code_run_end(uint64_t, uint64_t, uint64_t, uint64_t);

uint64_t zbal_code_groups_sizze(uint64_t, uint64_t, uint64_t, uint64_t);

unit zbal_write_code_groups(uint64_t, uint64_t, uint64_t);

uint64_t zbal_code_changes_sizze(uint64_t);

unit zbal_write_code_changes(uint64_t, uint64_t);

struct zBalAccountSizze zbal_account_sizze(uint64_t);

unit zbal_write_account(uint64_t);

struct zEncodedBlockAccessList zencode_block_access_list(unit);

void create_letbind_311(void);
void kill_letbind_311(void);


uint64_t zremaining_block_gas(uint64_t, uint64_t);

unit zrun_block_start_system_calls(unit);

struct zBlockExecutionResult zexecute_block_transactions(struct zBoundedSszzListRef, struct zByteSliceFields, uint64_t);

unit zapply_withdrawals(struct zBoundedSszzListRef);

unit zapply_block_end_state(struct zBlockBody);

struct zBlockExecutionResult zexecute_block_body(struct zBlockBody, struct zByteSliceFields, uint64_t);

struct zByteSliceFields zwithdrawal_rlp(struct zByteSliceFields);

sail_fixed_bytes_32 zblock_header_hash(struct zBlockHeader, sail_fixed_bytes_32, sail_fixed_bytes_32, sail_fixed_bytes_32, sail_fixed_bytes_32);

sail_fixed_bytes_32 ztransaction_trie_root(struct zBoundedSszzListRef);

sail_fixed_bytes_32 zwithdrawals_trie_root(struct zBoundedSszzListRef);

uint64_t zexpected_payload_excess_blob_gas(struct zWitnessContext);

sail_fixed_bytes_32 zexecution_requests_hash(struct zStatelessInputRef);

unit zvalidate_execution_payload(struct zStatelessInput, struct zStatelessInputRef, struct zWitnessContext);

unit zvalidate_executed_block(struct zBlock, struct zStatelessInputRef, struct zBlockExecutionResult);

void zverify_stateless_payload(struct zStatelessValidationResult *rop, struct zStatelessInputRef);

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


void create_letbind_360(void);
void kill_letbind_360(void);


sail_fixed_bytes_32 zsszz_zzero_hash(uint64_t);

uint64_t zhtr_leaf_capacity(uint64_t);

uint64_t zhtr_depth_increment(uint64_t);

void zmerkle_accumulator_empty(struct zMerkleAccumulator *rop, uint64_t);

void zmerkle_push(zz5listz8z5unionz0zzMerkleSlotz9 *rop, zz5listz8z5unionz0zzMerkleSlotz9, uint64_t, sail_fixed_bytes_32);

void zmerkle_accumulator_push(struct zMerkleAccumulator *rop, struct zMerkleAccumulator, sail_fixed_bytes_32);

sail_fixed_bytes_32 zmerkle_root_levels(zz5listz8z5unionz0zzMerkleSlotz9, uint64_t, sail_fixed_bytes_32, uint64_t, uint64_t);

sail_fixed_bytes_32 zmerkle_accumulator_root(struct zMerkleAccumulator);

void zmerkle_accumulate(struct zMerkleAccumulator *rop, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9, struct zMerkleAccumulator);

sail_fixed_bytes_32 zmerkleizze(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz832z9z9, uint64_t);

sail_fixed_bytes_32 zhtr_uint(uint64_t);

sail_fixed_bytes_32 zhtr_u256(sail_u256);

sail_fixed_bytes_32 zhtr_bytes32(sail_fixed_bytes_32);

sail_fixed_bytes_32 zhtr_addr(sail_fixed_bytes_20);

sail_fixed_bytes_32 zmix_in_length(sail_fixed_bytes_32, uint64_t);

uint64_t zclog2(uint64_t);

sail_fixed_bytes_32 zhtr_chunk(struct zByteSliceFields, uint64_t);

uint64_t zhtr_chunk_count(uint64_t);

sail_fixed_bytes_32 zhtr_bytes_root(struct zByteSliceFields, uint64_t);

sail_fixed_bytes_32 zhtr_bytevector(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_bytelist(struct zByteSliceFields, uint64_t);

sail_fixed_bytes_32 zhtr_withdrawal(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_transactions(struct zBoundedSszzListRef);

sail_fixed_bytes_32 zhtr_withdrawals(struct zBoundedSszzListRef);

sail_fixed_bytes_32 zhtr_execution_payload(struct zStatelessInputRef);

sail_fixed_bytes_32 zhtr_versioned_hashes(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_deposit(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_withdrawal_request(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_consolidation_request(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_builder_deposit_request(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_builder_exit_request(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_deposits(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_withdrawal_requests(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_consolidation_requests(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_builder_deposit_requests(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_builder_exit_requests(struct zByteSliceFields);

sail_fixed_bytes_32 zhtr_execution_requests(struct zStatelessInputRef);

sail_fixed_bytes_32 zhtr_new_payload_request(struct zStatelessInputRef);

void create_letbind_361(void);
void kill_letbind_361(void);


unit zresult_prefix(sail_fixed_bytes_32, bool);

unit zcommit_validation_result(sail_fixed_bytes_32, bool, struct zByteSliceFields);

unit zwrite_validation_result(struct zStatelessInputRef, bool);

unit zwrite_invalid_result(unit);

unit zmain(unit);

unit zinitializze_registers(unit);

sail_u256 zU256zIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128);

sail_u256 zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, sail_u128);

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

struct zBalContentCount zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(uint64_t, uint64_t, struct zBalContentCount);

struct zBalContentCount zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszGa3760b966d2df2175e3b9bd94efd7f00zK(uint64_t, uint64_t, struct zBalContentCount);

uint64_t zbal_bounded_byte_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(uint64_t, sail_u128);

uint64_t zbal_bounded_byte_length_addzIreprzGU64zCU64zCU64zKzIboundszGa3b2b826adddb1d18c77e659efbf6001zK(uint64_t, uint64_t);

uint64_t zbal_bounded_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszG43fd78b15b770f2258241a12de1a7119zK(uint64_t, uint64_t);

uint64_t zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(uint64_t, uint64_t);

uint64_t zbal_rlp_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszGebfa7455a07eda44e5082c02fe62be56zK(uint64_t, sail_u128);

uint64_t zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

uint64_t zbal_rlp_length_from_byte_lengthzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG6a2554a88fdc8146abe3845d0ca1cc6azK(sail_u128);

uint64_t zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

unit zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG6940b3315867b733b4ae91c7ea9466eczK(uint64_t, uint64_t);

sail_u256 zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

uint64_t zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128, uint64_t);

unit zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

struct zMemoryRangeFields zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

unit zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

bool zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(uint64_t);

bool zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

void zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(struct zoptionzIU64zK *rop, uint64_t, uint64_t);

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u256zKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t, sail_fixed_bytes_20);

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC32zKzCR__sail_c_repr_u256zKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t, sail_fixed_bytes_32);

bool zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u128zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20, sail_u128);

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20, uint64_t);

struct zMemoryExpansion zmemory_expansionzIreprzGR__sail_c_repr_u256zCU64zCU64zCRMemoryExpansionzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(sail_u256, uint64_t, uint64_t);

uint64_t zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128);

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG07d9444e226eec3cde1a1e781c91ddf4zK(uint64_t);

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

void zminimal_word_byteszIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, uint64_t);

unit zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zz5listz8z5bv8z9, uint64_t);

uint64_t zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

sail_u256 zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

void zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, uint64_t);

uint64_t zrlp_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zrlp_minimal_word_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

void zrlp_string_prefixzIreprzGU64zCB8zCz8LB8zCU64z9zKzIboundszG7d1a1d23d11135c1c1bff78d34e8a73dzK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *rop, uint64_t, uint64_t);

sail_u128 zrlp_string_sizzezIreprzGU64zCB8zCR__sail_c_repr_u128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

uint64_t zrlp_string_sizzezIreprzGU64zCB8zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(uint64_t, uint64_t);

uint64_t zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

unit zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

unit zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszG718308cac13a033149f52c385b3cc002zK(struct zByteSliceFields, uint64_t);

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszGd430ac7621e2b62bc50ae9cc272dba03zK(struct zByteSliceFields, uint64_t);

struct zByteSliceFields zsszz_fixed_list_atzIreprzGRBoundedSszzListRefzCU64zCU64zCRByteSliceFieldszKzIboundszG09801315d578666d9a3687b217e19ea1zK(struct zBoundedSszzListRef, uint64_t, uint64_t);

struct zByteSliceFields zsszz_list_atzIreprzGRBoundedSszzListRefzCU64zCRByteSliceFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(struct zBoundedSszzListRef, uint64_t);

uint64_t zstate_gas_spill_addzIreprzGU64zCI128zCU64zKzIboundszG1c0bd00f9b9bd1c00190715feb145f1dzK(uint64_t, __int128);

uint64_t zstate_gas_spill_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

sail_fixed_bytes_20 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC20zKzKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t, sail_u256);

sail_fixed_bytes_32 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC32zKzKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t, sail_u256);

uint64_t zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, uint64_t);

__int128 zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128, int64_t);

__int128 zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128, uint64_t);

sail_u256 zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, sail_u128);

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

uint64_t zword_bit_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zword_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u128zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u128, uint64_t);

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, sail_u128);

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

bool zword_greater_than_wordzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

bool zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128);

bool zword_is_zzerozIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

uint64_t zword_low_bytezIreprzGU64zCB8zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, sail_u128);

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

sail_u256 zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

sail_u256 zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128);

sail_u256 zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t);

sail_u256 zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

bool zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t, sail_u256);

bool zword_ultzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256, uint64_t);

extern struct zexception *current_exception;

extern bool have_exception;

extern sail_string *throw_location;

// register zscratch_arena
extern struct zByteSliceFields zscratch_arena;

// register zk_parent_state_root
extern sail_fixed_bytes_32 zk_parent_state_root;

// register zk_n_headers
extern uint64_t zk_n_headers;

// register zk_chain_id
extern uint64_t zk_chain_id;

// register zk_fork
extern enum zFork zk_fork;

// register zk_blob_schedule
extern struct zBlobSchedule zk_blob_schedule;

// register zk_header
extern struct zBlockHeader zk_header;

// register zk_tx
extern struct zTxEnv zk_tx;

// register zpc
extern uint64_t zpc;

// register zgas_remaining
extern uint64_t zgas_remaining;

// register zstate_gas_remaining
extern uint64_t zstate_gas_remaining;

// register zstate_gas_spilled
extern uint64_t zstate_gas_spilled;

// register zframe_refund
extern __int128 zframe_refund;

// register zframe_status
extern struct zFrameStatus zframe_status;

// register zmessage
extern struct zMessage zmessage;

// register zcall_depth
extern uint64_t zcall_depth;

// register zframe_stack
extern zz5vecz8z5unionz0zzFrameContinuationz9 zframe_stack;

// register zframe_stack_top
extern uint64_t zframe_stack_top;

// register zframe_code
extern struct zCode zframe_code;

// register zcalldata
extern struct zByteSliceFields zcalldata;

// register zreturndata
extern struct zByteSliceFields zreturndata;

// register zevm_memory
extern struct zByteSliceFields zevm_memory;



#ifdef __cplusplus
}
#endif
