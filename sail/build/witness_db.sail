/* Build backend: C FFI bindings for the witness storage-slot, trie-node, and
 * account-leaf stores (ffi/trie_node_db.c). */

val slotdb_reset_ffi = impure { _: "slotdb_reset" } : unit -> unit
function slotdb_reset() -> unit = slotdb_reset_ffi()

val slotdb_insert_ffi = impure { _: "slotdb_insert" } : (hash, hash, word) -> unit
function slotdb_insert(acct : hash, slot : hash, v : word) -> unit = slotdb_insert_ffi(acct, slot, v)

val slotdb_sel_ffi = impure { _: "slotdb_sel" } : (hash, hash) -> bool
function slotdb_sel(acct : hash, slot : hash) -> bool = slotdb_sel_ffi(acct, slot)

val slotdb_selval_ffi = impure { _: "slotdb_selval" } : unit -> word
function slotdb_selval() -> word = slotdb_selval_ffi()

val slotdb_sel_existed_ffi = impure { _: "slotdb_sel_existed" } : unit -> bool
function slotdb_sel_existed() -> bool = slotdb_sel_existed_ffi()

val slotdb_count_ffi = impure { _: "slotdb_count" } : unit -> bits(64)
function slotdb_count() -> bits(64) = slotdb_count_ffi()

val slotdb_at_ffi = impure { _: "slotdb_at" } : bits(64) -> unit
function slotdb_at(x0 : bits(64)) -> unit = slotdb_at_ffi(x0)

val slotdb_at_acct_ffi = impure { _: "slotdb_at_acct" } : unit -> hash
function slotdb_at_acct() -> hash = slotdb_at_acct_ffi()

val slotdb_at_slot_ffi = impure { _: "slotdb_at_slot" } : unit -> hash
function slotdb_at_slot() -> hash = slotdb_at_slot_ffi()

val slotdb_at_val_ffi = impure { _: "slotdb_at_val" } : unit -> word
function slotdb_at_val() -> word = slotdb_at_val_ffi()

val slotdb_at_existed_ffi = impure { _: "slotdb_at_existed" } : unit -> bool
function slotdb_at_existed() -> bool = slotdb_at_existed_ffi()

val storage_mark_incomplete_ffi = impure { _: "storage_mark_incomplete" } : hash -> unit
function storage_mark_incomplete(a : hash) -> unit = storage_mark_incomplete_ffi(a)

val storage_harvest_complete_raw_ffi = impure { _: "storage_harvest_complete" } : hash -> bool
function storage_harvest_complete_raw(a : hash) -> bool = storage_harvest_complete_raw_ffi(a)

val nodedb_reset_ffi = impure { _: "nodedb_reset" } : unit -> unit
function nodedb_reset() -> unit = nodedb_reset_ffi()

val nodedb_insert_ffi = impure { _: "nodedb_insert" } : (hash, bits(64), bits(64)) -> unit
function nodedb_insert(h : hash, off : bits(64), len : bits(64)) -> unit = nodedb_insert_ffi(h, off, len)

val nodedb_sel_ffi = impure { _: "nodedb_sel" } : hash -> bool
function nodedb_sel(h : hash) -> bool = nodedb_sel_ffi(h)

val nodedb_sel_off_ffi = impure { _: "nodedb_sel_off" } : unit -> bits(64)
function nodedb_sel_off() -> bits(64) = nodedb_sel_off_ffi()

val nodedb_sel_len_ffi = impure { _: "nodedb_sel_len" } : unit -> bits(64)
function nodedb_sel_len() -> bits(64) = nodedb_sel_len_ffi()

val acctdb_reset_ffi = impure { _: "acctdb_reset" } : unit -> unit
function acctdb_reset() -> unit = acctdb_reset_ffi()

val acctdb_insert_ffi = impure { _: "acctdb_insert" } : (hash, bits(64), bits(64)) -> unit
function acctdb_insert(h : hash, off : bits(64), len : bits(64)) -> unit = acctdb_insert_ffi(h, off, len)

val acctdb_sel_ffi = impure { _: "acctdb_sel" } : hash -> bool
function acctdb_sel(h : hash) -> bool = acctdb_sel_ffi(h)

val acctdb_sel_off_ffi = impure { _: "acctdb_sel_off" } : unit -> bits(64)
function acctdb_sel_off() -> bits(64) = acctdb_sel_off_ffi()

val acctdb_sel_len_ffi = impure { _: "acctdb_sel_len" } : unit -> bits(64)
function acctdb_sel_len() -> bits(64) = acctdb_sel_len_ffi()

val acctdb_count_ffi = impure { _: "acctdb_count" } : unit -> bits(64)
function acctdb_count() -> bits(64) = acctdb_count_ffi()

val acctdb_at_ffi = impure { _: "acctdb_at" } : bits(64) -> unit
function acctdb_at(x0 : bits(64)) -> unit = acctdb_at_ffi(x0)

val acctdb_at_key_ffi = impure { _: "acctdb_at_key" } : unit -> hash
function acctdb_at_key() -> hash = acctdb_at_key_ffi()

val acctdb_at_off_ffi = impure { _: "acctdb_at_off" } : unit -> bits(64)
function acctdb_at_off() -> bits(64) = acctdb_at_off_ffi()

val acctdb_at_len_ffi = impure { _: "acctdb_at_len" } : unit -> bits(64)
function acctdb_at_len() -> bits(64) = acctdb_at_len_ffi()
