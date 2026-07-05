/* Build backend: C FFI bindings for the witness storage-slot, trie-node, and
 * account-leaf stores (ffi/trie_node_db.c). */

val slotdb_reset_ffi = impure { _: "slotdb_reset" } : unit -> unit
function slotdb_reset() -> unit = slotdb_reset_ffi()

val slotdb_insert_ffi = impure { _: "slotdb_insert" } : (
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
function slotdb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
    x8 : bits(64),
    x9 : bits(64),
    x10 : bits(64),
    x11 : bits(64),
) -> (
    unit
) =
    slotdb_insert_ffi(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)

val slotdb_sel_ffi = impure { _: "slotdb_sel" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool
function slotdb_sel(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
    x6 : bits(64),
    x7 : bits(64),
) -> (
    bool
) =
    slotdb_sel_ffi(x0, x1, x2, x3, x4, x5, x6, x7)

val slotdb_selval_ffi = impure { _: "slotdb_selval" } : bits(64) -> bits(64)
function slotdb_selval(x0 : bits(64)) -> bits(64) = slotdb_selval_ffi(x0)

val slotdb_sel_existed_ffi = impure { _: "slotdb_sel_existed" } : unit -> bool
function slotdb_sel_existed() -> bool = slotdb_sel_existed_ffi()

val slotdb_count_ffi = impure { _: "slotdb_count" } : unit -> bits(64)
function slotdb_count() -> bits(64) = slotdb_count_ffi()

val slotdb_at_ffi = impure { _: "slotdb_at" } : bits(64) -> unit
function slotdb_at(x0 : bits(64)) -> unit = slotdb_at_ffi(x0)

val slotdb_at_acct_ffi = impure { _: "slotdb_at_acct" } : bits(64) -> bits(64)
function slotdb_at_acct(x0 : bits(64)) -> bits(64) = slotdb_at_acct_ffi(x0)

val slotdb_at_slot_ffi = impure { _: "slotdb_at_slot" } : bits(64) -> bits(64)
function slotdb_at_slot(x0 : bits(64)) -> bits(64) = slotdb_at_slot_ffi(x0)

val slotdb_at_val_ffi = impure { _: "slotdb_at_val" } : bits(64) -> bits(64)
function slotdb_at_val(x0 : bits(64)) -> bits(64) = slotdb_at_val_ffi(x0)

val slotdb_at_existed_ffi = impure { _: "slotdb_at_existed" } : unit -> bool
function slotdb_at_existed() -> bool = slotdb_at_existed_ffi()

val storage_mark_incomplete_ffi = impure { _: "storage_mark_incomplete" } : (bits(64), bits(64), bits(64), bits(64)) -> unit
function storage_mark_incomplete(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> unit = storage_mark_incomplete_ffi(
    x0,
    x1,
    x2,
    x3,
)

val storage_harvest_complete_raw_ffi = impure { _: "storage_harvest_complete" } : (
        bits(64),
        bits(64),
        bits(64),
        bits(64),
    ) -> bool
function storage_harvest_complete_raw(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = storage_harvest_complete_raw_ffi(
    x0,
    x1,
    x2,
    x3,
)

val nodedb_reset_ffi = impure { _: "nodedb_reset" } : unit -> unit
function nodedb_reset() -> unit = nodedb_reset_ffi()

val nodedb_insert_ffi = impure { _: "nodedb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
function nodedb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
) -> (
    unit
) =
    nodedb_insert_ffi(x0, x1, x2, x3, x4, x5)

val nodedb_sel_ffi = impure { _: "nodedb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bool
function nodedb_sel(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = nodedb_sel_ffi(x0, x1, x2, x3)

val nodedb_sel_off_ffi = impure { _: "nodedb_sel_off" } : unit -> bits(64)
function nodedb_sel_off() -> bits(64) = nodedb_sel_off_ffi()

val nodedb_sel_len_ffi = impure { _: "nodedb_sel_len" } : unit -> bits(64)
function nodedb_sel_len() -> bits(64) = nodedb_sel_len_ffi()

val acctdb_reset_ffi = impure { _: "acctdb_reset" } : unit -> unit
function acctdb_reset() -> unit = acctdb_reset_ffi()

val acctdb_insert_ffi = impure { _: "acctdb_insert" } : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
function acctdb_insert(
    x0 : bits(64),
    x1 : bits(64),
    x2 : bits(64),
    x3 : bits(64),
    x4 : bits(64),
    x5 : bits(64),
) -> (
    unit
) =
    acctdb_insert_ffi(x0, x1, x2, x3, x4, x5)

val acctdb_sel_ffi = impure { _: "acctdb_sel" } : (bits(64), bits(64), bits(64), bits(64)) -> bool
function acctdb_sel(x0 : bits(64), x1 : bits(64), x2 : bits(64), x3 : bits(64)) -> bool = acctdb_sel_ffi(x0, x1, x2, x3)

val acctdb_sel_off_ffi = impure { _: "acctdb_sel_off" } : unit -> bits(64)
function acctdb_sel_off() -> bits(64) = acctdb_sel_off_ffi()

val acctdb_sel_len_ffi = impure { _: "acctdb_sel_len" } : unit -> bits(64)
function acctdb_sel_len() -> bits(64) = acctdb_sel_len_ffi()

val acctdb_count_ffi = impure { _: "acctdb_count" } : unit -> bits(64)
function acctdb_count() -> bits(64) = acctdb_count_ffi()

val acctdb_at_ffi = impure { _: "acctdb_at" } : bits(64) -> unit
function acctdb_at(x0 : bits(64)) -> unit = acctdb_at_ffi(x0)

val acctdb_at_key_ffi = impure { _: "acctdb_at_key" } : bits(64) -> bits(64)
function acctdb_at_key(x0 : bits(64)) -> bits(64) = acctdb_at_key_ffi(x0)

val acctdb_at_off_ffi = impure { _: "acctdb_at_off" } : unit -> bits(64)
function acctdb_at_off() -> bits(64) = acctdb_at_off_ffi()

val acctdb_at_len_ffi = impure { _: "acctdb_at_len" } : unit -> bits(64)
function acctdb_at_len() -> bits(64) = acctdb_at_len_ffi()
