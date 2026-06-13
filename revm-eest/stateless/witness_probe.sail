/* ======================================================================== */
/* WITNESS RE-ROOT PROBE                                                    */
/*                                                                          */
/* A native probe for the stateless pre-state check: re-root a state-trie   */
/* witness and verify it against the committed root. This isolates the      */
/* witness reader (lib/mpt.sail) from block execution so its trie parsing   */
/* can be exercised directly.                                               */
/*                                                                          */
/* Input contract: a schema-prefixed SSZ SszStatelessInput read from stdin  */
/* through ssz_src (host/io.sail), the private-input byte source the zkVM    */
/* guest also uses. build_node_db loads the witness node store from it;      */
/* parent_state_root returns the committed pre-state root carried in the     */
/* input header.                                                            */
/*                                                                          */
/* witness_reroot walks the ENTIRE witness trie from that root and rebuilds  */
/* it through the hash builder, so REROOT == PARENT certifies that every     */
/* node (branch / extension / leaf, hex-prefix paths, embedded children) was */
/* parsed correctly. Output: a "PARENT <hex>" and a "REROOT <hex>" line for  */
/* the harness to diff.                                                     */
/*                                                                          */
/* The harness revm-eest/stateless/run_witness.py builds one input per EEST  */
/* `pre` alloc with a real py-trie witness and diffs REROOT against the      */
/* py-trie root, covering the full diversity of EEST pre-state trie shapes   */
/* -- the role execution-specs' stateless mode gives witness_state.py.       */
/* ======================================================================== */

$include "../../evm/evm.sail"
$include <string.sail>

function main() -> unit = {
    build_node_db();
    let parent = parent_state_root();
    let rerooted = witness_reroot(parent);
    print_endline(concat_str("PARENT ", hex_str(unsigned(parent))));
    print_endline(concat_str("REROOT ", hex_str(unsigned(rerooted))))
}
