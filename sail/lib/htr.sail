
/*md
# SSZ hash_tree_root

Consensus-layer merkleization (SHA-256). The standard specification follows
the recursive SSZ definition directly: a subtree is either its leaf, the
precomputed zero root at that depth, or the hash of its two children. Source
references are indexed only when recursion reaches a live leaf, so no leaf
list or explicit Merkle frontier is materialized.
*/
/*! The largest Merkle depth required by the supported execution-layer SSZ
    schemas. ByteList[2^30] is the widest one, with 2^25 chunks. */
type htr_depth = range(0, 25)
/*! A leaf count in any supported execution-layer SSZ Merkle tree. */
type htr_leaf_count = range(0, 33554432)

/*md
## Constants

The limits and byte offsets below are the SSZ dimensions of execution-layer
requests and their constituent fields.
*/
let HTR_BYTE_LIST_LIMIT : int(1073741824) = 1073741824
let HTR_DEPOSIT_LENGTH : int(192) = 192
let HTR_DEPOSIT_PUBKEY : int(0) = 0
let HTR_DEPOSIT_PUBKEY_LENGTH : int(48) = 48
let HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS : int(48) = 48
let HTR_DEPOSIT_AMOUNT : int(80) = 80
let HTR_DEPOSIT_SIGNATURE : int(88) = 88
let HTR_DEPOSIT_SIGNATURE_LENGTH : int(96) = 96
let HTR_DEPOSIT_INDEX : int(184) = 184
let HTR_WITHDRAWAL_REQUEST_LENGTH : int(76) = 76
let HTR_REQUEST_SOURCE_ADDRESS : int(0) = 0
let HTR_REQUEST_SOURCE_PUBKEY : int(20) = 20
let HTR_REQUEST_PUBKEY_LENGTH : int(48) = 48
let HTR_WITHDRAWAL_REQUEST_AMOUNT : int(68) = 68
let HTR_CONSOLIDATION_REQUEST_LENGTH : int(116) = 116
let HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY : int(68) = 68
let HTR_BUILDER_DEPOSIT_LENGTH : int(184) = 184
let HTR_BUILDER_DEPOSIT_PUBKEY : int(0) = 0
let HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS : int(48) = 48
let HTR_BUILDER_DEPOSIT_AMOUNT : int(80) = 80
let HTR_BUILDER_DEPOSIT_SIGNATURE : int(88) = 88
let HTR_BUILDER_EXIT_LENGTH : int(68) = 68
let HTR_BUILDER_EXIT_SOURCE_ADDRESS : int(0) = 0
let HTR_BUILDER_EXIT_PUBKEY : int(20) = 20

/* SSZ zero hashes by subtree level. Keeping these as constants avoids hashing
 * the same all-zero subtree once per level for every Merkle root. */
let SSZ_ZERO_HASH_01 : hash = word_to_hash(
    u256(unsigned(0xf5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b)),
)
let SSZ_ZERO_HASH_02 : hash = word_to_hash(
    u256(unsigned(0xdb56114e00fdd4c1f85c892bf35ac9a89289aaecb1ebd0a96cde606a748b5d71)),
)
let SSZ_ZERO_HASH_03 : hash = word_to_hash(
    u256(unsigned(0xc78009fdf07fc56a11f122370658a353aaa542ed63e44c4bc15ff4cd105ab33c)),
)
let SSZ_ZERO_HASH_04 : hash = word_to_hash(
    u256(unsigned(0x536d98837f2dd165a55d5eeae91485954472d56f246df256bf3cae19352a123c)),
)
let SSZ_ZERO_HASH_05 : hash = word_to_hash(
    u256(unsigned(0x9efde052aa15429fae05bad4d0b1d7c64da64d03d7a1854a588c2cb8430c0d30)),
)
let SSZ_ZERO_HASH_06 : hash = word_to_hash(
    u256(unsigned(0xd88ddfeed400a8755596b21942c1497e114c302e6118290f91e6772976041fa1)),
)
let SSZ_ZERO_HASH_07 : hash = word_to_hash(
    u256(unsigned(0x87eb0ddba57e35f6d286673802a4af5975e22506c7cf4c64bb6be5ee11527f2c)),
)
let SSZ_ZERO_HASH_08 : hash = word_to_hash(
    u256(unsigned(0x26846476fd5fc54a5d43385167c95144f2643f533cc85bb9d16b782f8d7db193)),
)
let SSZ_ZERO_HASH_09 : hash = word_to_hash(
    u256(unsigned(0x506d86582d252405b840018792cad2bf1259f1ef5aa5f887e13cb2f0094f51e1)),
)
let SSZ_ZERO_HASH_10 : hash = word_to_hash(
    u256(unsigned(0xffff0ad7e659772f9534c195c815efc4014ef1e1daed4404c06385d11192e92b)),
)
let SSZ_ZERO_HASH_11 : hash = word_to_hash(
    u256(unsigned(0x6cf04127db05441cd833107a52be852868890e4317e6a02ab47683aa75964220)),
)
let SSZ_ZERO_HASH_12 : hash = word_to_hash(
    u256(unsigned(0xb7d05f875f140027ef5118a2247bbb84ce8f2f0f1123623085daf7960c329f5f)),
)
let SSZ_ZERO_HASH_13 : hash = word_to_hash(
    u256(unsigned(0xdf6af5f5bbdb6be9ef8aa618e4bf8073960867171e29676f8b284dea6a08a85e)),
)
let SSZ_ZERO_HASH_14 : hash = word_to_hash(
    u256(unsigned(0xb58d900f5e182e3c50ef74969ea16c7726c549757cc23523c369587da7293784)),
)
let SSZ_ZERO_HASH_15 : hash = word_to_hash(
    u256(unsigned(0xd49a7502ffcfb0340b1d7885688500ca308161a7f96b62df9d083b71fcc8f2bb)),
)
let SSZ_ZERO_HASH_16 : hash = word_to_hash(
    u256(unsigned(0x8fe6b1689256c0d385f42f5bbe2027a22c1996e110ba97c171d3e5948de92beb)),
)
let SSZ_ZERO_HASH_17 : hash = word_to_hash(
    u256(unsigned(0x8d0d63c39ebade8509e0ae3c9c3876fb5fa112be18f905ecacfecb92057603ab)),
)
let SSZ_ZERO_HASH_18 : hash = word_to_hash(
    u256(unsigned(0x95eec8b2e541cad4e91de38385f2e046619f54496c2382cb6cacd5b98c26f5a4)),
)
let SSZ_ZERO_HASH_19 : hash = word_to_hash(
    u256(unsigned(0xf893e908917775b62bff23294dbbe3a1cd8e6cc1c35b4801887b646a6f81f17f)),
)
let SSZ_ZERO_HASH_20 : hash = word_to_hash(
    u256(unsigned(0xcddba7b592e3133393c16194fac7431abf2f5485ed711db282183c819e08ebaa)),
)
let SSZ_ZERO_HASH_21 : hash = word_to_hash(
    u256(unsigned(0x8a8d7fe3af8caa085a7639a832001457dfb9128a8061142ad0335629ff23ff9c)),
)
let SSZ_ZERO_HASH_22 : hash = word_to_hash(
    u256(unsigned(0xfeb3c337d7a51a6fbf00b9e34c52e1c9195c969bd4e7a0bfd51d5c5bed9c1167)),
)
let SSZ_ZERO_HASH_23 : hash = word_to_hash(
    u256(unsigned(0xe71f0aa83cc32edfbefa9f4d3e0174ca85182eec9f3a09f6a6c0df6377a510d7)),
)
let SSZ_ZERO_HASH_24 : hash = word_to_hash(
    u256(unsigned(0x31206fa80a50bb6abe29085058f16212212a60eec8f049fecb92d8c8e0a84bc0)),
)
let SSZ_ZERO_HASH_25 : hash = word_to_hash(
    u256(unsigned(0x21352bfecbeddde993839f614c3dac0a3ee37543f9b412b16199dc158e23b544)),
)

/*! The root of an all-zero subtree at `level`. */
function ssz_zero_hash(level : htr_depth) -> hash = match level {
    0 => ZERO_HASH,
    1 => SSZ_ZERO_HASH_01,
    2 => SSZ_ZERO_HASH_02,
    3 => SSZ_ZERO_HASH_03,
    4 => SSZ_ZERO_HASH_04,
    5 => SSZ_ZERO_HASH_05,
    6 => SSZ_ZERO_HASH_06,
    7 => SSZ_ZERO_HASH_07,
    8 => SSZ_ZERO_HASH_08,
    9 => SSZ_ZERO_HASH_09,
    10 => SSZ_ZERO_HASH_10,
    11 => SSZ_ZERO_HASH_11,
    12 => SSZ_ZERO_HASH_12,
    13 => SSZ_ZERO_HASH_13,
    14 => SSZ_ZERO_HASH_14,
    15 => SSZ_ZERO_HASH_15,
    16 => SSZ_ZERO_HASH_16,
    17 => SSZ_ZERO_HASH_17,
    18 => SSZ_ZERO_HASH_18,
    19 => SSZ_ZERO_HASH_19,
    20 => SSZ_ZERO_HASH_20,
    21 => SSZ_ZERO_HASH_21,
    22 => SSZ_ZERO_HASH_22,
    23 => SSZ_ZERO_HASH_23,
    24 => SSZ_ZERO_HASH_24,
    25 => SSZ_ZERO_HASH_25,
}

/*! Number of leaves in a complete supported SSZ tree at `depth`. */
function htr_leaf_capacity(depth : htr_depth) -> htr_leaf_count = match depth {
    0 => 1,
    1 => 2,
    2 => 4,
    3 => 8,
    4 => 16,
    5 => 32,
    6 => 64,
    7 => 128,
    8 => 256,
    9 => 512,
    10 => 1024,
    11 => 2048,
    12 => 4096,
    13 => 8192,
    14 => 16384,
    15 => 32768,
    16 => 65536,
    17 => 131072,
    18 => 262144,
    19 => 524288,
    20 => 1048576,
    21 => 2097152,
    22 => 4194304,
    23 => 8388608,
    24 => 16777216,
    25 => 33554432,
}

/*! Recursively merkleizes the live prefix of a fixed leaf vector. */
val merkleize_vector_subtree : forall 'n 'start, 1 <= 'n & 'n <= 32 & 0 <= 'start. (
        vector('n, dec, hash),
        int('start),
        htr_leaf_count,
        htr_depth,
    ) -> hash

function merkleize_vector_subtree(leaves, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if (count == 1) & (start < length(leaves)) then {
                leaves[length(leaves) - 1 - start]
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                merkleize_vector_subtree(leaves, start, left_count, child_depth),
                merkleize_vector_subtree(leaves, start + half, right_count, child_depth),
            )
        }
    }
termination_measure merkleize_vector_subtree(leaves, start, count, depth) = depth

/*! `merkleize` of a fixed leaf vector at `depth` (SSZ spec). */
val merkleize : forall 'n, 1 <= 'n & 'n <= 32. (vector('n, dec, hash), htr_depth) -> hash
function merkleize(leaves, depth) = {
    let count : htr_leaf_count = length(leaves);
    if count <= htr_leaf_capacity(depth) then {
        merkleize_vector_subtree(leaves, 0, count, depth)
    } else {
        fatal_error(WitnessDeficient);
        exit(())
    }
}

/* ---- basic types and byte sequences ----------------------------------- */

/* SSZ 64-bit unsigned integer -> 32-byte chunk (little-endian in low 8 bytes) */
/*! Encodes a protocol quantity as an SSZ little-endian basic-value chunk. */
function htr_uint(v : ssz_uint) -> hash = {
    var bytes = ZERO_HASH;
    bytes[0] = get_slice_int(8, v, 0);
    bytes[1] = get_slice_int(8, v, 8);
    bytes[2] = get_slice_int(8, v, 16);
    bytes[3] = get_slice_int(8, v, 24);
    bytes[4] = get_slice_int(8, v, 32);
    bytes[5] = get_slice_int(8, v, 40);
    bytes[6] = get_slice_int(8, v, 48);
    bytes[7] = get_slice_int(8, v, 56);
    B256(bytes)
}

/*! `uint256` as a 32-byte little-endian chunk. */
function htr_u256(value : word) -> hash = {
    var bytes = ZERO_HASH;
    bytes[0] = get_slice_int(8, value, 0);
    bytes[1] = get_slice_int(8, value, 8);
    bytes[2] = get_slice_int(8, value, 16);
    bytes[3] = get_slice_int(8, value, 24);
    bytes[4] = get_slice_int(8, value, 32);
    bytes[5] = get_slice_int(8, value, 40);
    bytes[6] = get_slice_int(8, value, 48);
    bytes[7] = get_slice_int(8, value, 56);
    bytes[8] = get_slice_int(8, value, 64);
    bytes[9] = get_slice_int(8, value, 72);
    bytes[10] = get_slice_int(8, value, 80);
    bytes[11] = get_slice_int(8, value, 88);
    bytes[12] = get_slice_int(8, value, 96);
    bytes[13] = get_slice_int(8, value, 104);
    bytes[14] = get_slice_int(8, value, 112);
    bytes[15] = get_slice_int(8, value, 120);
    bytes[16] = get_slice_int(8, value, 128);
    bytes[17] = get_slice_int(8, value, 136);
    bytes[18] = get_slice_int(8, value, 144);
    bytes[19] = get_slice_int(8, value, 152);
    bytes[20] = get_slice_int(8, value, 160);
    bytes[21] = get_slice_int(8, value, 168);
    bytes[22] = get_slice_int(8, value, 176);
    bytes[23] = get_slice_int(8, value, 184);
    bytes[24] = get_slice_int(8, value, 192);
    bytes[25] = get_slice_int(8, value, 200);
    bytes[26] = get_slice_int(8, value, 208);
    bytes[27] = get_slice_int(8, value, 216);
    bytes[28] = get_slice_int(8, value, 224);
    bytes[29] = get_slice_int(8, value, 232);
    bytes[30] = get_slice_int(8, value, 240);
    bytes[31] = get_slice_int(8, value, 248);
    B256(bytes)
}

/*! `Bytes32` chunks are themselves. */
function htr_bytes32(b : hash) -> hash = b

/*! A 20-byte address, left-aligned in its chunk. */
function htr_addr(address_bytes : address) -> hash = {
    var bytes = ZERO_HASH;
    bytes[0] = address_bytes[0];
    bytes[1] = address_bytes[1];
    bytes[2] = address_bytes[2];
    bytes[3] = address_bytes[3];
    bytes[4] = address_bytes[4];
    bytes[5] = address_bytes[5];
    bytes[6] = address_bytes[6];
    bytes[7] = address_bytes[7];
    bytes[8] = address_bytes[8];
    bytes[9] = address_bytes[9];
    bytes[10] = address_bytes[10];
    bytes[11] = address_bytes[11];
    bytes[12] = address_bytes[12];
    bytes[13] = address_bytes[13];
    bytes[14] = address_bytes[14];
    bytes[15] = address_bytes[15];
    bytes[16] = address_bytes[16];
    bytes[17] = address_bytes[17];
    bytes[18] = address_bytes[18];
    bytes[19] = address_bytes[19];
    B256(bytes)
}

/*! A source length as the SSZ little-endian `uint256` chunk used by
    `mix_in_length`. */
val htr_length_chunk : forall 'len, source_valid_length('len). int('len) -> hash
function htr_length_chunk(len) =
    htr_u256(word_of_byte_count(len))

/*! `mix_in_length`: hashes the list root with its uint256 length chunk. */
val mix_in_length : forall 'len, source_valid_length('len). (hash, int('len)) -> hash
function mix_in_length(root, len) =
    sha256_pair(root, htr_length_chunk(len))

/*! The ceiling of log2 — the Merkle depth of an `n`-chunk capacity. */
function clog2(n : htr_leaf_count) -> htr_depth = {
    var remaining : htr_leaf_count = if n == 0 then {
            0
        } else {
            n - 1
        };
    var depth : htr_depth = 0;
    foreach (_step from 0 to 24) {
        if remaining != 0 then {
            let current_depth = depth;
            depth = if current_depth < 25 then {
                    current_depth + 1
                } else {
                    fatal_error(WitnessDeficient);
                    exit(())
                };
            remaining = tdiv_nat(remaining, 2)
        }
    };
    depth
}

/*! The `chunk_index`-th 32-byte chunk of a byte span, zero-padded. */
val htr_chunk : forall 'index, 0 <= 'index. (StatelessInputSlice, int('index)) -> hash
function htr_chunk(bytes, chunk_index) =
    if chunk_index <= 33554432 then {
        let offset : source_pointer = chunk_index * WORD_BYTE_LENGTH;
        word_to_hash(slice_load(bytes, offset))
    } else {
        fatal_error(WitnessDeficient);
        exit(())
    }

/*! Returns the number of 32-byte chunks needed to contain `byte_len` bytes. */
val htr_chunk_count : forall 'byte_len, source_valid_length('byte_len). int('byte_len) -> htr_leaf_count
function htr_chunk_count(byte_len) =
    if byte_len <= HTR_BYTE_LIST_LIMIT then {
        (byte_len + 31) / 32
    } else {
        fatal_error(WitnessDeficient);
        exit(())
    }

/*! Recursively merkleizes the live chunk prefix of a byte span. */
val htr_bytes_subtree : forall 'start, 0 <= 'start. (StatelessInputSlice, int('start), htr_leaf_count, htr_depth) -> hash
function htr_bytes_subtree(bytes, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if count == 1 then {
                htr_chunk(bytes, start)
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                htr_bytes_subtree(bytes, start, left_count, child_depth),
                htr_bytes_subtree(bytes, start + half, right_count, child_depth),
            )
        }
    }
termination_measure htr_bytes_subtree(bytes, start, count, depth) = depth

/*! The Merkle root of a byte span's chunks at `depth`. */
function htr_bytes_root(bytes : StatelessInputSlice, depth : htr_depth) -> hash = {
    let count = htr_chunk_count(bytes.len);
    if count <= htr_leaf_capacity(depth) then {
            htr_bytes_subtree(bytes, 0, count, depth)
        } else {
            fatal_error(WitnessDeficient);
            exit(())
        }
}

/*md
## Helpers
*/
/*! `hash_tree_root` of a fixed-size `ByteVector`. */
function htr_bytevector(bytes : StatelessInputSlice) -> hash = {
    let chunks = htr_chunk_count(bytes.len);
    htr_bytes_root(bytes, clog2(chunks))
}

/* These source-bound helpers deliberately state the decode-then-HTR
 * equations used by the SSZ specification. The standard native build and
 * proof extractions retain these equations. The optimized C splice replaces
 * only the complete `htr_new_payload_request` operation, leaving this
 * readable specification unchanged. */
/*! HTR of an SSZ `uint64` field in an enclosing serialized object. */
val htr_ssz_uint : forall 'off, source_valid_range('off, 8). (StatelessInputSlice, int('off)) -> hash
function htr_ssz_uint(bytes, offset) =
    htr_uint(decode_ssz_uint(bytes, offset))

/*! HTR of an SSZ `uint256` field in an enclosing serialized object. */
val htr_ssz_u256 : forall 'off, source_valid_range('off, 32). (StatelessInputSlice, int('off)) -> hash
function htr_ssz_u256(bytes, offset) =
    htr_u256(ssz_u256(bytes, offset))

/*! HTR of an SSZ `Bytes32` field in an enclosing serialized object. */
val htr_ssz_bytes32 : forall 'off, source_valid_range('off, 32). (StatelessInputSlice, int('off)) -> hash
function htr_ssz_bytes32(bytes, offset) =
    htr_bytes32(ssz_bytes32(bytes, offset))

/*! HTR of an SSZ 20-byte address field in an enclosing serialized object. */
val htr_ssz_addr : forall 'off, source_valid_range('off, 20). (StatelessInputSlice, int('off)) -> hash
function htr_ssz_addr(bytes, offset) =
    htr_addr(ssz_addr(bytes, offset))

/*! HTR of a fixed-size `ByteVector` field in an enclosing serialized object. */
val htr_ssz_bytevector : forall 'off 'len, source_valid_range('off, 'len). (StatelessInputSlice, int('off), int('len)) -> hash
function htr_ssz_bytevector(bytes, offset, len) = {
    if offset + len <= bytes.len then {
        htr_bytevector(sub_slice(bytes, offset, len))
    } else {
        fatal_error(InvalidConfig);
        exit(())
    }
}

/*! `hash_tree_root` of a `ByteList[limit_bytes]` (root at the limit's
    depth, length mixed in). */
val htr_bytelist : forall 'limit, source_valid_length('limit). (StatelessInputSlice, int('limit)) -> hash
function htr_bytelist(bytes, limit_bytes) = {
    let capacity = htr_chunk_count(limit_bytes);
    mix_in_length(htr_bytes_root(bytes, clog2(capacity)), bytes.len)
}

/*md
## The execution payload
*/
/*! `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). */
function htr_withdrawal(withdrawal : StatelessInputSliceLength(44)) -> hash = {
    merkleize(
        [
            htr_ssz_uint(withdrawal, WD_INDEX),
            htr_ssz_uint(withdrawal, WD_VALIDATOR_INDEX),
            htr_ssz_addr(withdrawal, WD_ADDRESS),
            htr_ssz_uint(withdrawal, WD_AMOUNT),
        ],
        2,
    )
}

/*! Recursively merkleizes a range of transaction roots. */
val htr_transactions_subtree : forall 'start, 0 <= 'start. (TransactionListRef, int('start), htr_leaf_count, htr_depth) -> hash
function htr_transactions_subtree(transactions, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if count == 1 then {
                htr_bytelist(ssz_list_at(transactions, start), HTR_BYTE_LIST_LIMIT)
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                htr_transactions_subtree(transactions, start, left_count, child_depth),
                htr_transactions_subtree(transactions, start + half, right_count, child_depth),
            )
        }
    }
termination_measure htr_transactions_subtree(transactions, start, count, depth) = depth

/*! `hash_tree_root` of the transactions list
    (`List[ByteList[2^30], 2^20]`). */
function htr_transactions(transactions : TransactionListRef) -> hash = {
    let count : htr_leaf_count = transactions.count;
    mix_in_length(htr_transactions_subtree(transactions, 0, count, 20), transactions.count)
}

/*! Recursively merkleizes a range of withdrawal roots. */
val htr_withdrawals_subtree : forall 'start, 0 <= 'start. (WithdrawalListRef, int('start), htr_leaf_count, htr_depth) -> hash
function htr_withdrawals_subtree(withdrawals, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if count == 1 then {
                htr_withdrawal(ssz_fixed_list_at(withdrawals, start, WD_SIZE))
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                htr_withdrawals_subtree(withdrawals, start, left_count, child_depth),
                htr_withdrawals_subtree(withdrawals, start + half, right_count, child_depth),
            )
        }
    }
termination_measure htr_withdrawals_subtree(withdrawals, start, count, depth) = depth

/*! `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). */
function htr_withdrawals(withdrawals : WithdrawalListRef) -> hash = {
    let count : htr_leaf_count = withdrawals.count;
    mix_in_length(htr_withdrawals_subtree(withdrawals, 0, count, 4), withdrawals.count)
}

/*! `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). */
function htr_execution_payload(input_ref : StatelessInputRef) -> hash = {
    let payload = input_ref.execution_payload;
    merkleize(
        [
            htr_ssz_bytes32(payload, 0),
            htr_ssz_addr(payload, PL_FEE_RECIPIENT),
            htr_ssz_bytes32(payload, PL_STATE_ROOT),
            htr_ssz_bytes32(payload, PL_RECEIPTS_ROOT),
            htr_ssz_bytevector(payload, PL_LOGS_BLOOM, LOGS_BLOOM_BYTE_LENGTH),
            htr_ssz_bytes32(payload, PL_PREV_RANDAO),
            htr_ssz_uint(payload, PL_BLOCK_NUMBER),
            htr_ssz_uint(payload, PL_GAS_LIMIT),
            htr_ssz_uint(payload, PL_GAS_USED),
            htr_ssz_uint(payload, PL_TIMESTAMP),
            htr_bytelist(input_ref.extra_data, WORD_BYTE_LENGTH),
            htr_ssz_u256(payload, PL_BASE_FEE),
            htr_ssz_bytes32(payload, PL_BLOCK_HASH),
            htr_transactions(input_ref.transactions),
            htr_withdrawals(input_ref.withdrawals),
            htr_ssz_uint(payload, PL_BLOB_GAS_USED),
            htr_ssz_uint(payload, PL_EXCESS_BLOB_GAS),
            htr_bytelist(input_ref.block_access_list, HTR_BYTE_LIST_LIMIT),
            htr_ssz_uint(payload, PL_SLOT_NUMBER),
        ],
        5,
    )
}

/*! Recursively merkleizes a range of fixed-width versioned hashes. */
val htr_versioned_hashes_subtree : forall 'start, 0 <= 'start. (
        StatelessInputSlice,
        int('start),
        htr_leaf_count,
        htr_depth,
    ) -> hash
function htr_versioned_hashes_subtree(versioned_hashes, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if count == 1 then {
                htr_chunk(versioned_hashes, start)
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                htr_versioned_hashes_subtree(versioned_hashes, start, left_count, child_depth),
                htr_versioned_hashes_subtree(versioned_hashes, start + half, right_count, child_depth),
            )
        }
    }
termination_measure htr_versioned_hashes_subtree(versioned_hashes, start, count, depth) = depth

/*! `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). */
function htr_versioned_hashes(versioned_hashes : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(versioned_hashes, WORD_BYTE_LENGTH, sizeof(blob_commitment_count_bound));
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_versioned_hashes_subtree(versioned_hashes, 0, count, 12), items.count)
}

/*md
## The execution requests

The EIP-7685 execution-request commitments.
*/
/*! `hash_tree_root` of one deposit request (5 fields, depth 3). */
function htr_deposit(deposit : StatelessInputSliceLength(192)) -> hash = {
    merkleize(
        [
            htr_ssz_bytevector(deposit, HTR_DEPOSIT_PUBKEY, HTR_DEPOSIT_PUBKEY_LENGTH),
            htr_ssz_bytes32(deposit, HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS),
            htr_ssz_uint(deposit, HTR_DEPOSIT_AMOUNT),
            htr_ssz_bytevector(deposit, HTR_DEPOSIT_SIGNATURE, HTR_DEPOSIT_SIGNATURE_LENGTH),
            htr_ssz_uint(deposit, HTR_DEPOSIT_INDEX),
        ],
        3,
    )
}

/*! `hash_tree_root` of one withdrawal request (3 fields, depth 2). */
function htr_withdrawal_request(request : StatelessInputSliceLength(76)) -> hash = {
    merkleize(
        [
            htr_ssz_addr(request, HTR_REQUEST_SOURCE_ADDRESS),
            htr_ssz_bytevector(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH),
            htr_ssz_uint(request, HTR_WITHDRAWAL_REQUEST_AMOUNT),
        ],
        2,
    )
}

/*! `hash_tree_root` of one consolidation request (3 fields, depth 2). */
function htr_consolidation_request(request : StatelessInputSliceLength(116)) -> hash =
    merkleize(
        [
            htr_ssz_addr(request, HTR_REQUEST_SOURCE_ADDRESS),
            htr_ssz_bytevector(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH),
            htr_ssz_bytevector(request, HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH),
        ],
        2,
    )

/*! `hash_tree_root` of one builder deposit request (4 fields, depth 2). */
function htr_builder_deposit_request(request : StatelessInputSliceLength(184)) -> hash =
    merkleize(
        [
            htr_ssz_bytevector(request, HTR_BUILDER_DEPOSIT_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH),
            htr_ssz_bytes32(request, HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS),
            htr_ssz_uint(request, HTR_BUILDER_DEPOSIT_AMOUNT),
            htr_ssz_bytevector(request, HTR_BUILDER_DEPOSIT_SIGNATURE, HTR_DEPOSIT_SIGNATURE_LENGTH),
        ],
        2,
    )

/*! `hash_tree_root` of one builder exit request (2 fields, depth 1). */
function htr_builder_exit_request(request : StatelessInputSliceLength(68)) -> hash =
    merkleize(
        [
            htr_ssz_addr(request, HTR_BUILDER_EXIT_SOURCE_ADDRESS),
            htr_ssz_bytevector(request, HTR_BUILDER_EXIT_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH),
        ],
        1,
    )

/*! The closed fixed-width leaf-operation algebra carried through the generic
    request-list recursion. [htr_request_leaf][] interprets it at depth zero,
    replacing a higher-order leaf function without duplicating merkleization. */
enum HtrRequestKind = {
    HtrDeposit,
    HtrWithdrawalRequest,
    HtrConsolidationRequest,
    HtrBuilderDepositRequest,
    HtrBuilderExitRequest,
}

/*! Hashes one fixed-width execution-request item at `index`. */
val htr_request_leaf : forall 'index, 0 <= 'index. (StatelessInputSlice, int('index), HtrRequestKind) -> hash
function htr_request_leaf(requests, index, kind) =
    match kind {
        HtrDeposit => {
            let offset = index * HTR_DEPOSIT_LENGTH;
            if offset + HTR_DEPOSIT_LENGTH <= requests.len then {
                htr_deposit(sub_slice(requests, offset, HTR_DEPOSIT_LENGTH))
            } else {
                fatal_error(InvalidConfig);
                exit(())
            }
        },
        HtrWithdrawalRequest => {
            let offset = index * HTR_WITHDRAWAL_REQUEST_LENGTH;
            if offset + HTR_WITHDRAWAL_REQUEST_LENGTH <= requests.len then {
                htr_withdrawal_request(sub_slice(requests, offset, HTR_WITHDRAWAL_REQUEST_LENGTH))
            } else {
                fatal_error(InvalidConfig);
                exit(())
            }
        },
        HtrConsolidationRequest => {
            let offset = index * HTR_CONSOLIDATION_REQUEST_LENGTH;
            if offset + HTR_CONSOLIDATION_REQUEST_LENGTH <= requests.len then {
                htr_consolidation_request(sub_slice(requests, offset, HTR_CONSOLIDATION_REQUEST_LENGTH))
            } else {
                fatal_error(InvalidConfig);
                exit(())
            }
        },
        HtrBuilderDepositRequest => {
            let offset = index * HTR_BUILDER_DEPOSIT_LENGTH;
            if offset + HTR_BUILDER_DEPOSIT_LENGTH <= requests.len then {
                htr_builder_deposit_request(sub_slice(requests, offset, HTR_BUILDER_DEPOSIT_LENGTH))
            } else {
                fatal_error(InvalidConfig);
                exit(())
            }
        },
        HtrBuilderExitRequest => {
            let offset = index * HTR_BUILDER_EXIT_LENGTH;
            if offset + HTR_BUILDER_EXIT_LENGTH <= requests.len then {
                htr_builder_exit_request(sub_slice(requests, offset, HTR_BUILDER_EXIT_LENGTH))
            } else {
                fatal_error(InvalidConfig);
                exit(())
            }
        },
    }

/*! Recursively merkleizes a range of fixed-width execution-request roots. */
val htr_requests_subtree : forall 'start, 0 <= 'start. (
        StatelessInputSlice,
        HtrRequestKind,
        int('start),
        htr_leaf_count,
        htr_depth,
    ) -> hash
function htr_requests_subtree(requests, kind, start, count, depth) =
    if count == 0 then {
        ssz_zero_hash(depth)
    } else {
        if depth == 0 then {
            if count == 1 then {
                htr_request_leaf(requests, start, kind)
            } else {
                fatal_error(WitnessDeficient);
                exit(())
            }
        } else {
            let child_depth : htr_depth = depth - 1;
            let half = htr_leaf_capacity(child_depth);
            let left_count : htr_leaf_count = if count < half then { count } else { half };
            let right_count : htr_leaf_count = if count < half then {
                    0
                } else {
                    count - half
                };
            sha256_pair(
                htr_requests_subtree(requests, kind, start, left_count, child_depth),
                htr_requests_subtree(requests, kind, start + half, right_count, child_depth),
            )
        }
    }
termination_measure htr_requests_subtree(requests, kind, start, count, depth) = depth

/*! `hash_tree_root` of the deposit-request list (depth 13). */
function htr_deposits(deposits : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(deposits, HTR_DEPOSIT_LENGTH, sizeof(deposit_request_count_bound));
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_requests_subtree(deposits, HtrDeposit, 0, count, 13), items.count)
}

/*! `hash_tree_root` of the withdrawal-request list (depth 4). */
function htr_withdrawal_requests(requests : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(
        requests,
        HTR_WITHDRAWAL_REQUEST_LENGTH,
        sizeof(withdrawal_request_count_bound),
    );
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_requests_subtree(requests, HtrWithdrawalRequest, 0, count, 4), items.count)
}

/*! `hash_tree_root` of the consolidation-request list (depth 1). */
function htr_consolidation_requests(requests : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(
        requests,
        HTR_CONSOLIDATION_REQUEST_LENGTH,
        sizeof(consolidation_request_count_bound),
    );
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_requests_subtree(requests, HtrConsolidationRequest, 0, count, 1), items.count)
}

/*! `hash_tree_root` of the builder-deposit-request list (depth 6). */
function htr_builder_deposit_requests(requests : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(
        requests,
        HTR_BUILDER_DEPOSIT_LENGTH,
        sizeof(builder_deposit_request_count_bound),
    );
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_requests_subtree(requests, HtrBuilderDepositRequest, 0, count, 6), items.count)
}

/*! `hash_tree_root` of the builder-exit-request list (depth 4). */
function htr_builder_exit_requests(requests : StatelessInputSlice) -> hash = {
    let items = ssz_bounded_fixed_list_ref(requests, HTR_BUILDER_EXIT_LENGTH, sizeof(builder_exit_request_count_bound));
    let count : htr_leaf_count = items.count;
    mix_in_length(htr_requests_subtree(requests, HtrBuilderExitRequest, 0, count, 4), items.count)
}

/*! `hash_tree_root` of the `SszExecutionRequests` container. */
function htr_execution_requests(input_ref : StatelessInputRef) -> hash = {
    merkleize(
        [
            htr_deposits(input_ref.deposits),
            htr_withdrawal_requests(input_ref.withdrawal_requests),
            htr_consolidation_requests(input_ref.consolidation_requests),
            htr_builder_deposit_requests(input_ref.builder_deposit_requests),
            htr_builder_exit_requests(input_ref.builder_exit_requests),
        ],
        3,
    )
}

/*! `hash_tree_root` of the `SszNewPayloadRequest` (4 fields, depth 2) —
    the commitment the guest proves its input against. */
function htr_new_payload_request(input_ref : StatelessInputRef) -> hash = {
    merkleize(
        [
            htr_execution_payload(input_ref),
            htr_versioned_hashes(input_ref.versioned_hashes),
            htr_ssz_bytes32(input_ref.new_payload_request, NPR_BEACON_ROOT),
            htr_execution_requests(input_ref),
        ],
        2,
    )
}
