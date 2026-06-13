/* ======================================================================== */
/* SINGLE-BLOCK / EEST RUNNER                                               */
/*                                                                          */
/* The native entry point that runs an Ethereum block end to end. A test    */
/* harness compiles this model ONCE, then feeds it a stream of fixtures at  */
/* runtime: each case seeds a pre-state, sets the block environment, builds */
/* one transaction, runs it through process_block (block.sail), and dumps   */
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
/* case; "ACC addr nonce balance" per account; "STO addr slot value" per    */
/* live storage slot; "GAS gasUsed" for the tx; and, when wantRoot is set,  */
/* "ROOT <hex>" carrying the recomputed post-state root.                    */
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

/* emit the post-state for the external comparator: one "STO addr slot value"
 * line per live storage slot (the kernel's flat C overlay), all decimal. */
function dump_storage() -> unit = {
    let n = storage_count();
    foreach (j from 0 to (n - 1)) {
        let (a, s, v) = storage_entry(j);
        print_endline(
            concat_str(
                "STO ",
                concat_str(
                    dec_str(unsigned(sail_zero_extend(a, 256))),
                    concat_str(" ", concat_str(dec_str(unsigned(s)), concat_str(" ", dec_str(unsigned(v))))),
                ),
            ),
        )
    }
}

/* one "ACC addr nonce balance" line per account in the working set */
function dump_accounts(es : list(AccountEntry)) -> unit = match es {
    [||]   => (),
    e :: r => {
        print_endline(
            concat_str(
                "ACC ",
                concat_str(
                    dec_str(unsigned(sail_zero_extend(e.ae_addr, 256))),
                    concat_str(
                        " ",
                        concat_str(
                            dec_str(e.ae_acc.acc_nonce),
                            concat_str(" ", dec_str(unsigned(e.ae_acc.acc_balance))),
                        ),
                    ),
                ),
            ),
        );
        dump_accounts(r)
    },
}
function dump_state() -> unit = {
    dump_accounts(k_accounts);
    dump_storage()
}

/* decode and run one fixture: seed pre-state, set the block env, build the
 * single transaction, run process_block, and dump the post-state. */
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
    k_set_block(
        struct {
            be_number = bnum,
            be_timestamp = bts,
            be_gas_limit = bgl,
            be_coinbase = bcb,
            be_base_fee = bbf,
            be_prev_randao = bpr,
            be_chain_id = bci,
            be_blob_base_fee = bbbf,
            be_excess_blob_gas = word_of_int(0),
        },
    );
    let sender = na();
    let tnonce = ni();
    let gp = nw();
    let pf = nw();
    let gl = ni();
    let isc = ni();
    let dst = na();
    let tval = nw();
    let dlen = ni();

    /* stream calldata straight into the tx-input memory slot (no Sail list) */
    txin_begin(get_slice_int(64, 0, 0));
    foreach (i from 0 to (dlen - 1)) {
        let _ = i;
        txin_byte(get_slice_int(8, ni(), 0))
    };

    /* EIP-2930 access list: n_addrs addresses, then n_keys (addr, slot) pairs */
    let n_al_a = ni();
    var al_addrs : list(address) = [||];
    foreach (i from 0 to (n_al_a - 1)) {
        let _ = i;
        al_addrs = list_append(al_addrs, [|na()|])
    };
    let n_al_k = ni();
    var al_keys : list(SlotKey) = [||];
    foreach (i from 0 to (n_al_k - 1)) {
        let _ = i;
        let ka = na();
        let ks = nw();
        al_keys = list_append(al_keys, [|struct { sk_addr = ka, sk_slot = ks }|])
    };
    let nblob = ni();
    let maxfee = nw();
    let maxblobfee = nw();
    let maxprio = nw();
    let is4 = ni(); /* type-4 (set-code) envelope flag */

    /* EIP-7702 authorization list: n_auth tuples (valid_sig, authority, address, nonce, chain_id) */
    let n_auth = ni();
    var auths : list(Authorization) = [||];
    foreach (i from 0 to (n_auth - 1)) {
        let _ = i;
        let vsig = ni();
        let auth = na();
        let dele = na();
        let anon = ni();
        let acid = ni();
        auths = list_append(
            auths,
            [|
                struct {
                    au_valid_sig = vsig != 0,
                    au_authority = auth,
                    au_address = dele,
                    au_nonce = anon,
                    au_chain_id = acid,
                },
            |],
        )
    };

    /* EIP-4844 blob section: is_blob flag (type-3 marker), then n 32-byte
       versioned hashes (decimal words). Mirrors run_eest.py / revm-eest encode(). */
    let is_blob = ni();
    let n_bh = ni();
    var bhs : list(word) = [||];
    foreach (i from 0 to (n_bh - 1)) {
        let _ = i;
        bhs = list_append(bhs, [|nw()|])
    };
    let txn : Transaction = struct {
            t_sender = sender,
            t_nonce = tnonce,
            t_gas_price = gp,
            t_priority_fee = pf,
            t_gas_limit = to_gas(gl),
            t_is_create = isc != 0,
            t_to = dst,
            t_value = tval,
            t_index = 0,
            t_al_addrs = al_addrs,
            t_al_keys = al_keys,
            t_blob_count = nblob,
            t_max_fee = maxfee,
            t_max_blob_fee = maxblobfee,
            t_max_priority = maxprio,
            t_is_setcode = is4 != 0,
            t_auth_list = auths,
            t_is_blob = is_blob != 0,
            t_blob_hashes = bhs,
        };
    let rs = process_block(struct { blk_env = k_block, blk_txs = [|txn|], blk_withdrawals = [||] });
    dump_state();
    let gu : gas = match rs {
        r :: _ => r.rc_gas_used,
        [||] => 0,
    };
    print_endline(concat_str("GAS ", dec_str(gu)))
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
            /* recomputed world-state root (YP stateRoot), via the hash builder */
            print_endline(concat_str("ROOT ", hex_str(unsigned(compute_state_root_hb()))))
        };
        more = ni()
    }
}
