#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

void k_selfdestruct(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (cur.selfdestructed) {
    return;
  }
  store_account_(a, ((struct Account){.created = cur.created, .info = cur.info, .present = cur.present, .selfdestructed = true, .storage_cleared = cur.storage_cleared}));
}

bool k_is_selfdestructed(bytes20 a)
{
  return (k_aload_(a)).selfdestructed;
}

void k_mark_created(bytes20 a)
{
  struct Account cur = k_aload_(a);
  store_account_(a, ((struct Account){.created = true, .info = cur.info, .present = cur.present, .selfdestructed = cur.selfdestructed, .storage_cleared = cur.storage_cleared}));
}

bool k_was_created(bytes20 a)
{
  return (k_aload_(a)).created;
}

void k_zero_balance(bytes20 a)
{
  struct Account cur = k_aload_(a);
  if (eq_u256(cur.info.balance, WORD_ZERO)) {
    return;
  }
  store_account_info_(a, cur, ((struct AccountInfo){.balance = ZERO_WORD, .code_hash = cur.info.code_hash, .nonce = cur.info.nonce, .storage_root = cur.info.storage_root}));
}

