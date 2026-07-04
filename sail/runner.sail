/* ======================================================================== */
/* SINGLE-BLOCK / EEST RUNNER                                               */
/*                                                                          */
/* The native entry point that runs an Ethereum block end to end. A test    */
/* harness compiles this model ONCE, then feeds it a stream of fixtures at  */
/* runtime: each case seeds a pre-state, sets the block environment, builds */
/* one transaction, runs it through process_transaction, and dumps          */
/* the resulting accounts + storage for an external comparator. The data is */
/* INPUT, not generated code -- there is no per-fixture recompilation.      */
/*                                                                          */
/* The Ethereum Execution Spec Tests (EEST) driver revm-eest/run_eest.py    */
/* encodes each fixture into the wire stream, then diffs the ACC/STO/GAS/   */
/* ROOT lines below against the fixture's expected post-state.              */
/*                                                                          */
/* Input contract: the stream is read through ssz_src (host/io.sail), the   */
/* same input byte source the zkVM guest uses; revm-eest/runner_ffi.c       */
/* buffers stdin once and serves it by index. Each value is a varint        */
/* [len : 1 byte][big-endian magnitude : len bytes]; ni() reads one. Stream */
/* schema (a flat sequence of varints):                                     */
/*   fork  wantRoot                                                         */
/*   repeat: 1  <case>            ... then 0 to stop                        */
/*   <case> = nAcc                                                          */
/*             { addr nonce balance codeLen <codeBytes> nSto {slot val}* }* */
/*           number timestamp gasLimit coinbase baseFee prevRandao chainId  */
/*             blobBaseFee                                                  */
/*           sender txNonce gasPrice prioFee gasLimit isCreate to value     */
/*             dataLen <dataBytes>                                          */
/*           <EIP-2930 access list> <EIP-7702 auth list> <EIP-4844 blobs>   */
/*                                                                          */
/* Output contract (one line per record, decimal unless noted): "CASE" per  */
/* case; "ACC accountHash nonce balance" per account; "STO accountHash slot */
/* value" per live storage slot; "GAS gasUsed" for the tx; and, when        */
/* wantRoot is set, "ROOT <hex>" carrying the recomputed post-state root.   */
/* ======================================================================== */

$include "evm.sail"
$include <string.sail>

/* Cursor over the private-input byte stream (ssz_src, host/io.sail), the byte
 * source the model reads everywhere; revm-eest/runner_ffi.c serves it from
 * stdin. ni() reads one varint scalar: [len : 1 byte][big-endian magnitude]. */
register in_pos : int = 0
function ni() -> int = {
    let l = unsigned(ssz_src_byte(in_pos));
    var v : int = 0;
    foreach (k from 0 to (l - 1)) {
        v = v * 256 + unsigned(ssz_src_byte(in_pos + 1 + k))
    };
    in_pos = in_pos + 1 + l;
    v
}

/* one scalar as a 256-bit word */
function nw() -> word = word_of_int(ni())

/* one scalar as the low-20-byte address */
function na() -> address = word_to_address(word_of_int(ni()))

/* the next n scalars as raw bytes (code / calldata) */
function nbytes(n : int) -> list(byte) = {
    /* prepend then reverse (O(n)); list_append-per-byte was O(n^2) on large code/data */
    var out : list(byte) = [||];
    foreach (i from 0 to (n - 1)) {
        let _ = i;
        out = get_slice_int(8, ni(), 0) :: out
    };
    list_reverse(out)
}

/* emit the post-state for the external comparator: one
 * "STO accountHash slot value" line per storage row. The harness maps fixture
 * addresses to keccak(address) before comparing. */
function dump_storage() -> unit = {
    let ns = storage_count();
    foreach (j from 0 to (ns - 1)) {
        let (ak, s, v) = storage_hashed_entry(j);
        print_endline(
            concat_str(
                "STO ",
                concat_str(
                    dec_str(unsigned(ak)),
                    concat_str(" ", concat_str(dec_str(unsigned(s)), concat_str(" ", dec_str(unsigned(v))))),
                ),
            ),
        )
    }
}

/* one "ACC accountHash nonce balance" line per account in the working set */
function dump_accounts() -> unit = {
    let n = unsigned(acctmap_count());
    foreach (j from 0 to (n - 1)) {
        acctmap_at(get_slice_int(64, j, 0));
        let ak = selected_account_key_hash();
        print_endline(
            concat_str(
                "ACC ",
                concat_str(
                    dec_str(unsigned(ak)),
                    concat_str(
                        " ",
                        concat_str(
                            dec_str(unsigned(acctmap_at_nonce())),
                            concat_str(" ", dec_str(unsigned(selected_account_balance()))),
                        ),
                    ),
                ),
            ),
        )
    }
}
function dump_state() -> unit = {
    dump_accounts();
    dump_storage()
}

/* decode and run one fixture: seed pre-state, set the block env, build the
 * single transaction, run process_transaction, and dump the post-state. */
function run_one() -> unit = {
    k_world_reset();
    let nacc = ni();
    foreach (i from 0 to (nacc - 1)) {
        let _ = i;
        let a = na();
        let nonce = ni();
        let bal = nw();
        let clen = ni();
        let cbytes = nbytes(clen);
        k_seed_account(a, nonce, bal, cbytes);
        let nsto = ni();
        foreach (j from 0 to (nsto - 1)) {
            let _ = j;
            let s = nw();
            let v = nw();
            k_seed_storage(a, s, v)
        }
    };
    let bnum = nw();
    let bts = nw();
    let bgl = nw();
    let bcb = na();
    let bbf = nw();
    let bpr = nw();
    let bci = nw();
    let bbbf = nw();
    let bslot = nw(); /* EIP-7843 SLOTNUM: env slotNumber */
    k_set_block(
        struct {
            number = bnum,
            timestamp = bts,
            gas_limit = bgl,
            coinbase = bcb,
            base_fee = bbf,
            prev_randao = bpr,
            chain_id = bci,
            blob_base_fee = bbbf,
            excess_blob_gas = word_of_int(0),
            slot = bslot,
        },
    );
    let sender = na();
    let decoded_nonce = ni();
    let decoded_gas_price = nw();
    let decoded_priority_fee = nw();
    let decoded_gas_limit = ni();
    let decoded_is_create = ni();
    let decoded_recipient = na();
    let decoded_value = nw();
    let decoded_input_len = ni();

    var decoded_input : list(byte) = [||];
    foreach (i from 0 to (decoded_input_len - 1)) {
        let _ = i;
        decoded_input = get_slice_int(8, ni(), 0) :: decoded_input
    };
    decoded_input = list_reverse(decoded_input);

    /* EIP-2930 access list: n_addrs addresses, then n_keys (addr, slot) pairs */
    let access_list_address_count = ni();
    var decoded_access_list_addresses : list(address) = [||];
    foreach (i from 0 to (access_list_address_count - 1)) {
        let _ = i;
        decoded_access_list_addresses = list_append(decoded_access_list_addresses, [|na()|])
    };
    let access_list_slot_count = ni();
    var decoded_access_list_slots : list(SlotKey) = [||];
    foreach (i from 0 to (access_list_slot_count - 1)) {
        let _ = i;
        let ka = na();
        let ks = nw();
        decoded_access_list_slots = list_append(decoded_access_list_slots, [|struct { address = ka, slot = ks }|])
    };
    let is_access_list_tx = ni(); /* type-1 (EIP-2930) envelope flag */
    let is_dynamic_fee_tx = ni(); /* EIP-1559-style fee-market envelope flag */
    let decoded_blob_count = ni();
    let decoded_max_fee = nw();
    let decoded_max_blob_fee = nw();
    let decoded_max_priority_fee = nw();
    let decoded_is_set_code = ni(); /* type-4 (set-code) envelope flag */

    /* EIP-7702 authorization list: n_auth tuples (valid_sig, authority, address, nonce, chain_id) */
    let authorization_count = ni();
    var decoded_authorizations : list(Authorization) = [||];
    foreach (i from 0 to (authorization_count - 1)) {
        let _ = i;
        let decoded_valid_sig = ni();
        let decoded_authority = na();
        let decoded_delegation_address = na();
        let decoded_authorization_nonce = ni();
        let decoded_authorization_chain_id = ni();
        decoded_authorizations = list_append(
            decoded_authorizations,
            [|
                struct {
                    valid_sig = decoded_valid_sig != 0,
                    authority = decoded_authority,
                    address = decoded_delegation_address,
                    nonce = decoded_authorization_nonce,
                    chain_id = decoded_authorization_chain_id,
                },
            |],
        )
    };

    /* EIP-4844 blob section: is_blob flag (type-3 marker), then n 32-byte
       versioned hashes (decimal words). Mirrors run_eest.py / revm-eest encode(). */
    let is_blob = ni();
    let blob_hash_count = ni();
    var decoded_blob_hashes : list(word) = [||];
    foreach (i from 0 to (blob_hash_count - 1)) {
        let _ = i;
        decoded_blob_hashes = list_append(decoded_blob_hashes, [|nw()|])
    };
    let txn : Transaction = struct {
            tx_type =
                if decoded_is_set_code != 0 then {
                    4
                } else {
                    if is_blob != 0 then {
                        3
                    } else {
                        if is_dynamic_fee_tx != 0 then { 2 } else { if is_access_list_tx != 0 then { 1 } else { 0 } }
                    }
                },
            sender = sender,
            nonce = decoded_nonce,
            gas_price = decoded_gas_price,
            priority_fee = decoded_priority_fee,
            gas_limit = to_gas(decoded_gas_limit),
            is_create = decoded_is_create != 0,
            recipient = decoded_recipient,
            value = decoded_value,
            input = decoded_input,
            access_list_addresses = decoded_access_list_addresses,
            access_list_slots = decoded_access_list_slots,
            is_access_list = is_access_list_tx != 0,
            is_dynamic_fee = is_dynamic_fee_tx != 0,
            blob_count = decoded_blob_count,
            max_fee = decoded_max_fee,
            max_blob_fee = decoded_max_blob_fee,
            max_priority_fee = decoded_max_priority_fee,
            is_set_code = decoded_is_set_code != 0,
            authorizations = decoded_authorizations,
            is_blob = is_blob != 0,
            blob_hashes = decoded_blob_hashes,
        };

    /* the block env is already seeded (k_set_block above); a state-test case is a
       single transaction with no withdrawals, so the shared per-tx STF unit
       (process_transaction) IS the block -- no list-based block wrapper needed. */
    let r = process_transaction(txn);
    dump_state();
    print_endline(concat_str("GAS ", dec_str(r.gas_used)))
}

/* read the stream header (fork, wantRoot), then run cases until the 0 sentinel */
function main() -> unit = {
    k_fork = ni(); /* active fork level: selects the fork-gated gas/opcode rules */
    let want_root : int = ni(); /* 0 = skip state root (fast), 1 = compute it */
    var more : int = ni();
    while more != 0 do {
        print_endline("CASE");
        run_one();
        if want_root != 0 then {
            /* recomputed world-state root (YP stateRoot) from the seeded case state */
            print_endline(concat_str("ROOT ", hex_str(unsigned(compute_state_root_updates_only()))))
        };
        more = ni()
    }
}
