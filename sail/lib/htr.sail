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
    EmptyMerkleSlot : unit,
    OccupiedMerkleSlot : hash,
}

/*! Adds leaf number `count` to a low-to-high frontier. A set bit in
    `count` has one occupied slot at that level; carries consume those
    slots head-first. */
function merkle_push(frontier : list(MerkleSlot), count : int, leaf : hash) -> list(MerkleSlot) =
    if emod_int(count, 2) == 0 then {
        match frontier {
            _ :: rest => OccupiedMerkleSlot(leaf) :: rest,
            [||] => [|OccupiedMerkleSlot(leaf)|],
        }
    } else {
        match frontier {
            OccupiedMerkleSlot(left) :: rest => EmptyMerkleSlot() ::
                merkle_push(rest, ediv_int(count, 2), sha256_pair(left, leaf)),
            EmptyMerkleSlot() :: _ => throw InvalidBlock(WitnessDeficient),
            [||] => throw InvalidBlock(WitnessDeficient),
        }
    }

/*! Pads a frontier to `2^depth` leaves and returns its root. `zero`
    advances from the zero chunk to the zero subtree for the current
    level, so padding is O(depth). */
function merkle_frontier_root(frontier : list(MerkleSlot), count : int, depth : int) -> hash = {
    if count > pow2(depth) then {
        throw InvalidBlock(WitnessDeficient)
    };
    var slots : list(MerkleSlot) = frontier;
    var n : int = count;
    var level : int = 0;
    var zero : hash = sail_zeros(256);
    var acc : hash = sail_zeros(256);
    while level < depth do {
        var slot : MerkleSlot = EmptyMerkleSlot();
        match slots {
            head :: tail => {
                slot = head;
                slots = tail
            },
            [||] => (),
        };
        if emod_int(n, 2) == 1 then {
            match slot {
                OccupiedMerkleSlot(left) => acc = sha256_pair(left, acc),
                EmptyMerkleSlot() => throw InvalidBlock(WitnessDeficient),
            }
        } else {
            acc = sha256_pair(acc, zero)
        };
        n = ediv_int(n, 2);
        zero = sha256_pair(zero, zero);
        level = level + 1
    };
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
}

/*! `merkleize` of a fixed leaf list at `depth` (SSZ spec). */
function merkleize(leaves : list(hash), depth : int) -> hash = {
    var rest : list(hash) = leaves;
    var frontier : list(MerkleSlot) = [||];
    var count : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            leaf :: tail => {
                frontier = merkle_push(frontier, count, leaf);
                count = count + 1;
                rest = tail
            },
            [||] => more = false,
        }
    };
    merkle_frontier_root(frontier, count, depth)
}

/*md
## Helpers
*/

/*! `uint64` as a 32-byte chunk (value little-endian in the low 8
    bytes). */
function htr_u64(v : int) -> hash = {
    var acc : hash = sail_zeros(256);
    foreach (k from 0 to 7) {
        let byte = emod_int(ediv_int(v, pow2(8 * k)), 256);
        acc = or_vec(acc, sail_shiftleft(get_slice_int(256, byte, 0), 8 * (31 - k)))
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
function mix_in_length(root : hash, len : int) -> hash = sha256_pair(root, htr_u64(len))

/*! The ceiling of log2 — the Merkle depth of an `n`-chunk capacity. */
function clog2(n : int) -> int = {
    var depth : int = 0;
    var capacity : int = 1;
    while capacity < n do {
        capacity = capacity * 2;
        depth = depth + 1
    };
    depth
}

/*! The `chunk_index`-th 32-byte chunk of a byte span, zero-padded. */
function htr_chunk(bytes : ByteSlice, chunk_index : int) -> hash = {
    var acc : hash = sail_zeros(256);
    let len = unsigned(bytes.len);
    foreach (i from 0 to 31) {
        let index = chunk_index * 32 + i;
        let b : byte = if index < len then {
                slice_byte(bytes, index)
            } else {
                0x00
            };
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(b, 256))
    };
    acc
}

/*! The Merkle root of a byte span's chunks at `depth`. */
function htr_bytes_root(bytes : ByteSlice, depth : int) -> hash = {
    let count = ediv_int(unsigned(bytes.len) + 31, 32);
    var frontier : list(MerkleSlot) = [||];
    var i : int = 0;
    while i < count do {
        frontier = merkle_push(frontier, i, htr_chunk(bytes, i));
        i = i + 1
    };
    merkle_frontier_root(frontier, count, depth)
}

/*! `hash_tree_root` of a fixed-size `ByteVector`. */
function htr_bytevector(bytes : ByteSlice) -> hash = {
    let chunks = ediv_int(unsigned(bytes.len) + 31, 32);
    htr_bytes_root(bytes, clog2(chunks))
}

/*! `hash_tree_root` of a `ByteList[limit_bytes]` (root at the limit's
    depth, length mixed in). */
function htr_bytelist(bytes : ByteSlice, limit_bytes : int) -> hash = {
    let capacity = ediv_int(limit_bytes + 31, 32);
    mix_in_length(htr_bytes_root(bytes, clog2(capacity)), unsigned(bytes.len))
}

/*md
## The execution payload
*/

/*! `hash_tree_root` of one SSZ withdrawal (4 fields, depth 2). */
function htr_withdrawal(withdrawal : ByteSlice) -> hash = {
    let b = unsigned(withdrawal.off);
    merkleize(
        [|
            htr_u64(ssz_u64(b + WD_INDEX)),
            htr_u64(ssz_u64(b + WD_VALIDATOR_INDEX)),
            htr_addr(ssz_addr(b + WD_ADDRESS)),
            htr_u64(ssz_u64(b + WD_AMOUNT)),
        |],
        2,
    )
}

/*! `hash_tree_root` of the transactions list
    (`List[ByteList[2^30], 2^20]`). */
function htr_transactions(transactions : SszListRef) -> hash = {
    var cursor = ssz_list_cursor(transactions);
    var frontier : list(MerkleSlot) = [||];
    var count : int = 0;
    while not_bool(ssz_list_cursor_empty(cursor)) do {
        let (transaction, next) = ssz_list_pop(cursor);
        cursor = next;
        frontier = merkle_push(frontier, count, htr_bytelist(transaction, 1073741824));
        count = count + 1
    };
    mix_in_length(merkle_frontier_root(frontier, count, 20), count)
}

/*! `hash_tree_root` of the withdrawals list (`List[SszWithdrawal, 16]`). */
function htr_withdrawals(withdrawals : SszListRef) -> hash = {
    var rest : SszListRef = withdrawals;
    var frontier : list(MerkleSlot) = [||];
    var count : int = 0;
    while rest.count > 0 do {
        let (withdrawal, tail) = ssz_fixed_list_pop(rest, WD_SIZE);
        rest = tail;
        frontier = merkle_push(frontier, count, htr_withdrawal(withdrawal));
        count = count + 1
    };
    mix_in_length(merkle_frontier_root(frontier, count, 4), count)
}

/*! `hash_tree_root` of the `SszExecutionPayload` (19 fields, depth 5). */
function htr_execution_payload(input_ref : StatelessInputRef) -> hash = {
    let p = unsigned(input_ref.execution_payload.off);
    merkleize(
        [|
            htr_bytes32(ssz_bytes32(p + 0)),
            htr_addr(ssz_addr(p + PL_FEE_RECIPIENT)),
            htr_bytes32(ssz_bytes32(p + PL_STATE_ROOT)),
            htr_bytes32(ssz_bytes32(p + PL_RECEIPTS_ROOT)),
            htr_bytevector(sub_slice(input_ref.execution_payload, PL_LOGS_BLOOM, 256)),
            htr_bytes32(ssz_bytes32(p + PL_PREV_RANDAO)),
            htr_u64(ssz_u64(p + PL_BLOCK_NUMBER)),
            htr_u64(ssz_u64(p + PL_GAS_LIMIT)),
            htr_u64(ssz_u64(p + PL_GAS_USED)),
            htr_u64(ssz_u64(p + PL_TIMESTAMP)),
            htr_bytelist(input_ref.extra_data, 32),
            htr_u256(ssz_u256(p + PL_BASE_FEE)),
            htr_bytes32(ssz_bytes32(p + PL_BLOCK_HASH)),
            htr_transactions(input_ref.transactions),
            htr_withdrawals(input_ref.withdrawals),
            htr_u64(ssz_u64(p + PL_BLOB_GAS_USED)),
            htr_u64(ssz_u64(p + PL_EXCESS_BLOB_GAS)),
            htr_bytelist(input_ref.block_access_list, 1073741824),
            htr_u64(ssz_u64(p + PL_SLOT_NUMBER)),
        |],
        5,
    )
}

/*! `hash_tree_root` of the versioned hashes (`List[Bytes32, 4096]`). */
function htr_versioned_hashes(versioned_hashes : ByteSlice) -> hash = {
    let count = ediv_int(unsigned(versioned_hashes.len), 32);
    var rest : SszListRef = struct { bytes = versioned_hashes, count = count };
    var frontier : list(MerkleSlot) = [||];
    var i : int = 0;
    while rest.count > 0 do {
        let (versioned_hash, tail) = ssz_fixed_list_pop(rest, 32);
        rest = tail;
        frontier = merkle_push(frontier, i, slice_load(versioned_hash, 0));
        i = i + 1
    };
    mix_in_length(merkle_frontier_root(frontier, i, 12), i)
}

/*md
## The execution requests

The EIP-7685 execution-request commitments.
*/

/*! `hash_tree_root` of one deposit request (5 fields, depth 3). */
function htr_deposit(deposit : ByteSlice) -> hash = {
    let b = unsigned(deposit.off);
    merkleize(
        [|
            htr_bytevector(sub_slice(deposit, 0, 48)),
            htr_bytes32(ssz_bytes32(b + 48)),
            htr_u64(ssz_u64(b + 80)),
            htr_bytevector(sub_slice(deposit, 88, 96)),
            htr_u64(ssz_u64(b + 184)),
        |],
        3,
    )
}

/*! `hash_tree_root` of one withdrawal request (3 fields, depth 2). */
function htr_withdrawal_request(request : ByteSlice) -> hash = {
    let b = unsigned(request.off);
    merkleize(
        [|
            htr_bytevector(sub_slice(request, 0, 20)),
            htr_bytevector(sub_slice(request, 20, 48)),
            htr_u64(ssz_u64(b + 68)),
        |],
        2,
    )
}

/*! `hash_tree_root` of one consolidation request (3 fields, depth 2). */
function htr_consolidation_request(request : ByteSlice) -> hash =
    merkleize(
        [|
            htr_bytevector(sub_slice(request, 0, 20)),
            htr_bytevector(sub_slice(request, 20, 48)),
            htr_bytevector(sub_slice(request, 68, 48)),
        |],
        2,
    )

/*! `hash_tree_root` of the deposit-request list (depth 13). */
function htr_deposits(deposits : ByteSlice) -> hash = {
    let count = ediv_int(unsigned(deposits.len), 192);
    var rest : SszListRef = struct { bytes = deposits, count = count };
    var frontier : list(MerkleSlot) = [||];
    var i : int = 0;
    while rest.count > 0 do {
        let (deposit, tail) = ssz_fixed_list_pop(rest, 192);
        rest = tail;
        frontier = merkle_push(frontier, i, htr_deposit(deposit));
        i = i + 1
    };
    mix_in_length(merkle_frontier_root(frontier, i, 13), i)
}

/*! `hash_tree_root` of the withdrawal-request list (depth 4). */
function htr_withdrawal_requests(requests : ByteSlice) -> hash = {
    let count = ediv_int(unsigned(requests.len), 76);
    var rest : SszListRef = struct { bytes = requests, count = count };
    var frontier : list(MerkleSlot) = [||];
    var i : int = 0;
    while rest.count > 0 do {
        let (request, tail) = ssz_fixed_list_pop(rest, 76);
        rest = tail;
        frontier = merkle_push(frontier, i, htr_withdrawal_request(request));
        i = i + 1
    };
    mix_in_length(merkle_frontier_root(frontier, i, 4), i)
}

/*! `hash_tree_root` of the consolidation-request list (depth 1). */
function htr_consolidation_requests(requests : ByteSlice) -> hash = {
    let count = ediv_int(unsigned(requests.len), 116);
    var rest : SszListRef = struct { bytes = requests, count = count };
    var frontier : list(MerkleSlot) = [||];
    var i : int = 0;
    while rest.count > 0 do {
        let (request, tail) = ssz_fixed_list_pop(rest, 116);
        rest = tail;
        frontier = merkle_push(frontier, i, htr_consolidation_request(request));
        i = i + 1
    };
    mix_in_length(merkle_frontier_root(frontier, i, 1), i)
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
    let npr = unsigned(input_ref.new_payload_request.off);
    merkleize(
        [|
            htr_execution_payload(input_ref),
            htr_versioned_hashes(input_ref.versioned_hashes),
            htr_bytes32(ssz_bytes32(npr + NPR_BEACON_ROOT)),
            htr_execution_requests(input_ref),
        |],
        2,
    )
}
