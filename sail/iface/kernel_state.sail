/* ======================================================================== */
/* HOST INTERFACE CONTRACT: KERNEL COLLECTIONS                              */
/*                                                                          */
/* Abstract `val` declarations for the C-backed execution-time kernel       */
/* collections that were Sail registers holding mutable data buffers: the   */
/* EIP-2929 warm address/slot sets, the LOG series, the SELFDESTRUCT and    */
/* EIP-6780 created-this-tx address sets, and the call-frame journal (undo  */
/* log). Each refines an abstract set / ordered-list model with the dedup,  */
/* ordering, and call-frame revert laws the Sail registers had; the build   */
/* backend implements them in C (ffi/kernel_state.c). The `spec` backend    */
/* leaves them as bodyless axioms constrained by those laws.                */
/* ======================================================================== */

/* ---- EIP-2929 warm access sets: membership sets over address / (address,  */
/* slot). insert/remove pair per the journal (a frame revert un-warms a      */
/* newly-accessed entry); only `contains` is ever queried, so order is       */
/* irrelevant. reset clears both sets (per tx / per world reset). ---- */
val warm_reset : unit -> unit
val warm_addr_contains : address -> bool
val warm_addr_insert : address -> unit
val warm_addr_remove : address -> unit
val warm_slot_contains : (address, word) -> bool
val warm_slot_insert : (address, word) -> unit
val warm_slot_remove : (address, word) -> unit

/* ---- LOG series: an ordered list of records (emitter address, 0..4 topics, */
/* data bytes). Records are streamed (log_begin then topics/data), the most   */
/* recent record is dropped on a frame revert (JLog), and the whole ordered   */
/* list is read back into the receipt via the count/getters. ---- */
val logs_reset : unit -> unit
val log_begin : address -> unit
val log_add_topic : word -> unit
val log_add_data_byte : bits(8) -> unit
val log_drop_last : unit -> unit
val log_count : unit -> bits(64)
val log_addr : bits(64) -> word
val log_topic_count : bits(64) -> bits(64)
val log_topic : (bits(64), bits(64)) -> word
val log_data_len : bits(64) -> bits(64)
val log_data_byte : (bits(64), bits(64)) -> bits(8)

/* ---- SELFDESTRUCT set: ordered (push at end, drop-last on a frame revert   */
/* via JSelfD), with a membership test and index enumeration for the EIP-6780 */
/* tx-end finalization. ---- */
val selfdestr_reset : unit -> unit
val selfdestr_push : address -> unit
val selfdestr_drop_last : unit -> unit
val selfdestr_contains : address -> bool
val selfdestr_count : unit -> bits(64)
val selfdestr_get : bits(64) -> word

/* ---- EIP-6780 created-this-tx set: idempotent-insert membership set. NOT   */
/* journaled -- like EELS created_accounts the marker survives a frame        */
/* revert. ---- */
val created_reset : unit -> unit
val created_insert : address -> unit
val created_contains : address -> bool

/* ---- call-frame journal (undo log): a stack of undo entries. push_* append  */
/* one entry; commit drops the most recent checkpoint marker, folding the      */
/* frame's entries into the parent; revert reads the top entry's tag + payload */
/* (top_*), the caller dispatches its undo, then drop_top pops it, back to and */
/* including the checkpoint. top_tag returns JT_EMPTY on an empty journal. The */
/* payload getters are tag-indexed views over one entry record: addr, the      */
/* prior account (nonce/balance/storageRoot/codeHash), a (slot, value) pair,   */
/* and the prior refund word. ---- */
val journal_reset : unit -> unit
val journal_push_check : unit -> unit
val journal_push_acct : (address, bits(64), word, hash, hash) -> unit
val journal_push_tran : (address, word, word) -> unit
val journal_push_stor : (address, word, word) -> unit
val journal_push_warma : address -> unit
val journal_push_warms : (address, word) -> unit
val journal_push_log : unit -> unit
val journal_push_refund : bits(64) -> unit
val journal_push_selfd : unit -> unit
val journal_commit : unit -> unit
val journal_top_tag : unit -> bits(64)
val journal_drop_top : unit -> unit
val journal_top_addr : unit -> word
val journal_top_nonce : unit -> bits(64)
val journal_top_balance : unit -> word
val journal_top_sroot : unit -> hash
val journal_top_chash : unit -> hash
val journal_top_slot : unit -> word
val journal_top_val : unit -> word
val journal_top_refund : unit -> bits(64)

/* journal entry tags, shared with ffi/kernel_state.c (MUST stay in sync). */
let JT_EMPTY : bits(64) = 0x0000000000000000
let JT_CHECK : bits(64) = 0x0000000000000001
let JT_ACCT : bits(64) = 0x0000000000000002
let JT_TRAN : bits(64) = 0x0000000000000003
let JT_WARMA : bits(64) = 0x0000000000000004
let JT_WARMS : bits(64) = 0x0000000000000005
let JT_LOG : bits(64) = 0x0000000000000006
let JT_REFUND : bits(64) = 0x0000000000000007
let JT_SELFD : bits(64) = 0x0000000000000008
let JT_STOR : bits(64) = 0x0000000000000009
