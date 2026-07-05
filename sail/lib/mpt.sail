/* ======================================================================== */
/* WORLD-STATE TRIE ROOTS AND WITNESS MACHINERY                             */
/*                                                                          */
/* Everything above the canonical trie definition (lib/trie.sail, the       */
/* Yellow Paper TRIE/c/n anchor). This module refines that definition over  */
/* partial knowledge and applies it to Ethereum world state, in two layers. */
/*                                                                          */
/* (1) World-state trie: the state root over accounts, keyed by             */
/* keccak256(address) with value RLP([nonce, balance, storageRoot,          */
/* codeHash]) over per-account storage tries. This is the commitment a      */
/* block's stateRoot must equal. compute_state_root_overlay over the        */
/* post-execution world equals the block's stateRoot exactly when execution */
/* is correct -- it authenticates the result.                               */
/*                                                                          */
/* Design decision (final): post-state roots are computed by BATCH overlay  */
/* -- one recursive pass applying the whole ordered update list -- NOT by   */
/* per-op insert/delete maintenance. The Yellow Paper defines the trie      */
/* declaratively over the final map (Appendix D; S D.1 leaves maintenance   */
/* strategy to implementations); the batch pass touches and hashes each     */
/* node once, and a per-op delete would face the same blinded-sibling       */
/* collapse problem while multiplying hashing. Do not migrate this to       */
/* incremental per-operation insert/delete.                                 */
/* (2) Witness reads and the pre-state overlay: stateless verification --   */
/* materialize a hash-keyed node database from the witness, resolve         */
/* account/storage values lazily by Merkle proof from the authenticated     */
/* parent root, and compute post-state roots by recursively traversing the  */
/* witnessed node database, applying the ordered update list only along     */
/* touched paths and preserving untouched (possibly blinded) children by    */
/* reference. Every root produced here must agree with TRIE over the        */
/* composed map.                                                            */
/* ======================================================================== */


/* ======================================================================== */
/* (2) Ethereum world-state trie                                            */
/*                                                                          */
/* The state root (Yellow Paper section 4.1): the secure-trie root over all */
/* accounts. The trie is 'secure' -- keys are hashed: key =                 */
/* keccak256(address), value = RLP([nonce, balance, storageRoot,            */
/* codeHash]). storageRoot is the root of the account's storage trie, the   */
/* secure trie over keccak256(slot) -> RLP(value) with zero-valued slots    */
/* absent. Accounts that are empty per EIP-161 (zero nonce, zero balance,   */
/* no code) are excluded.                                                   */
/*                                                                          */
/* compute_state_root_overlay over the post-execution world equals the      */
/* block's stateRoot exactly when execution is correct -- it authenticates  */
/* the result.                                                              */
/* ======================================================================== */

/* Reused account-RLP payload accumulator. account_rlp is invoked once per account
 * when authenticating the post-state root (compute_state_root); a block-level
 * root or the proving path runs it over the whole account set, so it builds the
 * payload into this single register (O(1) prepend, reversed once) instead of
 * allocating fresh intermediate lists per account. Not reentrant: cleared at the
 * start of every account_rlp call, and account_rlp calls nothing that touches it. */
register rlp_scratch : list(byte) = [||]
function rlp_push(bs : list(byte)) -> unit = {
    var rest : list(byte) = bs;
    var more : bool = true;
    while more do {
        match rest {
            b :: t => {
                rlp_scratch = b :: rlp_scratch;
                rest = t
            },
            [||] => more = false,
        }
    }
}

/* rlp([nonce, balance, storageRoot, codeHash]) */
function account_rlp(nonce : int, balance : word, sroot : bits(256), chash : bits(256)) -> list(byte) = {
    rlp_scratch = [||];
    rlp_push(rlp_int(nonce));
    rlp_push(rlp_int(unsigned(balance)));
    rlp_push(rlp_word(sroot));
    rlp_push(rlp_word(chash));
    rlp_list(list_reverse(rlp_scratch)) /* prepend builds newest-first; reverse -> field order */
}

/* secure-trie updates for one account's post-state storage, selected by
 * keccak(address). Base-layer contract (defined per entry): the witness
 * backend consumes only the write set (untouched slots remain inside the
 * recursive overlay rooted at the account's pre-state storage root); the
 * native backend enumerates the full live cache/update union because it has
 * no authenticated witness root. */
val storage_updates : bits(256) -> list(TrieUpdate)

/* write-set-only updates (witness base): nonzero writes set RLP(value), zero
 * writes delete the slot; a zero write to a slot with no authenticated
 * pre-state leaf is dropped (nothing to delete). */
function storage_updates_writes_only(ak : bits(256)) -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = storage_update_count();
    foreach (j from 0 to (n - 1)) {
        let (eak, es, ev) = storage_hashed_update_entry(j);
        if eak == ak then {
            let sk = keccak256_word(es);
            if word_is_zero(ev) & not_bool(storage_map_cache_nonzero(ak, sk)) then {
                ()
            } else {
                out = update_insert_sorted(
                    struct {
                        key = to_nibbles(word_to_bytes32(sk)),
                        is_delete = word_is_zero(ev),
                        value = rlp_int(unsigned(ev)),
                    },
                    out,
                )
            }
        }
    };
    out
}

/* full live cache/update union (native fixture base) */
function storage_updates_union(ak : bits(256)) -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = storage_count();
    foreach (j from 0 to (n - 1)) {
        let (eak, es, ev) = storage_hashed_entry(j);
        if eak == ak then {
            let sk = keccak256_word(es);
            out = update_insert_sorted(
                struct {
                    key = to_nibbles(word_to_bytes32(sk)),
                    is_delete = word_is_zero(ev),
                    value = rlp_int(unsigned(ev)),
                },
                out,
            )
        }
    };
    out
}

/* post-state storage root for keccak(address). Defined after the recursive
 * overlay walker. */
val storage_root : address -> bits(256)
val storage_root_from : (bits(256), bits(256)) -> bits(256)

function account_empty(acc : Account) -> bool =
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == 0 & word_is_zero(acc.balance)

/* 256-bit account field (balance / code_hash) of the current acctmap_at row */
function selected_account_balance() -> word = acctmap_at_bal()
function selected_account_code_hash() -> word = acctmap_at_chash()
function selected_account_storage_root() -> word = acctmap_at_sroot()

/* secure trie key of the current acctmap_at row (keccak(address)) */
function selected_account_key_hash() -> bits(256) = acctmap_at_hkey()

/* secure-trie updates for the account map. Base-layer contract (defined per
 * entry): stateless block validation consumes only execution updates
 * (witness-materialized read-only accounts are base-state cache, not
 * post-state updates); the native EEST runner has no parent root anchor, so
 * it uses the complete cache/update union to build a full updates-only root. */
val state_updates_map : unit -> list(TrieUpdate)

/* trie update for the currently-selected acctmap row. skip_empty_unbased
 * drops empty accounts with no authenticated base leaf (witness base: there
 * is nothing to delete); otherwise empty accounts become deletes. */
function state_updates_selected_row(prev : list(TrieUpdate), skip_empty_unbased : bool) -> list(TrieUpdate) = {
    let ak_hash = selected_account_key_hash();
    let acc : Account = struct {
            nonce = unsigned(acctmap_at_nonce()),
            balance = selected_account_balance(),
            code_hash = selected_account_code_hash(),
            storage_root = selected_account_storage_root(),
        };
    let ak = to_nibbles(word_to_bytes32(ak_hash));
    let base_exists = acctmap_at_base_exists();
    if skip_empty_unbased & account_empty(acc) & not_bool(base_exists) then {
        prev
    } else {
        let upd : TrieUpdate = if account_empty(acc) then {
                struct { key = ak, is_delete = true, value = [||] }
            } else {
                struct {
                    key = ak,
                    is_delete = false,
                    value =
                        account_rlp(
                            acc.nonce,
                            acc.balance,
                            storage_root_from(ak_hash, acc.storage_root),
                            acc.code_hash,
                        ),
                }
            };
        update_insert_sorted(upd, prev)
    }
}

/* execution updates only (witness base) */
function state_updates_map_updates_only() -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = unsigned(acctmap_update_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_update_at(get_slice_int(64, j, 0));
        out = state_updates_selected_row(out, true)
    };
    out
}

/* complete cache/update union (native fixture base) */
function state_updates_map_union() -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_at(get_slice_int(64, j, 0));
        out = state_updates_selected_row(out, false)
    };
    out
}

/* Post-state roots by recursive trie composition. */
val compute_state_root_updates_only : unit -> bits(256)
val compute_state_root_overlay : unit -> bits(256)

/* ======================================================================== */
/* (3) stateless witness-backed reads                                       */
/*                                                                          */
/* Stateless verification of the world-state trie from a Merkle witness     */
/* (the node set covering the proof paths), mirroring execution-specs       */
/* witness_state.py. A node database keyed by keccak256(node) (C-backed,    */
/* ffi/trie_node_db.c) backs every read.                                    */
/*                                                                          */
/* Reads are LAZY: point reads walk the secure trie from the authenticated  */
/* parent root (k_parent_state_root) by keccak256(key) nibbles (branch /    */
/* extension / leaf, hex-prefix paths) to the account leaf and decode       */
/* RLP([nonce, balance, storageRoot, codeHash]); storage reads walk the     */
/* account's storage root the same way. Because the node database is keyed  */
/* by keccak256(node), every walk is itself a hash-chain proof from its     */
/* root -- no up-front re-root pass is needed to authenticate the witness.  */
/*                                                                          */
/* A node absent from the witness on a walked path is a missing proof and   */
/* fails closed. Depends on the RLP library and the SSZ                     */
/* witness-list readers (host/io.sail).                                     */
/* ======================================================================== */

/* EMPTY_TRIE_ROOT is defined in host/state.sail (next to KECCAK_EMPTY) so the
 * Account struct's storage_root default can reference it. */
let EMPTY_CODE_HASH : bits(256) = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

/* decode an account leaf value: (nonce, balance, storage_root, code_hash) */
function decode_account(leaf : list(byte)) -> (int, word, bits(256), bits(256)) = {
    let items = rlp_node_items(leaf);
    let nonce = rlp_field_int(rlp_item(items, 0));
    let balance = rlp_field_uint_word(rlp_item(items, 1));
    let sr_f = rlp_item(items, 2);
    let ch_f = rlp_item(items, 3);
    let sroot = if list_len(sr_f.content) == 0 then {
            EMPTY_TRIE_ROOT
        } else {
            bytes_be256(sr_f.content)
        };
    let chash = if list_len(ch_f.content) == 0 then {
            EMPTY_CODE_HASH
        } else {
            bytes_be256(ch_f.content)
        };
    (nonce, balance, sroot, chash)
}

/* 64 nibbles (MSB-first) of a 256-bit key hash */
function hash_nibbles(h : bits(256)) -> list(nibble) = {
    var rev : list(nibble) = [||];
    foreach (i from 0 to 63) {
        rev = sail_shiftright(h, 4 * (63 - i))[3 .. 0] :: rev
    };
    list_reverse(rev)
}

/* node bytes for keccak `h` from the C node-db, or [] if absent (read straight
 * from the witness buffer by the recorded span -- no per-node Sail copy held). */
function node_db_lookup(h : bits(256)) -> list(byte) = {
    if not_bool(nodedb_sel(h)) then {
        [||]
    } else {
        ssz_bytes(unsigned(nodedb_sel_off()), unsigned(nodedb_sel_len()))
    }
}

/* resolve a child reference: embedded node (full bytes), 32-byte hash, or empty.
 * A 32-byte hash that is absent from the witness is a MISSING node (the prover
 * omitted a node on a walked path) -- fail closed, so a resolved-empty result
 * always means a legitimate exclusion, never a withheld subtree. */
function resolve_child(f : RlpField) -> list(byte) =
    if f.is_list then {
        f.full
    } else {
        if list_len(f.content) == 32 then {
            let node = node_db_lookup(bytes_be256(f.content));

            /* a 32-byte ref absent from the witness means the proof is deficient:
               flag the block invalid (valid=false) and end the walk here, rather
               than aborting -- a missing proof is a NORMAL failure outcome
               (EIP-8025 optional proofs), not a guest crash. */
            if list_len(node) == 0 then {
                k_witness_bad = true
            };
            node
        } else {
            [||]
        }
    }

/* ---- recursive trie overlay -------------------------------------------- */

/* A patch is either a leaf write (is_ref = false, bytes = RLP value), or a
 * subtree reference (is_ref = true, bytes = child ref). For subtree refs,
 * `node` optionally carries the referenced node's full RLP: an overlaid
 * subtree exists only in memory (its post-state hash is NOT a witness node-db
 * key), so a later collapse into the ref must use these carried bytes -- a
 * node-db lookup of the fresh hash can never succeed. `node` is empty for
 * untouched witness subtrees, where the node-db lookup is the (EIP-8025
 * guaranteed) collapse-material path. */
struct TriePatch = {
    key : list(nibble),
    is_delete : bool,
    is_ref : bool,
    bytes : list(byte),
    node : list(byte),
}

function field_to_ref(f : RlpField) -> list(byte) =
    if f.is_list then {
        f.full
    } else {
        if list_len(f.content) == 32 then {
            hash_child_ref(f.content)
        } else {
            trie_empty_ref()
        }
    }

function merge_ext_node(prefix : list(nibble), childnode : list(byte)) -> list(byte) = {
    if list_len(prefix) == 0 then {
        childnode
    } else {
        if list_len(childnode) == 0 then {
            [||]
        } else {
            let items = rlp_node_items(childnode);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    leaf_node_rlp(key, list_nth(items, 1, RLP_NONE).content)
                } else {
                    extension_node_rlp(key, field_to_ref(list_nth(items, 1, RLP_NONE)))
                }
            } else {
                extension_node_rlp(prefix, node_to_ref(childnode))
            }
        }
    }
}

function merge_ext_ref(prefix : list(nibble), childref : list(byte)) -> list(byte) = {
    if list_len(prefix) == 0 then {
        childref
    } else {
        if trie_ref_is_empty(childref) then {
            trie_empty_ref()
        } else {
            match childref {
                0xa0 :: _ => extension_child_ref(prefix, childref),
                _ => {
                    let items = rlp_node_items(childref);
                    let n = list_len(items);
                    if n == 2 then {
                        let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                        let key = list_append(prefix, seg);
                        if is_leaf then {
                            leaf_child_ref(key, list_nth(items, 1, RLP_NONE).content)
                        } else {
                            extension_child_ref(key, field_to_ref(list_nth(items, 1, RLP_NONE)))
                        }
                    } else {
                        extension_child_ref(prefix, childref)
                    }
                },
            }
        }
    }
}

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
        [||]   => trie_empty_ref(),
    }

/* the Yellow Paper's branch canonicalization invariant (Appendix D): "a
 * branch is then only used when necessary; no branch nodes may exist that
 * contain only a single non-zero entry" -- a lone surviving child collapses
 * into its parent as an extension. Only overlay deletes can create this
 * shape; the from-scratch build (updates_node) is canonical by construction. */
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

function patch_live(p : TriePatch) -> bool = p.is_ref | not_bool(p.is_delete)

function patch_insert_sorted(p : TriePatch, ps : list(TriePatch)) -> list(TriePatch) =
    match ps {
        [||] => [|p|],
        q :: r => if nib_eq(p.key, q.key) then {
            p :: r
        } else {
            if nib_lt(p.key, q.key) then {
                p :: ps
            } else {
                q :: patch_insert_sorted(p, r)
            }
        },
    }

function patch_from_update(u : TrieUpdate) -> TriePatch =
    struct { key = u.key, is_delete = u.is_delete, is_ref = false, bytes = u.value, node = [||] }

function patches_from_updates(us : list(TrieUpdate)) -> list(TriePatch) =
    match us {
        [||]   => [||],
        u :: r => patch_insert_sorted(patch_from_update(u), patches_from_updates(r)),
    }

function patches_filter_live(ps : list(TriePatch)) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if patch_live(p) then {
            p :: patches_filter_live(r)
        } else {
            patches_filter_live(r)
        },
    }

function patch_ref(key : list(nibble), child : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = true, bytes = child, node = [||] }

function patch_insert_ref_if_live(key : list(nibble), child : list(byte), ps : list(TriePatch)) -> list(TriePatch) =
    if trie_ref_is_empty(child) then {
        ps
    } else {
        patch_insert_sorted(patch_ref(key, child), ps)
    }

/* subtree-ref patch for a freshly OVERLAID (in-memory) child node: the ref is
 * derived from the node, and the node bytes ride along for collapse (see
 * TriePatch). An empty node (subtree fully deleted) inserts nothing. */
function patch_insert_subtree_if_live(
    key : list(nibble),
    childnode : list(byte),
    ps : list(TriePatch),
) -> (
    list(TriePatch)
) = {
    let childref = node_to_ref(childnode);
    if trie_ref_is_empty(childref) then {
        ps
    } else {
        patch_insert_sorted(
            struct { key = key, is_delete = false, is_ref = true, bytes = childref, node = childnode },
            ps,
        )
    }
}

function patch_leaf(key : list(nibble), value : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = false, bytes = value, node = [||] }

function patches_first_key(ps : list(TriePatch)) -> list(nibble) =
    match ps {
        [||]   => [||],
        p :: _ => p.key,
    }

function patches_last_key(ps : list(TriePatch)) -> list(nibble) =
    match ps {
        [||] => [||],
        p :: r => match r {
            [||] => p.key,
            _    => patches_last_key(r),
        },
    }

function patches_at_nibble(ps : list(TriePatch), depth : int, nib : nibble) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if list_len(p.key) > depth & list_nth(p.key, depth, 0x0) == nib then {
            p :: patches_at_nibble(r, depth, nib)
        } else {
            patches_at_nibble(r, depth, nib)
        },
    }

function patch_entry_ref(p : TriePatch, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(p.key, depth);
    if p.is_ref then {
        if list_len(p.node) > 0 then {
            /* overlaid subtree: merge on the carried node bytes so a non-branch
             * top node (post-delete collapse) merges keys canonically */
            node_to_ref(merge_ext_node(suffix, p.node))
        } else {
            merge_ext_ref(suffix, p.bytes)
        }
    } else {
        leaf_child_ref(suffix, p.bytes)
    }
}

function ref_node_for_collapse(r : list(byte)) -> list(byte) =
    if trie_ref_is_empty(r) then {
        [||]
    } else {
        match r {
            0xa0 :: h => if list_len(h) == 32 then {
                let node = node_db_lookup(bytes_be256(h));
                if list_len(node) == 0 then {
                    k_witness_bad = true
                };
                node
            } else {
                r
            },
            _ => r,
        }
    }

function patch_entry_node(p : TriePatch, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(p.key, depth);
    if p.is_ref then {
        if list_len(p.node) > 0 then {
            /* overlaid subtree: its hash is not a node-db key, so the collapse
             * (and canonical key-merge) must use the carried node bytes */
            merge_ext_node(suffix, p.node)
        } else {
            if list_len(suffix) == 0 then {
                ref_node_for_collapse(p.bytes)
            } else {
                extension_node_rlp(suffix, p.bytes)
            }
        }
    } else {
        leaf_node_rlp(suffix, p.bytes)
    }
}

function patches_ref(ps0 : list(TriePatch), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        trie_empty_ref()
    } else {
        let ps = patches_filter_live(ps0);
        match ps {
            [||] => trie_empty_ref(),
            p :: [||] => patch_entry_ref(p, depth),
            _ => {
                let first = patches_first_key(ps);
                let last = patches_last_key(ps);
                let common = common_prefix_len(list_drop_prefix(first, depth), list_drop_prefix(last, depth));
                if common > 0 then {
                    let prefix = list_take_prefix(list_drop_prefix(first, depth), common);
                    merge_ext_ref(prefix, patches_ref(ps, depth + common, fuel - common))
                } else {
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|patches_ref(group, depth + 1, fuel - 1)|])
                        }
                    };
                    canonical_branch_ref(mask, children)
                }
            },
        }
    }
}
termination_measure patches_ref(ps0, depth, fuel) = fuel

function patches_node(ps0 : list(TriePatch), depth : int, fuel : int) -> list(byte) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        let ps = patches_filter_live(ps0);
        match ps {
            [||] => [||],
            p :: [||] => patch_entry_node(p, depth),
            _ => {
                let first = patches_first_key(ps);
                let last = patches_last_key(ps);
                let common = common_prefix_len(list_drop_prefix(first, depth), list_drop_prefix(last, depth));
                if common > 0 then {
                    let prefix = list_take_prefix(list_drop_prefix(first, depth), common);
                    merge_ext_node(prefix, patches_node(ps, depth + common, fuel - common))
                } else {
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    var lone_nib : nibble = 0x0;
                    var lone_child : list(byte) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, get_slice_int(4, i, 0));
                        if list_len(group) > 0 then {
                            let cnode = patches_node(group, depth + 1, fuel - 1);
                            if list_len(cnode) > 0 then {
                                mask = branch_mask_set_bit(mask, i);
                                children = list_append(children, [|node_to_ref(cnode)|]);
                                lone_nib = get_slice_int(4, i, 0);
                                lone_child = cnode
                            }
                        }
                    };
                    if mask == 0x0000 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            merge_ext_node([|lone_nib|], lone_child)
                        } else {
                            branch_node_rlp(mask, children)
                        }
                    }
                }
            },
        }
    }
}
termination_measure patches_node(ps0, depth, fuel) = fuel

function leaf_overlay_patches(key : list(nibble), value : list(byte), us : list(TrieUpdate)) -> list(TriePatch) = {
    let ups = patches_from_updates(us);
    if updates_has_key(us, key) then {
        ups
    } else {
        patch_insert_sorted(patch_leaf(key, value), ups)
    }
}

val trie_overlay_node_ref : (list(byte), list(nibble), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_ref : (RlpField, list(nibble), list(TrieUpdate), int) -> list(byte)
val trie_overlay_node_for_collapse : (list(byte), list(nibble), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_node_for_collapse : (RlpField, list(nibble), list(TrieUpdate), int) -> list(byte)
val trie_overlay_node_ref_preserve : (list(byte), list(nibble), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_ref_preserve : (RlpField, list(nibble), list(TrieUpdate), int) -> list(byte)

function trie_overlay_child_ref_preserve(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    list(byte)
) = {
    if updates_empty(updates) then {
        field_to_ref(f)
    } else {
        if f.is_list then {
            trie_overlay_node_ref_preserve(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                updates_ref(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        field_to_ref(f)
                    } else {
                        trie_overlay_node_ref_preserve(node, prefix, updates, fuel)
                    }
                } else {
                    trie_empty_ref()
                }
            }
        }
    }
}

function trie_overlay_node_ref_preserve(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    list(byte)
) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        trie_empty_ref()
    } else {
        if list_len(node) == 0 then {
            updates_ref(updates, list_len(prefix), fuel)
        } else {
            let items = rlp_node_items(node);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    patches_ref(
                        leaf_overlay_patches(key, list_nth(items, 1, RLP_NONE).content, updates),
                        list_len(prefix),
                        fuel,
                    )
                } else {
                    if updates_all_under_prefix(updates, key) then {
                        merge_ext_ref(
                            seg,
                            trie_overlay_child_ref_preserve(list_nth(items, 1, RLP_NONE), key, updates, fuel - 1),
                        )
                    } else {
                        let under = updates_under_prefix(updates, key);
                        let outside = updates_outside_prefix(updates, key);
                        let old = trie_overlay_child_ref_preserve(list_nth(items, 1, RLP_NONE), key, under, fuel - 1);
                        patches_ref(
                            patch_insert_ref_if_live(key, old, patches_from_updates(outside)),
                            list_len(prefix),
                            fuel,
                        )
                    }
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|get_slice_int(4, i, 0)|]);
                        let group = updates_at_nibble(updates, list_len(prefix), get_slice_int(4, i, 0));
                        let cref = trie_overlay_child_ref_preserve(field, child_prefix, group, fuel - 1);
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
}

function trie_overlay_child_ref(f : RlpField, prefix : list(nibble), updates : list(TrieUpdate), fuel : int) -> list(byte) = {
    if updates_empty(updates) then {
        field_to_ref(f)
    } else {
        if f.is_list then {
            trie_overlay_node_ref(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                updates_ref(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        field_to_ref(f)
                    } else {
                        trie_overlay_node_ref(node, prefix, updates, fuel)
                    }
                } else {
                    trie_empty_ref()
                }
            }
        }
    }
}

function trie_overlay_child_node_for_collapse(
    f : RlpField,
    prefix : list(nibble),
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    list(byte)
) = {
    if updates_empty(updates) then {
        if f.is_list then {
            trie_overlay_node_for_collapse(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                [||]
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        [||]
                    } else {
                        trie_overlay_node_for_collapse(node, prefix, updates, fuel)
                    }
                } else {
                    [||]
                }
            }
        }
    } else {
        if f.is_list then {
            trie_overlay_node_for_collapse(f.full, prefix, updates, fuel)
        } else {
            let c = f.content;
            if list_len(c) == 0 then {
                updates_node(updates, list_len(prefix), fuel)
            } else {
                if list_len(c) == 32 then {
                    let node = node_db_lookup(bytes_be256(c));
                    if list_len(node) == 0 then {
                        k_witness_bad = true;
                        [||]
                    } else {
                        trie_overlay_node_for_collapse(node, prefix, updates, fuel)
                    }
                } else {
                    [||]
                }
            }
        }
    }
}

function trie_overlay_node_for_collapse(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    list(byte)
) = {
    if fuel <= 0 then {
        k_witness_bad = true;
        [||]
    } else {
        if list_len(node) == 0 then {
            updates_node(updates, list_len(prefix), fuel)
        } else {
            let items = rlp_node_items(node);
            let n = list_len(items);
            if n == 2 then {
                let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
                let key = list_append(prefix, seg);
                if is_leaf then {
                    patches_node(
                        leaf_overlay_patches(key, list_nth(items, 1, RLP_NONE).content, updates),
                        list_len(prefix),
                        fuel,
                    )
                } else {
                    if updates_all_under_prefix(updates, key) then {
                        merge_ext_node(
                            seg,
                            trie_overlay_child_node_for_collapse(list_nth(items, 1, RLP_NONE), key, updates, fuel - 1),
                        )
                    } else {
                        let under = updates_under_prefix(updates, key);
                        let outside = updates_outside_prefix(updates, key);
                        if not_bool(updates_empty(under)) & not_bool(updates_has_live(outside)) then {
                            merge_ext_node(
                                seg,
                                trie_overlay_child_node_for_collapse(list_nth(items, 1, RLP_NONE), key, under, fuel - 1),
                            )
                        } else {
                            if updates_empty(under) then {
                                /* untouched witness subtree: a plain hash ref;
                                 * collapse material resolves via the node-db */
                                let old = trie_overlay_child_ref(list_nth(items, 1, RLP_NONE), key, under, fuel - 1);
                                patches_node(
                                    patch_insert_ref_if_live(key, old, patches_from_updates(outside)),
                                    list_len(prefix),
                                    fuel,
                                )
                            } else {
                                /* overlaid subtree: its post-state node exists only
                                 * in memory, so carry the node bytes in the patch --
                                 * sibling deletes may collapse into it, and its fresh
                                 * hash can never be found in the witness node-db */
                                let old_node = trie_overlay_child_node_for_collapse(
                                    list_nth(items, 1, RLP_NONE),
                                    key,
                                    under,
                                    fuel - 1,
                                );
                                patches_node(
                                    patch_insert_subtree_if_live(key, old_node, patches_from_updates(outside)),
                                    list_len(prefix),
                                    fuel,
                                )
                            }
                        }
                    }
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var mask : bits(16) = 0x0000;
                    var children : list(list(byte)) = [||];
                    var lone_nib : nibble = 0x0;
                    var lone_field : RlpField = RLP_NONE;
                    var lone_prefix : list(nibble) = [||];
                    var lone_updates : list(TrieUpdate) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|get_slice_int(4, i, 0)|]);
                        let group = updates_at_nibble(updates, list_len(prefix), get_slice_int(4, i, 0));
                        let cref = trie_overlay_child_ref(field, child_prefix, group, fuel - 1);
                        if not_bool(trie_ref_is_empty(cref)) then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|cref|]);
                            lone_nib = get_slice_int(4, i, 0);
                            lone_field = field;
                            lone_prefix = child_prefix;
                            lone_updates = group
                        }
                    };
                    if mask == 0x0000 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            let cnode = trie_overlay_child_node_for_collapse(
                                lone_field,
                                lone_prefix,
                                lone_updates,
                                fuel - 1,
                            );
                            merge_ext_node([|lone_nib|], cnode)
                        } else {
                            branch_node_rlp(mask, children)
                        }
                    }
                } else {
                    [||]
                }
            }
        }
    }
}

function trie_overlay_node_ref(
    node : list(byte),
    prefix : list(nibble),
    updates : list(TrieUpdate),
    fuel : int,
) -> (
    list(byte)
) = {
    node_to_ref(trie_overlay_node_for_collapse(node, prefix, updates, fuel))
}
termination_measure trie_overlay_node_ref(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_ref(f, prefix, updates, fuel) = fuel
termination_measure trie_overlay_node_for_collapse(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_node_for_collapse(f, prefix, updates, fuel) = fuel
termination_measure trie_overlay_node_ref_preserve(node, prefix, updates, fuel) = fuel
termination_measure trie_overlay_child_ref_preserve(f, prefix, updates, fuel) = fuel


function trie_overlay_root(root : bits(256), updates : list(TrieUpdate)) -> bits(256) = {
    if updates_empty(updates) then {
        root
    } else {
        if root == EMPTY_TRIE_ROOT then {
            updates_root(updates)
        } else {
            let node = node_db_lookup(root);
            if list_len(node) == 0 then {
                k_witness_bad = true;
                root
            } else {
                trie_ref_to_root(trie_overlay_node_ref(node, [||], updates, 256))
            }
        }
    }
}

function trie_overlay_root_preserve(root : bits(256), updates : list(TrieUpdate)) -> bits(256) = {
    if updates_empty(updates) then {
        root
    } else {
        if root == EMPTY_TRIE_ROOT then {
            updates_root(updates)
        } else {
            let node = node_db_lookup(root);
            if list_len(node) == 0 then {
                k_witness_bad = true;
                root
            } else {
                trie_ref_to_root(trie_overlay_node_ref_preserve(node, [||], updates, 256))
            }
        }
    }
}

/* walk the secure trie from `node` following key nibbles; return the leaf value */
function trie_walk(node : list(byte), key : list(nibble), pos : int, fuel : int) -> list(byte) = {
    if fuel <= 0 | list_len(node) == 0 then {
        [||]
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
            if not_bool(nibs_match(key, pos, seg)) then {
                [||]
            } else {
                if is_leaf then {
                    if pos + list_len(seg) == list_len(key) then {
                        list_nth(items, 1, RLP_NONE).content
                    } else {
                        [||]
                    }
                } else {
                    let child = resolve_child(list_nth(items, 1, RLP_NONE));
                    trie_walk(child, key, pos + list_len(seg), fuel - 1)
                }
            }
        } else {
            if n == 17 then {
                if pos == list_len(key) then {
                    list_nth(items, 16, RLP_NONE).content
                } else {
                    let child = resolve_child(list_nth(items, unsigned(list_nth(key, pos, 0x0)), RLP_NONE));
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            } else {
                [||]
            }
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

/* ---- account / storage resolution against the witness ------------------ */

/* populate the C node-db from witness.state: keccak(node) -> span. The node is
 * materialized once here to hash it; the recorded span (not the bytes) is what
 * the walk reads back, so no per-node Sail copy is retained. */
function build_node_db() -> unit = {
    nodedb_reset();
    let n = wit_n_state();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_witness_node_span(i);
        let h = witness_keccak(ssz_witness_node_slice(i));
        nodedb_insert(h, get_slice_int(64, off, 0), get_slice_int(64, len, 0))
    }
}

/* the parent header's state_root (field 3) = pre-state root. The witness headers
 * form a chain header[0] (oldest) .. header[n-1] (newest), where header[i]'s
 * parent is header[i-1] (validated by headers_ok). The block being validated
 * extends the NEWEST, so its parent is header[n-1] -- NOT header[0]. With a
 * single header they coincide; when BLOCKHASH/EIP-2935 pulls in older headers
 * (EIP-8025 witness header collection) the parent is the last one. */
function parent_state_root() -> bits(256) = {
    let items = rlp_node_items(ssz_witness_header(wit_n_headers() - 1));
    bytes_be256(list_nth(items, 3, RLP_NONE).content)
}

/* EIP-8025 (optional proofs): the witness omits an account's storage proof when
 * the block never touches its storage. Such an account has a non-empty committed
 * storage_root whose root node is ABSENT from the witness node-db -- its storage
 * is unchanged, so its post-state storage_root stays the committed value, and
 * any storage read against it is a missing proof. */
function storage_proof_omitted(sroot : bits(256)) -> bool =
    (list_len(node_db_lookup(sroot)) == 0) & (sroot != EMPTY_TRIE_ROOT)

/* Stateless storage resolver (declared in host/state.sail): the (a, s) pre-state
 * value, resolved (and its presence/absence PROVEN) by a lazy Merkle walk of the
 * account's storage trie from its authenticated pre-state root. The node-db is
 * keyed by keccak256(node), so the walk is itself a hash-chain proof from that
 * root:
 *   - the walk reaches the leaf            => present, use that value;
 *   - the walk reaches a diverging/empty
 *     terminus through present nodes        => PROVEN ABSENT => 0;
 *   - the walk hits a blinded (witness-
 *     absent) node on the path              => missing proof => fail closed
 *                                              (k_witness_bad).
 * The callers (k_sload / k_orig, host/base_witness.sail) seed the resolved value
 * into the layered working set, so repeat reads never re-walk. */
function stateless_storage(a : address, s : word) -> word = {
    let sroot = load_account(a).storage_root;
    if storage_proof_omitted(sroot) then {
        /* the block reads storage whose proof the witness omitted */
        k_witness_bad = true;
        sail_zeros(256)
    } else {
        let leaf = trie_walk(node_db_lookup(sroot), hash_nibbles(keccak256_word(s)), 0, 256);
        if list_len(leaf) > 0 then {
            rlp_str_word(leaf)
        } else {
            sail_zeros(256)
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch. Presence/absence is PROVEN by a lazy Merkle walk of the state
 * trie from the authenticated parent root (k_parent_state_root):
 *   - the walk reaches the leaf             => present, decode + seed;
 *   - reaches a diverging/empty terminus
 *     through present nodes                  => PROVEN ABSENT => empty account;
 *   - hits a blinded (witness-absent) node   => missing proof => fail closed
 *                                              (k_witness_bad).
 * So a not-materialized account is never silently treated as empty -- its
 * absence is a witness-proven exclusion. The result is cached raw in the acctmap
 * (k_seed_account_hashed), so subsequent load_account(a) is an O(1) raw read. */
function stateless_account_load(a : address) -> unit = {
    let ak = keccak256_address(a);
    let rootnode = node_db_lookup(k_parent_state_root);
    if (list_len(rootnode) == 0) & (k_parent_state_root != EMPTY_TRIE_ROOT) then {
        /* the anchor root itself is missing from the witness */
        k_witness_bad = true
    };
    let value = trie_walk(rootnode, hash_nibbles(ak), 0, 256);
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        k_seed_account_hashed(a, nonce, bal, sroot, chash);
        acctmap_mark_base_exists(a)
    } else {
        k_seed_account_hashed(a, 0, sail_zeros(256), EMPTY_TRIE_ROOT, KECCAK_EMPTY)
    }
}

/* ---- post-state roots by recursive overlay -----------------------------
 * Storage and account roots consume only ordered update lists. If a root is
 * non-empty, updates are applied by traversing the pre-state trie through the
 * node-db and preserving untouched children by reference. The standalone EEST
 * runner seeds the complete post-state and uses the updates-only entry point
 * because it has no parent root anchor. */
function storage_root_from(ak : bits(256), sroot : bits(256)) -> bits(256) = {
    let updates = storage_updates(ak);
    if updates_empty(updates) then {
        sroot
    } else {
        if storage_proof_omitted(sroot) then {
            k_witness_bad = true;
            sroot
        } else {
            if updates_has_delete(updates) then {
                /* deletes can collapse trie shape; the witness always carries
                 * the collapse material (EIP-8025), so the collapsing overlay
                 * is the single delete path for every backend */
                trie_overlay_root(sroot, updates)
            } else {
                trie_overlay_root_preserve(sroot, updates)
            }
        }
    }
}

function storage_root(a : address) -> bits(256) =
    storage_root_from(keccak256_address(a), load_account(a).storage_root)

function compute_state_root_updates_only() -> bits(256) =
    updates_root(state_updates_map())

function compute_state_root_overlay() -> bits(256) = {
    let updates = state_updates_map();
    let r = if updates_has_delete(updates) then {
            trie_overlay_root(k_parent_state_root, updates)
        } else {
            trie_overlay_root_preserve(k_parent_state_root, updates)
        };
    r
}

/* EIP-7928 block-access-list item count over the RLP-encoded BlockAccessList (a
 * ByteList in the payload): rlp([ [address, storage_changes, storage_reads,
 * balance_changes, nonce_changes, code_changes], ... ]). Item count =
 * #accounts + sum over accounts of (#storage_changes + #storage_reads), where
 * storage_changes (RLP item 1) is a list of [slot, changes] pairs and
 * storage_reads (item 2) is a list of slots. EELS
 * validate_block_access_list_gas_limit rejects the block if this exceeds
 * block_gas_limit / BLOCK_ACCESS_LIST_ITEM (2000). */
function bal_account_items(acc : list(byte)) -> int = {
    let items = rlp_node_items(acc);
    let sc = list_nth(items, 1, RLP_NONE);
    let sr = list_nth(items, 2, RLP_NONE);
    1 + list_len(rlp_node_items(sc.full)) + list_len(rlp_node_items(sr.full))
}

function bal_item_count() -> int = {
    let b = pl_bal_start();
    let e = payload_end();
    if b >= e then {
        0
    } else {
        var total : int = 0;
        var rest : list(RlpField) = rlp_node_items(ssz_bytes(b, e - b));
        var more : bool = true;
        while more do {
            match rest {
                a :: r => {
                    total = total + bal_account_items(a.full);
                    rest = r
                },
                [||] => more = false,
            }
        };
        total
    }
}
