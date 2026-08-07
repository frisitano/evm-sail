
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

/*! The closed pull-source algebra for ordered trie updates. Each variant owns
    an independently opened host iterator and [trie_update_source_next][] is
    its sole interpreter, allowing one cursor and rebuild algorithm without
    function-valued callbacks. */
union TrieUpdateSource = {
    /* changed storage rows for one account */
    StorageTrieUpdates : address,
    /* block-final account update candidates, net-filtered in Sail */
    ChangedAccountTrieUpdates : unit,
}

/*! One pull from an ordered update source. `update` is meaningful exactly
    when `available` is true. */
struct TrieUpdateFetch = { available : bool, update : TrieUpdate }

/*! The payload sentinel used by exhausted source pulls. */
let EMPTY_TRIE_UPDATE : TrieUpdate = struct { key = path_empty(), change = TrieDelete() }

/*! Pulls the next update from a source's non-destructive host iterator. */
val trie_update_source_next : TrieUpdateSource -> TrieUpdateFetch

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
    /* the source has no active update */
    UpdateSourceExhausted : unit,
}

/*! A consuming cursor over an ordered update source. `current` is the active
    item, not a lookahead: it remains owned by the cursor until
    [trie_updates_pop][] consumes it and loads its successor exactly once. */
struct TrieUpdateCursor = {
    source : TrieUpdateSource,
    current : TrieUpdate,
    relation : TrieUpdateRelation,
}

/*! Opens a pull cursor by fetching only its first update. */
function trie_updates_begin(source : TrieUpdateSource) -> TrieUpdateCursor = {
    let first = trie_update_source_next(source);
    let relation : TrieUpdateRelation =
        if first.available then UpdateUnderPrefix(first.update.key) else UpdateSourceExhausted();
    struct { source = source, current = first.update, relation = relation }
}

/*! Whether the pull cursor has reached the end of its source. */
function updates_empty(updates : TrieUpdateCursor) -> bool =
    match updates.relation {
        UpdateSourceExhausted(_) => true,
        _ => false,
    }

/*! Consumes the active update and pulls its successor. Sources establish strict
    ascending order and unique keys before opening the cursor. The successor
    starts beyond the consumed key's full prefix, carrying the one common-prefix
    depth computed for this adjacent key pair; parents rebase it to
    `UpdateUnderPrefix(path_postfix)` when that depth reaches their subtree. */
function trie_updates_pop(updates : TrieUpdateCursor) -> (TrieUpdate, TrieUpdateCursor) =
    match updates.relation {
        UpdateSourceExhausted(_) => fatal_error(WitnessDeficient),
        _ => {
            let current = updates.current;
            let successor = trie_update_source_next(updates.source);
            let next : TrieUpdateCursor =
                if successor.available then {
                    let common_prefix_len = common_prefix_length(current.key, successor.update.key);
                    struct {
                        source = updates.source,
                        current = successor.update,
                        relation = UpdateBeyondPrefix(common_prefix_len),
                    }
                } else {
                    struct { source = updates.source, current = EMPTY_TRIE_UPDATE, relation = UpdateSourceExhausted() }
                };
            (current, next)
        },
    }

/*! Reinterprets an already-loaded successor at an ancestor prefix. No source
    item is fetched and no prefix scan is repeated: the adjacent-key common
    prefix computed by [trie_updates_pop][] decides whether the successor has
    reached the subtree where traversal should resume. */
function trie_updates_rebase(updates : TrieUpdateCursor, prefix : TriePath) -> TrieUpdateCursor =
    let prefix_len = path_len(prefix) in
    match updates.relation {
        UpdateSourceExhausted(_) => updates,
        UpdateUnderPrefix(_) => updates,
        UpdateBeyondPrefix(common_prefix_len) => if prefix_len <= common_prefix_len then {
            let path_postfix = path_drop(updates.current.key, prefix_len);
            struct { source = updates.source, current = updates.current, relation = UpdateUnderPrefix(path_postfix) }
        } else {
            updates
        },
    }

/*! Moves an under-prefix active update through one selected child edge. */
function trie_updates_descend(updates : TrieUpdateCursor) -> TrieUpdateCursor =
    match updates.relation {
        UpdateUnderPrefix(path_postfix) => {
            let postfix_len = path_len(path_postfix);
            if postfix_len == 0 then {
                fatal_error(WitnessDeficient)
            } else {
                let child_postfix = path_drop(path_postfix, 1);
                var descended = updates;
                descended.relation = UpdateUnderPrefix(child_postfix);
                descended
            }
        },
        UpdateBeyondPrefix(_) => fatal_error(WitnessDeficient),
        UpdateSourceExhausted(_) => fatal_error(WitnessDeficient),
    }

/*md
## Structural items

The walker merges updates into structural items. Recursive parent calls turn
those items back into canonical trie nodes while untouched subtrees stay
references.
*/
/*! An item's payload: no subtree at all, a live leaf, a known-branch
    reference (extension children are always branches), or a subtree
    reference of unknown kind. The empty member realizes YP Eq. 207's
    `n(I,i) = () if I = {}` directly, so recursive assembly is total; the
    reference distinction permits untouched hashes to stay opaque. */
union TrieItemValue = {
    /* no subtree exists at this prefix (YP Eq. 207) */
    EmptySubtree : unit,
    /* a live leaf's bytes */
    LeafItem : TrieLeafValue,
    /* a known-branch child reference */
    BranchItem : NodeRef,
    /* an opaque subtree reference of unknown kind */
    SubtreeItem : NodeRef,
}

/*! A sorted-stream item: a path and its payload. */
struct TrieItem = { path : TriePath, value : TrieItemValue }

/*! The absent subtree (YP `n(I,i) = ()`). Its path carries no meaning. */
function trie_empty_subtree() -> TrieItem =
    struct { path = path_empty(), value = EmptySubtree() }

function trie_leaf(path : TriePath, value : TrieLeafValue) -> TrieItem =
    struct { path = path, value = LeafItem(value) }

function trie_input_leaf(path : TriePath, value : StatelessInputSlice) -> TrieItem = {
    let leaf_value = InputTrieLeaf(value);
    trie_leaf(path, leaf_value)
}

function trie_scratch_leaf(path : TriePath, value : ScratchSlice) -> TrieItem = {
    let leaf_value = ScratchTrieLeaf(value);
    trie_leaf(path, leaf_value)
}

function trie_branch(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = BranchItem(childref) }

function trie_subtree(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = SubtreeItem(childref) }

/*! The child reference of a single-item subtree at `depth`: the item's
    remaining path is absorbed into it. This is the one place a delete
    collapse can demand node material: an unknown-type hash reference
    absorbing a nonempty suffix resolves its node from the witness db
    (fail-closed). */
function trie_child_ref(it : TrieItem, depth : trie_path_len) -> NodeRef = {
    let suffix = path_drop(it.path, depth);
    let suffix_len = path_len(suffix);
    match it.value {
        EmptySubtree() => EmptyRef(),
        LeafItem(value) => leaf_child_ref(suffix, value),
        BranchItem(subref) => if suffix_len == 0 then {
            subref
        } else {
            merge_ext_ref(suffix, subref)
        },
        SubtreeItem(subref) => if suffix_len == 0 then {
            subref
        } else {
            match subref {
                HashRef(h) => {
                    let node = node_db_lookup(h);
                    if node.len == 0 then {
                        fatal_error(WitnessDeficient)
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
    only : TrieItem,
    count : range(0, 16),
}

/*! Constructs an empty child accumulator. `only` is meaningful iff exactly
    one child has been added. */
function trie_children_empty() -> TrieChildren = {
    let empty_ref = EmptyRef();
    let children = vector_init(16, empty_ref);
    let empty_subtree = trie_empty_subtree();
    struct { mask = 0x0000, children = children, only = empty_subtree, count = 0 }
}

/*! Adds one recursively built child at `index`; an empty subtree
    contributes nothing. */
function trie_children_add(
    children : TrieChildren,
    prefix : TriePath,
    index : nibble,
    child : TrieItem,
) -> (
    TrieChildren
) =
    match child.value {
        EmptySubtree() => children,
        _ => {
            let depth = path_len(prefix);
            let child_count = children.count;
            let child_depth : range(1, 64) =
                if depth < 64 then depth + 1 else fatal_error(WitnessDeficient);
            let next_child_count : range(1, 16) =
                if child_count < 16 then child_count + 1 else fatal_error(WitnessDeficient);
            let child_segment = path_single(index);
            let child_prefix = path_concat(prefix, child_segment);
            let path_below_child = path_prefix_of(child_prefix, child.path);
            let path_outside_child = not_bool(path_below_child);
            if path_outside_child then {
                fatal_error(WitnessDeficient)
            };
            let child_already_present = branch_mask_has(children.mask, index);
            if child_already_present then {
                fatal_error(WitnessDeficient)
            };
            var updated = children;
            updated.mask = branch_mask_set(updated.mask, index);
            updated.children[unsigned(index)] = trie_child_ref(child, child_depth);
            updated.only = child;
            updated.count = next_child_count;
            updated
        },
    }

/*! Finishes a recursive branch: zero children disappear, one child bubbles
    upward structurally, and multiple children form a canonical branch item. */
function trie_children_finish(prefix : TriePath, children : TrieChildren) -> TrieItem =
    if children.count == 0 then {
        struct { path = prefix, value = EmptySubtree() }
    } else if children.count == 1 then {
        children.only
    } else {
        let branch_ref = branch_child_ref(children.mask, children.children);
        trie_branch(prefix, branch_ref)
    }

/*! Converts a recursively assembled top-level item to the committed root. */
function trie_subtree_root(subtree : TrieItem) -> hash =
    match subtree.value {
        EmptySubtree() => EMPTY_TRIE_ROOT,
        _ => {
            let root_ref = trie_child_ref(subtree, 0);
            trie_ref_to_root(root_ref)
        },
    }
