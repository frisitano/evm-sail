
/*md
# Trie updates and canonical subtree assembly

Ordered leaf updates, their structural items, and the recursive assembly that
recomposes canonical nodes (YP Appendix D).
*/
/*! A pending change at a trie key: a put of new leaf bytes, or a
    delete. */
union TrieChange = {
    /* insert or replace the leaf bytes at the key */
    TriePut : ScratchSlice,
    /* remove the key */
    TrieDelete : unit,
}

/*! An update: a full-path key and its change. Sources yield updates in
    ascending key order. */
struct TrieUpdate = { key : TriePath, change : TrieChange }

/*! A pull source for ordered trie updates. Each variant owns an independently
    opened host iterator. */
union TrieUpdateSource = {
    /* changed storage rows for one account */
    StorageTrieUpdates : address,
    /* block-final account update candidates, net-filtered in Sail */
    ChangedAccountTrieUpdates : unit,
}

/*! Pulls the next update from a source's non-destructive host iterator. */
val trie_update_source_next : TrieUpdateSource -> option(TrieUpdate)

/*! The active update's position relative to the subtree currently consuming
    it. An under-prefix state carries only the unconsumed key suffix. A
    beyond-prefix state carries the absolute common-prefix depth of the update
    just consumed and its already loaded successor, allowing recursive callers
    to unwind directly to their divergence point. */
union TrieUpdateRelation = {
    /* the active update belongs to this prefix; payload is its remaining path */
    UpdateUnderPrefix : TriePath,
    /* the active successor is beyond this prefix; payload is its unwind depth */
    UpdateBeyondPrefix : trie_path_len,
}

/*! A consuming cursor over an ordered update source. `current` is the active
    item, not a lookahead: it remains owned by the cursor until
    [trie_updates_pop][] consumes it and loads its successor exactly once. */
struct TrieUpdateCursor = {
    source : TrieUpdateSource,
    current : option(TrieUpdate),
    relation : TrieUpdateRelation,
}

/*! Opens a pull cursor by fetching only its first update. */
function trie_updates_begin(source : TrieUpdateSource) -> TrieUpdateCursor = {
    let first = trie_update_source_next(source);
    let relation : TrieUpdateRelation = match first {
        Some(update) => UpdateUnderPrefix(update.key),
        None() => UpdateBeyondPrefix(0),
    };
    struct { source = source, current = first, relation = relation }
}

/*! Whether the pull cursor has reached the end of its source. */
function updates_empty(updates : TrieUpdateCursor) -> bool =
    match updates.current {
        None() => true,
        Some(_) => false,
    }

/*! Consumes the active update and pulls its successor. Sources establish strict
    ascending order and unique keys before opening the cursor. The successor
    starts beyond the consumed key's full prefix, carrying the one common-prefix
    depth computed for this adjacent key pair; parents rebase it to
    `UpdateUnderPrefix(path_postfix)` when that depth reaches their subtree. */
function trie_updates_pop(updates : TrieUpdateCursor) -> (TrieUpdate, TrieUpdateCursor) =
    match updates.current {
        Some(current) => {
            let successor = trie_update_source_next(updates.source);
            let next : TrieUpdateCursor = match successor {
                Some(update) => {
                    struct {
                        source = updates.source,
                        current = successor,
                        relation = UpdateBeyondPrefix(common_prefix_length(current.key, update.key)),
                    }
                },
                None() => struct { source = updates.source, current = None(), relation = UpdateBeyondPrefix(0) },
            };
            (current, next)
        },
        None() => throw InvalidBlock(WitnessDeficient),
    }

/*! Reinterprets an already-loaded successor at an ancestor prefix. No source
    item is fetched and no prefix scan is repeated: the adjacent-key common
    prefix computed by [trie_updates_pop][] decides whether the successor has
    reached the subtree where traversal should resume. */
function trie_updates_rebase(updates : TrieUpdateCursor, prefix : TriePath) -> TrieUpdateCursor =
    match updates.current {
        None() => updates,
        Some(update) => match updates.relation {
            UpdateUnderPrefix(_) => updates,
            UpdateBeyondPrefix(common_prefix_len) => if path_len(prefix) <= common_prefix_len then {
                struct {
                    source = updates.source,
                    current = updates.current,
                    relation = UpdateUnderPrefix(path_drop(update.key, path_len(prefix))),
                }
            } else {
                updates
            },
        },
    }

/*! Moves an under-prefix active update through one selected child edge. */
function trie_updates_descend(updates : TrieUpdateCursor) -> TrieUpdateCursor =
    match updates.relation {
        UpdateUnderPrefix(path_postfix) => if path_len(path_postfix) == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            var descended = updates;
            descended.relation = UpdateUnderPrefix(path_drop(path_postfix, 1));
            descended
        },
        UpdateBeyondPrefix(_) => throw InvalidBlock(WitnessDeficient),
    }

/*md
## Structural items

The walker merges updates into structural items. Recursive parent calls turn
those items back into canonical trie nodes while untouched subtrees stay
references.
*/
/*! An item's payload: a live leaf, a known-branch reference (extension
    children are always branches), or a subtree reference of unknown kind.
    The distinction permits untouched hashes to stay opaque. */
union TrieItemValue = {
    /* a live leaf's bytes */
    LeafItem : TrieLeafValue,
    /* a known-branch child reference */
    BranchItem : NodeRef,
    /* an opaque subtree reference of unknown kind */
    SubtreeItem : NodeRef,
}

/*! A sorted-stream item: a path and its payload. */
struct TrieItem = { path : TriePath, value : TrieItemValue }

function item_leaf(path : TriePath, value : TrieLeafValue) -> TrieItem =
    struct { path = path, value = LeafItem(value) }

function item_input_leaf(path : TriePath, value : StatelessInputSlice) -> TrieItem =
    item_leaf(path, InputTrieLeaf(value))

function item_scratch_leaf(path : TriePath, value : ScratchSlice) -> TrieItem =
    item_leaf(path, ScratchTrieLeaf(value))

function item_branch(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = BranchItem(childref) }

function item_subtree(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = SubtreeItem(childref) }

/*! The child reference of a single-item subtree at `depth`: the item's
    remaining path is absorbed into it. This is the one place a delete
    collapse can demand node material: an unknown-type hash reference
    absorbing a nonempty suffix resolves its node from the witness db
    (fail-closed). */
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
                    if node.len == 0 then {
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

/*md
## Recursive subtree assembly

The canonical builder (YP Appendix D) is expressed as a recursive partition
of the key space. Each recursive call returns either no subtree or one
structural item. A single child bubbles upward unchanged; two or more children
form a branch at the current prefix. Consequently leaf and extension paths
are encoded only when they meet a real branch or become the root, and no
explicit frame stack is required.

```text
c(I,i): leaf | extension | 17-item branch      (Eq. 208)  trie_children_finish
n(I,i): () if I = {}; c(I,i) if ||RLP|| < 32;
        KEC(RLP(c(I,i))) otherwise             (Eq. 207)  child_ref
TRIE(I) = KEC(RLP(c(I,0)))                     (Eq. 206)  trie_subtree_root
```
*/
/*! Child references accumulated while one recursive branch is assembled.
    `only` retains the structural item when exactly one child survives, so
    canonical leaf/extension collapse does not need to reopen an encoded
    hash. */
struct TrieChildren = {
    mask : branch_mask,
    children : BranchRefs,
    only : option(TrieItem),
    count : range(0, 16),
}

/*! Constructs an empty child accumulator. */
function trie_children_empty() -> TrieChildren =
    struct { mask = 0x0000, children = vector_init(16, EmptyRef()), only = None(), count = 0 }

/*! Adds one recursively built child at `index`. */
function trie_children_add(
    children : TrieChildren,
    prefix : TriePath,
    index : nibble,
    child : option(TrieItem),
) -> (
    TrieChildren
) =
    match child {
        None() => children,
        Some(item) => {
            let depth = path_len(prefix);
            let child_count = children.count;
            if 64 <= depth | child_count == 16 then {
                throw InvalidBlock(WitnessDeficient)
            };
            let child_prefix = path_concat(prefix, path_single(index));
            if not_bool(path_prefix_of(child_prefix, item.path)) then {
                throw InvalidBlock(WitnessDeficient)
            };
            if branch_mask_has(children.mask, index) then {
                throw InvalidBlock(WitnessDeficient)
            };
            var updated = children;
            updated.mask = branch_mask_set(updated.mask, index);
            updated.children[unsigned(index)] = item_ref(item, depth + 1);
            updated.only = Some(item);
            updated.count = child_count + 1;
            updated
        },
    }

/*! Finishes a recursive branch: zero children disappear, one child bubbles
    upward structurally, and multiple children form a canonical branch item. */
function trie_children_finish(prefix : TriePath, children : TrieChildren) -> option(TrieItem) =
    if children.count == 0 then {
        None()
    } else {
        if children.count == 1 then {
            children.only
        } else {
            Some(item_branch(prefix, branch_child_ref(children.mask, children.children)))
        }
    }

/*! Converts a recursively assembled top-level item to the committed root. */
function trie_subtree_root(subtree : option(TrieItem)) -> hash =
    match subtree {
        None() => EMPTY_TRIE_ROOT,
        Some(item) => trie_ref_to_root(item_ref(item, 0)),
    }
