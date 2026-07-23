/*md
# Validation debug (optional)

Default build variant discarding the optional validation debug
records.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! Clears the recorded failure at the start of a validation attempt. */
function validation_debug_reset() -> unit = ()

/*! Records the failing validation stage and reason for host
    inspection; a later record overwrites an earlier one. */
function validation_debug_record(_scope : bits(8), _reason : BlockError) -> unit = ()

function cached_account_trie_update_next() -> option(TrieUpdate) = None()

function debug_account_storage_root(_a : address) -> hash = EMPTY_TRIE_ROOT

function debug_rebuild_state_root() -> hash = EMPTY_TRIE_ROOT
