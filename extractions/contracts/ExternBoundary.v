From Coq Require Import Bool.Bool.
From Coq Require Import Lists.List.
From Coq Require Import ZArith.ZArith.

Import ListNotations.
Open Scope Z_scope.

Module EvmSailExternBoundary.

Definition byte := Z.
Definition word64 := Z.
Definition word160 := Z.
Definition word256 := Z.
Definition address := word160.
Definition account_hash := word256.
Definition storage_key := word256.
Definition byte_seq := list byte.

Inductive ByteSourceKind : Type :=
| StatelessInputSource
| EvmMemorySource
| CodeSource
| LogDataSource
| OutputSource
| ScratchSource.

Definition jumpdest_chunk := word256.

Definition byte_wf (b : byte) : Prop := 0 <= b < 256.
Definition bytes_wf (bs : byte_seq) : Prop := Forall byte_wf bs.
Definition uint64_wf (x : word64) : Prop := 0 <= x < 2 ^ 64.
Definition uint160_wf (x : word160) : Prop := 0 <= x < 2 ^ 160.
Definition uint256_wf (x : word256) : Prop := 0 <= x < 2 ^ 256.

Definition nth_byte (bs : byte_seq) (i : Z) : byte :=
  if i <? 0 then 0
  else
    match nth_error bs (Z.to_nat i) with
    | Some b => b
    | None => 0
    end.

Definition byte_window (bs : byte_seq) (off width : Z) : byte_seq :=
  map (fun k => nth_byte bs (off + Z.of_nat k)) (seq 0 (Z.to_nat width)).

Fixpoint bytes_le_value (bs : byte_seq) : Z :=
  match bs with
  | [] => 0
  | b :: rest => b + 256 * bytes_le_value rest
  end.

Fixpoint bytes_be_value (bs : byte_seq) : Z :=
  match bs with
  | [] => 0
  | b :: rest => b * 256 ^ Z.of_nat (length rest) + bytes_be_value rest
  end.

Record Account := {
  account_nonce : Z;
  account_balance : word256;
  account_storage_root : word256;
  account_code_hash : word256;
  account_present : bool;
  account_storage_cleared : bool;
  account_created : bool;
  account_selfdestructed : bool;
}.

Definition account_wf (a : Account) : Prop :=
  uint64_wf a.(account_nonce) /\
  uint256_wf a.(account_balance) /\
  uint256_wf a.(account_storage_root) /\
  uint256_wf a.(account_code_hash).

(* sail/host/byte_slice.sail: stateless_input returns the one source-backed
   slice supplied by the platform ABI. All decoding then uses the generic
   ByteSlice contract below; there is no separate per-byte input oracle. *)
Record InputOracle := {
  input_bytes : byte_seq;
  input_bytes_wf : bytes_wf input_bytes;
  stateless_input_value : byte_seq;
  stateless_input_spec : stateless_input_value = input_bytes;
}.

(* sail/main.sail and sail/host/output.sail: the guest publishes exactly one
   complete source-backed slice. Repeated writes are rejected by the host. *)
Record OutputTraceContract := {
  output_trace : Type;
  output_bytes : output_trace -> byte_seq;
  output_write_count : output_trace -> Z;
  public_output_write_step :
    output_trace -> byte_seq -> option output_trace;

  public_output_write_success :
    forall trace bytes next,
      bytes_wf bytes ->
      public_output_write_step trace bytes = Some next ->
      output_write_count trace = 0 /\
      output_bytes next = bytes /\
      output_write_count next = 1;
  public_output_write_rejects_repeat :
    forall trace bytes,
      output_write_count trace <> 0 ->
      public_output_write_step trace bytes = None;
}.

(* sail/primitives/crypto.sail and sail/host/byte_slice.sail: segmented hash axioms,
   abstract crypto operations, and source-backed precompile execution. *)
Record CryptoContract := {
  keccak256_ref : byte_seq -> word256;
  sha256_ref : byte_seq -> word256;
  precompile_ref : Z -> byte_seq -> option byte_seq;

  keccak256_ref_wf :
    forall msg, bytes_wf msg -> uint256_wf (keccak256_ref msg);
  sha256_ref_wf :
    forall msg, bytes_wf msg -> uint256_wf (sha256_ref msg);
  precompile_ref_wf :
    forall id input output,
      bytes_wf input ->
      precompile_ref id input = Some output ->
      bytes_wf output;

  staged_precompile_protocol_refines_ref : Prop;
}.

(* sail/primitives/bytes.sail and sail/host/byte_slice.sail. Calldata and code
   share this source/slice contract. Code analysis and storage are isolated in
   sail/{primitives,host}/code.sail. Native pointer/length resolution must
   refine the same materialized byte sequence for every source kind. *)
Record ByteSourceContract := {
  byte_source_state : Type;
  byte_source_bytes_ref : byte_source_state -> ByteSourceKind -> Z -> Z -> byte_seq;

  byte_source_bytes_wf :
    forall st kind off len,
      0 <= off ->
      0 <= len ->
      bytes_wf (byte_source_bytes_ref st kind off len);

  byte_source_resolve_contract : Prop;
  slice_byte_contract : Prop;
  slice_load_word_contract : Prop;
  slice_load_n_word_contract : Prop;
  slice_copy_contract : Prop;
  segmented_equality_refinement_contract : Prop;
  direct_slice_equality_refinement_contract : Prop;
  memory_slice_stability_contract : Prop;
  scratch_append_refinement_contract : Prop;
  scratch_truncate_refinement_contract : Prop;
  tx_input_view_contract : Prop;
  segmented_hash_refinement_contract : Prop;
  source_precompile_refinement_contract : Prop;
}.

(* sail/host/{memory,output,stack,frame_stack}.sail and
   sail/evm/machine.sail: mem_*, output_buffer_*, stack_*, frame_stack_*,
   generic ByteSlice views, the active Code value, and Sail-owned EVM
   returndata state. *)
Record MemoryStackContract := {
  memory_state : Type;
  stack_state : Type;
  byte_slice_state : Type;
  indexed_code_state : Type;
  output_buffer_state : Type;

  host_mem_read_ref : memory_state -> word64 -> byte;
  host_mem_write_ref : memory_state -> word64 -> byte -> memory_state;
  host_mem_read_write_same :
    forall mem addr b,
      byte_wf b ->
      host_mem_read_ref (host_mem_write_ref mem addr b) addr = b;
  host_mem_read_write_other :
    forall mem addr addr' b,
      addr <> addr' ->
      host_mem_read_ref (host_mem_write_ref mem addr b) addr' =
        host_mem_read_ref mem addr';

  memory_frame_lifo_contract : Prop;
  memory_move_contract : Prop;
  codecopy_to_memory_contract : Prop;
  generic_slice_view_contract : Prop;
  output_buffer_contract : Prop;
  evm_stack_lifo_contract : Prop;
  continuation_stack_lifo_contract : Prop;
  indexed_code_frame_contract : Prop;
}.

(* ------------------------------------------------------------------------- *)
(* Extensional world-state reference model                                   *)
(* ------------------------------------------------------------------------- *)

(* This is deliberately not the layout used by ffi/state_db.c or
   ffi/kernel_state.c.  In particular, there are no cache-presence bits,
   copy-on-write rows, clear generations, undo cursors, physical deletions, or
   first-write rules.  A backend may use any of those devices, but it refines
   the total maps and snapshots below. *)
Record PersistentWorld := {
  world_account_at : address -> Account;
  world_storage_at : address -> storage_key -> word256;
}.

Definition account_with_cleared_storage (account : Account) : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := true;
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_without_transaction_flags (account : Account) : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := false;
     account_selfdestructed := false |}.

Definition deleted_account
    (empty_code_hash : word256) (account : Account) : Account :=
  {| account_nonce := 0;
     account_balance := 0;
     (* Keep the authenticated anchor.  A cleared-storage bit selects the
        empty trie when the post-state root is assembled. *)
     account_storage_root := account.(account_storage_root);
     account_code_hash := empty_code_hash;
     account_present := false;
     account_storage_cleared := true;
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition world_write_account
    (world : PersistentWorld) (target : address) (value : Account)
    : PersistentWorld :=
  {| world_account_at :=
       fun candidate =>
         if Z.eqb candidate target then value
         else world.(world_account_at) candidate;
     world_storage_at := world.(world_storage_at) |}.

Definition world_write_storage
    (world : PersistentWorld) (target : address) (target_slot : storage_key)
    (value : word256) : PersistentWorld :=
  {| world_account_at := world.(world_account_at);
     world_storage_at :=
       fun candidate candidate_slot =>
         if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
         then value
         else world.(world_storage_at) candidate candidate_slot |}.

Definition world_clear_storage
    (world : PersistentWorld) (target : address) : PersistentWorld :=
  {| world_account_at :=
       fun candidate =>
         if Z.eqb candidate target
         then account_with_cleared_storage
                (world.(world_account_at) candidate)
         else world.(world_account_at) candidate;
     world_storage_at :=
       fun candidate candidate_slot =>
         if Z.eqb candidate target then 0
         else world.(world_storage_at) candidate candidate_slot |}.

Definition world_delete_account
    (empty_code_hash : word256) (world : PersistentWorld) (target : address)
    : PersistentWorld :=
  {| world_account_at :=
       fun candidate =>
         if Z.eqb candidate target
         then deleted_account empty_code_hash
                (world.(world_account_at) candidate)
         else world.(world_account_at) candidate;
     world_storage_at :=
       fun candidate candidate_slot =>
         if Z.eqb candidate target then 0
         else world.(world_storage_at) candidate candidate_slot |}.

Definition transient_state := address -> storage_key -> word256.
Definition warm_address_state := address -> bool.
Definition warm_slot_state := address -> storage_key -> bool.

Record LogicalLog := {
  logical_log_address : address;
  logical_log_topics : list word256;
  logical_log_data : byte_seq;
}.

Definition empty_transient_state : transient_state := fun _ _ => 0.
Definition empty_warm_address_state : warm_address_state := fun _ => false.
Definition empty_warm_slot_state : warm_slot_state := fun _ _ => false.

Definition transient_write
    (transient : transient_state) (target : address)
    (target_slot : storage_key) (value : word256) : transient_state :=
  fun candidate candidate_slot =>
    if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
    then value
    else transient candidate candidate_slot.

Definition warm_address_write
    (warm : warm_address_state) (target : address) : warm_address_state :=
  fun candidate => if Z.eqb candidate target then true else warm candidate.

Definition warm_slot_write
    (warm : warm_slot_state) (target : address)
    (target_slot : storage_key) : warm_slot_state :=
  fun candidate candidate_slot =>
    if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
    then true
    else warm candidate candidate_slot.

(* block_start is the authenticated parent state.  block_current accumulates
   successful transactions.  tx_start is the EIP-2200 original-value snapshot,
   while tx_current is the live state of the executing transaction. *)
Record ReferenceWorldState := {
  reference_block_start : PersistentWorld;
  reference_block_current : PersistentWorld;
  reference_tx_start : PersistentWorld;
  reference_tx_current : PersistentWorld;
  reference_transient : transient_state;
  reference_warm_addresses : warm_address_state;
  reference_warm_slots : warm_slot_state;
  reference_logs : list LogicalLog;
}.

Record TransactionSnapshot := {
  snapshot_tx_current : PersistentWorld;
  snapshot_transient : transient_state;
  snapshot_warm_addresses : warm_address_state;
  snapshot_warm_slots : warm_slot_state;
  snapshot_logs : list LogicalLog;
}.

Definition persistent_world_equiv
    (left right : PersistentWorld) : Prop :=
  (forall account,
      left.(world_account_at) account = right.(world_account_at) account) /\
  (forall account slot,
      left.(world_storage_at) account slot =
      right.(world_storage_at) account slot).

Definition transient_state_equiv
    (left right : transient_state) : Prop :=
  forall account slot, left account slot = right account slot.

Definition warm_address_state_equiv
    (left right : warm_address_state) : Prop :=
  forall account, left account = right account.

Definition warm_slot_state_equiv
    (left right : warm_slot_state) : Prop :=
  forall account slot, left account slot = right account slot.

Definition reference_world_state_equiv
    (left right : ReferenceWorldState) : Prop :=
  persistent_world_equiv left.(reference_block_start)
                         right.(reference_block_start) /\
  persistent_world_equiv left.(reference_block_current)
                         right.(reference_block_current) /\
  persistent_world_equiv left.(reference_tx_start)
                         right.(reference_tx_start) /\
  persistent_world_equiv left.(reference_tx_current)
                         right.(reference_tx_current) /\
  transient_state_equiv left.(reference_transient)
                        right.(reference_transient) /\
  warm_address_state_equiv left.(reference_warm_addresses)
                           right.(reference_warm_addresses) /\
  warm_slot_state_equiv left.(reference_warm_slots)
                        right.(reference_warm_slots) /\
  left.(reference_logs) = right.(reference_logs).

Definition transaction_snapshot_equiv
    (left right : TransactionSnapshot) : Prop :=
  persistent_world_equiv left.(snapshot_tx_current)
                         right.(snapshot_tx_current) /\
  transient_state_equiv left.(snapshot_transient)
                        right.(snapshot_transient) /\
  warm_address_state_equiv left.(snapshot_warm_addresses)
                           right.(snapshot_warm_addresses) /\
  warm_slot_state_equiv left.(snapshot_warm_slots)
                        right.(snapshot_warm_slots) /\
  left.(snapshot_logs) = right.(snapshot_logs).

Definition reference_start_block
    (initial : PersistentWorld) : ReferenceWorldState :=
  {| reference_block_start := initial;
     reference_block_current := initial;
     reference_tx_start := initial;
     reference_tx_current := initial;
     reference_transient := empty_transient_state;
     reference_warm_addresses := empty_warm_address_state;
     reference_warm_slots := empty_warm_slot_state;
     reference_logs := [] |}.

Definition reference_begin_transaction
    (state : ReferenceWorldState) : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_block_current);
     reference_tx_current := state.(reference_block_current);
     reference_transient := empty_transient_state;
     reference_warm_addresses := empty_warm_address_state;
     reference_warm_slots := empty_warm_slot_state;
     reference_logs := [] |}.

Definition reference_read_account
    (state : ReferenceWorldState) (account : address) : Account :=
  state.(reference_tx_current).(world_account_at) account.

Definition reference_read_storage_current
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  state.(reference_tx_current).(world_storage_at) account slot.

(* The SSTORE original value is defined by transaction entry, not by the first
   insertion into an implementation table. *)
Definition reference_read_storage_original
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  state.(reference_tx_start).(world_storage_at) account slot.

Definition reference_write_account
    (state : ReferenceWorldState) (account : address) (value : Account)
    : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current :=
       world_write_account state.(reference_tx_current) account value;
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

Definition reference_write_storage
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    (value : word256) : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current :=
       world_write_storage state.(reference_tx_current) account slot value;
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

Definition reference_clear_storage
    (state : ReferenceWorldState) (account : address)
    : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current :=
       world_clear_storage state.(reference_tx_current) account;
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

Definition reference_delete_account
    (empty_code_hash : word256) (state : ReferenceWorldState)
    (account : address) : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current :=
       world_delete_account empty_code_hash state.(reference_tx_current)
                            account;
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

Definition reference_read_transient
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  state.(reference_transient) account slot.

Definition reference_write_transient
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    (value : word256) : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current := state.(reference_tx_current);
     reference_transient :=
       transient_write state.(reference_transient) account slot value;
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

Definition reference_touch_warm_address
    (state : ReferenceWorldState) (account : address)
    : bool * ReferenceWorldState :=
  (state.(reference_warm_addresses) account,
   {| reference_block_start := state.(reference_block_start);
      reference_block_current := state.(reference_block_current);
      reference_tx_start := state.(reference_tx_start);
      reference_tx_current := state.(reference_tx_current);
      reference_transient := state.(reference_transient);
      reference_warm_addresses :=
        warm_address_write state.(reference_warm_addresses) account;
      reference_warm_slots := state.(reference_warm_slots);
      reference_logs := state.(reference_logs) |}).

Definition reference_touch_warm_slot
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : bool * ReferenceWorldState :=
  (state.(reference_warm_slots) account slot,
   {| reference_block_start := state.(reference_block_start);
      reference_block_current := state.(reference_block_current);
      reference_tx_start := state.(reference_tx_start);
      reference_tx_current := state.(reference_tx_current);
      reference_transient := state.(reference_transient);
      reference_warm_addresses := state.(reference_warm_addresses);
      reference_warm_slots :=
        warm_slot_write state.(reference_warm_slots) account slot;
      reference_logs := state.(reference_logs) |}).

Definition reference_append_log
    (state : ReferenceWorldState) (entry : LogicalLog) : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current := state.(reference_tx_current);
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) ++ [entry] |}.

Definition reference_checkpoint
    (state : ReferenceWorldState) : TransactionSnapshot :=
  {| snapshot_tx_current := state.(reference_tx_current);
     snapshot_transient := state.(reference_transient);
     snapshot_warm_addresses := state.(reference_warm_addresses);
     snapshot_warm_slots := state.(reference_warm_slots);
     snapshot_logs := state.(reference_logs) |}.

Definition reference_revert
    (state : ReferenceWorldState) (snapshot : TransactionSnapshot)
    : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current := snapshot.(snapshot_tx_current);
     reference_transient := snapshot.(snapshot_transient);
     reference_warm_addresses := snapshot.(snapshot_warm_addresses);
     reference_warm_slots := snapshot.(snapshot_warm_slots);
     reference_logs := snapshot.(snapshot_logs) |}.

Definition finalized_world
    (empty_code_hash : word256) (delete_at_end : address -> bool)
    (world : PersistentWorld) : PersistentWorld :=
  {| world_account_at :=
       fun account =>
         account_without_transaction_flags
           (if delete_at_end account
            then deleted_account empty_code_hash
                   (world.(world_account_at) account)
            else world.(world_account_at) account);
     world_storage_at :=
       fun account slot =>
         if delete_at_end account then 0
         else world.(world_storage_at) account slot |}.

(* delete_at_end is computed by the pure Sail fork policy before the host
   transition.  The host contract only applies that decision extensionally. *)
Definition reference_commit_transaction
    (empty_code_hash : word256) (delete_at_end : address -> bool)
    (state : ReferenceWorldState) : ReferenceWorldState :=
  let committed :=
      finalized_world empty_code_hash delete_at_end
                      state.(reference_tx_current) in
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := committed;
     reference_tx_start := committed;
     reference_tx_current := committed;
     reference_transient := state.(reference_transient);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_logs := state.(reference_logs) |}.

(* A finite delta is an extensional witness for every changed key.  It is not
   a materialization cache: membership is exactly logical inequality. *)
Record WorldDelta := {
  delta_accounts : list address;
  delta_storage : address -> list storage_key;
}.

Definition world_delta_describes
    (before after : PersistentWorld) (delta : WorldDelta) : Prop :=
  NoDup delta.(delta_accounts) /\
  (forall account,
      In account delta.(delta_accounts) <->
      before.(world_account_at) account <>
        after.(world_account_at) account \/
      exists slot,
        before.(world_storage_at) account slot <>
        after.(world_storage_at) account slot) /\
  (forall account,
      NoDup (delta.(delta_storage) account) /\
      (forall slot,
          In slot (delta.(delta_storage) account) <->
          before.(world_storage_at) account slot <>
          after.(world_storage_at) account slot)).

Fixpoint key_before_all {A : Type}
    (key : A -> Z) (first : A) (rest : list A) : Prop :=
  match rest with
  | [] => True
  | next :: tail =>
      key first < key next /\ key_before_all key first tail
  end.

Fixpoint strictly_sorted_by {A : Type}
    (key : A -> Z) (items : list A) : Prop :=
  match items with
  | [] => True
  | first :: rest =>
      key_before_all key first rest /\ strictly_sorted_by key rest
  end.

Definition fixed_be_byte
    (width index : nat) (value : Z) : byte :=
  (value / (256 ^ Z.of_nat (width - S index))) mod 256.

Definition fixed_be_bytes (width : nat) (value : Z) : byte_seq :=
  map (fun index => fixed_be_byte width index value) (seq 0 width).

Definition account_secure_key
    (crypto : CryptoContract) (account : address) : word256 :=
  crypto.(keccak256_ref) (fixed_be_bytes 20 account).

Definition storage_secure_key
    (crypto : CryptoContract) (slot : storage_key) : word256 :=
  crypto.(keccak256_ref) (fixed_be_bytes 32 slot).

Definition world_delta_securely_ordered
    (crypto : CryptoContract) (delta : WorldDelta) : Prop :=
  strictly_sorted_by (account_secure_key crypto) delta.(delta_accounts) /\
  forall account,
    strictly_sorted_by (storage_secure_key crypto)
                       (delta.(delta_storage) account).

(* sail/host/state.sail account/storage/transient/warm/log externs collectively refine
   these semantic operations:

     k_aload                         -> world_read_account_ref
     k_sload                         -> current/original storage reads
     acct_tx_*/storage_tx_* writes   -> world_*_write_step
     state_checkpoint/state_revert   -> opaque handle/snapshot/revert
     k_tx_merge + block rows         -> commit and finite deltas

   The split tx/block caches and physical checkpoint storage are therefore a
   backend protocol, not part of the abstract state. *)
Record WorldStateContract := {
  world_checkpoint_handle : Type;
  world_state : Type;

  world_observe : world_state -> ReferenceWorldState;
  world_start_block_step : PersistentWorld -> world_state;
  world_begin_transaction_step : world_state -> world_state;
  world_read_account_ref : world_state -> address -> Account;
  world_read_storage_current_ref :
    world_state -> address -> storage_key -> word256;
  world_read_storage_original_ref :
    world_state -> address -> storage_key -> word256;
  world_write_account_step :
    world_state -> address -> Account -> world_state;
  world_write_storage_step :
    world_state -> address -> storage_key -> word256 -> world_state;
  world_clear_storage_step : world_state -> address -> world_state;
  world_delete_account_step :
    word256 -> world_state -> address -> world_state;
  world_checkpoint_step :
    world_state -> (world_checkpoint_handle * world_state)%type;
  world_checkpoint_denotes :
    world_state -> world_checkpoint_handle -> TransactionSnapshot -> Prop;
  world_revert_step :
    world_state -> world_checkpoint_handle -> option world_state;
  world_commit_transaction_step :
    word256 -> (address -> bool) -> world_state -> world_state;

  (* Read-through materialization must not change the logical observation. *)
  world_cache_account_step :
    world_state -> address -> Account -> world_state;
  world_cache_storage_step :
    world_state -> address -> storage_key -> word256 -> world_state;

  world_read_transient_ref :
    world_state -> address -> storage_key -> word256;
  world_write_transient_step :
    world_state -> address -> storage_key -> word256 -> world_state;
  world_touch_warm_address_step :
    world_state -> address -> (bool * world_state)%type;
  world_touch_warm_slot_step :
    world_state -> address -> storage_key -> (bool * world_state)%type;
  world_append_log_step : world_state -> LogicalLog -> world_state;
  world_read_logs_ref : world_state -> list LogicalLog;

  world_transaction_delta_ref : world_state -> WorldDelta;
  world_block_delta_ref : world_state -> WorldDelta;

  code_lookup_ref : world_state -> word256 -> option byte_seq;
  code_lookup_wf :
    forall world code_hash code,
      code_lookup_ref world code_hash = Some code ->
      uint256_wf code_hash /\ bytes_wf code;

  jumpdest_allocation_contract : Prop;
  jumpdest_chunk_store_contract : Prop;
  jumpdest_ref_contract : Prop;
  indexed_code_lookup_contract : Prop;
  code_db_contract : Prop;
  code_delegation_contract : Prop;
}.

Definition world_checkpoint_denotations_preserved
    (contract : WorldStateContract)
    (before after : contract.(world_state)) : Prop :=
  forall checkpoint snapshot,
    contract.(world_checkpoint_denotes) before checkpoint snapshot ->
    contract.(world_checkpoint_denotes) after checkpoint snapshot.

(* Ordinary call-frame mutations preserve live checkpoint denotations.
   Transaction boundaries may invalidate every handle and therefore
   deliberately have no such obligation. *)

(* The canonical semantic instantiation uses the reference world directly and
   a TransactionSnapshot itself as its checkpoint handle.  A native backend
   may use any opaque handle related to that snapshot by
   world_checkpoint_denotes.  Delta enumerators and code-database obligations
   are explicit because they concern finite witnesses and a separate store. *)
Definition reference_world_state_contract
    (transaction_delta block_delta : ReferenceWorldState -> WorldDelta)
    (code_lookup : ReferenceWorldState -> word256 -> option byte_seq)
    (code_lookup_well_formed :
       forall world code_hash code,
         code_lookup world code_hash = Some code ->
         uint256_wf code_hash /\ bytes_wf code)
    (jumpdest_allocation jumpdest_chunk_store jumpdest_ref
       indexed_code_lookup code_db code_delegation : Prop)
    : WorldStateContract :=
  {| world_checkpoint_handle := TransactionSnapshot;
     world_state := ReferenceWorldState;
     world_observe := fun state => state;
     world_start_block_step := reference_start_block;
     world_begin_transaction_step := reference_begin_transaction;
     world_read_account_ref := reference_read_account;
     world_read_storage_current_ref := reference_read_storage_current;
     world_read_storage_original_ref := reference_read_storage_original;
     world_write_account_step := reference_write_account;
     world_write_storage_step := reference_write_storage;
     world_clear_storage_step := reference_clear_storage;
     world_delete_account_step := reference_delete_account;
     world_checkpoint_step :=
       fun state => (reference_checkpoint state, state);
     world_checkpoint_denotes :=
       fun _ handle snapshot => transaction_snapshot_equiv handle snapshot;
     world_revert_step :=
       fun state snapshot => Some (reference_revert state snapshot);
     world_commit_transaction_step := reference_commit_transaction;
     world_cache_account_step := fun state _ _ => state;
     world_cache_storage_step := fun state _ _ _ => state;
     world_read_transient_ref := reference_read_transient;
     world_write_transient_step := reference_write_transient;
     world_touch_warm_address_step := reference_touch_warm_address;
     world_touch_warm_slot_step := reference_touch_warm_slot;
     world_append_log_step := reference_append_log;
     world_read_logs_ref := reference_logs;
     world_transaction_delta_ref := transaction_delta;
     world_block_delta_ref := block_delta;
     code_lookup_ref := code_lookup;
     code_lookup_wf := code_lookup_well_formed;
     jumpdest_allocation_contract := jumpdest_allocation;
     jumpdest_chunk_store_contract := jumpdest_chunk_store;
     jumpdest_ref_contract := jumpdest_ref;
     indexed_code_lookup_contract := indexed_code_lookup;
     code_db_contract := code_db;
     code_delegation_contract := code_delegation |}.

Definition world_state_boundary
    (crypto : CryptoContract) (contract : WorldStateContract) : Prop :=
  (forall initial,
      let state := contract.(world_start_block_step) initial in
      reference_world_state_equiv
        (contract.(world_observe) state)
        (reference_start_block initial)) /\
  (forall state,
      let next := contract.(world_begin_transaction_step) state in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_begin_transaction (contract.(world_observe) state))) /\
  (forall state account,
      contract.(world_read_account_ref) state account =
      reference_read_account (contract.(world_observe) state) account) /\
  (forall state account slot,
      contract.(world_read_storage_current_ref) state account slot =
      reference_read_storage_current
        (contract.(world_observe) state) account slot) /\
  (forall state account slot,
      contract.(world_read_storage_original_ref) state account slot =
      reference_read_storage_original
        (contract.(world_observe) state) account slot) /\
  (forall state account value,
      let next := contract.(world_write_account_step) state account value in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_write_account
          (contract.(world_observe) state) account value) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state account slot value,
      let next :=
          contract.(world_write_storage_step) state account slot value in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_write_storage
          (contract.(world_observe) state) account slot value) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state account,
      let next := contract.(world_clear_storage_step) state account in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_clear_storage
          (contract.(world_observe) state) account) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall empty_code_hash state account,
      let next :=
          contract.(world_delete_account_step) empty_code_hash state account in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_delete_account
          empty_code_hash (contract.(world_observe) state) account) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state,
      let result := contract.(world_checkpoint_step) state in
      reference_world_state_equiv
        (contract.(world_observe) (snd result))
        (contract.(world_observe) state) /\
      contract.(world_checkpoint_denotes) (snd result) (fst result)
        (reference_checkpoint (contract.(world_observe) state)) /\
      world_checkpoint_denotations_preserved contract state (snd result)) /\
  (forall state checkpoint snapshot,
      contract.(world_checkpoint_denotes) state checkpoint snapshot ->
      exists next,
        contract.(world_revert_step) state checkpoint = Some next /\
        reference_world_state_equiv
          (contract.(world_observe) next)
          (reference_revert (contract.(world_observe) state) snapshot)) /\
  (forall empty_code_hash delete_at_end state,
      let next :=
          contract.(world_commit_transaction_step)
            empty_code_hash delete_at_end state in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_commit_transaction
          empty_code_hash delete_at_end
          (contract.(world_observe) state))) /\
  (forall state account,
      let next :=
          contract.(world_cache_account_step) state account
            (contract.(world_read_account_ref) state account) in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (contract.(world_observe) state) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state account slot,
      let next :=
          contract.(world_cache_storage_step) state account slot
            (contract.(world_read_storage_current_ref)
              state account slot) in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (contract.(world_observe) state) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state account slot,
      contract.(world_read_transient_ref) state account slot =
      reference_read_transient
        (contract.(world_observe) state) account slot) /\
  (forall state account slot value,
      let next :=
          contract.(world_write_transient_step) state account slot value in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_write_transient
          (contract.(world_observe) state) account slot value) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state account,
      let result := contract.(world_touch_warm_address_step) state account in
      let expected :=
          reference_touch_warm_address
            (contract.(world_observe) state) account in
      fst result = fst expected /\
      reference_world_state_equiv
        (contract.(world_observe) (snd result)) (snd expected) /\
      world_checkpoint_denotations_preserved contract state (snd result)) /\
  (forall state account slot,
      let result :=
          contract.(world_touch_warm_slot_step) state account slot in
      let expected :=
          reference_touch_warm_slot
            (contract.(world_observe) state) account slot in
      fst result = fst expected /\
      reference_world_state_equiv
        (contract.(world_observe) (snd result)) (snd expected) /\
      world_checkpoint_denotations_preserved contract state (snd result)) /\
  (forall state entry,
      let next := contract.(world_append_log_step) state entry in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_append_log (contract.(world_observe) state) entry) /\
      world_checkpoint_denotations_preserved contract state next) /\
  (forall state,
      contract.(world_read_logs_ref) state =
        (contract.(world_observe) state).(reference_logs)) /\
  (forall state,
      world_delta_describes
        (contract.(world_observe) state).(reference_tx_start)
        (contract.(world_observe) state).(reference_tx_current)
        (contract.(world_transaction_delta_ref) state)) /\
  (forall state,
      world_delta_describes
        (contract.(world_observe) state).(reference_block_start)
        (contract.(world_observe) state).(reference_block_current)
        (contract.(world_block_delta_ref) state)) /\
  (forall state,
      world_delta_securely_ordered
        crypto (contract.(world_block_delta_ref) state)).

Definition span := (Z * Z)%type.

(* sail/host/nodes.sail witness-facing externs: nodedb_*. The node table plus the
   Sail-side authenticated parent-root register (k_parent_state_root) is the
   whole lazy-witness model: every walk from that root is a hash-chain proof
   through keccak(node)-keyed lookups. *)
Record WitnessDbContract := {
  witness_state : Type;

  node_lookup_ref : witness_state -> word256 -> option span;
  witness_bytes_ref : witness_state -> span -> byte_seq;

  node_lookup_wf :
    forall witness node_hash span,
      node_lookup_ref witness node_hash = Some span ->
      uint256_wf node_hash /\ bytes_wf (witness_bytes_ref witness span);

  node_db_insert_select_contract : Prop;
  witness_span_bounds_contract : Prop;
  mpt_authenticated_frontier_contract : Prop;
}.

Record GuestExternContract := {
  guest_input : InputOracle;
  guest_output : OutputTraceContract;
  guest_crypto : CryptoContract;
  guest_byte_sources : ByteSourceContract;
  guest_memory_stack : MemoryStackContract;
  guest_world_state : WorldStateContract;
  guest_witness_db : WitnessDbContract;
}.

Definition input_well_formed_boundary (contract : GuestExternContract) : Prop :=
  bytes_wf (input_bytes (guest_input contract)) /\
  stateless_input_value (guest_input contract) =
    input_bytes (guest_input contract).

Definition main_boundary (contract : GuestExternContract) : Prop :=
  staged_precompile_protocol_refines_ref contract.(guest_crypto) /\
  byte_source_resolve_contract contract.(guest_byte_sources) /\
  slice_byte_contract contract.(guest_byte_sources) /\
  slice_load_word_contract contract.(guest_byte_sources) /\
  slice_load_n_word_contract contract.(guest_byte_sources) /\
  slice_copy_contract contract.(guest_byte_sources) /\
  segmented_equality_refinement_contract contract.(guest_byte_sources) /\
  memory_slice_stability_contract contract.(guest_byte_sources) /\
  tx_input_view_contract contract.(guest_byte_sources) /\
  segmented_hash_refinement_contract contract.(guest_byte_sources) /\
  source_precompile_refinement_contract contract.(guest_byte_sources) /\
  memory_frame_lifo_contract contract.(guest_memory_stack) /\
  generic_slice_view_contract contract.(guest_memory_stack) /\
  output_buffer_contract contract.(guest_memory_stack) /\
  evm_stack_lifo_contract contract.(guest_memory_stack) /\
  continuation_stack_lifo_contract contract.(guest_memory_stack) /\
  indexed_code_frame_contract contract.(guest_memory_stack) /\
  world_state_boundary contract.(guest_crypto)
                       contract.(guest_world_state) /\
  jumpdest_allocation_contract contract.(guest_world_state) /\
  jumpdest_chunk_store_contract contract.(guest_world_state) /\
  jumpdest_ref_contract contract.(guest_world_state) /\
  indexed_code_lookup_contract contract.(guest_world_state) /\
  code_db_contract contract.(guest_world_state) /\
  node_db_insert_select_contract contract.(guest_witness_db) /\
  mpt_authenticated_frontier_contract contract.(guest_witness_db).

End EvmSailExternBoundary.
