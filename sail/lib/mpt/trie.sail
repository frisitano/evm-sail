/* Witness-native Ethereum MPT. `trie_root` merges ordered updates into an
 * authenticated base trie and fails closed when a touched hash is absent.
 * With an empty base the same builder computes Yellow Paper TRIE(I). */

/* ---- the witness walker --------------------------------------------------
 *
 * witness_items(node, prefix, updates, fuel): the sorted post-state item
 * stream of the subtree rooted at `node` (whose position is `prefix`) and
 * the first update after that subtree. `updates` is a cursor into the one
 * globally sorted update stream; each call consumes its contiguous prefix
 * range and returns the cursor to its parent.
 *
 * The walker descends ONLY along touched paths; untouched children pass
 * through as single ref items with zero node-db work. Deletes are consumed
 * here and only here: a delete suppresses its base leaf, and a delete with
 * no base leaf (the walk proves absence) emits nothing. Whatever survives
 * is rebuilt by build_group, so a branch losing children to deletes
 * collapses in the builder with no walker involvement. RLP fields retain their
 * source and spans, so embedded nodes and leaf values remain witness slices. */
val witness_items : (Bytes, TriePath, list(TrieUpdate), int) -> (list(TrieItem), list(TrieUpdate))
val witness_child_items : (RlpFieldRef, TriePath, list(TrieUpdate), int) -> (list(TrieItem), list(TrieUpdate))

function witness_child_items(
    f : RlpFieldRef,
    prefix : TriePath,
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    (list(TrieItem), list(TrieUpdate))
) = {
    match field_to_ref(f) {
        InlineRef(node) => witness_items(node, prefix, updates, fuel),
        EmptyRef() => {
            /* proven-absent subtree: the surviving updates are pure inserts */
            live_update_items_under(updates, prefix)
        },
        HashRef(h) => {
            let node = node_db_lookup(h);
            if bytes_len(node) == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                witness_items(node, prefix, updates, fuel)
            }
        },
    }
}

function witness_items(
    node : Bytes,
    prefix : TriePath,
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    (list(TrieItem), list(TrieUpdate))
) = {
    if fuel <= 0 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        if bytes_len(node) == 0 then {
            live_update_items_under(updates, prefix)
        } else {
            match decode_trie_node(node) {
                LeafNode(leaf) => {
                    let key = path_concat(prefix, leaf.path);
                    leaf_overlay_items(updates, prefix, key, rlp_ref_content(leaf.value))
                },
                ExtensionNode(ext) => {
                    let key = path_concat(prefix, ext.path);
                    var before_rev : list(TrieItem) = [||];
                    var remaining : list(TrieUpdate) = updates;
                    var before_child : bool = true;

                    /* Diverging updates below `prefix` but ordered before the
                       extension child are emitted until the cursor reaches the
                       child range or the next sibling subtree. */
                    while before_child do {
                        match remaining {
                            [||] => before_child = false,
                            update :: rest => if   not_bool(path_prefix_of(prefix, update.key))
                                                 | path_prefix_of(key, update.key)
                                                 | not_bool(path_lt(update.key, key)) then {
                                before_child = false
                            } else {
                                match update.change {
                                    TrieDelete() => (),
                                    TriePut(value) => before_rev = item_leaf(update.key, value) :: before_rev,
                                };
                                remaining = rest
                            },
                        }
                    };

                    let (sub, after_sub) = if next_update_under(remaining, key) then {
                            witness_child_items(ext.child, key, remaining, fuel - 1)
                        } else {
                            /* untouched extension child: one whole-subtree
                               ref; an extension's child is always a branch */
                            ([|item_branch(key, field_to_ref(ext.child))|], remaining)
                        };
                    let (after, next) = live_update_items_under(after_sub, prefix);
                    (list_append(list_reverse(before_rev), list_append(sub, after)), next)
                },
                BranchNode(branch) => {
                    /* a branch value at slot 16 is impossible for 64-nibble hashed
                       keys (no key is a prefix of another): a malformed witness. */
                    if unsigned(branch.value.content_len) > 0 then {
                        throw InvalidBlock(WitnessDeficient)
                    };
                    var out_rev : list(TrieItem) = [||];
                    var remaining : list(TrieUpdate) = updates;
                    foreach (i from 0 to 15) {
                        let nib : nibble = get_slice_int(4, i, 0);
                        let field = branch.children[i];
                        let child_prefix = path_concat(prefix, path_single(nib));

                        /* a child slot holds an embedded node, a 32-byte hash,
                           or nothing (field_to_ref of anything else is empty) */
                        let childref = field_to_ref(field);
                        let present : bool = match childref {
                            EmptyRef() => false,
                            _ => true,
                        };
                        let (contrib, next) = if next_update_under(remaining, child_prefix) then {
                                if present then {
                                    witness_child_items(field, child_prefix, remaining, fuel - 1)
                                } else {
                                    /* no base child: the surviving updates are
                                       pure inserts */
                                    live_update_items_under(remaining, child_prefix)
                                }
                            } else {
                                if present then {
                                    /* untouched branch child: node type unknown
                                       for hash refs (is_branch = false) */
                                    ([|item_subtree(child_prefix, childref)|], remaining)
                                } else {
                                    ([||], remaining)
                                }
                            };
                        out_rev = rev_onto(contrib, out_rev);
                        remaining = next
                    };
                    (list_reverse(out_rev), remaining)
                },
                InvalidNode() => {
                    let (_, remaining) = live_update_items_under(updates, prefix);
                    ([||], remaining)
                },
            }
        }
    }
}
termination_measure witness_items(node, prefix, updates, fuel) = fuel
termination_measure witness_child_items(f, prefix, updates, fuel) = fuel

/* ---- THE trie entry ------------------------------------------------------
 *
 * trie_root(base_root, updates): the root of the trie anchored at base_root
 * after applying the ordered update list. This is the ONLY public root
 * computation: witness-native and fail-closed -- the walker resolves every
 * touched hash reference in the witness node-db and any missing node throws
 * InvalidBlock(WitnessDeficient); otherwise the builder recomposes the emitted
 * stream canonically.
 *
 * Theorem-shaped remark: restricted to an empty base
 * (base_root = EMPTY_TRIE_ROOT), the walker is the identity on the live
 * update leaves and trie_root computes TRIE(I) of Appendix D directly -- an
 * empty base contains no hash refs, so the node-db is never consulted and
 * no failure path can fire. The native (full-state) backend exercises
 * exactly this restriction: same implementation, different input. */
function trie_root(base_root : bits(256), updates : list(TrieUpdate)) -> bits(256) = {
    if updates_empty(updates) then {
        base_root
    } else {
        if base_root == EMPTY_TRIE_ROOT then {
            items_root(live_update_items(updates))
        } else {
            let node = node_db_lookup(base_root);
            if bytes_len(node) == 0 then {
                throw InvalidBlock(WitnessDeficient)
            } else {
                let (items, remaining) = witness_items(node, path_empty(), updates, 256);
                if updates_empty(remaining) then {
                    items_root(items)
                } else {
                    throw InvalidBlock(WitnessDeficient)
                }
            }
        }
    }
}

/* Walk the secure trie without copying its leaf value. */
function trie_walk(node : Bytes, key : TriePath, pos : int, fuel : int) -> Bytes = {
    if fuel <= 0 | bytes_len(node) == 0 then {
        BytesList([||])
    } else {
        match decode_trie_node(node) {
            LeafNode(leaf) => if not_bool(path_matches(key, pos, leaf.path)) then {
                BytesList([||])
            } else {
                if pos + path_len(leaf.path) == path_len(key) then {
                    rlp_ref_content(leaf.value)
                } else {
                    BytesList([||])
                }
            },
            ExtensionNode(ext) => if not_bool(path_matches(key, pos, ext.path)) then {
                BytesList([||])
            } else {
                trie_walk(resolve_ref(field_to_ref(ext.child)), key, pos + path_len(ext.path), fuel - 1)
            },
            BranchNode(branch) => {
                if pos == path_len(key) then {
                    rlp_ref_content(branch.value)
                } else {
                    let child = resolve_ref(field_to_ref(branch.children[unsigned(path_nibble(key, pos))]));
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            },
            InvalidNode() => BytesList([||]),
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

function trie_lookup(root : hash, key : TriePath) -> Bytes = {
    if root == EMPTY_TRIE_ROOT then {
        BytesList([||])
    } else {
        let node = node_db_lookup(root);
        if bytes_len(node) == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            trie_walk(node, key, 0, 256)
        }
    }
}
