#pragma once

#include "evmsail/spec/host/output.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum EnvField
enum EnvField { F_Number, F_Timestamp, F_Coinbase, F_BaseFee, F_ChainId, F_GasLimit, F_PrevRandao, F_Origin, F_GasPrice, F_SlotNumber };

u256 k_env(enum EnvField f);

bytes20 k_coinbase(void);

u256 blockhash_word_distance(u256 current, u256 number);

bytes32 k_blockhash(u256 number_word);

u256 k_blobhash(u256 index_word);

bytes20 k_create_addr(bytes20 a, uint64_t nonce);

bytes20 k_create2_addr(bytes20 a, u256 salt, bytes32 inithash);

// register zk_parent_state_root
extern bytes32 k_parent_state_root;

// register zk_n_headers
extern uint16_t k_n_headers;

// register zk_chain_id
extern uint64_t k_chain_id;

// register zk_execution_profile
extern struct ExecutionProfileFields k_execution_profile;

// register zk_header
extern struct BlockHeader k_header;

// register zk_tx
extern struct TxEnvFields k_tx;

// register zk_current_transaction_epoch
extern uint32_t k_current_transaction_epoch;


#ifdef __cplusplus
}
#endif
