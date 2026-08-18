
/* Updates and surviving witness subtrees are recomposed directly by a
 * recursive trie walk. Each level partitions the ordered update cursor by
 * child nibble and returns at most one structural item to its parent. */

/*md
# The witness-native trie

The witness-native Ethereum Merkle-Patricia trie. [trie_root][] merges
ordered updates into an authenticated base trie and fails closed when a
touched hash is absent from the witness. With an empty base the same
builder computes the Yellow Paper `TRIE(I)` directly.

## The witness walker

`witness_subtree(node, prefix, updates, cursor)` returns the post-state
subtree rooted at `node` together with the first update after that subtree.
`updates` is a cursor into the one globally sorted update stream; each call
consumes its contiguous prefix range and returns the cursor to its parent.

The walker descends only along touched paths; untouched children pass
through as single reference items with zero node-db work. Deletes are
consumed here and only here: a delete suppresses its base leaf, and a delete
with no base leaf (the walk proves absence) contributes no subtree. The
recursive child combiner collapses branches as it returns. RLP fields retain
their source and spans, so embedded nodes and leaf values remain witness
slices.
*/

/*! Whether the active update belongs to the subtree at the cursor's current
    prefix. This inspects only the relation already carried by the cursor; it
    never pulls or compares another source key. */
function update_under_current_prefix(updates : TrieUpdateCursor) -> bool =
    match updates.relation {
        UpdateUnderPrefix(_) => true,
        UpdateBeyondPrefix(_) => false,
        UpdateSourceExhausted(_) => false,
    }

/*! The active update's next child nibble. */
function update_child_nibble(updates : TrieUpdateCursor) -> nibble =
    match updates.relation {
        UpdateUnderPrefix(path_postfix) => {
            let postfix_len = path_len(path_postfix);
            if postfix_len == 0 then {
                fatal_error(WitnessDeficient)
            } else {
                path_nibble(path_postfix, 0)
            }
        },
        UpdateBeyondPrefix(_) => fatal_error(WitnessDeficient),
        UpdateSourceExhausted(_) => fatal_error(WitnessDeficient),
    }

/*! Remaining sibling-key order after the active update's child nibble.
    Recursive consumption returns only a strictly later sibling range. */
function update_child_ranges_remaining(updates : TrieUpdateCursor) -> range(0, 16) =
    let update_pending = update_under_current_prefix(updates) in
    if update_pending then {
        let child_nibble = update_child_nibble(updates) in let child_index = unsigned(child_nibble) in 16 - child_index
    } else {
        0
    }

/*! Remaining event order while one existing path is merged with update child
    ranges. The next event is either that path's nibble or the active update's
    nibble, whichever is earlier. */
function overlay_child_ranges_remaining(
    updates : TrieUpdateCursor,
    existing_pending : bool,
    existing_nibble : nibble,
) -> (
    range(1, 17)
) = {
    let update_pending = update_under_current_prefix(updates);
    let update_position : range(0, 16) =
        if update_pending then {
            let child_nibble = update_child_nibble(updates);
            unsigned(child_nibble)
        } else {
            16
        };
    let existing_position : range(0, 16) =
        if existing_pending then unsigned(existing_nibble) else 16;
    let next_position =
        if update_position < existing_position then update_position else existing_position;
    17 - next_position
}

/*! Builds the trie formed by live put updates beneath `prefix`, consuming
    exactly that contiguous range from the ordered update stream. */
function updates_subtree(
    updates : TrieUpdateCursor,
    prefix : TriePath,
    cursor : trie_path_cursor,
) -> (
    (TrieItem, TrieUpdateCursor)
) = {
    let prefix_len = path_len(prefix);
    if prefix_len != cursor then {
        fatal_error(WitnessDeficient)
    };
    let has_update = update_under_current_prefix(updates);
    if not_bool(has_update) then {
        (trie_empty_subtree(), updates)
    } else if cursor == 64 then {
        match updates.relation {
            UpdateUnderPrefix(path_postfix) => {
                let postfix_len = path_len(path_postfix);
                if postfix_len != 0 then {
                    fatal_error(WitnessDeficient)
                }
            },
            UpdateBeyondPrefix(_) => fatal_error(WitnessDeficient),
            UpdateSourceExhausted(_) => fatal_error(WitnessDeficient),
        };
        let (update, next) = trie_updates_pop(updates);
        let update_matches_prefix = path_eq(update.key, prefix);
        if not_bool(update_matches_prefix) then {
            fatal_error(WitnessDeficient)
        };
        match update.change {
            TrieDelete() => (trie_empty_subtree(), next),
            TriePut(value) => (trie_scratch_leaf(update.key, value), next),
        }
    } else {
        let next_cursor : trie_path_cursor = cursor + 1;
        var children = trie_children_empty();
        var remaining = updates;
        var update_pending = update_under_current_prefix(remaining);
        while update_pending termination_measure(update_child_ranges_remaining(remaining)) do {
            let nib = update_child_nibble(remaining);
            let child_path = path_single(nib);
            let child_prefix = path_concat(prefix, child_path);
            let descended = trie_updates_descend(remaining);
            let (child, next) = updates_subtree(descended, child_prefix, next_cursor);
            children = trie_children_add(children, prefix, nib, child);
            let rebased = trie_updates_rebase(next, prefix);
            let rebased_update_pending = update_under_current_prefix(rebased);
            if rebased_update_pending then {
                let rebased_nibble = update_child_nibble(rebased);
                let rebased_index = unsigned(rebased_nibble);
                let previous_index = unsigned(nib);
                if rebased_index <= previous_index then {
                    fatal_error(WitnessDeficient)
                }
            };
            remaining = rebased;
            update_pending = rebased_update_pending
        };
        (trie_children_finish(prefix, children), remaining)
    }
}
termination_measure updates_subtree(updates, prefix, cursor) = 64 - cursor

/*! Merges one witness leaf with all ordered updates beneath its containing
    prefix. Virtual single-child branches along the leaf path are expanded
    recursively and collapse again on return. */
function overlay_leaf_subtree(
    updates : TrieUpdateCursor,
    prefix : TriePath,
    key : TriePath,
    value : StatelessInputSlice,
    cursor : trie_path_cursor,
) -> (
    (TrieItem, TrieUpdateCursor)
) = {
    let prefix_len = path_len(prefix);
    let key_below_prefix = path_prefix_of(prefix, key);
    if prefix_len != cursor | not_bool(key_below_prefix) then {
        fatal_error(WitnessDeficient)
    };
    if cursor == 64 then {
        let key_matches_prefix = path_eq(prefix, key);
        if not_bool(key_matches_prefix) then {
            fatal_error(WitnessDeficient)
        };
        let has_update = update_under_current_prefix(updates);
        if has_update then {
            match updates.relation {
                UpdateUnderPrefix(path_postfix) => {
                    let postfix_len = path_len(path_postfix);
                    if postfix_len != 0 then {
                        fatal_error(WitnessDeficient)
                    }
                },
                UpdateBeyondPrefix(_) => fatal_error(WitnessDeficient),
                UpdateSourceExhausted(_) => fatal_error(WitnessDeficient),
            };
            let (update, next) = trie_updates_pop(updates);
            let update_matches_key = path_eq(update.key, key);
            if not_bool(update_matches_key) then {
                fatal_error(WitnessDeficient)
            };
            match update.change {
                TrieDelete() => (trie_empty_subtree(), next),
                TriePut(updated) => (trie_scratch_leaf(key, updated), next),
            }
        } else {
            (trie_input_leaf(key, value), updates)
        }
    } else {
        let next_cursor : trie_path_cursor = cursor + 1;
        let leaf_nibble = path_nibble(key, cursor);
        var children = trie_children_empty();
        var remaining = updates;
        var leaf_pending : bool = true;
        var update_pending = update_under_current_prefix(remaining);
        while leaf_pending | update_pending termination_measure(
            overlay_child_ranges_remaining(remaining, leaf_pending, leaf_nibble)
        ) do {
            if update_pending then {
                let update_nibble = update_child_nibble(remaining);
                let leaf_index = unsigned(leaf_nibble);
                let update_index = unsigned(update_nibble);
                if leaf_pending & (leaf_index < update_index) then {
                    let leaf = trie_input_leaf(key, value);
                    children = trie_children_add(children, prefix, leaf_nibble, leaf);
                    leaf_pending = false
                } else {
                    let child_path = path_single(update_nibble);
                    let child_prefix = path_concat(prefix, child_path);
                    let descended = trie_updates_descend(remaining);
                    let consumes_leaf = leaf_pending & (update_nibble == leaf_nibble);
                    let (child, next) =
                        if consumes_leaf
                        then overlay_leaf_subtree(descended, child_prefix, key, value, next_cursor)
                        else updates_subtree(descended, child_prefix, next_cursor);
                    children = trie_children_add(children, prefix, update_nibble, child);
                    if consumes_leaf then {
                        leaf_pending = false
                    };
                    let rebased = trie_updates_rebase(next, prefix);
                    let rebased_update_pending = update_under_current_prefix(rebased);
                    if rebased_update_pending then {
                        let rebased_nibble = update_child_nibble(rebased);
                        let rebased_index = unsigned(rebased_nibble);
                        if rebased_index <= update_index then {
                            fatal_error(WitnessDeficient)
                        }
                    };
                    remaining = rebased;
                    update_pending = rebased_update_pending
                }
            } else {
                let leaf = trie_input_leaf(key, value);
                children = trie_children_add(children, prefix, leaf_nibble, leaf);
                leaf_pending = false
            }
        };
        (trie_children_finish(prefix, children), remaining)
    }
}
termination_measure overlay_leaf_subtree(updates, prefix, key, value, cursor) = 64 - cursor

/*! Recursive witness walk signature, declared before extension expansion
    because an extension's terminal child resumes the decoded-node walk. */
val witness_subtree : (StatelessInputSlice, TriePath, TrieUpdateCursor, trie_path_cursor) -> (
        TrieItem,
        TrieUpdateCursor,
    )

/*! Merges updates with the virtual single-child branches represented by an
    extension path, resolving the real child only when an update reaches it. */
function overlay_extension_subtree(
    childref : NodeRef,
    child_prefix : TriePath,
    updates : TrieUpdateCursor,
    prefix : TriePath,
    cursor : trie_path_cursor,
) -> (
    (TrieItem, TrieUpdateCursor)
) = {
    let prefix_len = path_len(prefix);
    let child_below_prefix = path_prefix_of(prefix, child_prefix);
    if prefix_len != cursor | not_bool(child_below_prefix) then {
        fatal_error(WitnessDeficient)
    };
    let at_child_prefix = path_eq(prefix, child_prefix);
    if at_child_prefix then {
        let has_update = update_under_current_prefix(updates);
        if has_update then {
            let child = resolve_witness_ref(childref);
            witness_subtree(child, child_prefix, updates, cursor)
        } else {
            (trie_branch(child_prefix, childref), updates)
        }
    } else if cursor == 64 then {
        fatal_error(WitnessDeficient)
    } else {
        let next_cursor : trie_path_cursor = cursor + 1;
        let extension_nibble = path_nibble(child_prefix, cursor);
        var children = trie_children_empty();
        var remaining = updates;
        var extension_pending : bool = true;
        var update_pending = update_under_current_prefix(remaining);
        while extension_pending | update_pending termination_measure(
            overlay_child_ranges_remaining(remaining, extension_pending, extension_nibble)
        ) do {
            if update_pending then {
                let update_nibble = update_child_nibble(remaining);
                let extension_index = unsigned(extension_nibble);
                let update_index = unsigned(update_nibble);
                if extension_pending & (extension_index < update_index) then {
                    let extension = trie_branch(child_prefix, childref);
                    children = trie_children_add(children, prefix, extension_nibble, extension);
                    extension_pending = false
                } else {
                    let next_path = path_single(update_nibble);
                    let next_prefix = path_concat(prefix, next_path);
                    let descended = trie_updates_descend(remaining);
                    let consumes_extension = extension_pending & (update_nibble == extension_nibble);
                    let (child, next) =
                        if consumes_extension
                        then overlay_extension_subtree(childref, child_prefix, descended, next_prefix, next_cursor)
                        else updates_subtree(descended, next_prefix, next_cursor);
                    children = trie_children_add(children, prefix, update_nibble, child);
                    if consumes_extension then {
                        extension_pending = false
                    };
                    let rebased = trie_updates_rebase(next, prefix);
                    let rebased_update_pending = update_under_current_prefix(rebased);
                    if rebased_update_pending then {
                        let rebased_nibble = update_child_nibble(rebased);
                        let rebased_index = unsigned(rebased_nibble);
                        if rebased_index <= update_index then {
                            fatal_error(WitnessDeficient)
                        }
                    };
                    remaining = rebased;
                    update_pending = rebased_update_pending
                }
            } else {
                let extension = trie_branch(child_prefix, childref);
                children = trie_children_add(children, prefix, extension_nibble, extension);
                extension_pending = false
            }
        };
        (trie_children_finish(prefix, children), remaining)
    }
}
termination_measure overlay_extension_subtree(childref, child_prefix, updates, prefix, cursor) = 64 - cursor

/*! Walks a touched witness subtree and recursively returns its canonical
    post-update structural item. */
function witness_subtree(node, prefix, updates, cursor) = {
    let prefix_len = path_len(prefix);
    if prefix_len != cursor then {
        fatal_error(WitnessDeficient)
    };
    if node.len == 0 then {
        updates_subtree(updates, prefix, cursor)
    } else {
        let decoded = decode_input_trie_node(node);
        match decoded {
            InputLeafNode(path, value) => {
                let key = path_concat(prefix, path);
                overlay_leaf_subtree(updates, prefix, key, value, cursor)
            },
            InputExtensionNode(path, childref) => {
                let extension_len = path_len(path);
                let next_cursor = cursor + extension_len;
                if extension_len == 0 | 64 < next_cursor then {
                    fatal_error(WitnessDeficient)
                } else {
                    let child_prefix = path_concat(prefix, path);
                    overlay_extension_subtree(childref, child_prefix, updates, prefix, cursor)
                }
            },
            InputBranchNode(children, value) => {
                if value.len != 0 | 64 <= cursor then {
                    fatal_error(WitnessDeficient)
                } else {
                    let next_cursor = cursor + 1;
                    var built = trie_children_empty();
                    var remaining = updates;
                    var nib : nibble = 0x0;
                    foreach (i from 0 to 15) {
                        let child_path = path_single(nib);
                        let child_prefix = path_concat(prefix, child_path);
                        let childref = children[i];
                        var present : bool = false;
                        match childref {
                            EmptyRef() => (),
                            _ => present = true,
                        };
                        let update_pending = update_under_current_prefix(remaining);
                        let update_here =
                            if update_pending then {
                                let update_nibble = update_child_nibble(remaining);
                                let update_index = unsigned(update_nibble);
                                let child_index = unsigned(nib);
                                if update_index < child_index then {
                                    fatal_error(WitnessDeficient)
                                };
                                update_nibble == nib
                            } else {
                                false
                            };
                        let (child, next_updates) =
                            if update_here then {
                                let descended = trie_updates_descend(remaining);
                                if present then {
                                    let child = resolve_witness_ref(childref);
                                    witness_subtree(child, child_prefix, descended, next_cursor)
                                } else {
                                    updates_subtree(descended, child_prefix, next_cursor)
                                }
                            } else if present
                                   then (trie_subtree(child_prefix, childref), remaining)
                                   else (trie_empty_subtree(), remaining);

                        built = trie_children_add(built, prefix, nib, child);
                        remaining =
                            if update_here then {
                                trie_updates_rebase(next_updates, prefix)
                            } else {
                                next_updates
                            };
                        nib = add_bits(nib, 0x1)
                    };
                    let update_pending = update_under_current_prefix(remaining);
                    if update_pending then {
                        fatal_error(WitnessDeficient)
                    };
                    (trie_children_finish(prefix, built), remaining)
                }
            },
        }
    }
}
termination_measure witness_subtree(node, prefix, updates, cursor) = 64 - cursor

/*md
## The root computation
*/
/*! The root of the trie anchored at `base_root` after applying the
    ordered update stream. This is the only public root computation:
    witness-native and fail-closed — the walker resolves every touched
    hash reference in the witness node-db and any missing node calls
    `fatal_error(WitnessDeficient)`; otherwise the builder recomposes the
    emitted stream canonically.

    Theorem-shaped remark: restricted to an empty base
    (`base_root = EMPTY_TRIE_ROOT`), the walker is the identity on the
    live update leaves and `trie_root` computes `TRIE(I)` of Appendix D
    directly — an empty base contains no hash references, so the node-db
    is never consulted and no failure path can fire. The native
    (full-state) backend exercises exactly this restriction: same
    implementation, different input. */
struct TrieRootResult = { root : hash, changed : bool }

/*! Applies an already-open update cursor. The `changed` result records
    whether the source contained at least one update. */
function trie_root_cursor(base_root : hash, updates : TrieUpdateCursor) -> TrieRootResult = {
    let no_updates = updates_empty(updates);
    if no_updates then {
        struct { root = base_root, changed = false }
    } else {
        let empty_prefix = path_empty();
        let (subtree, remaining) =
            if base_root == EMPTY_TRIE_ROOT then {
                updates_subtree(updates, empty_prefix, 0)
            } else {
                let node = node_db_lookup(base_root);
                if node.len == 0 then {
                    fatal_error(WitnessDeficient)
                } else {
                    witness_subtree(node, empty_prefix, updates, 0)
                }
            };
        let all_updates_consumed = updates_empty(remaining);
        if all_updates_consumed then {
            struct { root = trie_subtree_root(subtree), changed = true }
        } else {
            fatal_error(WitnessDeficient)
        }
    }
}
termination_measure trie_root_cursor(base_root, updates) = 1024

/*! The root of the trie after pulling and applying the source's ordered
    updates. The source's host iterator must be opened by its owner first. */
function trie_root(base_root : hash, source : TrieUpdateSource) -> TrieRootResult =
    let updates = trie_updates_begin(source) in
    trie_root_cursor(base_root, updates)

/*! Walks the trie toward `key` from `pos`, returning the leaf value
    without copying it; absent paths yield empty bytes. */
function trie_walk(node : StatelessInputSlice, key : TriePath, pos : trie_path_cursor) -> StatelessInputSlice = {
    if node.len == 0 then {
        EMPTY_STATELESS_INPUT_SLICE
    } else {
        let decoded = decode_input_trie_node(node);
        match decoded {
            InputLeafNode(path, value) => {
                let matches = path_matches(key, pos, path);
                if not_bool(matches) then {
                    EMPTY_STATELESS_INPUT_SLICE
                } else {
                    let path_length = path_len(path);
                    let key_length = path_len(key);
                    if pos + path_length == key_length then {
                        value
                    } else {
                        EMPTY_STATELESS_INPUT_SLICE
                    }
                }
            },
            InputExtensionNode(path, childref) => {
                let extension_len : trie_path_len = path_len(path);
                if extension_len == 0 then {
                    EMPTY_STATELESS_INPUT_SLICE
                } else {
                    let matches = path_matches(key, pos, path);
                    if not_bool(matches) then {
                        EMPTY_STATELESS_INPUT_SLICE
                    } else {
                        let next_pos = pos + extension_len;
                        if next_pos <= 64 then {
                            let child = resolve_witness_ref(childref);
                            trie_walk(child, key, next_pos)
                        } else {
                            EMPTY_STATELESS_INPUT_SLICE
                        }
                    }
                }
            },
            InputBranchNode(children, value) => {
                let key_length = path_len(key);
                if pos == key_length then {
                    value
                } else if pos < key_length then {
                    let child_nibble = path_nibble(key, pos);
                    let childref = branch_refs_get(children, child_nibble);
                    let child = resolve_witness_ref(childref);
                    trie_walk(child, key, pos + 1)
                } else {
                    EMPTY_STATELESS_INPUT_SLICE
                }
            },
        }
    }
}
termination_measure trie_walk(node, key, pos) = 64 - pos

/*! Looks up `key` from a root hash; the root node itself must be
    witnessed. */
function trie_lookup(root : hash, key : TriePath) -> StatelessInputSlice = {
    if root == EMPTY_TRIE_ROOT then {
        EMPTY_STATELESS_INPUT_SLICE
    } else {
        let node = node_db_lookup(root);
        if node.len == 0 then {
            fatal_error(WitnessDeficient)
        } else {
            trie_walk(node, key, 0)
        }
    }
}
