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
        code_db_store_witness_code(ssz_list_slice(start, n, stop, i))
    }
}

/* Parse the witness header chain, asserting it against the ALREADY-decoded
 * block: each header is hashed once (kept in the C headerhash store for
 * BLOCKHASH/EIP-2935, k_blockhash), the chain prefix must be contiguous
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
    headerhash_reset();
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
        headerhash_push(prev)
    };
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

/* Materialize the hash-keyed witness node store and anchor the authenticated
 * pre-state root. There is no eager re-root or harvest pass: every later
 * stateless read is a lazy walk from k_parent_state_root, and because the
 * node-db is keyed by keccak(node) each walk is itself a hash-chain proof --
 * witness material is authenticated exactly where it is consumed. The anchor
 * register is set only after the witness header chain validates.
 *
 * No execution-overlay wipe here: the world starts empty (fresh process for the
 * production guest; the ctypes harness calls evmsail_clear_memory before each
 * run -- test_utils.c), which is why the model no longer defines k_world_reset. */
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

/* Everything the single decode pass returns: the decoded block (header,
 * transactions, withdrawals) plus the witness-header results the validation
 * layer reads. The parent header travels as its RLP items so field decoding
 * (and its canonicity throws) stays at the original consumer sites. */
struct StatelessInput = {
    block : Block,
    fork : int,
    headers_ok : bool, /* witness header chain contiguous              */
    parent_hash_ok : bool, /* newest witness header IS the block's parent  */
    parent_header_items : list(RlpField),
}

/* THE single traversal over the stateless input. In one pass, in container
 * order: the geometry (each offset once), the witness node DB and code DB
 * (C-side stores), the header chain (hashed once each, hashes into the C
 * headerhash store for BLOCKHASH, newest split once), the block header, and
 * every transaction (sender recovery + signature verification, as EELS
 * decodes the block body before executing it) and withdrawal. Also installs
 * the execution-context registers (k_header, k_n_headers,
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
    var txs_rev : list(Transaction) = [||];
    foreach (i from 0 to (n_txs - 1)) {
        txs_rev =
            rlp_decode_tx(ssz_list_slice(txs, n_txs, wds, i), witness_byte_slice(pubkeys + i * 65, 65)) ::
                txs_rev
    };
    var wds_rev : list(Withdrawal) = [||];
    foreach (i from 0 to (n_wds - 1)) {
        wds_rev = withdrawal_at(wds, i) :: wds_rev
    };

    /* the witness region, asserted against the decoded block */
    build_node_db(ws, n_ws, wc);
    register_witness_codes(wc, n_wc, wh);
    let (hok, pok, pitems) = index_witness_headers(wh, n_wh, cc, header.parent_hash);
    k_n_headers = n_wh;
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
        fork = cc_fork(),
        headers_ok = hok,
        parent_hash_ok = pok,
        parent_header_items = pitems,
    }
}
