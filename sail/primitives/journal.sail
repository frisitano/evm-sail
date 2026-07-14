/*md
# Journal types

The journal (undo-log) entry union the kernel pushes and the revert path
decodes. Pure data — no registers, no externs.
*/

type JournalCheckpoint = u64
type AccountCheckpoint = u64
type StorageCheckpoint = u64
type LogCheckpoint = u64

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
  JTran   : (address, word, word),    /* restore transient slot                  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JRefund : i64                       /* restore the refund counter              */
}
