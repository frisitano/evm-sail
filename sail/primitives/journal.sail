/* ======================================================================== */
/* PRIMITIVES: JOURNAL TYPES                                                */
/*                                                                          */
/* The journal (undo-log) entry union the kernel pushes and the revert path */
/* decodes. Pure data -- no registers, no externs.                          */
/* ======================================================================== */

type JournalCheckpoint = protocol_quantity
type AccountCheckpoint = protocol_quantity
type StorageCheckpoint = protocol_quantity
type LogCheckpoint = protocol_quantity

struct StateCheckpoint = {
  journal  : JournalCheckpoint,
  accounts : AccountCheckpoint,
  storage  : StorageCheckpoint,
  logs     : LogCheckpoint,
}

union JEntry = {
  JTran  : (address, word, word), /* restore transient slot           */
  JWarmA : address,               /* un-warm a newly-accessed address */
  JWarmS : (address, word)        /* un-warm a newly-accessed slot    */
}
