/* ======================================================================== */
/* SPEC BACKEND: PURE REFERENCE MODELS (STATE)                              */
/*                                                                          */
/* Pure Sail reference implementations of the C-backed host mechanisms,     */
/* over registers holding materialized data. These give the impure          */
/* interface (sail/iface/) definitional semantics in the spec backend: the  */
/* Coq export gets state-monad definitions instead of unconstrained         */
/* parameters, and the build backend's pointer implementations must refine  */
/* exactly this behavior. The registers exist ONLY in spec builds; the      */
/* build backend keeps its state opaquely in C.                             */
/* ======================================================================== */

/* ---- operand stack (iface/stack.sail; refines ffi/stack.c) --------------
 * A stack of call frames, each a list of words with the head as the top of
 * the stack. Observable laws (matching the C): reset -> one empty frame;
 * enter pushes an empty frame; leave drops the top frame but never the
 * bottom one; peek(n) is the nth-from-top word and reads zero past the
 * height; pop/set out of range are no-ops. The EVM's 1024-word limit is
 * enforced by the interpreter before push, not here. */

register spec_hs_frames : list(list(word)) = [|[||]|]

function hs_reset() -> unit = spec_hs_frames = [|[||]|]

function hs_enter() -> unit = spec_hs_frames = [||] :: spec_hs_frames

function hs_leave() -> unit =
    match spec_hs_frames {
        _ :: (f :: rest) => spec_hs_frames = f :: rest,
        _ => (),
    }

function hs_depth() -> bits(64) =
    match spec_hs_frames {
        f :: _ => get_slice_int(64, list_len(f), 0),
        [||] => get_slice_int(64, 0, 0),
    }

function hs_push_w(w : word) -> unit =
    match spec_hs_frames {
        f :: rest => spec_hs_frames = (w :: f) :: rest,
        [||] => (),
    }

function hs_pop() -> unit =
    match spec_hs_frames {
        (_ :: f) :: rest => spec_hs_frames = f :: rest,
        _ => (),
    }

function hs_peek_w(n : bits(64)) -> word =
    match spec_hs_frames {
        f :: _ => list_nth(f, unsigned(n), sail_zeros(256)),
        [||] => sail_zeros(256),
    }

/* replace element `i` of a list (identity when i is out of range) */
val spec_list_set_word : (list(word), int, word) -> list(word)
function spec_list_set_word(xs, i, w) =
    match xs {
        [||] => [||],
        x :: t => if i == 0 then {
            w :: t
        } else {
            x :: spec_list_set_word(t, i - 1, w)
        },
    }

function hs_set_w(n : bits(64), w : word) -> unit =
    match spec_hs_frames {
        f :: rest => spec_hs_frames = spec_list_set_word(f, unsigned(n), w) :: rest,
        [||] => (),
    }

/* ---- transient storage (iface/state.sail; refines ffi/transient_storage.c)
 * A flat map from (address, slot) to word for id 0 (EIP-1153); absent slots
 * read zero, stores overwrite, reset clears. Ids other than 0 are inert
 * (writes dropped, reads zero). Frame revert is the Sail journal (JTran)
 * replaying inverse writes, so the map itself has no layering. */

struct SpecTransientEntry = { taddr : address, tslot : word, tval : word }

register spec_tstore : list(SpecTransientEntry) = [||]

val spec_tstore_get : (list(SpecTransientEntry), address, word) -> word
function spec_tstore_get(m, a, s) =
    match m {
        [||] => sail_zeros(256),
        e :: t => if (e.taddr == a) & (e.tslot == s) then {
            e.tval
        } else {
            spec_tstore_get(t, a, s)
        },
    }

val spec_tstore_put : (list(SpecTransientEntry), address, word, word) -> list(SpecTransientEntry)
function spec_tstore_put(m, a, s, v) =
    match m {
        [||] => [|struct { taddr = a, tslot = s, tval = v }|],
        e :: t => if (e.taddr == a) & (e.tslot == s) then {
            struct { taddr = a, tslot = s, tval = v } :: t
        } else {
            e :: spec_tstore_put(t, a, s, v)
        },
    }

function transient_storage_reset(id : bits(64)) -> unit =
    if unsigned(id) == 0 then {
        spec_tstore = [||]
    } else {
        ()
    }

function transient_storage_write(id : bits(64), a : address, s : word, v : word) -> unit =
    if unsigned(id) == 0 then {
        spec_tstore = spec_tstore_put(spec_tstore, a, s, v)
    } else {
        ()
    }

function transient_storage_read(id : bits(64), a : address, s : word) -> word =
    if unsigned(id) == 0 then {
        spec_tstore_get(spec_tstore, a, s)
    } else {
        sail_zeros(256)
    }

/* ---- persistent storage map (iface/state.sail; refines ffi/state_db.c) --
 * Rows carry (address, slot, value); the C backend keys them by the secure
 * pair (keccak(address), keccak(slot)) which it derives and memoizes
 * internally. The model keys point operations by the raw (address, slot)
 * pair -- observationally equal to the C's hashed keys up to keccak
 * collisions -- and only computes the secure hashes where they are
 * observable: the iteration order and it_acct_hash.
 *
 * Shape: one CACHE table (authenticated pre-state / fixture seeds) plus a
 * non-empty stack of LAYERED update tables (head = top). store writes the
 * top layer; push adds an empty top; pop_commit overlays the top onto the
 * layer below (top wins) and pop_discard drops it -- both only when at
 * least two layers exist, so the base layer is never popped. load resolves
 * top-down through the layers and then the cache; base_load/base_present
 * consult only the bottom update layer and then the cache (the EIP-2200
 * "original" view); present is any layer or the cache. wipe_addr deletes
 * the address's rows from the cache and every layer.
 *
 * Iteration: count/at enumerate the cache/layer union with upper layers
 * overriding, ascending by (keccak(address), keccak(slot)) compared as
 * unsigned 256-bit integers; update_count/update_at enumerate the merged
 * update overlay only. Both rebuild the row snapshot and reset the cursor
 * to the requested index (count resets it to 0); it_* read the selected
 * row and yield zeros when the selection is out of range. Matching the C,
 * every mutation except push invalidates the snapshot, and at/update_at
 * always see a freshly merged view. */

struct SpecStorageRow = { srow_addr : address, srow_slot : word, srow_val : word }

/* an iteration row: the secure-keyed view of a merged storage row */
struct SpecStorageIterRow = { sit_ah : hash, sit_sh : hash, sit_slot : word, sit_val : word }

register spec_sm_cache : list(SpecStorageRow) = [||]
register spec_sm_layers : list(list(SpecStorageRow)) = [|[||]|]
register spec_sm_iter_rows : list(SpecStorageIterRow) = [||]
register spec_sm_iter_sel : bits(64) = 0x0000000000000000

val spec_srow_lookup : (list(SpecStorageRow), address, word) -> (bool, word)
function spec_srow_lookup(rows, a, s) =
    match rows {
        [||] => (false, sail_zeros(256)),
        e :: t => if (e.srow_addr == a) & (e.srow_slot == s) then {
            (true, e.srow_val)
        } else {
            spec_srow_lookup(t, a, s)
        },
    }

val spec_srow_put : (list(SpecStorageRow), address, word, word) -> list(SpecStorageRow)
function spec_srow_put(rows, a, s, v) =
    match rows {
        [||] => [|struct { srow_addr = a, srow_slot = s, srow_val = v }|],
        e :: t => if (e.srow_addr == a) & (e.srow_slot == s) then {
            struct { srow_addr = a, srow_slot = s, srow_val = v } :: t
        } else {
            e :: spec_srow_put(t, a, s, v)
        },
    }

val spec_srow_wipe : (list(SpecStorageRow), address) -> list(SpecStorageRow)
function spec_srow_wipe(rows, a) =
    match rows {
        [||] => [||],
        e :: t => if e.srow_addr == a then {
            spec_srow_wipe(t, a)
        } else {
            e :: spec_srow_wipe(t, a)
        },
    }

/* walk the update layers top-down */
val spec_sm_layers_lookup : (list(list(SpecStorageRow)), address, word) -> (bool, word)
function spec_sm_layers_lookup(ls, a, s) =
    match ls {
        [||] => (false, sail_zeros(256)),
        l :: t => {
            let (found, v) = spec_srow_lookup(l, a, s);
            if found then {
                (true, v)
            } else {
                spec_sm_layers_lookup(t, a, s)
            }
        },
    }

/* the bottom (base) update layer */
val spec_sm_base_layer : list(list(SpecStorageRow)) -> list(SpecStorageRow)
function spec_sm_base_layer(ls) =
    match ls {
        [||] => [||],
        [|l|] => l,
        _ :: t => spec_sm_base_layer(t),
    }

/* overlay `top` onto `below` (top wins on key clashes) */
val spec_sm_commit_layer : (list(SpecStorageRow), list(SpecStorageRow)) -> list(SpecStorageRow)
function spec_sm_commit_layer(below, top) =
    match top {
        [||] => below,
        e :: t => spec_sm_commit_layer(spec_srow_put(below, e.srow_addr, e.srow_slot, e.srow_val), t),
    }

/* insert into a list sorted ascending by (keccak(addr), keccak(slot)),
 * replacing the row when the secure key is already present */
val spec_sm_sorted_put : (list(SpecStorageIterRow), SpecStorageIterRow) -> list(SpecStorageIterRow)
function spec_sm_sorted_put(rows, r) =
    match rows {
        [||] => [|r|],
        e :: t => if (e.sit_ah == r.sit_ah) & (e.sit_sh == r.sit_sh) then {
            r :: t
        } else {
            if (unsigned(r.sit_ah) < unsigned(e.sit_ah)) |
                ((r.sit_ah == e.sit_ah) & (unsigned(r.sit_sh) < unsigned(e.sit_sh))) then {
                r :: (e :: t)
            } else {
                e :: spec_sm_sorted_put(t, r)
            }
        },
    }

val spec_sm_overlay_rows : (list(SpecStorageIterRow), list(SpecStorageRow)) -> list(SpecStorageIterRow)
function spec_sm_overlay_rows(acc, rows) =
    match rows {
        [||] => acc,
        e :: t => spec_sm_overlay_rows(
            spec_sm_sorted_put(
                acc,
                struct {
                    sit_ah = keccak256_address(e.srow_addr),
                    sit_sh = keccak256_word(e.srow_slot),
                    sit_slot = e.srow_slot,
                    sit_val = e.srow_val,
                },
            ),
            t,
        ),
    }

/* overlay the layers bottom-up (head = top is applied last, so it wins) */
val spec_sm_overlay_layers : (list(SpecStorageIterRow), list(list(SpecStorageRow))) -> list(SpecStorageIterRow)
function spec_sm_overlay_layers(acc, ls) =
    match ls {
        [||] => acc,
        l :: t => spec_sm_overlay_rows(spec_sm_overlay_layers(acc, t), l),
    }

val spec_sm_merged : bool -> list(SpecStorageIterRow)
function spec_sm_merged(updates_only) = {
    let base : list(SpecStorageIterRow) = if updates_only then {
            [||]
        } else {
            spec_sm_overlay_rows([||], spec_sm_cache)
        };
    spec_sm_overlay_layers(base, spec_sm_layers)
}

val spec_sm_iter_clear : unit -> unit
function spec_sm_iter_clear() = {
    spec_sm_iter_rows = [||];
    spec_sm_iter_sel = 0x0000000000000000
}

function storage_map_reset() -> unit = {
    spec_sm_cache = [||];
    spec_sm_layers = [|[||]|];
    spec_sm_iter_clear()
}

function storage_map_push() -> unit = spec_sm_layers = [||] :: spec_sm_layers

function storage_map_pop_commit() -> unit = {
    match spec_sm_layers {
        top :: (below :: rest) => spec_sm_layers = spec_sm_commit_layer(below, top) :: rest,
        _ => (),
    };
    spec_sm_iter_clear()
}

function storage_map_pop_discard() -> unit = {
    match spec_sm_layers {
        _ :: (below :: rest) => spec_sm_layers = below :: rest,
        _ => (),
    };
    spec_sm_iter_clear()
}

function storage_map_seed(a : address, s : word, v : word) -> unit = {
    spec_sm_cache = spec_srow_put(spec_sm_cache, a, s, v);
    spec_sm_iter_clear()
}

function storage_map_store(a : address, s : word, v : word) -> unit = {
    match spec_sm_layers {
        top :: rest => spec_sm_layers = spec_srow_put(top, a, s, v) :: rest,
        [||] => spec_sm_layers = [|spec_srow_put([||], a, s, v)|],
    };
    spec_sm_iter_clear()
}

function storage_map_load(a : address, s : word) -> word = {
    let (found, v) = spec_sm_layers_lookup(spec_sm_layers, a, s);
    if found then {
        v
    } else {
        let (_, cv) = spec_srow_lookup(spec_sm_cache, a, s);
        cv
    }
}

function storage_map_base_load(a : address, s : word) -> word = {
    let (found, v) = spec_srow_lookup(spec_sm_base_layer(spec_sm_layers), a, s);
    if found then {
        v
    } else {
        let (_, cv) = spec_srow_lookup(spec_sm_cache, a, s);
        cv
    }
}

function storage_map_present(a : address, s : word) -> bool = {
    let (found, _) = spec_sm_layers_lookup(spec_sm_layers, a, s);
    if found then {
        true
    } else {
        let (cfound, _) = spec_srow_lookup(spec_sm_cache, a, s);
        cfound
    }
}

function storage_map_base_present(a : address, s : word) -> bool = {
    let (found, _) = spec_srow_lookup(spec_sm_base_layer(spec_sm_layers), a, s);
    if found then {
        true
    } else {
        let (cfound, _) = spec_srow_lookup(spec_sm_cache, a, s);
        cfound
    }
}

val spec_sm_wipe_layers : (list(list(SpecStorageRow)), address) -> list(list(SpecStorageRow))
function spec_sm_wipe_layers(ls, a) =
    match ls {
        [||] => [||],
        l :: t => spec_srow_wipe(l, a) :: spec_sm_wipe_layers(t, a),
    }

function storage_map_wipe_addr(a : address) -> unit = {
    spec_sm_cache = spec_srow_wipe(spec_sm_cache, a);
    spec_sm_layers = spec_sm_wipe_layers(spec_sm_layers, a);
    spec_sm_iter_clear()
}

function storage_map_count() -> bits(64) = {
    spec_sm_iter_rows = spec_sm_merged(false);
    spec_sm_iter_sel = 0x0000000000000000;
    get_slice_int(64, list_len(spec_sm_iter_rows), 0)
}

function storage_map_update_count() -> bits(64) = {
    spec_sm_iter_rows = spec_sm_merged(true);
    spec_sm_iter_sel = 0x0000000000000000;
    get_slice_int(64, list_len(spec_sm_iter_rows), 0)
}

function storage_map_at(j : bits(64)) -> unit = {
    spec_sm_iter_rows = spec_sm_merged(false);
    spec_sm_iter_sel = j
}

function storage_map_update_at(j : bits(64)) -> unit = {
    spec_sm_iter_rows = spec_sm_merged(true);
    spec_sm_iter_sel = j
}

/* the selected iteration row; all-zero fields when out of range */
val spec_sm_iter_row : unit -> SpecStorageIterRow
function spec_sm_iter_row() =
    list_nth(
        spec_sm_iter_rows,
        unsigned(spec_sm_iter_sel),
        struct {
            sit_ah = sail_zeros(256),
            sit_sh = sail_zeros(256),
            sit_slot = sail_zeros(256),
            sit_val = sail_zeros(256),
        },
    )

function storage_map_it_acct_hash() -> hash = spec_sm_iter_row().sit_ah

function storage_map_it_slot() -> word = spec_sm_iter_row().sit_slot

function storage_map_it_val() -> word = spec_sm_iter_row().sit_val

/* ---- account map (iface/state.sail; refines ffi/state_db.c) -------------
 * The kernel account working set: TWO tables of account rows -- CACHE
 * (authenticated pre-state / fixture seeds, written by acctmap_seed) and
 * UPDATES (execution writes, written by acctmap_store). The C backend keys
 * both by hkey = keccak(address), derived and memoized internally; the
 * model keys them by the raw address (equal up to keccak collisions) and
 * computes hkey only where observable (iteration order, at_hkey).
 *
 * Laws (matching the C): point reads resolve updates first, cache second,
 * and yield zeros when absent. seed overwrites the cache row and CLEARS its
 * base_exists flag; store overwrites the update row, whose base_exists is
 * the existing update row's flag if set, else inherited from the cache row.
 * mark_base_exists sets the flag on the cache row -- mirrored onto an
 * existing update row -- ONLY when a cache row exists. remove deletes the
 * address from both tables. Iteration: count/at enumerate the cache/update
 * union ascending by hkey with update rows overriding; update_count/
 * update_at enumerate the update table alone in the same order. at/
 * update_at cache a cursor row read by the at_* accessors; an out-of-range
 * index and every mutation (except count, which merely snapshots)
 * invalidate the cursor, making at_* read zeros/false. */

struct SpecAccountRow = {
    arow_addr : address,
    arow_nonce : bits(64),
    arow_bal : word,
    arow_sroot : hash,
    arow_chash : hash,
    arow_base : bool,
}

/* the iteration cursor row: the secure-keyed view of a merged account row */
struct SpecAccountIterRow = {
    ait_hkey : hash,
    ait_nonce : bits(64),
    ait_bal : word,
    ait_sroot : hash,
    ait_chash : hash,
    ait_base : bool,
}

register spec_am_cache : list(SpecAccountRow) = [||]
register spec_am_updates : list(SpecAccountRow) = [||]

/* the cached iteration row: empty = invalid cursor (at_* read zeros) */
register spec_am_iter : list(SpecAccountIterRow) = [||]

val spec_arow_find : (list(SpecAccountRow), address) -> (bool, SpecAccountRow)
function spec_arow_find(rows, a) =
    match rows {
        [||] => (
            false,
            struct {
                arow_addr = a,
                arow_nonce = 0x0000000000000000,
                arow_bal = sail_zeros(256),
                arow_sroot = sail_zeros(256),
                arow_chash = sail_zeros(256),
                arow_base = false,
            },
        ),
        e :: t => if e.arow_addr == a then {
            (true, e)
        } else {
            spec_arow_find(t, a)
        },
    }

val spec_arow_put : (list(SpecAccountRow), SpecAccountRow) -> list(SpecAccountRow)
function spec_arow_put(rows, r) =
    match rows {
        [||] => [|r|],
        e :: t => if e.arow_addr == r.arow_addr then {
            r :: t
        } else {
            e :: spec_arow_put(t, r)
        },
    }

val spec_arow_remove : (list(SpecAccountRow), address) -> list(SpecAccountRow)
function spec_arow_remove(rows, a) =
    match rows {
        [||] => [||],
        e :: t => if e.arow_addr == a then {
            t
        } else {
            e :: spec_arow_remove(t, a)
        },
    }

function acctmap_reset() -> unit = {
    spec_am_cache = [||];
    spec_am_updates = [||];
    spec_am_iter = [||]
}

/* the row bound to `a` (updates override cache rows) */
val spec_am_row_of : address -> (bool, SpecAccountRow)
function spec_am_row_of(a) = {
    let (ufound, u) = spec_arow_find(spec_am_updates, a);
    if ufound then {
        (true, u)
    } else {
        spec_arow_find(spec_am_cache, a)
    }
}

function acctmap_present(a : address) -> bool = {
    let (found, _) = spec_am_row_of(a);
    found
}

function acctmap_seed(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit = {
    spec_am_cache = spec_arow_put(
        spec_am_cache,
        struct {
            arow_addr = a,
            arow_nonce = nonce,
            arow_bal = bal,
            arow_sroot = sroot,
            arow_chash = chash,
            arow_base = false,
        },
    );
    spec_am_iter = [||]
}

function acctmap_store(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit = {
    let (ufound, u) = spec_arow_find(spec_am_updates, a);
    let (cfound, c) = spec_arow_find(spec_am_cache, a);
    let base : bool = if ufound & u.arow_base then {
            true
        } else {
            if cfound then {
                c.arow_base
            } else {
                false
            }
        };
    spec_am_updates = spec_arow_put(
        spec_am_updates,
        struct {
            arow_addr = a,
            arow_nonce = nonce,
            arow_bal = bal,
            arow_sroot = sroot,
            arow_chash = chash,
            arow_base = base,
        },
    );
    spec_am_iter = [||]
}

function acctmap_mark_base_exists(a : address) -> unit = {
    let (cfound, c) = spec_arow_find(spec_am_cache, a);
    if cfound then {
        spec_am_cache = spec_arow_put(spec_am_cache, { c with arow_base = true });
        let (ufound, u) = spec_arow_find(spec_am_updates, a);
        if ufound then {
            spec_am_updates = spec_arow_put(spec_am_updates, { u with arow_base = true })
        };
        spec_am_iter = [||]
    }
}

function acctmap_nonce(a : address) -> bits(64) = {
    let (_, r) = spec_am_row_of(a);
    r.arow_nonce
}

function acctmap_bal(a : address) -> word = {
    let (found, r) = spec_am_row_of(a);
    if found then {
        r.arow_bal
    } else {
        sail_zeros(256)
    }
}

function acctmap_sroot(a : address) -> hash = {
    let (found, r) = spec_am_row_of(a);
    if found then {
        r.arow_sroot
    } else {
        sail_zeros(256)
    }
}

function acctmap_chash(a : address) -> hash = {
    let (found, r) = spec_am_row_of(a);
    if found then {
        r.arow_chash
    } else {
        sail_zeros(256)
    }
}

function acctmap_remove(a : address) -> unit = {
    spec_am_cache = spec_arow_remove(spec_am_cache, a);
    spec_am_updates = spec_arow_remove(spec_am_updates, a);
    spec_am_iter = [||]
}

/* insert into a list sorted ascending by hkey, replacing on an equal key */
val spec_am_sorted_put : (list(SpecAccountIterRow), SpecAccountIterRow) -> list(SpecAccountIterRow)
function spec_am_sorted_put(rows, r) =
    match rows {
        [||] => [|r|],
        e :: t => if e.ait_hkey == r.ait_hkey then {
            r :: t
        } else {
            if unsigned(r.ait_hkey) < unsigned(e.ait_hkey) then {
                r :: (e :: t)
            } else {
                e :: spec_am_sorted_put(t, r)
            }
        },
    }

val spec_am_hashed_row : SpecAccountRow -> SpecAccountIterRow
function spec_am_hashed_row(r) =
    struct {
        ait_hkey = keccak256_address(r.arow_addr),
        ait_nonce = r.arow_nonce,
        ait_bal = r.arow_bal,
        ait_sroot = r.arow_sroot,
        ait_chash = r.arow_chash,
        ait_base = r.arow_base,
    }

val spec_am_overlay_rows : (list(SpecAccountIterRow), list(SpecAccountRow)) -> list(SpecAccountIterRow)
function spec_am_overlay_rows(acc, rows) =
    match rows {
        [||] => acc,
        e :: t => spec_am_overlay_rows(spec_am_sorted_put(acc, spec_am_hashed_row(e)), t),
    }

/* the cache/update union sorted by hkey, update rows overriding */
val spec_am_order : unit -> list(SpecAccountIterRow)
function spec_am_order() = spec_am_overlay_rows(spec_am_overlay_rows([||], spec_am_cache), spec_am_updates)

/* the update table alone, sorted by hkey */
val spec_am_updates_order : unit -> list(SpecAccountIterRow)
function spec_am_updates_order() = spec_am_overlay_rows([||], spec_am_updates)

function acctmap_count() -> bits(64) = get_slice_int(64, list_len(spec_am_order()), 0)

function acctmap_update_count() -> bits(64) = get_slice_int(64, list_len(spec_am_updates), 0)

val spec_am_select : (list(SpecAccountIterRow), bits(64)) -> unit
function spec_am_select(rows, idx) = {
    let i = unsigned(idx);
    if i < list_len(rows) then {
        spec_am_iter =
            [|
                list_nth(
                    rows,
                    i,
                    struct {
                        ait_hkey = sail_zeros(256),
                        ait_nonce = 0x0000000000000000,
                        ait_bal = sail_zeros(256),
                        ait_sroot = sail_zeros(256),
                        ait_chash = sail_zeros(256),
                        ait_base = false,
                    },
                ),
            |]
    } else {
        spec_am_iter = [||]
    }
}

function acctmap_at(idx : bits(64)) -> unit = spec_am_select(spec_am_order(), idx)

function acctmap_update_at(idx : bits(64)) -> unit = spec_am_select(spec_am_updates_order(), idx)

/* the cursor row; all-zero fields when the cursor is invalid */
val spec_am_iter_row : unit -> SpecAccountIterRow
function spec_am_iter_row() =
    match spec_am_iter {
        r :: _ => r,
        [||] => struct {
            ait_hkey = sail_zeros(256),
            ait_nonce = 0x0000000000000000,
            ait_bal = sail_zeros(256),
            ait_sroot = sail_zeros(256),
            ait_chash = sail_zeros(256),
            ait_base = false,
        },
    }

function acctmap_at_hkey() -> hash = spec_am_iter_row().ait_hkey

function acctmap_at_nonce() -> bits(64) = spec_am_iter_row().ait_nonce

function acctmap_at_bal() -> word = spec_am_iter_row().ait_bal

function acctmap_at_sroot() -> hash = spec_am_iter_row().ait_sroot

function acctmap_at_chash() -> hash = spec_am_iter_row().ait_chash

function acctmap_at_base_exists() -> bool = spec_am_iter_row().ait_base

/* ---- code content store (iface/code_db.sail; refines ffi/code_db.c) -----
 * The content-addressed contract-code store: codeHash -> code bytes.
 * Streaming writes go through a cursor: code_db_stream_code_begin(h)
 * selects the entry for h and returns 0 iff h is already stored with a
 * non-empty byte string (same hash = same bytes, so re-streaming is
 * skipped); otherwise it (re)initializes the entry to empty and returns 1,
 * and each code_db_stream_code_byte appends one byte to the selected
 * entry. Matching the C, the streamed bytes are visible immediately (the
 * entry lives in the table while it grows), the cursor stays on the entry
 * selected by the last begin, and a stream byte before any begin is a
 * no-op. stored_code_length reads 0 for absent hashes. read_delegation
 * probes EIP-7702 designations: 0x01 @ target when the stored code is
 * exactly ef 01 00 ++ target (23 bytes), else zero. The JUMPDEST bitmap
 * the C precomputes per entry is derived state; the model recomputes
 * validity from the bytes where it is observed (the frame-code hooks in
 * sail/spec/model_buffers.sail). */

struct SpecCodeEntry = { ce_hash : hash, ce_code : list(byte) }

register spec_code_db : list(SpecCodeEntry) = [||]

/* the streaming cursor: hash selected by the last stream_code_begin */
register spec_code_cur : list(hash) = [||]

val spec_code_find : (list(SpecCodeEntry), hash) -> (bool, list(byte))
function spec_code_find(es, h) =
    match es {
        [||] => (false, [||]),
        e :: t => if e.ce_hash == h then {
            (true, e.ce_code)
        } else {
            spec_code_find(t, h)
        },
    }

val spec_code_put : (list(SpecCodeEntry), hash, list(byte)) -> list(SpecCodeEntry)
function spec_code_put(es, h, c) =
    match es {
        [||] => [|struct { ce_hash = h, ce_code = c }|],
        e :: t => if e.ce_hash == h then {
            struct { ce_hash = h, ce_code = c } :: t
        } else {
            e :: spec_code_put(t, h, c)
        },
    }

function code_db_stream_code_begin(h : hash) -> bits(64) = {
    spec_code_cur = [|h|];
    let (found, c) = spec_code_find(spec_code_db, h);
    if found & (list_len(c) > 0) then {
        0x0000000000000000
    } else {
        spec_code_db = spec_code_put(spec_code_db, h, [||]);
        0x0000000000000001
    }
}

function code_db_stream_code_byte(b : byte) -> unit =
    match spec_code_cur {
        h :: _ => {
            let (found, c) = spec_code_find(spec_code_db, h);
            if found then {
                spec_code_db = spec_code_put(spec_code_db, h, list_append(c, [|b|]))
            }
        },
        [||] => (),
    }

function code_db_stored_code_length(h : hash) -> bits(64) = {
    let (_, c) = spec_code_find(spec_code_db, h);
    get_slice_int(64, list_len(c), 0)
}

function code_db_read_delegation(h : hash) -> bits(168) = {
    let (found, c) = spec_code_find(spec_code_db, h);
    if found & (list_len(c) == 23) & (bytes_get(c, 0) == 0xef) & (bytes_get(c, 1) == 0x01) & (bytes_get(c, 2) == 0x00) then {
        append(0x01, word_to_address(bytes_be256(bytes_slice(c, 3, 20))))
    } else {
        sail_zeros(168)
    }
}

/* ---- witness databases (iface/witness_db.sail; refines ffi/trie_node_db.c)
 * Three stores harvested from the SSZ stateless witness, plus the
 * incomplete-storage set:
 *
 *   nodedb : keccak(node) -> (off, len) span into the witness buffer
 *   acctdb : keccak(addr) -> (off, len) span of the account's leaf node
 *   slotdb : (keccak(addr), keccak(slot)) -> (value, existed)
 *
 * The span stores are FIRST-insert-wins; slotdb inserts overwrite and
 * recompute existed = (value != 0). Each sel is a point lookup that caches
 * the matched row for its accessors; on a miss the model clears the cursor
 * so the accessors read zeros, where the C leaves the previous hit's
 * values -- every Sail caller either guards on the sel result
 * (lib/mpt.sail) or selects a key it just inserted (the acctdb_insert site
 * at lib/mpt.sail:1460), so the two are observationally equal; new callers
 * must preserve that precondition. Iteration (acctdb/slotdb count + at +
 * at_*) presents each row exactly once; the model uses insertion order
 * while the C uses its hash-table order, so callers must not depend on the
 * order, only on the row multiset. An out-of-range at clears the cursor
 * (in the C it keeps stale row data; callers stay in range).
 * slotdb_reset also clears the incomplete-storage set; the nodedb/acctdb
 * resets do not touch it. */

struct SpecSpanEntry = { se_key : hash, se_off : bits(64), se_len : bits(64) }

struct SpecSlotEntry = { sle_acct : hash, sle_slot : hash, sle_val : word, sle_existed : bool }

register spec_ndb : list(SpecSpanEntry) = [||]
register spec_ndb_sel : list(SpecSpanEntry) = [||]

register spec_adb : list(SpecSpanEntry) = [||]
register spec_adb_sel : list(SpecSpanEntry) = [||]
register spec_adb_iter : list(SpecSpanEntry) = [||]

register spec_sdb : list(SpecSlotEntry) = [||]
register spec_sdb_sel : list(SpecSlotEntry) = [||]
register spec_sdb_iter : list(SpecSlotEntry) = [||]
register spec_sdb_incomplete : list(hash) = [||]

/* first-insert-wins append (keeps insertion order) */
val spec_span_insert : (list(SpecSpanEntry), hash, bits(64), bits(64)) -> list(SpecSpanEntry)
function spec_span_insert(es, h, off, len) =
    match es {
        [||] => [|struct { se_key = h, se_off = off, se_len = len }|],
        e :: t => if e.se_key == h then {
            e :: t
        } else {
            e :: spec_span_insert(t, h, off, len)
        },
    }

val spec_span_find : (list(SpecSpanEntry), hash) -> list(SpecSpanEntry)
function spec_span_find(es, h) =
    match es {
        [||] => [||],
        e :: t => if e.se_key == h then {
            [|e|]
        } else {
            spec_span_find(t, h)
        },
    }

val spec_span_off : list(SpecSpanEntry) -> bits(64)
function spec_span_off(sel) =
    match sel {
        e :: _ => e.se_off,
        [||] => 0x0000000000000000,
    }

val spec_span_len : list(SpecSpanEntry) -> bits(64)
function spec_span_len(sel) =
    match sel {
        e :: _ => e.se_len,
        [||] => 0x0000000000000000,
    }

function nodedb_reset() -> unit = {
    spec_ndb = [||];
    spec_ndb_sel = [||]
}

function nodedb_insert(h : hash, off : bits(64), len : bits(64)) -> unit =
    spec_ndb = spec_span_insert(spec_ndb, h, off, len)

function nodedb_sel(h : hash) -> bool = {
    spec_ndb_sel = spec_span_find(spec_ndb, h);
    match spec_ndb_sel {
        [||] => false,
        _ => true,
    }
}

function nodedb_sel_off() -> bits(64) = spec_span_off(spec_ndb_sel)

function nodedb_sel_len() -> bits(64) = spec_span_len(spec_ndb_sel)

function acctdb_reset() -> unit = {
    spec_adb = [||];
    spec_adb_sel = [||];
    spec_adb_iter = [||]
}

function acctdb_insert(h : hash, off : bits(64), len : bits(64)) -> unit =
    spec_adb = spec_span_insert(spec_adb, h, off, len)

function acctdb_sel(h : hash) -> bool = {
    spec_adb_sel = spec_span_find(spec_adb, h);
    match spec_adb_sel {
        [||] => false,
        _ => true,
    }
}

function acctdb_sel_off() -> bits(64) = spec_span_off(spec_adb_sel)

function acctdb_sel_len() -> bits(64) = spec_span_len(spec_adb_sel)

function acctdb_count() -> bits(64) = get_slice_int(64, list_len(spec_adb), 0)

function acctdb_at(idx : bits(64)) -> unit =
    if unsigned(idx) < list_len(spec_adb) then {
        spec_adb_iter =
            [|
                list_nth(
                    spec_adb,
                    unsigned(idx),
                    struct { se_key = sail_zeros(256), se_off = 0x0000000000000000, se_len = 0x0000000000000000 },
                ),
            |]
    } else {
        spec_adb_iter = [||]
    }

function acctdb_at_key() -> hash =
    match spec_adb_iter {
        e :: _ => e.se_key,
        [||] => sail_zeros(256),
    }

function acctdb_at_off() -> bits(64) = spec_span_off(spec_adb_iter)

function acctdb_at_len() -> bits(64) = spec_span_len(spec_adb_iter)

/* overwrite-on-insert, existed recomputed from the new value */
val spec_slot_insert : (list(SpecSlotEntry), hash, hash, word) -> list(SpecSlotEntry)
function spec_slot_insert(es, a, s, v) =
    match es {
        [||] => [|struct { sle_acct = a, sle_slot = s, sle_val = v, sle_existed = v != sail_zeros(256) }|],
        e :: t => if (e.sle_acct == a) & (e.sle_slot == s) then {
            struct { sle_acct = a, sle_slot = s, sle_val = v, sle_existed = v != sail_zeros(256) } :: t
        } else {
            e :: spec_slot_insert(t, a, s, v)
        },
    }

val spec_slot_find : (list(SpecSlotEntry), hash, hash) -> list(SpecSlotEntry)
function spec_slot_find(es, a, s) =
    match es {
        [||] => [||],
        e :: t => if (e.sle_acct == a) & (e.sle_slot == s) then {
            [|e|]
        } else {
            spec_slot_find(t, a, s)
        },
    }

val spec_hash_member : (list(hash), hash) -> bool
function spec_hash_member(hs, h) =
    match hs {
        [||] => false,
        x :: t => if x == h then {
            true
        } else {
            spec_hash_member(t, h)
        },
    }

function slotdb_reset() -> unit = {
    spec_sdb = [||];
    spec_sdb_sel = [||];
    spec_sdb_iter = [||];
    spec_sdb_incomplete = [||]
}

function slotdb_insert(a : hash, s : hash, v : word) -> unit = spec_sdb = spec_slot_insert(spec_sdb, a, s, v)

function slotdb_sel(a : hash, s : hash) -> bool = {
    spec_sdb_sel = spec_slot_find(spec_sdb, a, s);
    match spec_sdb_sel {
        [||] => false,
        _ => true,
    }
}

function slotdb_selval() -> word =
    match spec_sdb_sel {
        e :: _ => e.sle_val,
        [||] => sail_zeros(256),
    }

function slotdb_sel_existed() -> bool =
    match spec_sdb_sel {
        e :: _ => e.sle_existed,
        [||] => false,
    }

function slotdb_count() -> bits(64) = get_slice_int(64, list_len(spec_sdb), 0)

function slotdb_at(idx : bits(64)) -> unit =
    if unsigned(idx) < list_len(spec_sdb) then {
        spec_sdb_iter =
            [|
                list_nth(
                    spec_sdb,
                    unsigned(idx),
                    struct {
                        sle_acct = sail_zeros(256),
                        sle_slot = sail_zeros(256),
                        sle_val = sail_zeros(256),
                        sle_existed = false,
                    },
                ),
            |]
    } else {
        spec_sdb_iter = [||]
    }

function slotdb_at_acct() -> hash =
    match spec_sdb_iter {
        e :: _ => e.sle_acct,
        [||] => sail_zeros(256),
    }

function slotdb_at_slot() -> hash =
    match spec_sdb_iter {
        e :: _ => e.sle_slot,
        [||] => sail_zeros(256),
    }

function slotdb_at_val() -> word =
    match spec_sdb_iter {
        e :: _ => e.sle_val,
        [||] => sail_zeros(256),
    }

function slotdb_at_existed() -> bool =
    match spec_sdb_iter {
        e :: _ => e.sle_existed,
        [||] => false,
    }

function storage_mark_incomplete(a : hash) -> unit =
    if spec_hash_member(spec_sdb_incomplete, a) then {
        ()
    } else {
        spec_sdb_incomplete = a :: spec_sdb_incomplete
    }

function storage_harvest_complete_raw(a : hash) -> bool = not_bool(spec_hash_member(spec_sdb_incomplete, a))
