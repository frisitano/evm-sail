/*md
# zkVM cryptographic accelerators

Raw zkVM cryptographic accelerators (the eth-act zkvm-standards surface).
These axioms know the accelerator ABI but not EVM precompile addresses,
fork activation, gas, or validation; byte-array results are written to the
shared precompile output arena. Proof targets see them as bodyless axioms.
*/

val accelerator_ripemd160 = impure { c: "accelerator_ripemd160" } : ByteSlice -> bool
val host_accelerator_modexp = impure { c: "accelerator_modexp" } : (ByteSlice, bits(64), bits(64), bits(64)) -> bool
val accelerator_bn254_add = impure { c: "accelerator_bn254_add" } : ByteSlice -> bool
val accelerator_bn254_mul = impure { c: "accelerator_bn254_mul" } : ByteSlice -> bool
val accelerator_bn254_pairing = impure { c: "accelerator_bn254_pairing" } : ByteSlice -> bits(2)
val host_accelerator_blake2f = impure { c: "accelerator_blake2f" } : (ByteSlice, bits(64), bits(64)) -> bool
val accelerator_kzg_point_evaluation = impure { c: "accelerator_kzg_point_evaluation" } : ByteSlice -> bool
val accelerator_bls_g1_add = impure { c: "accelerator_bls_g1_add" } : ByteSlice -> bool
val accelerator_bls_g1_msm = impure { c: "accelerator_bls_g1_msm" } : ByteSlice -> bool
val accelerator_bls_g2_add = impure { c: "accelerator_bls_g2_add" } : ByteSlice -> bool
val accelerator_bls_g2_msm = impure { c: "accelerator_bls_g2_msm" } : ByteSlice -> bool
val accelerator_bls_pairing = impure { c: "accelerator_bls_pairing" } : ByteSlice -> bits(2)
val accelerator_bls_map_fp_to_g1 = impure { c: "accelerator_bls_map_fp_to_g1" } : ByteSlice -> bool
val accelerator_bls_map_fp2_to_g2 = impure { c: "accelerator_bls_map_fp2_to_g2" } : ByteSlice -> bool
val accelerator_p256_verify = impure { c: "accelerator_p256_verify" } : ByteSlice -> bool

/*! secp256k1 signature verification over `(hash, r, s, X, Y)` — the
    sender-authentication axiom. */
val secp256k1_verify = impure { c: "precompile_secp256k1_verify_hash_sig_pub" } : (
        bits(256),
        word,
        word,
        bits(256),
        bits(256),
    ) -> bool
/*! secp256k1 public-key recovery; the leading bit of the 168-bit result
    flags success, the low 160 bits are the address. */
val host_ecrecover = impure { c: "precompile_ecrecover_hash_sig" } : (bits(256), bits(64), word, word) -> bits(168)

/*! `MODEXP` accelerator adapter (integer lengths to the 64-bit ABI). */
function accelerator_modexp(input : ByteSlice, base_len : int, exponent_len : int, modulus_len : int) -> bool =
    host_accelerator_modexp(
        input,
        get_slice_int(64, base_len, 0),
        get_slice_int(64, exponent_len, 0),
        get_slice_int(64, modulus_len, 0),
    )

/*! `BLAKE2f` accelerator adapter. */
function accelerator_blake2f(input : ByteSlice, rounds : int, final_block : int) -> bool =
    host_accelerator_blake2f(input, get_slice_int(64, rounds, 0), get_slice_int(64, final_block, 0))
