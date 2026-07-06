/* ======================================================================== */
/* HOST INTERFACE CONTRACT: CODE DATABASE                                   */
/*                                                                          */
/* Abstract `val` declarations for the C-backed contract-code store (the    */
/* code-lookup half of the "World state and code DB" contract group of      */
/* proof/extern-boundary.md). Content-addressed code lookup/update refines  */
/* abstract byte-list models keyed by codeHash: build implementations must  */
/* refine those materialized byte-list semantics. The ACTIVE frame's code   */
/* view is a separate contract (iface/frame_code.sail).                     */
/* ======================================================================== */

/* ---- C code store (ffi/code_db.c): contract code, content-addressed --------
 * Keyed by codeHash (not address): each entry holds the code bytes AND the
 * JUMPDEST validity bitmap (built once per code WRITE -- pre-state seeding,
 * CREATE deploys, EIP-7702 delegations). Content-addressing makes the store
 * de-duplicated and append-only, so REVERT needs no code rollback: restoring an
 * account's codeHash restores its code. An account resolves its code by codeHash
 * (k_code_key).
 *
 * Writes are streamed (begin selects/creates the entry, byte appends) because
 * the bytes originate in Sail lists on the native seeding and CREATE-deploy
 * paths; the stateless witness path bypasses streaming and stores code by
 * source span (code_db_store_slice below). */
val code_db_stream_code_begin : hash -> bits(64)
val code_db_stream_code_byte : bits(8) -> unit
val code_db_stored_code_length : hash -> bits(64)
val code_db_copy_stored_code_to_memory : (hash, bits(64), bits(64), bits(64)) -> unit

/* Store code from a byte source directly under its keccak hash (the stateless
 * witness path): build copies the source span into the store without a Sail
 * list; spec/hybrid materialize the bytes and store the list. Refines
 * code_db_store_code(keccak256(read_byte_slice(s)), read_byte_slice(s)). */
val code_db_store_slice : ByteSlice -> unit

/* EIP-7702 delegation probe for codeHash `h`, packed for one FFI call on the
 * hot CALL-family path: bit 160 = "the code is a delegation designation
 * (0xef0100 ++ address)", bits 159..0 = the target address (zeros otherwise). */
val code_db_read_delegation : hash -> bits(168)
