/* Build backend: C FFI bindings for the execution-time kernel collections --
 * the EIP-2929 warm sets, the LOG series, the SELFDESTRUCT / EIP-6780 created
 * sets, and the call-frame journal (ffi/kernel_state.c). Thin `_ffi` externs
 * plus wrappers refining sail/iface/kernel_state.sail. */

val warm_reset_ffi = impure { _: "warm_reset" } : unit -> unit
function warm_reset() -> unit = warm_reset_ffi()

val warm_addr_contains_ffi = impure { _: "warm_addr_contains" } : address -> bool
function warm_addr_contains(a : address) -> bool = warm_addr_contains_ffi(a)

val warm_addr_insert_ffi = impure { _: "warm_addr_insert" } : address -> unit
function warm_addr_insert(a : address) -> unit = warm_addr_insert_ffi(a)

val warm_addr_remove_ffi = impure { _: "warm_addr_remove" } : address -> unit
function warm_addr_remove(a : address) -> unit = warm_addr_remove_ffi(a)

val warm_slot_contains_ffi = impure { _: "warm_slot_contains" } : (address, word) -> bool
function warm_slot_contains(a : address, s : word) -> bool = warm_slot_contains_ffi(a, s)

val warm_slot_insert_ffi = impure { _: "warm_slot_insert" } : (address, word) -> unit
function warm_slot_insert(a : address, s : word) -> unit = warm_slot_insert_ffi(a, s)

val warm_slot_remove_ffi = impure { _: "warm_slot_remove" } : (address, word) -> unit
function warm_slot_remove(a : address, s : word) -> unit = warm_slot_remove_ffi(a, s)

val logs_reset_ffi = impure { _: "logs_reset" } : unit -> unit
function logs_reset() -> unit = logs_reset_ffi()

val log_begin_ffi = impure { _: "log_begin" } : address -> unit
function log_begin(a : address) -> unit = log_begin_ffi(a)

val log_add_topic_ffi = impure { _: "log_add_topic" } : word -> unit
function log_add_topic(t : word) -> unit = log_add_topic_ffi(t)

val log_add_data_byte_ffi = impure { _: "log_add_data_byte" } : bits(8) -> unit
function log_add_data_byte(b : bits(8)) -> unit = log_add_data_byte_ffi(b)

val log_drop_last_ffi = impure { _: "log_drop_last" } : unit -> unit
function log_drop_last() -> unit = log_drop_last_ffi()

val log_count_ffi = impure { _: "log_count" } : unit -> bits(64)
function log_count() -> bits(64) = log_count_ffi()

val log_addr_ffi = impure { _: "log_addr" } : bits(64) -> word
function log_addr(i : bits(64)) -> word = log_addr_ffi(i)

val log_topic_count_ffi = impure { _: "log_topic_count" } : bits(64) -> bits(64)
function log_topic_count(i : bits(64)) -> bits(64) = log_topic_count_ffi(i)

val log_topic_ffi = impure { _: "log_topic" } : (bits(64), bits(64)) -> word
function log_topic(i : bits(64), j : bits(64)) -> word = log_topic_ffi(i, j)

val log_data_len_ffi = impure { _: "log_data_len" } : bits(64) -> bits(64)
function log_data_len(i : bits(64)) -> bits(64) = log_data_len_ffi(i)

val log_data_byte_ffi = impure { _: "log_data_byte" } : (bits(64), bits(64)) -> bits(8)
function log_data_byte(i : bits(64), k : bits(64)) -> bits(8) = log_data_byte_ffi(i, k)

val selfdestr_reset_ffi = impure { _: "selfdestr_reset" } : unit -> unit
function selfdestr_reset() -> unit = selfdestr_reset_ffi()

val selfdestr_push_ffi = impure { _: "selfdestr_push" } : address -> unit
function selfdestr_push(a : address) -> unit = selfdestr_push_ffi(a)

val selfdestr_drop_last_ffi = impure { _: "selfdestr_drop_last" } : unit -> unit
function selfdestr_drop_last() -> unit = selfdestr_drop_last_ffi()

val selfdestr_contains_ffi = impure { _: "selfdestr_contains" } : address -> bool
function selfdestr_contains(a : address) -> bool = selfdestr_contains_ffi(a)

val selfdestr_count_ffi = impure { _: "selfdestr_count" } : unit -> bits(64)
function selfdestr_count() -> bits(64) = selfdestr_count_ffi()

val selfdestr_get_ffi = impure { _: "selfdestr_get" } : bits(64) -> word
function selfdestr_get(i : bits(64)) -> word = selfdestr_get_ffi(i)

val created_reset_ffi = impure { _: "created_reset" } : unit -> unit
function created_reset() -> unit = created_reset_ffi()

val created_insert_ffi = impure { _: "created_insert" } : address -> unit
function created_insert(a : address) -> unit = created_insert_ffi(a)

val created_contains_ffi = impure { _: "created_contains" } : address -> bool
function created_contains(a : address) -> bool = created_contains_ffi(a)

val journal_reset_ffi = impure { _: "journal_reset" } : unit -> unit
function journal_reset() -> unit = journal_reset_ffi()

val journal_push_check_ffi = impure { _: "journal_push_check" } : unit -> unit
function journal_push_check() -> unit = journal_push_check_ffi()

val journal_push_acct_ffi = impure { _: "journal_push_acct" } : (address, bits(64), word, hash, hash) -> unit
function journal_push_acct(a : address, nonce : bits(64), bal : word, sroot : hash, chash : hash) -> unit =
    journal_push_acct_ffi(a, nonce, bal, sroot, chash)

val journal_push_tran_ffi = impure { _: "journal_push_tran" } : (address, word, word) -> unit
function journal_push_tran(a : address, slot : word, val_ : word) -> unit = journal_push_tran_ffi(a, slot, val_)

val journal_push_warma_ffi = impure { _: "journal_push_warma" } : address -> unit
function journal_push_warma(a : address) -> unit = journal_push_warma_ffi(a)

val journal_push_warms_ffi = impure { _: "journal_push_warms" } : (address, word) -> unit
function journal_push_warms(a : address, slot : word) -> unit = journal_push_warms_ffi(a, slot)

val journal_push_log_ffi = impure { _: "journal_push_log" } : unit -> unit
function journal_push_log() -> unit = journal_push_log_ffi()

val journal_push_refund_ffi = impure { _: "journal_push_refund" } : bits(64) -> unit
function journal_push_refund(old : bits(64)) -> unit = journal_push_refund_ffi(old)

val journal_push_selfd_ffi = impure { _: "journal_push_selfd" } : unit -> unit
function journal_push_selfd() -> unit = journal_push_selfd_ffi()

val journal_commit_ffi = impure { _: "journal_commit" } : unit -> unit
function journal_commit() -> unit = journal_commit_ffi()

val journal_top_tag_ffi = impure { _: "journal_top_tag" } : unit -> bits(64)
function journal_top_tag() -> bits(64) = journal_top_tag_ffi()

val journal_drop_top_ffi = impure { _: "journal_drop_top" } : unit -> unit
function journal_drop_top() -> unit = journal_drop_top_ffi()

val journal_top_addr_ffi = impure { _: "journal_top_addr" } : unit -> word
function journal_top_addr() -> word = journal_top_addr_ffi()

val journal_top_nonce_ffi = impure { _: "journal_top_nonce" } : unit -> bits(64)
function journal_top_nonce() -> bits(64) = journal_top_nonce_ffi()

val journal_top_balance_ffi = impure { _: "journal_top_balance" } : unit -> word
function journal_top_balance() -> word = journal_top_balance_ffi()

val journal_top_sroot_ffi = impure { _: "journal_top_sroot" } : unit -> hash
function journal_top_sroot() -> hash = journal_top_sroot_ffi()

val journal_top_chash_ffi = impure { _: "journal_top_chash" } : unit -> hash
function journal_top_chash() -> hash = journal_top_chash_ffi()

val journal_top_slot_ffi = impure { _: "journal_top_slot" } : unit -> word
function journal_top_slot() -> word = journal_top_slot_ffi()

val journal_top_val_ffi = impure { _: "journal_top_val" } : unit -> word
function journal_top_val() -> word = journal_top_val_ffi()

val journal_top_refund_ffi = impure { _: "journal_top_refund" } : unit -> bits(64)
function journal_top_refund() -> bits(64) = journal_top_refund_ffi()
