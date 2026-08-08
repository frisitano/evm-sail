From Stdlib Require Import Bool.Bool.
From Stdlib Require Import Lists.List.
From Stdlib Require Import ZArith.ZArith.

Import ListNotations.
Open Scope Z_scope.

Module EvmSailExternBoundary.

Definition byte := Z.
Definition word64 := Z.
Definition word160 := Z.
Definition word256 := Z.
Definition address := word160.
Definition storage_key := word256.
Definition byte_seq := list byte.
(* block_access_index doubles as the EIP-2929 warm epoch and the EIP-7928
   position index; storage generations count account-wide storage clears. *)
Definition access_index := Z.
Definition generation := Z.

Definition byte_wf (b : byte) : Prop := 0 <= b < 256.
Definition bytes_wf (bs : byte_seq) : Prop := Forall byte_wf bs.
Definition uint64_wf (x : word64) : Prop := 0 <= x < 2 ^ 64.
Definition uint256_wf (x : word256) : Prop := 0 <= x < 2 ^ 256.

Definition nth_byte (bs : byte_seq) (i : Z) : byte :=
  if i <? 0 then 0
  else
    match nth_error bs (Z.to_nat i) with
    | Some b => b
    | None => 0
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

(* sail/host/region_access.sail: stateless_input returns the one source-backed
   slice supplied by the platform ABI.  Every derived read is a region extern
   over that nominal slice family; there is no per-byte input oracle. *)
Record InputOracle := {
  input_bytes : byte_seq;
  input_bytes_wf : bytes_wf input_bytes;
  stateless_input_value : byte_seq;
  stateless_input_spec : stateless_input_value = input_bytes;
}.

(* sail/main.sail and sail/host/output.sail: the guest publishes exactly one
   complete scratch-backed slice (public_output_write : ScratchSlice -> bool).
   The trace below denotes the published byte sequence extensionally. *)
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

(* sail/primitives/crypto.sail: the axiomatic hash core.  keccak256 and sha256
   are pure functions of a contiguous byte sequence; the per-region entry
   points (stateless_input/scratch/memory/code/output/log_data keccak256, the
   three sha256 sources, keccak256_word, keccak256_address, sha256_pair) must
   all refine these two references over the denoted region bytes. *)
Record CryptoContract := {
  keccak256_ref : byte_seq -> word256;
  sha256_ref : byte_seq -> word256;

  keccak256_ref_wf :
    forall msg, bytes_wf msg -> uint256_wf (keccak256_ref msg);
  sha256_ref_wf :
    forall msg, bytes_wf msg -> uint256_wf (sha256_ref msg);
}.

(* sail/host/accelerators.sail: the typed accelerator interface.  Each
   accelerator is a partial function of its materialized calldata bytes; the
   CalldataSlice union (input- or memory-backed) only selects the source
   region.  pairing results use the two-bit success/result encoding. *)
Record AcceleratorContract := {
  accelerator_ripemd160_ref : byte_seq -> option byte_seq;
  accelerator_modexp_ref : byte_seq -> option byte_seq;
  accelerator_bn254_add_ref : byte_seq -> option byte_seq;
  accelerator_bn254_mul_ref : byte_seq -> option byte_seq;
  accelerator_bn254_pairing_ref : byte_seq -> option Z;
  accelerator_blake2f_ref : byte_seq -> option byte_seq;
  accelerator_kzg_point_evaluation_ref : byte_seq -> option byte_seq;
  accelerator_bls_g1_add_ref : byte_seq -> option byte_seq;
  accelerator_bls_g1_msm_ref : byte_seq -> option byte_seq;
  accelerator_bls_g2_add_ref : byte_seq -> option byte_seq;
  accelerator_bls_g2_msm_ref : byte_seq -> option byte_seq;
  accelerator_bls_pairing_ref : byte_seq -> option Z;
  accelerator_bls_map_fp_to_g1_ref : byte_seq -> option byte_seq;
  accelerator_bls_map_fp2_to_g2_ref : byte_seq -> option byte_seq;
  accelerator_p256_verify_ref : byte_seq -> option byte_seq;
  (* host_ecrecover : (hash, y_parity, r, s) -> success * address *)
  host_ecrecover_ref :
    word256 -> Z -> word256 -> word256 -> (bool * address)%type;

  accelerator_output_wf :
    forall input output,
      bytes_wf input ->
      accelerator_modexp_ref input = Some output ->
      bytes_wf output;
  pairing_check_result_encoding_contract : Prop;
}.

(* sail/primitives/bytes.sail and sail/host/region_access.sail: six nominal
   region families.  The nominal Sail type of a slice selects the backing
   region; no runtime source tag crosses the boundary.  Every region operation
   (byte_at, load_word, load_n_word, copy_to_memory, strided_zero,
   count_nonzero, the three provenance-pair equalities, and the per-region
   hash entry points) must refine the one materialized byte sequence this
   contract denotes for its region. *)
Inductive Region : Type :=
| StatelessInputRegion
| EvmMemoryRegion
| CodeRegion
| ScratchRegion
| LogDataRegion
| OutputRegion.

Record RegionAccessContract := {
  region_state : Type;
  region_bytes_ref : region_state -> Region -> byte_seq;

  region_bytes_wf :
    forall st region, bytes_wf (region_bytes_ref st region);

  region_byte_at_contract : Prop;
  region_load_word_contract : Prop;
  region_load_n_word_contract : Prop;
  region_copy_to_memory_contract : Prop;
  region_strided_zero_contract : Prop;
  region_count_nonzero_contract : Prop;
  region_slices_equal_contract : Prop;
  region_hash_refinement_contract : Prop;
}.

(* sail/host/scratch.sail: the Sail-cursor-owned executor scratch arena.
   Composite hash preimages and RLP payloads are written in order and then
   cross the boundary as one scratch-backed slice. *)
Record ScratchArenaContract := {
  scratch_state : Type;
  scratch_bytes_ref : scratch_state -> byte_seq;

  scratch_bytes_wf : forall st, bytes_wf (scratch_bytes_ref st);

  scratch_reserve_contract : Prop;
  scratch_store_contract : Prop;
  scratch_truncate_contract : Prop;
}.

(* sail/host/{memory,stack,frame_stack,output}.sail: EVM memory with frame
   enter/leave, the operand stack with frame windows, the suspended-frame
   continuation stack, and the returndata output buffer. *)
Record MemoryStackContract := {
  memory_state : Type;
  stack_state : Type;
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
  memory_expand_contract : Prop;
  memory_move_contract : Prop;
  output_buffer_contract : Prop;
  evm_stack_lifo_contract : Prop;
  continuation_stack_lifo_contract : Prop;
}.

(* sail/host/environment.sail: the fixed-size, O(1) ancestor-hash table.
   ancestor_hash_write installs the hash at a distance index during input
   decoding; ancestor_hash_read serves BLOCKHASH. *)
Record AncestorHashContract := {
  ancestor_state : Type;
  ancestor_hash_ref : ancestor_state -> Z -> word256;
  ancestor_hash_write_step : ancestor_state -> Z -> word256 -> ancestor_state;

  ancestor_hash_write_read :
    forall st index value,
      uint256_wf value ->
      ancestor_hash_ref (ancestor_hash_write_step st index value) index =
        value;
  ancestor_hash_write_other :
    forall st index index' value,
      index <> index' ->
      ancestor_hash_ref (ancestor_hash_write_step st index value) index' =
        ancestor_hash_ref st index';
}.

(* sail/host/code.sail: content-addressed code storage, the packed JUMPDEST
   tables, region interning (input/memory/output/delegation sources), and
   EIP-7702 delegation designator decoding. *)
Record CodeStoreContract := {
  code_state : Type;

  code_lookup_ref : code_state -> word256 -> option byte_seq;
  code_lookup_wf :
    forall st code_hash code,
      code_lookup_ref st code_hash = Some code ->
      uint256_wf code_hash /\ bytes_wf code;

  code_content_addressing_contract : Prop;
  code_region_interning_contract : Prop;
  jumpdest_allocation_contract : Prop;
  jumpdest_mark_contract : Prop;
  jumpdest_ref_contract : Prop;
  code_delegation_contract : Prop;
}.

(* ------------------------------------------------------------------------- *)
(* Extensional world-state reference model                                   *)
(* ------------------------------------------------------------------------- *)

(* This is deliberately not the layout used by either C backend.  In
   particular there are no cache-presence bits, copy-on-write rows, undo
   cursors, physical deletions, or first-write rules.  A backend may use any
   of those devices, but it refines the total maps, the epoch/generation
   stamps, and the journal algebra below. *)
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

(* The end-of-transaction deletion form.  preserve_balance is the
   fork-selected EIP-8246 choice: the pure Sail merge policy decides it and
   the host applies it extensionally. *)
Definition deleted_account
    (preserve_balance : bool) (empty_code_hash : word256)
    (account : Account) : Account :=
  {| account_nonce := 0;
     account_balance :=
       if preserve_balance then account.(account_balance) else 0;
     (* Keep the authenticated anchor.  A cleared-storage bit selects the
        empty trie when the post-state root is assembled. *)
     account_storage_root := account.(account_storage_root);
     account_code_hash := empty_code_hash;
     account_present := false;
     account_storage_cleared := true;
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_balance (value : word256) (account : Account)
    : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := value;
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_nonce (value : Z) (account : Account) : Account :=
  {| account_nonce := value;
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_code_hash (value : word256) (account : Account)
    : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := value;
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_present (value : bool) (account : Account)
    : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := value;
     account_storage_cleared := account.(account_storage_cleared);
     account_created := account.(account_created);
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_created (value : bool) (account : Account)
    : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := value;
     account_selfdestructed := account.(account_selfdestructed) |}.

Definition account_with_selfdestructed (value : bool) (account : Account)
    : Account :=
  {| account_nonce := account.(account_nonce);
     account_balance := account.(account_balance);
     account_storage_root := account.(account_storage_root);
     account_code_hash := account.(account_code_hash);
     account_present := account.(account_present);
     account_storage_cleared := account.(account_storage_cleared);
     account_created := account.(account_created);
     account_selfdestructed := value |}.

Definition world_write_account
    (world : PersistentWorld) (target : address) (value : Account)
    : PersistentWorld :=
  {| world_account_at :=
       fun candidate =>
         if Z.eqb candidate target then value
         else world.(world_account_at) candidate;
     world_storage_at := world.(world_storage_at) |}.

Definition world_update_account
    (world : PersistentWorld) (target : address)
    (update : Account -> Account) : PersistentWorld :=
  world_write_account world target
    (update (world.(world_account_at) target)).

Definition world_write_storage
    (world : PersistentWorld) (target : address) (target_slot : storage_key)
    (value : word256) : PersistentWorld :=
  {| world_account_at := world.(world_account_at);
     world_storage_at :=
       fun candidate candidate_slot =>
         if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
         then value
         else world.(world_storage_at) candidate candidate_slot |}.

Definition transient_state := address -> storage_key -> word256.
(* EIP-2929 warmth is an epoch stamp, not a boolean: warm_reset installs the
   transaction's block-access epoch, marking stamps the current epoch, and a
   location is warm exactly when its stamp has reached the current epoch.
   Frame rollback restores a *prior stamp* (JournalWarm*Changed.prior_epoch),
   never a boolean. *)
Definition warm_address_state := address -> access_index.
Definition warm_slot_state := address -> storage_key -> access_index.
(* An account-wide storage generation plus per-row generations model
   storage clears: a row is visible only while its generation matches its
   account's.  Clears bump the account generation; both bumps are journal
   entries and therefore individually revertible. *)
Definition account_generation_state := address -> generation.
Definition row_generation_state := address -> storage_key -> generation.

Record LogicalLog := {
  logical_log_address : address;
  logical_log_topics : list word256;
  logical_log_data : byte_seq;
}.

Definition empty_transient_state : transient_state := fun _ _ => 0.
Definition empty_warm_address_state : warm_address_state := fun _ => 0.
Definition empty_warm_slot_state : warm_slot_state := fun _ _ => 0.
Definition empty_account_generation_state : account_generation_state :=
  fun _ => 0.
Definition empty_row_generation_state : row_generation_state := fun _ _ => 0.

Definition transient_write
    (transient : transient_state) (target : address)
    (target_slot : storage_key) (value : word256) : transient_state :=
  fun candidate candidate_slot =>
    if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
    then value
    else transient candidate candidate_slot.

Definition warm_address_stamp
    (warm : warm_address_state) (target : address) (epoch : access_index)
    : warm_address_state :=
  fun candidate => if Z.eqb candidate target then epoch else warm candidate.

Definition warm_slot_stamp
    (warm : warm_slot_state) (target : address) (target_slot : storage_key)
    (epoch : access_index) : warm_slot_state :=
  fun candidate candidate_slot =>
    if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
    then epoch
    else warm candidate candidate_slot.

Definition account_generation_write
    (generations : account_generation_state) (target : address)
    (value : generation) : account_generation_state :=
  fun candidate =>
    if Z.eqb candidate target then value else generations candidate.

Definition row_generation_write
    (generations : row_generation_state) (target : address)
    (target_slot : storage_key) (value : generation)
    : row_generation_state :=
  fun candidate candidate_slot =>
    if andb (Z.eqb candidate target) (Z.eqb candidate_slot target_slot)
    then value
    else generations candidate candidate_slot.

(* block_start is the authenticated parent state.  block_current accumulates
   successful transactions.  tx_start is the EIP-2200 original-value snapshot,
   while tx_current is the live state of the executing transaction.  The
   worklists record the transaction overlay's insertion order: rollback
   removes their most recently appended entries.  Logs live in a
   block-lifetime store; log_tx_start opens the current transaction's
   window. *)
Record ReferenceWorldState := {
  reference_block_start : PersistentWorld;
  reference_block_current : PersistentWorld;
  reference_tx_start : PersistentWorld;
  reference_tx_current : PersistentWorld;
  reference_transient : transient_state;
  reference_warm_epoch : access_index;
  reference_warm_addresses : warm_address_state;
  reference_warm_slots : warm_slot_state;
  reference_account_generations : account_generation_state;
  reference_row_generations : row_generation_state;
  reference_tx_accounts : list address;
  reference_tx_storage : address -> list storage_key;
  reference_logs : list LogicalLog;
  reference_log_tx_start : Z;
}.

(* The semantic content of one open frame: exactly the frame-revertible
   sub-state.  block layers, the warm epoch counter, and the log window
   start are transaction-scoped and deliberately absent. *)
Record TransactionSnapshot := {
  snapshot_tx_current : PersistentWorld;
  snapshot_transient : transient_state;
  snapshot_warm_addresses : warm_address_state;
  snapshot_warm_slots : warm_slot_state;
  snapshot_account_generations : account_generation_state;
  snapshot_row_generations : row_generation_state;
  snapshot_tx_accounts : list address;
  snapshot_tx_storage : address -> list storage_key;
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

Definition account_generation_state_equiv
    (left right : account_generation_state) : Prop :=
  forall account, left account = right account.

Definition row_generation_state_equiv
    (left right : row_generation_state) : Prop :=
  forall account slot, left account slot = right account slot.

Definition tx_storage_lists_equiv
    (left right : address -> list storage_key) : Prop :=
  forall account, left account = right account.

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
  left.(reference_warm_epoch) = right.(reference_warm_epoch) /\
  warm_address_state_equiv left.(reference_warm_addresses)
                           right.(reference_warm_addresses) /\
  warm_slot_state_equiv left.(reference_warm_slots)
                        right.(reference_warm_slots) /\
  account_generation_state_equiv left.(reference_account_generations)
                                 right.(reference_account_generations) /\
  row_generation_state_equiv left.(reference_row_generations)
                             right.(reference_row_generations) /\
  left.(reference_tx_accounts) = right.(reference_tx_accounts) /\
  tx_storage_lists_equiv left.(reference_tx_storage)
                         right.(reference_tx_storage) /\
  left.(reference_logs) = right.(reference_logs) /\
  left.(reference_log_tx_start) = right.(reference_log_tx_start).

Definition reference_checkpoint
    (state : ReferenceWorldState) : TransactionSnapshot :=
  {| snapshot_tx_current := state.(reference_tx_current);
     snapshot_transient := state.(reference_transient);
     snapshot_warm_addresses := state.(reference_warm_addresses);
     snapshot_warm_slots := state.(reference_warm_slots);
     snapshot_account_generations := state.(reference_account_generations);
     snapshot_row_generations := state.(reference_row_generations);
     snapshot_tx_accounts := state.(reference_tx_accounts);
     snapshot_tx_storage := state.(reference_tx_storage);
     snapshot_logs := state.(reference_logs) |}.

Definition reference_revert
    (state : ReferenceWorldState) (snapshot : TransactionSnapshot)
    : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_tx_start);
     reference_tx_current := snapshot.(snapshot_tx_current);
     reference_transient := snapshot.(snapshot_transient);
     reference_warm_epoch := state.(reference_warm_epoch);
     reference_warm_addresses := snapshot.(snapshot_warm_addresses);
     reference_warm_slots := snapshot.(snapshot_warm_slots);
     reference_account_generations :=
       snapshot.(snapshot_account_generations);
     reference_row_generations := snapshot.(snapshot_row_generations);
     reference_tx_accounts := snapshot.(snapshot_tx_accounts);
     reference_tx_storage := snapshot.(snapshot_tx_storage);
     reference_logs := snapshot.(snapshot_logs);
     reference_log_tx_start := state.(reference_log_tx_start) |}.

(* ------------------------------------------------------------------------- *)
(* The state journal: a closed operation algebra                             *)
(* ------------------------------------------------------------------------- *)

(* sail/host/journal.sail: every frame-revertible mutation records exactly
   the semantic field needed to restore its predecessor, and the
   checkpoint/commit markers encode the frame nesting inside the same
   ordered stream.  The four externs are all unit -> unit: no handle
   crosses the boundary.  Variant names and payloads mirror the Sail union
   one for one; the journal is newest-first here. *)
Inductive StateJournalEntry : Type :=
| JournalTransientChanged (account : address) (slot : storage_key)
                          (prior : word256)
| JournalWarmAccountChanged (account : address) (prior_epoch : access_index)
| JournalWarmStorageChanged (account : address) (slot : storage_key)
                            (prior_epoch : access_index)
| JournalAccountBalanceChanged (account : address) (prior : word256)
| JournalAccountNonceChanged (account : address) (prior : Z)
| JournalAccountCodeHashChanged (account : address) (prior : word256)
| JournalAccountExistsChanged (account : address) (prior : bool)
| JournalAccountCreatedChanged (account : address) (prior : bool)
| JournalAccountSelfdestructedChanged (account : address) (prior : bool)
| JournalTransactionAccountListed
| JournalTransactionStorageListed (account : address)
| JournalLogAppended
| JournalStorageValueChanged (account : address) (slot : storage_key)
                             (prior : word256)
| JournalStorageRowGenerationChanged (account : address)
                                     (slot : storage_key)
                                     (prior : generation)
| JournalAccountStorageGenerationChanged (account : address)
                                         (prior : generation)
| JournalFrameCheckpointed
| JournalFrameCommitted.

Definition state_journal := list StateJournalEntry.

Definition journal_entry_is_marker (entry : StateJournalEntry) : Prop :=
  entry = JournalFrameCheckpointed \/ entry = JournalFrameCommitted.

Definition journal_entry_semantic (entry : StateJournalEntry) : Prop :=
  ~ journal_entry_is_marker entry.

Definition removelast_storage
    (lists : address -> list storage_key) (target : address)
    : address -> list storage_key :=
  fun candidate =>
    if Z.eqb candidate target then removelast (lists candidate)
    else lists candidate.

(* Applies the inverse of one journal entry.  Structural markers have no
   semantic inverse.  Note there is no storage-root variant: transaction
   writes cannot change an account's witness anchor, and the algebra's
   closedness makes that a boundary obligation rather than a convention. *)
Definition apply_inverse
    (entry : StateJournalEntry) (state : ReferenceWorldState)
    : ReferenceWorldState :=
  match entry with
  | JournalTransientChanged account slot prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient :=
           transient_write state.(reference_transient) account slot prior;
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalWarmAccountChanged account prior_epoch =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses :=
           warm_address_stamp state.(reference_warm_addresses) account
                              prior_epoch;
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalWarmStorageChanged account slot prior_epoch =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots :=
           warm_slot_stamp state.(reference_warm_slots) account slot
                           prior_epoch;
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountBalanceChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_balance prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountNonceChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_nonce prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountCodeHashChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_code_hash prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountExistsChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_present prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountCreatedChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_created prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountSelfdestructedChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_update_account state.(reference_tx_current) account
                                (account_with_selfdestructed prior);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalTransactionAccountListed =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := removelast state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalTransactionStorageListed account =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage :=
           removelast_storage state.(reference_tx_storage) account;
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalLogAppended =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := removelast state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalStorageValueChanged account slot prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current :=
           world_write_storage state.(reference_tx_current) account slot
                               prior;
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalStorageRowGenerationChanged account slot prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           state.(reference_account_generations);
         reference_row_generations :=
           row_generation_write state.(reference_row_generations) account
                                slot prior;
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalAccountStorageGenerationChanged account prior =>
      {| reference_block_start := state.(reference_block_start);
         reference_block_current := state.(reference_block_current);
         reference_tx_start := state.(reference_tx_start);
         reference_tx_current := state.(reference_tx_current);
         reference_transient := state.(reference_transient);
         reference_warm_epoch := state.(reference_warm_epoch);
         reference_warm_addresses := state.(reference_warm_addresses);
         reference_warm_slots := state.(reference_warm_slots);
         reference_account_generations :=
           account_generation_write state.(reference_account_generations)
                                    account prior;
         reference_row_generations := state.(reference_row_generations);
         reference_tx_accounts := state.(reference_tx_accounts);
         reference_tx_storage := state.(reference_tx_storage);
         reference_logs := state.(reference_logs);
         reference_log_tx_start := state.(reference_log_tx_start) |}
  | JournalFrameCheckpointed => state
  | JournalFrameCommitted => state
  end.

(* The Coq image of the backwards frame-marker scan: walking the journal
   newest-first, a JournalFrameCommitted marker closes one earlier
   checkpoint; the innermost open JournalFrameCheckpointed marker splits the
   stream.  The suffix (everything newer than the marker, committed child
   frames included) is what revert replays; the base is the journal that
   remains once the marker is consumed. *)
Fixpoint split_at_open_checkpoint_from
    (journal : state_journal) (closed : nat)
    : option (list StateJournalEntry * state_journal) :=
  match journal with
  | [] => None
  | JournalFrameCheckpointed :: rest =>
      match closed with
      | O => Some ([], rest)
      | S remaining =>
          match split_at_open_checkpoint_from rest remaining with
          | None => None
          | Some (suffix, base) =>
              Some (JournalFrameCheckpointed :: suffix, base)
          end
      end
  | JournalFrameCommitted :: rest =>
      match split_at_open_checkpoint_from rest (S closed) with
      | None => None
      | Some (suffix, base) => Some (JournalFrameCommitted :: suffix, base)
      end
  | entry :: rest =>
      match split_at_open_checkpoint_from rest closed with
      | None => None
      | Some (suffix, base) => Some (entry :: suffix, base)
      end
  end.

Definition split_at_open_checkpoint (journal : state_journal)
    : option (list StateJournalEntry * state_journal) :=
  split_at_open_checkpoint_from journal O.

Definition journal_replay
    (suffix : list StateJournalEntry) (state : ReferenceWorldState)
    : ReferenceWorldState :=
  fold_left (fun current entry => apply_inverse entry current) suffix state.

(* state_journal_checkpoint: appends the frame marker. *)
Definition journal_checkpoint (journal : state_journal) : state_journal :=
  JournalFrameCheckpointed :: journal.

(* state_journal_commit: appends a JournalFrameCommitted for the innermost
   open checkpoint.  The committed frame's mutations remain in the journal
   and are therefore still revertible by a parent.  Committing with no open
   checkpoint is a host failure. *)
Definition journal_commit (journal : state_journal)
    : option state_journal :=
  match split_at_open_checkpoint journal with
  | None => None
  | Some _ => Some (JournalFrameCommitted :: journal)
  end.

(* state_journal_revert: replays entries backwards to the innermost open
   checkpoint, then removes its marker.  Reverting with no open checkpoint
   is a host failure. *)
Definition reference_journal_revert
    (state : ReferenceWorldState) (journal : state_journal)
    : option (ReferenceWorldState * state_journal) :=
  match split_at_open_checkpoint journal with
  | None => None
  | Some (suffix, base) => Some (journal_replay suffix state, base)
  end.

(* ------------------------------------------------------------------------- *)
(* Reference lifecycle and reads                                             *)
(* ------------------------------------------------------------------------- *)

Definition reference_start_block
    (initial : PersistentWorld) : ReferenceWorldState :=
  {| reference_block_start := initial;
     reference_block_current := initial;
     reference_tx_start := initial;
     reference_tx_current := initial;
     reference_transient := empty_transient_state;
     reference_warm_epoch := 1;
     reference_warm_addresses := empty_warm_address_state;
     reference_warm_slots := empty_warm_slot_state;
     reference_account_generations := empty_account_generation_state;
     reference_row_generations := empty_row_generation_state;
     reference_tx_accounts := [];
     reference_tx_storage := fun _ => [];
     reference_logs := [];
     reference_log_tx_start := 0 |}.

(* The k_tx_reset bundle: transient, worklists, generations, the log window,
   and the journal reset together; warm_reset installs the transaction's
   block-access epoch (the new current epoch is epoch + 1, so nothing stamped
   in an earlier transaction stays warm). *)
Definition reference_begin_transaction
    (epoch : access_index) (state : ReferenceWorldState)
    : ReferenceWorldState :=
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := state.(reference_block_current);
     reference_tx_start := state.(reference_block_current);
     reference_tx_current := state.(reference_block_current);
     reference_transient := empty_transient_state;
     reference_warm_epoch := epoch + 1;
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_account_generations := empty_account_generation_state;
     reference_row_generations := empty_row_generation_state;
     reference_tx_accounts := [];
     reference_tx_storage := fun _ => [];
     reference_logs := state.(reference_logs);
     reference_log_tx_start := Z.of_nat (length state.(reference_logs)) |}.

Definition reference_read_account
    (state : ReferenceWorldState) (account : address) : Account :=
  state.(reference_tx_current).(world_account_at) account.

(* Storage reads are generation-gated: a row is visible only while its
   generation matches its account's current storage generation.  A stale row
   reads as zero — the semantic content of an account-wide storage clear. *)
Definition reference_storage_visible
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : bool :=
  Z.eqb (state.(reference_row_generations) account slot)
        (state.(reference_account_generations) account).

Definition reference_read_storage_current
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  if reference_storage_visible state account slot
  then state.(reference_tx_current).(world_storage_at) account slot
  else 0.

(* The SSTORE original value is defined by transaction entry, not by the
   first insertion into an implementation table. *)
Definition reference_read_storage_original
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  state.(reference_tx_start).(world_storage_at) account slot.

(* A transaction-layer lookup distinguishes an actual visible row from a slot
   made known-zero by an account storage clear (StorageTxLookup's
   Hit / Cleared / Miss cases): the cleared case still records the first
   EIP-7928 read of the slot. *)
Inductive StorageLookupClass : Type :=
| StorageLookupHit (value : word256)
| StorageLookupCleared
| StorageLookupMiss.

Definition reference_is_account_warm
    (state : ReferenceWorldState) (account : address) : bool :=
  state.(reference_warm_epoch) <=? state.(reference_warm_addresses) account.

Definition reference_is_storage_warm
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : bool :=
  state.(reference_warm_epoch) <=?
    state.(reference_warm_slots) account slot.

Definition reference_read_transient
    (state : ReferenceWorldState) (account : address) (slot : storage_key)
    : word256 :=
  state.(reference_transient) account slot.

Definition finalized_world
    (preserve_balance : bool) (empty_code_hash : word256)
    (delete_at_end : address -> bool) (world : PersistentWorld)
    : PersistentWorld :=
  {| world_account_at :=
       fun account =>
         account_without_transaction_flags
           (if delete_at_end account
            then deleted_account preserve_balance empty_code_hash
                   (world.(world_account_at) account)
            else world.(world_account_at) account);
     world_storage_at :=
       fun account slot =>
         if delete_at_end account then 0
         else world.(world_storage_at) account slot |}.

(* delete_at_end and preserve_balance are computed by the pure Sail fork
   policy before the host transition; the host applies them extensionally.
   Committing materializes the generation-gated storage view into the plain
   maps and clears the transaction layer. *)
Definition materialized_tx_world
    (state : ReferenceWorldState) : PersistentWorld :=
  {| world_account_at :=
       state.(reference_tx_current).(world_account_at);
     world_storage_at :=
       fun account slot => reference_read_storage_current state account slot
  |}.

Definition reference_commit_transaction
    (preserve_balance : bool) (empty_code_hash : word256)
    (delete_at_end : address -> bool) (state : ReferenceWorldState)
    : ReferenceWorldState :=
  let committed :=
      finalized_world preserve_balance empty_code_hash delete_at_end
                      (materialized_tx_world state) in
  {| reference_block_start := state.(reference_block_start);
     reference_block_current := committed;
     reference_tx_start := committed;
     reference_tx_current := committed;
     reference_transient := state.(reference_transient);
     reference_warm_epoch := state.(reference_warm_epoch);
     reference_warm_addresses := state.(reference_warm_addresses);
     reference_warm_slots := state.(reference_warm_slots);
     reference_account_generations := empty_account_generation_state;
     reference_row_generations := empty_row_generation_state;
     reference_tx_accounts := [];
     reference_tx_storage := fun _ => [];
     reference_logs := state.(reference_logs);
     reference_log_tx_start := state.(reference_log_tx_start) |}.

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

(* sail/host/state.sail and sail/host/journal.sail externs collectively
   refine these semantic operations:

     k_aload / acct_tx_* / acct_block_*      -> account reads and writes
     k_sload / storage_tx_* / storage_block_* -> generation-gated storage
     transient_* / warm marks                 -> transient and epoch stamps
     state_journal_{reset,checkpoint,revert,commit}
                                              -> the journal algebra
     k_tx_merge + block iterators             -> commit and finite deltas

   The split tx/block caches and the physical journal storage are a backend
   protocol, not part of the abstract state. *)
Record WorldStateContract := {
  world_state : Type;

  world_observe : world_state -> ReferenceWorldState;
  (* The ghost journal: relates a backend state to the reference journal it
     denotes.  No handle crosses the boundary; the nesting structure is data
     in the same ordered stream. *)
  world_journal_denotes : world_state -> state_journal -> Prop;

  world_start_block_step : PersistentWorld -> world_state;
  world_begin_transaction_step :
    access_index -> world_state -> world_state;
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
    bool -> word256 -> world_state -> address -> world_state;

  world_checkpoint_step : world_state -> world_state;
  world_commit_frame_step : world_state -> option world_state;
  world_revert_frame_step : world_state -> option world_state;

  world_commit_transaction_step :
    bool -> word256 -> (address -> bool) -> world_state -> world_state;

  (* Read-through materialization must not change the logical observation;
     the supplied digest is the precomputed secure key. *)
  world_cache_account_step :
    world_state -> address -> word256 -> Account -> world_state;
  world_cache_storage_step :
    world_state -> address -> storage_key -> word256 -> word256 ->
    world_state;

  world_read_transient_ref :
    world_state -> address -> storage_key -> word256;
  world_write_transient_step :
    world_state -> address -> storage_key -> word256 -> world_state;

  world_account_is_warm_ref : world_state -> address -> bool;
  world_storage_is_warm_ref :
    world_state -> address -> storage_key -> bool;
  world_mark_warm_account_step : world_state -> address -> world_state;
  world_mark_warm_storage_step :
    world_state -> address -> storage_key -> world_state;

  world_append_log_step : world_state -> LogicalLog -> world_state;
  world_read_logs_ref : world_state -> list LogicalLog;
  log_record_builder_contract : Prop;

  world_transaction_delta_ref : world_state -> WorldDelta;
  world_block_delta_ref : world_state -> WorldDelta;
}.

(* The closed-algebra ghost law: a mutating step appends exactly the
   semantic (marker-free) entries whose inverse replay restores the prior
   observation.  Observation-preserving steps append nothing. *)
Definition journal_step_recorded
    (contract : WorldStateContract)
    (before after : contract.(world_state)) : Prop :=
  forall journal,
    contract.(world_journal_denotes) before journal ->
    exists appended,
      Forall journal_entry_semantic appended /\
      contract.(world_journal_denotes) after (appended ++ journal) /\
      reference_world_state_equiv
        (journal_replay appended (contract.(world_observe) after))
        (contract.(world_observe) before).

(* The canonical semantic instantiation pairs the reference world with the
   reference journal itself; a native backend may use any physical journal
   storage related to it by world_journal_denotes.  The appended entries in
   the canonical mutators are one faithful choice; the boundary only demands
   inverse-replay exactness. *)
Definition reference_world_pair := (ReferenceWorldState * state_journal)%type.

Definition reference_world_state_contract
    (transaction_delta block_delta : ReferenceWorldState -> WorldDelta)
    (log_record_builder : Prop)
    : WorldStateContract :=
  {| world_state := reference_world_pair;
     world_observe := fst;
     world_journal_denotes := fun state journal => snd state = journal;
     world_start_block_step :=
       fun initial => (reference_start_block initial, []);
     world_begin_transaction_step :=
       fun epoch state =>
         (reference_begin_transaction epoch (fst state), []);
     world_read_account_ref :=
       fun state => reference_read_account (fst state);
     world_read_storage_current_ref :=
       fun state => reference_read_storage_current (fst state);
     world_read_storage_original_ref :=
       fun state => reference_read_storage_original (fst state);
     world_write_account_step :=
       fun state account value =>
         let prior := reference_read_account (fst state) account in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current :=
               world_write_account (fst state).(reference_tx_current)
                                   account value;
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalAccountBalanceChanged account prior.(account_balance)
            :: JournalAccountNonceChanged account prior.(account_nonce)
            :: JournalAccountCodeHashChanged account
                 prior.(account_code_hash)
            :: JournalAccountExistsChanged account prior.(account_present)
            :: JournalAccountCreatedChanged account prior.(account_created)
            :: JournalAccountSelfdestructedChanged account
                 prior.(account_selfdestructed)
            :: snd state);
     world_write_storage_step :=
       fun state account slot value =>
         let prior_value :=
             reference_read_storage_current (fst state) account slot in
         let prior_row_generation :=
             (fst state).(reference_row_generations) account slot in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current :=
               world_write_storage (fst state).(reference_tx_current)
                                   account slot value;
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               row_generation_write
                 (fst state).(reference_row_generations) account slot
                 ((fst state).(reference_account_generations) account);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalStorageValueChanged account slot prior_value
            :: JournalStorageRowGenerationChanged account slot
                 prior_row_generation
            :: snd state);
     world_clear_storage_step :=
       fun state account =>
         let prior_generation :=
             (fst state).(reference_account_generations) account in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current :=
               world_update_account (fst state).(reference_tx_current)
                                    account account_with_cleared_storage;
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               account_generation_write
                 (fst state).(reference_account_generations) account
                 (prior_generation + 1);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalAccountStorageGenerationChanged account prior_generation
            :: JournalAccountExistsChanged account
                 (reference_read_account (fst state) account)
                   .(account_present)
            :: snd state);
     world_delete_account_step :=
       fun preserve_balance empty_code_hash state account =>
         let prior := reference_read_account (fst state) account in
         let prior_generation :=
             (fst state).(reference_account_generations) account in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current :=
               world_update_account (fst state).(reference_tx_current)
                 account
                 (deleted_account preserve_balance empty_code_hash);
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               account_generation_write
                 (fst state).(reference_account_generations) account
                 (prior_generation + 1);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalAccountBalanceChanged account prior.(account_balance)
            :: JournalAccountNonceChanged account prior.(account_nonce)
            :: JournalAccountCodeHashChanged account
                 prior.(account_code_hash)
            :: JournalAccountExistsChanged account prior.(account_present)
            :: JournalAccountStorageGenerationChanged account
                 prior_generation
            :: snd state);
     world_checkpoint_step :=
       fun state => (fst state, journal_checkpoint (snd state));
     world_commit_frame_step :=
       fun state =>
         match journal_commit (snd state) with
         | None => None
         | Some journal => Some (fst state, journal)
         end;
     world_revert_frame_step :=
       fun state =>
         match reference_journal_revert (fst state) (snd state) with
         | None => None
         | Some (reverted, journal) => Some (reverted, journal)
         end;
     world_commit_transaction_step :=
       fun preserve_balance empty_code_hash delete_at_end state =>
         (reference_commit_transaction preserve_balance empty_code_hash
                                       delete_at_end (fst state),
          snd state);
     world_cache_account_step := fun state _ _ _ => state;
     world_cache_storage_step := fun state _ _ _ _ => state;
     world_read_transient_ref :=
       fun state => reference_read_transient (fst state);
     world_write_transient_step :=
       fun state account slot value =>
         let prior := reference_read_transient (fst state) account slot in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current := (fst state).(reference_tx_current);
             reference_transient :=
               transient_write (fst state).(reference_transient) account
                               slot value;
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalTransientChanged account slot prior :: snd state);
     world_account_is_warm_ref :=
       fun state => reference_is_account_warm (fst state);
     world_storage_is_warm_ref :=
       fun state => reference_is_storage_warm (fst state);
     world_mark_warm_account_step :=
       fun state account =>
         let prior := (fst state).(reference_warm_addresses) account in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current := (fst state).(reference_tx_current);
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               warm_address_stamp (fst state).(reference_warm_addresses)
                 account ((fst state).(reference_warm_epoch));
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalWarmAccountChanged account prior :: snd state);
     world_mark_warm_storage_step :=
       fun state account slot =>
         let prior := (fst state).(reference_warm_slots) account slot in
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current := (fst state).(reference_tx_current);
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots :=
               warm_slot_stamp (fst state).(reference_warm_slots) account
                 slot ((fst state).(reference_warm_epoch));
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs);
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalWarmStorageChanged account slot prior :: snd state);
     world_append_log_step :=
       fun state entry =>
         ({| reference_block_start := (fst state).(reference_block_start);
             reference_block_current :=
               (fst state).(reference_block_current);
             reference_tx_start := (fst state).(reference_tx_start);
             reference_tx_current := (fst state).(reference_tx_current);
             reference_transient := (fst state).(reference_transient);
             reference_warm_epoch := (fst state).(reference_warm_epoch);
             reference_warm_addresses :=
               (fst state).(reference_warm_addresses);
             reference_warm_slots := (fst state).(reference_warm_slots);
             reference_account_generations :=
               (fst state).(reference_account_generations);
             reference_row_generations :=
               (fst state).(reference_row_generations);
             reference_tx_accounts := (fst state).(reference_tx_accounts);
             reference_tx_storage := (fst state).(reference_tx_storage);
             reference_logs := (fst state).(reference_logs) ++ [entry];
             reference_log_tx_start :=
               (fst state).(reference_log_tx_start) |},
          JournalLogAppended :: snd state);
     world_read_logs_ref := fun state => (fst state).(reference_logs);
     log_record_builder_contract := log_record_builder;
     world_transaction_delta_ref :=
       fun state => transaction_delta (fst state);
     world_block_delta_ref := fun state => block_delta (fst state) |}.

Definition world_state_boundary
    (crypto : CryptoContract) (contract : WorldStateContract) : Prop :=
  (forall initial,
      let state := contract.(world_start_block_step) initial in
      reference_world_state_equiv
        (contract.(world_observe) state)
        (reference_start_block initial) /\
      contract.(world_journal_denotes) state []) /\
  (forall epoch state,
      let next := contract.(world_begin_transaction_step) epoch state in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_begin_transaction
          epoch (contract.(world_observe) state)) /\
      contract.(world_journal_denotes) next []) /\
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
      persistent_world_equiv
        ((contract.(world_observe) next).(reference_tx_current))
        (world_write_account
          ((contract.(world_observe) state).(reference_tx_current))
          account value) /\
      journal_step_recorded contract state next) /\
  (forall state account slot value,
      let next :=
          contract.(world_write_storage_step) state account slot value in
      reference_read_storage_current
        (contract.(world_observe) next) account slot = value /\
      journal_step_recorded contract state next) /\
  (forall state account,
      let next := contract.(world_clear_storage_step) state account in
      (forall slot,
          reference_read_storage_current
            (contract.(world_observe) next) account slot = 0) /\
      journal_step_recorded contract state next) /\
  (forall preserve_balance empty_code_hash state account,
      let next :=
          contract.(world_delete_account_step)
            preserve_balance empty_code_hash state account in
      reference_read_account (contract.(world_observe) next) account =
        deleted_account preserve_balance empty_code_hash
          (reference_read_account (contract.(world_observe) state)
             account) /\
      journal_step_recorded contract state next) /\
  (forall state,
      let next := contract.(world_checkpoint_step) state in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (contract.(world_observe) state) /\
      (forall journal,
          contract.(world_journal_denotes) state journal ->
          contract.(world_journal_denotes) next
            (JournalFrameCheckpointed :: journal))) /\
  (forall state journal,
      contract.(world_journal_denotes) state journal ->
      match journal_commit journal with
      | Some committed =>
          exists next,
            contract.(world_commit_frame_step) state = Some next /\
            reference_world_state_equiv
              (contract.(world_observe) next)
              (contract.(world_observe) state) /\
            contract.(world_journal_denotes) next committed
      | None => contract.(world_commit_frame_step) state = None
      end) /\
  (forall state journal,
      contract.(world_journal_denotes) state journal ->
      match reference_journal_revert
              (contract.(world_observe) state) journal with
      | Some (reverted, remaining) =>
          exists next,
            contract.(world_revert_frame_step) state = Some next /\
            reference_world_state_equiv
              (contract.(world_observe) next) reverted /\
            contract.(world_journal_denotes) next remaining
      | None => contract.(world_revert_frame_step) state = None
      end) /\
  (forall preserve_balance empty_code_hash delete_at_end state,
      let next :=
          contract.(world_commit_transaction_step)
            preserve_balance empty_code_hash delete_at_end state in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (reference_commit_transaction
          preserve_balance empty_code_hash delete_at_end
          (contract.(world_observe) state))) /\
  (forall state account,
      let next :=
          contract.(world_cache_account_step) state account
            (account_secure_key crypto account)
            (contract.(world_read_account_ref) state account) in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (contract.(world_observe) state) /\
      journal_step_recorded contract state next) /\
  (forall state account slot,
      let next :=
          contract.(world_cache_storage_step) state account slot
            (storage_secure_key crypto slot)
            (contract.(world_read_storage_current_ref)
              state account slot) in
      reference_world_state_equiv
        (contract.(world_observe) next)
        (contract.(world_observe) state) /\
      journal_step_recorded contract state next) /\
  (forall state account slot,
      contract.(world_read_transient_ref) state account slot =
      reference_read_transient
        (contract.(world_observe) state) account slot) /\
  (forall state account slot value,
      let next :=
          contract.(world_write_transient_step) state account slot value in
      transient_state_equiv
        ((contract.(world_observe) next).(reference_transient))
        (transient_write
          ((contract.(world_observe) state).(reference_transient))
          account slot value) /\
      journal_step_recorded contract state next) /\
  (forall state account,
      contract.(world_account_is_warm_ref) state account =
      reference_is_account_warm
        (contract.(world_observe) state) account) /\
  (forall state account slot,
      contract.(world_storage_is_warm_ref) state account slot =
      reference_is_storage_warm
        (contract.(world_observe) state) account slot) /\
  (forall state account,
      let next := contract.(world_mark_warm_account_step) state account in
      reference_is_account_warm
        (contract.(world_observe) next) account = true /\
      journal_step_recorded contract state next) /\
  (forall state account slot,
      let next :=
          contract.(world_mark_warm_storage_step) state account slot in
      reference_is_storage_warm
        (contract.(world_observe) next) account slot = true /\
      journal_step_recorded contract state next) /\
  (forall state entry,
      let next := contract.(world_append_log_step) state entry in
      (contract.(world_observe) next).(reference_logs) =
        (contract.(world_observe) state).(reference_logs) ++ [entry] /\
      journal_step_recorded contract state next) /\
  (forall state,
      contract.(world_read_logs_ref) state =
        (contract.(world_observe) state).(reference_logs)) /\
  contract.(log_record_builder_contract) /\
  (forall state,
      world_delta_describes
        (contract.(world_observe) state).(reference_tx_start)
        (materialized_tx_world (contract.(world_observe) state))
        (contract.(world_transaction_delta_ref) state)) /\
  (forall state,
      world_delta_describes
        (contract.(world_observe) state).(reference_block_start)
        (contract.(world_observe) state).(reference_block_current)
        (contract.(world_block_delta_ref) state)) /\
  (forall state,
      world_delta_securely_ordered
        crypto (contract.(world_block_delta_ref) state)).

(* ------------------------------------------------------------------------- *)
(* Block access list, authorization tracker, witness DB                      *)
(* ------------------------------------------------------------------------- *)

(* sail/host/state.sail BalIterEntry: the canonical account-delimited event
   stream the EIP-7928 recorder canonicalizes into.  Multiplicity matters:
   each (index, value) change is one event, so two transactions changing the
   same slot yield two BalStorageChange events. *)
Inductive BalIterEntry : Type :=
| BalAccount (account : address)
| BalStorageChange (slot : word256) (index : access_index)
                   (value : word256)
| BalStorageRead (slot : word256)
| BalBalanceChange (index : access_index) (value : word256)
| BalNonceChange (index : access_index) (value : Z)
| BalCodeChange (index : access_index) (code_hash : word256)
| BalAccountEnd
| BalEmpty.

Record BalRecorderContract := {
  bal_state : Type;

  bal_observe : bal_state -> list BalIterEntry;

  bal_stream_account_delimited_contract : Prop;
  bal_stream_securely_ordered_contract : Prop;
  bal_reads_survive_rollback_contract : Prop;
  bal_changes_follow_rollback_contract : Prop;
  bal_read_shadowed_by_change_contract : Prop;
}.

(* sail/host/state.sail authorization_tracker_*: EIP-7702 transaction-local
   authority bookkeeping.  Deliberately outside the journal's scope — no
   StateJournalEntry variant records it, so tracker state is not
   frame-revertible. *)
Record AuthorizationTrackerContract := {
  authorization_state : Type;

  authorization_seen_ref : authorization_state -> address -> bool;
  authorization_originally_delegated_ref :
    authorization_state -> address -> bool;
  authorization_delegation_set_ref :
    authorization_state -> address -> bool;
  authorization_commit_step :
    authorization_state -> address -> bool -> bool -> authorization_state;
  authorization_reset_step : Z -> authorization_state;

  authorization_reset_clears :
    forall size account,
      authorization_seen_ref (authorization_reset_step size) account =
        false /\
      authorization_delegation_set_ref (authorization_reset_step size)
        account = false;
  authorization_commit_marks_seen :
    forall st account originally_delegated delegation_set,
      authorization_seen_ref
        (authorization_commit_step st account originally_delegated
           delegation_set) account = true;
  (* The originally-delegated flag records the state before the authority's
     FIRST successful tuple; later commits must not revise it. *)
  authorization_first_commit_wins_contract : Prop;
}.

Definition span := (Z * Z)%type.

(* sail/host/nodes.sail witness-facing externs: nodedb_*.  The node table
   plus the Sail-side authenticated parent-root register is the whole
   lazy-witness model: every walk from that root is a hash-chain proof
   through keccak(node)-keyed lookups.  nodedb_lookup is total — the empty
   slice denotes an unwitnessed node. *)
Record WitnessDbContract := {
  witness_state : Type;

  node_lookup_ref : witness_state -> word256 -> span;
  witness_bytes_ref : witness_state -> span -> byte_seq;

  node_lookup_wf :
    forall witness node_hash,
      bytes_wf (witness_bytes_ref witness (node_lookup_ref witness
                                             node_hash));

  node_db_reset_contract : Prop;
  node_db_insert_select_contract : Prop;
  witness_span_bounds_contract : Prop;
  mpt_authenticated_frontier_contract : Prop;
}.

(* ------------------------------------------------------------------------- *)
(* The complete guest boundary                                               *)
(* ------------------------------------------------------------------------- *)

Record GuestExternContract := {
  guest_input : InputOracle;
  guest_output : OutputTraceContract;
  guest_crypto : CryptoContract;
  guest_accelerators : AcceleratorContract;
  guest_regions : RegionAccessContract;
  guest_scratch : ScratchArenaContract;
  guest_memory_stack : MemoryStackContract;
  guest_ancestors : AncestorHashContract;
  guest_code_store : CodeStoreContract;
  guest_world_state : WorldStateContract;
  guest_bal_recorder : BalRecorderContract;
  guest_authorizations : AuthorizationTrackerContract;
  guest_witness_db : WitnessDbContract;
}.

(* Every named obligation of every sub-contract must appear below exactly
   once: keep this conjunction in 1:1 correspondence with the Prop fields so
   an obligation cannot silently drop out of the top-level boundary. *)
Definition main_boundary (contract : GuestExternContract) : Prop :=
  pairing_check_result_encoding_contract contract.(guest_accelerators) /\
  region_byte_at_contract contract.(guest_regions) /\
  region_load_word_contract contract.(guest_regions) /\
  region_load_n_word_contract contract.(guest_regions) /\
  region_copy_to_memory_contract contract.(guest_regions) /\
  region_strided_zero_contract contract.(guest_regions) /\
  region_count_nonzero_contract contract.(guest_regions) /\
  region_slices_equal_contract contract.(guest_regions) /\
  region_hash_refinement_contract contract.(guest_regions) /\
  scratch_reserve_contract contract.(guest_scratch) /\
  scratch_store_contract contract.(guest_scratch) /\
  scratch_truncate_contract contract.(guest_scratch) /\
  memory_frame_lifo_contract contract.(guest_memory_stack) /\
  memory_expand_contract contract.(guest_memory_stack) /\
  memory_move_contract contract.(guest_memory_stack) /\
  output_buffer_contract contract.(guest_memory_stack) /\
  evm_stack_lifo_contract contract.(guest_memory_stack) /\
  continuation_stack_lifo_contract contract.(guest_memory_stack) /\
  code_content_addressing_contract contract.(guest_code_store) /\
  code_region_interning_contract contract.(guest_code_store) /\
  jumpdest_allocation_contract contract.(guest_code_store) /\
  jumpdest_mark_contract contract.(guest_code_store) /\
  jumpdest_ref_contract contract.(guest_code_store) /\
  code_delegation_contract contract.(guest_code_store) /\
  world_state_boundary contract.(guest_crypto)
                       contract.(guest_world_state) /\
  bal_stream_account_delimited_contract contract.(guest_bal_recorder) /\
  bal_stream_securely_ordered_contract contract.(guest_bal_recorder) /\
  bal_reads_survive_rollback_contract contract.(guest_bal_recorder) /\
  bal_changes_follow_rollback_contract contract.(guest_bal_recorder) /\
  bal_read_shadowed_by_change_contract contract.(guest_bal_recorder) /\
  authorization_first_commit_wins_contract
    contract.(guest_authorizations) /\
  node_db_reset_contract contract.(guest_witness_db) /\
  node_db_insert_select_contract contract.(guest_witness_db) /\
  witness_span_bounds_contract contract.(guest_witness_db) /\
  mpt_authenticated_frontier_contract contract.(guest_witness_db).

End EvmSailExternBoundary.
