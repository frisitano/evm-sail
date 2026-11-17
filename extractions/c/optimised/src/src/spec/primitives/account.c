#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct AccountInfo EMPTY_ACCOUNT_INFO;

struct Account EMPTY_ACCOUNT;

void create_letbind_71(void)
{

  EMPTY_ACCOUNT_INFO = ((struct AccountInfo){.balance = ZERO_WORD,
                                             .code_hash = KECCAK_EMPTY,
                                             .nonce = UINT64_C(0),
                                             .storage_root = EMPTY_TRIE_ROOT});
}
void kill_letbind_71(void) {}

void create_letbind_72(void)
{

  EMPTY_ACCOUNT = ((struct Account){.created = false,
                                    .info = EMPTY_ACCOUNT_INFO,
                                    .present = false,
                                    .selfdestructed = false,
                                    .storage_cleared = true});
}
void kill_letbind_72(void) {}

struct Account account_from_info(struct AccountInfo info)
{
  return ((struct Account){.created = false,
                           .info = info,
                           .present = true,
                           .selfdestructed = false,
                           .storage_cleared = false});
}
