/* ======================================================================== */
/* NATIVE BASE BACKEND                                                      */
/*                                                                          */
/* Pre-state base-layer refinements for fixture-driven entries (the EEST    */
/* runner and bench). The driver seeds the COMPLETE pre-state into the      */
/* account/storage maps before execution, so the base layer is the seeded   */
/* material and a key absent from the maps is absent from the pre-state     */
/* (reads as 0 / EMPTY_ACCOUNT). Ancestor block hashes are unmodeled and    */
/* post-state roots are built from the full cache/update union because      */
/* there is no authenticated parent root anchor.                            */
/*                                                                          */
/* The witness-backed refinements live in host/base_witness.sail; the       */
/* shared spec depends only on the abstract base-layer contracts.           */
/* ======================================================================== */

function k_sload(a : address, s : word) -> word = storage_load(a, s)

function k_orig(a : address, s : word) -> word = storage_base_load(a, s)

/* BLOCKHASH unmodeled natively (state-test fixtures carry no ancestors) */
function k_blockhash(_ : word) -> hash = word_of_int(0)

function load_account(a : address) -> Account = account_lookup(a)

function storage_updates(ak : bits(256)) -> list(TrieUpdate) = storage_updates_union(ak)

function state_updates_map() -> list(TrieUpdate) = state_updates_map_union()

function storage_root_deletes(_ : bits(256), sroot : bits(256), updates : list(TrieUpdate)) -> bits(256) =
    trie_overlay_root(sroot, updates)
