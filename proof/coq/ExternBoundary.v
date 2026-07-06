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
| WitnessSource
| MemorySource
| TxInputSource.

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

(* sail/host/io.sail: ssz_src_len, ssz_src_byte, ssz_src_le, ssz_src_be. *)
Record InputOracle := {
  input_bytes : byte_seq;
  input_bytes_wf : bytes_wf input_bytes;
  ssz_src_len_value : word64;
  ssz_src_byte_value : Z -> byte;
  ssz_src_le_value : Z -> Z -> word64;
  ssz_src_be_value : Z -> Z -> word64;

  ssz_src_len_spec :
    ssz_src_len_value = Z.of_nat (length input_bytes);
  ssz_src_byte_in_bounds :
    forall i,
      0 <= i < ssz_src_len_value ->
      ssz_src_byte_value i = nth_byte input_bytes i;
  ssz_src_byte_out_of_bounds :
    forall i,
      i < 0 \/ ssz_src_len_value <= i ->
      ssz_src_byte_value i = 0;
  ssz_src_le_spec :
    forall off width,
      0 <= width <= 8 ->
      ssz_src_le_value off width =
        bytes_le_value (byte_window input_bytes off width);
  ssz_src_be_spec :
    forall off width,
      0 <= width <= 8 ->
      ssz_src_be_value off width =
        bytes_be_value (byte_window input_bytes off width);
}.

(* sail/main.sail: emit_out. *)
Record OutputTraceContract := {
  output_trace : Type;
  output_bytes : output_trace -> byte_seq;
  emit_out_step : output_trace -> byte -> output_trace;

  emit_out_appends :
    forall trace b,
      byte_wf b ->
      output_bytes (emit_out_step trace b) = output_bytes trace ++ [b];
  emit_out_preserves_wf :
    forall trace b,
      bytes_wf (output_bytes trace) ->
      byte_wf b ->
      bytes_wf (output_bytes (emit_out_step trace b));
}.

(* sail/host/io.sail: abstract crypto/precompile functions plus staged
   precompile wrappers for the executable implementation. *)
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

(* C-side byte sources. Sail hash semantics are byte-list functions; optimized
   native paths such as code_db_store_source may resolve a pointer/length pair,
   but must refine the same materialized byte sequence. *)
Record ByteSourceContract := {
  byte_source_state : Type;
  byte_source_bytes_ref : byte_source_state -> ByteSourceKind -> Z -> Z -> byte_seq;

  byte_source_bytes_wf :
    forall st kind off len,
      0 <= off ->
      0 <= len ->
      bytes_wf (byte_source_bytes_ref st kind off len);

  byte_source_resolve_contract : Prop;
  byte_source_keccak_refinement_contract : Prop;
  witness_sha256_request_digest_refinement_contract : Prop;
  byte_source_code_store_contract : Prop;
}.

(* sail/host/memory.sail and sail/evm/machine.sail host_mem_*, hm_*,
   returndata_*, txin_*, txd_*, frame_code_*/frame_*, calldata_load_word,
   code_db_*, hs_*. *)
Record MemoryStackContract := {
  memory_state : Type;
  stack_state : Type;
  code_state : Type;
  calldata_state : Type;
  returndata_state : Type;

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
  calldata_view_contract : Prop;
  returndata_frame_contract : Prop;
  tx_input_view_contract : Prop;
  evm_stack_lifo_contract : Prop;
  frame_code_contract : Prop;
}.

(* sail/host/state.sail and sail/host/kernel.sail state-facing externs:
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
  code_db_contract : Prop;
  code_delegation_contract : Prop;
}.

Definition span := (Z * Z)%type.

(* sail/lib/mpt.sail witness-facing externs: nodedb_*. The node table plus the
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
  ssz_src_len_value (guest_input contract) =
    Z.of_nat (length (input_bytes (guest_input contract))).

Definition main_boundary (contract : GuestExternContract) : Prop :=
  staged_precompile_protocol_refines_ref contract.(guest_crypto) /\
  byte_source_resolve_contract contract.(guest_byte_sources) /\
  byte_source_keccak_refinement_contract contract.(guest_byte_sources) /\
  witness_sha256_request_digest_refinement_contract contract.(guest_byte_sources) /\
  byte_source_code_store_contract contract.(guest_byte_sources) /\
  memory_frame_lifo_contract contract.(guest_memory_stack) /\
  calldata_view_contract contract.(guest_memory_stack) /\
  returndata_frame_contract contract.(guest_memory_stack) /\
  evm_stack_lifo_contract contract.(guest_memory_stack) /\
  account_cache_update_contract contract.(guest_world_state) /\
  storage_cache_update_contract contract.(guest_world_state) /\
  storage_frame_commit_revert_contract contract.(guest_world_state) /\
  code_db_contract contract.(guest_world_state) /\
  node_db_insert_select_contract contract.(guest_witness_db) /\
  mpt_authenticated_frontier_contract contract.(guest_witness_db).

End EvmSailExternBoundary.
