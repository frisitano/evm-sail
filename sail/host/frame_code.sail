/* ======================================================================== */
/* ACTIVE FRAME CODE VIEW                                                   */
/*                                                                          */
/* The code the current frame executes (Yellow Paper I_b), as a C-side view */
/* (ffi/code_db.c, sharing the frame-buffer machinery): frame_code_bind_*   */
/* binds where the frame's code comes from -- a store entry by codeHash, a  */
/* caller-memory slice (CREATE init code, copied before the frame switch so */
/* executing code never aliases mutable memory), or the tx input -- and     */
/* returns the bound code's length. The byte fetch, JUMPDEST test, and      */
/* immediate-word reads serve the fetch-decode loop; the copy hook          */
/* serves CODECOPY (host/memory.sail mem_codecopy).                         */
/* ======================================================================== */

val frame_code_bind_stored = impure { c: "frame_code_bind_stored" } : hash -> bits(64)
val frame_code_bind_memory = impure { c: "frame_code_bind_memory" } : (bits(64), bits(64)) -> bits(64)
val frame_code_bind_tx_input = impure { c: "frame_code_bind_tx_input" } : unit -> bits(64)
val frame_code_clear = impure { c: "frame_code_clear" } : unit -> unit
val frame_code_length = impure { c: "frame_code_length" } : unit -> bits(64)
val frame_code_byte = impure { c: "frame_code_byte" } : bits(64) -> bits(8)
val frame_jumpdest_valid = impure { c: "frame_jumpdest_valid" } : bits(64) -> bool
val frame_push_immediate_word = impure { c: "frame_push_immediate_word" } : (bits(64), bits(64)) -> word
val frame_code_copy_to_memory = impure { c: "frame_code_copy_to_memory" } : (bits(64), bits(64), bits(64)) -> unit
