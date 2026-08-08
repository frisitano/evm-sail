#pragma once

#include "evmsail/spec/lib/rlp/codecs/transactions.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void state_journal_reset(void);

void state_journal_checkpoint(void);

void state_journal_revert(void);

void state_journal_commit(void);


#ifdef __cplusplus
}
#endif
