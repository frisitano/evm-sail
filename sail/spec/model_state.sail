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
