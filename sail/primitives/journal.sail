/* ======================================================================== */
/* PRIMITIVES: JOURNAL TYPES                                                */
/*                                                                          */
/* The journal (undo-log) entry union the kernel pushes and the revert path */
/* decodes. Pure data -- no registers, no externs.                          */
/* ======================================================================== */

/* Journal (undo-log) entry: one reversible state mutation. A call frame's
 * effects must roll back atomically on REVERT or an exceptional halt; rather
 * than snapshot the whole world per frame (O(state)), each mutation records a
 * small undo entry. k_snapshot pushes a JCheck checkpoint marker, k_revert
 * replays undos back to it, k_commit drops the marker so the frame's entries
 * fold into the parent (a later parent revert still undoes them). This is the
 * EELS journal model: O(changes), not O(state).
 *
 * JEntry is the kernel-facing type: the kernel journals a mutation as
 * `journal_push(JAcct(..))` and k_revert dispatches undos with `match e`. The
 * journal itself is abstract: journal_push / journal_pop (below) are
 * `c:`-bound axioms and whole JEntry values cross the boundary, so a JEntry
 * is only ever an ephemeral view -- built for a push, decoded for a revert --
 * never a Sail-side buffer. The C side (ffi/journal_glue.c over the
 * ffi/kernel_state.c scalar store) owns both the storage and the
 * (en/de)coding. */
union JEntry = {
  JCheck  : unit,                     /* call-frame checkpoint marker            */
  JAcct   : (address, Account),       /* restore an account to its prior value   */
  JTran   : (address, word, word),    /* restore transient slot                  */
  JStor   : (address, word, word),    /* restore a storage slot's current value  */
  JWarmA  : address,                  /* un-warm a newly-accessed address        */
  JWarmS  : (address, word),          /* un-warm a newly-accessed slot           */
  JLog    : unit,                     /* drop the most recent log                */
  JRefund : i64,                      /* restore the refund counter              */
  JSelfD  : unit                      /* drop the most recent selfdestruct       */
}
