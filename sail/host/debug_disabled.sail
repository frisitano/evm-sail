function validation_debug_reset() -> unit = ()

function validation_debug_record(_scope : bits(8), _reason : BlockError) -> unit = ()

function debug_account_storage_root(_a : address) -> hash = EMPTY_TRIE_ROOT

function debug_rebuild_state_root() -> hash = EMPTY_TRIE_ROOT
