/* Fixed-size BLOCKHASH ancestor table.
 *
 * Index d holds the ancestor hash at distance d + 1 (see
 * sail/host/environment.sail). Sail's ancestor_index type is range(0, 255)
 * and k_blockhash bounds reads by the witnessed header count, so an
 * out-of-range index here is an invariant breach, not a protocol condition. */
#include "evmsail/prelude.h"

#include "evmsail/host/environment.h"
#include "workspace.h"

#include <string.h>

static Hash32 *ancestor_hashes;

void environment_workspace_bind(void) {
  WORKSPACE_BIND(ancestor_hashes, GUEST_ANCESTOR_HASHES);
}

unit environment_reset(const unit u) {
  (void)u;
  memset(ancestor_hashes, 0,
         GUEST_ANCESTOR_HASHES * sizeof(*ancestor_hashes));
  return UNIT;
}

unit ancestor_hash_write(uint8_t index, Hash32 hash) {
  ancestor_hashes[index] = hash;
  return UNIT;
}

Hash32 ancestor_hash_read(uint8_t index) {
  return ancestor_hashes[index];
}
