#ifndef EVMSAIL_JOURNAL_GLUE_H
#define EVMSAIL_JOURNAL_GLUE_H

/*
 * Whole-operation optimized state access crosses generated Account and
 * StorageValue layouts. Include the generated model header before declaring
 * those functions so the layouts continue to be owned solely by Sail.
 *
 * This header is injected only into optimized C builds. Standard C and proof
 * extraction retain the explicit Sail implementations.
 */
#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include "sail_abi.h"

void evmsail_k_aload(struct zOptimizzedAccountResult *result,
                     sail_hash parent_state_root, sail_address address);
void evmsail_k_sload(struct zOptimizzedStorageResult *result,
                     sail_hash parent_state_root, sail_address address,
                     sail_word slot);
unit evmsail_k_sstore(sail_address address, sail_word slot,
                      sail_word current, sail_word original);
unit evmsail_store_account(sail_address address, struct zAccount account);
unit evmsail_store_account_info(sail_address address,
                                struct zAccount account,
                                struct zAccountInfo info);

#endif
