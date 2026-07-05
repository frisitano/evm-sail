/* ======================================================================== */
/* HOST INTERFACE CONTRACT: CODE DATABASE                                   */
/*                                                                          */
/* Abstract `val` declarations for the C-backed contract-code store and the */
/* active frame's code view (spanning the code views of the "Memory,        */
/* calldata, returndata, code, stack" group and the code-lookup half of     */
/* the "World state and code DB" contract group of                          */
/* proof/extern-boundary.md). Frame code views, code copies, and            */
/* content-addressed code lookup/update refine abstract byte-list models    */
/* keyed by codeHash: build implementations must refine those materialized  */
/* byte-list semantics.                                                     */
/* ======================================================================== */

val code_db_frame_jumpdest_valid : bits(64) -> bool
val code_db_frame_code_byte : bits(64) -> bits(8)
val code_db_frame_push_immediate_word : (bits(64), bits(64)) -> word
val code_db_frame_set_stored_code : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val code_db_prepare_child_frame_memory_code : (bits(64), bits(64)) -> bits(64)
val code_db_frame_set_tx_input_code : unit -> bits(64)
val code_db_frame_clear_code : unit -> unit
val code_db_frame_code_length : unit -> bits(64)

val code_db_copy_frame_code_to_memory : (bits(64), bits(64), bits(64)) -> unit
val code_db_calldata_load_word : bits(64) -> word

val code_db_read_delegation : (bits(64), bits(64), bits(64), bits(64)) -> bits(168)

/* ---- C code store (ffi/code_db.c): contract code, content-addressed --------
 * Keyed by codeHash (not address): each entry holds the code bytes AND the
 * JUMPDEST validity bitmap (built once per code WRITE -- pre-state seeding,
 * CREATE deploys, EIP-7702 delegations). Content-addressing makes the store
 * de-duplicated and append-only, so REVERT needs no code rollback: restoring an
 * account's codeHash restores its code. An account resolves its code by codeHash
 * (k_code_key). */
val code_db_stream_code_begin : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val code_db_stored_code_length : (bits(64), bits(64), bits(64), bits(64)) -> bits(64)
val code_db_copy_stored_code_to_memory : (bits(64), bits(64), bits(64), bits(64), bits(64), bits(64), bits(64)) -> unit
val code_db_stream_code_byte : bits(8) -> unit
