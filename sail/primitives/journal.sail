
/*md
# Journal types

The journal (undo-log) entry union the kernel pushes and the revert path
decodes. Pure data — no registers, no externs.
*/
/*! A length checkpoint of the undo journal. */
type JournalCheckpoint = protocol_quantity
/*! A length checkpoint of the account overlay. */
type AccountCheckpoint = protocol_quantity
/*! A length checkpoint of the storage overlay. */
type StorageCheckpoint = protocol_quantity
/*! A length checkpoint of the log store. */
type LogCheckpoint = protocol_quantity

/*! A four-counter snapshot of the mutable state stores; reverting a frame
    truncates each store back to its counter. */
struct StateCheckpoint = {
  journal  : JournalCheckpoint,
  accounts : AccountCheckpoint,
  storage  : StorageCheckpoint,
  logs     : LogCheckpoint,
}

/*! An undo-log entry. Transient-storage writes (EIP-1153), access-list
    warmings (EIP-2929), and refund-counter changes are journaled; a revert
    replays entries newest-first to restore the checkpointed state. */
union JEntry = {
  /* restore transient slot */
  JTran : (address, word, word),
  /* un-warm a newly-accessed address */
  JWarmA : address,
  /* un-warm a newly-accessed slot */
  JWarmS : (address, word)
}
