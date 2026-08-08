#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void k_selfdestruct(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.selfdestructed) {
    return;
  }
  struct Account tmp_3_3187 = cur;
  tmp_3_3187.selfdestructed = true;
  store_account_(a, tmp_3_3187);
}

bool k_is_selfdestructed(bytes20 a)
{
  struct Account k_aload_result_2_1842 = k_aload_(a);
  return k_aload_result_2_1842.selfdestructed;
}

void k_mark_created(bytes20 a)
{
  struct Account cur = k_aload_(a);
  struct Account tmp_3_3186 = cur;
  tmp_3_3186.created = true;
  store_account_(a, tmp_3_3186);
}

bool k_was_created(bytes20 a)
{
  struct Account k_aload_result_2_1840 = k_aload_(a);
  return k_aload_result_2_1840.created;
}

void k_zero_balance(bytes20 a)
{
  struct Account cur = k_aload_(a);
  bool balance_is_zero = word_is_zero(cur.info.balance);
  if (balance_is_zero) {
    return;
  }
  struct AccountInfo tmp_3_3185 = cur.info;
  tmp_3_3185.balance = ZERO_WORD;
  store_account_info_(a, cur, tmp_3_3185);
}

