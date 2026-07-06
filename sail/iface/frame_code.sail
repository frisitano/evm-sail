/* ======================================================================== */
/* HOST INTERFACE CONTRACT: ACTIVE FRAME CODE VIEW                          */
/*                                                                          */
/* Abstract `val` declarations for the executing frame's code view (the     */
/* code views of the "Memory, calldata, returndata, code, stack" contract   */
/* group of proof/extern-boundary.md; ffi/code_db.c holds the C             */
/* implementation, sharing the frame-buffer machinery). Fetches refine an   */
/* abstract byte-list model of the bound code: build implementations must   */
/* refine those materialized byte-list semantics.                           */
/*                                                                          */
/* The frame_code_bind_* family binds where the frame's code comes from --  */
/* a store entry by codeHash, a caller-memory slice (CREATE/CREATE2 init    */
/* code, copied before the frame switch so executing code never aliases     */
/* mutable memory), or the transaction input (creation-tx init code) --     */
/* and each returns the bound code's length.                                */
/* ======================================================================== */

val frame_code_bind_stored : hash -> bits(64)
val frame_code_bind_memory : (bits(64), bits(64)) -> bits(64)
val frame_code_bind_tx_input : unit -> bits(64)
val frame_code_clear : unit -> unit

val frame_code_length : unit -> bits(64)
val frame_code_byte : bits(64) -> bits(8)
val frame_jumpdest_valid : bits(64) -> bool
val frame_push_immediate_word : (bits(64), bits(64)) -> word
val frame_code_copy_to_memory : (bits(64), bits(64), bits(64)) -> unit
