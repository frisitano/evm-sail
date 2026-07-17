/* Ordered leaf updates. */

union TrieChange = {
    TriePut : ByteSlice,
    TrieDelete : unit,
}

struct TrieUpdate = { key : TriePath, change : TrieChange }

function updates_empty(updates : list(TrieUpdate)) -> bool =
    match updates {
        [||] => true,
        _ => false,
    }

function next_update_under(updates : list(TrieUpdate), prefix : TriePath) -> bool =
    match updates {
        update :: _ => path_prefix_of(prefix, update.key),
        [||] => false,
    }

/* The walker merges updates into an ordered item stream; the builder turns
 * that stream back into canonical trie nodes. Untouched subtrees stay refs. */

/* Extension children are known branches; branch children have unknown kind
 * until resolved. That distinction permits untouched hashes to stay opaque. */
union TrieItemValue = {
    LeafItem : ByteSlice,
    BranchItem : NodeRef,
    SubtreeItem : NodeRef,
}

struct TrieItem = { path : TriePath, value : TrieItemValue }

function item_leaf(path : TriePath, value : ByteSlice) -> TrieItem =
    struct { path = path, value = LeafItem(value) }

function item_branch(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = BranchItem(childref) }

function item_subtree(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = SubtreeItem(childref) }

/* the child ref of a single-item subtree at `depth`: the item's remaining
 * path is absorbed into it. This is the ONE place a delete collapse can
 * demand node material: an unknown-type hash ref absorbing a nonempty
 * suffix resolves its node from the witness db (fail-closed). */
function item_ref(it : TrieItem, depth : trie_path_len) -> NodeRef = {
    let suffix = path_drop(it.path, depth);
    match it.value {
        LeafItem(value) => leaf_child_ref(suffix, value),
        BranchItem(subref) => if path_len(suffix) == 0 then {
            subref
        } else {
            merge_ext_ref(suffix, subref)
        },
        SubtreeItem(subref) => if path_len(suffix) == 0 then {
            subref
        } else {
            match subref {
                HashRef(h) => {
                    let node = node_db_lookup(h);
                    if node.len == BYTE_ZERO then {
                        throw InvalidBlock(WitnessDeficient)
                    } else {
                        merge_ext_node(suffix, node)
                    }
                },
                _ => merge_ext_ref(suffix, subref),
            }
        },
    }
}

/* Incremental canonical builder for an already ordered, prefix-free item
 * stream. It realizes the Yellow Paper c(I, i) / n(I, i) grouping directly:
 * the next key determines which branch frames remain open, so no prefix
 * partition or second walk is needed. The current value is fully consumed
 * before this function returns. */
struct TrieBranchFrame = {
    depth : trie_depth,
    mask : bits(16),
    children : BranchRefs,
}

struct TrieBuilder = {
    /* The open branch is the head. Sail lists are reference-counted, so a
     * builder copy, push, or pop never copies the other open frames. */
    frames : list(TrieBranchFrame),
    root : NodeRef,
    complete : bool,
}

function empty_trie_branch_frame(depth : trie_depth) -> TrieBranchFrame =
    struct { depth = depth, mask = 0x0000, children = vector_init(16, EmptyRef()) }

function trie_builder_empty() -> TrieBuilder =
    struct { frames = [||], root = EmptyRef(), complete = false }

function trie_builder_push(builder : TrieBuilder, depth : trie_depth) -> TrieBuilder =
    struct {
        frames = empty_trie_branch_frame(depth) :: builder.frames,
        root = builder.root,
        complete = builder.complete,
    }

function trie_builder_attach(builder : TrieBuilder, path : TriePath, child : NodeRef) -> TrieBuilder =
    match builder.frames {
        [||] => throw InvalidBlock(WitnessDeficient),
        current :: rest => {
            var frame = current;
            let depth = frame.depth;
            if path_len(path) <= depth then {
                throw InvalidBlock(WitnessDeficient)
            };
            let child_index = path_nibble(path, depth);
            if branch_mask_has(frame.mask, child_index) then {
                throw InvalidBlock(WitnessDeficient)
            };
            frame.mask = branch_mask_set(frame.mask, child_index);
            frame.children = branch_refs_set(frame.children, child_index, child);
            struct { frames = frame :: rest, root = builder.root, complete = builder.complete }
        },
    }

function trie_builder_pop(builder : TrieBuilder) -> (TrieBranchFrame, TrieBuilder) =
    match builder.frames {
        [||] => throw InvalidBlock(WitnessDeficient),
        frame :: rest => (frame, struct { frames = rest, root = builder.root, complete = builder.complete }),
    }

function trie_builder_wrap_branch(
    anchor : TriePath,
    parent_depth : trie_depth,
    child_depth : trie_depth,
    child : NodeRef,
) -> (
    NodeRef
) = {
    let child_start : trie_path_len = parent_depth + 1;
    if child_depth <= child_start then {
        child
    } else {
        let gap : trie_path_len = child_depth - child_start;
        extension_child_ref(path_take(path_drop(anchor, child_start), gap), child)
    }
}

function trie_builder_close(
    builder : TrieBuilder,
    anchor : TriePath,
    next_common : option(trie_depth),
    fuel : trie_path_len,
) -> (
    TrieBuilder
) = {
    if fuel == 0 then {
        throw InvalidBlock(WitnessDeficient)
    };
    match builder.frames {
        [||] => builder,
        top :: _ => {
            let should_close : bool = match next_common {
                None() => true,
                Some(depth) => depth < top.depth,
            };
            if not_bool(should_close) then {
                builder
            } else {
                let (frame, popped) = trie_builder_pop(builder);
                let child = branch_child_ref(frame.mask, frame.children);
                let with_parent : TrieBuilder = match popped.frames {
                    parent :: _ => match next_common {
                        Some(common) => if parent.depth < common then {
                            let intermediate = trie_builder_push(popped, common);
                            trie_builder_attach(
                                intermediate,
                                anchor,
                                trie_builder_wrap_branch(anchor, common, frame.depth, child),
                            )
                        } else {
                            trie_builder_attach(
                                popped,
                                anchor,
                                trie_builder_wrap_branch(anchor, parent.depth, frame.depth, child),
                            )
                        },
                        None() => trie_builder_attach(
                            popped,
                            anchor,
                            trie_builder_wrap_branch(anchor, parent.depth, frame.depth, child),
                        ),
                    },
                    [||] => match next_common {
                        Some(common) => {
                            let parent = trie_builder_push(popped, common);
                            trie_builder_attach(
                                parent,
                                anchor,
                                trie_builder_wrap_branch(anchor, common, frame.depth, child),
                            )
                        },
                        None() => {
                            let depth = frame.depth;
                            let root = if depth == 0 then {
                                    child
                                } else {
                                    extension_child_ref(path_take(anchor, depth), child)
                                };
                            struct { frames = popped.frames, root = root, complete = true }
                        },
                    },
                };
                trie_builder_close(with_parent, anchor, next_common, fuel - 1)
            }
        },
    }
}
termination_measure trie_builder_close(builder, anchor, next_common, fuel) = fuel

function trie_item_next_common(item : TrieItem, next_key : option(TriePath)) -> option(trie_depth) =
    match next_key {
        None() => None(),
        Some(next) => {
            let common = common_prefix_from(item.path, next, 0);
            if not_bool(path_lt(item.path, next)) | path_len(item.path) <= common | path_len(next) <= common then {
                throw InvalidBlock(WitnessDeficient)
            };
            Some(to_trie_depth(common))
        },
    }

function trie_insert_item(builder : TrieBuilder, item : TrieItem, next_key : option(TriePath)) -> TrieBuilder = {
    if builder.complete then {
        throw InvalidBlock(WitnessDeficient)
    };
    let next_common = trie_item_next_common(item, next_key);
    let open_child : bool = match next_common {
        None() => false,
        Some(common) => match builder.frames {
            [||] => true,
            top :: _ => top.depth < common,
        },
    };
    if open_child then {
        match next_common {
            Some(common) => {
                let opened = trie_builder_push(builder, common);
                trie_builder_attach(opened, item.path, item_ref(item, common + 1))
            },
            None() => throw InvalidBlock(WitnessDeficient),
        }
    } else {
        match builder.frames {
            [||] => struct { frames = builder.frames, root = item_ref(item, 0), complete = true },
            top :: _ => {
                let attached = trie_builder_attach(builder, item.path, item_ref(item, top.depth + 1));
                trie_builder_close(attached, item.path, next_common, 64)
            },
        }
    }
}

/* One-item lookahead turns any ordered producer into a streaming trie input.
 * The pending item is consumed as soon as its successor supplies the next
 * key; no item retains a source-backed payload after that call returns. */
struct TrieItemSink = {
    builder : TrieBuilder,
    pending : option(TrieItem),
}

function trie_sink_empty() -> TrieItemSink =
    struct { builder = trie_builder_empty(), pending = None() }

function trie_sink_emit(sink : TrieItemSink, item : TrieItem) -> TrieItemSink =
    match sink.pending {
        None() => struct { builder = sink.builder, pending = Some(item) },
        Some(previous) => struct {
            builder = trie_insert_item(sink.builder, previous, Some(item.path)),
            pending = Some(item),
        },
    }

function trie_sink_finish(sink : TrieItemSink) -> TrieItemSink =
    match sink.pending {
        None() => sink,
        Some(item) => struct { builder = trie_insert_item(sink.builder, item, None()), pending = None() },
    }

function trie_builder_root(builder : TrieBuilder) -> hash =
    match builder.frames {
        _ :: _ => throw InvalidBlock(WitnessDeficient),
        [||] => if builder.complete then {
            trie_ref_to_root(builder.root)
        } else {
            EMPTY_TRIE_ROOT
        },
    }

function trie_sink_root(sink : TrieItemSink) -> hash =
    match sink.pending {
        Some(_) => throw InvalidBlock(WitnessDeficient),
        None() => trie_builder_root(sink.builder),
    }
