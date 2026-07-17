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
  account_base_exists : bool;
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
  memory_slice_stability_contract : Prop;
  scratch_append_refinement_contract : Prop;
  scratch_truncate_refinement_contract : Prop;
  tx_input_view_contract : Prop;
  segmented_hash_refinement_contract : Prop;
  source_precompile_refinement_contract : Prop;
}.

(* sail/host/{memory,output,stack}.sail and sail/evm/machine.sail:
   mem_*, output_buffer_*, stack_*, generic ByteSlice views, the active Code
   value, and Sail-owned EVM returndata state. *)
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
  indexed_code_frame_contract : Prop;
}.

(* sail/host/state.sail state-facing externs:
   transient_storage_*, storage_map_*, acctmap_*, code_db_*. *)
Record WorldStateContract := {
  world_state : Type;

  account_lookup_ref : world_state -> account_hash -> option Account;
  storage_lookup_ref : world_state -> account_hash -> storage_key -> option word256;
  code_lookup_ref : world_state -> word256 -> option byte_seq;
  transient_lookup_ref : world_state -> address -> address -> storage_key -> word256;

  account_lookup_wf :
    forall world key account,
      account_lookup_ref world key = Some account ->
      uint256_wf key /\ account_wf account;
  storage_lookup_wf :
    forall world account_key slot value,
      storage_lookup_ref world account_key slot = Some value ->
      uint256_wf account_key /\ uint256_wf slot /\ uint256_wf value;
  code_lookup_wf :
    forall world code_hash code,
      code_lookup_ref world code_hash = Some code ->
      uint256_wf code_hash /\ bytes_wf code;

  account_cache_update_contract : Prop;
  storage_cache_update_contract : Prop;
  storage_frame_commit_revert_contract : Prop;
  transient_storage_contract : Prop;
  jumpdest_allocation_contract : Prop;
  jumpdest_chunk_store_contract : Prop;
  jumpdest_ref_contract : Prop;
  indexed_code_lookup_contract : Prop;
  code_db_contract : Prop;
  code_delegation_contract : Prop;
}.

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
  indexed_code_frame_contract contract.(guest_memory_stack) /\
  account_cache_update_contract contract.(guest_world_state) /\
  storage_cache_update_contract contract.(guest_world_state) /\
  storage_frame_commit_revert_contract contract.(guest_world_state) /\
  jumpdest_allocation_contract contract.(guest_world_state) /\
  jumpdest_chunk_store_contract contract.(guest_world_state) /\
  jumpdest_ref_contract contract.(guest_world_state) /\
  indexed_code_lookup_contract contract.(guest_world_state) /\
  code_db_contract contract.(guest_world_state) /\
  node_db_insert_select_contract contract.(guest_witness_db) /\
  mpt_authenticated_frontier_contract contract.(guest_witness_db).

End EvmSailExternBoundary.
