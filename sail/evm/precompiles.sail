/*md
# Precompiled contracts

EVM precompile semantics (YP Appendix E and the precompile EIPs). Sail
selects the active address, validates the Ethereum input encoding, and
returns the EVM-visible output; the host functions called here are only
raw zkVM cryptographic accelerators.
*/

let ACCELERATOR_INPUT_MAX : int = 2097152
let FIELD_ELEMENTS_PER_BLOB : word = 0x0000000000000000000000000000000000000000000000000000000000001000
let BLS_MODULUS : word = 0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001

/*! A precompile's outcome: success and the EVM-visible output bytes.
    Failure consumes the frame's gas like any exceptional call. */
struct PrecompileResult = {
  success : bool,
  output  : ByteSlice
}

/*! A successful result carrying `output`. */
function precompile_success(output : ByteSlice) -> PrecompileResult =
    struct { success = true, output = output }

/*! The failed result (empty output; the call reports failure). */
function precompile_failure() -> PrecompileResult =
    struct { success = false, output = EMPTY_SLICE }

/*! Adapts an accelerator's boolean outcome: on success the output is
    the accelerator's write into the output arena. */
function accelerator_result(success : bool, output_len : int) -> PrecompileResult =
    if success then {
        precompile_success(output_buffer_slice(output_len))
    } else {
        precompile_failure()
    }

/*! `IDENTITY` (0x04): the input, copied through the output buffer. */
function copied_result(data : ByteSlice) -> PrecompileResult = {
    let output = freeze_output(data);
    if unsigned(output.len) == unsigned(data.len) then {
        precompile_success(output)
    } else {
        precompile_failure()
    }
}

/*! A 32-byte `0`/`1` result word (pairing checks). */
function boolean_result(value : bool) -> PrecompileResult =
    precompile_success(output_buffer_word(if value then { WORD_ONE } else { WORD_ZERO }))

/*! Whether address `n` is an active precompile at the current fork:
    1–4 always; 5–8 from Byzantium; 9 from Istanbul; 10 from Cancun
    (EIP-4844); 11–17 from Prague (EIP-2537); 0x100 from Osaka
    (EIP-7951). */
function is_precompile(n : int) -> bool = {
    let base = (n >= 1) & (n <= 4);
    let byzantium = (n >= 5) & (n <= 8) & (k_fork >= Byzantium);
    let istanbul = (n == 9) & (k_fork >= Istanbul);
    let kzg = (n == 10) & (k_fork >= Cancun);
    let bls = (n >= 11) & (n <= 17) & (k_fork >= Prague);
    let p256 = (n == 256) & (k_fork >= Osaka);
    base | byzantium | istanbul | kzg | bls | p256
}

/*! `ECRECOVER` (0x01): recovers the signer address; any invalid input
    yields a successful call with empty output. */
function run_ecrecover(input : ByteSlice) -> PrecompileResult = {
    let v = slice_load(input, 32);
    let valid_v = (v == word_of_int(27)) | (v == word_of_int(28));
    if valid_v then {
        let (recovered, address) = ecrecover_addr(
            slice_load(input, 0),
            unsigned(v) - 27,
            slice_load(input, 64),
            slice_load(input, 96),
        );
        if recovered then {
            precompile_success(output_buffer_word(address_to_word(address)))
        } else {
            precompile_success(EMPTY_SLICE)
        }
    } else {
        precompile_success(EMPTY_SLICE)
    }
}

/*! `SHA256` (0x02). */
function run_sha256(input : ByteSlice) -> PrecompileResult =
    precompile_success(output_buffer_word(sha256([|BytesSlice(input)|])))

/*! `RIPEMD160` (0x03): 20-byte digest, left-padded to 32. */
function run_ripemd160(input : ByteSlice) -> PrecompileResult =
    accelerator_result(accelerator_ripemd160(input), 32)

/*! `MODEXP` (0x05, EIP-198): arbitrary-precision modular
    exponentiation; a zero-length modulus yields empty output, and inputs
    beyond the accelerator bound fail the call. */
function run_modexp(input : ByteSlice) -> PrecompileResult = {
    let base_len = pc_be(input, 0, 32);
    let exponent_len = pc_be(input, 32, 32);
    let modulus_len = pc_be(input, 64, 32);
    if modulus_len == 0 then {
        precompile_success(EMPTY_SLICE)
    } else {
        if   (base_len > ACCELERATOR_INPUT_MAX)
           | (exponent_len > ACCELERATOR_INPUT_MAX)
           | (modulus_len > ACCELERATOR_INPUT_MAX)
           | (96 + base_len + exponent_len + modulus_len > ACCELERATOR_INPUT_MAX) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_modexp(input, base_len, exponent_len, modulus_len), modulus_len)
        }
    }
}

/*! Decodes an accelerator pairing result: bit 1 is validity of the
    input, bit 0 the pairing outcome. */
function pairing_result(result : bits(2)) -> PrecompileResult =
    if result[1] == bitzero then {
        precompile_failure()
    } else {
        boolean_result(result[0] == bitone)
    }

/*! `BLAKE2F` (0x09, EIP-152): the compression function; the input must
    be exactly 213 bytes with a 0/1 final-block flag. */
function run_blake2f(input : ByteSlice) -> PrecompileResult = {
    let final_block = unsigned(slice_byte(input, 212));
    if (unsigned(input.len) != 213) | ((final_block != 0) & (final_block != 1)) then {
        precompile_failure()
    } else {
        accelerator_result(accelerator_blake2f(input, pc_be(input, 0, 4), final_block), 64)
    }
}

/*! The EIP-4844 versioned-hash binding: the input's claimed hash must
    equal `0x01 ‖ sha256(commitment)[1:]`. */
function kzg_versioned_hash_matches(input : ByteSlice) -> bool = {
    let commitment_hash = sha256([|BytesSlice(sub_slice(input, 96, 48))|]);
    let expected : word = append(0x01, commitment_hash[247 .. 0]);
    slice_load(input, 0) == expected
}

/*! `POINT_EVALUATION` (0x0a, EIP-4844): verifies a KZG proof; success
    returns the field-elements-per-blob and BLS modulus constants. */
function run_kzg_point_evaluation(input : ByteSlice) -> PrecompileResult =
    if unsigned(input.len) != 192 then {
        precompile_failure()
    } else {
        if not_bool(kzg_versioned_hash_matches(input)) then {
            precompile_failure()
        } else {
            if accelerator_kzg_point_evaluation(input) then {
                precompile_success(output_buffer_words(FIELD_ELEMENTS_PER_BLOB, BLS_MODULUS))
            } else {
                precompile_failure()
            }
        }
    }

/*! The EIP-2537 padding rule for G1 points: every 48-byte Fp value is
    padded to 64 bytes with sixteen leading zero bytes, checked with one
    bulk source predicate per field position. */
function bls_g1_padding(input : ByteSlice, base : int, stride : int, count : int) -> bool =
    slice_strided_zero(input, base, stride, 16, count) & slice_strided_zero(input, base + 64, stride, 16, count)

/*! The EIP-2537 padding rule for G2 points (four Fp components). */
function bls_g2_padding(input : ByteSlice, base : int, stride : int, count : int) -> bool =
      slice_strided_zero(input, base, stride, 16, count)
    & slice_strided_zero(input, base + 64, stride, 16, count)
    & slice_strided_zero(input, base + 128, stride, 16, count)
    & slice_strided_zero(input, base + 192, stride, 16, count)

/*! `BLS12_G1ADD` (0x0b, EIP-2537). */
function run_bls_g1_add(input : ByteSlice) -> PrecompileResult =
    if unsigned(input.len) != 256 then {
        precompile_failure()
    } else {
        if not_bool(bls_g1_padding(input, 0, 128, 2)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g1_add(input), 128)
        }
    }

/*! `BLS12_G1MSM` (0x0c, EIP-2537): input is `k` 160-byte pairs. */
function run_bls_g1_msm(input : ByteSlice) -> PrecompileResult = {
    let len = unsigned(input.len);
    if (len == 0) | (emod_int(len, 160) != 0) then {
        precompile_failure()
    } else {
        if not_bool(bls_g1_padding(input, 0, 160, ediv_int(len, 160))) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g1_msm(input), 128)
        }
    }
}

/*! `BLS12_G2ADD` (0x0d, EIP-2537). */
function run_bls_g2_add(input : ByteSlice) -> PrecompileResult =
    if unsigned(input.len) != 512 then {
        precompile_failure()
    } else {
        if not_bool(bls_g2_padding(input, 0, 256, 2)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g2_add(input), 256)
        }
    }

/*! `BLS12_G2MSM` (0x0e, EIP-2537): input is `k` 288-byte pairs. */
function run_bls_g2_msm(input : ByteSlice) -> PrecompileResult = {
    let len = unsigned(input.len);
    if (len == 0) | (emod_int(len, 288) != 0) then {
        precompile_failure()
    } else {
        if not_bool(bls_g2_padding(input, 0, 288, ediv_int(len, 288))) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g2_msm(input), 256)
        }
    }
}

/*! `BLS12_PAIRING_CHECK` (0x0f, EIP-2537): input is `k` 384-byte
    G1×G2 pairs. */
function run_bls_pairing(input : ByteSlice) -> PrecompileResult = {
    let len = unsigned(input.len);
    if (len == 0) | (emod_int(len, 384) != 0) then {
        precompile_failure()
    } else {
        let count = ediv_int(len, 384);
        if bls_g1_padding(input, 0, 384, count) & bls_g2_padding(input, 128, 384, count) then {
            pairing_result(accelerator_bls_pairing(input))
        } else {
            precompile_failure()
        }
    }
}

/*! `BLS12_MAP_FP_TO_G1` (0x10, EIP-2537). */
function run_bls_map_fp_to_g1(input : ByteSlice) -> PrecompileResult =
    if unsigned(input.len) != 64 then {
        precompile_failure()
    } else {
        if not_bool(slice_strided_zero(input, 0, 64, 16, 1)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_map_fp_to_g1(input), 128)
        }
    }

/*! `BLS12_MAP_FP2_TO_G2` (0x11, EIP-2537). */
function run_bls_map_fp2_to_g2(input : ByteSlice) -> PrecompileResult =
    if unsigned(input.len) != 128 then {
        precompile_failure()
    } else {
        if not_bool(slice_strided_zero(input, 0, 64, 16, 2)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_map_fp2_to_g2(input), 256)
        }
    }

/*! `P256VERIFY` (0x100, EIP-7951): every malformed or invalid signature
    is a successful call with empty output; only a valid signature
    returns the word one. */
function run_p256_verify(input : ByteSlice) -> PrecompileResult = {
    let verified = if unsigned(input.len) == 160 then {
            accelerator_p256_verify(input)
        } else {
            false
        };
    if verified then {
        precompile_success(output_buffer_word(WORD_ONE))
    } else {
        precompile_success(EMPTY_SLICE)
    }
}

/*! The precompile dispatch: address to implementation. Gas has already
    been charged by the caller ([precompile_gas][]). */
function run_precompile_slice(num : int, input : ByteSlice) -> PrecompileResult =
    match num {
        1 => run_ecrecover(input),
        2 => run_sha256(input),
        3 => run_ripemd160(input),
        4 => copied_result(input),
        5 => run_modexp(input),
        6 => accelerator_result(accelerator_bn254_add(input), 64),
        7 => accelerator_result(accelerator_bn254_mul(input), 64),
        8 => if emod_int(unsigned(input.len), 192) == 0 then {
            pairing_result(accelerator_bn254_pairing(input))
        } else {
            precompile_failure()
        },
        9 => run_blake2f(input),
        10 => run_kzg_point_evaluation(input),
        11 => run_bls_g1_add(input),
        12 => run_bls_g1_msm(input),
        13 => run_bls_g2_add(input),
        14 => run_bls_g2_msm(input),
        15 => run_bls_pairing(input),
        16 => run_bls_map_fp_to_g1(input),
        17 => run_bls_map_fp2_to_g2(input),
        256 => run_p256_verify(input),
        _ => precompile_failure(),
    }
