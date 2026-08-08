/* C-backed transaction runtime collections for evm-sail.
 *
 * EIP-2929 warm access and EIP-7702 authorization tracking live behind the
 * abstract host interfaces declared in Sail. Logs and the append-only state
 * journal are separate subsystems in logs.c and journal.c respectively.
 *
 * Addresses, words, and hashes cross through the selected standard/optimized
 * ABI and are stored without Sail runtime wrappers; counts, tags, and bytes
 * cross as scalars. Each collection owns fixed workspace storage, resets only
 * its live count, and fails closed before a write can exceed its capacity. */
#include "host/state/runtime.h"
#include "evmsail/prelude.h"
#include "host/state/internal.h"
#include "workspace.h"
#include "primitives/value.h"
#include <stdint.h>
#include <string.h>

/* ------------------------------ warm sets ------------------------------- */
/* The protocol epoch is also the BAL index: zero denotes pre-execution,
 * transactions use index + 1, and the final index denotes post-execution.
 * Warm rows store that epoch plus one so zero remains the unwarmed/rollback
 * sentinel without conflating it with the pre-execution system-call phase. */
uint32_t current_warm_epoch;

void warm_reset(uint32_t current_transaction_epoch)
{
  /* The Sail argument type bounds the epoch to at most 2^20 + 1. */
  current_warm_epoch = current_transaction_epoch + 1;
}
/* --------------------- EIP-7702 authority tracker ---------------------- */

typedef struct {
  bytes20 key;
  uint32_t epoch;
  uint8_t originally_delegated;
  uint8_t delegation_set;
} auth_tracker_entry;

static auth_tracker_entry *auth_tracker;
static uint32_t auth_tracker_limit;
static uint32_t auth_tracker_epoch = 1U;

static uint64_t auth_tracker_hash(const bytes20 *a)
{
  uint64_t h = UINT64_C(1469598103934665603);
  const uint8_t *bytes = bytes20_data(a);
  for (unsigned i = 0; i < 20; ++i) {
    h ^= bytes[i];
    h *= UINT64_C(1099511628211);
  }
  h ^= h >> 32;
  h *= UINT64_C(0xd6e8feb86659fd93);
  h ^= h >> 32;
  return h;
}

static auth_tracker_entry *auth_tracker_find(const bytes20 *key, int insert)
{
  uint32_t slot = (uint32_t)auth_tracker_hash(key) & (auth_tracker_limit - 1);
  for (;;) {
    auth_tracker_entry *entry = &auth_tracker[slot];
    if (entry->epoch != auth_tracker_epoch) {
      if (!insert) {
        return NULL;
      }
      entry->epoch = auth_tracker_epoch;
      entry->key = *key;
      entry->originally_delegated = 0;
      entry->delegation_set = 0;
      return entry;
    }
    if (address_equal(&entry->key, key)) {
      return entry;
    }
    slot = (slot + 1) & (auth_tracker_limit - 1);
  }
}

void authorization_tracker_reset(uint16_t count_hint)
{
  uint64_t need = count_hint < 8 ? 16 : UINT64_C(2) * count_hint;
  uint32_t cap = 16;
  while ((uint64_t)cap < need) {
    cap *= 2;
  }
  auth_tracker_limit = cap;
  auth_tracker_epoch++;
  if (auth_tracker_epoch == 0) {
    memset(auth_tracker, 0, GUEST_AUTHORIZATION_ENTRIES * sizeof(*auth_tracker));
    auth_tracker_epoch = 1U;
  }
}

bool authorization_tracker_seen(bytes20 authority)
{
  return auth_tracker_find(&authority, 0) != NULL;
}

bool authorization_tracker_originally_delegated(bytes20 authority)
{
  auth_tracker_entry *entry = auth_tracker_find(&authority, 0);
  return (entry != NULL && entry->originally_delegated != 0) != 0;
}

bool authorization_tracker_delegation_set(bytes20 authority)
{
  auth_tracker_entry *entry = auth_tracker_find(&authority, 0);
  return (entry != NULL && entry->delegation_set != 0) != 0;
}

void authorization_tracker_commit(bytes20 authority, bool originally_delegated,
                                  bool sets_delegation)
{
  auth_tracker_entry *entry = auth_tracker_find(&authority, 1);
  if (originally_delegated) {
    entry->originally_delegated = 1;
  }
  if (sets_delegation) {
    entry->delegation_set = 1;
  }
}

void state_runtime_workspace_bind(void)
{
  WORKSPACE_BIND(auth_tracker, GUEST_AUTHORIZATION_ENTRIES);
}
