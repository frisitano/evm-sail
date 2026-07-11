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

function expected_payload_excess_blob_gas(items : list(RlpField)) -> int =
    next_excess_blob_gas(
        parent_header_int_field(items, 18),
        parent_header_int_field(items, 17),
        parent_header_int_field(items, 15),
    )

function run_block_start_system_calls() -> unit = {
    /* EIP-4788 / EIP-2935: write the parent beacon-block root and parent block
       hash into their system contracts before the transactions run. */
    system_call(BEACON_ROOTS_ADDR(), k_header.parent_beacon_block_root);
    system_call(HISTORY_STORAGE_ADDR(), k_header.parent_hash)
}

function execute_block_transactions(txs : list(Transaction), block_gas_limit : int) -> BlockExecutionResult = {
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
    /* Transactions were decoded (and signature-verified) by the single input
       pass, as EELS decodes the block body before executing it; this loop only
       executes them. */
    var rest : list(Transaction) = txs;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            tx :: txs_rest => {
                rest = txs_rest;
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
            },
        }
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

function apply_withdrawals(wds : list(Withdrawal)) -> unit = {
    var rest : list(Withdrawal) = wds;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            w :: r => {
                /* EIP-4895: the withdrawal `amount` field is denominated in Gwei;
                   the credited balance is amount * 10^9 wei. */
                k_add_balance(w.address, word_of_int(w.amount * 1000000000));
                rest = r
            },
        }
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

function transaction_trie_root(txs : list(Transaction)) -> hash = {
    var updates : list(TriePatch) = [||];
    var rest : list(Transaction) = txs;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            t :: r => {
                updates = patch_insert_sorted(patch_leaf(trie_index_key(i), t.raw), updates);
                rest = r;
                i = i + 1
            },
        }
    };
    trie_root(EMPTY_TRIE_ROOT, updates)
}

function withdrawals_trie_root(wds : list(Withdrawal)) -> hash = {
    trie_arena_reset();
    var updates : list(TriePatch) = [||];
    var rest : list(Withdrawal) = wds;
    var i : int = 0;
    var more : bool = true;
    while more do {
        match rest {
            [||] => more = false,
            w :: r => {
                updates = patch_insert_sorted(patch_leaf(trie_index_key(i), arena_intern(withdrawal_rlp(w))), updates);
                rest = r;
                i = i + 1
            },
        }
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
    payload = list_append(payload, rlp_bytes(read_byte_slice(log.data)));
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
    if tx_type_byte(r.tx_type) == 0 then {
        payload
    } else {
        let type_byte : byte = get_slice_int(8, tx_type_byte(r.tx_type), 0);
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

function validate_block(input : StatelessInput, exec_ok : bool, block_result : BlockExecutionResult) -> unit = {
    /* Stateless validation outcome (a failure is a NORMAL result -- the guest
       reports successful_validation=false, it does not abort):
         - chain config selects the Amsterdam fork
         - the witness headers are contiguous
         - RLP scalar fields are canonical integers
         - every tx's witness public key is authenticated against its signature
         - every tx is VALID/applicable (a reverting valid tx keeps the block valid)
         - the recomputed post-state root commits to payload.state_root
       Every check mirrors an EELS `raise InvalidBlock` (amsterdam state_transition).
       ALL checks are computed unconditionally (fixed witness-read footprint); the
       FIRST failure is reported as its BlockError; a valid block yields None. */
    let header = k_header;
    let fork_ok = input.chain_config.fork_index == 20; /* canonical ProtocolFork index for Amsterdam
                                  (execution-specs@02c6c2510: enum pruned, Osaka=17 BPO1=18 BPO2=19
                                  Amsterdam=20). */

    let active_ok = input.chain_config.activation_active;
    let cfg_ok = fork_ok & active_ok;

    let hdr_ok = input.headers_ok;
    let gas_used_ok = block_result.gas_acc == header.gas_used;
    let blob_gas_used_ok = block_result.blob_gas_acc == unsigned(header.blob_gas_used);
    let excess_blob_gas_ok = unsigned(header.excess_blob_gas) == expected_payload_excess_blob_gas(input.parent_header_items);
    let computed_root = compute_state_root();
    let poststate_ok = computed_root == header.state_root;
    let receipts_root = receipts_trie_root(block_result.receipts);
    let receipts_root_ok = receipts_root == header.receipts_root;
    let logs_bloom_ok = bytes_equal(receipts_bloom(block_result.receipts), header.logs_bloom);
    let transactions_root = transaction_trie_root(input.block.body.transactions);
    let withdrawals_root = withdrawals_trie_root(input.block.body.withdrawals);
    let requests_hash = execution_requests_hash();
    let block_access_list_hash = keccak256(input.block.body.block_access_list);
    let block_hash = block_header_hash(header, transactions_root, withdrawals_root, requests_hash, block_access_list_hash);
    let block_hash_ok = block_hash == header.block_hash;
    let parent_ok = input.parent_hash_ok; /* asserted by the input pass against the witness chain */

    /* EIP-7928: the block access list must not exceed block_gas_limit / 2000 items. */
    let bal_limit = ediv_int(header.gas_limit, 2000);
    let bal_ok : bool = bal_item_count(input.block.body.block_access_list) <= bal_limit;

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

    /* THE single traversal: geometry, node/code/header stores, block header,
       every tx (sender recovery + sig verify) and withdrawal -- decoded once,
       returned as values; also installs k_header / the ancestor-hash table /
       k_parent_state_root. */
    let input = decode_stateless_input();

    /* The witness backs the base layer for BOTH accounts and storage (the
       witness base backend, host/base_witness.sail, statically linked into this
       entry). A touched account is proven present-or-absent by a lazy Merkle
       walk from the authenticated parent root on first k_aload (then
       cached raw in the acctmap); a touched slot is proven the same way from
       the account's storage root on first read (k_sload) and seeded into
       the working set. Nothing is pre-seeded -- no eager account/slot pass. */

    /* Block STF: system calls, then each decoded tx through the shared per-tx
       STF unit, then withdrawals and block-end system calls. */
    run_block_start_system_calls();

    let block_result = execute_block_transactions(input.block.body.transactions, k_header.gas_limit);
    apply_withdrawals(input.block.body.withdrawals);
    let exec_ok : bool = run_checked_block_end_system_calls(block_result.all_ok);

    /* EELS verify_stateless_new_payload: any InvalidBlock throw -> valid=false. */
    let valid : bool = try {
        validate_block(input, exec_ok, block_result);
        true
    } catch {
        InvalidBlock(_) => false,
    };

    let gas_used = to_gas(block_result.gas_acc);

    /* Unused debug scalars stay ZERO: reading any account here (a minimal
       witness proves only what EXECUTION touched) would turn a valid block
       into a WitnessDeficient abort -- touch pre-state only where EELS does. */
    zkvm_out_gas = gas_used;
    zkvm_out_storage0 = 0;
    zkvm_out_wbal = 0;
    zkvm_out_cbbal = 0;
    zkvm_out_tracelen = 0;
    zkvm_out_success = if valid then { 1 } else { 0 };

    /* emit the canonical SSZ SszStatelessValidationResult as public output */
    emit_result(build_zerohashes(40), valid)
}
