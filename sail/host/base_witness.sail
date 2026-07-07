/* ======================================================================== */
/* WITNESS BASE BACKEND                                                     */
/*                                                                          */
/* Pre-state base-layer refinements for stateless entries (the zkVM guest   */
/* and the witness probe). The authenticated SSZ witness IS the base layer: */
/* nothing is pre-seeded; accounts and slots materialize into the working   */
/* set on first touch, absence is a witness-PROVEN exclusion (fail-closed   */
/* on missing proof material), ancestor hashes come from witness headers    */
/* (EIP-2935), and post-state roots overlay only the execution write set on */
/* the authenticated parent root.                                           */
/* ======================================================================== */

/* Stateless storage resolver (declared in host/state.sail): the (a, s) pre-state
 * value, resolved (and its presence/absence PROVEN) by a lazy Merkle walk of the
 * account's storage trie from its authenticated pre-state root. The node-db is
 * keyed by keccak256(node), so the walk is itself a hash-chain proof from that
 * root: reaching the leaf => present; a diverging/empty terminus through present
 * nodes => PROVEN ABSENT => 0; a blinded (witness-absent) node => missing proof
 * => fail closed (k_witness_bad). This is the base BELOW the writes-only overlay
 * (host/kernel.sail k_sload/k_orig): no read-through caching, so a repeated read
 * simply re-walks the witness. */
function stateless_storage(a : address, s : word) -> word = {
    let sroot = load_account(a).storage_root;
    if storage_proof_omitted(sroot) then {
        /* the block reads storage whose proof the witness omitted */
        k_witness_bad = true;
        sail_zeros(256)
    } else {
        let leaf = trie_walk(node_db_lookup(sroot), hash_nibbles(keccak256_word(s)), 0, 256);
        if list_len(leaf) > 0 then {
            rlp_str_word(leaf)
        } else {
            sail_zeros(256)
        }
    }
}

/* Stateless account materialization (declared in host/state.sail): bind `a` on
 * first touch. Presence/absence is PROVEN by a lazy Merkle walk of the state
 * trie from the authenticated parent root (k_parent_state_root): reaching the
 * leaf => present, decode + seed; a diverging/empty terminus through present
 * nodes => PROVEN ABSENT => empty account; a blinded node => missing proof =>
 * fail closed (k_witness_bad). The result is cached raw in the acctmap
 * (k_seed_account_hashed), so subsequent load_account(a) is an O(1) raw read. */
function stateless_account_load(a : address) -> unit = {
    let ak = keccak256_address(a);
    let rootnode = node_db_lookup(k_parent_state_root);
    if (list_len(rootnode) == 0) & (k_parent_state_root != EMPTY_TRIE_ROOT) then {
        /* the anchor root itself is missing from the witness */
        k_witness_bad = true
    };
    let value = trie_walk(rootnode, hash_nibbles(ak), 0, 256);
    if list_len(value) > 0 then {
        let (nonce, bal, sroot, chash) = decode_account(value);
        k_seed_account_hashed(a, nonce, bal, sroot, chash);
        acctmap_mark_base_exists(a)
    } else {
        k_seed_account_hashed(a, 0, sail_zeros(256), EMPTY_TRIE_ROOT, KECCAK_EMPTY)
    }
}

/* EIP-2935/BLOCKHASH: the hash of the ancestor `offset` blocks back (1..256)
 * is keccak of the witness header at that depth. The witness headers run
 * oldest..newest with the parent (offset 1) LAST, so the ancestor at `offset`
 * is header[n_headers - offset]; if that index is absent the witness is
 * deficient for this access -> invalid block. */
function k_blockhash(n : word) -> hash = {
    let req = unsigned(n);
    let offset = pl_block_number() - req;
    if offset < 1 | offset > 256 then {
        word_of_int(0)
    } else {
        let idx = wit_n_headers() - offset;
        if idx < 0 then {
            k_witness_bad = true;
            word_of_int(0)
        } else {
            witness_keccak(ssz_witness_header_slice(idx))
        }
    }
}

/* A not-yet-materialized account is bound from the authenticated witness on
 * first access (un-journaled, so a frame revert keeps the revealed pre-state
 * and only undoes recorded mutations). */
function load_account(a : address) -> Account = {
    if not_bool(account_loaded(a)) then {
        stateless_account_load(a)
    };
    account_lookup(a)
}

function storage_updates(ak : bits(256)) -> list(TriePatch) = storage_updates_writes_only(ak)

function state_updates_map() -> list(TriePatch) = state_updates_map_updates_only()

/* the state trie anchors at the authenticated parent root; trie_root fails
 * closed on any witness node missing along a touched path */
function state_root_anchor() -> bits(256) = k_parent_state_root
