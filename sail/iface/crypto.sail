/* ======================================================================== */
/* HOST INTERFACE CONTRACT: CRYPTO                                          */
/*                                                                          */
/* Abstract `val` declarations for hashing and the signing precompiles (the */
/* "Crypto and precompiles" contract group of proof/extern-boundary.md).    */
/* Each C source tag (Witness, Memory, TxInput, ActiveCode) denotes an      */
/* abstract byte sequence; optimized pointer/length build implementations   */
/* must refine the corresponding materialized byte-list semantics, e.g.     */
/* bytes_keccak(s) = keccak256(read_byte_slice(s)) and the prefixed SHA-256 */
/* digest equals sha256(type_byte :: bytes). Fixed-width hash and signing   */
/* helpers refine the corresponding canonical byte serialization; typed     */
/* precompile outputs carry explicit success bits. Source-backed code       */
/* storage is iface/code_db.sail; precompile-by-address execution is        */
/* iface/precompile.sail.                                                   */
/* ======================================================================== */

/* The hash primitives over materialized byte lists. keccak256 and sha256
 * are the axiomatic crypto core: abstract in the spec backend, implemented
 * in the build backend by streaming the list through the C host-bytes
 * channel. keccak_rlp_payload is a refinement pair whose canonical meaning
 * is keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload); the
 * build backend streams the three parts without concatenating them. All
 * shared consensus hashing (lib/consensus_hash.sail) is written against
 * these three, so RLP/signing/header layouts exist exactly once. */
val keccak256 : list(byte) -> bits(256)
val sha256 : list(byte) -> bits(256)
val keccak_rlp_payload : (list(byte), list(byte)) -> bits(256)

val bytes_keccak : ByteSlice -> bits(256)
val bytes_sha256_prefixed : (byte, ByteSlice) -> bits(256)

val keccak256_word : word -> bits(256)
val keccak256_address : address -> bits(256)

/* SSZ merkle parent: SHA-256(left ++ right) over two 32-byte chunks. */
val sha256_pair : (bits(256), bits(256)) -> bits(256)

/* Signature/authentication helpers. Spec uses the mathematical precompile
 * input byte strings; build may use direct staged or native accelerator calls. */
val secp256k1_verify_precompile : (bits(256), word, word, bits(256), bits(256)) -> bool
val ecrecover_precompile : (bits(256), int, word, word) -> (bool, address)
