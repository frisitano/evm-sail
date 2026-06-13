/* ======================================================================== */
/* SSZ hash_tree_root (merkleization, SHA-256) — produces the public           */
/* new_payload_request_root. Mirrors the SSZ spec: basic types pack into a      */
/* 32-byte little-endian chunk; vectors/lists/containers merkleize their chunk  */
/* / element / field roots; lists additionally mix_in_length. Huge list limits  */
/* (2^20 txs, ...) are handled by a PRUNED subtree walk + a zero-hash table, so */
/* an N-element list costs O(N*depth) hashes, not O(2^depth).                   */
/* ======================================================================== */

/* zero-hash table: zh[0] = 0 chunk, zh[d] = sha256(zh[d-1] ++ zh[d-1]) */
function build_zerohashes(maxd : int) -> list(bits(256)) = {
  var zs : list(bits(256)) = [| sail_zeros(256) |];
  var prev : bits(256) = sail_zeros(256);
  foreach (d from 1 to maxd) {
    let cur = sha256_pair(prev, prev);
    zs = list_append(zs, [| cur |]);
    prev = cur
  };
  zs
}
function zh_at(zh : list(bits(256)), d : int) -> bits(256) = list_nth(zh, d, sail_zeros(256))

/* root of a 2^depth-leaf subtree whose actual leaves are leaves[0 .. nleaves-1];
 * any subtree entirely past nleaves collapses to a zero hash (the pruning that
 * makes 2^20-leaf trees cheap). */
function merkle_subtree(zh : list(bits(256)), leaves : list(bits(256)),
                        nleaves : int, base : int, depth : int) -> bits(256) = {
  if base >= nleaves then zh_at(zh, depth)
  else if depth == 0 then list_nth(leaves, base, sail_zeros(256))
  else {
    let half = pow2(depth - 1);
    let l = merkle_subtree(zh, leaves, nleaves, base, depth - 1);
    let r = merkle_subtree(zh, leaves, nleaves, base + half, depth - 1);
    sha256_pair(l, r)
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
    let byte = (sail_shiftright(w, 8 * k))[7 .. 0];
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
  var d : int = 0; var c : int = 1;
  while c < n do { c = c * 2; d = d + 1 };
  d
}
/* the j-th 32-byte chunk of `len` bytes at input offset `start` (zero-padded) */
function chunk_be(start : int, len : int, j : int) -> bits(256) = {
  var acc : bits(256) = sail_zeros(256);
  foreach (i from 0 to 31) {
    let idx = j * 32 + i;
    let byte : bits(8) = if idx < len then ssz_src_byte(start + idx) else 0x00;
    acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(byte, 256))
  };
  acc
}
/* htr of a fixed ByteVector of nbytes at `start` (merkleize its chunks) */
function htr_bytevector(zh : list(bits(256)), start : int, nbytes : int) -> bits(256) = {
  let nchunks = ediv_int(nbytes + 31, 32);
  var leaves : list(bits(256)) = [||];
  foreach (j from 0 to (nchunks - 1)) { leaves = list_append(leaves, [| chunk_be(start, nbytes, j) |]) };
  merkleize(zh, leaves, nchunks, clog2(nchunks))
}
/* htr of a ByteList (len bytes at `start`, capacity limit_bytes): merkleize the
 * packed chunks to the capacity's chunk-tree depth, then mix in the length */
function htr_bytelist(zh : list(bits(256)), start : int, len : int, limit_bytes : int) -> bits(256) = {
  let nchunks = ediv_int(len + 31, 32);
  let cap     = ediv_int(limit_bytes + 31, 32);
  var leaves : list(bits(256)) = [||];
  foreach (j from 0 to (nchunks - 1)) { leaves = list_append(leaves, [| chunk_be(start, len, j) |]) };
  mix_in_length(merkleize(zh, leaves, nchunks, clog2(cap)), len)
}

/* ---- containers / lists we need --------------------------------------- */

/* SszWithdrawal { index:u64, validator_index:u64, address[20], amount:u64 } */
function htr_withdrawal(zh : list(bits(256)), i : int) -> bits(256) = {
  let b = pl_wds_start() + i * 44;
  let leaves : list(bits(256)) = [|
    htr_u64(ssz_u64(b + 0)),
    htr_u64(ssz_u64(b + 8)),
    htr_addr(ssz_addr(b + 16)),
    htr_u64(ssz_u64(b + 36))
  |];
  merkleize(zh, leaves, 4, 2)
}

/* transactions: List[ByteList[2^30], 2^20] -> mix_in_length(merkleize(htrs), n) */
function htr_transactions(zh : list(bits(256))) -> bits(256) = {
  let n = pl_n_txs();
  var leaves : list(bits(256)) = [||];
  foreach (i from 0 to (n - 1)) {
    let (s, l) = ssz_list_span(pl_txs_start(), n, pl_wds_start(), i);
    leaves = list_append(leaves, [| htr_bytelist(zh, s, l, 1073741824) |])
  };
  mix_in_length(merkleize(zh, leaves, n, 20), n)
}
/* withdrawals: List[SszWithdrawal, 16] */
function htr_withdrawals_list(zh : list(bits(256))) -> bits(256) = {
  let n = pl_n_withdrawals();
  var leaves : list(bits(256)) = [||];
  foreach (i from 0 to (n - 1)) { leaves = list_append(leaves, [| htr_withdrawal(zh, i) |]) };
  mix_in_length(merkleize(zh, leaves, n, 4), n)
}

/* SszExecutionPayload: 19 fields -> depth 5 (next pow2 of 19 = 32) */
function htr_execution_payload(zh : list(bits(256))) -> bits(256) = {
  let p = payload_start();
  let extra_start = p + ssz_u32(p + PL_EXTRA_OFF);
  let extra_len   = pl_txs_start() - extra_start;
  let bal_start   = p + ssz_u32(p + PL_BAL_OFF);
  let bal_len     = payload_end() - bal_start;
  let leaves : list(bits(256)) = [|
    htr_bytes32(ssz_bytes32(p + 0)),            /* parent_hash    */
    htr_addr(ssz_addr(p + PL_FEE_RECIPIENT)),   /* fee_recipient  */
    htr_bytes32(ssz_bytes32(p + PL_STATE_ROOT)),/* state_root     */
    htr_bytes32(ssz_bytes32(p + 84)),           /* receipts_root  */
    htr_bytevector(zh, p + 116, 256),           /* logs_bloom     */
    htr_bytes32(ssz_bytes32(p + PL_PREV_RANDAO)), /* prev_randao  */
    htr_u64(ssz_u64(p + PL_BLOCK_NUMBER)),
    htr_u64(ssz_u64(p + PL_GAS_LIMIT)),
    htr_u64(ssz_u64(p + PL_GAS_USED)),
    htr_u64(ssz_u64(p + PL_TIMESTAMP)),
    htr_bytelist(zh, extra_start, extra_len, 32),   /* extra_data */
    htr_u256(ssz_u256(p + PL_BASE_FEE)),            /* base_fee_per_gas */
    htr_bytes32(ssz_bytes32(p + PL_BLOCK_HASH)),    /* block_hash */
    htr_transactions(zh),
    htr_withdrawals_list(zh),
    htr_u64(ssz_u64(p + 512)),                      /* blob_gas_used   */
    htr_u64(ssz_u64(p + 520)),                      /* excess_blob_gas */
    htr_bytelist(zh, bal_start, bal_len, 1073741824), /* block_access_list */
    htr_u64(ssz_u64(p + 532))                       /* slot_number     */
  |];
  merkleize(zh, leaves, 19, 5)
}

/* versioned_hashes: List[Bytes32, 4096] (each element is one chunk) */
function htr_versioned_hashes(zh : list(bits(256))) -> bits(256) = {
  let n = npr_start();
  let vh_start = n + ssz_u32(n + 4);
  let vh_end   = n + ssz_u32(n + 40);
  let cnt = ediv_int(vh_end - vh_start, 32);
  var leaves : list(bits(256)) = [||];
  foreach (i from 0 to (cnt - 1)) { leaves = list_append(leaves, [| ssz_bytes32(vh_start + i * 32) |]) };
  mix_in_length(merkleize(zh, leaves, cnt, 12), cnt)
}

/* SszExecutionRequests: 3 empty lists in the fixture (deposits 2^13, withdrawals
 * 2^4, consolidations 2^1). Non-empty request lists are a documented follow-on. */
function htr_execution_requests(zh : list(bits(256))) -> bits(256) = {
  let leaves : list(bits(256)) = [|
    mix_in_length(zh_at(zh, 13), 0),
    mix_in_length(zh_at(zh, 4), 0),
    mix_in_length(zh_at(zh, 1), 0)
  |];
  merkleize(zh, leaves, 3, 2)
}

/* SszNewPayloadRequest: 4 fields -> depth 2. This is new_payload_request_root. */
function htr_new_payload_request(zh : list(bits(256))) -> bits(256) = {
  let leaves : list(bits(256)) = [|
    htr_execution_payload(zh),
    htr_versioned_hashes(zh),
    htr_bytes32(ssz_bytes32(npr_start() + 8)),   /* parent_beacon_block_root */
    htr_execution_requests(zh)
  |];
  merkleize(zh, leaves, 4, 2)
}
