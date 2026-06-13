/* Differential check: the alloy-trie stack HashBuilder (hb_root) must agree
 * with the recursive trie_root oracle on the same key-sorted leaf sets. */

$include "../evm/evm.sail"
$include "evm_primitives.sail"
$include <string.sail>

function main() -> unit = {
    hb_selfcheck();
    print_endline("hash_builder: stack root == recursive trie_root on all sets  OK");
    hb_blind_selfcheck();
    print_endline("hash_builder: blinded partial root == full root (both orders)  OK")
}
