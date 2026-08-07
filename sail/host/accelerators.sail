
/*md
# The accelerator interface

Raw cryptographic accelerators (the eth-act zkvm-standards surface).
These axioms know the accelerator ABI but not EVM precompile addresses,
fork activation, gas, or validation; byte-array results are written to the
shared precompile output arena. Proof targets see them as bodyless axioms.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/
/*! Encoded pairing-check status: values `0`/`1` are malformed input and
    values `2`/`3` are valid input with a false/true pairing result. */
type pairing_check_result = range(0, 3)

/*! RIPEMD-160 over the input, writing the left-padded 32-byte digest to
    the output arena; the flag reports acceleration success. */
val accelerator_ripemd160 = impure { c: "accelerator_ripemd160" } : CalldataSlice -> bool
/*! Modular exponentiation over the concatenated base, exponent, and modulus
    fields, writing a modulus-sized result; the flag reports acceleration
    success. */
val accelerator_modexp = impure { c: "accelerator_modexp" } : (
        CalldataSlice,
        calldata_length,
        calldata_length,
        calldata_length,
    ) -> bool
/*! BN254 G1 point addition over the 128-byte two-point input, writing
    the 64-byte sum; `false` on a malformed point. */
val accelerator_bn254_add = impure { c: "accelerator_bn254_add" } : CalldataSlice -> bool
/*! BN254 G1 scalar multiplication over the 96-byte point-and-scalar
    input, writing the 64-byte product; `false` on a malformed point. */
val accelerator_bn254_mul = impure { c: "accelerator_bn254_mul" } : CalldataSlice -> bool
/*! BN254 pairing check over the `(G1, G2)` pair list: bit 1 flags input
    validity, bit 0 the pairing outcome (decoded by [pairing_result][]). */
val accelerator_bn254_pairing = impure { c: "accelerator_bn254_pairing" } : CalldataSlice -> pairing_check_result
/*! The BLAKE2 compression function over its fixed-width input, using the
    supplied round count and final-block flag and writing the 64-byte result. */
val accelerator_blake2f = impure { c: "accelerator_blake2f" } : (CalldataSlice, blake2_rounds, y_parity) -> bool
/*! KZG point-evaluation proof verification (EIP-4844) over the 192-byte
    versioned-hash/point/proof input, writing the constant 64-byte return
    payload on success. */
val accelerator_kzg_point_evaluation = impure { c: "accelerator_kzg_point_evaluation" } : CalldataSlice -> bool
/*! BLS12-381 G1 point addition (EIP-2537) over two 128-byte encoded
    points, writing the 128-byte sum; `false` on a malformed input. */
val accelerator_bls_g1_add = impure { c: "accelerator_bls_g1_add" } : CalldataSlice -> bool
/*! BLS12-381 G1 multi-scalar multiplication (EIP-2537), writing the
    128-byte result; `false` on a malformed input. */
val accelerator_bls_g1_msm = impure { c: "accelerator_bls_g1_msm" } : CalldataSlice -> bool
/*! BLS12-381 G2 point addition (EIP-2537) over two 256-byte encoded
    points, writing the 256-byte sum; `false` on a malformed input. */
val accelerator_bls_g2_add = impure { c: "accelerator_bls_g2_add" } : CalldataSlice -> bool
/*! BLS12-381 G2 multi-scalar multiplication (EIP-2537), writing the
    256-byte result; `false` on a malformed input. */
val accelerator_bls_g2_msm = impure { c: "accelerator_bls_g2_msm" } : CalldataSlice -> bool
/*! BLS12-381 pairing check (EIP-2537): bit 1 flags input validity,
    bit 0 the pairing outcome (decoded by [pairing_result][]). */
val accelerator_bls_pairing = impure { c: "accelerator_bls_pairing" } : CalldataSlice -> pairing_check_result
/*! Maps a 64-byte encoded field element to a G1 point (EIP-2537),
    writing the 128-byte result. */
val accelerator_bls_map_fp_to_g1 = impure { c: "accelerator_bls_map_fp_to_g1" } : CalldataSlice -> bool
/*! Maps a 128-byte encoded Fp2 element to a G2 point (EIP-2537),
    writing the 256-byte result. */
val accelerator_bls_map_fp2_to_g2 = impure { c: "accelerator_bls_map_fp2_to_g2" } : CalldataSlice -> bool
/*! P-256 (secp256r1) signature verification over the 160-byte
    `(hash, r, s, x, y)` input (EIP-7951); the flag is the verification
    outcome. */
val accelerator_p256_verify = impure { c: "accelerator_p256_verify" } : CalldataSlice -> bool

/*! secp256k1 public-key recovery, returning its success flag and address. */
val host_ecrecover = impure { c: "precompile_ecrecover_hash_sig" } : (hash, y_parity, word, word) -> AddressResult
