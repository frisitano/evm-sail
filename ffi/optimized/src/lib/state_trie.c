/* Optimized Ethereum state-trie root boundary. */
#include "evmsail/prelude.h"

#include "lib/mpt/trie.h"
#include "evmsail/lib/state_trie.h"

Hash32
compute_state_root(unit ignored) {
  (void)ignored;
  return mpt_compute_state_root(k_parent_state_root);
}
