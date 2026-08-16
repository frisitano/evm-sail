#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void k_selfdestruct(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.selfdestructed) {
    return;
  }
  struct Account tmp_3_3114 = cur;
  tmp_3_3114.selfdestructed = true;
  store_account_(a, tmp_3_3114);
}

bool k_is_selfdestructed(bytes20 a)
{
  struct Account k_aload_result_2_2038 = k_aload_(a);
  return k_aload_result_2_2038.selfdestructed;
}

void k_mark_created(bytes20 a)
{
  struct Account cur = k_aload_(a);
  struct Account tmp_3_3113 = cur;
  tmp_3_3113.created = true;
  store_account_(a, tmp_3_3113);
}

bool k_was_created(bytes20 a)
{
  struct Account k_aload_result_2_2036 = k_aload_(a);
  return k_aload_result_2_2036.created;
}

void k_zero_balance(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (eq_u256(cur.info.balance, WORD_ZERO)) {
    return;
  }
  struct AccountInfo tmp_3_3112 = cur.info;
  tmp_3_3112.balance = ZERO_WORD;
  store_account_info_(a, cur, tmp_3_3112);
}

