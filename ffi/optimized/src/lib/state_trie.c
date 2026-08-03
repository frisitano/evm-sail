/* Optimized Ethereum state-trie root boundary. */
#include "evmsail/prelude.h"

#include "lib/mpt/trie.h"
#include "evmsail/lib/state_trie.h"

Hash32
compute_state_root(Hash32 parent_state_root) {
  return mpt_compute_state_root(parent_state_root);
}
