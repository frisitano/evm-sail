/*md
# Trie updates and the canonical builder

Ordered leaf updates, their merge into item streams, and the trie builder
that recomposes canonical nodes (YP Appendix D).
*/

/*! A pending change at a trie key: a put of new leaf bytes, or a
    delete. */
union TrieChange = {
    TriePut : Bytes,
    TrieDelete : unit,
}

/*! An update: a full-path key and its change. Update lists are kept
    sorted by key. */
struct TrieUpdate = { key : TriePath, change : TrieChange }

/*! Splits an update list at `count` (merge-sort helper). */
function split_updates(updates : list(TrieUpdate), count : int) -> (list(TrieUpdate), list(TrieUpdate)) =
    if count <= 0 then {
        ([||], updates)
    } else {
        match updates {
            [||] => ([||], [||]),
            update :: rest => {
                let (left, right) = split_updates(rest, count - 1);
                (update :: left, right)
            },
        }
    }
termination_measure split_updates(updates, count) = count

/*! Merges two sorted update lists, keeping key order. */
function merge_updates(left : list(TrieUpdate), right : list(TrieUpdate)) -> list(TrieUpdate) =
    match (left, right) {
        ([||], _) => right,
        (_, [||]) => left,
        (l :: ls, r :: rs) => if path_lt(r.key, l.key) then {
            r :: merge_updates(left, rs)
        } else {
            l :: merge_updates(ls, right)
        },
    }

function sort_updates_n(updates : list(TrieUpdate), count : int) -> list(TrieUpdate) =
    if count <= 1 then {
        updates
    } else {
        let left_count = ediv_int(count, 2);
        let (left, right) = split_updates(updates, left_count);
        merge_updates(sort_updates_n(left, left_count), sort_updates_n(right, count - left_count))
    }
termination_measure sort_updates_n(updates, count) = count

function sort_updates(updates : list(TrieUpdate)) -> list(TrieUpdate) =
    sort_updates_n(updates, list_len(updates))

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

/*md
## The item streams

The walker merges updates into an ordered item stream; the builder turns
that stream back into canonical trie nodes. Untouched subtrees stay
references.
*/

/*! An item's payload: a live leaf, a known-branch reference (extension
    children are always branches), or a subtree reference of unknown kind.
    The distinction permits untouched hashes to stay opaque. */
union TrieItemValue = {
    LeafItem : Bytes,
    BranchItem : NodeRef,
    SubtreeItem : NodeRef,
}

/*! A sorted-stream item: a path and its payload. */
struct TrieItem = { path : TriePath, value : TrieItemValue }

function item_leaf(path : TriePath, value : Bytes) -> TrieItem =
    struct { path = path, value = LeafItem(value) }

function item_branch(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = BranchItem(childref) }

function item_subtree(path : TriePath, childref : NodeRef) -> TrieItem =
    struct { path = path, value = SubtreeItem(childref) }

/*! The sorted live-leaf items of an update list; deletes emit nothing. */
function live_update_items(updates : list(TrieUpdate)) -> list(TrieItem) =
    match updates {
        [||] => [||],
        update :: rest => match update.change {
            TrieDelete() => live_update_items(rest),
            TriePut(value) => item_leaf(update.key, value) :: live_update_items(rest),
        },
    }

/*! Consumes the leading updates under `prefix`, returning the first
    update outside that subtree to the caller. Sorted keys make the
    matching range contiguous, so every update advances through the
    witness walk once. */
function live_update_items_under(updates : list(TrieUpdate), prefix : TriePath) -> (list(TrieItem), list(TrieUpdate)) =
    match updates {
        [||] => ([||], [||]),
        update :: rest => if path_prefix_of(prefix, update.key) then {
            let (items, remaining) = live_update_items_under(rest, prefix);
            match update.change {
                TrieDelete() => (items, remaining),
                TriePut(value) => (item_leaf(update.key, value) :: items, remaining),
            }
        } else {
            ([||], updates)
        },
    }

/*! Overlays the contiguous update range for a base leaf, returning the
    cursor positioned at the next sibling subtree. */
function leaf_overlay_items(
    updates : list(TrieUpdate),
    prefix : TriePath,
    key : TriePath,
    value : Bytes,
) -> (
    (list(TrieItem), list(TrieUpdate))
) =
    match updates {
        [||] => ([|item_leaf(key, value)|], [||]),
        update :: rest => if not_bool(path_prefix_of(prefix, update.key)) then {
            ([|item_leaf(key, value)|], updates)
        } else {
            if path_eq(update.key, key) then {
                let (items, remaining) = live_update_items_under(rest, prefix);
                match update.change {
                    TrieDelete() => (items, remaining),
                    TriePut(updated) => (item_leaf(update.key, updated) :: items, remaining),
                }
            } else {
                if path_lt(key, update.key) then {
                    let (items, remaining) = live_update_items_under(updates, prefix);
                    (item_leaf(key, value) :: items, remaining)
                } else {
                    let (items, remaining) = leaf_overlay_items(rest, prefix, key, value);
                    match update.change {
                        TrieDelete() => (items, remaining),
                        TriePut(updated) => (item_leaf(update.key, updated) :: items, remaining),
                    }
                }
            }
        },
    }

/*! The path of the last item in a stream. */
function items_last_path(items : list(TrieItem)) -> TriePath =
    match items {
        [||] => path_empty(),
        it :: r => match r {
            [||] => it.path,
            _ => items_last_path(r),
        },
    }

/*! Takes the leading items whose nibble at `depth` is `nib`. */
function take_nibble_group(items : list(TrieItem), depth : int, nib : nibble) -> (list(TrieItem), list(TrieItem)) =
    match items {
        [||] => ([||], [||]),
        it :: r => if path_len(it.path) > depth & path_nibble(it.path, depth) == nib then {
            let (group, remaining) = take_nibble_group(r, depth, nib);
            (it :: group, remaining)
        } else {
            ([||], items)
        },
    }

/*! The child reference of a single-item subtree at `depth`: the item's
    remaining path is absorbed into it. This is the one place a delete
    collapse can demand node material: an unknown-type hash reference
    absorbing a nonempty suffix resolves its node from the witness db
    (fail-closed). */
function item_ref(it : TrieItem, depth : int) -> NodeRef = {
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
                    if bytes_len(node) == 0 then {
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
## The trie builder

The canonical builder (YP Appendix D). On a leaf-only stream this is the
protocol trie definition, transcribed from the Yellow Paper (Shanghai
version) and specialized to fixed-length secure-trie keys (so a branch
node's value slot — the 17th item — is always empty):

```text
c(I,i): leaf | extension | 17-item branch      (Eq. 208)  build_group
n(I,i): () if I = {}; c(I,i) if ||RLP|| < 32;
        KEC(RLP(c(I,i))) otherwise             (Eq. 207)  child_ref
TRIE(I) = KEC(RLP(c(I,0)))                     (Eq. 206)  items_root
```
*/

/*! Composes the subtree of a sorted, prefix-free item group whose paths
    all extend the group's first `depth` nibbles. Node closure is by
    lookahead on the sorted keys: an extension covers exactly the prefix
    shared by the whole group (first/last suffices when sorted), and a
    branch materializes only where ≥ 2 key groups actually diverge — the
    canonical form is reached by construction, with no separate collapse
    or canonicalization step. `fuel` only bounds recursion for the
    termination checker: keys are 64 nibbles, callers pass 256. */
function build_group(items : list(TrieItem), depth : int, fuel : int) -> NodeRef = {
    if fuel <= 0 then {
        throw InvalidBlock(WitnessDeficient)
    } else {
        match items {
            [||] => EmptyRef(),
            it :: [||] => item_ref(it, depth),
            it :: _ => {
                let last = items_last_path(items);
                let common = common_prefix_from(it.path, last, depth);
                if common > 0 then {
                    /* >= 2 items share `shared`, so the child one level down
                       is necessarily a branch: a plain extension wrap is
                       canonical */
                    let shared = path_take(path_drop(it.path, depth), common);
                    extension_child_ref(shared, build_group(items, depth + common, fuel - common))
                } else {
                    /* >= 2 items diverge right here: a branch with >= 2
                       children by construction (groups of a nonempty sorted
                       stream are nonempty and never build to empty) */
                    var mask : bits(16) = 0x0000;
                    var children_rev : list(NodeRef) = [||];
                    var remaining : list(TrieItem) = items;
                    var more : bool = true;
                    while more do {
                        match remaining {
                            [||] => more = false,
                            next :: _ => if path_len(next.path) <= depth then {
                                throw InvalidBlock(WitnessDeficient)
                            } else {
                                let nib = path_nibble(next.path, depth);
                                let (group, rest) = take_nibble_group(remaining, depth, nib);
                                mask = branch_mask_set_bit(mask, unsigned(nib));
                                children_rev = build_group(group, depth + 1, fuel - 1) :: children_rev;
                                remaining = rest
                            },
                        }
                    };
                    branch_child_ref(mask, list_reverse(children_rev))
                }
            },
        }
    }
}
termination_measure build_group(items, depth, fuel) = fuel

/*! `TRIE` of a sorted item stream (Eq. 206 on a leaf-only stream). */
function items_root(items : list(TrieItem)) -> bits(256) =
    match items {
        [||] => EMPTY_TRIE_ROOT,
        _ => trie_ref_to_root(build_group(items, 0, 256)),
    }
