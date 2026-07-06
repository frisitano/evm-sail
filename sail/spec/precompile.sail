/* ======================================================================== */
/* SPEC BACKEND: PRECOMPILE EXECUTION                                       */
/*                                                                          */
/* The abstract precompile executor over a materialized input list. Build   */
/* refines it with a pointer-backed source runner (build/precompile.sail).  */
/* run_precompile_memory_input / run_precompile_tx_input (iface/precompile) */
/* stay abstract in the spec backend -- their canonical meaning is          */
/* run_precompile(id, read_byte_slice(source)); only the list-typed axiom   */
/* and the signing-precompile checks in spec/crypto.sail are given bodies.  */
/* ======================================================================== */

val run_precompile : (int, list(byte)) -> (bool, list(byte))
