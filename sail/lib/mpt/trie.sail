
/* Updates and surviving witness subtrees are emitted directly into a sink.
 * The sink retains one item for key lookahead, so touched paths are rebuilt
 * without materialising or partitioning an intermediate item list. */

/*md
# The witness-native trie

The witness-native Ethereum Merkle-Patricia trie. [trie_root][] merges
ordered updates into an authenticated base trie and fails closed when a
touched hash is absent from the witness. With an empty base the same
builder computes the Yellow Paper `TRIE(I)` directly.

## The witness walker

`witness_emit(node, prefix, updates, sink, cursor)` emits the sorted
post-state items of the subtree rooted at `node` (whose position is
`prefix`) and returns the updated sink together with the first update
after that subtree. `updates` is a cursor into the one globally sorted
update stream; each call consumes its contiguous prefix range and returns
the cursor to its parent.

The walker descends only along touched paths; untouched children pass
through as single reference items with zero node-db work. Deletes are
consumed here and only here: a delete suppresses its base leaf, and a
delete with no base leaf (the walk proves absence) emits nothing. Whatever
survives is streamed through [trie_sink_emit][] and finalized by
[trie_sink_finish][], so a branch losing children to deletes collapses in
the builder with no walker involvement. RLP fields retain their source and
spans, so embedded nodes and leaf values remain witness slices.
*/
/*! Emits all live put updates beneath a prefix and consumes that contiguous
    range from the ordered update stream. */
function emit_live_updates_under(
    sink : TrieItemSink,
    updates : TrieUpdateCursor,
    prefix : TriePath,
) -> (
    (TrieItemSink, TrieUpdateCursor)
) =
    match updates.pending {
        None() => (sink, updates),
        Some(update) => if path_prefix_of(prefix, update.key) then {
            let next_sink : TrieItemSink = match update.change {
                TrieDelete() => sink,
                TriePut(value) => trie_sink_emit(sink, item_leaf(update.key, value)),
            };
            emit_live_updates_under(next_sink, trie_updates_advance(updates), prefix)
        } else {
            (sink, updates)
        },
    }

termination_measure emit_live_updates_under(sink, updates, prefix) = 2 ^ 64

/*! Emits live updates preceding a witness child while retaining the first
    update at or after that child. */
function emit_updates_before_child(
    sink : TrieItemSink,
    updates : TrieUpdateCursor,
    prefix : TriePath,
    child : TriePath,
) -> (
    (TrieItemSink, TrieUpdateCursor)
) =
    match updates.pending {
        None() => (sink, updates),
        Some(update) => if   not_bool(path_prefix_of(prefix, update.key))
                           | path_prefix_of(child, update.key)
                           | not_bool(path_lt(update.key, child)) then {
            (sink, updates)
        } else {
            let next_sink : TrieItemSink = match update.change {
                TrieDelete() => sink,
                TriePut(value) => trie_sink_emit(sink, item_leaf(update.key, value)),
            };
            emit_updates_before_child(next_sink, trie_updates_advance(updates), prefix, child)
        },
    }

termination_measure emit_updates_before_child(sink, updates, prefix, child) = 2 ^ 64

/*! Merges one witness leaf with all ordered updates in the same subtree. */
function emit_leaf_overlay(
    sink : TrieItemSink,
    updates : TrieUpdateCursor,
    prefix : TriePath,
    key : TriePath,
    value : ByteSlice,
) -> (
    (TrieItemSink, TrieUpdateCursor)
) =
    match updates.pending {
        None() => (trie_sink_emit(sink, item_leaf(key, value)), updates),
        Some(update) => if not_bool(path_prefix_of(prefix, update.key)) then {
            (trie_sink_emit(sink, item_leaf(key, value)), updates)
        } else {
            if path_eq(update.key, key) then {
                let updated_sink : TrieItemSink = match update.change {
                    TrieDelete() => sink,
                    TriePut(updated) => trie_sink_emit(sink, item_leaf(key, updated)),
                };
                emit_live_updates_under(updated_sink, trie_updates_advance(updates), prefix)
            } else {
                if path_lt(key, update.key) then {
                    emit_live_updates_under(trie_sink_emit(sink, item_leaf(key, value)), updates, prefix)
                } else {
                    let updated_sink : TrieItemSink = match update.change {
                        TrieDelete() => sink,
                        TriePut(updated) => trie_sink_emit(sink, item_leaf(update.key, updated)),
                    };
                    emit_leaf_overlay(updated_sink, trie_updates_advance(updates), prefix, key, value)
                }
            }
        },
    }

termination_measure emit_leaf_overlay(sink, updates, prefix, key, value) = 2 ^ 64

/*! Walks a touched witness subtree and streams its post-update items into the
    canonical trie builder. */
function witness_emit(
    node : ByteSlice,
    prefix : TriePath,
    updates : TrieUpdateCursor,
    sink : TrieItemSink,
    cursor : trie_path_cursor,
) -> (
    (TrieItemSink, TrieUpdateCursor)
) = {
    if node.len == 0 then {
        emit_live_updates_under(sink, updates, prefix)
    } else {
        match decode_trie_node(node) {
            LeafNode(path, value) => {
                let key = path_concat(prefix, path);
                emit_leaf_overlay(sink, updates, prefix, key, value)
            },
            ExtensionNode(path, childref) => {
                let extension_len = path_len(path);
                let next_cursor = cursor + extension_len;
                if extension_len == 0 | 64 < next_cursor then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    let child_prefix = path_concat(prefix, path);
                    let (before_sink, child_updates) = emit_updates_before_child(sink, updates, prefix, child_prefix);
                    let (child_sink, later_updates) = if next_update_under(child_updates, child_prefix) then {
                            let child = resolve_ref(childref);
                            if child.len == 0 then {
                                emit_live_updates_under(before_sink, child_updates, child_prefix)
                            } else {
                                witness_emit(child, child_prefix, child_updates, before_sink, next_cursor)
                            }
                        } else {
                            (trie_sink_emit(before_sink, item_branch(child_prefix, childref)), child_updates)
                        };
                    emit_live_updates_under(child_sink, later_updates, prefix)
                }
            },
            BranchNode(children, value) => {
                if value.len != 0 | 64 <= cursor then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    let next_cursor = cursor + 1;
                    var current_sink = sink;
                    var remaining = updates;
                    var nib : nibble = 0x0;
                    foreach (i from 0 to 15) {
                        let child_prefix = path_concat(prefix, path_single(nib));
                        let childref = children[i];
                        let present : bool = match childref {
                            EmptyRef() => false,
                            _ => true,
                        };
                        if next_update_under(remaining, child_prefix) then {
                            let (next_sink, next_updates) = if present then {
                                    witness_emit(
                                        resolve_ref(childref),
                                        child_prefix,
                                        remaining,
                                        current_sink,
                                        next_cursor,
                                    )
                                } else {
                                    emit_live_updates_under(current_sink, remaining, child_prefix)
                                };
                            current_sink = next_sink;
                            remaining = next_updates
                        } else {
                            if present then {
                                current_sink = trie_sink_emit(current_sink, item_subtree(child_prefix, childref))
                            }
                        };
                        nib = add_bits(nib, 0x1)
                    };
                    (current_sink, remaining)
                }
            },
        }
    }
}
termination_measure witness_emit(node, prefix, updates, sink, cursor) = 64 - cursor

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
        let sink = trie_sink_empty();
        let (updated_sink, remaining) = if base_root == EMPTY_TRIE_ROOT then {
                emit_live_updates_under(sink, updates, path_empty())
            } else {
                let node = node_db_lookup(base_root);
                if node.len == 0 then {
                    throw InvalidBlock(WitnessDeficient)
                } else {
                    witness_emit(node, path_empty(), updates, sink, 0)
                }
            };
        if updates_empty(remaining) then {
            struct { root = trie_sink_root(trie_sink_finish(updated_sink)), changed = true }
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
function trie_walk(node : ByteSlice, key : TriePath, pos : trie_path_cursor) -> ByteSlice = {
    if node.len == 0 then {
        EMPTY_SLICE
    } else {
        match decode_trie_node(node) {
            LeafNode(path, value) => if not_bool(path_matches(key, pos, path)) then {
                EMPTY_SLICE
            } else {
                if pos + path_len(path) == path_len(key) then { value } else { EMPTY_SLICE }
            },
            ExtensionNode(path, childref) => {
                let extension_len : trie_path_len = path_len(path);
                if extension_len == 0 then {
                    EMPTY_SLICE
                } else {
                    if not_bool(path_matches(key, pos, path)) then {
                        EMPTY_SLICE
                    } else {
                        let next_pos = pos + extension_len;
                        if next_pos <= 64 then {
                            trie_walk(resolve_ref(childref), key, next_pos)
                        } else {
                            EMPTY_SLICE
                        }
                    }
                }
            },
            BranchNode(children, value) => {
                if pos == path_len(key) then {
                    value
                } else {
                    if pos < path_len(key) then {
                        let child = resolve_ref(branch_refs_get(children, path_nibble(key, pos)));
                        trie_walk(child, key, pos + 1)
                    } else {
                        EMPTY_SLICE
                    }
                }
            },
        }
    }
}
termination_measure trie_walk(node, key, pos) = 64 - pos

/*! Looks up `key` from a root hash; the root node itself must be
    witnessed. */
function trie_lookup(root : hash, key : TriePath) -> ByteSlice = {
    if root == EMPTY_TRIE_ROOT then {
        EMPTY_SLICE
    } else {
        let node = node_db_lookup(root);
        if node.len == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            trie_walk(node, key, 0)
        }
    }
}
