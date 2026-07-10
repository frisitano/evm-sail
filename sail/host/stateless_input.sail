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

/* ---- one-shot container index ------------------------------------------ */
/* The SSZ container geometry, decoded ONCE per input (stateless_input_index,
 * called at each entry's start) and held in registers: the navigation getters
 * below read cached offsets instead of re-deriving the offset chain from the
 * container root on every call (each ssz_u32 in that chain costs four byte
 * crossings, and the witness preload loops call these per element). Mirrors
 * EELS, which decodes the SSZ input into objects up front. */
register in_npr : int = 0 /* SszNewPayloadRequest                    */
register in_witness : int = 0 /* SszExecutionWitness (= npr end)         */
register in_cc : int = 0 /* SszChainConfig                          */
register in_cc_end : int = 0 /* chain_config end (= public_keys start)  */
register in_payload : int = 0 /* SszExecutionPayload fixed region        */
register in_payload_end : int = 0 /* payload end (= versioned_hashes start)  */
register in_dep : int = 0 /* execution requests: deposits            */
register in_wreq : int = 0 /* execution requests: withdrawal reqs     */
register in_creq : int = 0 /* execution requests: consolidation reqs  */
register in_txs : int = 0 /* transactions List[ByteList]             */
register in_wds : int = 0 /* withdrawals (fixed 44-byte elements)    */
register in_bal : int = 0 /* block_access_list                       */
register in_extra : int = 0 /* extra_data                              */
register in_n_txs : int = 0
register in_n_wds : int = 0
register in_wit_state : int = 0 /* witness state nodes List[ByteList]      */
register in_wit_codes : int = 0 /* witness codes List[ByteList]            */
register in_wit_headers : int = 0 /* witness headers List[ByteList]          */
register in_n_wit_state : int = 0
register in_n_wit_codes : int = 0
register in_n_wit_headers : int = 0

/* the single decode pass: each container offset is read exactly once */
function stateless_input_index() -> unit = {
    in_npr = SSZ_BODY + ssz_u32(SSZ_BODY + IN_NPR_OFF);
    in_witness = SSZ_BODY + ssz_u32(SSZ_BODY + IN_WITNESS_OFF);
    in_cc = SSZ_BODY + ssz_u32(SSZ_BODY + IN_CHAIN_CONFIG_OFF);
    in_cc_end = SSZ_BODY + ssz_u32(SSZ_BODY + IN_PUBLIC_KEYS_OFF);

    /* SszNewPayloadRequest fixed region: execution_payload offset @0,
       versioned_hashes offset @4, parent_beacon_block_root @8 (inline Bytes32),
       execution_requests offset @40; SszExecutionRequests holds three variable
       lists (offsets at req+{0,4,8}) and ends where npr does. */
    in_payload = in_npr + ssz_u32(in_npr + NPR_PAYLOAD_OFF);
    in_payload_end = in_npr + ssz_u32(in_npr + NPR_VHASHES_OFF);
    let req = in_npr + ssz_u32(in_npr + NPR_REQUESTS_OFF);
    in_dep = req + ssz_u32(req + REQ_DEPOSITS_OFF);
    in_wreq = req + ssz_u32(req + REQ_WITHDRAWALS_OFF);
    in_creq = req + ssz_u32(req + REQ_CONSOLIDATIONS_OFF);

    /* payload variable sub-regions; tx count from the List[ByteList] leading
       offset, withdrawal count from the fixed element size */
    in_extra = in_payload + ssz_u32(in_payload + PL_EXTRA_OFF);
    in_txs = in_payload + ssz_u32(in_payload + PL_TXS_OFF);
    in_wds = in_payload + ssz_u32(in_payload + PL_WDS_OFF);
    in_bal = in_payload + ssz_u32(in_payload + PL_BAL_OFF);
    in_n_txs =
        if in_txs == in_wds then {
            0
        } else {
            ediv_int(ssz_u32(in_txs), SSZ_OFF_BYTES)
        };
    in_n_wds = ediv_int(in_bal - in_wds, WD_SIZE);

    /* SszExecutionWitness: 3 variable List fields, offsets at witness+{0,4,8};
       the witness container ends where chain_config begins */
    in_wit_state = in_witness + ssz_u32(in_witness + WIT_STATE_OFF);
    in_wit_codes = in_witness + ssz_u32(in_witness + WIT_CODES_OFF);
    in_wit_headers = in_witness + ssz_u32(in_witness + WIT_HEADERS_OFF);
    in_n_wit_state =
        if in_wit_state == in_wit_codes then {
            0
        } else {
            ediv_int(ssz_u32(in_wit_state), SSZ_OFF_BYTES)
        };
    in_n_wit_codes =
        if in_wit_codes == in_wit_headers then {
            0
        } else {
            ediv_int(ssz_u32(in_wit_codes), SSZ_OFF_BYTES)
        };
    in_n_wit_headers =
        if in_wit_headers == in_cc then {
            0
        } else {
            ediv_int(ssz_u32(in_wit_headers), SSZ_OFF_BYTES)
        }
}

/* navigation getters over the index (kept as the read API) */
function npr_start() -> int = in_npr
function witness_start() -> int = in_witness
function cc_start() -> int = in_cc
function cc_end() -> int = in_cc_end /* = public_keys start */
function payload_start() -> int = in_payload
function payload_end() -> int = in_payload_end
function npr_end() -> int = in_witness /* npr is the first top field */
function dep_start() -> int = in_dep
function wreq_start() -> int = in_wreq
function creq_start() -> int = in_creq
function req_end() -> int = in_witness /* execution_requests is the last npr field */

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

function pl_block_number() -> int = ssz_u64(payload_start() + PL_BLOCK_NUMBER)
function pl_slot_number() -> u64 = ssz_src_le(payload_start() + PL_SLOT_NUMBER, 8) /* EIP-7843 */
function pl_gas_limit() -> int = ssz_u64(payload_start() + PL_GAS_LIMIT)
function pl_gas_used() -> int = ssz_u64(payload_start() + PL_GAS_USED)
function pl_timestamp() -> int = ssz_u64(payload_start() + PL_TIMESTAMP)
function pl_fee_recipient() -> address = ssz_addr(payload_start() + PL_FEE_RECIPIENT)
function pl_prev_randao() -> bits(256) = ssz_bytes32(payload_start() + PL_PREV_RANDAO)
function pl_base_fee() -> word = ssz_u256(payload_start() + PL_BASE_FEE)
function pl_blob_gas_used() -> u64 = ssz_src_le(payload_start() + PL_BLOB_GAS_USED, 8)
function pl_excess_blob_gas() -> u64 = ssz_src_le(payload_start() + PL_EXCESS_BLOB_GAS, 8)
function pl_state_root() -> bits(256) = ssz_bytes32(payload_start() + PL_STATE_ROOT)
function pl_receipts_root() -> bits(256) = ssz_bytes32(payload_start() + PL_RECEIPTS_ROOT)
function pl_logs_bloom() -> list(byte) = ssz_bytes(payload_start() + PL_LOGS_BLOOM, 256)
function pl_block_hash() -> bits(256) = ssz_bytes32(payload_start() + PL_BLOCK_HASH)

/* variable sub-regions of the payload (decoded by stateless_input_index) */
function pl_txs_start() -> int = in_txs
function pl_wds_start() -> int = in_wds
function pl_bal_start() -> int = in_bal
function pl_extra_start() -> int = in_extra
function pl_extra_data() -> list(byte) =
    ssz_bytes(pl_extra_start(), pl_txs_start() - pl_extra_start())
function pl_block_access_list() -> list(byte) =
    ssz_bytes(pl_bal_start(), payload_end() - pl_bal_start())

/* block_access_list (simplified): u32-LE count, then count*(addr[20] ++ slot[32 BE]).
 * bal_load materializes the permitted (addr,slot) set; the kernel's k_sload/k_sstore
 * panic on any access outside it (k_bal_enable / bal_guard). */
function bal_count() -> int = ssz_u32(pl_bal_start())
function bal_load() -> list(StorageKey) = {
    let base = pl_bal_start() + BAL_COUNT_BYTES;
    let n = bal_count();
    var xs : list(StorageKey) = [||];
    foreach (i from 0 to (n - 1)) {
        xs =
            struct {
                addr = ssz_addr(base + i * BAL_ENTRY_SIZE),
                slot = ssz_bytes32(base + i * BAL_ENTRY_SIZE + BAL_SLOT_OFF),
            } ::
                xs
    };
    xs
}

/* withdrawals: FIXED 44-byte SszWithdrawal elements, span [wds, bal) */
function pl_n_withdrawals() -> int = in_n_wds
function withdrawal_index(i : int) -> int = ssz_u64(pl_wds_start() + i * WD_SIZE + WD_INDEX)
function withdrawal_validator_index(i : int) -> int = ssz_u64(pl_wds_start() + i * WD_SIZE + WD_VALIDATOR_INDEX)
function withdrawal_amount(i : int) -> int = ssz_u64(pl_wds_start() + i * WD_SIZE + WD_AMOUNT)
function withdrawal_addr(i : int) -> address = ssz_addr(pl_wds_start() + i * WD_SIZE + WD_ADDRESS)

function pl_n_txs() -> int = in_n_txs

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
    let bn_ok = not_bool(bn_present) | (pl_block_number() >= ssz_u64(bn_start));
    let ts_ok = not_bool(ts_present) | (pl_timestamp() >= ssz_u64(ts_start));
    (bn_present | ts_present) & bn_ok & ts_ok
}

/* SszExecutionWitness list getters over the index */
function wit_state_start() -> int = in_wit_state
function wit_codes_start() -> int = in_wit_codes
function wit_headers_start() -> int = in_wit_headers
function wit_n_state() -> int = in_n_wit_state
function wit_n_codes() -> int = in_n_wit_codes
function wit_n_headers() -> int = in_n_wit_headers

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
function ssz_witness_node_slice(i : int) -> ByteSlice =
    ssz_list_slice(wit_state_start(), wit_n_state(), wit_codes_start(), i)
function ssz_witness_code_slice(i : int) -> ByteSlice =
    ssz_list_slice(wit_codes_start(), wit_n_codes(), wit_headers_start(), i)
function ssz_witness_header_slice(i : int) -> ByteSlice =
    ssz_list_slice(wit_headers_start(), wit_n_headers(), cc_start(), i)

/* transactions[i] raw bytes (legacy = RLP list; typed = type byte ++ payload) */
