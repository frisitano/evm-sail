
/*md
# Precompiled contracts

EVM precompile semantics (YP Appendix E and the precompile EIPs). Sail
selects the active address, validates the Ethereum input encoding, and
returns the EVM-visible output; the host functions called here are only
raw zkVM cryptographic accelerators.

## Constants

The fixed offsets and lengths describe the canonical byte layouts accepted
by each precompile; the field modulus and blob element count validate their
respective algebraic inputs.
*/
let ACCELERATOR_INPUT_MAX : int(2097152) = 2097152
let FIELD_ELEMENTS_PER_BLOB : word = u256(unsigned(0x0000000000000000000000000000000000000000000000000000000000001000))
let BLS_MODULUS : word = u256(unsigned(0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001))

let BLAKE2F_INPUT_LENGTH : int(213) = 213
let BLAKE2F_FINAL_BLOCK_OFFSET : int(212) = 212
let BLAKE2F_OUTPUT_LENGTH : int(64) = DOUBLE_WORD_BYTE_LENGTH

let KZG_INPUT_LENGTH : int(192) = 192
let KZG_COMMITMENT_OFFSET : int(96) = 96
let KZG_COMMITMENT_LENGTH : int(48) = 48

let BLS_FIELD_PADDING_LENGTH : int(16) = 16
let BLS_PADDED_FIELD_LENGTH : int(64) = DOUBLE_WORD_BYTE_LENGTH
let BLS_G1_POINT_LENGTH : int(128) = 128
let BLS_G2_POINT_LENGTH : int(256) = 256
let BLS_G2_FINAL_FIELD_OFFSET : int(192) = 192
let BLS_G1_ADD_INPUT_LENGTH : int(256) = 256
let BLS_G1_MSM_ITEM_LENGTH : int(160) = 160
let BLS_G2_ADD_INPUT_LENGTH : int(512) = 512
let BLS_G2_MSM_ITEM_LENGTH : int(288) = 288
let BLS_PAIRING_ITEM_LENGTH : int(384) = 384

let P256_INPUT_LENGTH : int(160) = 160
let BN254_PAIRING_ITEM_LENGTH : int(192) = 192
let PRECOMPILE_WORD_LENGTH : int(32) = WORD_BYTE_LENGTH
let PRECOMPILE_DOUBLE_WORD_LENGTH : int(64) = DOUBLE_WORD_BYTE_LENGTH
let PRECOMPILE_WORD_OFFSET : int(32) = 32
let PRECOMPILE_DOUBLE_WORD_OFFSET : int(64) = 64
let ECRECOVER_S_OFFSET : source_pointer = 96
let TWO_COMPONENTS : int(2) = 2
let BLS_G2_POINT_OFFSET : int(128) = 128

/* Full-width address constants keep precompile recognition as ordinary
 * address equality. Construct them through the canonical word conversion so
 * their representation is identical to transaction and CALL targets. */
let PRECOMPILE_ADDRESS_1 : address = word_to_address(u256(1))
let PRECOMPILE_ADDRESS_2 : address = word_to_address(u256(2))
let PRECOMPILE_ADDRESS_3 : address = word_to_address(u256(3))
let PRECOMPILE_ADDRESS_4 : address = word_to_address(u256(4))
let PRECOMPILE_ADDRESS_5 : address = word_to_address(u256(5))
let PRECOMPILE_ADDRESS_6 : address = word_to_address(u256(6))
let PRECOMPILE_ADDRESS_7 : address = word_to_address(u256(7))
let PRECOMPILE_ADDRESS_8 : address = word_to_address(u256(8))
let PRECOMPILE_ADDRESS_9 : address = word_to_address(u256(9))
let PRECOMPILE_ADDRESS_10 : address = word_to_address(u256(10))
let PRECOMPILE_ADDRESS_11 : address = word_to_address(u256(11))
let PRECOMPILE_ADDRESS_12 : address = word_to_address(u256(12))
let PRECOMPILE_ADDRESS_13 : address = word_to_address(u256(13))
let PRECOMPILE_ADDRESS_14 : address = word_to_address(u256(14))
let PRECOMPILE_ADDRESS_15 : address = word_to_address(u256(15))
let PRECOMPILE_ADDRESS_16 : address = word_to_address(u256(16))
let PRECOMPILE_ADDRESS_17 : address = word_to_address(u256(17))
let PRECOMPILE_ADDRESS_256 : address = word_to_address(u256(256))

/*! A precompile's outcome: success and the EVM-visible output bytes.
    Failure consumes the frame's gas like any exceptional call. */
struct PrecompileResult = {
  success : bool,
  output  : OutputSlice
}

/*! A successful result carrying `output`. */
function precompile_success(output : OutputSlice) -> PrecompileResult =
    struct { success = true, output = output }

/*! The failed result (empty output; the call reports failure). */
function precompile_failure() -> PrecompileResult =
    struct { success = false, output = EMPTY_OUTPUT_SLICE }

/*! Adapts an accelerator's boolean outcome: on success the output is
    the accelerator's write into the output arena. */
val accelerator_result : forall 'output_len, source_valid_length('output_len). (bool, int('output_len)) -> PrecompileResult
function accelerator_result(success, output_len) =
    if success then {
        precompile_success(output_buffer_slice(output_len))
    } else {
        precompile_failure()
    }

/*! `IDENTITY` (0x04): the input, copied through the output buffer. */
function copied_result(data : CalldataSlice) -> PrecompileResult = {
    let output = freeze_output(data);
    if output.len == calldata_slice_length(data) then {
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
function precompile_active_at_fork(n : precompile_id) -> bool = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    match n {
        NotPrecompile => false,
        Ecrecover => true,
        Sha256 => true,
        Ripemd160 => true,
        Identity => true,
        Modexp => profile.fork >= Byzantium,
        Bn254Add => profile.fork >= Byzantium,
        Bn254Mul => profile.fork >= Byzantium,
        Bn254Pairing => profile.fork >= Byzantium,
        Blake2f => profile.fork >= Istanbul,
        KzgPointEvaluation => profile.fork >= Cancun,
        BlsG1Add => profile.fork >= Prague,
        BlsG1Msm => profile.fork >= Prague,
        BlsG2Add => profile.fork >= Prague,
        BlsG2Msm => profile.fork >= Prague,
        BlsPairing => profile.fork >= Prague,
        BlsMapFpToG1 => profile.fork >= Prague,
        BlsMapFp2ToG2 => profile.fork >= Prague,
        P256Verify => profile.fork >= Osaka,
    }
}

/*! Returns the active precompile represented by `target`, or
    `NotPrecompile` when it is an ordinary address at the current fork. */
function precompile_id_if_active(candidate : precompile_id) -> precompile_id =
    if precompile_active_at_fork(candidate) then {
        candidate
    } else {
        NotPrecompile
    }

function precompile_id_for_address(bytes : address) -> precompile_id = {
    match bytes {
        _ if bytes == PRECOMPILE_ADDRESS_1 => precompile_id_if_active(Ecrecover),
        _ if bytes == PRECOMPILE_ADDRESS_2 => precompile_id_if_active(Sha256),
        _ if bytes == PRECOMPILE_ADDRESS_3 => precompile_id_if_active(Ripemd160),
        _ if bytes == PRECOMPILE_ADDRESS_4 => precompile_id_if_active(Identity),
        _ if bytes == PRECOMPILE_ADDRESS_5 => precompile_id_if_active(Modexp),
        _ if bytes == PRECOMPILE_ADDRESS_6 => precompile_id_if_active(Bn254Add),
        _ if bytes == PRECOMPILE_ADDRESS_7 => precompile_id_if_active(Bn254Mul),
        _ if bytes == PRECOMPILE_ADDRESS_8 => precompile_id_if_active(Bn254Pairing),
        _ if bytes == PRECOMPILE_ADDRESS_9 => precompile_id_if_active(Blake2f),
        _ if bytes == PRECOMPILE_ADDRESS_10 => precompile_id_if_active(KzgPointEvaluation),
        _ if bytes == PRECOMPILE_ADDRESS_11 => precompile_id_if_active(BlsG1Add),
        _ if bytes == PRECOMPILE_ADDRESS_12 => precompile_id_if_active(BlsG1Msm),
        _ if bytes == PRECOMPILE_ADDRESS_13 => precompile_id_if_active(BlsG2Add),
        _ if bytes == PRECOMPILE_ADDRESS_14 => precompile_id_if_active(BlsG2Msm),
        _ if bytes == PRECOMPILE_ADDRESS_15 => precompile_id_if_active(BlsPairing),
        _ if bytes == PRECOMPILE_ADDRESS_16 => precompile_id_if_active(BlsMapFpToG1),
        _ if bytes == PRECOMPILE_ADDRESS_17 => precompile_id_if_active(BlsMapFp2ToG2),
        _ if bytes == PRECOMPILE_ADDRESS_256 => precompile_id_if_active(P256Verify),
        _ => NotPrecompile,
    }
}

/*! `ECRECOVER` (0x01): recovers the signer address; any invalid input
    yields a successful call with empty output. */
function run_ecrecover(input : CalldataSlice) -> PrecompileResult = {
    let v = slice_load(input, PRECOMPILE_WORD_OFFSET);
    let valid_v = (v == u256(27)) | (v == u256(28));
    if valid_v then {
        let parity : y_parity = if v == u256(27) then { 0 } else { 1 };
        let (recovered, address) = ecrecover_addr(
            word_to_hash(slice_load(input, 0)),
            parity,
            slice_load(input, PRECOMPILE_DOUBLE_WORD_OFFSET),
            slice_load(input, ECRECOVER_S_OFFSET),
        );
        if recovered then {
            precompile_success(output_buffer_word(address_to_word(address)))
        } else {
            precompile_success(EMPTY_OUTPUT_SLICE)
        }
    } else {
        precompile_success(EMPTY_OUTPUT_SLICE)
    }
}

/*! `SHA256` (0x02). */
function run_sha256(input : CalldataSlice) -> PrecompileResult =
    precompile_success(output_buffer_word(hash_to_word(sha256(input))))

/*! `RIPEMD160` (0x03): 20-byte digest, left-padded to 32. */
function run_ripemd160(input : CalldataSlice) -> PrecompileResult =
    accelerator_result(accelerator_ripemd160(input), PRECOMPILE_WORD_LENGTH)

/*! `MODEXP` (0x05, EIP-198): arbitrary-precision modular
    exponentiation; a zero-length modulus yields empty output, and inputs
    beyond the accelerator bound fail the call. */
function run_modexp(input : CalldataSlice) -> PrecompileResult = {
    let base_len : word = pc_word(input, 0, 32);
    let exponent_len : word = pc_word(input, 32, 32);
    let modulus_len : word = pc_word(input, 64, 32);
    if modulus_len == 0 then {
        precompile_success(EMPTY_OUTPUT_SLICE)
    } else {
        if ACCELERATOR_INPUT_MAX < base_len | ACCELERATOR_INPUT_MAX < exponent_len | ACCELERATOR_INPUT_MAX < modulus_len then {
            precompile_failure()
        } else {
            let bounded_base : range(0, 2097152) = base_len;
            let bounded_exponent : range(0, 2097152) = exponent_len;
            let bounded_modulus : range(0, 2097152) = modulus_len;
            let input_end : range(96, 96 + 3 * 2097152) = 96 + bounded_base + bounded_exponent + bounded_modulus;
            if ACCELERATOR_INPUT_MAX < input_end then {
                precompile_failure()
            } else {
                accelerator_result(
                    accelerator_modexp(input, bounded_base, bounded_exponent, bounded_modulus),
                    bounded_modulus,
                )
            }
        }
    }
}

/*! Decodes an accelerator pairing result: values below two denote malformed
    input; the low-order parity of a valid value is the pairing outcome. */
function pairing_result(result : pairing_check_result) -> PrecompileResult =
    if result < 2 then {
        precompile_failure()
    } else {
        boolean_result(tmod_nat(result, 2) == 1)
    }

/*! `BLAKE2F` (0x09, EIP-152): the compression function; the input must
    be exactly 213 bytes with a 0/1 final-block flag. */
function run_blake2f(input : CalldataSlice) -> PrecompileResult = {
    let final_byte = slice_byte(input, BLAKE2F_FINAL_BLOCK_OFFSET);
    if (calldata_slice_length(input) != BLAKE2F_INPUT_LENGTH) | ((final_byte != 0x00) & (final_byte != 0x01)) then {
        precompile_failure()
    } else {
        let final_block : y_parity = if final_byte == 0x00 then { 0 } else { 1 };
        accelerator_result(accelerator_blake2f(input, pc_blake2_rounds(input), final_block), BLAKE2F_OUTPUT_LENGTH)
    }
}

/*! The EIP-4844 versioned-hash binding: the input's claimed hash must
    equal `0x01 ‖ sha256(commitment)[1:]`. */
function kzg_versioned_hash_matches(input : CalldataSlice) -> bool = {
    let commitment_hash = sha256(sub_slice(input, KZG_COMMITMENT_OFFSET, KZG_COMMITMENT_LENGTH));
    var expected = commitment_hash;
    expected[0] = 0x01;
    word_to_hash(slice_load(input, 0)) == expected
}

/*! `POINT_EVALUATION` (0x0a, EIP-4844): verifies a KZG proof; success
    returns the field-elements-per-blob and BLS modulus constants. */
function run_kzg_point_evaluation(input : CalldataSlice) -> PrecompileResult =
    if calldata_slice_length(input) != KZG_INPUT_LENGTH then {
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
val bls_g1_padding : forall 'base 'stride 'count,
                       source_valid_range('base, 64) & source_valid_length('stride) & source_valid_length('count). (
        CalldataSlice,
        int('base),
        int('stride),
        int('count),
    ) -> bool
function bls_g1_padding(input, base, stride, count) = {
    slice_strided_zero(input, base, stride, BLS_FIELD_PADDING_LENGTH, count) &
        slice_strided_zero(input, base + BLS_PADDED_FIELD_LENGTH, stride, BLS_FIELD_PADDING_LENGTH, count)
}

/*! The EIP-2537 padding rule for G2 points (four Fp components). */
val bls_g2_padding : forall 'base 'stride 'count,
                       source_valid_range('base, 192) & source_valid_length('stride) & source_valid_length('count). (
        CalldataSlice,
        int('base),
        int('stride),
        int('count),
    ) -> bool
function bls_g2_padding(input, base, stride, count) = {
      slice_strided_zero(input, base, stride, BLS_FIELD_PADDING_LENGTH, count)
    & slice_strided_zero(input, base + BLS_PADDED_FIELD_LENGTH, stride, BLS_FIELD_PADDING_LENGTH, count)
    & slice_strided_zero(input, base + BLS_G1_POINT_LENGTH, stride, BLS_FIELD_PADDING_LENGTH, count)
    & slice_strided_zero(input, base + BLS_G2_FINAL_FIELD_OFFSET, stride, BLS_FIELD_PADDING_LENGTH, count)
}

/*! `BLS12_G1ADD` (0x0b, EIP-2537). */
function run_bls_g1_add(input : CalldataSlice) -> PrecompileResult =
    if calldata_slice_length(input) != BLS_G1_ADD_INPUT_LENGTH then {
        precompile_failure()
    } else {
        if not_bool(bls_g1_padding(input, 0, BLS_G1_POINT_LENGTH, TWO_COMPONENTS)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g1_add(input), BLS_G1_POINT_LENGTH)
        }
    }

/*! `BLS12_G1MSM` (0x0c, EIP-2537): input is `k` 160-byte pairs. */
function run_bls_g1_msm(input : CalldataSlice) -> PrecompileResult = {
    let length = calldata_slice_length(input);
    let item_length = BLS_G1_MSM_ITEM_LENGTH;
    let pairs = length / item_length;
    if (length == 0) | (length != pairs * item_length) then {
        precompile_failure()
    } else {
        if not_bool(bls_g1_padding(input, 0, BLS_G1_MSM_ITEM_LENGTH, pairs)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g1_msm(input), BLS_G1_POINT_LENGTH)
        }
    }
}

/*! `BLS12_G2ADD` (0x0d, EIP-2537). */
function run_bls_g2_add(input : CalldataSlice) -> PrecompileResult =
    if calldata_slice_length(input) != BLS_G2_ADD_INPUT_LENGTH then {
        precompile_failure()
    } else {
        if not_bool(bls_g2_padding(input, 0, BLS_G2_POINT_LENGTH, TWO_COMPONENTS)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g2_add(input), BLS_G2_POINT_LENGTH)
        }
    }

/*! `BLS12_G2MSM` (0x0e, EIP-2537): input is `k` 288-byte pairs. */
function run_bls_g2_msm(input : CalldataSlice) -> PrecompileResult = {
    let length = calldata_slice_length(input);
    let item_length = BLS_G2_MSM_ITEM_LENGTH;
    let pairs = length / item_length;
    if (length == 0) | (length != pairs * item_length) then {
        precompile_failure()
    } else {
        if not_bool(bls_g2_padding(input, 0, BLS_G2_MSM_ITEM_LENGTH, pairs)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_g2_msm(input), BLS_G2_POINT_LENGTH)
        }
    }
}

/*! `BLS12_PAIRING_CHECK` (0x0f, EIP-2537): input is `k` 384-byte
    G1×G2 pairs. */
function run_bls_pairing(input : CalldataSlice) -> PrecompileResult = {
    let length = calldata_slice_length(input);
    let item_length = BLS_PAIRING_ITEM_LENGTH;
    let pairs = length / item_length;
    if (length == 0) | (length != pairs * item_length) then {
        precompile_failure()
    } else {
        let pair_count : source_length = pairs;
        if bls_g1_padding(input, 0, BLS_PAIRING_ITEM_LENGTH, pair_count) &
            bls_g2_padding(input, BLS_G2_POINT_OFFSET, BLS_PAIRING_ITEM_LENGTH, pair_count) then {
            pairing_result(accelerator_bls_pairing(input))
        } else {
            precompile_failure()
        }
    }
}

/*! `BLS12_MAP_FP_TO_G1` (0x10, EIP-2537). */
function run_bls_map_fp_to_g1(input : CalldataSlice) -> PrecompileResult =
    if calldata_slice_length(input) != BLS_PADDED_FIELD_LENGTH then {
        precompile_failure()
    } else {
        if not_bool(slice_strided_zero(input, 0, BLS_PADDED_FIELD_LENGTH, BLS_FIELD_PADDING_LENGTH, 1)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_map_fp_to_g1(input), BLS_G1_POINT_LENGTH)
        }
    }

/*! `BLS12_MAP_FP2_TO_G2` (0x11, EIP-2537). */
function run_bls_map_fp2_to_g2(input : CalldataSlice) -> PrecompileResult =
    if calldata_slice_length(input) != BLS_G1_POINT_LENGTH then {
        precompile_failure()
    } else {
        if not_bool(slice_strided_zero(input, 0, BLS_PADDED_FIELD_LENGTH, BLS_FIELD_PADDING_LENGTH, TWO_COMPONENTS)) then {
            precompile_failure()
        } else {
            accelerator_result(accelerator_bls_map_fp2_to_g2(input), BLS_G2_POINT_LENGTH)
        }
    }

/*! `P256VERIFY` (0x100, EIP-7951): every malformed or invalid signature
    is a successful call with empty output; only a valid signature
    returns the word one. */
function run_p256_verify(input : CalldataSlice) -> PrecompileResult = {
    let verified = if calldata_slice_length(input) == P256_INPUT_LENGTH then {
            accelerator_p256_verify(input)
        } else {
            false
        };
    if verified then {
        precompile_success(output_buffer_word(WORD_ONE))
    } else {
        precompile_success(EMPTY_OUTPUT_SLICE)
    }
}

/*! The precompile dispatch: address to implementation. Gas has already
    been charged by the caller ([precompile_gas][]). */
function run_precompile_slice(num : precompile_id, input : CalldataSlice) -> PrecompileResult =
    match num {
        NotPrecompile => precompile_failure(),
        Ecrecover => run_ecrecover(input),
        Sha256 => run_sha256(input),
        Ripemd160 => run_ripemd160(input),
        Identity => copied_result(input),
        Modexp => run_modexp(input),
        Bn254Add => accelerator_result(accelerator_bn254_add(input), PRECOMPILE_DOUBLE_WORD_LENGTH),
        Bn254Mul => accelerator_result(accelerator_bn254_mul(input), PRECOMPILE_DOUBLE_WORD_LENGTH),
        Bn254Pairing => {
            let input_length = calldata_slice_length(input);
            let item_length = BN254_PAIRING_ITEM_LENGTH;
            if input_length == (input_length / item_length) * item_length then {
                pairing_result(accelerator_bn254_pairing(input))
            } else {
                precompile_failure()
            }
        },
        Blake2f => run_blake2f(input),
        KzgPointEvaluation => run_kzg_point_evaluation(input),
        BlsG1Add => run_bls_g1_add(input),
        BlsG1Msm => run_bls_g1_msm(input),
        BlsG2Add => run_bls_g2_add(input),
        BlsG2Msm => run_bls_g2_msm(input),
        BlsPairing => run_bls_pairing(input),
        BlsMapFpToG1 => run_bls_map_fp_to_g1(input),
        BlsMapFp2ToG2 => run_bls_map_fp2_to_g2(input),
        P256Verify => run_p256_verify(input),
    }
