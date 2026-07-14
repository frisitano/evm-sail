/* ======================================================================== */
/* PRIMITIVES: JOURNAL TYPES                                                */
/*                                                                          */
/* The journal (undo-log) entry union the kernel pushes and the revert path */
/* decodes. Pure data -- no registers, no externs.                          */
/* ======================================================================== */

type JournalCheckpoint = u64
type AccountCheckpoint = u64
type StorageCheckpoint = u64
type LogCheckpoint = u64

struct StateCheckpoint = {
  journal  : JournalCheckpoint,
  accounts : AccountCheckpoint,
  storage  : StorageCheckpoint,
  logs     : LogCheckpoint,
}

union JEntry = {
  JTran   : (address, word, word),    /* restore transient slot                  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JRefund : i64                       /* restore the refund counter              */
}
