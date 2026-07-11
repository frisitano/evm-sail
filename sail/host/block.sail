/* ======================================================================== */
/* SSZ BLOCK DECODE  (shared: guest + runner)                               */
/*                                                                          */
/* The SszStatelessInput decode layer (header -> k_header register plus     */
/* per-index tx/withdrawal decoders), extracted from main.sail so           */
/* BOTH the zkVM guest (main.sail: full-block validation) and the EEST      */
/* runner (runner.sail: execute-tx + dump post-state) reach execution data  */
/* through ONE input structure. Depends only on the common layer (io.sail   */
/* SSZ accessors, rlp.sail, transaction.sail, mpt.sail witness walk); it    */
/* does NOT touch any entry-specific code.                                  */
/* ======================================================================== */

/* Register every witness code blob into the hash-keyed C code store. The C side
 * hashes the SSZ input span and copies the bytes into the owned code_db entry. */
function register_witness_codes(start : int, n : int, stop : int) -> unit = {
    foreach (i from 0 to (n - 1)) {
        code_db_store_slice(ssz_list_slice(start, n, stop, i))
    }
}

/* Parse the witness header chain, asserting it against the ALREADY-decoded
 * block: each header is hashed once (captured in the distance-indexed
 * ancestor-hash table for BLOCKHASH/EIP-2935), the chain prefix must be contiguous
 * (header[i].parent_hash == keccak(header[i-1])), and the NEWEST header must
 * be the decoded block's parent (keccak(header[n-1]) == expected_parent).
 * Returns (chain contiguous, block parent hash matches, newest header's RLP
 * items); field DECODING from the captured items stays at the original
 * consumer sites (parent_state_root, parent_header_int_field), so a
 * non-canonical field still throws exactly where it used to. */
function index_witness_headers(
    start : int,
    n : int,
    stop : int,
    expected_parent : hash,
) -> (
    (bool, bool, list(RlpField))
) = {

    var prev : hash = ZERO_WORD;
    var ok : bool = n >= 1;
    var pitems : list(RlpField) = [||];
    foreach (i from 0 to (n - 1)) {
        let s = ssz_list_slice(start, n, stop, i);
        if (i > 0) | (i == n - 1) then {
            let items = rlp_node_items(read_byte_slice(s));
            if i > 0 then {
                if bytes_be256(list_nth(items, 0, RLP_NONE).content) != prev then {
                    ok = false
                }
            };
            if i == n - 1 then {
                pitems = items
            }
        };
        prev = keccak256(s);
        let j = n - 1 - i; /* distance-1 from the block under validation */
        if j < 256 then {
            headerhash_set(get_slice_int(64, j, 0), prev)
        }
    };
    k_n_headers = n;
    (ok, prev == expected_parent, pitems)
}

/* ---- EIP-4788 / EIP-2935 block-start system calls ---------------------------
 * Before the transactions, the protocol issues unchecked system calls from
 * SYSTEM_ADDRESS to the beacon-roots (EIP-4788) and block-hash-history (EIP-2935)
 * contracts, writing the parent beacon-block root and the parent block hash into
 * their ring buffers. Each takes a 32-byte input, 30M gas, and bumps no nonce /
 * charges no fee / counts toward no block gas; it is skipped if the contract has
 * no code. Their committed storage writes shape the post-state root. */
function SYSTEM_ADDRESS() -> address = 0xfffffffffffffffffffffffffffffffffffffffe
function BEACON_ROOTS_ADDR() -> address = 0x000f3df6d732807ef1319fb7b8bb8522d0beac02
function HISTORY_STORAGE_ADDR() -> address = 0x0000f90827f1c53a10cb7a02335b175320002935

/* Enter a top-level protocol system-call frame.
 *
 * The caller stages txdata before entering this helper.
 * Keeping frame initialization in one place gives the block driver a named
 * boundary: pre-transaction and post-transaction system calls differ in
 * validation policy, not in execution-frame construction.
 */
function enter_system_call_frame(tgt : address, input_len : int) -> unit = {
    k_snapshot();
    pc = 0;
    call_depth = 0;
    gas_remaining = to_gas(30000000);
    stack_clear();
    mem_clear();
    returndata_reset_all();
    memory_size = 0;
    frame_status = Running();
    calldata_bind_tx_input(); /* AFTER mem_clear: it points the base frame's calldata at the slot */
    message =
        struct {
            caller = SYSTEM_ADDRESS(),
            address = tgt,
            code_address = tgt,
            value = ZERO_WORD,
            data_len = input_len,
            is_static = false,
            depth = 0,
        };
    frame_code_addr(tgt)
}

function system_call(tgt : address, input : bits(256)) -> unit = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        () /* system contract absent -> skip (EIP-4788 / EIP-2935) */
    } else {
        let _ = txdata_set_word(input); /* the call's 32-byte calldata */
        enter_system_call_frame(tgt, 32);
        interpret();
        if frame_succeeded() then {
            k_commit()
        } else {
            k_revert()
        };
        k_tx_merge() /* system call is a top-level boundary: merge its storage */
    }
}

/* ---- EIP-7002 / EIP-7251 block-END system calls (general-purpose requests) ---
 * After the transactions and withdrawals, the protocol dequeues the queued
 * withdrawal (EIP-7002) and consolidation (EIP-7251) requests by a CHECKED
 * system call (SYSTEM_ADDRESS, EMPTY calldata, 30M gas) to each predeploy. The
 * dequeue clears the request queue in the contract's storage, shaping the
 * post-state root. "Checked" means: a missing contract (no code) OR a failed
 * call makes the whole block INVALID (returns false here). */
function WITHDRAWAL_REQUEST_ADDR() -> address = 0x00000961ef480eb55e80d19ad83579a64c007002
function CONSOLIDATION_REQUEST_ADDR() -> address = 0x0000bbddc7ce488642fb579f8b00f3a590007251

function system_call_checked(tgt : address) -> bool = {
    if k_code_key(tgt) == KECCAK_EMPTY then {
        false /* checked: a missing system predeploy invalidates the block */
    } else {
        let _ = txdata_set_source(byte_slice(WitnessSource, 0, 0)); /* empty calldata: the SYSTEM caller dequeues */
        enter_system_call_frame(tgt, 0);
        interpret();
        let ok = frame_succeeded();
        if ok then {
            k_commit()
        } else {
            k_revert() /* checked: a reverting/erroring system call invalidates the block */
        };
        k_tx_merge(); /* system call is a top-level boundary: merge its storage */
        ok
    }
}

/* Reject a malformed SszStatelessInput: the 2-byte schema id must be 1, the
 * 4 field offsets (LE u32 at body+{0,4,8,12}) must start at 16, be monotonic,
 * and end within the input, and the trailing public_keys region must be a whole
 * number of 65-byte keys (no truncation / trailing garbage). EELS's consumer
 * rejects such input with successful_validation=false; the guest must too rather
 * than echo garbage. */
function input_well_formed() -> bool = {
    let len = unsigned(ssz_src_len());
    if len < 18 then {
        false
    } else {
        if ssz_schema_id() != 1 then {
            false
        } else {
            let o0 = ssz_u32(2);
            let o1 = ssz_u32(6);
            let o2 = ssz_u32(10);
            let o3 = ssz_u32(14);
            (o0 == 16) & (o0 <= o1) & (o1 <= o2) & (o2 <= o3) & (2 + o3 <= len) & (emod_int(len - (2 + o3), 65) == 0)
        }
    }
}

function decode_block_header_ssz(p : int, npr : int) -> BlockHeader =
    struct {
        number = pl_block_number(p),
        timestamp = pl_timestamp(p),
        gas_limit = pl_gas_limit(p),
        gas_used = pl_gas_used(p),
        prev_randao = pl_prev_randao(p),
        base_fee = pl_base_fee(p),
        blob_gas_used = pl_blob_gas_used(p),
        excess_blob_gas = pl_excess_blob_gas(p),
        state_root = pl_state_root(p),
        receipts_root = pl_receipts_root(p),
        logs_bloom = pl_logs_bloom(p),
        block_hash = pl_block_hash(p),
        fee_recipient = pl_fee_recipient(p),
        parent_hash = ssz_bytes32(p + 0),
        parent_beacon_block_root = ssz_bytes32(npr + NPR_BEACON_ROOT),
        slot_number = pl_slot_number(p),
    }

function withdrawal_at(wds : int, i : int) -> Withdrawal =
    struct {
        index = withdrawal_index(wds, i),
        validator_index = withdrawal_validator_index(wds, i),
        address = withdrawal_addr(wds, i),
        amount = withdrawal_amount(wds, i),
    }

/* Decode the SszChainConfig at cc: chain_id (u64 @0) and the active
 * SszForkConfig (offset @8: fork index u64 @0, SszForkActivation offset @8,
 * blob_schedule offset @12). The activation point (optional block_number /
 * timestamp, List[u64,1] each) must be REACHED by this payload (EELS
 * validate_chain_config -> _is_activation_active): at least one bound set,
 * none exceeding the payload's -- a future activation invalidates the block. */
function decode_chain_config(cc : int, number : int, timestamp : int) -> ChainConfig = {
    let f = cc + ssz_u32(cc + CC_ACTIVE_FORK_OFF);
    let fidx = ssz_u64(f + FC_FORK);
    let a = f + ssz_u32(f + FC_ACTIVATION_OFF);
    let bs_start = f + ssz_u32(f + FC_BLOBSCHED_OFF);
    let bn_start = a + ssz_u32(a + FA_BLOCK_NUMBER_OFF);
    let ts_start = a + ssz_u32(a + FA_TIMESTAMP_OFF);
    let bn_present = (ts_start - bn_start) >= 8; /* the optional list is non-empty */
    let ts_present = (bs_start - ts_start) >= 8;
    let bn_ok = not_bool(bn_present) | (number >= ssz_u64(bn_start));
    let ts_ok = not_bool(ts_present) | (timestamp >= ssz_u64(ts_start));
    struct {
        chain_id = ssz_u64(cc + CC_CHAIN_ID),
        fork_index = fidx,
        fork = fork_of_protocol_index(fidx),
        activation_active = (bn_present | ts_present) & bn_ok & ts_ok,
    }
}

/* Everything the single decode pass returns: the decoded block (header,
 * transactions, withdrawals) plus the witness-header results the validation
 * layer reads. The parent header travels as its RLP items so field decoding
 * (and its canonicity throws) stays at the original consumer sites. */
struct StatelessInput = {
    block : Block,
    chain_config : ChainConfig,
    headers_ok : bool, /* witness header chain contiguous              */
    parent_hash_ok : bool, /* newest witness header IS the block's parent  */
    parent_header_items : list(RlpField),
}

/* THE single traversal over the stateless input. In one pass, in container
 * order: the geometry (each offset once), the witness node DB and code DB
 * (C-side stores), the header chain (hashed once each, hashes into the C
 * the ancestor-hash table for BLOCKHASH, newest split once), the block header, and
 * every transaction (sender recovery + signature verification, as EELS
 * decodes the block body before executing it) and withdrawal. Also installs
 * the execution-context registers (k_header, k_header_hashes,
 * k_parent_state_root) the EVM reads ambiently. Throws InvalidBlock on a
 * broken header chain, malformed tx, or bad signature -- same sites, same
 * order as the previous incremental decode. */
function decode_stateless_input() -> StatelessInput = {
    /* geometry: each container offset read exactly once, held in locals */
    let npr = SSZ_BODY + ssz_u32(SSZ_BODY + IN_NPR_OFF);
    let wit = SSZ_BODY + ssz_u32(SSZ_BODY + IN_WITNESS_OFF);
    let cc = SSZ_BODY + ssz_u32(SSZ_BODY + IN_CHAIN_CONFIG_OFF);
    let pubkeys = SSZ_BODY + ssz_u32(SSZ_BODY + IN_PUBLIC_KEYS_OFF); /* = chain_config end */
    let p = npr + ssz_u32(npr + NPR_PAYLOAD_OFF);
    let p_end = npr + ssz_u32(npr + NPR_VHASHES_OFF);
    let txs = p + ssz_u32(p + PL_TXS_OFF);
    let wds = p + ssz_u32(p + PL_WDS_OFF);
    let bal = p + ssz_u32(p + PL_BAL_OFF);
    let n_txs = if txs == wds then {
            0
        } else {
            ediv_int(ssz_u32(txs), SSZ_OFF_BYTES)
        };
    let n_wds = ediv_int(bal - wds, WD_SIZE);
    let ws = wit + ssz_u32(wit + WIT_STATE_OFF);
    let wc = wit + ssz_u32(wit + WIT_CODES_OFF);
    let wh = wit + ssz_u32(wit + WIT_HEADERS_OFF);
    let n_ws = if ws == wc then {
            0
        } else {
            ediv_int(ssz_u32(ws), SSZ_OFF_BYTES)
        };
    let n_wc = if wc == wh then {
            0
        } else {
            ediv_int(ssz_u32(wc), SSZ_OFF_BYTES)
        };
    let n_wh = if wh == cc then {
            0
        } else {
            ediv_int(ssz_u32(wh), SSZ_OFF_BYTES)
        };

    /* the block itself (payload region, first in container order); each tx is
       an envelope slice + its 65-byte witness pubkey, decoded standalone */
    let header = decode_block_header_ssz(p, npr);
    k_set_header(header);
    let chain_config = decode_chain_config(cc, header.number, header.timestamp);
    k_chain_id = chain_config.chain_id;
    var txs_rev : list(Transaction) = [||];
    foreach (i from 0 to (n_txs - 1)) {
        txs_rev = rlp_decode_tx(ssz_list_slice(txs, n_txs, wds, i), witness_byte_slice(pubkeys + i * 65, 65)) :: txs_rev
    };
    var wds_rev : list(Withdrawal) = [||];
    foreach (i from 0 to (n_wds - 1)) {
        wds_rev = withdrawal_at(wds, i) :: wds_rev
    };

    /* The witness region, asserted against the decoded block. The node DB is
       keyed by keccak(node): every later stateless read is a lazy walk from
       k_parent_state_root, so witness material is authenticated exactly where
       it is consumed -- no eager re-root pass. The anchor is set only after
       the header chain validates. */
    build_node_db(ws, n_ws, wc);
    register_witness_codes(wc, n_wc, wh);
    let (hok, pok, pitems) = index_witness_headers(wh, n_wh, cc, header.parent_hash);
    if hok then {
        k_parent_state_root = parent_state_root(pitems)
    } else {
        throw InvalidBlock(WitnessDeficient)
    };
    struct {
        block =
            struct {
                header = header,
                body =
                    struct {
                        transactions = list_reverse(txs_rev),
                        withdrawals = list_reverse(wds_rev),
                        block_access_list = witness_byte_slice(bal, p_end - bal),
                    },
            },
        chain_config = chain_config,
        headers_ok = hok,
        parent_hash_ok = pok,
        parent_header_items = pitems,
    }
}
