/*md
# State journal

The frame-revertible state journal is a closed operation algebra. Every
mutation records exactly the semantic field needed to restore its predecessor;
checkpoint and commit markers encode the nesting structure in the same ordered
stream. Backends may refine addresses and storage keys to dense identifiers,
but may not introduce rollback behavior outside these variants.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules.
*/

/*! A prior transient-storage value restored by frame rollback. */
struct JournalTransientChange = {
    address : address,
    slot : word,
    prior : word,
}

/*! The prior warm epoch of an account. */
struct JournalWarmAccountChange = {
    address : address,
    prior_epoch : block_access_index,
}

/*! The prior warm epoch of a storage location. */
struct JournalWarmStorageChange = {
    key : StorageKey,
    prior_epoch : block_access_index,
}

/*! The prior transaction-visible account balance. */
struct JournalAccountBalanceChange = {
    address : address,
    prior : word,
}

/*! The prior transaction-visible account nonce. */
struct JournalAccountNonceChange = {
    address : address,
    prior : account_nonce,
}

/*! The prior transaction-visible account code hash. */
struct JournalAccountCodeHashChange = {
    address : address,
    prior : hash,
}

/*! The prior transaction-visible account-existence flag. */
struct JournalAccountExistsChange = {
    address : address,
    prior : bool,
}

/*! The prior same-transaction account-creation flag. */
struct JournalAccountCreatedChange = {
    address : address,
    prior : bool,
}

/*! The prior transaction-visible selfdestruction flag. */
struct JournalAccountSelfdestructedChange = {
    address : address,
    prior : bool,
}

/*! The prior transaction-visible storage value. */
struct JournalStorageValueChange = {
    key : StorageKey,
    prior : word,
}

/*! The prior incarnation attached to one transaction-visible storage row. */
struct JournalStorageRowGenerationChange = {
    key : StorageKey,
    prior : storage_generation,
}

/*! The prior account-wide storage incarnation. */
struct JournalAccountStorageGenerationChange = {
    address : address,
    prior : storage_generation,
}

/*! One append-only state-journal record. */
union StateJournalEntry = {
    /*! Restores the prior transient-storage value on rollback. */
    JournalTransientChanged : JournalTransientChange,
    /*! Restores an account's prior warm epoch on rollback. */
    JournalWarmAccountChanged : JournalWarmAccountChange,
    /*! Restores a storage location's prior warm epoch on rollback. */
    JournalWarmStorageChanged : JournalWarmStorageChange,
    /*! Restores the prior account balance on rollback. */
    JournalAccountBalanceChanged : JournalAccountBalanceChange,
    /*! Restores the prior account nonce on rollback. */
    JournalAccountNonceChanged : JournalAccountNonceChange,
    /*! Restores the prior account code hash on rollback. */
    JournalAccountCodeHashChanged : JournalAccountCodeHashChange,
    /*! Restores the prior account-existence flag on rollback. */
    JournalAccountExistsChanged : JournalAccountExistsChange,
    /*! Restores the prior same-transaction account-creation flag on
        rollback. */
    JournalAccountCreatedChanged : JournalAccountCreatedChange,
    /*! Restores the prior selfdestruction flag on rollback. */
    JournalAccountSelfdestructedChanged : JournalAccountSelfdestructedChange,
    /*! Removes the most recently appended transaction account on rollback. */
    JournalTransactionAccountListed : unit,
    /*! Removes the most recently appended transaction storage identity for
        the given account on rollback. */
    JournalTransactionStorageListed : address,
    /*! Removes the most recently appended log and its topic/data ranges. */
    JournalLogAppended : unit,
    JournalStorageValueChanged : JournalStorageValueChange,
    JournalStorageRowGenerationChanged : JournalStorageRowGenerationChange,
    JournalAccountStorageGenerationChanged : JournalAccountStorageGenerationChange,
    JournalFrameCheckpointed : unit,
    JournalFrameCommitted : unit,
}

/*! Empties the transaction-local state journal. */
val state_journal_reset = impure { c: "state_journal_reset" } : unit -> unit
/*! Appends a `JournalFrameCheckpointed` entry. */
val state_journal_checkpoint = impure { c: "state_journal_checkpoint" } : unit -> unit
/*! Replays entries backwards to the innermost open checkpoint, then removes
    its marker. */
val state_journal_revert = impure { c: "state_journal_revert" } : unit -> unit
/*! Appends a `JournalFrameCommitted` entry for the innermost open checkpoint.
    Its mutations remain in the journal and are therefore revertible by a
    parent. */
val state_journal_commit = impure { c: "state_journal_commit" } : unit -> unit
