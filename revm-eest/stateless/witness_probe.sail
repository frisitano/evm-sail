/* ======================================================================== */
/* WITNESS RE-ROOT PROBE                                                    */
/*                                                                          */
/* A native probe for the stateless witness reader: re-root a state-trie    */
/* witness and verify it against the committed root. This exercises the     */
/* trie parsing shared with lib/mpt.sail (node-db, RLP node decoding,       */
/* hex-prefix paths, child references) directly, without block execution.   */
/*                                                                          */
/* The eager re-root walk itself lives HERE, not in lib/mpt.sail: block     */
/* validation authenticates witness material lazily (walks from the         */
/* anchored parent root, each a hash-chain proof through the keccak(node)-  */
/* keyed node-db), so the full-trie walk survives only as this probe's      */
/* debugging device.                                                        */
/*                                                                          */
/* Input contract: a schema-prefixed SSZ SszStatelessInput read from stdin  */
/* through ssz_src (host/io.sail), the private-input byte source the zkVM   */
/* guest also uses. build_node_db loads the witness node store from it;     */
/* parent_state_root returns the committed pre-state root carried in the    */
/* input header.                                                            */
/*                                                                          */
/* witness_reroot walks the ENTIRE witness trie from that root recursively, */
/* so REROOT == PARENT certifies that every node (branch / extension / leaf,*/
/* hex-prefix paths, embedded children) was parsed correctly. Output: a     */
/* "PARENT <hex>" and a "REROOT <hex>" line for the harness to diff.        */
/*                                                                          */
/* The harness revm-eest/stateless/run_witness.py builds one input per EEST */
/* `pre` alloc with a real py-trie witness and diffs REROOT against the py- */
/* trie root, covering the full diversity of EEST pre-state trie shapes --  */
/* the role execution-specs' stateless mode gives witness_state.py.         */
/* ======================================================================== */

$include <string.sail>

/* Probe-local branch reassembly (formerly lib/mpt.sail): tolerate a branch
 * left with a single present child (non-canonical witness input) by merging
 * it into an extension. The production builder never needs this -- its
 * branches have >= 2 children by construction. */
function mask_single_index(mask : bits(16)) -> int = {
    var ix : int = 0;
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            ix = i
        }
    };
    ix
}

function first_child_ref(children : list(list(byte))) -> list(byte) =
    match children {
        c :: _ => c,
        [||] => trie_empty_ref(),
    }

function canonical_branch_ref(mask : bits(16), children : list(list(byte))) -> list(byte) =
    if mask == 0x0000 then {
        trie_empty_ref()
    } else {
        if branch_mask_popcount(mask) == 1 then {
            merge_ext_ref([|get_slice_int(4, mask_single_index(mask), 0)|], first_child_ref(children))
        } else {
            branch_child_ref(mask, children)
        }
    }

/* Recursive re-root walk: present children are traversed, child hashes
 * missing from the node-db are preserved as blinded frontier references.
 * No leaves are recorded -- the probe only certifies parsing. */
val witness_node_ref : (list(byte), int) -> list(byte)
val witness_child_ref : (RlpField, int) -> list(byte)

/* child field: descend if present, preserve if blinded */
function witness_child_ref(f : RlpField, fuel : int) -> list(byte) = {
    if f.is_list then {
        witness_node_ref(f.full, fuel)
    } else {
        let c = f.content;
        if list_len(c) == 32 then {
            let node = node_db_lookup(bytes_be256(c));
            if list_len(node) == 0 then {
                hash_child_ref(c)
            } else {
                witness_node_ref(node, fuel)
            }
        } else {
            trie_empty_ref()
        }
    }
}

function witness_node_ref(node : list(byte), fuel : int) -> list(byte) = {
    if fuel <= 0 | list_len(node) == 0 then {
        trie_empty_ref()
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hex_prefix_decode(list_nth(items, 0, RLP_NONE).content);
            if is_leaf then {
                leaf_child_ref(seg, list_nth(items, 1, RLP_NONE).content)
            } else {
                merge_ext_ref(seg, witness_child_ref(list_nth(items, 1, RLP_NONE), fuel - 1))
            }
        } else {
            if n == 17 then {
                var mask : bits(16) = 0x0000;
                var children : list(list(byte)) = [||];
                foreach (i from 0 to 15) {
                    let cref = witness_child_ref(list_nth(items, i, RLP_NONE), fuel - 1);
                    if not_bool(trie_ref_is_empty(cref)) then {
                        mask = branch_mask_set_bit(mask, i);
                        children = list_append(children, [|cref|])
                    }
                };
                canonical_branch_ref(mask, children)
            } else {
                trie_empty_ref()
            }
        }
    }
}
termination_measure witness_node_ref(node, fuel) = fuel
termination_measure witness_child_ref(f, fuel) = fuel

/* re-root the trie at `root` from the witness node-db */
function witness_reroot(root : bits(256)) -> bits(256) =
    if root == EMPTY_TRIE_ROOT then {
        EMPTY_TRIE_ROOT
    } else {
        let rootnode = node_db_lookup(root);
        if list_len(rootnode) == 0 then {
            trie_ref_to_root(trie_empty_ref())
        } else {
            trie_ref_to_root(witness_node_ref(rootnode, 256))
        }
    }

function main() -> unit = {
    build_node_db();
    let parent = parent_state_root();
    let rerooted = witness_reroot(parent);
    print_endline(concat_str("PARENT ", hex_str(unsigned(parent))));
    print_endline(concat_str("REROOT ", hex_str(unsigned(rerooted))))
}
