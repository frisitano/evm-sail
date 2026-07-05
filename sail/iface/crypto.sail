/* ======================================================================== */
/* HOST INTERFACE CONTRACT: BYTE SOURCES, CRYPTO, AND PRECOMPILES           */
/*                                                                          */
/* Abstract `val` declarations for source-backed hashing/storage/precompile */
/* execution and the typed fixed-width crypto helpers (the "Byte sources"   */
/* and "Crypto and precompiles" contract groups of                          */
/* proof/extern-boundary.md). Each C source tag (Witness, Memory, TxInput,  */
/* ActiveCode) denotes an abstract byte sequence; optimized pointer/length  */
/* build implementations must refine the corresponding materialized         */
/* byte-list semantics, e.g. bytes_keccak(s) =                              */
/* keccak256(read_byte_slice(s)), the prefixed SHA-256 digest equals        */
/* sha256(type_byte :: bytes), code storage equals                          */
/* code_db_store_code(keccak256(bytes), bytes), and source-backed           */
/* precompile execution equals run_precompile(id, bytes). Fixed-width hash  */
/* and signing helpers refine the corresponding canonical byte              */
/* serialization; typed precompile outputs carry explicit success bits.     */
/* ======================================================================== */

/* Run a precompile over a concrete input source. The gas rules inspect the
 * same source bytes directly (memory or top-level tx data) before this is
 * called. Build backends may stage/snapshot the input internally and write
 * successful output directly into pending returndata. */
val run_precompile_memory_input : (int, int, int) -> bool
val run_precompile_tx_input : int -> bool

/* ======================================================================= */
/* HASHED SEMANTIC OPERATIONS                                              */
/*                                                                         */
/* Common execution code names the operation it needs: secure trie keys,   */
/* SSZ Merkle parents, code storage, tx/auth preimage hashing, and signing */
/* precompile checks. spec.sail defines these in terms of canonical byte   */
/* lists. build.sail may refine these with pointer/length FFI hooks or     */
/* staged byte sinks.                                                      */
/* ======================================================================= */

val bytes_keccak : ByteSlice -> bits(256)
val bytes_sha256_prefixed : (byte, ByteSlice) -> bits(256)
val code_db_store_slice : ByteSlice -> unit

val keccak256_word : word -> bits(256)
val keccak256_address : address -> bits(256)

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
val sha256_pair : (bits(256), bits(256)) -> bits(256)

/* Signature/authentication helpers. Spec uses the mathematical precompile
 * input byte strings; build may use direct staged or native accelerator calls. */
val secp256k1_verify_precompile : (bits(256), word, word, bits(256), bits(256)) -> bool
val ecrecover_precompile : (bits(256), int, word, word) -> (bool, address)
