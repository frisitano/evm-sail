#pragma once

#include "evmsail/spec/kernel/selfdestruct.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct TransactionMergeSemantics
struct TransactionMergeSemantics {
  bool delete_only_created;
  bool preserve_selfdestruct_balance;
};

void k_journal_checkpoint(void);

void k_set_header(struct BlockHeader h);

void k_set_tx(struct TxEnvFields env);

void k_tx_reset(void);

struct TransactionMergeSemantics transaction_merge_semantics(uint8_t fork);

void tx_merge(struct TransactionMergeSemantics /* arg_0 */, uint32_t /* arg_1 */);

void k_tx_merge(void);

void k_journal_revert(void);

void k_journal_commit(void);


#ifdef __cplusplus
}
#endif
