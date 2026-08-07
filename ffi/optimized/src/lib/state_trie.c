/* Optimized Ethereum state-trie root boundary. */
#include "evmsail/prelude.h"

#include "evmsail/spec/kernel/environment.h"
#include "lib/mpt/trie.h"
bytes32 compute_state_root(void)
{
  return mpt_compute_state_root(k_parent_state_root);
}
