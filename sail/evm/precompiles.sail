
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
let FIELD_ELEMENTS_PER_BLOB : word = U256(unsigned(0x0000000000000000000000000000000000000000000000000000000000001000))
let BLS_MODULUS : word = U256(unsigned(0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001))

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
let PRECOMPILE_ADDRESS_1 : address = word_to_address(U256(1))
let PRECOMPILE_ADDRESS_2 : address = word_to_address(U256(2))
let PRECOMPILE_ADDRESS_3 : address = word_to_address(U256(3))
let PRECOMPILE_ADDRESS_4 : address = word_to_address(U256(4))
let PRECOMPILE_ADDRESS_5 : address = word_to_address(U256(5))
let PRECOMPILE_ADDRESS_6 : address = word_to_address(U256(6))
let PRECOMPILE_ADDRESS_7 : address = word_to_address(U256(7))
let PRECOMPILE_ADDRESS_8 : address = word_to_address(U256(8))
let PRECOMPILE_ADDRESS_9 : address = word_to_address(U256(9))
let PRECOMPILE_ADDRESS_10 : address = word_to_address(U256(10))
let PRECOMPILE_ADDRESS_11 : address = word_to_address(U256(11))
let PRECOMPILE_ADDRESS_12 : address = word_to_address(U256(12))
let PRECOMPILE_ADDRESS_13 : address = word_to_address(U256(13))
let PRECOMPILE_ADDRESS_14 : address = word_to_address(U256(14))
let PRECOMPILE_ADDRESS_15 : address = word_to_address(U256(15))
let PRECOMPILE_ADDRESS_16 : address = word_to_address(U256(16))
let PRECOMPILE_ADDRESS_17 : address = word_to_address(U256(17))
let PRECOMPILE_ADDRESS_256 : address = word_to_address(U256(256))

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
    let base = n <= 4;
    let byzantium = (5 <= n) & (n <= 8) & (k_fork >= Byzantium);
    let istanbul = (n == 9) & (k_fork >= Istanbul);
    let kzg = (n == 10) & (k_fork >= Cancun);
    let bls = (11 <= n) & (n <= 17) & (k_fork >= Prague);
    let p256 = (n == 256) & (k_fork >= Osaka);
    base | byzantium | istanbul | kzg | bls | p256
}

/*! Returns the active precompile number represented by `target`, or zero
    when it is an ordinary address at the current fork. */
function precompile_number(bytes : address) -> precompile_selector = {
    let selector : range(0, 2 ^ 16 - 1) =
        unsigned(bytes[18]) * 256 + unsigned(bytes[19]);
    let candidate : precompile_selector = match selector {
        1 => if bytes == PRECOMPILE_ADDRESS_1 then { 1 } else { 0 },
        2 => if bytes == PRECOMPILE_ADDRESS_2 then { 2 } else { 0 },
        3 => if bytes == PRECOMPILE_ADDRESS_3 then { 3 } else { 0 },
        4 => if bytes == PRECOMPILE_ADDRESS_4 then { 4 } else { 0 },
        5 => if bytes == PRECOMPILE_ADDRESS_5 then { 5 } else { 0 },
        6 => if bytes == PRECOMPILE_ADDRESS_6 then { 6 } else { 0 },
        7 => if bytes == PRECOMPILE_ADDRESS_7 then { 7 } else { 0 },
        8 => if bytes == PRECOMPILE_ADDRESS_8 then { 8 } else { 0 },
        9 => if bytes == PRECOMPILE_ADDRESS_9 then { 9 } else { 0 },
        10 => if bytes == PRECOMPILE_ADDRESS_10 then { 10 } else { 0 },
        11 => if bytes == PRECOMPILE_ADDRESS_11 then { 11 } else { 0 },
        12 => if bytes == PRECOMPILE_ADDRESS_12 then { 12 } else { 0 },
        13 => if bytes == PRECOMPILE_ADDRESS_13 then { 13 } else { 0 },
        14 => if bytes == PRECOMPILE_ADDRESS_14 then { 14 } else { 0 },
        15 => if bytes == PRECOMPILE_ADDRESS_15 then { 15 } else { 0 },
        16 => if bytes == PRECOMPILE_ADDRESS_16 then { 16 } else { 0 },
        17 => if bytes == PRECOMPILE_ADDRESS_17 then { 17 } else { 0 },
        256 => if bytes == PRECOMPILE_ADDRESS_256 then { 256 } else { 0 },
        _ => 0,
    };
    if candidate != 0 then {
        let id : precompile_id = candidate;
        if precompile_active_at_fork(id) then { id } else { 0 }
    } else {
        0
    }
}

/*! `ECRECOVER` (0x01): recovers the signer address; any invalid input
    yields a successful call with empty output. */
function run_ecrecover(input : CalldataSlice) -> PrecompileResult = {
    let v = slice_load(input, PRECOMPILE_WORD_OFFSET);
    let valid_v = (v == U256(27)) | (v == U256(28));
    if valid_v then {
        let parity : y_parity = if v == U256(27) then { 0 } else { 1 };
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
        1 => run_ecrecover(input),
        2 => run_sha256(input),
        3 => run_ripemd160(input),
        4 => copied_result(input),
        5 => run_modexp(input),
        6 => accelerator_result(accelerator_bn254_add(input), PRECOMPILE_DOUBLE_WORD_LENGTH),
        7 => accelerator_result(accelerator_bn254_mul(input), PRECOMPILE_DOUBLE_WORD_LENGTH),
        8 => {
            let input_length = calldata_slice_length(input);
            let item_length = BN254_PAIRING_ITEM_LENGTH;
            if input_length == (input_length / item_length) * item_length then {
                pairing_result(accelerator_bn254_pairing(input))
            } else {
                precompile_failure()
            }
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
