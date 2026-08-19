
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
let FIELD_ELEMENTS_PER_BLOB : word = word_from_bits(0x0000000000000000000000000000000000000000000000000000000000001000)
let BLS_MODULUS : word = word_from_bits(0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001)

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
let PRECOMPILE_ADDRESS_1 : address = address_from_nat(1)
let PRECOMPILE_ADDRESS_2 : address = address_from_nat(2)
let PRECOMPILE_ADDRESS_3 : address = address_from_nat(3)
let PRECOMPILE_ADDRESS_4 : address = address_from_nat(4)
let PRECOMPILE_ADDRESS_5 : address = address_from_nat(5)
let PRECOMPILE_ADDRESS_6 : address = address_from_nat(6)
let PRECOMPILE_ADDRESS_7 : address = address_from_nat(7)
let PRECOMPILE_ADDRESS_8 : address = address_from_nat(8)
let PRECOMPILE_ADDRESS_9 : address = address_from_nat(9)
let PRECOMPILE_ADDRESS_10 : address = address_from_nat(10)
let PRECOMPILE_ADDRESS_11 : address = address_from_nat(11)
let PRECOMPILE_ADDRESS_12 : address = address_from_nat(12)
let PRECOMPILE_ADDRESS_13 : address = address_from_nat(13)
let PRECOMPILE_ADDRESS_14 : address = address_from_nat(14)
let PRECOMPILE_ADDRESS_15 : address = address_from_nat(15)
let PRECOMPILE_ADDRESS_16 : address = address_from_nat(16)
let PRECOMPILE_ADDRESS_17 : address = address_from_nat(17)
let PRECOMPILE_ADDRESS_256 : address = address_from_nat(256)

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
        let output = output_buffer_slice(output_len);
        precompile_success(output)
    } else {
        precompile_failure()
    }

/*! `IDENTITY` (0x04): the input, copied through the output buffer. */
function copied_result(data : CalldataSlice) -> PrecompileResult = {
    let output = freeze_output(data);
    let input_length = calldata_slice_length(data);
    if output.len == input_length then {
        precompile_success(output)
    } else {
        precompile_failure()
    }
}

/*! A 32-byte `0`/`1` result word (pairing checks). */
function boolean_result(value : bool) -> PrecompileResult = {
    let encoded_word =
        if value then WORD_ONE else WORD_ZERO;
    let output = output_buffer_word(encoded_word);
    precompile_success(output)
}

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
function precompile_id_if_active(candidate : precompile_id) -> precompile_id = {
    let active = precompile_active_at_fork(candidate);
    if active then {
        candidate
    } else {
        NotPrecompile
    }
}

/*! Maps an address to its active precompile identifier; any other address,
    including one whose precompile is not yet active at the current fork,
    is `NotPrecompile`. */
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
    let v_27 = u256(27);
    let v_28 = u256(28);
    let valid_v = (v == v_27) | (v == v_28);
    if valid_v then {
        let parity : y_parity =
            if v == v_27 then 0 else 1;
        let message_word = slice_load(input, 0);
        let message_hash = word_to_hash(message_word);
        let r = slice_load(input, PRECOMPILE_DOUBLE_WORD_OFFSET);
        let s = slice_load(input, ECRECOVER_S_OFFSET);
        let recovered = ecrecover_addr(message_hash, parity, r, s);
        if recovered.success then {
            let address_word = address_to_word(recovered.address);
            let output = output_buffer_word(address_word);
            precompile_success(output)
        } else {
            precompile_success(EMPTY_OUTPUT_SLICE)
        }
    } else {
        precompile_success(EMPTY_OUTPUT_SLICE)
    }
}

/*! `SHA256` (0x02). */
function run_sha256(input : CalldataSlice) -> PrecompileResult = {
    let digest = sha256(input);
    let digest_word = hash_to_word(digest);
    let output = output_buffer_word(digest_word);
    precompile_success(output)
}

/*! `RIPEMD160` (0x03): 20-byte digest, left-padded to 32. */
function run_ripemd160(input : CalldataSlice) -> PrecompileResult = {
    let success = accelerator_ripemd160(input);
    accelerator_result(success, PRECOMPILE_WORD_LENGTH)
}

/*! `MODEXP` (0x05, EIP-198): arbitrary-precision modular
    exponentiation; a zero-length modulus yields empty output, and inputs
    beyond the accelerator bound fail the call. */
function run_modexp(input : CalldataSlice) -> PrecompileResult = {
    let base_len : word = pc_word(input, 0, 32);
    let exponent_len : word = pc_word(input, 32, 32);
    let modulus_len : word = pc_word(input, 64, 32);
    if modulus_len == 0 then {
        precompile_success(EMPTY_OUTPUT_SLICE)
    } else if   ACCELERATOR_INPUT_MAX
              < base_len
              | ACCELERATOR_INPUT_MAX
              < exponent_len
              | ACCELERATOR_INPUT_MAX
              < modulus_len then {
        precompile_failure()
    } else {
        let bounded_base : range(0, 2097152) = base_len;
        let bounded_exponent : range(0, 2097152) = exponent_len;
        let bounded_modulus : range(0, 2097152) = modulus_len;
        let exponent_end : range(96, 96 + 2 * 2097152) = 96 + bounded_base + bounded_exponent;
        let input_end : range(96, 96 + 3 * 2097152) = exponent_end + bounded_modulus;
        if ACCELERATOR_INPUT_MAX < input_end then {
            precompile_failure()
        } else {
            let success = accelerator_modexp(input, bounded_base, bounded_exponent, bounded_modulus);
            accelerator_result(success, bounded_modulus)
        }
    }
}

/*! Decodes an accelerator pairing result: values below two denote malformed
    input; the low-order parity of a valid value is the pairing outcome. */
function pairing_result(result : pairing_check_result) -> PrecompileResult =
    if result < 2 then {
        precompile_failure()
    } else {
        let parity = tmod_nat(result, 2);
        boolean_result(parity == 1)
    }

/*! `BLAKE2F` (0x09, EIP-152): the compression function; the input must
    be exactly 213 bytes with a 0/1 final-block flag. */
function run_blake2f(input : CalldataSlice) -> PrecompileResult = {
    let final_byte = slice_byte(input, BLAKE2F_FINAL_BLOCK_OFFSET);
    let input_length = calldata_slice_length(input);
    if (input_length != BLAKE2F_INPUT_LENGTH) | ((final_byte != 0x00) & (final_byte != 0x01)) then {
        precompile_failure()
    } else {
        let final_block : y_parity =
            if final_byte == 0x00 then 0 else 1;
        let rounds = pc_blake2_rounds(input);
        let success = accelerator_blake2f(input, rounds, final_block);
        accelerator_result(success, BLAKE2F_OUTPUT_LENGTH)
    }
}

/*! The EIP-4844 versioned-hash binding: the input's claimed hash must
    equal `0x01 ‖ sha256(commitment)[1:]`. */
function kzg_versioned_hash_matches(input : CalldataSlice) -> bool = {
    let commitment = sub_slice(input, KZG_COMMITMENT_OFFSET, KZG_COMMITMENT_LENGTH);
    let commitment_hash = sha256(commitment);
    var expected = commitment_hash;
    expected[0] = 0x01;
    let claimed_word = slice_load(input, 0);
    let claimed_hash = word_to_hash(claimed_word);
    claimed_hash == expected
}

/*! `POINT_EVALUATION` (0x0a, EIP-4844): verifies a KZG proof; success
    returns the field-elements-per-blob and BLS modulus constants. */
function run_kzg_point_evaluation(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    if input_length != KZG_INPUT_LENGTH then {
        precompile_failure()
    } else {
        let versioned_hash_matches = kzg_versioned_hash_matches(input);
        let invalid_versioned_hash = not_bool(versioned_hash_matches);
        if invalid_versioned_hash then {
            precompile_failure()
        } else {
            let valid_proof = accelerator_kzg_point_evaluation(input);
            if valid_proof then {
                let output = output_buffer_words(FIELD_ELEMENTS_PER_BLOB, BLS_MODULUS);
                precompile_success(output)
            } else {
                precompile_failure()
            }
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
function run_bls_g1_add(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    if input_length != BLS_G1_ADD_INPUT_LENGTH then {
        precompile_failure()
    } else {
        let valid_padding = bls_g1_padding(input, 0, BLS_G1_POINT_LENGTH, TWO_COMPONENTS);
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_g1_add(input);
            accelerator_result(success, BLS_G1_POINT_LENGTH)
        }
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
        let valid_padding = bls_g1_padding(input, 0, BLS_G1_MSM_ITEM_LENGTH, pairs);
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_g1_msm(input);
            accelerator_result(success, BLS_G1_POINT_LENGTH)
        }
    }
}

/*! `BLS12_G2ADD` (0x0d, EIP-2537). */
function run_bls_g2_add(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    if input_length != BLS_G2_ADD_INPUT_LENGTH then {
        precompile_failure()
    } else {
        let valid_padding = bls_g2_padding(input, 0, BLS_G2_POINT_LENGTH, TWO_COMPONENTS);
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_g2_add(input);
            accelerator_result(success, BLS_G2_POINT_LENGTH)
        }
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
        let valid_padding = bls_g2_padding(input, 0, BLS_G2_MSM_ITEM_LENGTH, pairs);
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_g2_msm(input);
            accelerator_result(success, BLS_G2_POINT_LENGTH)
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
        let valid_g1_padding = bls_g1_padding(input, 0, BLS_PAIRING_ITEM_LENGTH, pair_count);
        let valid_g2_padding = bls_g2_padding(input, BLS_G2_POINT_OFFSET, BLS_PAIRING_ITEM_LENGTH, pair_count);
        if valid_g1_padding & valid_g2_padding then {
            let result = accelerator_bls_pairing(input);
            pairing_result(result)
        } else {
            precompile_failure()
        }
    }
}

/*! `BLS12_MAP_FP_TO_G1` (0x10, EIP-2537). */
function run_bls_map_fp_to_g1(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    if input_length != BLS_PADDED_FIELD_LENGTH then {
        precompile_failure()
    } else {
        let valid_padding = slice_strided_zero(input, 0, BLS_PADDED_FIELD_LENGTH, BLS_FIELD_PADDING_LENGTH, 1);
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_map_fp_to_g1(input);
            accelerator_result(success, BLS_G1_POINT_LENGTH)
        }
    }
}

/*! `BLS12_MAP_FP2_TO_G2` (0x11, EIP-2537). */
function run_bls_map_fp2_to_g2(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    if input_length != BLS_G1_POINT_LENGTH then {
        precompile_failure()
    } else {
        let valid_padding = slice_strided_zero(
            input,
            0,
            BLS_PADDED_FIELD_LENGTH,
            BLS_FIELD_PADDING_LENGTH,
            TWO_COMPONENTS,
        );
        let invalid_padding = not_bool(valid_padding);
        if invalid_padding then {
            precompile_failure()
        } else {
            let success = accelerator_bls_map_fp2_to_g2(input);
            accelerator_result(success, BLS_G2_POINT_LENGTH)
        }
    }
}

/*! `P256VERIFY` (0x100, EIP-7951): every malformed or invalid signature
    is a successful call with empty output; only a valid signature
    returns the word one. */
function run_p256_verify(input : CalldataSlice) -> PrecompileResult = {
    let input_length = calldata_slice_length(input);
    let verified =
        if input_length == P256_INPUT_LENGTH then accelerator_p256_verify(input) else false;
    if verified then {
        let output = output_buffer_word(WORD_ONE);
        precompile_success(output)
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
        Bn254Add => {
            let success = accelerator_bn254_add(input);
            accelerator_result(success, PRECOMPILE_DOUBLE_WORD_LENGTH)
        },
        Bn254Mul => {
            let success = accelerator_bn254_mul(input);
            accelerator_result(success, PRECOMPILE_DOUBLE_WORD_LENGTH)
        },
        Bn254Pairing => {
            let input_length = calldata_slice_length(input);
            let item_length = BN254_PAIRING_ITEM_LENGTH;
            if input_length == (input_length / item_length) * item_length then {
                let result = accelerator_bn254_pairing(input);
                pairing_result(result)
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
