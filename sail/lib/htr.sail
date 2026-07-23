
/*md
# SSZ hash_tree_root

Consensus-layer merkleization (SHA-256). Variable SSZ lists are consumed
in source order: each real leaf is folded into a Merkle frontier exactly
once, and final zero padding is then folded one level at a time. No leaf
list or zero-hash lookup table is materialized.
*/
/*! One level of the Merkle frontier: empty, or holding the pending left
    subtree hash for that level. */
union MerkleSlot = {
    /* no pending subtree at this level */
    EmptyMerkleSlot : unit,
    /* the pending left-subtree hash */
    OccupiedMerkleSlot : hash,
}

/*! The largest Merkle depth required by the supported execution-layer SSZ
    schemas. ByteList[2^30] is the widest one, with 2^25 chunks. */
type htr_depth = range(0, 25)
/*! A leaf count in any supported execution-layer SSZ Merkle tree. */
type htr_leaf_count = range(0, 33554432)

/*! A streaming Merkle frontier together with the number of leaves already
    incorporated. */
struct MerkleAccumulator = {
    frontier : list(MerkleSlot),
    count : htr_leaf_count,
    depth : htr_depth,
}

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
let SSZ_ZERO_HASH_01 : hash = B256(to_bytes_le(32, 0xf5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b))
let SSZ_ZERO_HASH_02 : hash = B256(to_bytes_le(32, 0xdb56114e00fdd4c1f85c892bf35ac9a89289aaecb1ebd0a96cde606a748b5d71))
let SSZ_ZERO_HASH_03 : hash = B256(to_bytes_le(32, 0xc78009fdf07fc56a11f122370658a353aaa542ed63e44c4bc15ff4cd105ab33c))
let SSZ_ZERO_HASH_04 : hash = B256(to_bytes_le(32, 0x536d98837f2dd165a55d5eeae91485954472d56f246df256bf3cae19352a123c))
let SSZ_ZERO_HASH_05 : hash = B256(to_bytes_le(32, 0x9efde052aa15429fae05bad4d0b1d7c64da64d03d7a1854a588c2cb8430c0d30))
let SSZ_ZERO_HASH_06 : hash = B256(to_bytes_le(32, 0xd88ddfeed400a8755596b21942c1497e114c302e6118290f91e6772976041fa1))
let SSZ_ZERO_HASH_07 : hash = B256(to_bytes_le(32, 0x87eb0ddba57e35f6d286673802a4af5975e22506c7cf4c64bb6be5ee11527f2c))
let SSZ_ZERO_HASH_08 : hash = B256(to_bytes_le(32, 0x26846476fd5fc54a5d43385167c95144f2643f533cc85bb9d16b782f8d7db193))
let SSZ_ZERO_HASH_09 : hash = B256(to_bytes_le(32, 0x506d86582d252405b840018792cad2bf1259f1ef5aa5f887e13cb2f0094f51e1))
let SSZ_ZERO_HASH_10 : hash = B256(to_bytes_le(32, 0xffff0ad7e659772f9534c195c815efc4014ef1e1daed4404c06385d11192e92b))
let SSZ_ZERO_HASH_11 : hash = B256(to_bytes_le(32, 0x6cf04127db05441cd833107a52be852868890e4317e6a02ab47683aa75964220))
let SSZ_ZERO_HASH_12 : hash = B256(to_bytes_le(32, 0xb7d05f875f140027ef5118a2247bbb84ce8f2f0f1123623085daf7960c329f5f))
let SSZ_ZERO_HASH_13 : hash = B256(to_bytes_le(32, 0xdf6af5f5bbdb6be9ef8aa618e4bf8073960867171e29676f8b284dea6a08a85e))
let SSZ_ZERO_HASH_14 : hash = B256(to_bytes_le(32, 0xb58d900f5e182e3c50ef74969ea16c7726c549757cc23523c369587da7293784))
let SSZ_ZERO_HASH_15 : hash = B256(to_bytes_le(32, 0xd49a7502ffcfb0340b1d7885688500ca308161a7f96b62df9d083b71fcc8f2bb))
let SSZ_ZERO_HASH_16 : hash = B256(to_bytes_le(32, 0x8fe6b1689256c0d385f42f5bbe2027a22c1996e110ba97c171d3e5948de92beb))
let SSZ_ZERO_HASH_17 : hash = B256(to_bytes_le(32, 0x8d0d63c39ebade8509e0ae3c9c3876fb5fa112be18f905ecacfecb92057603ab))
let SSZ_ZERO_HASH_18 : hash = B256(to_bytes_le(32, 0x95eec8b2e541cad4e91de38385f2e046619f54496c2382cb6cacd5b98c26f5a4))
let SSZ_ZERO_HASH_19 : hash = B256(to_bytes_le(32, 0xf893e908917775b62bff23294dbbe3a1cd8e6cc1c35b4801887b646a6f81f17f))
let SSZ_ZERO_HASH_20 : hash = B256(to_bytes_le(32, 0xcddba7b592e3133393c16194fac7431abf2f5485ed711db282183c819e08ebaa))
let SSZ_ZERO_HASH_21 : hash = B256(to_bytes_le(32, 0x8a8d7fe3af8caa085a7639a832001457dfb9128a8061142ad0335629ff23ff9c))
let SSZ_ZERO_HASH_22 : hash = B256(to_bytes_le(32, 0xfeb3c337d7a51a6fbf00b9e34c52e1c9195c969bd4e7a0bfd51d5c5bed9c1167))
let SSZ_ZERO_HASH_23 : hash = B256(to_bytes_le(32, 0xe71f0aa83cc32edfbefa9f4d3e0174ca85182eec9f3a09f6a6c0df6377a510d7))
let SSZ_ZERO_HASH_24 : hash = B256(to_bytes_le(32, 0x31206fa80a50bb6abe29085058f16212212a60eec8f049fecb92d8c8e0a84bc0))
let SSZ_ZERO_HASH_25 : hash = B256(to_bytes_le(32, 0x21352bfecbeddde993839f614c3dac0a3ee37543f9b412b16199dc158e23b544))

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

/*! Advances within the supported execution-layer SSZ depth range. */
function htr_depth_increment(depth : htr_depth) -> htr_depth =
    if depth < 25 then {
        depth + 1
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

/*! Creates an empty accumulator whose capacity is determined by its SSZ tree
    depth. */
function merkle_accumulator_empty(depth : htr_depth) -> MerkleAccumulator =
    struct { frontier = [||], count = 0, depth = depth }

/* Add leaf number `count` to a low-to-high frontier. A set bit in count has
 * one occupied slot at that level; carries consume those slots head-first. */
/*! Adds leaf number `count` to a low-to-high frontier. A set bit in
    `count` has one occupied slot at that level; carries consume those
    slots head-first. */
function merkle_push(frontier : list(MerkleSlot), count : htr_leaf_count, leaf : hash) -> list(MerkleSlot) =
    if tmod_int(count, 2) == 0 then {
        match frontier {
            _ :: rest => OccupiedMerkleSlot(leaf) :: rest,
            [||] => [|OccupiedMerkleSlot(leaf)|],
        }
    } else {
        match frontier {
            OccupiedMerkleSlot(left) :: rest => EmptyMerkleSlot() ::
                merkle_push(rest, tdiv_nat(count, 2), sha256_pair(left, leaf)),
            EmptyMerkleSlot() :: _ => throw InvalidBlock(WitnessDeficient),
            [||] => throw InvalidBlock(WitnessDeficient),
        }
    }

/*! Adds one leaf to a streaming Merkle accumulator. */
function merkle_accumulator_push(accumulator : MerkleAccumulator, leaf : hash) -> MerkleAccumulator = {
    let count = accumulator.count;
    if count < htr_leaf_capacity(accumulator.depth) then {
        struct {
            frontier = merkle_push(accumulator.frontier, count, leaf),
            count = count + 1,
            depth = accumulator.depth,
        }
    } else {
        throw InvalidBlock(WitnessDeficient)
    }
}

/* Pad a frontier to 2^depth leaves. The zero subtree for each level is a
 * precomputed SSZ constant, so padding is O(depth) without zero hashing. */
/*! Completes a frontier with zero subtrees through `remaining` levels and
    returns its Merkle root. */
function merkle_root_levels(
    slots : list(MerkleSlot),
    n : htr_leaf_count,
    acc : hash,
    level : htr_depth,
    remaining : htr_depth,
) -> (
    hash
) =
    if remaining == 0 then {
        if n == 0 then {
            acc
        } else {
            if n == 1 then {
                match slots {
                    OccupiedMerkleSlot(root) :: _ => root,
                    _ => throw InvalidBlock(WitnessDeficient),
                }
            } else {
                throw InvalidBlock(WitnessDeficient)
            }
        }
    } else {
        let zero = ssz_zero_hash(level);
        var slot : MerkleSlot = EmptyMerkleSlot();
        var rest : list(MerkleSlot) = [||];
        match slots {
            head :: tail => {
                slot = head;
                rest = tail
            },
            [||] => (),
        };
        let next_acc = if tmod_int(n, 2) == 1 then {
                match slot {
                    OccupiedMerkleSlot(left) => sha256_pair(left, acc),
                    EmptyMerkleSlot() => throw InvalidBlock(WitnessDeficient),
                }
            } else {
                sha256_pair(acc, zero)
            };
        let next_level = htr_depth_increment(level);
        merkle_root_levels(rest, tdiv_nat(n, 2), next_acc, next_level, remaining - 1)
    }
termination_measure merkle_root_levels(slots, n, acc, level, remaining) = remaining

/*! Pads an incremental Merkle accumulator to its declared depth and returns
    its root. */
function merkle_accumulator_root(accumulator : MerkleAccumulator) -> hash = {
    cycle_scope_start(SCOPE_HTR_MERKLE_PADDING);
    let root = merkle_root_levels(accumulator.frontier, accumulator.count, ZERO_HASH, 0, accumulator.depth);
    cycle_scope_end(SCOPE_HTR_MERKLE_PADDING);
    root
}

/*! Folds a leaf list into an existing Merkle accumulator in source order. */
function merkle_accumulate(leaves : list(hash), accumulator : MerkleAccumulator) -> MerkleAccumulator =
    match leaves {
        [||] => accumulator,
        leaf :: rest => merkle_accumulate(rest, merkle_accumulator_push(accumulator, leaf)),
    }

/*! `merkleize` of a fixed leaf list at `depth` (SSZ spec). */
function merkleize(leaves : list(hash), depth : htr_depth) -> hash =
    merkle_accumulator_root(merkle_accumulate(leaves, merkle_accumulator_empty(depth)))

/* ---- basic types and byte sequences ----------------------------------- */

/* SSZ 64-bit unsigned integer -> 32-byte chunk (little-endian in low 8 bytes) */
/*! Encodes a protocol quantity as an SSZ little-endian basic-value chunk. */
function htr_uint(v : ssz_uint) -> hash = {
    var bytes = ZERO_HASH;
    bytes[31] = get_slice_int(8, v, 0);
    bytes[30] = get_slice_int(8, v, 8);
    bytes[29] = get_slice_int(8, v, 16);
    bytes[28] = get_slice_int(8, v, 24);
    bytes[27] = get_slice_int(8, v, 32);
    bytes[26] = get_slice_int(8, v, 40);
    bytes[25] = get_slice_int(8, v, 48);
    bytes[24] = get_slice_int(8, v, 56);
    B256(bytes)
}

/*! `uint256` as a 32-byte little-endian chunk. */
function htr_u256(value : word) -> hash = {
    var bytes = ZERO_HASH;
    bytes[31] = get_slice_int(8, value, 0);
    bytes[30] = get_slice_int(8, value, 8);
    bytes[29] = get_slice_int(8, value, 16);
    bytes[28] = get_slice_int(8, value, 24);
    bytes[27] = get_slice_int(8, value, 32);
    bytes[26] = get_slice_int(8, value, 40);
    bytes[25] = get_slice_int(8, value, 48);
    bytes[24] = get_slice_int(8, value, 56);
    bytes[23] = get_slice_int(8, value, 64);
    bytes[22] = get_slice_int(8, value, 72);
    bytes[21] = get_slice_int(8, value, 80);
    bytes[20] = get_slice_int(8, value, 88);
    bytes[19] = get_slice_int(8, value, 96);
    bytes[18] = get_slice_int(8, value, 104);
    bytes[17] = get_slice_int(8, value, 112);
    bytes[16] = get_slice_int(8, value, 120);
    bytes[15] = get_slice_int(8, value, 128);
    bytes[14] = get_slice_int(8, value, 136);
    bytes[13] = get_slice_int(8, value, 144);
    bytes[12] = get_slice_int(8, value, 152);
    bytes[11] = get_slice_int(8, value, 160);
    bytes[10] = get_slice_int(8, value, 168);
    bytes[9] = get_slice_int(8, value, 176);
    bytes[8] = get_slice_int(8, value, 184);
    bytes[7] = get_slice_int(8, value, 192);
    bytes[6] = get_slice_int(8, value, 200);
    bytes[5] = get_slice_int(8, value, 208);
    bytes[4] = get_slice_int(8, value, 216);
    bytes[3] = get_slice_int(8, value, 224);
    bytes[2] = get_slice_int(8, value, 232);
    bytes[1] = get_slice_int(8, value, 240);
    bytes[0] = get_slice_int(8, value, 248);
    B256(bytes)
}

/*! `Bytes32` chunks are themselves. */
function htr_bytes32(b : hash) -> hash = b
/*! A 20-byte address, left-aligned in its chunk. */
function htr_addr(address_bytes : address) -> hash = {
    var bytes = ZERO_HASH;
    bytes[12] = address_bytes[0];
    bytes[13] = address_bytes[1];
    bytes[14] = address_bytes[2];
    bytes[15] = address_bytes[3];
    bytes[16] = address_bytes[4];
    bytes[17] = address_bytes[5];
    bytes[18] = address_bytes[6];
    bytes[19] = address_bytes[7];
    bytes[20] = address_bytes[8];
    bytes[21] = address_bytes[9];
    bytes[22] = address_bytes[10];
    bytes[23] = address_bytes[11];
    bytes[24] = address_bytes[12];
    bytes[25] = address_bytes[13];
    bytes[26] = address_bytes[14];
    bytes[27] = address_bytes[15];
    bytes[28] = address_bytes[16];
    bytes[29] = address_bytes[17];
    bytes[30] = address_bytes[18];
    bytes[31] = address_bytes[19];
    B256(bytes)
}

/*! `mix_in_length`: hashes the list root with its uint256 length chunk. */
val mix_in_length : forall 'len, source_valid_length('len). (hash, int('len)) -> hash
function mix_in_length(root, len) =
    sha256_pair(root, htr_u256(word_of_byte_count(len)))

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
            depth = htr_depth_increment(depth);
            remaining = tdiv_nat(remaining, 2)
        }
    };
    depth
}

/*! The `chunk_index`-th 32-byte chunk of a byte span, zero-padded. */
function htr_chunk(bytes : ByteSlice, chunk_index : htr_leaf_count) -> hash =
    word_to_hash(slice_load(bytes, chunk_index * WORD_BYTE_LENGTH))

/*! Returns the number of 32-byte chunks needed to contain `byte_len` bytes. */
val htr_chunk_count : forall 'byte_len, source_valid_length('byte_len). int('byte_len) -> htr_leaf_count
function htr_chunk_count(byte_len) =
    if byte_len <= HTR_BYTE_LIST_LIMIT then {
        (byte_len + 31) / 32
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

/*! The Merkle root of a byte span's chunks at `depth`. */
function htr_bytes_root(bytes : ByteSlice, depth : htr_depth) -> hash = {
    cycle_scope_start(SCOPE_HTR_BYTES_ROOT);
    let count = htr_chunk_count(bytes.len);
    var accumulator = merkle_accumulator_empty(depth);
    var index : htr_leaf_count = 0;
    while index < count termination_measure(count - index) do {
        accumulator = merkle_accumulator_push(accumulator, htr_chunk(bytes, index));
        index = accumulator.count
    };
    let root = merkle_accumulator_root(accumulator);
    cycle_scope_end(SCOPE_HTR_BYTES_ROOT);
    root
}

/*md
## Helpers
*/
/*! `hash_tree_root` of a fixed-size `ByteVector`. */
function htr_bytevector(bytes : ByteSlice) -> hash = {
    let chunks = htr_chunk_count(bytes.len);
    htr_bytes_root(bytes, clog2(chunks))
}

/*! `hash_tree_root` of a `ByteList[limit_bytes]` (root at the limit's
    depth, length mixed in). */
val htr_bytelist : forall 'limit, source_valid_length('limit). (ByteSlice, int('limit)) -> hash
function htr_bytelist(bytes, limit_bytes) = {
    let capacity = htr_chunk_count(limit_bytes);
    mix_in_length(htr_bytes_root(bytes, clog2(capacity)), bytes.len)
}

/*md
## The execution payload
*/
/*! `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). */
function htr_withdrawal(withdrawal : ByteSliceLength(44)) -> hash = {
    merkleize(
        [|
            htr_uint(decode_ssz_uint(withdrawal, WD_INDEX)),
            htr_uint(decode_ssz_uint(withdrawal, WD_VALIDATOR_INDEX)),
            htr_addr(ssz_addr(withdrawal, WD_ADDRESS)),
            htr_uint(decode_ssz_uint(withdrawal, WD_AMOUNT)),
        |],
        2,
    )
}

/*! `hash_tree_root` of the transactions list
    (`List[ByteList[2^30], 2^20]`). */
function htr_transactions(transactions : TransactionListRef) -> hash = {
    cycle_scope_start(SCOPE_HTR_TRANSACTIONS);
    var cursor = ssz_list_cursor(transactions);
    var accumulator = merkle_accumulator_empty(20);
    while not_bool(ssz_list_cursor_empty(cursor)) termination_measure(cursor.items.count - cursor.index) do {
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        accumulator = merkle_accumulator_push(accumulator, htr_bytelist(transaction, HTR_BYTE_LIST_LIMIT))
    };
    let root = mix_in_length(merkle_accumulator_root(accumulator), accumulator.count);
    cycle_scope_end(SCOPE_HTR_TRANSACTIONS);
    root
}

/*! `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). */
function htr_withdrawals(withdrawals : WithdrawalListRef) -> hash = {
    cycle_scope_start(SCOPE_HTR_WITHDRAWALS);
    var rest : WithdrawalListRef = withdrawals;
    var accumulator = merkle_accumulator_empty(4);
    while rest.count != 0 termination_measure(rest.count) do {
        let (withdrawal, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_withdrawal(withdrawal))
    };
    let root = mix_in_length(merkle_accumulator_root(accumulator), accumulator.count);
    cycle_scope_end(SCOPE_HTR_WITHDRAWALS);
    root
}

/*! `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). */
function htr_execution_payload(input_ref : StatelessInputRef) -> hash = {
    cycle_scope_start(SCOPE_HTR_EXECUTION_PAYLOAD);
    let payload = input_ref.execution_payload;
    let root = merkleize(
        [|
            htr_bytes32(ssz_bytes32(payload, 0)),
            htr_addr(ssz_addr(payload, PL_FEE_RECIPIENT)),
            htr_bytes32(ssz_bytes32(payload, PL_STATE_ROOT)),
            htr_bytes32(ssz_bytes32(payload, PL_RECEIPTS_ROOT)),
            htr_bytevector(sub_slice(payload, PL_LOGS_BLOOM, LOGS_BLOOM_BYTE_LENGTH)),
            htr_bytes32(ssz_bytes32(payload, PL_PREV_RANDAO)),
            htr_uint(decode_ssz_uint(payload, PL_BLOCK_NUMBER)),
            htr_uint(decode_ssz_uint(payload, PL_GAS_LIMIT)),
            htr_uint(decode_ssz_uint(payload, PL_GAS_USED)),
            htr_uint(decode_ssz_uint(payload, PL_TIMESTAMP)),
            htr_bytelist(input_ref.extra_data, WORD_BYTE_LENGTH),
            htr_u256(ssz_u256(payload, PL_BASE_FEE)),
            htr_bytes32(ssz_bytes32(payload, PL_BLOCK_HASH)),
            htr_transactions(input_ref.transactions),
            htr_withdrawals(input_ref.withdrawals),
            htr_uint(decode_ssz_uint(payload, PL_BLOB_GAS_USED)),
            htr_uint(decode_ssz_uint(payload, PL_EXCESS_BLOB_GAS)),
            htr_bytelist(input_ref.block_access_list, HTR_BYTE_LIST_LIMIT),
            htr_uint(decode_ssz_uint(payload, PL_SLOT_NUMBER)),
        |],
        5,
    );
    cycle_scope_end(SCOPE_HTR_EXECUTION_PAYLOAD);
    root
}

/*! `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). */
function htr_versioned_hashes(versioned_hashes : ByteSlice) -> hash = {
    cycle_scope_start(SCOPE_HTR_VERSIONED_HASHES);
    var rest = ssz_bounded_fixed_list_ref(versioned_hashes, WORD_BYTE_LENGTH, sizeof(blob_commitment_count_bound));
    var accumulator = merkle_accumulator_empty(12);
    while rest.count != 0 termination_measure(rest.count) do {
        let (versioned_hash, tail) = ssz_fixed_list_pop(rest, WORD_BYTE_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, word_to_hash(slice_load(versioned_hash, 0)))
    };
    let root = mix_in_length(merkle_accumulator_root(accumulator), accumulator.count);
    cycle_scope_end(SCOPE_HTR_VERSIONED_HASHES);
    root
}

/*md
## The execution requests

The EIP-7685 execution-request commitments.
*/
/*! `hash_tree_root` of one deposit request (5 fields, depth 3). */
function htr_deposit(deposit : ByteSliceLength(192)) -> hash = {
    merkleize(
        [|
            htr_bytevector(sub_slice(deposit, HTR_DEPOSIT_PUBKEY, HTR_DEPOSIT_PUBKEY_LENGTH)),
            htr_bytes32(ssz_bytes32(deposit, HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS)),
            htr_uint(decode_ssz_uint(deposit, HTR_DEPOSIT_AMOUNT)),
            htr_bytevector(sub_slice(deposit, HTR_DEPOSIT_SIGNATURE, HTR_DEPOSIT_SIGNATURE_LENGTH)),
            htr_uint(decode_ssz_uint(deposit, HTR_DEPOSIT_INDEX)),
        |],
        3,
    )
}

/*! `hash_tree_root` of one withdrawal request (3 fields, depth 2). */
function htr_withdrawal_request(request : ByteSliceLength(76)) -> hash = {
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_ADDRESS, ADDRESS_BYTE_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
            htr_uint(decode_ssz_uint(request, HTR_WITHDRAWAL_REQUEST_AMOUNT)),
        |],
        2,
    )
}

/*! `hash_tree_root` of one consolidation request (3 fields, depth 2). */
function htr_consolidation_request(request : ByteSliceLength(116)) -> hash =
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_ADDRESS, ADDRESS_BYTE_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
        |],
        2,
    )

/*! `hash_tree_root` of one builder deposit request (4 fields, depth 2). */
function htr_builder_deposit_request(request : ByteSliceLength(184)) -> hash =
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_BUILDER_DEPOSIT_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
            htr_bytes32(ssz_bytes32(request, HTR_BUILDER_DEPOSIT_WITHDRAWAL_CREDENTIALS)),
            htr_uint(decode_ssz_uint(request, HTR_BUILDER_DEPOSIT_AMOUNT)),
            htr_bytevector(sub_slice(request, HTR_BUILDER_DEPOSIT_SIGNATURE, HTR_DEPOSIT_SIGNATURE_LENGTH)),
        |],
        2,
    )

/*! `hash_tree_root` of one builder exit request (2 fields, depth 1). */
function htr_builder_exit_request(request : ByteSliceLength(68)) -> hash =
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_BUILDER_EXIT_SOURCE_ADDRESS, ADDRESS_BYTE_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_BUILDER_EXIT_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
        |],
        1,
    )

/*! `hash_tree_root` of the deposit-request list (depth 13). */
function htr_deposits(deposits : ByteSlice) -> hash = {
    var rest = ssz_bounded_fixed_list_ref(deposits, HTR_DEPOSIT_LENGTH, sizeof(deposit_request_count_bound));
    var accumulator = merkle_accumulator_empty(13);
    while rest.count != 0 termination_measure(rest.count) do {
        let (deposit, tail) = ssz_fixed_list_pop(rest, HTR_DEPOSIT_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_deposit(deposit))
    };
    mix_in_length(merkle_accumulator_root(accumulator), accumulator.count)
}

/*! `hash_tree_root` of the withdrawal-request list (depth 4). */
function htr_withdrawal_requests(requests : ByteSlice) -> hash = {
    var rest = ssz_bounded_fixed_list_ref(
        requests,
        HTR_WITHDRAWAL_REQUEST_LENGTH,
        sizeof(withdrawal_request_count_bound),
    );
    var accumulator = merkle_accumulator_empty(4);
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_WITHDRAWAL_REQUEST_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_withdrawal_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator), accumulator.count)
}

/*! `hash_tree_root` of the consolidation-request list (depth 1). */
function htr_consolidation_requests(requests : ByteSlice) -> hash = {
    var rest = ssz_bounded_fixed_list_ref(
        requests,
        HTR_CONSOLIDATION_REQUEST_LENGTH,
        sizeof(consolidation_request_count_bound),
    );
    var accumulator = merkle_accumulator_empty(1);
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_CONSOLIDATION_REQUEST_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_consolidation_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator), accumulator.count)
}

/*! `hash_tree_root` of the builder-deposit-request list (depth 6). */
function htr_builder_deposit_requests(requests : ByteSlice) -> hash = {
    var rest = ssz_bounded_fixed_list_ref(
        requests,
        HTR_BUILDER_DEPOSIT_LENGTH,
        sizeof(builder_deposit_request_count_bound),
    );
    var accumulator = merkle_accumulator_empty(6);
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_BUILDER_DEPOSIT_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_builder_deposit_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator), accumulator.count)
}

/*! `hash_tree_root` of the builder-exit-request list (depth 4). */
function htr_builder_exit_requests(requests : ByteSlice) -> hash = {
    var rest = ssz_bounded_fixed_list_ref(requests, HTR_BUILDER_EXIT_LENGTH, sizeof(builder_exit_request_count_bound));
    var accumulator = merkle_accumulator_empty(4);
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_BUILDER_EXIT_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_builder_exit_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator), accumulator.count)
}

/*! `hash_tree_root` of the `SszExecutionRequests` container. */
function htr_execution_requests(input_ref : StatelessInputRef) -> hash = {
    cycle_scope_start(SCOPE_HTR_EXECUTION_REQUESTS);
    let root = merkleize(
        [|
            htr_deposits(input_ref.deposits),
            htr_withdrawal_requests(input_ref.withdrawal_requests),
            htr_consolidation_requests(input_ref.consolidation_requests),
            htr_builder_deposit_requests(input_ref.builder_deposit_requests),
            htr_builder_exit_requests(input_ref.builder_exit_requests),
        |],
        3,
    );
    cycle_scope_end(SCOPE_HTR_EXECUTION_REQUESTS);
    root
}

/*! `hash_tree_root` of the `SszNewPayloadRequest` (4 fields, depth 2) —
    the commitment the guest proves its input against. */
function htr_new_payload_request(input_ref : StatelessInputRef) -> hash = {
    merkleize(
        [|
            htr_execution_payload(input_ref),
            htr_versioned_hashes(input_ref.versioned_hashes),
            htr_bytes32(ssz_bytes32(input_ref.new_payload_request, NPR_BEACON_ROOT)),
            htr_execution_requests(input_ref),
        |],
        2,
    )
}
