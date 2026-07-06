/* ======================================================================== */
/* HOST INTERFACE CONTRACT: PRECOMPILE EXECUTION                            */
/*                                                                          */
/* Abstract `val` declarations for running a precompile over a concrete     */
/* input byte source (part of the "Byte sources" / "Crypto and precompiles" */
/* contract groups of proof/extern-boundary.md). The gas rules inspect the  */
/* same source bytes directly (memory or top-level tx data) before this is  */
/* called; build backends may stage/snapshot the input internally and write */
/* successful output directly into pending returndata. The canonical        */
/* meaning is run_precompile(id, read_byte_slice(source)) -- the spec       */
/* backend's list-typed executor axiom (spec/precompile.sail).              */
/* ======================================================================== */

val run_precompile_memory_input : (int, int, int) -> bool
val run_precompile_tx_input : int -> bool
