/* ======================================================================== */
/* SHARED BASE BACKEND (MPT-walk pre-state resolvers)                       */
/*                                                                          */
/* The single pre-state base-layer backend for every executable entry. The  */
/* base is a lazy Merkle walk from an authenticated parent root             */
/* (k_parent_state_root) over the node DB: for the guest it comes from the  */
/* SSZ witness; for the runner both come from                               */
/* the harness-built alloc MPT (populated in runner.sail). Either way the   */
/* trie is complete along every touched path (fail closed on a missing      */
/* node), so the resolvers are identical. BLOCKHASH, once the only backend  */
/* difference, is unified below: it reads the shared block number (k_header)*/
/* and the ancestor-header count (k_n_headers), which both entries set from */
/* the witness (authenticate_parent_state -> wit_n_headers()). There is no  */
/* longer a native/witness base split.                                      */
/* ======================================================================== */

/* EIP-2935/BLOCKHASH: the hash of the ancestor `offset` blocks back (1..256)
 * is keccak of the header at that depth. Headers run oldest..newest with the
 * parent (offset 1) LAST, so the ancestor at `offset` is header[k_n_headers -
 * offset]. k_n_headers is the authenticated witness header count
 * (authenticate_parent_state -> wit_n_headers()): the guest carries its real
 * ancestor chain; the native runner carries a single dummy parent (count 1), so
 * only BLOCKHASH of the immediate parent resolves and anything deeper is idx < 0.
 * A requested header beyond the witness is a deficient proof -> throw
 * InvalidBlock(WitnessDeficient), caught once at the top (run_one). */
function k_blockhash(n : word) -> hash = {
    let req = unsigned(n);
    let offset = k_header.number - req;
    if offset < 1 | offset > 256 then {
        word_of_int(0)
    } else {
        let idx = k_n_headers - offset;
        if idx < 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            witness_keccak(ssz_witness_header_slice(idx))
        }
    }
}

/* Stateless storage resolver (declared in host/state.sail): the (a, s)
 * pre-state value via a Merkle walk of the account's storage trie from its
 * pre-state root. An empty storage_root short-circuits to 0 (no walk); a
 * committed non-empty root whose node is absent is a proof the witness omitted
 * (EIP-8025) -> fail closed (never fires for a complete alloc MPT); a diverging
 * or empty terminus is a proven-absent slot => 0. Base BELOW the writes-only
 * overlay (host/kernel.sail k_sload/k_sload_orig): no read-through caching. */
function stateless_storage(a : address, s : word) -> word = {
    let sroot = load_account(a).storage_root;
    if sroot == EMPTY_TRIE_ROOT then {
        sail_zeros(256)
    } else {
        let rootnode = node_db_lookup(sroot);
        if list_len(rootnode) == 0 then {
            throw InvalidBlock(WitnessDeficient)
        } else {
            let leaf = trie_walk(rootnode, hash_nibbles(keccak256_word(s)), 0, 256);
            if list_len(leaf) > 0 then {
                rlp_str_word(leaf)
            } else {
                sail_zeros(256)
            }
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch by walking the state trie from the parent-root anchor. Reaching
 * the leaf => present (decode + seed); a diverging/empty terminus => proven-empty
 * account; a blinded node => missing proof => fail closed. The result is bound as
 * a block-level read member of the account working set (acct_wset_seed_read),
 * so subsequent load_account(a) is an O(1) working-set read. */
function stateless_account_load(a : address) -> unit = {
    let ak = keccak256_address(a);
    let rootnode = node_db_lookup(k_parent_state_root);
    if (list_len(rootnode) == 0) & (k_parent_state_root != EMPTY_TRIE_ROOT) then {
        throw InvalidBlock(WitnessDeficient)
    };
    let value = trie_walk(rootnode, hash_nibbles(ak), 0, 256);
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        acct_wset_seed_read(a, get_slice_int(64, nonce, 0), bal, sroot, chash);
        acct_wset_mark_base_exists(a)
    } else {
        acct_wset_seed_read(a, sail_zeros(64), sail_zeros(256), EMPTY_TRIE_ROOT, KECCAK_EMPTY)
    }
}

/* A not-yet-materialized account is bound from the authenticated base on first
 * access (un-journaled, so a frame revert keeps the revealed pre-state and only
 * undoes recorded mutations). */
function load_account(a : address) -> Account = {
    if not_bool(account_loaded(a)) then {
        stateless_account_load(a)
    };
    account_lookup(a)
}

function storage_updates(ak : bits(256)) -> list(TriePatch) = storage_updates_writes_only(ak)

function state_updates_map() -> list(TriePatch) = state_updates_map_updates_only()
