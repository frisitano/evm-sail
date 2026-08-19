#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void k_journal_checkpoint(void)
{
  state_journal_checkpoint();
}

void k_set_header(struct BlockHeader h)
{
  k_header = h;
}

void k_set_tx(struct TxEnvFields env)
{
  k_tx = env;
}

void k_tx_reset(void)
{
  storage_tx_reset();
  acct_tx_reset();
  warm_reset(k_current_transaction_epoch);
  transient_storage_reset();
  logs_tx_reset();
  state_journal_reset();
}

struct TransactionMergeSemantics transaction_merge_semantics(uint8_t fork)
{
  if (fork >= Amsterdam) {
    return ((struct TransactionMergeSemantics){.delete_only_created = true, .preserve_selfdestruct_balance = true});
  }
  if (fork >= Cancun) {
    return ((struct TransactionMergeSemantics){.delete_only_created = true, .preserve_selfdestruct_balance = false});
  }
  return ((struct TransactionMergeSemantics){.delete_only_created = false, .preserve_selfdestruct_balance = false});
}

void k_tx_merge(void)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  tx_merge((transaction_merge_semantics(execution_profile.protocol.fork)), k_current_transaction_epoch);
}

void k_journal_revert(void)
{
  state_journal_revert();
}

void k_journal_commit(void)
{
  state_journal_commit();
}

