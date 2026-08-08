#pragma once

#include "evmsail/spec/executor/stateless.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// enum HtrRequestKind
enum HtrRequestKind { HtrDeposit, HtrWithdrawalRequest, HtrConsolidationRequest, HtrBuilderDepositRequest, HtrBuilderExitRequest };

bytes32 htr_new_payload_request_(struct StatelessInputRef input_ref);


#ifdef __cplusplus
}
#endif
