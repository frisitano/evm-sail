#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void k_selfdestruct(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.selfdestructed) {
    return;
  }
  struct Account tmp_3_3373 = cur;
  tmp_3_3373.selfdestructed = true;
  store_account_(a, tmp_3_3373);
}

bool k_is_selfdestructed(bytes20 a)
{
  struct Account k_aload_result_2_2059 = k_aload_(a);
  return k_aload_result_2_2059.selfdestructed;
}

void k_mark_created(bytes20 a)
{
  struct Account cur = k_aload_(a);
  struct Account tmp_3_3372 = cur;
  tmp_3_3372.created = true;
  store_account_(a, tmp_3_3372);
}

bool k_was_created(bytes20 a)
{
  struct Account k_aload_result_2_2057 = k_aload_(a);
  return k_aload_result_2_2057.created;
}

void k_zero_balance(bytes20 a)
{
  struct Account cur = k_aload_(a);
  bool balance_is_zero = eq_u256(cur.info.balance, WORD_ZERO);
  if (balance_is_zero) {
    return;
  }
  struct AccountInfo tmp_3_3371 = cur.info;
  tmp_3_3371.balance = ZERO_WORD;
  store_account_info_(a, cur, tmp_3_3371);
}

