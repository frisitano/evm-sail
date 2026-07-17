/* Raw zkVM cryptographic accelerators. These functions know the accelerator
 * ABI but not EVM precompile addresses, fork activation, gas, or validation.
 * Byte-array results are written to the shared precompile output arena. */

val accelerator_ripemd160 = impure { c: "accelerator_ripemd160" } : ByteSlice -> bool
val accelerator_modexp = impure { c: "accelerator_modexp" } : (ByteSlice, byte_length, byte_length, byte_length) -> bool
val accelerator_bn254_add = impure { c: "accelerator_bn254_add" } : ByteSlice -> bool
val accelerator_bn254_mul = impure { c: "accelerator_bn254_mul" } : ByteSlice -> bool
val accelerator_bn254_pairing = impure { c: "accelerator_bn254_pairing" } : ByteSlice -> bits(2)
val accelerator_blake2f = impure { c: "accelerator_blake2f" } : (ByteSlice, blake2_rounds, y_parity) -> bool
val accelerator_kzg_point_evaluation = impure { c: "accelerator_kzg_point_evaluation" } : ByteSlice -> bool
val accelerator_bls_g1_add = impure { c: "accelerator_bls_g1_add" } : ByteSlice -> bool
val accelerator_bls_g1_msm = impure { c: "accelerator_bls_g1_msm" } : ByteSlice -> bool
val accelerator_bls_g2_add = impure { c: "accelerator_bls_g2_add" } : ByteSlice -> bool
val accelerator_bls_g2_msm = impure { c: "accelerator_bls_g2_msm" } : ByteSlice -> bool
val accelerator_bls_pairing = impure { c: "accelerator_bls_pairing" } : ByteSlice -> bits(2)
val accelerator_bls_map_fp_to_g1 = impure { c: "accelerator_bls_map_fp_to_g1" } : ByteSlice -> bool
val accelerator_bls_map_fp2_to_g2 = impure { c: "accelerator_bls_map_fp2_to_g2" } : ByteSlice -> bool
val accelerator_p256_verify = impure { c: "accelerator_p256_verify" } : ByteSlice -> bool

val secp256k1_verify = impure { c: "precompile_secp256k1_verify_hash_sig_pub" } : (
        bits(256),
        word,
        word,
        bits(256),
        bits(256),
    ) -> bool
val host_ecrecover = impure { c: "precompile_ecrecover_hash_sig" } : (bits(256), y_parity, word, word) -> bits(168)
