/* ======================================================================== */
/* SSZ decoder for the canonical stateless input, read from PRIVATE INPUT.    */
/*                                                                           */
/* Mirrors execution-specs (projects/zkevm) forks/amsterdam stateless_ssz.py: */
/* a 2-byte big-endian schema id (0x0001) precedes SSZ(SszStatelessInput).     */
/* SSZ layout used here: fixed fields inline; each variable field holds a      */
/* 4-byte little-endian offset (relative to its container start) into the      */
/* variable section, laid out in field order. Bytes come from an abstract       */
/* source (ssz_src_byte/ssz_src_len) that a harness binds to its input; the      */
/* zkVM guest binds it to the private-input region (zkvm/el_input.{h,c}). This   */
/* file is a model-level codec (like rlp.sail) and is NOT included by           */
/* evm.sail, so the core model stays free of the input FFI.                   */
/*                                                                           */
/* Field positions below are computed from the container definitions in       */
/* STATELESS.md; see SszExecutionPayload for the 540-byte fixed region map.    */
/* ======================================================================== */

val ssz_src_len  = impure "ssz_src_len"  : unit -> int
val ssz_src_byte = impure "ssz_src_byte" : int -> bits(8)
/* bulk readers: n (<=8) bytes from the input in one FFI call (LE / BE) */
val ssz_src_le   = impure "ssz_src_le"   : (int, int) -> bits(64)
val ssz_src_be   = impure "ssz_src_be"   : (int, int) -> bits(64)

function ib(i : int) -> int = unsigned(ssz_src_byte(i))

/* little-endian uintN (n <= 8 bytes) at absolute input offset b — one FFI call */
function ssz_uint(b : int, n : int) -> int = unsigned(ssz_src_le(b, n))
function ssz_u32(b : int) -> int = unsigned(ssz_src_le(b, 4))
function ssz_u64(b : int) -> int = unsigned(ssz_src_le(b, 8))

/* big-endian 20-byte address at offset b — three FFI reads (8+8+4) */
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

/* SSZ body starts after the 2-byte big-endian schema id */
let SSZ_BODY : int = 2
function ssz_schema_id() -> int = ib(0) * 256 + ib(1)

/* SszStatelessInput: 4 variable fields, offsets at body+{0,4,8,12} */
function npr_start()     -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 0)
function witness_start() -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 4)
function cc_start()      -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 8)
function cc_end()        -> int = SSZ_BODY + ssz_u32(SSZ_BODY + 12)  /* = public_keys start */

/* SszNewPayloadRequest fixed region: execution_payload offset @0, versioned_hashes
 * offset @4, parent_beacon_block_root @8 (Bytes32), execution_requests offset @40 */
function payload_start() -> int = { let n = npr_start(); n + ssz_u32(n + 0) }
function payload_end()   -> int = { let n = npr_start(); n + ssz_u32(n + 4) }
function npr_end()       -> int = witness_start()   /* npr is the first top field */

/* SszExecutionPayload fixed-region field offsets (bytes from payload start) */
let PL_FEE_RECIPIENT : int = 32
let PL_STATE_ROOT    : int = 52
let PL_BLOCK_NUMBER  : int = 404
let PL_GAS_LIMIT     : int = 412
let PL_GAS_USED      : int = 420
let PL_TIMESTAMP     : int = 428
let PL_EXTRA_OFF     : int = 436
let PL_BASE_FEE      : int = 440
let PL_BLOCK_HASH    : int = 472
let PL_TXS_OFF       : int = 504
let PL_WDS_OFF       : int = 508
let PL_BAL_OFF       : int = 528

/* big-endian 32-byte field (Bytes32, e.g. prev_randao) — four FFI reads */
function ssz_bytes32(b : int) -> bits(256) = {
  let w0 = ssz_src_be(b + 0, 8);
  let w1 = ssz_src_be(b + 8, 8);
  let w2 = ssz_src_be(b + 16, 8);
  let w3 = ssz_src_be(b + 24, 8);
  append(w0, append(w1, append(w2, w3)))
}
/* little-endian 32-byte uint256 (SSZ) — four FFI reads, low word first */
function ssz_u256(b : int) -> word = {
  let c0 = ssz_src_le(b + 0, 8);
  let c1 = ssz_src_le(b + 8, 8);
  let c2 = ssz_src_le(b + 16, 8);
  let c3 = ssz_src_le(b + 24, 8);
  append(c3, append(c2, append(c1, c0)))
}

let PL_PREV_RANDAO : int = 372
function pl_block_number()  -> int     = ssz_u64(payload_start() + PL_BLOCK_NUMBER)
function pl_gas_limit()     -> int     = ssz_u64(payload_start() + PL_GAS_LIMIT)
function pl_gas_used()      -> int     = ssz_u64(payload_start() + PL_GAS_USED)
function pl_timestamp()     -> int     = ssz_u64(payload_start() + PL_TIMESTAMP)
function pl_fee_recipient() -> address = ssz_addr(payload_start() + PL_FEE_RECIPIENT)
function pl_prev_randao()   -> bits(256) = ssz_bytes32(payload_start() + PL_PREV_RANDAO)
function pl_base_fee()      -> word      = ssz_u256(payload_start() + PL_BASE_FEE)
function pl_state_root()    -> bits(256) = ssz_bytes32(payload_start() + PL_STATE_ROOT)
function pl_block_hash()    -> bits(256) = ssz_bytes32(payload_start() + PL_BLOCK_HASH)

/* variable sub-regions of the payload (offsets relative to payload start) */
function pl_txs_start() -> int = { let p = payload_start(); p + ssz_u32(p + PL_TXS_OFF) }
function pl_wds_start() -> int = { let p = payload_start(); p + ssz_u32(p + PL_WDS_OFF) }
function pl_bal_start() -> int = { let p = payload_start(); p + ssz_u32(p + PL_BAL_OFF) }

/* block_access_list (simplified): u32-LE count, then count*(addr[20] ++ slot[32 BE]).
 * bal_load materializes the permitted (addr,slot) set; the kernel's k_sload/k_sstore
 * panic on any access outside it (k_bal_enable / bal_guard). */
function bal_count() -> int = ssz_u32(pl_bal_start())
function bal_load() -> list(SlotKey) = {
  let base = pl_bal_start() + 4;
  let n = bal_count();
  var xs : list(SlotKey) = [||];
  foreach (i from 0 to (n - 1)) {
    xs = struct { sk_addr = ssz_addr(base + i * 52), sk_slot = ssz_bytes32(base + i * 52 + 20) } :: xs
  };
  xs
}

/* withdrawals: FIXED 44-byte SszWithdrawal elements, span [wds, bal) */
function pl_n_withdrawals() -> int = ediv_int(pl_bal_start() - pl_wds_start(), 44)
function withdrawal_amount(i : int) -> int     = ssz_u64(pl_wds_start() + i * 44 + 36)
function withdrawal_addr(i : int)   -> address = ssz_addr(pl_wds_start() + i * 44 + 16)

/* transactions: List[ByteList], span [txs, wds); count via the leading offset */
function pl_n_txs() -> int = {
  let t = pl_txs_start();
  let w = pl_wds_start();
  if t == w then 0 else ediv_int(ssz_u32(t), 4)
}

/* SszChainConfig: chain_id u64 @ cc+0, active_fork (SszForkConfig) offset @ cc+8;
 * SszForkConfig: fork u64 @ 0 (ProtocolFork index; Amsterdam = 20). */
function cc_chain_id() -> int = ssz_u64(cc_start() + 0)
function cc_active_fork_start() -> int = { let c = cc_start(); c + ssz_u32(c + 8) }
function cc_fork() -> int = ssz_u64(cc_active_fork_start() + 0)

/* SszExecutionWitness: 3 variable List fields, offsets at witness+{0,4,8} */
function wit_state_start()   -> int = { let w = witness_start(); w + ssz_u32(w + 0) }
function wit_codes_start()   -> int = { let w = witness_start(); w + ssz_u32(w + 4) }
function wit_headers_start() -> int = { let w = witness_start(); w + ssz_u32(w + 8) }
function wit_n_state() -> int = {
  let s = wit_state_start();
  let c = wit_codes_start();
  if s == c then 0 else ediv_int(ssz_u32(s), 4)
}
function wit_n_codes() -> int = {
  let c = wit_codes_start();
  let h = wit_headers_start();
  if c == h then 0 else ediv_int(ssz_u32(c), 4)
}
function wit_n_headers() -> int = {
  let h = wit_headers_start();
  let e = cc_start();   /* the witness container ends where chain_config begins */
  if h == e then 0 else ediv_int(ssz_u32(h), 4)
}

/* element i bytes of a List[ByteList] given its [start, end) span + count */
function ssz_list_elem(start : int, n : int, stop : int, i : int) -> list(byte) = {
  let off_i    = ssz_u32(start + i * 4);
  let off_next = if i + 1 < n then ssz_u32(start + (i + 1) * 4) else (stop - start);
  ssz_bytes(start + off_i, off_next - off_i)
}
/* (start, len) of element i of a List[ByteList] given its [start, stop) span */
function ssz_list_span(start : int, n : int, stop : int, i : int) -> (int, int) = {
  let off_i    = ssz_u32(start + i * 4);
  let off_next = if i + 1 < n then ssz_u32(start + (i + 1) * 4) else (stop - start);
  (start + off_i, off_next - off_i)
}
function ssz_witness_node(i : int)   -> list(byte) =
  ssz_list_elem(wit_state_start(),   wit_n_state(),   wit_codes_start(),   i)
function ssz_witness_code(i : int)   -> list(byte) =
  ssz_list_elem(wit_codes_start(),   wit_n_codes(),   wit_headers_start(), i)
function ssz_witness_code_span(i : int) -> (int, int) =
  ssz_list_span(wit_codes_start(),   wit_n_codes(),   wit_headers_start(), i)
function ssz_witness_header(i : int) -> list(byte) =
  ssz_list_elem(wit_headers_start(), wit_n_headers(), cc_start(),          i)

/* transactions[i] raw bytes (legacy = RLP list; typed = type byte ++ payload) */

