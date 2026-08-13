#pragma once

#include "evmsail/spec/kernel/logs.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bool account_info_empty(struct AccountInfo info);

struct Account account_clear_storage(struct Account acc);

void store_account_(bytes20 a, struct Account account);

void store_account_info_(bytes20 a, struct Account account, struct AccountInfo info);

u256 k_get_balance(bytes20 a);

uint64_t k_get_nonce(bytes20 a);

bool k_account_exists(bytes20 a);

bool k_account_is_empty(bytes20 a);

bool k_account_occupied(bytes20 a);

void k_transfer(bytes20 src, bytes20 dst, u256 v);

void k_bump_nonce(bytes20 a);

void k_add_balance(bytes20 a, u256 v);

void k_sub_balance(bytes20 a, u256 v);

void k_clear_storage(bytes20 a);

void k_add_balance_bytes20_u128_to_unit(bytes20 a, u128 v);

void k_add_balance_bytes20_uint64_t_to_unit(bytes20 a, uint64_t v);


#ifdef __cplusplus
}
#endif
