/* ======================================================================== */
/* HOST INTERFACE CONTRACT: MEMORY, RETURNDATA, CALLDATA, TX INPUT          */
/*                                                                          */
/* Abstract `val` declarations for per-frame EVM memory, the returndata     */
/* buffer, calldata, and the transaction-input view (part of the "Memory,   */
/* calldata, returndata, code, stack" contract group of                     */
/* proof/extern-boundary.md). Host memory has read-after-write laws; frame  */
/* push/pop is LIFO; memory moves, calldata, returndata, and transaction    */
/* input views refine abstract byte models: build implementations must      */
/* refine those materialized byte-list semantics.                           */
/* ======================================================================== */

val host_mem_read : bits(64) -> bits(8)
val host_mem_write : (bits(64), bits(8)) -> unit
val host_mem_reset : unit -> unit
val host_mem_push : unit -> unit
val host_mem_pop : unit -> unit

/* ---- range copies (CALLDATACOPY / CODECOPY / RETURNDATACOPY / MCOPY):
 * endpoints already live in C buffers, so copies stay on the C side. ---- */
val hm_move : (bits(64), bits(64), bits(64)) -> unit

/* ---- RETURNDATA buffer (per call frame; see ffi/returndata.c) ----
 * The bytes a child frame returned (RETURN/REVERT) or a precompile produced,
 * read by RETURNDATASIZE/RETURNDATACOPY (EIP-211). Cleared at the start of
 * every sub-call and replaced by the child's output on return. The build
 * backend keeps the bytes in C; the common model uses domain-level operations
 * instead of exposing that implementation detail. ---- */
val returndata_reset_all : unit -> unit
val returndata_clear_current : unit -> unit
val returndata_discard_pending : unit -> unit
val returndata_capture_memory : (bits(64), bits(64)) -> unit
val returndata_adopt_pending : unit -> unit
val returndata_current_len : unit -> bits(64)
val returndata_pending_len : unit -> bits(64)
val returndata_pending_byte_at : bits(64) -> bits(8)
val returndata_copy_current_to_memory : (bits(64), bits(64), bits(64)) -> unit
val returndata_copy_current_prefix_to_memory : (bits(64), bits(64)) -> unit

/* ---- C-backed CALLDATA (per call frame) ----
 * The current frame's input data, read by CALLDATALOAD/CALLDATASIZE/CALLDATACOPY.
 * For a CALL-family child it aliases the slice of the caller's memory passed as
 * args; for the top-level frame it is the transaction input. ---- */
val cd_set : (bits(64), bits(64)) -> unit
val cd_set_empty : unit -> unit
val cd_set_tx : unit -> unit

/* Byte-loop-free tx-input staging (the build backend does each C-side in one
 * call; spec/hybrid materialize + loop). txdata_set_source stages a byte source
 * (the witness span for a stateless tx, or the already-staged native-runner
 * input) and txdata_set_word a 32-byte word (a block system call); each returns
 * the staged length. txd_count_nonzero counts nonzero staged bytes for the
 * EIP-2028 / EIP-7623 calldata gas. */
val txdata_set_source : ByteSlice -> int
val txdata_set_word : word -> int
val txd_count_nonzero : unit -> bits(64)

/* Streamed tx-input staging: retained ONLY for the native-runner pre-state
 * decode path (it decodes the input a scalar at a time); the stateless guest
 * proving path never streams -- it stages the witness span in one memcpy. */
val txin_begin : unit -> unit
val txin_byte : bits(8) -> unit
val cd_to_mem : (bits(64), bits(64), bits(64)) -> unit

/* CALLDATALOAD: the 32-byte word at calldata offset i (zero-padded past the
 * end). Implemented in ffi/code_db.c, which owns the frame-buffer machinery
 * that calldata shares with the frame code view. */
val calldata_load_word : bits(64) -> word
val txd_at : bits(64) -> bits(8)
val txd_length : unit -> bits(64)

/* Source -> bytes: the read side of the byte-source interface. Build mode
 * hashes/stages each source C-side without materializing, so this is uncalled
 * there; spec mode uses it as the abstract read axiom that the crypto and
 * precompile refinements are written over. Law: read_byte_slice(s) yields
 * exactly s.len bytes of source s starting at offset s.off. */
val read_byte_slice : ByteSlice -> list(byte)
