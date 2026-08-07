/* Optimized persistent-storage opcode paths for `sail/evm/execute.sail`. */

/* The complete optimized opcode implementations retain their dense account and
   storage handles entirely inside C. The Sail-visible contract remains the
   same unit-to-unit opcode semantics as the canonical definitions. */
val execute_sload_host = impure { c: "execute_sload" } : unit -> unit
val execute_sstore_host = impure { c: "execute_sstore" } : unit -> unit

function execute_sload() -> unit = execute_sload_host()
function execute_sstore() -> unit = execute_sstore_host()
