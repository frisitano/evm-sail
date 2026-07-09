/* ======================================================================== */
/* C BINDINGS: private-input oracle + public-output trace                   */
/*                                                                          */
/* Including this file selects the C implementation of the zkVM I/O surface */
/* (test_utils.c natively, zkvm_input.c on the guest). Sail's extern rule   */
/* makes each binding OVERRIDE the shared pure body where one exists        */
/* (host/io.sail ssz_src_le/be); the primitives (ssz_src_len/byte,          */
/* emit_out) have no pure body -- they are the input/output axioms. Proof   */
/* targets see all of these as bodyless axioms (the c: key binds only C).   */
/* ======================================================================== */

val ssz_src_len = impure { c: "ssz_src_len" } : unit -> bits(64)
val ssz_src_byte = impure { c: "ssz_src_byte" } : int -> bits(8)
val ssz_src_le = impure { c: "ssz_src_le" } : (int, int) -> bits(64)
val ssz_src_be = impure { c: "ssz_src_be" } : (int, int) -> bits(64)
val emit_out = impure { c: "el_emit_out" } : bits(8) -> unit
