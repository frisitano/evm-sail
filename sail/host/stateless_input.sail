/* ======================================================================== */
/* HOST: STATELESS SSZ INPUT                                                */
/*                                                                          */
/* Decode the canonical SszStatelessInput from the private-input region     */
/* (ssz_src_*): the execution payload, the execution witness (state nodes,  */
/* codes, headers), the block access list (EIP-7928), and the chain config. */
/* Getters over the wire layout named in primitives/stateless_input.sail;   */
/* mirrors execution-specs (projects/zkevm) forks/amsterdam                 */
/* stateless_ssz.py. Called only by the outer stateless layers (the         */
/* lib/mpt.sail witness reader, host/block.sail, the sail/bin entries).     */
/* ======================================================================== */

/* Private-input oracle + guest output trace: the input/output axioms (no pure
 * body exists; C provides them -- test_utils.c natively, zkvm_input.c /
 * zkvm_io.c on the guest). All input reads are consistent views over one
 * bounded byte vector. emit_out appends exactly one byte to the guest
 * public-output trace; only guest entries call it, core specs leave it
 * abstract. */
val ssz_src_len = impure { c: "ssz_src_len" } : unit -> bits(64)
val ssz_src_byte_at = impure { c: "ssz_src_byte" } : u64 -> bits(8)

/* int-offset adapter: the ONE remaining sail_int FFI crossing became u64;
 * all decode arithmetic stays in the int domain and converts here. */
function ssz_src_byte(i : int) -> byte = ssz_src_byte_at(get_slice_int(64, i, 0))

/* The bulk readers below are pure derivations of ssz_src_byte -- consistent
 * LE/BE views over one bounded byte vector. */
function ssz_src_le(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(acc, sail_shiftleft(sail_zero_extend(ssz_src_byte(b + k), 64), 8 * k))
    };
    acc
}

function ssz_src_be(b : int, n : int) -> bits(64) = {
    var acc : bits(64) = 0x0000000000000000;
    foreach (k from 0 to (n - 1)) {
        acc = or_vec(sail_shiftleft(acc, 8), sail_zero_extend(ssz_src_byte(b + k), 64))
    };
    acc
}

function ib(i : int) -> int = unsigned(ssz_src_byte(i))

/* little-endian uintN (n <= 8 bytes) at absolute input offset b -- one FFI call */
function ssz_uint(b : int, n : int) -> int = unsigned(ssz_src_le(b, n))
function ssz_u32(b : int) -> int = unsigned(ssz_src_le(b, 4))
function ssz_u64(b : int) -> int = unsigned(ssz_src_le(b, 8))

/* big-endian 20-byte address at offset b -- three FFI reads (8+8+4) */
function ssz_addr(b : int) -> address = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 4);
    append(w0, append(w1, w2[31 .. 0]))
}

/* read len bytes at absolute offset b into a list(byte) (O(len), cons+reverse) */
function ssz_bytes(b : int, len : int) -> list(byte) = {
    var out : list(byte) = [||];
    foreach (k from 0 to (len - 1)) {
        out = ssz_src_byte(b + (len - 1 - k)) :: out
    };
    out
}

/* ---- container navigation --------------------------------------------- */

function ssz_schema_id() -> int = ib(0) * 256 + ib(1)

/* Container navigation. The HOT paths (witness preloads, tx/withdrawal
 * decode) run inside the single decode pass over the SszLayout value built by
 * decode_input_layout below -- each offset read once. The lazy getters here
 * re-derive from the container root and serve only the COLD once-per-block
 * consumers (output HTR/echo, chain-config validation, header/trie hashing);
 * no decode state lives in registers. */
function npr_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + IN_NPR_OFF)
function witness_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + IN_WITNESS_OFF)
function cc_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + IN_CHAIN_CONFIG_OFF)
function cc_end() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + IN_PUBLIC_KEYS_OFF) /* = public_keys start */
function payload_start() -> int = {
    let n = npr_start();
    n + ssz_u32(n + NPR_PAYLOAD_OFF)
}
function payload_end() -> int = {
    let n = npr_start();
    n + ssz_u32(n + NPR_VHASHES_OFF)
}
function req_start() -> int = npr_start() + ssz_u32(npr_start() + NPR_REQUESTS_OFF)
function dep_start() -> int = req_start() + ssz_u32(req_start() + REQ_DEPOSITS_OFF)
function wreq_start() -> int = req_start() + ssz_u32(req_start() + REQ_WITHDRAWALS_OFF)
function creq_start() -> int = req_start() + ssz_u32(req_start() + REQ_CONSOLIDATIONS_OFF)
function req_end() -> int = witness_start() /* execution_requests is the last npr field */

/* big-endian 32-byte field (Bytes32, e.g. prev_randao) -- four FFI reads */
function ssz_bytes32(b : int) -> bits(256) = {
    let w0 = ssz_src_be(b + 0, 8);
    let w1 = ssz_src_be(b + 8, 8);
    let w2 = ssz_src_be(b + 16, 8);
    let w3 = ssz_src_be(b + 24, 8);
    append(w0, append(w1, append(w2, w3)))
}

/* little-endian 32-byte uint256 (SSZ) -- four FFI reads, low word first */
function ssz_u256(b : int) -> word = {
    let c0 = ssz_src_le(b + 0, 8);
    let c1 = ssz_src_le(b + 8, 8);
    let c2 = ssz_src_le(b + 16, 8);
    let c3 = ssz_src_le(b + 24, 8);
    append(c3, append(c2, append(c1, c0)))
}

function pl_block_number(p : int) -> int = ssz_u64(p + PL_BLOCK_NUMBER)
function pl_slot_number(p : int) -> u64 = ssz_src_le(p + PL_SLOT_NUMBER, 8) /* EIP-7843 */
function pl_gas_limit(p : int) -> int = ssz_u64(p + PL_GAS_LIMIT)
function pl_gas_used(p : int) -> int = ssz_u64(p + PL_GAS_USED)
function pl_timestamp(p : int) -> int = ssz_u64(p + PL_TIMESTAMP)
function pl_fee_recipient(p : int) -> address = ssz_addr(p + PL_FEE_RECIPIENT)
function pl_prev_randao(p : int) -> bits(256) = ssz_bytes32(p + PL_PREV_RANDAO)
function pl_base_fee(p : int) -> word = ssz_u256(p + PL_BASE_FEE)
function pl_blob_gas_used(p : int) -> u64 = ssz_src_le(p + PL_BLOB_GAS_USED, 8)
function pl_excess_blob_gas(p : int) -> u64 = ssz_src_le(p + PL_EXCESS_BLOB_GAS, 8)
function pl_state_root(p : int) -> bits(256) = ssz_bytes32(p + PL_STATE_ROOT)
function pl_receipts_root(p : int) -> bits(256) = ssz_bytes32(p + PL_RECEIPTS_ROOT)
function pl_logs_bloom(p : int) -> list(byte) = ssz_bytes(p + PL_LOGS_BLOOM, 256)
function pl_block_hash(p : int) -> bits(256) = ssz_bytes32(p + PL_BLOCK_HASH)

/* variable sub-regions of the payload (lazy; cold callers only) */
function pl_txs_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_TXS_OFF)
}
function pl_wds_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_WDS_OFF)
}
function pl_bal_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_BAL_OFF)
}
function pl_extra_start() -> int = {
    let p = payload_start();
    p + ssz_u32(p + PL_EXTRA_OFF)
}

/* withdrawals: FIXED 44-byte SszWithdrawal elements at base wds (layout) */
function withdrawal_index(wds : int, i : int) -> int = ssz_u64(wds + i * WD_SIZE + WD_INDEX)
function withdrawal_validator_index(wds : int, i : int) -> int = ssz_u64(wds + i * WD_SIZE + WD_VALIDATOR_INDEX)
function withdrawal_amount(wds : int, i : int) -> int = ssz_u64(wds + i * WD_SIZE + WD_AMOUNT)
function withdrawal_addr(wds : int, i : int) -> address = ssz_addr(wds + i * WD_SIZE + WD_ADDRESS)

/* lazy counts (cold callers; the pass uses layout.n_txs / layout.n_wds) */
function pl_n_txs() -> int = {
    let t = pl_txs_start();
    let w = pl_wds_start();
    if t == w then {
        0
    } else {
        ediv_int(ssz_u32(t), SSZ_OFF_BYTES)
    }
}
function pl_n_withdrawals() -> int = ediv_int(pl_bal_start() - pl_wds_start(), WD_SIZE)

/* SszChainConfig: chain_id u64 @ cc+0, active_fork (SszForkConfig) offset @ cc+8;
 * SszForkConfig: fork u64 @ 0 (ProtocolFork index; Amsterdam = 20 in current
 * projects/zkevm fixtures). */
function cc_chain_id() -> int = ssz_u64(cc_start() + CC_CHAIN_ID)
function cc_active_fork_start() -> int = {
    let c = cc_start();
    c + ssz_u32(c + CC_ACTIVE_FORK_OFF)
}
function cc_fork() -> int = ssz_u64(cc_active_fork_start() + FC_FORK)

/* SszForkConfig { fork:u64 @0, activation:SszForkActivation (offset @8),
 * blob_schedule (offset @12) }; SszForkActivation { block_number, timestamp :
 * SszOptionalForkActivationValue (List[u64,1], offsets @ activation+{0,4}) }.
 * The active_fork's activation must be ACTIVE for the block (EELS
 * validate_chain_config -> _is_activation_active): at least one of block_number
 * / timestamp set, and any set value must not exceed the payload's -- a FUTURE
 * activation point makes the block invalid. */
function cc_activation_start() -> int = {
    let f = cc_active_fork_start();
    f + ssz_u32(f + FC_ACTIVATION_OFF)
}
function cc_blobsched_start() -> int = {
    let f = cc_active_fork_start();
    f + ssz_u32(f + FC_BLOBSCHED_OFF)
}
function cc_activation_active() -> bool = {
    let a = cc_activation_start();
    let bn_start = a + ssz_u32(a + FA_BLOCK_NUMBER_OFF);
    let ts_start = a + ssz_u32(a + FA_TIMESTAMP_OFF);
    let bs_start = cc_blobsched_start();
    let bn_present = (ts_start - bn_start) >= 8; /* the optional list is non-empty */
    let ts_present = (bs_start - ts_start) >= 8;
    let p = payload_start();
    let bn_ok = not_bool(bn_present) | (pl_block_number(p) >= ssz_u64(bn_start));
    let ts_ok = not_bool(ts_present) | (pl_timestamp(p) >= ssz_u64(ts_start));
    (bn_present | ts_present) & bn_ok & ts_ok
}

/* (start, len) of element i of a List[ByteList] given its [start, stop) span */
function ssz_list_span(start : int, n : int, stop : int, i : int) -> (int, int) = {
    let off_i = ssz_u32(start + i * SSZ_OFF_BYTES);
    let off_next = if i + 1 < n then {
            ssz_u32(start + (i + 1) * SSZ_OFF_BYTES)
        } else {
            stop - start
        };
    (start + off_i, off_next - off_i)
}
function ssz_list_slice(start : int, n : int, stop : int, i : int) -> ByteSlice = {
    let (off, len) = ssz_list_span(start, n, stop, i);
    witness_byte_slice(off, len)
}
function sha256_request_digest(request_type : int, s : ByteSlice) -> bits(256) =
    bytes_sha256_prefixed(get_slice_int(8, request_type, 0), s)

/* transactions[i] raw bytes (legacy = RLP list; typed = type byte ++ payload) */
