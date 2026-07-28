#ifndef EVMSAIL_OPTIMIZED_STATE_H
#define EVMSAIL_OPTIMIZED_STATE_H

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

struct zAccount evmsail_k_aload(sail_fixed_bytes_32 parent_state_root,
                                sail_fixed_bytes_20 address);
struct zStorageValue evmsail_k_sload(sail_fixed_bytes_32 parent_state_root,
                                     sail_fixed_bytes_20 address, sail_u256 slot);
unit evmsail_k_sstore(sail_fixed_bytes_20 address, sail_u256 slot,
                      sail_u256 current, sail_u256 original);
unit evmsail_store_account(sail_fixed_bytes_20 address, struct zAccount account);
unit evmsail_store_account_info(sail_fixed_bytes_20 address,
                                struct zAccount account,
                                struct zAccountInfo info);

#endif
