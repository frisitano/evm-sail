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

/* SLOAD base: the slot's value in the layered working set; on a miss the
 * authenticated pre-value is resolved from the witness and cached
 * read-through. */
function k_sload(a : address, s : word) -> word =
    if storage_present(a, s) then {
        storage_load(a, s)
    } else {
        let v = stateless_storage(a, s);
        storage_seed(a, s, v);
        v
    }

/* EIP-2200 "original" value: the slot at tx start = the BASE layer (committed
 * prior txs were pop_commit-merged in). A slot never committed to the base is
 * still at its witness pre-value, so resolve it from the witness. */
function k_orig(a : address, s : word) -> word =
    if storage_base_present(a, s) then {
        storage_base_load(a, s)
    } else {
        let v = stateless_storage(a, s);
        storage_seed(a, s, v);
        v
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
