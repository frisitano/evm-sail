/* ======================================================================== */
/* MERKLE-PATRICIA TRIE                                                     */
/*                                                                          */
/* The authenticated key/value structure underlying Ethereum world state    */
/* (Yellow Paper Appendix D), with hex-prefix path encoding (Appendix C).   */
/* This module specifies one trie in three layers.                          */
/*                                                                          */
/* (1) Recursive trie composition: leaf / extension / branch nodes are      */
/* encoded directly; child references embed small nodes or point to         */
/* keccak-256 hashes for large or blinded nodes.                            */
/*                                                                          */
/* (2) World-state trie: the state root over accounts, keyed by             */
/* keccak256(address) with value RLP([nonce, balance, storageRoot,          */
/* codeHash]) over per-account storage tries. This is the commitment a      */
/* block's stateRoot must equal.                                            */
/*                                                                          */
/* (3) Witness reads: stateless verification -- re-root a partial trie from */
/* a node witness to authenticate it against a committed root, and resolve  */
/* account/storage values by Merkle proof.                                  */
/* ======================================================================== */

/* ======================================================================== */
/* (1) trie node references and recursive composition                       */
/*                                                                          */
/* Trie nodes are composed directly in the Yellow Paper Appendix D shape:   */
/* leaf, extension, and branch. A child is either embedded RLP, or an       */
/* external 0xa0 ++ keccak256(node) reference. Post-state roots are computed*/
/* by recursively traversing the witnessed node database and applying an    */
/* ordered update list only along touched paths; untouched children are     */
/* preserved by reference.                                                  */
/* ======================================================================== */

/* ---- small generic list helpers (take/drop n from the front) ----------- */
/* ---- shared trie primitives (formerly lib/mpt.sail) -------------------- */

/* 32-byte key -> 64 nibbles, high nibble first (single pass + reverse). */
function to_nibbles(bs : list(byte)) -> list(int) = {
    var out : list(int) = [||];
    var rest : list(byte) = bs;
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            b :: r => {
                let bb = unsigned(b);
                out = ediv_int(bb, 16) :: out;
                out = emod_int(bb, 16) :: out;
                rest = r
            },
            [||] => (),
        }
    };
    list_reverse(out)
}

/* hex-prefix (compact) encoding of a nibble path + leaf/extension flag (YP C.1). */
function hp_compact(nibs : list(int), is_leaf : bool) -> list(byte) = {
    let len = list_len(nibs);
    let odd = emod_int(len, 2) == 1;
    let flag : int = if is_leaf then { 2 } else { 0 };
    var out : list(byte) = [||];
    var rest : list(int) = nibs;
    if odd then {
        match rest {
            n0 :: t => {
                out = get_slice_int(8, (flag + 1) * 16 + n0, 0) :: out;
                rest = t
            },
            [||] => (),
        }
    } else {
        out = get_slice_int(8, flag * 16, 0) :: out
    };
    while match rest {
        [||] => false,
        _ => true,
    } do {
        match rest {
            a :: (b :: t) => {
                out = get_slice_int(8, a * 16 + b, 0) :: out;
                rest = t
            },
            _ => rest = [||],
        }
    };
    list_reverse(out)
}

/* reference to a child node (Yellow Paper Appendix D, the node cap function n):
 * the node's RLP inline when under 32 bytes, otherwise 0xa0 ++ keccak256(node). */
function child_ref(enc : list(byte)) -> list(byte) =
    if list_len(enc) < 32 then {
        enc
    } else {
        rlp_bytes(word_to_bytes32(keccak256(enc)))
    }

/* A trie leaf VALUE is the RLP encoding of the slot value (a string), e.g.
 * RLP(132) = 0x81 0x84, RLP(32-byte) = 0xa0 ++ 32 bytes. rlp_field_word reads
 * content as a raw big-endian word, which is only correct AFTER the RLP
 * string header is stripped: decode the short-string header (0x80..0xb7 -> drop
 * the length byte; a single byte < 0x80 is the value itself). Without this, a
 * small value keeps its 0x81 prefix (e.g. -> 0x8184) and is double-encoded on
 * re-RLP; large 32-byte values happened to work only because the 0xa0 prefix
 * overflows out of the 256-bit word. */
function rlp_str_word(bs : list(byte)) -> word =
    match bs {
        [||] => sail_zeros(256),
        b0 :: rest => if unsigned(b0) < 128 then {
            rlp_field_word(struct { is_list = false, content = bs, full = bs })
        } else {
            rlp_field_word(struct { is_list = false, content = rest, full = rest })
        },
    }

function list_take_prefix forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var out : list('a) = [||];
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            h :: t => {
                out = h :: out;
                rest = t
            },
            [||] => (),
        };
        i = i + 1
    };
    list_reverse(out)
}
function list_drop_prefix forall ('a : Type). (xs : list('a), n : int) -> list('a) = {
    var rest : list('a) = xs;
    var i : int = 0;
    while i < n do {
        match rest {
            _ :: t => rest = t,
            [||] => (),
        };
        i = i + 1
    };
    rest
}

/* ---- bit / mask helpers (16-bit branch masks held as int) --------------- */
function branch_mask_pow2(n : int) -> int = {
    var r : int = 1;
    var i : int = 0;
    while i < n do {
        r = r * 2;
        i = i + 1
    };
    r
}
function branch_mask_has_bit(mask : int, i : int) -> bool = emod_int(ediv_int(mask, branch_mask_pow2(i)), 2) == 1
function branch_mask_set_bit(mask : int, i : int) -> int = if branch_mask_has_bit(mask, i) then {
    mask
} else {
    mask + branch_mask_pow2(i)
}
function branch_mask_popcount(mask : int) -> int = {
    var c : int = 0;
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            c = c + 1
        }
    };
    c
}

/* longest common prefix length of two nibble lists */
function common_prefix_len(a : list(int), b : list(int)) -> int = {
    var i : int = 0;
    var ra : list(int) = a;
    var rb : list(int) = b;
    var stop : bool = false;
    while not_bool(stop) do {
        match (ra, rb) {
            (x :: ta, y :: tb) => if x == y then {
                i = i + 1;
                ra = ta;
                rb = tb
            } else {
                stop = true
            },
            (_, _) => stop = true,
        }
    };
    i
}

/* 32 big-endian bytes -> bits(256) (local; avoids clash with mpt_witness) */
function bytes_to_word_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(list_nth(c, k, 0x00), 256))
    };
    acc
}

/* ---- node child-refs (the values the stack holds) ----------------------- */
/* leaf:      child_ref( rlp[ hp_compact(key,leaf) , value ] )                 */
/* extension: child_ref( rlp[ hp_compact(key,ext)  , child-ref ] )            */
/* branch:    child_ref( rlp[ 16 child-refs-or-0x80 , 0x80 ] )                 */
/* word:      0xa0 ++ hash   (a blinded subtree / >=32B child, already a ref)  */
function leaf_node_rlp(key : list(int), value : list(byte)) -> list(byte) =
    rlp_list(list_append(rlp_bytes(hp_compact(key, true)), rlp_bytes(value)))
function extension_node_rlp(key : list(int), childref : list(byte)) -> list(byte) =
    rlp_list(list_append(rlp_bytes(hp_compact(key, false)), childref))
function leaf_child_ref(key : list(int), value : list(byte)) -> list(byte) =
    child_ref(leaf_node_rlp(key, value))
function extension_child_ref(key : list(int), childref : list(byte)) -> list(byte) =
    child_ref(extension_node_rlp(key, childref))
function hash_child_ref(hash : list(byte)) -> list(byte) = list_append([|0xa0|], hash)

function branch_payload_rlp(mask : int, children : list(list(byte))) -> list(byte) = {
    var payload : list(byte) = [||];
    var ch : list(list(byte)) = children; /* ascending nibble order */
    foreach (i from 0 to 15) {
        if branch_mask_has_bit(mask, i) then {
            match ch {
                c :: t => {
                    payload = list_append(payload, c);
                    ch = t
                },
                [||] => (),
            }
        } else {
            payload = list_append(payload, [|0x80|])
        }
    };
    list_append(payload, [|0x80|]) /* branch value slot: empty */
}
function branch_node_rlp(mask : int, children : list(list(byte))) -> list(byte) =
    rlp_list(branch_payload_rlp(mask, children))
function branch_child_ref(mask : int, children : list(list(byte))) -> list(byte) =
    child_ref(branch_node_rlp(mask, children))

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

/* ---- ordered post-state updates ---------------------------------------- */
/* lexicographic nibble-key order for recursive trie composition */
struct TrieUpdate = { key : list(int), is_delete : bool, value : list(byte) }

function nib_lt(a : list(int), b : list(int)) -> bool = {
    var ra : list(int) = a;
    var rb : list(int) = b;
    var res : int = 0; /* 0 undecided, 1 a<b, 2 a>=b */
    while res == 0 do {
        match (ra, rb) {
            ([||], [||]) => res = 2,
            ([||], _) => res = 1,
            (_, [||]) => res = 2,
            (x :: ta, y :: tb) => if x < y then {
                res = 1
            } else {
                if x > y then {
                    res = 2
                } else {
                    ra = ta;
                    rb = tb
                }
            },
        }
    };
    res == 1
}
function nib_eq(a : list(int), b : list(int)) -> bool =
    not_bool(nib_lt(a, b)) & not_bool(nib_lt(b, a))
function nib_prefix_of(prefix : list(int), key : list(int)) -> bool =
    match prefix {
        [||] => true,
        x :: xs => match key {
            [||] => false,
            y :: ys => (x == y) & nib_prefix_of(xs, ys),
        },
    }

function update_insert_sorted(u : TrieUpdate, us : list(TrieUpdate)) -> list(TrieUpdate) =
    match us {
        [||] => [|u|],
        v :: r => if nib_eq(u.key, v.key) then {
            u :: r
        } else {
            if nib_lt(u.key, v.key) then {
                u :: us
            } else {
                v :: update_insert_sorted(u, r)
            }
        },
    }

function updates_insert_all(src : list(TrieUpdate), dst : list(TrieUpdate)) -> list(TrieUpdate) =
    match src {
        [||]   => dst,
        u :: r => updates_insert_all(r, update_insert_sorted(u, dst)),
    }

function updates_strictly_sorted(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => true,
        u :: r => match r {
            [||]   => true,
            v :: _ => nib_lt(u.key, v.key) & updates_strictly_sorted(r),
        },
    }

function updates_empty(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => true,
        _    => false,
    }

function updates_has_live(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => false,
        u :: r => if u.is_delete then {
            updates_has_live(r)
        } else {
            true
        },
    }

function updates_has_delete(us : list(TrieUpdate)) -> bool =
    match us {
        [||] => false,
        u :: r => u.is_delete | updates_has_delete(r),
    }

function updates_at_nibble(us : list(TrieUpdate), depth : int, nib : int) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if list_len(u.key) > depth & list_nth(u.key, depth, -1) == nib then {
            u :: updates_at_nibble(r, depth, nib)
        } else {
            updates_at_nibble(r, depth, nib)
        },
    }

function updates_all_under_prefix(us : list(TrieUpdate), prefix : list(int)) -> bool =
    match us {
        [||]   => true,
        u :: r => nib_prefix_of(prefix, u.key) & updates_all_under_prefix(r, prefix),
    }

function updates_under_prefix(us : list(TrieUpdate), prefix : list(int)) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if nib_prefix_of(prefix, u.key) then {
            u :: updates_under_prefix(r, prefix)
        } else {
            updates_under_prefix(r, prefix)
        },
    }

function updates_outside_prefix(us : list(TrieUpdate), prefix : list(int)) -> list(TrieUpdate) =
    match us {
        [||] => [||],
        u :: r => if nib_prefix_of(prefix, u.key) then {
            updates_outside_prefix(r, prefix)
        } else {
            u :: updates_outside_prefix(r, prefix)
        },
    }

function updates_has_key(us : list(TrieUpdate), key : list(int)) -> bool =
    match us {
        [||] => false,
        u :: r => if nib_eq(u.key, key) then {
            true
        } else {
            updates_has_key(r, key)
        },
    }

/* C-backed witness storage-slot store (ffi/trie_node_db.c):
 * (keccak(addr), keccak(slot)) -> 256-bit value. The harvested, authenticated
 * pre-state storage. Point lookup (slotdb_sel + slotdb_selval) serves stateless
 * reads; iteration (slotdb_count + slotdb_at + slotdb_at_*) feeds the post-state
 * root. Each 256-bit key half crosses as four big-endian words (w3 most sig). */
val slotdb_reset = impure { _: "slotdb_reset" } : unit -> unit
val slotdb_insert = impure { _: "slotdb_insert" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> unit
val slotdb_sel = impure { _: "slotdb_sel" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bits(64)
val slotdb_selval = impure { _: "slotdb_selval" } : bits(64) -> bits(64)
val slotdb_sel_existed = impure { _: "slotdb_sel_existed" } : unit -> bits(64)
val slotdb_count = impure { _: "slotdb_count" } : unit -> bits(64)
val slotdb_at = impure { _: "slotdb_at" } : bits(64) -> unit
val slotdb_at_acct = impure { _: "slotdb_at_acct" } : bits(64) -> bits(64)
val slotdb_at_slot = impure { _: "slotdb_at_slot" } : bits(64) -> bits(64)
val slotdb_at_val = impure { _: "slotdb_at_val" } : bits(64) -> bits(64)
val slotdb_at_existed = impure { _: "slotdb_at_existed" } : unit -> bits(64)
val storage_mark_incomplete = impure { _: "storage_mark_incomplete" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
val storage_harvest_complete_raw = impure { _: "storage_harvest_complete" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(
        64,
    )

/* harvest context: while re-rooting an account's storage trie, the recursive
 * witness leaf sink switches from the account leaf store to slotdb, keyed by
 * hv_acct. */
register hv_storage : bool = false
register hv_acct : bits(256) = sail_zeros(256)

/* insert a harvested (acct, slot) -> value into the slot store */
function slot_insert(acct : bits(256), slot : bits(256), v : word) -> unit =
    slotdb_insert(
        acct[255 .. 192],
        acct[191 .. 128],
        acct[127 .. 64],
        acct[63 .. 0],
        slot[255 .. 192],
        slot[191 .. 128],
        slot[127 .. 64],
        slot[63 .. 0],
        v[255 .. 192],
        v[191 .. 128],
        v[127 .. 64],
        v[63 .. 0],
    )

function storage_mark_partial(acct : bits(256)) -> unit =
    storage_mark_incomplete(acct[255 .. 192], acct[191 .. 128], acct[127 .. 64], acct[63 .. 0])

function storage_harvest_complete(acct : bits(256)) -> bool =
    storage_harvest_complete_raw(acct[255 .. 192], acct[191 .. 128], acct[127 .. 64], acct[63 .. 0]) !=
        0x0000000000000000

function slot_existed(acct : bits(256), slot : bits(256)) -> bool =
    (slotdb_sel(
        acct[255 .. 192],
        acct[191 .. 128],
        acct[127 .. 64],
        acct[63 .. 0],
        slot[255 .. 192],
        slot[191 .. 128],
        slot[127 .. 64],
        slot[63 .. 0],
    ) !=
        0x0000000000000000) &
        (slotdb_sel_existed() != 0x0000000000000000)

/* the cached slotdb_sel value as a 256-bit word */
function slot_selword() -> word =
    append(
        slotdb_selval(get_slice_int(64, 3, 0)),
        append(
            slotdb_selval(get_slice_int(64, 2, 0)),
            append(slotdb_selval(get_slice_int(64, 1, 0)), slotdb_selval(get_slice_int(64, 0, 0))),
        ),
    )

/* 256-bit acct key / slot key / value of the currently-selected slotdb row */
function slot_at_slot() -> bits(256) =
    append(
        slotdb_at_slot(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_slot(get_slice_int(64, 2, 0)),
            append(slotdb_at_slot(get_slice_int(64, 1, 0)), slotdb_at_slot(get_slice_int(64, 0, 0))),
        ),
    )
function slot_at_val() -> word =
    append(
        slotdb_at_val(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_val(get_slice_int(64, 2, 0)),
            append(slotdb_at_val(get_slice_int(64, 1, 0)), slotdb_at_val(get_slice_int(64, 0, 0))),
        ),
    )
function slot_at_acct() -> bits(256) =
    append(
        slotdb_at_acct(get_slice_int(64, 3, 0)),
        append(
            slotdb_at_acct(get_slice_int(64, 2, 0)),
            append(slotdb_at_acct(get_slice_int(64, 1, 0)), slotdb_at_acct(get_slice_int(64, 0, 0))),
        ),
    )

/* secure-trie updates for one account's post-state storage, selected by
 * keccak(address). In stateless mode the update list is only the write set:
 * nonzero writes set RLP(value), zero writes delete the slot. Untouched storage
 * slots remain inside the recursive overlay rooted at the account's pre-state
 * storage root. Native fixture mode has no authenticated witness root, so it
 * enumerates the full live cache/update union. */
function storage_updates(ak : bits(256)) -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = if k_stateless then {
            storage_update_count()
        } else {
            storage_count()
        };
    foreach (j from 0 to (n - 1)) {
        let (eak, es, ev) = if k_stateless then {
                storage_hashed_update_entry(j)
            } else {
                storage_hashed_entry(j)
            };
        if eak == ak then {
            let sk = keccak256(word_to_bytes32(es));
            if k_stateless & word_is_zero(ev) & not_bool(slot_existed(ak, sk)) then {
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

/* Full live post-state storage for a stateless account whose storage harvest is
 * complete. This is used for delete overlays: deletion can collapse
 * branch/extension structure in ways that would otherwise require materializing
 * an untouched child. The slotdb rows are the authenticated pre-state storage
 * leaves, while `writes` is the ordered transaction update set; composing the
 * surviving slotdb entries plus live writes gives the same textbook sorted-map
 * trie build as execution-specs. Partial optional-proof witnesses must use the
 * preserving overlay instead, because omitted blinded children may contain
 * untouched live slots. */
function storage_live_updates_from_harvest(ak : bits(256), writes : list(TrieUpdate)) -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = unsigned(slotdb_count());
    foreach (j from 0 to (n - 1)) {
        slotdb_at(get_slice_int(64, j, 0));
        if slot_at_acct() == ak then {
            let sk = slot_at_slot();
            let key = to_nibbles(word_to_bytes32(sk));
            let slot_val = slot_at_val();
            if   (slotdb_at_existed() != 0x0000000000000000)
               & word_nonzero(slot_val)
               & not_bool(updates_has_key(writes, key)) then {
                out = update_insert_sorted(
                    struct { key = key, is_delete = false, value = rlp_int(unsigned(slot_val)) },
                    out,
                )
            }
        }
    };
    updates_insert_all(writes, out)
}

/* post-state storage root for keccak(address). Defined after the recursive
 * overlay walker. */
val storage_root : address -> bits(256)
val storage_root_from : (bits(256), bits(256)) -> bits(256)

function account_empty(acc : Account) -> bool =
    (acc.code_hash == KECCAK_EMPTY) & acc.nonce == 0 & word_is_zero(acc.balance)

/* 256-bit account field (balance / code_hash) of the current acctmap_at row */
function selected_account_balance() -> word =
    append(
        acctmap_at_bal(get_slice_int(64, 3, 0)),
        append(
            acctmap_at_bal(get_slice_int(64, 2, 0)),
            append(acctmap_at_bal(get_slice_int(64, 1, 0)), acctmap_at_bal(get_slice_int(64, 0, 0))),
        ),
    )
function selected_account_code_hash() -> word =
    append(
        acctmap_at_chash(get_slice_int(64, 3, 0)),
        append(
            acctmap_at_chash(get_slice_int(64, 2, 0)),
            append(acctmap_at_chash(get_slice_int(64, 1, 0)), acctmap_at_chash(get_slice_int(64, 0, 0))),
        ),
    )
function selected_account_storage_root() -> word =
    append(
        acctmap_at_sroot(get_slice_int(64, 3, 0)),
        append(
            acctmap_at_sroot(get_slice_int(64, 2, 0)),
            append(acctmap_at_sroot(get_slice_int(64, 1, 0)), acctmap_at_sroot(get_slice_int(64, 0, 0))),
        ),
    )

/* secure trie key of the current acctmap_at row (keccak(address), h3..h0) */
function selected_account_key_hash() -> bits(256) =
    append(
        acctmap_at_hkey(get_slice_int(64, 0, 0)),
        append(
            acctmap_at_hkey(get_slice_int(64, 1, 0)),
            append(acctmap_at_hkey(get_slice_int(64, 2, 0)), acctmap_at_hkey(get_slice_int(64, 3, 0))),
        ),
    )

/* secure-trie updates for the account map. Stateless block validation consumes
 * only execution updates: witness-materialized read-only accounts are base-state
 * cache, not post-state updates. The native EEST runner has no parent root
 * anchor, so it uses the complete cache/update union to build a full
 * updates-only root. */
function state_updates_map() -> list(TrieUpdate) = {
    var out : list(TrieUpdate) = [||];
    let n = unsigned(
        if k_stateless then {
            acctmap_update_count()
        } else {
            acctmap_count()
        },
    );
    foreach (j from 0 to (n - 1)) {
        if k_stateless then {
            acctmap_update_at(get_slice_int(64, j, 0))
        } else {
            acctmap_at(get_slice_int(64, j, 0))
        };
        let ak_hash = selected_account_key_hash();
        let acc : Account = struct {
                nonce = unsigned(acctmap_at_nonce()),
                balance = selected_account_balance(),
                code_hash = selected_account_code_hash(),
                storage_root = selected_account_storage_root(),
            };
        let ak = to_nibbles(word_to_bytes32(ak_hash));
        let base_exists = unsigned(acctmap_at_base_exists()) == 1;
        if k_stateless & account_empty(acc) & not_bool(base_exists) then {
            ()
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
            out = update_insert_sorted(upd, out)
        }
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
/* ffi/trie_node_db.c) backs two operations.                                 */
/*                                                                          */
/* Re-rooting recursively traverses the witnessed nodes and checks the      */
/* result against the committed pre-state root, authenticating every        */
/* supplied node at once. Point reads walk the secure trie by               */
/* keccak256(key) nibbles (branch / extension / leaf, hex-prefix paths) to  */
/* the account leaf and decode RLP([nonce, balance, storageRoot,            */
/* codeHash]); storage reads walk the account's storage root the same way.  */
/*                                                                          */
/* A node absent from the witness on a walked path is a missing proof and   */
/* fails closed. Depends on the RLP decoder (rlp_decode.sail) and the SSZ   */
/* witness-list readers (host/io.sail).                                     */
/* ======================================================================== */

/* EMPTY_TRIE_ROOT is defined in host/state.sail (next to KECCAK_EMPTY) so the
 * Account struct's storage_root default can reference it. */
let EMPTY_CODE_HASH : bits(256) = 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470

let RLP_NONE : RlpField = struct { is_list = false, content = [||], full = [||] }

/* big-endian bytes -> 256-bit word */
function bytes_be256(c : list(byte)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(c);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(bytes_get(c, k), 256))
    };
    acc
}

/* decode an account leaf value: (nonce, balance, storage_root, code_hash).
 * Defined here (ahead of the witness reader) so both the inline storage harvest
 * and the lazy account read can use it. */
function decode_account(leaf : list(byte)) -> (int, word, bits(256), bits(256)) = {
    let items = rlp_node_items(leaf);
    let nonce = rlp_field_int(list_nth(items, 0, RLP_NONE));
    let balance = rlp_field_word(list_nth(items, 1, RLP_NONE));
    let sr_f = list_nth(items, 2, RLP_NONE);
    let ch_f = list_nth(items, 3, RLP_NONE);
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
function hash_nibbles(h : bits(256)) -> list(int) = {
    var rev : list(int) = [||];
    foreach (i from 0 to 63) {
        rev = unsigned(sail_shiftright(h, 4 * (63 - i))[3 .. 0]) :: rev
    };
    list_reverse(rev)
}

/* hex-prefix decode: (is_leaf, path nibbles) */
function hp_decode(p : list(byte)) -> (bool, list(int)) = {
    let n = list_len(p);
    if n == 0 then {
        (false, [||])
    } else {
        let fb = unsigned(bytes_get(p, 0));
        let flag = ediv_int(fb, 16);
        let is_leaf : bool = emod_int(flag, 4) >= 2;
        let odd : bool = emod_int(flag, 2) == 1;
        var rev : list(int) = [||];
        if odd then {
            rev = emod_int(fb, 16) :: rev
        };
        foreach (i from 1 to (n - 1)) {
            let byte = unsigned(bytes_get(p, i));
            rev = ediv_int(byte, 16) :: rev;
            rev = emod_int(byte, 16) :: rev
        };
        (is_leaf, list_reverse(rev))
    }
}

/* do seg nibbles match key[pos ..] ? */
function nibs_match(key : list(int), pos : int, seg : list(int)) -> bool = {
    let m = list_len(seg);
    var ok : bool = true;
    foreach (k from 0 to (m - 1)) {
        if list_nth(key, pos + k, -1) != list_nth(seg, k, -2) then {
            ok = false
        }
    };
    ok
}

/* node db: keccak(node) -> node bytes (linear scan; witnesses are small) */
/* C-backed node-db (ffi/trie_node_db.c): keccak(node) -> (offset, len) span into
 * the SSZ witness buffer. Replaces a Sail assoc list whose linear node_db_lookup
 * (plus the whole list deep-copied through every recursive walk) made re-rooting
 * O(N^2); this is an O(1)-amortized hash map, so the walk is O(N). The 256-bit
 * key crosses as four big-endian words (k3 most significant); nodedb_sel caches
 * the matched span for the off/len accessors. */
val nodedb_reset = impure { _: "nodedb_reset" } : unit -> unit
val nodedb_insert = impure { _: "nodedb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val nodedb_sel = impure { _: "nodedb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val nodedb_sel_off = impure { _: "nodedb_sel_off" } : unit -> bits(64)
val nodedb_sel_len = impure { _: "nodedb_sel_len" } : unit -> bits(64)

/* C-backed witness account-leaf store (ffi/trie_node_db.c): keccak(addr) ->
 * (offset, len) span of the account's leaf node in the SSZ witness buffer.
 * Recorded during the state re-root walk; a stateless account read looks the span
 * up here (O(1)) and decodes the leaf lazily. Same span model as the node-db. */
val acctdb_reset = impure { _: "acctdb_reset" } : unit -> unit
val acctdb_insert = impure { _: "acctdb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val acctdb_sel = impure { _: "acctdb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val acctdb_sel_off = impure { _: "acctdb_sel_off" } : unit -> bits(64)
val acctdb_sel_len = impure { _: "acctdb_sel_len" } : unit -> bits(64)
val acctdb_count = impure { _: "acctdb_count" } : unit -> bits(64)
val acctdb_at = impure { _: "acctdb_at" } : bits(64) -> unit
val acctdb_at_key = impure { _: "acctdb_at_key" } : bits(64) -> bits(64)
val acctdb_at_off = impure { _: "acctdb_at_off" } : unit -> bits(64)
val acctdb_at_len = impure { _: "acctdb_at_len" } : unit -> bits(64)

/* node bytes for keccak `h` from the C node-db, or [] if absent (read straight
 * from the witness buffer by the recorded span -- no per-node Sail copy held). */
function node_db_lookup(h : bits(256)) -> list(byte) = {
    if nodedb_sel(h[255 .. 192], h[191 .. 128], h[127 .. 64], h[63 .. 0]) == 0x0000000000000000 then {
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

struct TriePatch = { key : list(int), is_delete : bool, is_ref : bool, bytes : list(byte) }

function trie_empty_ref() -> list(byte) = [|0x80|]

function trie_ref_is_empty(r : list(byte)) -> bool =
    list_len(r) == 1 & list_nth(r, 0, 0x00) == 0x80

function trie_ref_to_root(r : list(byte)) -> bits(256) =
    match r {
        0xa0 :: h => if list_len(h) == 32 then {
            bytes_to_word_be256(h)
        } else {
            keccak256(r)
        },
        _ => keccak256(r),
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

function node_to_ref(node : list(byte)) -> list(byte) =
    if list_len(node) == 0 then {
        trie_empty_ref()
    } else {
        child_ref(node)
    }

function merge_ext_node(prefix : list(int), childnode : list(byte)) -> list(byte) = {
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

function merge_ext_ref(prefix : list(int), childref : list(byte)) -> list(byte) = {
    if list_len(prefix) == 0 then {
        childref
    } else {
        if trie_ref_is_empty(childref) then {
            trie_empty_ref()
        } else {
            match childref {
                0xa0 :: h => if list_len(h) == 32 then {
                    extension_child_ref(prefix, childref)
                } else {
                    extension_child_ref(prefix, childref)
                },
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

function mask_single_index(mask : int) -> int = {
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

function canonical_branch_ref(mask : int, children : list(list(byte))) -> list(byte) =
    if mask == 0 then {
        trie_empty_ref()
    } else {
        if branch_mask_popcount(mask) == 1 then {
            merge_ext_ref([|mask_single_index(mask)|], first_child_ref(children))
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
    struct { key = u.key, is_delete = u.is_delete, is_ref = false, bytes = u.value }

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

function patch_ref(key : list(int), child : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = true, bytes = child }

function patch_insert_ref_if_live(key : list(int), child : list(byte), ps : list(TriePatch)) -> list(TriePatch) =
    if trie_ref_is_empty(child) then {
        ps
    } else {
        patch_insert_sorted(patch_ref(key, child), ps)
    }

function patch_leaf(key : list(int), value : list(byte)) -> TriePatch =
    struct { key = key, is_delete = false, is_ref = false, bytes = value }

function patches_first_key(ps : list(TriePatch)) -> list(int) =
    match ps {
        [||]   => [||],
        p :: _ => p.key,
    }

function patches_last_key(ps : list(TriePatch)) -> list(int) =
    match ps {
        [||] => [||],
        p :: r => match r {
            [||] => p.key,
            _    => patches_last_key(r),
        },
    }

function patches_at_nibble(ps : list(TriePatch), depth : int, nib : int) -> list(TriePatch) =
    match ps {
        [||] => [||],
        p :: r => if list_len(p.key) > depth & list_nth(p.key, depth, -1) == nib then {
            p :: patches_at_nibble(r, depth, nib)
        } else {
            patches_at_nibble(r, depth, nib)
        },
    }

function patch_entry_ref(p : TriePatch, depth : int) -> list(byte) = {
    let suffix = list_drop_prefix(p.key, depth);
    if p.is_ref then {
        merge_ext_ref(suffix, p.bytes)
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
        if list_len(suffix) == 0 then {
            ref_node_for_collapse(p.bytes)
        } else {
            extension_node_rlp(suffix, p.bytes)
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
                    var mask : int = 0;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, i);
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
                    var mask : int = 0;
                    var children : list(list(byte)) = [||];
                    var lone_index : int = 0;
                    var lone_child : list(byte) = [||];
                    foreach (i from 0 to 15) {
                        let group = patches_at_nibble(ps, depth, i);
                        if list_len(group) > 0 then {
                            let cnode = patches_node(group, depth + 1, fuel - 1);
                            if list_len(cnode) > 0 then {
                                mask = branch_mask_set_bit(mask, i);
                                children = list_append(children, [|node_to_ref(cnode)|]);
                                lone_index = i;
                                lone_child = cnode
                            }
                        }
                    };
                    if mask == 0 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            merge_ext_node([|lone_index|], lone_child)
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

function updates_node(us : list(TrieUpdate), depth : int, fuel : int) -> list(byte) =
    patches_node(patches_from_updates(us), depth, fuel)

function updates_ref(us : list(TrieUpdate), depth : int, fuel : int) -> list(byte) =
    node_to_ref(updates_node(us, depth, fuel))

function leaf_overlay_patches(key : list(int), value : list(byte), us : list(TrieUpdate)) -> list(TriePatch) = {
    let ups = patches_from_updates(us);
    if updates_has_key(us, key) then {
        ups
    } else {
        patch_insert_sorted(patch_leaf(key, value), ups)
    }
}

val trie_overlay_node_ref : (list(byte), list(int), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_ref : (RlpField, list(int), list(TrieUpdate), int) -> list(byte)
val trie_overlay_node_for_collapse : (list(byte), list(int), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_node_for_collapse : (RlpField, list(int), list(TrieUpdate), int) -> list(byte)
val trie_overlay_node_ref_preserve : (list(byte), list(int), list(TrieUpdate), int) -> list(byte)
val trie_overlay_child_ref_preserve : (RlpField, list(int), list(TrieUpdate), int) -> list(byte)

function trie_overlay_child_ref_preserve(
    f : RlpField,
    prefix : list(int),
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
    prefix : list(int),
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
                    var mask : int = 0;
                    var children : list(list(byte)) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|i|]);
                        let group = updates_at_nibble(updates, list_len(prefix), i);
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

function trie_overlay_child_ref(f : RlpField, prefix : list(int), updates : list(TrieUpdate), fuel : int) -> list(byte) = {
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
    prefix : list(int),
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
    prefix : list(int),
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
                            let old = trie_overlay_child_ref(list_nth(items, 1, RLP_NONE), key, under, fuel - 1);
                            patches_node(
                                patch_insert_ref_if_live(key, old, patches_from_updates(outside)),
                                list_len(prefix),
                                fuel,
                            )
                        }
                    }
                }
            } else {
                if n == 17 then {
                    let branch_value = list_nth(items, 16, RLP_NONE).content;
                    if list_len(branch_value) > 0 then {
                        k_witness_bad = true
                    };
                    var mask : int = 0;
                    var children : list(list(byte)) = [||];
                    var lone_index : int = 0;
                    var lone_field : RlpField = RLP_NONE;
                    var lone_prefix : list(int) = [||];
                    var lone_updates : list(TrieUpdate) = [||];
                    foreach (i from 0 to 15) {
                        let field = list_nth(items, i, RLP_NONE);
                        let child_prefix = list_append(prefix, [|i|]);
                        let group = updates_at_nibble(updates, list_len(prefix), i);
                        let cref = trie_overlay_child_ref(field, child_prefix, group, fuel - 1);
                        if not_bool(trie_ref_is_empty(cref)) then {
                            mask = branch_mask_set_bit(mask, i);
                            children = list_append(children, [|cref|]);
                            lone_index = i;
                            lone_field = field;
                            lone_prefix = child_prefix;
                            lone_updates = group
                        }
                    };
                    if mask == 0 then {
                        [||]
                    } else {
                        if branch_mask_popcount(mask) == 1 then {
                            let cnode = trie_overlay_child_node_for_collapse(
                                lone_field,
                                lone_prefix,
                                lone_updates,
                                fuel - 1,
                            );
                            merge_ext_node([|lone_index|], cnode)
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
    prefix : list(int),
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

function updates_root(us : list(TrieUpdate)) -> bits(256) =
    trie_ref_to_root(updates_ref(us, 0, 256))

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
function trie_walk(node : list(byte), key : list(int), pos : int, fuel : int) -> list(byte) = {
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
                    let child = resolve_child(list_nth(items, list_nth(key, pos, 0), RLP_NONE));
                    trie_walk(child, key, pos + 1, fuel - 1)
                }
            } else {
                [||]
            }
        }
    }
}
termination_measure trie_walk(node, key, pos, fuel) = fuel

/* ---- witness recursive re-root ----------------------------------------- */
/* A left-to-right recursive walk reconstructs the witnessed trie root. Present
 * children are traversed; child hashes missing from the node-db are preserved as
 * blinded frontier references. Account leaves are recorded in acctdb for lazy
 * reads, and storage leaves are harvested into slotdb during storage traversal. */

/* 64 nibbles (MSB-first) -> bits(256) */
function nibbles_to_h256(nibs : list(int)) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    let n = list_len(nibs);
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 4), sail_zero_extend(get_slice_int(4, list_nth(nibs, k, 0), 0), 256))
    };
    acc
}

val witness_node_ref : (list(byte), list(int), int) -> list(byte)
val witness_child_ref : (RlpField, list(int), int) -> list(byte)

function record_witness_leaf(node : list(byte), key : list(int), value : list(byte)) -> unit = {
    if list_len(key) == 64 then {
        if hv_storage then {
            let sv = rlp_str_word(value);
            slot_insert(hv_acct, nibbles_to_h256(key), sv)
        } else {
            let ak = nibbles_to_h256(key);
            let hn = keccak256(node);
            let _ = nodedb_sel(hn[255 .. 192], hn[191 .. 128], hn[127 .. 64], hn[63 .. 0]);
            acctdb_insert(
                ak[255 .. 192],
                ak[191 .. 128],
                ak[127 .. 64],
                ak[63 .. 0],
                nodedb_sel_off(),
                nodedb_sel_len(),
            )
        }
    }
}

/* child field at nibble-path `key`: descend if present, preserve if blinded */
function witness_child_ref(f : RlpField, key : list(int), fuel : int) -> list(byte) = {
    if f.is_list then {
        witness_node_ref(f.full, key, fuel)
    } else {
        let c = f.content;
        if list_len(c) == 0 then {
            trie_empty_ref()
        } else {
            if list_len(c) == 32 then {
                let node = node_db_lookup(bytes_be256(c));
                if list_len(node) == 0 then {
                    if hv_storage then {
                        storage_mark_partial(hv_acct)
                    };
                    hash_child_ref(c)
                } else {
                    witness_node_ref(node, key, fuel)
                }
            } else {
                trie_empty_ref()
            }
        }
    }
}

function witness_node_ref(node : list(byte), prefix : list(int), fuel : int) -> list(byte) = {
    if fuel <= 0 | list_len(node) == 0 then {
        if fuel <= 0 then {
            k_witness_bad = true
        };
        trie_empty_ref()
    } else {
        let items = rlp_node_items(node);
        let n = list_len(items);
        if n == 2 then {
            let (is_leaf, seg) = hp_decode(list_nth(items, 0, RLP_NONE).content);
            let key = list_append(prefix, seg);
            if is_leaf then {
                let value = list_nth(items, 1, RLP_NONE).content;
                record_witness_leaf(node, key, value);
                leaf_child_ref(seg, value)
            } else {
                merge_ext_ref(seg, witness_child_ref(list_nth(items, 1, RLP_NONE), key, fuel - 1))
            } /* extension */
        } else {
            if n == 17 then {
                let v = list_nth(items, 16, RLP_NONE).content;
                if list_len(v) > 0 then {
                    k_witness_bad = true
                };
                var mask : int = 0;
                var children : list(list(byte)) = [||];
                foreach (i from 0 to 15) {
                    let cref = witness_child_ref(list_nth(items, i, RLP_NONE), list_append(prefix, [|i|]), fuel - 1);
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
termination_measure witness_node_ref(node, prefix, fuel) = fuel
termination_measure witness_child_ref(f, key, fuel) = fuel

/* re-root the trie at `root` from the witness. The walk also records each
 * account's leaf span into acctdb; storage tries are authenticated and harvested
 * in a second pass driven by acctdb. */
function witness_reroot(root : bits(256)) -> bits(256) = {
    acctdb_reset();
    if root == EMPTY_TRIE_ROOT then {
        EMPTY_TRIE_ROOT
    } else {
        let rootnode = node_db_lookup(root);
        if list_len(rootnode) == 0 then {
            keccak256([|0x80|])
        } else {
            trie_ref_to_root(witness_node_ref(rootnode, [||], 256))
        }
    }
}

/* ---- account / storage resolution against the witness ------------------ */

/* populate the C node-db from witness.state: keccak(node) -> span. The node is
 * materialized once here to hash it; the recorded span (not the bytes) is what
 * the walk reads back, so no per-node Sail copy is retained. */
function build_node_db() -> unit = {
    nodedb_reset();
    let n = wit_n_state();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_witness_node_span(i);
        let h = keccak256(ssz_bytes(off, len));
        nodedb_insert(
            h[255 .. 192],
            h[191 .. 128],
            h[127 .. 64],
            h[63 .. 0],
            get_slice_int(64, off, 0),
            get_slice_int(64, len, 0),
        )
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

/* Stateless storage resolver (declared in host/state.sail): the (a, s) pre-state
 * value from the witness. The slot store holds the present pre-state slots
 * harvested during the witness pass, so a hit is an O(1) lookup. On a MISS the
 * value is resolved (and its presence/absence PROVEN) by a Merkle walk of the
 * account's storage trie from its authenticated pre-state root:
 *   - the walk reaches the leaf            => present, use that value;
 *   - the walk reaches a diverging/empty
 *     terminus through present nodes        => PROVEN ABSENT => 0;
 *   - the walk hits a blinded (witness-
 *     absent) node on the path              => trie_walk asserts "witness missing
 *                                              trie node" => fail closed.
 * The resolved value (or proven-absent 0) is cached in the slot store, so repeat
 * reads are O(1) without repeating the walk. */
function stateless_storage(a : address, s : word) -> word = {
    let ak = keccak256(address_to_bytes(a));
    let sk = keccak256(word_to_bytes32(s));
    if slotdb_sel(
        ak[255 .. 192],
        ak[191 .. 128],
        ak[127 .. 64],
        ak[63 .. 0],
        sk[255 .. 192],
        sk[191 .. 128],
        sk[127 .. 64],
        sk[63 .. 0],
    ) !=
        0x0000000000000000 then {
        slot_selword()
    } else {
        let sroot = load_account(a).storage_root;
        let leaf = trie_walk(node_db_lookup(sroot), hash_nibbles(sk), 0, 256);
        let v : word = if list_len(leaf) > 0 then {
                rlp_str_word(leaf)
            } else {
                sail_zeros(256)
            };
        slot_insert(ak, sk, v); /* cache the proven value / proven-absent 0 */
        v
    }
}

/* Authenticate + harvest one account's storage trie: recursively re-root it from
 * the witness (with hv_storage routing leaves into the slot store under `ak`)
 * and assert the reconstructed root equals the account's committed storage root. */
/* EIP-8025 (optional proofs): the witness omits an account's storage proof when
 * the block never touches its storage. Such an account has a non-empty committed
 * storage_root whose root node is ABSENT from the witness node-db -- there is
 * nothing to harvest or re-root, and the storage is unchanged, so its post-state
 * storage_root stays the committed value. */
function storage_proof_omitted(sroot : bits(256)) -> bool =
    (list_len(node_db_lookup(sroot)) == 0) & (sroot != EMPTY_TRIE_ROOT)

function harvest_storage(ak : bits(256), sroot : bits(256)) -> unit = {
    if storage_proof_omitted(sroot) then {
        ()
    } else {
        hv_storage = true;
        hv_acct = ak;
        let got = if sroot == EMPTY_TRIE_ROOT then {
                EMPTY_TRIE_ROOT
            } else {
                trie_ref_to_root(witness_node_ref(node_db_lookup(sroot), [||] : list(int), 256))
            };
        hv_storage = false;

        /* a storage proof that does not re-root to the account's committed
           storage_root is deficient -> graceful invalid (not an abort). */
        if got != sroot then {
            k_witness_bad = true
        }
    }
}

/* 256-bit keccak(addr) of the current acctdb_at row */
function acctdb_at_acct() -> bits(256) =
    append(
        acctdb_at_key(get_slice_int(64, 3, 0)),
        append(
            acctdb_at_key(get_slice_int(64, 2, 0)),
            append(acctdb_at_key(get_slice_int(64, 1, 0)), acctdb_at_key(get_slice_int(64, 0, 0))),
        ),
    )

/* Storage-harvest pass: for every account leaf recorded in the acctdb during the
 * state re-root, authenticate + harvest its storage trie into the slot store.
 * Runs after witness_reroot, driven by the acctdb (the C-backed cross-pass
 * enumerator -- no Sail leaf list). After this the slot
 * store holds the witnessed pre-state storage leaves, plus a per-account
 * completeness marker for storage tries that preserved blinded children. */
function harvest_all_storage() -> unit = {
    let n = unsigned(acctdb_count());
    foreach (i from 0 to (n - 1)) {
        acctdb_at(get_slice_int(64, i, 0));
        let ak = acctdb_at_acct();
        let node = ssz_bytes(unsigned(acctdb_at_off()), unsigned(acctdb_at_len()));
        let value = list_nth(rlp_node_items(node), 1, RLP_NONE).content;
        let (_, _, sroot, _) = decode_account(value);
        if sroot != EMPTY_TRIE_ROOT then {
            harvest_storage(ak, sroot)
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch. A hit in the acctdb (account leaves recorded during the witness
 * re-root) materializes from the leaf span. On a MISS, presence/absence is PROVEN
 * by a Merkle walk of the state trie from the authenticated pre-state root:
 *   - the walk reaches the leaf             => present, decode + seed;
 *   - reaches a diverging/empty terminus
 *     through present nodes                  => PROVEN ABSENT => empty account;
 *   - hits a blinded (witness-absent) node   => trie_walk asserts => fail closed.
 * So a not-materialized account is never silently treated as empty -- its
 * absence is a witness-proven exclusion. The result is cached raw in the acctmap
 * (k_seed_account_hashed), so subsequent load_account(a) is an O(1) raw read. */
function stateless_account_load(a : address) -> unit = {
    let ak = keccak256(address_to_bytes(a));
    let value : list(byte) = if acctdb_sel(ak[255 .. 192], ak[191 .. 128], ak[127 .. 64], ak[63 .. 0]) !=
            0x0000000000000000 then {
            list_nth(rlp_node_items(ssz_bytes(unsigned(acctdb_sel_off()), unsigned(acctdb_sel_len()))), 1, RLP_NONE).content
        } else {
            trie_walk(node_db_lookup(parent_state_root()), hash_nibbles(ak), 0, 256)
        };
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        k_seed_account_hashed(a, nonce, bal, sroot, chash);
        acctmap_mark_base_exists()
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
            let r = if updates_has_delete(updates) then {
                    if k_stateless then {
                        if storage_harvest_complete(ak) then {
                            updates_root(storage_live_updates_from_harvest(ak, updates))
                        } else {
                            trie_overlay_root(sroot, updates)
                        }
                    } else {
                        trie_overlay_root(sroot, updates)
                    }
                } else {
                    trie_overlay_root_preserve(sroot, updates)
                };
            r
        }
    }
}

function storage_root(a : address) -> bits(256) =
    storage_root_from(keccak256(address_to_bytes(a)), load_account(a).storage_root)

function compute_state_root_updates_only() -> bits(256) =
    updates_root(state_updates_map())

function compute_state_root_overlay() -> bits(256) = {
    let updates = state_updates_map();
    let r = if updates_has_delete(updates) then {
            trie_overlay_root(parent_state_root(), updates)
        } else {
            trie_overlay_root_preserve(parent_state_root(), updates)
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
