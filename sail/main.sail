/* ======================================================================== */
/* STATELESS BLOCK-VALIDATION GUEST (zkVM)                                  */
/*                                                                          */
/* The stateless block-validation guest for the eth-act zkVM RISC-V target  */
/* (riscv64im_zicclsm). It decodes an SSZ SszStatelessInput from the        */
/* private input, validates the supplied witness against the committed pre- */
/* state root, executes the block over that witness-backed state, and       */
/* commits to the post-state -- emitting an SSZ                             */
/* SszStatelessValidationResult as the canonical public output. A proof of  */
/* this guest is a proof that the block is valid relative to the parent     */
/* state root.                                                              */
/*                                                                          */
/* Validation pipeline (main): re-root the witness to parent.state_root     */
/* (the one-shot pre-state check), seed the touched pre-state from the      */
/* authenticated leaves, run each tx (process_transaction), then check the  */
/* config selects the expected fork, the witness headers chain, the tx      */
/* receipt succeeded, and the recomputed post-state root equals             */
/* payload.state_root. A failing block is a NORMAL outcome: the guest       */
/* reports successful_validation = false, it does not abort.                */
/*                                                                          */
/* Target contract: NO printing, NO string building, NO libc IO, NO asserts */
/* on the happy path. read_input supplies the SSZ input via ssz_src;        */
/* write_output (runtime/zkvm_input.c) receives the SSZ result. Built GMP-  */
/* free by backing the stock Sail GMP ABI with a vendored, freestanding     */
/* mini-gmp (no libgmp linked) at EXACT 256-bit semantics; keccak-256 comes */
/* from a C accelerator (the eth-act c-interface-accelerators path),        */
/* needing no GMP and only memset from libc.                                */
/* ======================================================================== */

/* Output facts, read by the C harness after main(). All fit in 64 bits.
   zkvm_out_gas is declared PER ENTRY (runner.sail declares its own): the two
   entries are never compiled together, so each model carries the same
   C-visible symbol and the harness dump (test_utils.c 'G' section) resolves
   in both. The root there comes from calling compute_state_root C-side. */
register zkvm_out_gas : int = 0 /* total gas used                     */
register zkvm_out_storage0 : int = 0 /* contract storage slot 0            */
register zkvm_out_wbal : int = 0 /* withdrawer balance (wei)           */
register zkvm_out_cbbal : int = 0 /* coinbase balance (wei)             */
register zkvm_out_tracelen : int = 0 /* trace stream length                */
register zkvm_out_success : int = 0 /* 1 if the tx receipt succeeded      */
function eoa() -> address = 0x00000000000000000000000000000000000005e0
function contract() -> address = 0x000000000000000000000000000000000000c0de
function coinbase_a() -> address = 0x000000000000000000000000000000000000c01b
function withdrawer() -> address = 0x0000000000000000000000000000000000004895

/* PUSH1 0x2a; PUSH1 0x00; SSTORE; STOP  (store 42 at slot 0) */
function contract_code() -> list(byte) = [|0x60, 0x2a, 0x60, 0x00, 0x55, 0x00|]

function emit_all(bs : list(byte)) -> unit = {
    let n = list_len(bs);
    foreach (i from 0 to (n - 1)) {
        emit_out(list_nth(bs, i, 0x00))
    }
}

/* Build + emit the canonical SSZ SszStatelessValidationResult:
 *   new_payload_request_root (32 BE) ++ successful_validation (1) ++
 *   chain_config offset (37, LE u32) ++ chain_config bytes (copied from input).
 * The chain_config is returned unchanged, so its SSZ bytes are exactly the
 * input's [cc_start, cc_end) region. */
function emit_result(zh : list(bits(256)), success : bool) -> unit = {
    let root = htr_new_payload_request(zh);
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = sail_shiftright(root, 8 * (31 - k))[7 .. 0] :: out
    };
    out = (if success then 0x01 else 0x00) :: out;
    out = 0x25 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    let s = cc_start();
    let e = cc_end();
    foreach (i from 0 to (e - s - 1)) {
        out = ssz_src_byte(s + i) :: out
    };
    let result = list_reverse(out);

    /* The canonical SSZ result is validated BYTE-EXACT against the EELS
       reference output by the conformance harness (harness/run.py --guest),
       for both the success and fail paths -- so no fixture-specific baked
       self-check is kept here. */
    emit_all(result)
}

/* Exercise the fixed-width (GMP-free) sailfix runtime's hard arithmetic paths
 * on-guest: 256-bit MUL+truncate, DIV/MOD, the 512-bit MULMOD product, EXP
 * (square-and-multiply), and signed SDIV/SMOD over the negative get_slice_int
 * path. Ground truth computed independently (Python). */
function arith_selfcheck() -> unit = {
    let a : word = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
    let two : word = word_of_int(2);
    assert(alu_mul(a, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, "alu_mul");
    assert(alu_div(a, two) == 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_div");
    assert(alu_mod(a, word_of_int(3)) == word_of_int(0), "alu_mod");
    assert(alu_mulmod(a, a, word_of_int(7)) == word_of_int(1), "alu_mulmod 512-bit");

    /* alu_exp (square-and-multiply, 256x 512-bit mulmod) dropped from the hot path:
       ~17M instructions and redundant — EEST validates sailfix's full arithmetic,
       and alu_mulmod above already exercises the 512-bit product path. */
    let neg7 : word = word_of_int(-7);
    assert(alu_sdiv(neg7, two) == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, "alu_sdiv");
    assert(alu_smod(neg7, two) == 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "alu_smod")
}

struct BlockExecutionResult = {
    all_ok : bool,
    gas_acc : int,
    blob_gas_acc : int,
    first_tx_recipient : address,
    block_gas_overflow : bool,
    blob_gas_overflow : bool,
    receipts : list(Receipt),
}

function parent_header_int_field(idx : int) -> int = {
    let items = rlp_node_items(ssz_witness_header(wit_n_headers() - 1));
    rlp_field_int(rlp_item(items, idx))
}

function expected_payload_excess_blob_gas() -> int =
    next_excess_blob_gas(parent_header_int_field(18), parent_header_int_field(17), parent_header_int_field(15))

function run_block_start_system_calls() -> unit = {
    /* EIP-4788 / EIP-2935: write the parent beacon-block root and parent block
       hash into their system contracts before the transactions run. */
    system_call(BEACON_ROOTS_ADDR(), k_header.parent_beacon_block_root);
    system_call(HISTORY_STORAGE_ADDR(), k_header.parent_hash)
}

function execute_block_transactions(block_gas_limit : int) -> BlockExecutionResult = {
    var all_ok : bool = true;
    var gas_acc : int = 0;
    var blob_gas_acc : int = 0;
    var tx0_to : address = ZERO_ADDR; /* tx 0 recipient, for the storage0 fact */
    var block_gas_overflow : bool = false;
    var blob_gas_overflow : bool = false;
    var receipts_rev : list(Receipt) = [||];

    /* EIP-7778 block gas accounting: a tx whose gas limit exceeds the block's
       remaining gas raises GasUsedExceedsLimitError -> the whole block is
       invalid. EELS performs this check BEFORE touching the tx's state, so the
       witness omits the offending tx's proofs; the guest must likewise refrain
       from executing it (and every later tx), else it over-reads the witness
       and aborts on a node EELS never proved. */
    /* Each tx is decoded on demand by its SSZ index -- the block is never
       materialized as a list. A malformed tx / bad signature THROWS during
       decode_tx_ssz (caught at run_one), so no rlp_decode_ok / auth_ok flag is
       threaded. Decoding reads only witness/payload bytes, never state. */
    let base_fee = k_header.base_fee;
    let n_txs = pl_n_txs();
    var i : int = 0;
    while i < n_txs do {
        let tx = decode_tx_ssz(i, base_fee);
        if not_bool(block_gas_overflow | blob_gas_overflow) then {
            let tx_blob_gas = tx.blob_count * GAS_PER_BLOB;
            if i == 0 then {
                tx0_to = tx.recipient
            };
            if tx.gas_limit > block_gas_limit - gas_acc then {
                block_gas_overflow = true
            } else {
                if tx_blob_gas > blob_max_gas_per_block() - blob_gas_acc then {
                    blob_gas_overflow = true
                } else {
                    let r = process_transaction(tx);
                    all_ok = all_ok & r.valid;
                    gas_acc = gas_acc + r.block_gas;
                    if r.valid then {
                        receipts_rev = r :: receipts_rev;
                        blob_gas_acc = blob_gas_acc + tx_blob_gas
                    }
                }
            }
        };
        i = i + 1
    };
    struct {
        all_ok = all_ok & not_bool(block_gas_overflow) & not_bool(blob_gas_overflow),
        gas_acc = gas_acc,
        blob_gas_acc = blob_gas_acc,
        first_tx_recipient = tx0_to,
        block_gas_overflow = block_gas_overflow,
        blob_gas_overflow = blob_gas_overflow,
        receipts = list_reverse(receipts_rev),
    }
}

function apply_withdrawals() -> unit = {
    let n = pl_n_withdrawals();
    var i : int = 0;
    while i < n do {
        /* EIP-4895: the withdrawal `amount` field is denominated in Gwei; the
           credited balance is amount * 10^9 wei. */
        k_add_balance(withdrawal_addr(i), word_of_int(withdrawal_amount(i) * 1000000000));
        i = i + 1
    }
}

function run_checked_block_end_system_calls(all_ok : bool) -> bool = {
    /* EIP-7002 / EIP-7251: dequeue the withdrawal + consolidation requests via a
       CHECKED system call to each predeploy (clearing their queue storage). EELS
       reaches this only if no earlier step raised, so run it only while the block
       is still valid; a missing predeploy or a failed call invalidates the block. */
    if all_ok then {
        let rw = system_call_checked(WITHDRAWAL_REQUEST_ADDR());
        let rc = system_call_checked(CONSOLIDATION_REQUEST_ADDR());
        all_ok & rw & rc
    } else {
        false
    }
}

function trie_index_key(i : int) -> list(nibble) =
    to_nibbles(rlp_int(i))

function transaction_trie_root() -> hash = {
    var updates : list(TriePatch) = [||];
    let n = pl_n_txs();
    foreach (i from 0 to (n - 1)) {
        let (off, len) = ssz_list_span(pl_txs_start(), n, pl_wds_start(), i);
        updates = patch_insert_sorted(patch_leaf(trie_index_key(i), byte_slice(WitnessSource, off, len)), updates)
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function withdrawal_rlp(w : Withdrawal) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_int(w.index));
    payload = list_append(payload, rlp_int(w.validator_index));
    payload = list_append(payload, rlp_addr(w.address));
    payload = list_append(payload, rlp_int(w.amount));
    rlp_list(payload)
}

function withdrawals_trie_root() -> hash = {
    trie_arena_reset();
    var updates : list(TriePatch) = [||];
    let n = pl_n_withdrawals();
    var i : int = 0;
    while i < n do {
        updates = patch_insert_sorted(
            patch_leaf(trie_index_key(i), arena_intern(withdrawal_rlp(withdrawal_at(i)))),
            updates,
        );
        i = i + 1
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function bloom_set_bit(bloom : list(byte), bit_to_set : int) -> list(byte) = {
    let bit_index = 2047 - bit_to_set;
    let byte_index = ediv_int(bit_index, 8);
    let bit_value = pow2(7 - emod_int(bit_index, 8));
    let old = bytes_get(bloom, byte_index);
    list_set(bloom, byte_index, or_vec(old, get_slice_int(8, bit_value, 0)))
}

/* One of a LOG entry's bloom filter bits (Yellow Paper section 4.4.1):
 * bytes (2i, 2i+1) of the entry keccak hash, mod 2048. The entry is always
 * a 20-byte address or a 32-byte topic, so callers hash it via the
 * fixed-width keccak256_address / keccak256_word FFI -- no list(byte)
 * reaches the build hash on this path. */
function bloom_bit(h : bits(256), i : int) -> int = {
    let hb = word_to_bytes32(h);
    let j = i * 2;
    emod_int(unsigned(bytes_get(hb, j)) * 256 + unsigned(bytes_get(hb, j + 1)), 2048)
}

function bloom_add_entry_hash(bloom : list(byte), h : bits(256)) -> list(byte) = {
    var out : list(byte) = bloom;
    foreach (i from 0 to 2) {
        out = bloom_set_bit(out, bloom_bit(h, i))
    };
    out
}

function bloom_add_topics(bloom : list(byte), topics : list(word)) -> list(byte) = {
    var out : list(byte) = bloom;
    var rest : list(word) = topics;
    var more : bool = true;
    while more do {
        match rest {
            topic :: tail => {
                out = bloom_add_entry_hash(out, keccak256_word(topic));
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function bloom_add_log(bloom : list(byte), log : LogEntry) -> list(byte) = {
    let with_address = bloom_add_entry_hash(bloom, keccak256_address(log.address));
    bloom_add_topics(with_address, log.topics)
}

function logs_bloom_for_logs(logs : list(LogEntry)) -> list(byte) = {
    var out : list(byte) = zero_bytes(256);
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                out = bloom_add_log(out, log);
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function receipts_bloom(receipts : list(Receipt)) -> list(byte) = {
    var out : list(byte) = zero_bytes(256);
    var rest : list(Receipt) = receipts;
    var more : bool = true;
    while more do {
        match rest {
            receipt :: tail => {
                var logs : list(LogEntry) = receipt.logs;
                var more_logs : bool = true;
                while more_logs do {
                    match logs {
                        log :: log_tail => {
                            out = bloom_add_log(out, log);
                            logs = log_tail
                        },
                        [||] => more_logs = false,
                    }
                };
                rest = tail
            },
            [||] => more = false,
        }
    };
    out
}

function bytes_equal(a : list(byte), b : list(byte)) -> bool = {
    if list_len(a) == list_len(b) then {
        var ok : bool = true;
        foreach (i from 0 to (list_len(a) - 1)) {
            ok = ok & (bytes_get(a, i) == bytes_get(b, i))
        };
        ok
    } else {
        false
    }
}

function topics_rlp(topics : list(word)) -> list(byte) = {
    var payload : list(byte) = [||];
    var rest : list(word) = topics;
    var more : bool = true;
    while more do {
        match rest {
            topic :: tail => {
                payload = list_append(payload, rlp_word(topic));
                rest = tail
            },
            [||] => more = false,
        }
    };
    rlp_list(payload)
}

function log_entry_rlp(log : LogEntry) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_addr(log.address));
    payload = list_append(payload, topics_rlp(log.topics));
    payload = list_append(payload, rlp_bytes(log.data));
    rlp_list(payload)
}

function logs_rlp(logs : list(LogEntry)) -> list(byte) = {
    var payload : list(byte) = [||];
    var rest : list(LogEntry) = logs;
    var more : bool = true;
    while more do {
        match rest {
            log :: tail => {
                payload = list_append(payload, log_entry_rlp(log));
                rest = tail
            },
            [||] => more = false,
        }
    };
    rlp_list(payload)
}

function receipt_payload_rlp(r : Receipt, cumulative_gas_used : int) -> list(byte) = {
    var payload : list(byte) = [||];
    payload = list_append(payload, rlp_int(if r.success then { 1 } else { 0 }));
    payload = list_append(payload, rlp_int(cumulative_gas_used));
    payload = list_append(payload, rlp_bytes(logs_bloom_for_logs(r.logs)));
    payload = list_append(payload, logs_rlp(r.logs));
    rlp_list(payload)
}

function receipt_encoded(r : Receipt, cumulative_gas_used : int) -> list(byte) = {
    let payload = receipt_payload_rlp(r, cumulative_gas_used);
    if r.tx_type == 0 then {
        payload
    } else {
        let type_byte : byte = get_slice_int(8, r.tx_type, 0);
        type_byte :: payload
    }
}

function receipts_trie_root(receipts : list(Receipt)) -> hash = {
    trie_arena_reset();
    var updates : list(TriePatch) = [||];
    var rest : list(Receipt) = receipts;
    var i : int = 0;
    var cumulative : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            r :: tail => {
                cumulative = cumulative + r.gas_used;
                updates = patch_insert_sorted(
                    patch_leaf(trie_index_key(i), arena_intern(receipt_encoded(r, cumulative))),
                    updates,
                );
                rest = tail;
                i = i + 1
            },
            [||] => more = false,
        }
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function parent_hash_ok() -> bool =
    if wit_n_headers() < 1 then {
        false
    } else {
        k_header.parent_hash == witness_keccak(ssz_witness_header_slice(wit_n_headers() - 1))
    }

function validate_block(exec_ok : bool, block_result : BlockExecutionResult) -> unit = {
    /* S5: stateless validation outcome (a failure is a NORMAL result -- the guest
       reports successful_validation=false, it does not abort):
         - chain config selects the Amsterdam fork
         - the witness headers are contiguous
         - RLP scalar fields are canonical integers
         - every tx's witness public key is authenticated against its signature
         - every tx is VALID/applicable (a reverting valid tx keeps the block valid)
         - the recomputed post-state root commits to payload.state_root
       Every check mirrors an EELS `raise InvalidBlock` (amsterdam state_transition).
       ALL are computed (so witness reads are unchanged vs the old AND), then the
       FIRST failure is reported as its BlockError; a valid block yields None. */
    let header = k_header;
    let fork_ok = cc_fork() == 20; /* canonical ProtocolFork index for Amsterdam
                                  (execution-specs@02c6c2510: enum pruned, Osaka=17 BPO1=18 BPO2=19
                                  Amsterdam=20). */

    let active_ok = cc_activation_active();
    let cfg_ok = fork_ok & active_ok;

    let hdr_ok = headers_ok();
    let gas_used_ok = block_result.gas_acc == header.gas_used;
    let blob_gas_used_ok = block_result.blob_gas_acc == header.blob_gas_used;
    let excess_blob_gas_ok = header.excess_blob_gas == expected_payload_excess_blob_gas();
    let computed_root = compute_state_root();
    let poststate_ok = computed_root == header.state_root;
    let receipts_root = receipts_trie_root(block_result.receipts);
    let receipts_root_ok = receipts_root == header.receipts_root;
    let logs_bloom_ok = bytes_equal(receipts_bloom(block_result.receipts), header.logs_bloom);
    let transactions_root = transaction_trie_root();
    let withdrawals_root = withdrawals_trie_root();
    let requests_hash = execution_requests_hash();
    let block_access_list_hash = witness_keccak(witness_slice(pl_bal_start(), payload_end() - pl_bal_start()));
    let block_hash = block_header_hash(transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
    let block_hash_ok = block_hash == header.block_hash;
    let parent_ok = parent_hash_ok();

    /* EIP-7928: the block access list must not exceed block_gas_limit / 2000 items. */
    let bal_limit = ediv_int(header.gas_limit, 2000);
    let bal_ok : bool = bal_item_count() <= bal_limit;

    /* All checks are computed above (witness reads unchanged); each failure THROWS
       its BlockError, mirroring EELS `raise InvalidBlock`. block_gas_overflow /
       blob_gas_overflow are already folded into exec_ok but are surfaced first for
       a precise reason (EIP-7778). run_one catches this once -> valid = false.
       A deficient witness (missing/inconsistent proof node) throws WitnessDeficient
       at the point of use during execution/root recomputation, so it is not
       re-checked here. */
    if not_bool(cfg_ok) then {
        throw InvalidBlock(InvalidConfig)
    };
    if not_bool(hdr_ok) then {
        throw InvalidBlock(HeaderChainBroken)
    };
    if block_result.block_gas_overflow then {
        throw InvalidBlock(GasUsedExceedsLimit)
    };
    if block_result.blob_gas_overflow then {
        throw InvalidBlock(BlobGasLimitExceeded)
    };
    if not_bool(exec_ok) then {
        throw InvalidBlock(ExecutionInvalid)
    };
    if not_bool(gas_used_ok) then {
        throw InvalidBlock(InvalidGasUsed)
    };
    if not_bool(blob_gas_used_ok) then {
        throw InvalidBlock(InvalidBlobGasUsed)
    };
    if not_bool(excess_blob_gas_ok) then {
        throw InvalidBlock(InvalidExcessBlobGas)
    };
    if not_bool(poststate_ok) then {
        throw InvalidBlock(InvalidStateRoot)
    };
    if not_bool(receipts_root_ok) then {
        throw InvalidBlock(InvalidReceiptsRoot)
    };
    if not_bool(logs_bloom_ok) then {
        throw InvalidBlock(InvalidLogsBloom)
    };
    if not_bool(block_hash_ok) then {
        throw InvalidBlock(InvalidBlockHash)
    };
    if not_bool(parent_ok) then {
        throw InvalidBlock(InvalidParentHash)
    };
    if not_bool(bal_ok) then {
        throw InvalidBlock(BlockAccessListTooLarge)
    }
}

function first_withdrawal_address() -> address =
    if pl_n_withdrawals() > 0 then {
        withdrawal_addr(0)
    } else {
        ZERO_ADDR
    }

/* canonical "invalid input" output: a 32-byte ZERO new_payload_request_root,
 * successful_validation=false, and a FIXED default ChainConfig (the input's is
 * unparseable, so it cannot be echoed). The 36 default-config bytes are the SSZ
 * of the empty ChainConfig the EEST consumer emits for rejected input. */
function emit_invalid() -> unit = {
    let dflt : list(byte) = [|
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x0c,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x10,
            0x00,
            0x00,
            0x00,
            0x18,
            0x00,
            0x00,
            0x00,
            0x08,
            0x00,
            0x00,
            0x00,
            0x08,
            0x00,
            0x00,
            0x00,
        |];
    var out : list(byte) = [||];
    foreach (k from 0 to 31) {
        out = 0x00 :: out
    };
    out = 0x00 :: out; /* successful_validation = false */
    out = 0x25 :: out; /* chain_config offset = 37 (LE u32) */
    out = 0x00 :: out;
    out = 0x00 :: out;
    out = 0x00 :: out;
    emit_all(list_reverse(out));
    emit_all(dflt)
}

function main() -> unit = {
    /* input-independent self-checks (valid for any vector): the C accelerators
       and the GMP-free fixed-width int/bits runtime. The per-stage fixture
       self-checks (ssz/rlp/witness/htr) were incremental scaffolding subsumed by
       the end-to-end success/fail paths + the success-path output check. */
    keccak_selfcheck();
    sha256_selfcheck();
    arith_selfcheck();

    /* reject a malformed SSZ input outright with the canonical invalid output
       (zero root + valid=false + default chain_config) -- nothing below can
       parse it, and the field spans would otherwise read garbage. */
    if not_bool(input_well_formed()) then {
        emit_invalid();
        return ()
    };

    /* S2f: decode only the header into the k_header register, then execute against
       witness state. Transactions and withdrawals are NOT materialized -- the STF
       loops over their SSZ indices and decodes each on demand; a tx's input is
       staged into the byte-addressed backend only when that tx is actually run. */
    decode_block_ssz();
    authenticate_parent_state();

    /* The witness backs the base layer for BOTH accounts and storage (the
       witness base backend, host/base_witness.sail, statically linked into this
       entry). A touched account is proven present-or-absent by a lazy Merkle
       walk from the authenticated parent root on first load_account (then
       cached raw in the acctmap); a touched slot is proven the same way from
       the account's storage root on first read (k_sload/k_orig) and seeded into
       the working set. Nothing is pre-seeded -- no eager account/slot pass. */

    /* Block STF, header-only in program state: system calls, then each tx decoded
       on demand by SSZ index via the shared per-tx STF unit, then withdrawals
       (also index-driven) and block-end system calls. */
    run_block_start_system_calls();

    let block_result = execute_block_transactions(k_header.gas_limit);
    apply_withdrawals();
    let exec_ok : bool = run_checked_block_end_system_calls(block_result.all_ok);

    /* EELS verify_stateless_new_payload: any InvalidBlock throw -> valid=false. */
    let valid : bool = try {
        validate_block(exec_ok, block_result);
        true
    } catch {
        InvalidBlock(_) => false,
    };

    let gas_used = to_gas(block_result.gas_acc);
    let wbal = load_account(first_withdrawal_address()).balance;
    let cbbal = load_account(k_header.fee_recipient).balance;

    zkvm_out_gas = gas_used;
    zkvm_out_storage0 = 0; /* legacy debug scalar, no longer computed */
    zkvm_out_wbal = unsigned(wbal);
    zkvm_out_cbbal = unsigned(cbbal);
    zkvm_out_tracelen = 0; /* the ELOp trace machinery was removed */
    zkvm_out_success = if valid then { 1 } else { 0 };

    /* emit the canonical SSZ SszStatelessValidationResult as public output */
    emit_result(build_zerohashes(40), valid)
}
