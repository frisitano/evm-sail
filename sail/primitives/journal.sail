
/*md
# Semantic state checkpoint

An opaque handle denoting the complete revertible transaction state at a
call-frame boundary. The model may store and return the handle, but cannot
inspect subsystem cursors or replay an implementation journal.
*/
/*! An opaque host-issued handle for a semantic transaction-state snapshot.
    Its representation and rollback mechanism are deliberately unobservable. */
newtype StateCheckpoint = StateCheckpoint : protocol_quantity
