/* Witness-reader probe (host, native).
 *
 * Reads a schema-prefixed SSZ `SszStatelessInput` from stdin (via ssz_src, the
 * same byte source the zkVM guest uses), builds the witness node-db, and
 * re-roots the state trie from it. `witness_reroot` walks the ENTIRE witness
 * trie from the root and rebuilds it through the hash builder, so
 * `REROOT == PARENT` certifies that every node (branch / extension / leaf,
 * hex-prefix, embedded children) was parsed correctly.
 *
 * The harness (revm-eest/stateless/run_witness.py) builds one such input per
 * EEST `pre` alloc with a real py-trie witness, then diffs REROOT against the
 * py-trie root — giving the witness reader (lib/mpt.sail) coverage across the
 * full diversity of EEST pre-state trie shapes, the way execution-specs'
 * stateless mode exercises witness_state.py. */

$include "../../evm/evm.sail"
$include <string.sail>

function main() -> unit = {
    build_node_db();
    let parent = parent_state_root();
    let rerooted = witness_reroot(parent);
    print_endline(concat_str("PARENT ", hex_str(unsigned(parent))));
    print_endline(concat_str("REROOT ", hex_str(unsigned(rerooted))))
}
