From Stdlib Require Import Bool Lia ZArith.

Require Import evm_types.

Open Scope Z_scope.

(** A successfully decoded item carries exactly the bound required by the
    caller-owned cursor transition. *)
Lemma rlp_decoded_item_allows_advance
    (source_off source_len full_len content_len : Z) :
  rlp_decoded_item_valid source_off source_len full_len content_len = true ->
  rlp_cursor_advance_valid source_len full_len = true.
Proof.
  unfold rlp_decoded_item_valid, rlp_cursor_advance_valid.
  intros decoded.
  apply andb_true_iff in decoded as [_ bounds].
  exact bounds.
Qed.

(** Advancing a valid cursor cannot produce a negative remaining length. *)
Lemma rlp_advance_remaining_nonnegative (source_len consumed : Z) :
  rlp_cursor_advance_valid source_len consumed = true ->
  0 <= source_len - consumed.
Proof.
  unfold rlp_cursor_advance_valid.
  rewrite andb_true_iff.
  intros [_ within].
  apply Z.leb_le in within.
  lia.
Qed.

(** Every successful item decode makes strict progress. *)
Lemma rlp_advance_strictly_decreases (source_len consumed : Z) :
  rlp_cursor_advance_valid source_len consumed = true ->
  source_len - consumed < source_len.
Proof.
  unfold rlp_cursor_advance_valid.
  rewrite andb_true_iff.
  intros [positive _].
  apply Z.ltb_lt in positive.
  lia.
Qed.

(** The absolute end of a slice is invariant under cursor advancement. *)
Lemma rlp_advance_preserves_end
    (source_off source_len consumed : Z) :
  (source_off + consumed) + (source_len - consumed) =
    source_off + source_len.
Proof.
  lia.
Qed.

(** Two valid advances may be combined into one valid advance. *)
Lemma rlp_advance_composes (source_len first second : Z) :
  rlp_cursor_advance_valid source_len first = true ->
  rlp_cursor_advance_valid (source_len - first) second = true ->
  rlp_cursor_advance_valid source_len (first + second) = true.
Proof.
  unfold rlp_cursor_advance_valid.
  repeat rewrite andb_true_iff.
  intros [first_positive first_within] [second_positive second_within].
  apply Z.ltb_lt in first_positive.
  apply Z.leb_le in first_within.
  apply Z.ltb_lt in second_positive.
  apply Z.leb_le in second_within.
  split.
  - apply Z.ltb_lt. lia.
  - apply Z.leb_le. lia.
Qed.
