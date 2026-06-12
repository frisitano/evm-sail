/* ======================================================================== */
/* SHA-256 via the zkvm-standards accelerator boundary (accelerators.sail ->    */
/* ffi/zkvm_accelerators.h zkvm_sha256, id 2) -- used by SSZ hash_tree_root      */
/* merkleization (distinct from execution keccak). Same C host as keccak and     */
/* the precompiles; a real zkVM swaps the reference impl for a native sha256.    */
/* ======================================================================== */

function sha256(msg : list(byte)) -> bits(256) = acc_hash(2, msg)

/* SSZ merkle parent: SHA-256(left ++ right), both 32-byte chunks */
function sha256_pair(a : bits(256), b : bits(256)) -> bits(256) =
  acc_hash(2, list_append(word_to_bytes32(a), word_to_bytes32(b)))

/* ---- S4a self-check: SHA-256 reference vectors on guest ---------------- */
function sha256_selfcheck() -> unit = {
  assert(sha256([||]) == 0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855,
         "sha256('') on guest");
  assert(sha256([| 0x61, 0x62, 0x63 |]) == 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad,
         "sha256('abc') on guest")
}
