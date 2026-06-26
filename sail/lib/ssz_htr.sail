/* ======================================================================== */
/* SSZ hash_tree_root: consensus-layer merkleization (SHA-256)              */
/*                                                                          */
/* The consensus layer commits to the execution payload by SSZ              */
/* hash_tree_root (the SimpleSerialize merkleization), distinct from the    */
/* execution layer's keccak/RLP/Merkle-Patricia commitments. This module    */
/* computes one such root, new_payload_request_root, over the               */
/* SszNewPayloadRequest. It is part of the zkVM block harness (zkvm/), not  */
/* the core EVM state transition.                                           */
/*                                                                          */
/* Per the SSZ spec: a basic type packs into a 32-byte little-endian chunk; */
/* a vector/list/container merkleizes its chunk / element / field roots     */
/* into a binary tree padded to a power-of-two leaf count; a list           */
/* additionally mixes in its length (mix_in_length). The huge declared      */
/* limits (2^20 transactions, ...) never materialize: a PRUNED subtree walk */
/* over a precomputed zero-hash table makes an N-element list cost          */
/* O(N*depth) hashes, not O(2^depth).                                       */
/* ======================================================================== */

/* zero-hash table: zh[0] = 0 chunk, zh[d] = sha256(zh[d-1] ++ zh[d-1]) */
function build_zerohashes(maxd : int) -> list(bits(256)) = {
    var zs : list(bits(256)) = [|sail_zeros(256)|];
    var prev : bits(256) = sail_zeros(256);
    foreach (d from 1 to maxd) {
        let cur = sha256_pair(prev, prev);
        zs = list_append(zs, [|cur|]);
        prev = cur
    };
    zs
}
function zh_at(zh : list(bits(256)), d : int) -> bits(256) = list_nth(zh, d, sail_zeros(256))

/* root of a 2^depth-leaf subtree whose actual leaves are leaves[0 .. nleaves-1];
 * any subtree entirely past nleaves collapses to a zero hash (the pruning that
 * makes 2^20-leaf trees cheap). */
function merkle_subtree(
    zh : list(bits(256)),
    leaves : list(bits(256)),
    nleaves : int,
    base : int,
    depth : int,
) -> (
    bits(256)
) = {
    if base >= nleaves then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            list_nth(leaves, base, sail_zeros(256))
        } else {
            let half = pow2(depth - 1);
            let l = merkle_subtree(zh, leaves, nleaves, base, depth - 1);
            let r = merkle_subtree(zh, leaves, nleaves, base + half, depth - 1);
            sha256_pair(l, r)
        }
    }
}
termination_measure merkle_subtree(zh, leaves, nleaves, base, depth) = depth

function merkleize(zh : list(bits(256)), leaves : list(bits(256)), nleaves : int, depth : int) -> bits(256) =
    merkle_subtree(zh, leaves, nleaves, 0, depth)

/* ---- leaf packing for basic types ------------------------------------- */

/* uint64 -> 32-byte chunk (value little-endian in the low 8 bytes) */
function htr_u64(v : int) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    foreach (k from 0 to 7) {
        let byte = emod_int(ediv_int(v, pow2(8 * k)), 256);
        acc = or_vec(acc, sail_shiftleft(get_slice_int(256, byte, 0), 8 * (31 - k)))
    };
    acc
}

/* uint256 word -> 32-byte little-endian chunk (byteswap of the big-endian word) */
function htr_u256(w : word) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    foreach (k from 0 to 31) {
        let byte = sail_shiftright(w, 8 * k)[7 .. 0];
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(byte, 256), 8 * (31 - k)))
    };
    acc
}

/* Bytes32 chunk is the 32 bytes themselves (already big-endian) */
function htr_bytes32(b : bits(256)) -> bits(256) = b

/* ByteVector[20] (an address): 20 bytes in the high bytes, 12 zero in the low */
function htr_addr(a : address) -> bits(256) = sail_shiftleft(sail_zero_extend(a, 256), 96)

/* mix_in_length(root, length) = sha256(root ++ length-as-uint256-chunk) */
function mix_in_length(root : bits(256), len : int) -> bits(256) =
    sha256_pair(root, htr_u64(len))

/* ceil(log2(n)), min 0 (tree depth for a chunk/element capacity of n) */
function clog2(n : int) -> int = {
    var d : int = 0;
    var c : int = 1;
    while c < n do {
        c = c * 2;
        d = d + 1
    };
    d
}

/* the j-th 32-byte chunk of `len` bytes at input offset `start` (zero-padded) */
function chunk_be(start : int, len : int, j : int) -> bits(256) = {
    var acc : bits(256) = sail_zeros(256);
    foreach (i from 0 to 31) {
        let idx = j * 32 + i;
        let byte : bits(8) = if idx < len then {
                ssz_src_byte(start + idx)
            } else {
                0x00
            };
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(byte, 256))
    };
    acc
}

/* On-demand merkle root over the packed 32-byte chunks of `len` bytes at input
 * offset `start`: a 2^depth-leaf tree whose real leaves are chunks 0..nchunks-1
 * (chunks past nchunks collapse to a zero hash). Each leaf is read from the
 * input only when the walk reaches it (chunk_be), so an N-chunk subtree costs
 * O(depth) stack and O(N*depth) work, never building an N-element leaf list ---
 * the same on-demand discipline as the witness node walk in lib/mpt.sail. */
function merkle_bytes(
    zh : list(bits(256)),
    start : int,
    len : int,
    nchunks : int,
    base : int,
    depth : int,
) -> (
    bits(256)
) = {
    if base >= nchunks then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            chunk_be(start, len, base)
        } else {
            let half = pow2(depth - 1);
            let l = merkle_bytes(zh, start, len, nchunks, base, depth - 1);
            let r = merkle_bytes(zh, start, len, nchunks, base + half, depth - 1);
            sha256_pair(l, r)
        }
    }
}
termination_measure merkle_bytes(zh, start, len, nchunks, base, depth) = depth

/* htr of a fixed ByteVector of nbytes at `start` (merkleize its chunks) */
function htr_bytevector(zh : list(bits(256)), start : int, nbytes : int) -> bits(256) = {
    let nchunks = ediv_int(nbytes + 31, 32);
    merkle_bytes(zh, start, nbytes, nchunks, 0, clog2(nchunks))
}

/* htr of a ByteList (len bytes at `start`, capacity limit_bytes): merkleize the
 * packed chunks to the capacity's chunk-tree depth, then mix in the length */
function htr_bytelist(zh : list(bits(256)), start : int, len : int, limit_bytes : int) -> bits(256) = {
    let nchunks = ediv_int(len + 31, 32);
    let cap = ediv_int(limit_bytes + 31, 32);
    mix_in_length(merkle_bytes(zh, start, len, nchunks, 0, clog2(cap)), len)
}

/* ---- containers / lists we need --------------------------------------- */

/* SszWithdrawal { index:u64, validator_index:u64, address[20], amount:u64 } */
function htr_withdrawal(zh : list(bits(256)), i : int) -> bits(256) = {
    let b = pl_wds_start() + i * 44;
    let leaves : list(bits(256)) = [|
            htr_u64(ssz_u64(b + 0)),
            htr_u64(ssz_u64(b + 8)),
            htr_addr(ssz_addr(b + 16)),
            htr_u64(ssz_u64(b + 36)),
        |];
    merkleize(zh, leaves, 4, 2)
}

/* on-demand merkle root over the n transactions: leaf i is the htr of the i-th
 * tx ByteList, computed when the walk reaches it (no n-element leaf list). */
function merkle_txs(zh : list(bits(256)), n : int, base : int, depth : int) -> bits(256) = {
    if base >= n then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            let (s, l) = ssz_list_span(pl_txs_start(), n, pl_wds_start(), base);
            htr_bytelist(zh, s, l, 1073741824)
        } else {
            let half = pow2(depth - 1);
            let lft = merkle_txs(zh, n, base, depth - 1);
            let rgt = merkle_txs(zh, n, base + half, depth - 1);
            sha256_pair(lft, rgt)
        }
    }
}
termination_measure merkle_txs(zh, n, base, depth) = depth

/* transactions: List[ByteList[2^30], 2^20] -> mix_in_length(merkleize(htrs), n) */
function htr_transactions(zh : list(bits(256))) -> bits(256) = {
    let n = pl_n_txs();
    mix_in_length(merkle_txs(zh, n, 0, 20), n)
}

/* on-demand merkle root over the n withdrawals: leaf i = htr_withdrawal(i). */
function merkle_withdrawals(zh : list(bits(256)), n : int, base : int, depth : int) -> bits(256) = {
    if base >= n then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            htr_withdrawal(zh, base)
        } else {
            let half = pow2(depth - 1);
            let lft = merkle_withdrawals(zh, n, base, depth - 1);
            let rgt = merkle_withdrawals(zh, n, base + half, depth - 1);
            sha256_pair(lft, rgt)
        }
    }
}
termination_measure merkle_withdrawals(zh, n, base, depth) = depth

/* withdrawals: List[SszWithdrawal, 16] */
function htr_withdrawals_list(zh : list(bits(256))) -> bits(256) = {
    let n = pl_n_withdrawals();
    mix_in_length(merkle_withdrawals(zh, n, 0, 4), n)
}

/* SszExecutionPayload: 19 fields -> depth 5 (next pow2 of 19 = 32) */
function htr_execution_payload(zh : list(bits(256))) -> bits(256) = {
    let p = payload_start();
    let extra_start = p + ssz_u32(p + PL_EXTRA_OFF);
    let extra_len = pl_txs_start() - extra_start;
    let bal_start = p + ssz_u32(p + PL_BAL_OFF);
    let bal_len = payload_end() - bal_start;
    let leaves : list(bits(256)) = [|
            htr_bytes32(ssz_bytes32(p + 0)), /* parent_hash    */
            htr_addr(ssz_addr(p + PL_FEE_RECIPIENT)), /* fee_recipient  */
            htr_bytes32(ssz_bytes32(p + PL_STATE_ROOT)), /* state_root     */
            htr_bytes32(ssz_bytes32(p + 84)), /* receipts_root  */
            htr_bytevector(zh, p + 116, 256), /* logs_bloom     */
            htr_bytes32(ssz_bytes32(p + PL_PREV_RANDAO)), /* prev_randao  */
            htr_u64(ssz_u64(p + PL_BLOCK_NUMBER)),
            htr_u64(ssz_u64(p + PL_GAS_LIMIT)),
            htr_u64(ssz_u64(p + PL_GAS_USED)),
            htr_u64(ssz_u64(p + PL_TIMESTAMP)),
            htr_bytelist(zh, extra_start, extra_len, 32), /* extra_data */
            htr_u256(ssz_u256(p + PL_BASE_FEE)), /* base_fee_per_gas */
            htr_bytes32(ssz_bytes32(p + PL_BLOCK_HASH)), /* block_hash */
            htr_transactions(zh),
            htr_withdrawals_list(zh),
            htr_u64(ssz_u64(p + 512)), /* blob_gas_used   */
            htr_u64(ssz_u64(p + 520)), /* excess_blob_gas */
            htr_bytelist(zh, bal_start, bal_len, 1073741824), /* block_access_list ByteList[MAX_BYTES_PER_TRANSACTION = 2^30] */
            htr_u64(ssz_u64(p + 532)), /* slot_number     */
        |];
    merkleize(zh, leaves, 19, 5)
}

/* on-demand merkle root over cnt Bytes32 versioned hashes: leaf i is the 32-byte
 * chunk at vh_start + i*32, read when the walk reaches it. */
function merkle_vhashes(zh : list(bits(256)), vh_start : int, cnt : int, base : int, depth : int) -> bits(256) = {
    if base >= cnt then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            ssz_bytes32(vh_start + base * 32)
        } else {
            let half = pow2(depth - 1);
            let lft = merkle_vhashes(zh, vh_start, cnt, base, depth - 1);
            let rgt = merkle_vhashes(zh, vh_start, cnt, base + half, depth - 1);
            sha256_pair(lft, rgt)
        }
    }
}
termination_measure merkle_vhashes(zh, vh_start, cnt, base, depth) = depth

/* versioned_hashes: List[Bytes32, 4096] (each element is one chunk) */
function htr_versioned_hashes(zh : list(bits(256))) -> bits(256) = {
    let n = npr_start();
    let vh_start = n + ssz_u32(n + 4);
    let vh_end = n + ssz_u32(n + 40);
    let cnt = ediv_int(vh_end - vh_start, 32);
    mix_in_length(merkle_vhashes(zh, vh_start, cnt, 0, 12), cnt)
}

/* SszExecutionRequests is a container of three variable lists (offsets at
 * req+{0,4,8}); execution_requests is field 3 of SszNewPayloadRequest, whose
 * offset sits at npr+40 (after the 32-byte parent_beacon_block_root @ npr+8). */
function req_start() -> int = npr_start() + ssz_u32(npr_start() + 40)
function dep_start() -> int = req_start() + ssz_u32(req_start() + 0)
function wreq_start() -> int = req_start() + ssz_u32(req_start() + 4)
function creq_start() -> int = req_start() + ssz_u32(req_start() + 8)
function req_end() -> int = npr_end() /* execution_requests is the last npr field */

/* SszDepositRequest { pubkey[48], withdrawal_credentials[32], amount:u64,
 * signature[96], index:u64 } -> 192 bytes, 5 fields (depth 3). */
function htr_deposit(zh : list(bits(256)), i : int) -> bits(256) = {
    let b = dep_start() + i * 192;
    let leaves : list(bits(256)) = [|
            htr_bytevector(zh, b + 0, 48),
            htr_bytes32(ssz_bytes32(b + 48)),
            htr_u64(ssz_u64(b + 80)),
            htr_bytevector(zh, b + 88, 96),
            htr_u64(ssz_u64(b + 184)),
        |];
    merkleize(zh, leaves, 5, 3)
}

/* SszWithdrawalRequest { source_address[20], validator_pubkey[48], amount:u64 }
 * -> 76 bytes, 3 fields (depth 2). */
function htr_wreq(zh : list(bits(256)), i : int) -> bits(256) = {
    let b = wreq_start() + i * 76;
    let leaves : list(bits(256)) = [|
            htr_bytevector(zh, b + 0, 20),
            htr_bytevector(zh, b + 20, 48),
            htr_u64(ssz_u64(b + 68)),
        |];
    merkleize(zh, leaves, 3, 2)
}

/* SszConsolidationRequest { source_address[20], source_pubkey[48],
 * target_pubkey[48] } -> 116 bytes, 3 fields (depth 2). */
function htr_creq(zh : list(bits(256)), i : int) -> bits(256) = {
    let b = creq_start() + i * 116;
    let leaves : list(bits(256)) = [|
            htr_bytevector(zh, b + 0, 20),
            htr_bytevector(zh, b + 20, 48),
            htr_bytevector(zh, b + 68, 48),
        |];
    merkleize(zh, leaves, 3, 2)
}

function merkle_deposits(zh : list(bits(256)), n : int, base : int, depth : int) -> bits(256) = {
    if base >= n then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            htr_deposit(zh, base)
        } else {
            let half = pow2(depth - 1);
            sha256_pair(merkle_deposits(zh, n, base, depth - 1), merkle_deposits(zh, n, base + half, depth - 1))
        }
    }
}
termination_measure merkle_deposits(zh, n, base, depth) = depth

function merkle_wreqs(zh : list(bits(256)), n : int, base : int, depth : int) -> bits(256) = {
    if base >= n then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            htr_wreq(zh, base)
        } else {
            let half = pow2(depth - 1);
            sha256_pair(merkle_wreqs(zh, n, base, depth - 1), merkle_wreqs(zh, n, base + half, depth - 1))
        }
    }
}
termination_measure merkle_wreqs(zh, n, base, depth) = depth

function merkle_creqs(zh : list(bits(256)), n : int, base : int, depth : int) -> bits(256) = {
    if base >= n then {
        zh_at(zh, depth)
    } else {
        if depth == 0 then {
            htr_creq(zh, base)
        } else {
            let half = pow2(depth - 1);
            sha256_pair(merkle_creqs(zh, n, base, depth - 1), merkle_creqs(zh, n, base + half, depth - 1))
        }
    }
}
termination_measure merkle_creqs(zh, n, base, depth) = depth

/* SszExecutionRequests: deposits List[_, 2^13], withdrawals List[_, 2^4],
 * consolidations List[_, 2^1]; element counts from each list's byte span. */
function htr_execution_requests(zh : list(bits(256))) -> bits(256) = {
    let n_dep = ediv_int(wreq_start() - dep_start(), 192);
    let n_wreq = ediv_int(creq_start() - wreq_start(), 76);
    let n_creq = ediv_int(req_end() - creq_start(), 116);
    let leaves : list(bits(256)) = [|
            mix_in_length(merkle_deposits(zh, n_dep, 0, 13), n_dep),
            mix_in_length(merkle_wreqs(zh, n_wreq, 0, 4), n_wreq),
            mix_in_length(merkle_creqs(zh, n_creq, 0, 1), n_creq),
        |];
    merkleize(zh, leaves, 3, 2)
}

/* SszNewPayloadRequest: 4 fields -> depth 2. This is new_payload_request_root. */
function htr_new_payload_request(zh : list(bits(256))) -> bits(256) = {
    let leaves : list(bits(256)) = [|
            htr_execution_payload(zh),
            htr_versioned_hashes(zh),
            htr_bytes32(ssz_bytes32(npr_start() + 8)), /* parent_beacon_block_root */
            htr_execution_requests(zh),
        |];
    merkleize(zh, leaves, 4, 2)
}
