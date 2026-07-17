
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

/*! A streaming Merkle frontier together with the number of leaves already
    incorporated. */
struct MerkleAccumulator = {
    frontier : list(MerkleSlot),
    count : item_count,
}

/*md
## Constants

The limits and byte offsets below are the SSZ dimensions of execution-layer
requests and their constituent fields.
*/
let EMPTY_MERKLE_ACCUMULATOR : MerkleAccumulator = struct { frontier = [||], count = 0 }

let HTR_BYTE_LIST_LIMIT : byte_length = ByteQuantity(1073741824)
let HTR_DEPOSIT_LENGTH : byte_length = ByteQuantity(192)
let HTR_DEPOSIT_PUBKEY : source_pointer = BYTE_ZERO
let HTR_DEPOSIT_PUBKEY_LENGTH : byte_length = ByteQuantity(48)
let HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS : source_pointer = ByteQuantity(48)
let HTR_DEPOSIT_AMOUNT : source_pointer = ByteQuantity(80)
let HTR_DEPOSIT_SIGNATURE : source_pointer = ByteQuantity(88)
let HTR_DEPOSIT_SIGNATURE_LENGTH : byte_length = ByteQuantity(96)
let HTR_DEPOSIT_INDEX : source_pointer = ByteQuantity(184)
let HTR_WITHDRAWAL_REQUEST_LENGTH : byte_length = ByteQuantity(76)
let HTR_REQUEST_SOURCE_ADDRESS : source_pointer = BYTE_ZERO
let HTR_REQUEST_SOURCE_PUBKEY : source_pointer = ByteQuantity(20)
let HTR_REQUEST_PUBKEY_LENGTH : byte_length = ByteQuantity(48)
let HTR_WITHDRAWAL_REQUEST_AMOUNT : source_pointer = ByteQuantity(68)
let HTR_CONSOLIDATION_REQUEST_LENGTH : byte_length = ByteQuantity(116)
let HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY : source_pointer = ByteQuantity(68)

/* Add leaf number `count` to a low-to-high frontier. A set bit in count has
 * one occupied slot at that level; carries consume those slots head-first. */
/*! Adds leaf number `count` to a low-to-high frontier. A set bit in
    `count` has one occupied slot at that level; carries consume those
    slots head-first. */
function merkle_push(frontier : list(MerkleSlot), count : item_count, leaf : hash) -> list(MerkleSlot) =
    if tmod_int(count, 2) == 0 then {
        match frontier {
            _ :: rest => OccupiedMerkleSlot(leaf) :: rest,
            [||] => [|OccupiedMerkleSlot(leaf)|],
        }
    } else {
        match frontier {
            OccupiedMerkleSlot(left) :: rest => EmptyMerkleSlot() ::
                merkle_push(rest, protocol_quantity_quotient(count, 2), sha256_pair(left, leaf)),
            EmptyMerkleSlot() :: _ => throw InvalidBlock(WitnessDeficient),
            [||] => throw InvalidBlock(WitnessDeficient),
        }
    }

/*! Adds one leaf to a streaming Merkle accumulator. */
function merkle_accumulator_push(accumulator : MerkleAccumulator, leaf : hash) -> MerkleAccumulator =
    if accumulator.count < 2 ^ 64 - 1 then {
        struct {
            frontier = merkle_push(accumulator.frontier, accumulator.count, leaf),
            count = item_count_increment(accumulator.count),
        }
    } else {
        throw InvalidBlock(WitnessDeficient)
    }

/* Pad a frontier to 2^depth leaves. `zero` advances from the zero chunk to
 * the zero subtree for the current level, so padding is O(depth). */
/*! Completes a frontier with zero subtrees through `remaining` levels and
    returns its Merkle root. */
function merkle_root_levels(
    slots : list(MerkleSlot),
    n : item_count,
    zero : hash,
    acc : hash,
    remaining : merkle_depth,
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
        merkle_root_levels(rest, protocol_quantity_quotient(n, 2), sha256_pair(zero, zero), next_acc, remaining - 1)
    }
termination_measure merkle_root_levels(slots, n, zero, acc, remaining) = remaining

function merkle_accumulator_root(accumulator : MerkleAccumulator, depth : merkle_depth) -> hash =
    merkle_root_levels(accumulator.frontier, accumulator.count, sail_zeros(256), sail_zeros(256), depth)

/*! Folds a leaf list into an existing Merkle accumulator in source order. */
function merkle_accumulate(leaves : list(hash), accumulator : MerkleAccumulator) -> MerkleAccumulator =
    match leaves {
        [||] => accumulator,
        leaf :: rest => merkle_accumulate(rest, merkle_accumulator_push(accumulator, leaf)),
    }

/*! `merkleize` of a fixed leaf list at `depth` (SSZ spec). */
function merkleize(leaves : list(hash), depth : merkle_depth) -> hash =
    merkle_accumulator_root(merkle_accumulate(leaves, EMPTY_MERKLE_ACCUMULATOR), depth)

/* ---- basic types and byte sequences ----------------------------------- */

/* SSZ 64-bit unsigned integer -> 32-byte chunk (little-endian in low 8 bytes) */
/*! Encodes a protocol quantity as an SSZ little-endian basic-value chunk. */
function htr_uint(v : protocol_quantity) -> hash = {
    let value = word_of_nat(v);
    var acc : hash = sail_zeros(256);
    foreach (k from 0 to 7) {
        let byte = sail_shiftright(value, 8 * k)[7 .. 0];
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(byte, 256), 8 * (31 - k)))
    };
    acc
}

/*! `uint256` as a 32-byte little-endian chunk. */
function htr_u256(w : word) -> hash = {
    var acc : hash = sail_zeros(256);
    foreach (k from 0 to 31) {
        let byte = sail_shiftright(w, 8 * k)[7 .. 0];
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(byte, 256), 8 * (31 - k)))
    };
    acc
}

/*! `Bytes32` chunks are themselves. */
function htr_bytes32(b : bits(256)) -> hash = b
/*! A 20-byte address, left-aligned in its chunk. */
function htr_addr(a : address) -> hash = sail_shiftleft(sail_zero_extend(a, 256), 96)

/*! `mix_in_length`: hashes the list root with its length chunk. */
function mix_in_length(root : hash, len : item_count) -> hash = sha256_pair(root, htr_uint(len))

/*! The ceiling of log2 — the Merkle depth of an `n`-chunk capacity. */
function clog2(n : item_count) -> merkle_depth = {
    var remaining : item_count = if n == 0 then {
            0
        } else {
            n - 1
        };
    var depth : merkle_depth = 0;
    foreach (_step from 0 to 63) {
        if remaining != 0 then {
            depth = merkle_depth_increment(depth);
            remaining = protocol_quantity_quotient(remaining, 2)
        }
    };
    depth
}

/*! The `chunk_index`-th 32-byte chunk of a byte span, zero-padded. */
function htr_chunk(bytes : ByteSlice, chunk_index : item_index) -> hash = {
    var acc : hash = sail_zeros(256);
    let start : source_pointer = ByteQuantity(chunk_index) * WORD_BYTE_LENGTH;
    var cursor : source_pointer = start;
    foreach (byte_index from 0 to 31) {
        let b : byte = if cursor < bytes.len then {
                slice_byte(bytes, cursor)
            } else {
                0x00
            };
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(b, 256));
        if byte_index < 31 then {
            cursor = cursor + BYTE_ONE
        }
    };
    acc
}

/*! Returns the number of 32-byte chunks needed to contain `byte_len` bytes. */
function htr_chunk_count(ByteQuantity(byte_len) : byte_length) -> item_count = {
    let count = exact_quotient(byte_len + 31, 32);
    if count <= 2 ^ 64 - 1 then {
        count
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! Returns the number of fixed-width items in an exactly divisible byte span. */
function htr_fixed_count(ByteQuantity(byte_len) : byte_length, ByteQuantity(item_len) : byte_length) -> item_count = {
    let count = exact_quotient(byte_len, item_len);
    if count <= 2 ^ 64 - 1 then {
        count
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*! The Merkle root of a byte span's chunks at `depth`. */
function htr_bytes_root(bytes : ByteSlice, depth : merkle_depth) -> hash = {
    let count = htr_chunk_count(bytes.len);
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    var index : item_index = 0;
    while index < count termination_measure(count - index) do {
        accumulator = merkle_accumulator_push(accumulator, htr_chunk(bytes, index));
        index = item_index_increment(index)
    };
    merkle_accumulator_root(accumulator, depth)
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
function htr_bytelist(bytes : ByteSlice, limit_bytes : byte_length) -> hash = {
    let capacity = htr_chunk_count(limit_bytes);
    let ByteQuantity(byte_len) = bytes.len;
    if byte_len <= 2 ^ 64 - 1 then {
        mix_in_length(htr_bytes_root(bytes, clog2(capacity)), byte_len)
    } else {
        throw InvalidBlock(InvalidConfig)
    }
}

/*md
## The execution payload
*/
/*! `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). */
function htr_withdrawal(withdrawal : ByteSlice) -> hash = {
    merkleize(
        [|
            htr_uint(ssz_uint(withdrawal, WD_INDEX)),
            htr_uint(ssz_uint(withdrawal, WD_VALIDATOR_INDEX)),
            htr_addr(ssz_addr(withdrawal, WD_ADDRESS)),
            htr_uint(ssz_uint(withdrawal, WD_AMOUNT)),
        |],
        2,
    )
}

/*! `hash_tree_root` of the transactions list
    (`List[ByteList[2^30], 2^20]`). */
function htr_transactions(transactions : SszListRef) -> hash = {
    var cursor = ssz_list_cursor(transactions);
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while not_bool(ssz_list_cursor_empty(cursor)) termination_measure(cursor.items.count - cursor.index) do {
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        accumulator = merkle_accumulator_push(accumulator, htr_bytelist(transaction, HTR_BYTE_LIST_LIMIT))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 20), accumulator.count)
}

/*! `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). */
function htr_withdrawals(withdrawals : SszListRef) -> hash = {
    var rest : SszListRef = withdrawals;
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while rest.count != 0 termination_measure(rest.count) do {
        let (withdrawal, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_withdrawal(withdrawal))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 4), accumulator.count)
}

/*! `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). */
function htr_execution_payload(input_ref : StatelessInputRef) -> hash = {
    let payload = input_ref.execution_payload;
    merkleize(
        [|
            htr_bytes32(ssz_bytes32(payload, BYTE_ZERO)),
            htr_addr(ssz_addr(payload, PL_FEE_RECIPIENT)),
            htr_bytes32(ssz_bytes32(payload, PL_STATE_ROOT)),
            htr_bytes32(ssz_bytes32(payload, PL_RECEIPTS_ROOT)),
            htr_bytevector(sub_slice(payload, PL_LOGS_BLOOM, LOGS_BLOOM_BYTE_LENGTH)),
            htr_bytes32(ssz_bytes32(payload, PL_PREV_RANDAO)),
            htr_uint(ssz_uint(payload, PL_BLOCK_NUMBER)),
            htr_uint(ssz_uint(payload, PL_GAS_LIMIT)),
            htr_uint(ssz_uint(payload, PL_GAS_USED)),
            htr_uint(ssz_uint(payload, PL_TIMESTAMP)),
            htr_bytelist(input_ref.extra_data, WORD_BYTE_LENGTH),
            htr_u256(ssz_u256(payload, PL_BASE_FEE)),
            htr_bytes32(ssz_bytes32(payload, PL_BLOCK_HASH)),
            htr_transactions(input_ref.transactions),
            htr_withdrawals(input_ref.withdrawals),
            htr_uint(ssz_uint(payload, PL_BLOB_GAS_USED)),
            htr_uint(ssz_uint(payload, PL_EXCESS_BLOB_GAS)),
            htr_bytelist(input_ref.block_access_list, HTR_BYTE_LIST_LIMIT),
            htr_uint(ssz_uint(payload, PL_SLOT_NUMBER)),
        |],
        5,
    )
}

/*! `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). */
function htr_versioned_hashes(versioned_hashes : ByteSlice) -> hash = {
    let count = htr_fixed_count(versioned_hashes.len, WORD_BYTE_LENGTH);
    var rest : SszListRef = struct { bytes = versioned_hashes, count = count };
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while rest.count != 0 termination_measure(rest.count) do {
        let (versioned_hash, tail) = ssz_fixed_list_pop(rest, WORD_BYTE_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, slice_load(versioned_hash, BYTE_ZERO))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 12), accumulator.count)
}

/*md
## The execution requests

The EIP-7685 execution-request commitments.
*/
/*! `hash_tree_root` of one deposit request (5 fields, depth 3). */
function htr_deposit(deposit : ByteSlice) -> hash = {
    merkleize(
        [|
            htr_bytevector(sub_slice(deposit, HTR_DEPOSIT_PUBKEY, HTR_DEPOSIT_PUBKEY_LENGTH)),
            htr_bytes32(ssz_bytes32(deposit, HTR_DEPOSIT_WITHDRAWAL_CREDENTIALS)),
            htr_uint(ssz_uint(deposit, HTR_DEPOSIT_AMOUNT)),
            htr_bytevector(sub_slice(deposit, HTR_DEPOSIT_SIGNATURE, HTR_DEPOSIT_SIGNATURE_LENGTH)),
            htr_uint(ssz_uint(deposit, HTR_DEPOSIT_INDEX)),
        |],
        3,
    )
}

/*! `hash_tree_root` of one withdrawal request (3 fields, depth 2). */
function htr_withdrawal_request(request : ByteSlice) -> hash = {
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_ADDRESS, ADDRESS_BYTE_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
            htr_uint(ssz_uint(request, HTR_WITHDRAWAL_REQUEST_AMOUNT)),
        |],
        2,
    )
}

/*! `hash_tree_root` of one consolidation request (3 fields, depth 2). */
function htr_consolidation_request(request : ByteSlice) -> hash =
    merkleize(
        [|
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_ADDRESS, ADDRESS_BYTE_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_REQUEST_SOURCE_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
            htr_bytevector(sub_slice(request, HTR_CONSOLIDATION_REQUEST_TARGET_PUBKEY, HTR_REQUEST_PUBKEY_LENGTH)),
        |],
        2,
    )

/*! `hash_tree_root` of the deposit-request list (depth 13). */
function htr_deposits(deposits : ByteSlice) -> hash = {
    let count = htr_fixed_count(deposits.len, HTR_DEPOSIT_LENGTH);
    var rest : SszListRef = struct { bytes = deposits, count = count };
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while rest.count != 0 termination_measure(rest.count) do {
        let (deposit, tail) = ssz_fixed_list_pop(rest, HTR_DEPOSIT_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_deposit(deposit))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 13), accumulator.count)
}

/*! `hash_tree_root` of the withdrawal-request list (depth 4). */
function htr_withdrawal_requests(requests : ByteSlice) -> hash = {
    let count = htr_fixed_count(requests.len, HTR_WITHDRAWAL_REQUEST_LENGTH);
    var rest : SszListRef = struct { bytes = requests, count = count };
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_WITHDRAWAL_REQUEST_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_withdrawal_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 4), accumulator.count)
}

/*! `hash_tree_root` of the consolidation-request list (depth 1). */
function htr_consolidation_requests(requests : ByteSlice) -> hash = {
    let count = htr_fixed_count(requests.len, HTR_CONSOLIDATION_REQUEST_LENGTH);
    var rest : SszListRef = struct { bytes = requests, count = count };
    var accumulator = EMPTY_MERKLE_ACCUMULATOR;
    while rest.count != 0 termination_measure(rest.count) do {
        let (request, tail) = ssz_fixed_list_pop(rest, HTR_CONSOLIDATION_REQUEST_LENGTH);
        rest = tail;
        accumulator = merkle_accumulator_push(accumulator, htr_consolidation_request(request))
    };
    mix_in_length(merkle_accumulator_root(accumulator, 1), accumulator.count)
}

/*! `hash_tree_root` of the `SszExecutionRequests` container. */
function htr_execution_requests(input_ref : StatelessInputRef) -> hash =
    merkleize(
        [|
            htr_deposits(input_ref.deposits),
            htr_withdrawal_requests(input_ref.withdrawal_requests),
            htr_consolidation_requests(input_ref.consolidation_requests),
        |],
        2,
    )

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
