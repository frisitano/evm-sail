
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
    match updates.current {
        None() => false,
        Some(_) => match updates.relation {
            UpdateUnderPrefix(_) => true,
            UpdateBeyondPrefix(_) => false,
        },
    }

/*! The active update's next child nibble. */
function update_child_nibble(updates : TrieUpdateCursor) -> nibble =
    match updates.relation {
        UpdateUnderPrefix(path_postfix) => if path_len(path_postfix) == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            path_nibble(path_postfix, 0)
        },
        UpdateBeyondPrefix(_) => throw InvalidBlock(WitnessDeficient),
    }

/*! Remaining sibling-key order after the active update's child nibble.
    Recursive consumption returns only a strictly later sibling range. */
function update_child_ranges_remaining(updates : TrieUpdateCursor) -> range(0, 16) =
    if update_under_current_prefix(updates) then {
        16 - unsigned(update_child_nibble(updates))
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
    let update_position : range(0, 16) = if update_under_current_prefix(updates) then {
            unsigned(update_child_nibble(updates))
        } else {
            16
    };
    let existing_position : range(0, 16) = if existing_pending then {
            unsigned(existing_nibble)
        } else {
            16
        };
    let next_position = if update_position < existing_position then { update_position } else { existing_position };
    17 - next_position
}

/*! Builds the trie formed by live put updates beneath `prefix`, consuming
    exactly that contiguous range from the ordered update stream. */
function updates_subtree(
    updates : TrieUpdateCursor,
    prefix : TriePath,
    cursor : trie_path_cursor,
) -> (
    (option(TrieItem), TrieUpdateCursor)
) = {
    if path_len(prefix) != cursor then {
        throw InvalidBlock(WitnessDeficient)
    };
    if not_bool(update_under_current_prefix(updates)) then {
        (None(), updates)
    } else {
        if cursor == 64 then {
            match updates.relation {
                UpdateUnderPrefix(path_postfix) => if path_len(path_postfix) != 0 then {
                    throw InvalidBlock(WitnessDeficient)
                },
                UpdateBeyondPrefix(_) => throw InvalidBlock(WitnessDeficient),
            };
            let (update, next) = trie_updates_pop(updates);
            if not_bool(path_eq(update.key, prefix)) then {
                throw InvalidBlock(WitnessDeficient)
            };
            let subtree : option(TrieItem) = match update.change {
                TrieDelete() => None(),
                TriePut(value) => Some(item_scratch_leaf(update.key, value)),
            };
            (subtree, next)
        } else {
            let next_cursor : trie_path_cursor = cursor + 1;
            var children = trie_children_empty();
            var remaining = updates;
            while update_under_current_prefix(remaining) termination_measure(update_child_ranges_remaining(remaining)) do {
                let nib = update_child_nibble(remaining);
                let child_prefix = path_concat(prefix, path_single(nib));
                let descended = trie_updates_descend(remaining);
                let (child, next) = updates_subtree(descended, child_prefix, next_cursor);
                children = trie_children_add(children, prefix, nib, child);
                let rebased = trie_updates_rebase(next, prefix);
                if update_under_current_prefix(rebased) then {
                    if unsigned(update_child_nibble(rebased)) <= unsigned(nib) then {
                        throw InvalidBlock(WitnessDeficient)
                    }
                };
                remaining = rebased
            };
            (trie_children_finish(prefix, children), remaining)
        }
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
    (option(TrieItem), TrieUpdateCursor)
) = {
    if path_len(prefix) != cursor | not_bool(path_prefix_of(prefix, key)) then {
        throw InvalidBlock(WitnessDeficient)
    };
    if cursor == 64 then {
        if not_bool(path_eq(prefix, key)) then {
            throw InvalidBlock(WitnessDeficient)
        };
        if update_under_current_prefix(updates) then {
            match updates.relation {
                UpdateUnderPrefix(path_postfix) => if path_len(path_postfix) != 0 then {
                    throw InvalidBlock(WitnessDeficient)
                },
                UpdateBeyondPrefix(_) => throw InvalidBlock(WitnessDeficient),
            };
            let (update, next) = trie_updates_pop(updates);
            if not_bool(path_eq(update.key, key)) then {
                throw InvalidBlock(WitnessDeficient)
            };
            let subtree : option(TrieItem) = match update.change {
                TrieDelete() => None(),
                TriePut(updated) => Some(item_scratch_leaf(key, updated)),
            };
            (subtree, next)
        } else {
            (Some(item_input_leaf(key, value)), updates)
        }
    } else {
        let next_cursor : trie_path_cursor = cursor + 1;
        let leaf_nibble = path_nibble(key, cursor);
        var children = trie_children_empty();
        var remaining = updates;
        var leaf_pending : bool = true;
        while leaf_pending | update_under_current_prefix(remaining) termination_measure(
            overlay_child_ranges_remaining(remaining, leaf_pending, leaf_nibble)
        ) do {
            if update_under_current_prefix(remaining) then {
                let update_nibble = update_child_nibble(remaining);
                if leaf_pending & (unsigned(leaf_nibble) < unsigned(update_nibble)) then {
                    children = trie_children_add(children, prefix, leaf_nibble, Some(item_input_leaf(key, value)));
                    leaf_pending = false
                } else {
                    let child_prefix = path_concat(prefix, path_single(update_nibble));
                    let descended = trie_updates_descend(remaining);
                    let consumes_leaf = leaf_pending & (update_nibble == leaf_nibble);
                    let (child, next) = if consumes_leaf then {
                            overlay_leaf_subtree(descended, child_prefix, key, value, next_cursor)
                        } else {
                            updates_subtree(descended, child_prefix, next_cursor)
                        };
                    children = trie_children_add(children, prefix, update_nibble, child);
                    if consumes_leaf then {
                        leaf_pending = false
                    };
                    let rebased = trie_updates_rebase(next, prefix);
                    if update_under_current_prefix(rebased) then {
                        if unsigned(update_child_nibble(rebased)) <= unsigned(update_nibble) then {
                            throw InvalidBlock(WitnessDeficient)
                        }
                    };
                    remaining = rebased
                }
            } else {
                children = trie_children_add(children, prefix, leaf_nibble, Some(item_input_leaf(key, value)));
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
        option(TrieItem),
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
    (option(TrieItem), TrieUpdateCursor)
) = {
    if path_len(prefix) != cursor | not_bool(path_prefix_of(prefix, child_prefix)) then {
        throw InvalidBlock(WitnessDeficient)
    };
    if path_eq(prefix, child_prefix) then {
        if update_under_current_prefix(updates) then {
            witness_subtree(resolve_witness_ref(childref), child_prefix, updates, cursor)
        } else {
            (Some(item_branch(child_prefix, childref)), updates)
        }
    } else {
        if cursor == 64 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            let next_cursor : trie_path_cursor = cursor + 1;
            let extension_nibble = path_nibble(child_prefix, cursor);
            var children = trie_children_empty();
            var remaining = updates;
            var extension_pending : bool = true;
            while extension_pending | update_under_current_prefix(remaining) termination_measure(
                overlay_child_ranges_remaining(remaining, extension_pending, extension_nibble)
            ) do {
                if update_under_current_prefix(remaining) then {
                    let update_nibble = update_child_nibble(remaining);
                    if extension_pending & (unsigned(extension_nibble) < unsigned(update_nibble)) then {
                        children = trie_children_add(
                            children,
                            prefix,
                            extension_nibble,
                            Some(item_branch(child_prefix, childref)),
                        );
                        extension_pending = false
                    } else {
                        let next_prefix = path_concat(prefix, path_single(update_nibble));
                        let descended = trie_updates_descend(remaining);
                        let consumes_extension = extension_pending & (update_nibble == extension_nibble);
                        let (child, next) = if consumes_extension then {
                                overlay_extension_subtree(childref, child_prefix, descended, next_prefix, next_cursor)
                            } else {
                                updates_subtree(descended, next_prefix, next_cursor)
                            };
                        children = trie_children_add(children, prefix, update_nibble, child);
                        if consumes_extension then {
                            extension_pending = false
                        };
                        let rebased = trie_updates_rebase(next, prefix);
                        if update_under_current_prefix(rebased) then {
                            if unsigned(update_child_nibble(rebased)) <= unsigned(update_nibble) then {
                                throw InvalidBlock(WitnessDeficient)
                            }
                        };
                        remaining = rebased
                    }
                } else {
                    children = trie_children_add(
                        children,
                        prefix,
                        extension_nibble,
                        Some(item_branch(child_prefix, childref)),
                    );
                    extension_pending = false
                }
            };
            (trie_children_finish(prefix, children), remaining)
        }
    }
}
termination_measure overlay_extension_subtree(childref, child_prefix, updates, prefix, cursor) = 64 - cursor

/*! Walks a touched witness subtree and recursively returns its canonical
    post-update structural item. */
function witness_subtree(node, prefix, updates, cursor) = {
    if path_len(prefix) != cursor then {
        throw InvalidBlock(WitnessDeficient)
    };
    if node.len == 0 then {
        updates_subtree(updates, prefix, cursor)
    } else {
        match decode_input_trie_node(node) {
            InputLeafNode(path, value) => {
                let key = path_concat(prefix, path);
                overlay_leaf_subtree(updates, prefix, key, value, cursor)
            },
            InputExtensionNode(path, childref) => {
                let extension_len = path_len(path);
                let next_cursor = cursor + extension_len;
                if extension_len == 0 | 64 < next_cursor then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    overlay_extension_subtree(childref, path_concat(prefix, path), updates, prefix, cursor)
                }
            },
            InputBranchNode(children, value) => {
                if value.len != 0 | 64 <= cursor then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    let next_cursor = cursor + 1;
                    var built = trie_children_empty();
                    var remaining = updates;
                    var nib : nibble = 0x0;
                    foreach (i from 0 to 15) {
                        let child_prefix = path_concat(prefix, path_single(nib));
                        let childref = children[i];
                        let present : bool = match childref {
                            EmptyRef() => false,
                            _ => true,
                        };
                        let update_here = if update_under_current_prefix(remaining) then {
                                let update_nibble = update_child_nibble(remaining);
                                if unsigned(update_nibble) < unsigned(nib) then {
                                    throw InvalidBlock(WitnessDeficient)
                                };
                                update_nibble == nib
                            } else {
                                false
                            };
                        let (child, next_updates) = if update_here then {
                                let descended = trie_updates_descend(remaining);
                                if present then {
                                    witness_subtree(resolve_witness_ref(childref), child_prefix, descended, next_cursor)
                                } else {
                                    updates_subtree(descended, child_prefix, next_cursor)
                                }
                            } else {
                                if present then {
                                    (Some(item_subtree(child_prefix, childref)), remaining)
                                } else {
                                    (None(), remaining)
                                }
                            };
                        built = trie_children_add(built, prefix, nib, child);
                        remaining =
                            if update_here then {
                                trie_updates_rebase(next_updates, prefix)
                            } else {
                                next_updates
                            };
                        nib = add_bits(nib, 0x1)
                    };
                    if update_under_current_prefix(remaining) then {
                        throw InvalidBlock(WitnessDeficient)
                    };
                    (trie_children_finish(prefix, built), remaining)
                }
            },
        }
    }
}
termination_measure witness_subtree(node, prefix, updates, cursor) = 64 - cursor

/*md
## The trie entry point
*/
/*! The root of the trie anchored at `base_root` after applying the
    ordered update stream. This is the only public root computation:
    witness-native and fail-closed — the walker resolves every touched
    hash reference in the witness node-db and any missing node throws
    `InvalidBlock(WitnessDeficient)`; otherwise the builder recomposes the
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
    if updates_empty(updates) then {
        struct { root = base_root, changed = false }
    } else {
        let (subtree, remaining) = if base_root == EMPTY_TRIE_ROOT then {
                updates_subtree(updates, path_empty(), 0)
            } else {
                let node = node_db_lookup(base_root);
                if node.len == 0 then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    witness_subtree(node, path_empty(), updates, 0)
                }
            };
        if updates_empty(remaining) then {
            struct { root = trie_subtree_root(subtree), changed = true }
        } else {
            throw InvalidBlock(WitnessDeficient)
        }
    }
}

/*! The root of the trie after pulling and applying the source's ordered
    updates. The source's host iterator must be opened by its owner first. */
function trie_root(base_root : hash, source : TrieUpdateSource) -> TrieRootResult =
    trie_root_cursor(base_root, trie_updates_begin(source))

/*! Walks the trie toward `key` from `pos`, returning the leaf value
    without copying it; absent paths yield empty bytes. */
function trie_walk(node : StatelessInputSlice, key : TriePath, pos : trie_path_cursor) -> StatelessInputSlice = {
    if node.len == 0 then {
        EMPTY_STATELESS_INPUT_SLICE
    } else {
        match decode_input_trie_node(node) {
            InputLeafNode(path, value) => if not_bool(path_matches(key, pos, path)) then {
                EMPTY_STATELESS_INPUT_SLICE
            } else {
                if pos + path_len(path) == path_len(key) then { value } else { EMPTY_STATELESS_INPUT_SLICE }
            },
            InputExtensionNode(path, childref) => {
                let extension_len : trie_path_len = path_len(path);
                if extension_len == 0 then {
                    EMPTY_STATELESS_INPUT_SLICE
                } else {
                    if not_bool(path_matches(key, pos, path)) then {
                        EMPTY_STATELESS_INPUT_SLICE
                    } else {
                        let next_pos = pos + extension_len;
                        if next_pos <= 64 then {
                            trie_walk(resolve_witness_ref(childref), key, next_pos)
                        } else {
                            EMPTY_STATELESS_INPUT_SLICE
                        }
                    }
                }
            },
            InputBranchNode(children, value) => {
                if pos == path_len(key) then {
                    value
                } else {
                    if pos < path_len(key) then {
                        let child = resolve_witness_ref(branch_refs_get(children, path_nibble(key, pos)));
                        trie_walk(child, key, pos + 1)
                    } else {
                        EMPTY_STATELESS_INPUT_SLICE
                    }
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
            throw InvalidBlock(WitnessDeficient)
        } else {
            trie_walk(node, key, 0)
        }
    }
}
