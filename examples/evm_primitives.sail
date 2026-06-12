/* ======================================================================== */
/* EL-IR runnable primitive bodies (for EXECUTION, not the canonical spec)   */
/*                                                                           */
/* The canonical model (sail/) leaves KECCAK-256, CREATE address derivation, */
/* and the precompiles as declared `val` primitives. To *execute* the model  */
/* (compile to C and run a block), those vals need bodies. The bodies here    */
/* are deterministic, total, and executable — but the crypto ones are        */
/* NON-CRYPTOGRAPHIC placeholders (a simple FNV-style fold), present only so  */
/* an example block can run end-to-end. A production oracle would link the    */
/* real keccak / secp256k1 / bn254 / kzg implementations here instead.       */
/* ======================================================================== */

/* keccak256 and create_address/create2_address are now real (sail/keccak.sail),
 * and the EVM precompiles now run through the C host accelerator
 * (sail/precompile.sail -> ffi/precompile_ffi.c), structured like keccak. So this
 * file no longer supplies any placeholder bodies; it remains as a stable include
 * point for examples. */
