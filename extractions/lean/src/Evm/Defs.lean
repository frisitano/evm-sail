import Sail
open PreSail

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

namespace Evm

namespace Defs

instance {α : Type} {β : α → Type} [DecidableEq α] [∀ a, BEq (β a)] :
    BEq (Sigma β) where
  beq left right :=
    if h : left.1 = right.1 then
      (h ▸ left.2) == right.2
    else
      false

instance {α : Type} {β : α → Type} [Inhabited α] [∀ a, Inhabited (β a)] :
    Inhabited (Sigma β) where
  default := ⟨default, default⟩

instance {α : Type} {β : α → Type} [Repr α] [∀ a, Repr (β a)] :
    Repr (Sigma β) where
  reprPrec value precedence :=
    reprPrec value.1 precedence ++ " => " ++ reprPrec value.2 precedence


abbrev bit := (BitVec 1)

abbrev bits (k_n : Int) := (BitVec k_n)

/- Type quantifiers: k_a : Type -/
inductive option (k_a : Type) where
  | Some (_ : k_a)
  | None (_ : Unit)
  deriving Inhabited, BEq, Repr
  open option

/-- An 8-bit byte. -/
abbrev byte := (BitVec 8)

/-- The EVM 256-bit machine word (YP §9.1).  A transparent range keeps the
mathematical subtype relation visible: narrower non-negative ranges can
be passed as words without a model-level conversion. -/
abbrev word := Nat

/-- A 20-byte account address (YP §4.1), in canonical protocol byte order. -/
abbrev address := (Vector byte 20)

/-- A KECCAK-256 / storage-key sized digest, in canonical protocol byte order. -/
abbrev b256 := (Vector byte 32)

/-- The common digest type used by trie, code, and block hashes. -/
abbrev hash := b256

/-- A host address operation's success flag and address result. -/
structure AddressResult where
  success : Bool
  address : address
  deriving BEq, Inhabited, Repr

/-- The type-level modulus of EVM-word arithmetic. -/
abbrev word_modulus : Int := (2 ^ 256)

/-- A bit count within one EVM word. -/
abbrev word_bit_count := Nat

/-- The largest account nonce admitted by EIP-2681. -/
abbrev account_nonce_bound : Int := (2 ^ 64 - 1)

/-- An account transaction-count nonce (EIP-2681). -/
abbrev account_nonce := Nat

/-- Largest value represented by an SSZ `uint64`. This is a wire-schema
constraint, sourced from the consensus `ExecutionPayload` container and
the Amsterdam `SszExecutionPayload`, rather than an implementation word
size. -/
abbrev ssz_uint_bound : Int := (2 ^ 64 - 1)

/-- An execution block number. Provenance: the execution-payload SSZ schema
declares `block_number: uint64`. The execution rules do not impose a
tighter supported-fork bound. -/
abbrev block_number := Nat

/-- An execution block timestamp in seconds. Provenance: the
execution-payload SSZ schema declares `timestamp: uint64`; no tighter
supported-fork protocol bound is applied. -/
abbrev block_timestamp := Nat

/-- Blob gas charged per blob, `2^17` (EIP-4844). -/
abbrev gas_per_blob_value : Int := (2 ^ 17)

/-- Named blob-schedule constants retained by the profile domains below. -/
abbrev blob_schedule_inactive_count : Int := 0

/-- Cancun's per-block blob target of 3 (EIP-4844). -/
abbrev cancun_blob_target_count : Int := 3

/-- Prague's per-block blob target of 6 (EIP-7691). -/
abbrev prague_blob_target_count : Int := 6

/-- The BPO1 per-block blob target of 10. -/
abbrev bpo1_blob_target_count : Int := 10

/-- The BPO2 per-block blob target of 14. -/
abbrev bpo2_blob_target_count : Int := 14

/-- Cancun's per-block blob maximum of 6 (EIP-4844). -/
abbrev cancun_blob_max_count : Int := 6

/-- Prague's per-block blob maximum of 9 (EIP-7691). -/
abbrev prague_blob_max_count : Int := 9

/-- The BPO1 per-block blob maximum of 15. -/
abbrev bpo1_blob_max_count : Int := 15

/-- The BPO2 per-block blob maximum of 21. -/
abbrev bpo2_blob_max_count : Int := 21

/-- Largest fee-update fraction in the supported blob schedules: BPO2's
`BLOB_BASE_FEE_UPDATE_FRACTION = 11684671` (EIP-8135). -/
abbrev blob_fee_update_fraction_bound : Int := 11684671

/-- A conservative exponent at which an EIP-4844 blob base fee can no longer
fit in the EVM's 256-bit word domain: `e^256 > 2^256`. -/
abbrev blob_fee_word_exponent_limit : Int := 256

/-- Whether a value is the inactive zero or one of the supported schedules'
per-block blob targets. -/
def blob_schedule_target_value (k_value : Int) : Prop :=
  k_value = blob_schedule_inactive_count ∨
  k_value = cancun_blob_target_count ∨
  k_value = prague_blob_target_count ∨
  k_value = bpo1_blob_target_count ∨ k_value = bpo2_blob_target_count

/-- Whether a value is the inactive zero or one of the supported schedules'
per-block blob maxima. -/
def blob_schedule_max_value (k_value : Int) : Prop :=
  k_value = blob_schedule_inactive_count ∨
  k_value = cancun_blob_max_count ∨
  k_value = prague_blob_max_count ∨
  k_value = bpo1_blob_max_count ∨ k_value = bpo2_blob_max_count

/-- A target selected by one of the blob schedules supported by the schema.
This is distinct from an observed blob count, which may be any value in
its contiguous range. -/
abbrev blob_schedule_target_count := Nat

/-- A maximum selected by one of the blob schedules supported by the schema. -/
abbrev blob_schedule_max_count := Nat

/-- The fork-selected maximum number of blobs carried by one transaction.
Zero denotes a profile before blob transactions activate. -/
def transaction_blob_limit_value (k_value : Int) : Prop :=
  k_value = blob_schedule_inactive_count ∨
  k_value = cancun_blob_max_count ∨ k_value = prague_blob_max_count

/-- A fork-selected per-transaction blob limit drawn from the supported
schedules; zero for profiles before blob transactions activate. -/
abbrev transaction_blob_limit := Nat

/-- A transaction blob count under one fork-selected transaction limit.
The limit is a finite-set profile parameter; the observed count occupies
the complete contiguous range beneath that selected limit. -/
abbrev transaction_blob_count (k_limit : Int) := Nat

/-- Blob gas used by one supported block. The existential count retains that
every value is exactly a multiple of `GAS_PER_BLOB`; profile-indexed
decoding applies the selected schedule's tighter range before values enter
this heterogeneous header domain. -/
abbrev blob_gas_used := Nat

/-- Blob gas contributed by one transaction. Profile-indexed blob-hash
decoding establishes the selected limit before this derived quantity is
widened to the common transaction-cost domain. -/
abbrev transaction_blob_gas := Nat

/-- Wire ceiling for the EIP-4844 `excess_blob_gas` header field.
Provenance: both the Deneb consensus `ExecutionPayload` and Amsterdam's
stateless SSZ payload encode the field as `uint64`. -/
abbrev excess_blob_gas_wire_bound : Int := (2 ^ 64 - 1)

/-- Union ceiling of the fork-indexed inductive equations retained by
`ProtocolProfile`. The expression deliberately uses the largest admitted
denominator and block growth rather than embedding a precomputed result. -/
abbrev excess_blob_gas_reachable_bound : Int := (256 * 11684671 + 21 * 2 ^ 17)

/-- The supported-fork reachable accumulated excess-blob-gas value. -/
abbrev excess_blob_gas_bound : Int := (256 * 11684671 + 21 * 2 ^ 17)

/-- The accumulated excess blob gas carried between headers (EIP-4844). -/
abbrev excess_blob_gas := Nat

/-- The positive denominator governing excess-blob-gas fee adjustment. -/
abbrev blob_fee_update_fraction := Nat

/-- Named EIP-4844/BPO fee-update fractions. -/
abbrev inactive_blob_fee_update_fraction : Int := 1

/-- Cancun's `BLOB_BASE_FEE_UPDATE_FRACTION` (EIP-4844). -/
abbrev cancun_blob_fee_update_fraction : Int := 3338477

/-- Prague's `BLOB_BASE_FEE_UPDATE_FRACTION` (EIP-7691). -/
abbrev prague_blob_fee_update_fraction : Int := 5007716

/-- The BPO1 blob-fee update fraction. -/
abbrev bpo1_blob_fee_update_fraction : Int := 8346193

/-- The BPO2 blob-fee update fraction (EIP-8135). -/
abbrev bpo2_blob_fee_update_fraction : Int := 11684671

/-- Whether a value is the inactive unit denominator or one of the supported
schedules' fee-update fractions. -/
def blob_schedule_fee_update_fraction_value (k_value : Int) : Prop :=
  k_value = inactive_blob_fee_update_fraction ∨
  k_value = cancun_blob_fee_update_fraction ∨
  k_value = prague_blob_fee_update_fraction ∨
  k_value = bpo1_blob_fee_update_fraction ∨ k_value = bpo2_blob_fee_update_fraction

/-- A fee-update denominator selected by one of the supported blob schedules.
Observed arithmetic still uses the broader positive denominator domain;
configuration itself can only choose these protocol constants. -/
abbrev blob_schedule_fee_update_fraction := Nat

/-- Fork-selected limits retained in the validated protocol profile. A fork
without a stricter transaction-gas cap admits the complete SSZ `uint64`
block-gas domain; the concrete header limit is applied separately. -/
abbrev pre_amsterdam_deployed_code_size_limit : Int := 24576

/-- Amsterdam's raised deployed-code size limit of 65,536 bytes. -/
abbrev amsterdam_deployed_code_size_limit : Int := 65536

/-- The zero initcode limit for profiles before EIP-3860 activates. -/
abbrev inactive_initcode_size_limit : Int := 0

/-- The EIP-3860 initcode size limit of 49,152 bytes. -/
abbrev pre_amsterdam_initcode_size_limit : Int := 49152

/-- Amsterdam's raised initcode size limit of 131,072 bytes. -/
abbrev amsterdam_initcode_size_limit : Int := 131072

/-- The EIP-7825 per-transaction gas cap of `2^24`. -/
abbrev eip7825_transaction_gas_limit : Int := (2 ^ 24)

/-- Whether a value is one of the supported deployed-code size limits. -/
def protocol_deployed_code_size_limit_value (k_value : Int) : Prop :=
  k_value = pre_amsterdam_deployed_code_size_limit ∨ k_value = amsterdam_deployed_code_size_limit

/-- The fork-selected deployed-code size limit. -/
abbrev protocol_deployed_code_size_limit := Nat

/-- Whether a value is the inactive zero or one of the supported initcode
size limits. -/
def protocol_initcode_size_limit_value (k_value : Int) : Prop :=
  k_value = inactive_initcode_size_limit ∨
  k_value = pre_amsterdam_initcode_size_limit ∨ k_value = amsterdam_initcode_size_limit

/-- The fork-selected initcode size limit. Zero denotes a profile before
EIP-3860 activates. -/
abbrev protocol_initcode_size_limit := Nat

/-- Whether a value is a supported per-transaction total-gas ceiling: the
EIP-7825 cap or the unrestricted SSZ block-gas domain. -/
def protocol_transaction_total_gas_limit_value (k_value : Int) : Prop :=
  k_value = eip7825_transaction_gas_limit ∨ k_value = ssz_uint_bound

/-- The fork-selected per-transaction total-gas ceiling. -/
abbrev protocol_transaction_total_gas_limit := Nat

/-- Whether a value is a supported per-transaction regular-gas ceiling: the
EIP-7825 cap or the unrestricted SSZ block-gas domain. -/
def protocol_transaction_regular_gas_limit_value (k_value : Int) : Prop :=
  k_value = eip7825_transaction_gas_limit ∨ k_value = ssz_uint_bound

/-- The fork-selected per-transaction regular-gas ceiling. -/
abbrev protocol_transaction_regular_gas_limit := Nat

/-- Largest chain identifier admitted by the typed-transaction wire decoder. -/
abbrev chain_identifier_bound : Int := (2 ^ 64 - 1)

/-- A chain identifier. Typed-transaction chain identifiers and the stateless
chain configuration are decoded as unsigned 64-bit integers. -/
abbrev chain_identifier := Nat

/-- A beacon-chain slot number. Provenance: EIP-7843 and Amsterdam's
stateless SSZ payload declare this field as `uint64`. -/
abbrev slot_number := Nat

/-- An EIP-4895 withdrawal index, encoded as SSZ `uint64`. -/
abbrev withdrawal_index := Nat

/-- An EIP-4895 validator index, encoded as SSZ `uint64`. -/
abbrev validator_index := Nat

/-- An EIP-4895 withdrawal amount in gwei, encoded as SSZ `uint64`. -/
abbrev withdrawal_amount := Nat

/-- An eight-byte unsigned integer read from the stateless-input SSZ schema.
This is a transport type; decoders widen or validate it into the semantic
field type at the container boundary. -/
abbrev ssz_uint := Nat

/-- A container-relative offset carried by an SSZ `uint32`. -/
abbrev ssz_offset := Nat

/-- An index into a table of four-byte SSZ offsets. -/
abbrev ssz_offset_index := Nat

/-- The EVM call-frame depth ceiling (Yellow Paper `I_e`). -/
abbrev call_depth_limit : Int := 1024

/-- The nesting depth of an execution frame. -/
abbrev frame_depth := Nat

/-- The immediate-byte width of a PUSH instruction. -/
abbrev push_width := Nat

/-- The number of words on an operand stack. -/
abbrev operand_stack_height := Nat

/-- A zero-based index from the top of the operand stack. -/
abbrev stack_index := Nat

/-- The operand-stack cursor for the active frame, threaded by value through
the interpreter in the state-passing convention and held in the
`stack_top` frame register at frame boundaries. `storage` is an opaque
host coordinate while `height` is the semantic stack height. Keeping the
height in the cursor makes stack validation independent of the host stack
representation. Optimized C refines `storage` to a native `u256 *`. -/
structure StackPointer where
  storage : (BitVec 64)
  height : operand_stack_height
  deriving BEq, Inhabited, Repr

/-- The number of slots an operand-stack cursor moves in one advance or
retreat. -/
abbrev stack_slot_count := Nat

/-- A nonzero operand-stack position used by DUP and SWAP. -/
abbrev stack_operation_index := Nat

/-- The one-based deep-stack index decoded by EIP-8024 DUPN and SWAPN. -/
abbrev deep_stack_index := Nat

/-- The number of indexed topics attached to one log. -/
abbrev log_topic_count := Nat

/-- The parity bit used by transaction signatures. -/
abbrev y_parity := Nat

/-- A depth in a 64-level binary Merkle tree. -/
abbrev merkle_depth := Nat

/-- An EVM instruction byte. -/
abbrev opcode := Nat

/-- The closed first-order selector for the precompile catalog. Availability,
gas pricing, and execution are separate interpreters of this identifier so
their protocol equations remain explicit without function-valued records. -/
inductive PrecompileId where | NotPrecompile | Ecrecover | Sha256 | Ripemd160 | Identity | Modexp | Bn254Add | Bn254Mul | Bn254Pairing | Blake2f | KzgPointEvaluation | BlsG1Add | BlsG1Msm | BlsG2Add | BlsG2Msm | BlsPairing | BlsMapFpToG1 | BlsMapFp2ToG2 | P256Verify
  deriving BEq, Inhabited, Repr
  open PrecompileId

/-- The quantity alias carried by the precompile interpreters for the closed
selector above. -/
abbrev precompile_id := PrecompileId

/-- The round count supplied to the BLAKE2 compression precompile. -/
abbrev blake2_rounds := Nat

/-- A fixed-point discount factor used by BLS precompile pricing. -/
abbrev bls_discount := Nat

/-- A bit position in the 2048-bit log bloom. -/
abbrev bloom_bit_index := Nat

/-- A 64-bit word position in the log bloom. -/
abbrev bloom_u64_index := Nat

/-- A bit position within a log-bloom u64. -/
abbrev bloom_u64_bit := Nat

/-- The number of authenticated ancestor hashes available to `BLOCKHASH`. -/
abbrev ancestor_hash_count := Nat

/-- An index into the 256 most recent ancestor block hashes. -/
abbrev ancestor_index := Nat

/-- Default capacity of an implementation-owned byte arena. -/
abbrev default_host_region_bound : Int := (2 ^ 32 - 1)

/-- Immutable stateless-input envelope capacity. -/
abbrev stateless_input_region_bound : Int := (2 ^ 32 - 1)

/-- Executor scratch-arena capacity. -/
abbrev scratch_region_bound : Int := (2 ^ 32 - 1)

/-- Shared per-frame EVM-memory arena capacity. -/
abbrev memory_region_bound : Int := (2 ^ 32 - 1)

/-- Content-addressed executable-code arena capacity. -/
abbrev code_region_bound : Int := (2 ^ 32 - 1)

/-- Retained transaction-log data arena capacity. -/
abbrev log_data_region_bound : Int := (2 ^ 32 - 1)

/-- Guest output-buffer capacity. -/
abbrev output_region_bound : Int := (2 ^ 32 - 1)

/-- Whether an offset/length pair is representable by a generic host
interface. Region-bearing values use the more specific predicates below. -/
def host_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ default_host_region_bound

/-- Common bound for relative source coordinates used by generic cursor
operations. Nominal slices retain their region-specific invariant. -/
def source_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ default_host_region_bound

/-- Whether an offset/length pair is contained by the immutable
stateless-input envelope. -/
def stateless_input_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ stateless_input_region_bound

/-- Whether an offset/length pair is contained by the executor scratch
arena. -/
def scratch_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ scratch_region_bound

/-- Whether an offset/length pair is contained by the shared EVM-memory
arena. -/
def memory_region_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ memory_region_bound

/-- Whether an offset/length pair is contained by the executable-code
arena. -/
def code_region_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ code_region_bound

/-- Whether an offset/length pair is contained by retained log-data
storage. -/
def log_data_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ log_data_region_bound

/-- Whether an offset/length pair is contained by the guest output
region. -/
def output_region_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ output_region_bound

/-- Whether one host byte quantity is representable. Unlike
`host_valid_range`, this does not assert that two quantities can be added
without overflow. -/
def host_valid_access (k_value : Int) : Prop :=
  0 ≤ k_value ∧ k_value ≤ default_host_region_bound

/-- Whether one relative source coordinate is representable. -/
def source_valid_length (k_value : Int) : Prop :=
  0 ≤ k_value ∧ k_value ≤ default_host_region_bound

/-- A position representable by a generic host byte-store interface. -/
abbrev host_access := Nat

/-- An absolute byte position in a named source region. -/
abbrev source_pointer := Nat

/-- A byte length or regular-layout count derived from a source region. -/
abbrev source_length := Nat

/-- A coordinate in the immutable stateless-input envelope. -/
abbrev stateless_input_pointer := Nat

/-- A length in the immutable stateless-input envelope. -/
abbrev stateless_input_length := Nat

/-- A coordinate in the executor scratch arena. -/
abbrev scratch_pointer := Nat

/-- A length in the executor scratch arena. -/
abbrev scratch_length := Nat

/-- A coordinate in retained log-data storage. -/
abbrev log_data_pointer := Nat

/-- A length in retained log-data storage. -/
abbrev log_data_length := Nat

/-- A coordinate in the guest output region. -/
abbrev output_pointer := Nat

/-- A length in the guest output region. -/
abbrev output_length := Nat

/-- A relative coordinate in calldata. Calldata may borrow either immutable
stateless input or the active EVM-memory frame, so this is the semantic
boundary for that explicit sum type rather than a generic host quantity. -/
abbrev calldata_pointer := Nat

/-- A byte length in calldata, independent of which calldata variant owns the
bytes. -/
abbrev calldata_length := Nat

/-- The number of bytes loaded into one EVM word. -/
abbrev word_byte_count := Nat

/-- A cursor into a transaction-local journal-owned worklist. -/
abbrev journal_cursor := Nat

/-- The monotonically increasing storage incarnation owned by an account.
Generation zero is reserved for the absence of a generation. -/
abbrev storage_generation := Nat

/-- An absolute byte position in the shared EVM-memory arena. -/
abbrev memory_base := Nat

/-- A materialized length or allocation size in the EVM-memory arena. -/
abbrev memory_length := Nat

/-- The active EVM frame's exact relative byte high-water mark. It is carried
beside the frame's absolute `memory_base`; the host retains no hidden
frame coordinate or lifecycle state. -/
abbrev memory_height := memory_length

/-- Largest possible initial/live gas value from the SSZ-backed block-gas
domain. Fork and header limits may make this ceiling smaller; this union
ceiling is sufficient to prove one representation valid for every
supported profile. Runtime affordability continues to use remaining gas. -/
abbrev memory_expansion_proof_gas_ceiling : Int := (2 ^ 64 - 1)

/-- First memory word count whose quadratic Yellow Paper charge alone exceeds
`memory_expansion_proof_gas_ceiling`:
`(2^37)^2 / 512 = 2^65 > 2^64 - 1`. This is a derived proof threshold,
not a runtime memory or host-allocation limit. -/
abbrev memory_unaffordable_word_threshold : Int := (2 ^ 37)

/-- Largest whole-word endpoint which can precede that threshold. -/
abbrev memory_affordable_word_bound : Int := (2 ^ 37 - 1)

/-- Largest byte endpoint retained by optimized expansion planning after the
maximum-gas proof. Each attempted expansion is still checked against its
exact live `gas_remaining`. -/
abbrev memory_affordable_byte_bound : Int := (32 * (2 ^ 37 - 1))

/-- Largest word count representable by the current EVM-memory region. -/
abbrev memory_region_word_bound : Int := (2 ^ 27)

/-- Whether a word count may be presented to the cumulative memory-cost
equation. This follows from the independently configured host-region
boundary; the exact live-gas check remains the protocol affordability
rule. -/
def memory_cost_input (k_words : Int) : Prop := 0 ≤ k_words

/-- The invariant carried by every canonical EVM memory range. -/
def memory_valid_range (k_off : Int) (k_len : Int) : Prop :=
  0 ≤ k_off ∧ 0 ≤ k_len ∧ (k_off + k_len) ≤ memory_region_bound

/-- A memory endpoint which may be presented to the expansion-cost equation.
The canonical specification admits every mathematical byte position;
executable splices may strengthen this representation boundary after
proving that every excluded endpoint is unaffordable. -/
def memory_expansion_endpoint (k_required : Int) : Prop := 0 ≤ k_required

/-- Relates a canonical memory operand to its exclusive endpoint. A zero-size
operand accesses no memory, so both its retained range and endpoint are
canonicalized to zero. -/
def memory_access_relation (k_off : Int) (k_len : Int) (k_required : Int) : Prop :=
  (memory_valid_range k_off k_len) ∧
  (memory_expansion_endpoint k_required) ∧
  k_len = 0 ∧ k_off = 0 ∧ k_required = 0 ∨ 0 < k_len ∧ k_required = (k_off + k_len)

/-- The indexed fields of an EVM memory range. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (memory_valid_range k_off k_len) -/
structure MemoryRangeFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def MemoryRangeFields.off {k_off : Nat} {k_len : Nat} (_ : (MemoryRangeFields k_off k_len))
  : Nat :=
  k_off
@[simp] def MemoryRangeFields.len {k_off : Nat} {k_len : Nat} (_ : (MemoryRangeFields k_off k_len))
  : Nat :=
  k_len

/-- A memory range retaining its offset, length, and containment proof. -/
abbrev MemoryRange :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))

/-- One logical EVM memory operand together with the exact endpoint which
contributes to the shared expansion plan. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, k_required : Nat, (memory_access_relation k_off k_len k_required) -/
structure MemoryAccessFields (k_off : Nat) (k_len : Nat) (k_required : Nat) where
  range : (MemoryRangeFields k_off k_len)
  deriving BEq, Inhabited, Repr

@[simp] def MemoryAccessFields.requested_height {k_off : Nat} {k_len : Nat} {k_required : Nat}
  (_ : (MemoryAccessFields k_off k_len k_required)) : Nat :=
  k_required

/-- A memory operand retaining its range/endpoint relationship existentially. -/
abbrev MemoryAccess :=
  (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) =>
  (Sigma fun (k_required : Nat) => (MemoryAccessFields k_off k_len k_required))))

/-- An absolute byte position in the code arena. -/
abbrev code_pointer := Nat

/-- A contract-code length. -/
abbrev code_length := Nat

/-- The representation invariant required of executable code, including
enough cursor headroom for a complete PUSH32 immediate. -/
def code_valid_length (k_len : Int) : Prop := 0 ≤ k_len ∧ (k_len + 32) ≤ code_region_bound

/-- An opcode-aligned scan cursor: a code position that still carries the
PUSH32 immediate headroom, so reading past the current opcode stays in
the code region. -/
abbrev code_scan_position := Nat

/-- The maximum block gas limit admitted by the execution-payload SSZ
`uint64` field. Provenance: consensus `ExecutionPayload.gas_limit` and
Amsterdam `SszExecutionPayload.gas_limit`. -/
abbrev block_gas_limit_bound : Int := (2 ^ 64 - 1)

/-- EIP-7928 charges one BAL item against each 2,000 units of block gas. -/
abbrev block_access_list_item_gas : Int := 2000

/-- Available gas in a running EVM frame. Every admitted transaction gas
limit originates in the execution payload's SSZ `uint64` gas-limit
domain, and child frames can only receive gas from their parent. -/
abbrev gas := Nat

/-- Representation invariant for a value copied from the live frame gas
counter. -/
def live_gas_valid (k_value : Int) : Prop := 0 ≤ k_value ∧ k_value ≤ (2 ^ 64 - 1)

/-- Gas that can be restored to a particular live counter without exceeding
the admitted `uint64` gas domain. The index is type-level only; optimized
C represents the value as the same native gas scalar. -/
abbrev gas_credit (k_available : Int) := Nat

/-- State-gas spill that can be restored to the indicated execution gas. -/
abbrev state_gas_spill_credit (k_available : Int) := Nat

/-- Amsterdam's per-frame state-gas reservoir. The transaction's total gas
allowance remains in the execution payload's `uint64` domain; only the
regular-gas portion and state-gas spill into that portion are capped by
EIP-7825. -/
abbrev state_gas := Nat

/-- Child state gas that can be restored to the indicated parent reservoir. -/
abbrev state_gas_credit (k_available : Int) := Nat

/-- Gas supplied by a transaction before fork-specific validation. Any value
above the execution payload's SSZ `uint64` block-gas-limit domain cannot be
admitted, so the RLP boundary rejects it before constructing a
transaction. -/
abbrev transaction_gas := Nat

/-- A transient computed charge after its affordability or structural bound
has been established. Unaffordable larger computations are represented by
`GasCharge.affordable = false` rather than materialized as a cost. -/
abbrev gas_cost := Nat

/-- Exact exclusive byte endpoint used by the canonical memory-expansion
equation. The optimized C splice may replace endpoints beyond its
materializable arena with one proven-unaffordable sentinel. -/
abbrev memory_required_endpoint := Nat

/-- Exact incremental memory cost. Optimized C retains one additional
sentinel value above every representable live-gas value so affordability
can be decided without overflowing the native cost representation. -/
abbrev memory_expansion_charge := Nat

/-- One affordability decision together with its bounded cost. The payload is
meaningful only when `affordable` is true. -/
structure GasCharge where
  affordable : Bool
  cost : gas_cost
  deriving BEq, Inhabited, Repr

/-- Intermediate MODEXP affordability factors, bounded by live gas and the
at-most-255-bit exponent-head contribution. -/
abbrev modexp_factor := Nat

/-- Products of two bounded MODEXP affordability factors. -/
abbrev modexp_product := Nat

/-- Long-exponent byte counts after the fork-specific affordability check. -/
abbrev modexp_osaka_extra := Nat

/-- Long-exponent byte counts used by the pre-Osaka MODEXP gas formula. -/
abbrev modexp_pre_osaka_extra := Nat

/-- EIP-2537's base-cost/discount product. -/
abbrev bls_msm_coefficient := Nat

/-- Exact EIP-2537 MSM products before their affordability check. -/
abbrev bls_msm_product := Nat

/-- EIP-2537's bounded remainder product before division by 1000. -/
abbrev bls_msm_tail_product := Nat

/-- Linear precompile multiplier before adding the fixed base charge. -/
abbrev linear_gas_variable_product := Nat

/-- Exact linear precompile cost before its live-gas affordability check. -/
abbrev linear_gas_product := Nat

/-- The EIP-7825 per-transaction execution-gas limit reused by EIP-8037's
regular-gas pool. -/
abbrev transaction_execution_gas_limit_value : Int := (2 ^ 24)

/-- Gas admitted to Amsterdam's regular execution pool after applying the
EIP-7825 per-transaction cap. -/
abbrev transaction_execution_gas := Nat

/-- Execution gas temporarily consumed by Amsterdam state charges. EIP-8037
draws spill only from the regular-gas pool, which is capped by EIP-7825. -/
abbrev state_gas_spill := Nat

/-- A block header's gas limit. Its SSZ execution-payload field is `uint64`;
`GASLIMIT` widens this bounded natural into an EVM word when it pushes the
value onto the stack (Yellow Paper equation 147). -/
abbrev block_gas_limit := Nat

/-- Gas consumed by a block header. Provenance: the execution-payload SSZ
schema declares `gas_used: uint64`. The separate payload-validity rule
`gas_used <= gas_limit` is checked when the header is admitted. -/
abbrev block_gas := Nat

/-- Gas accumulated in execution-order receipts. Before Amsterdam this is
bounded by the block gas limit. Amsterdam receipts account for regular and
state gas together, so the execution profile supplies the tighter
header-dependent bound. -/
abbrev receipt_cumulative_gas := Nat

/-- Whether a value lies in the receipt cumulative-gas domain. -/
def receipt_cumulative_gas_value_valid (k_value : Int) : Prop :=
  0 ≤ k_value ∧ k_value ≤ (2 * block_gas_limit_bound)

/-- A fixed gas-schedule value used as an opcode or transaction base cost. -/
abbrev gas_constant := Nat

/-- Absolute lifecycle bound for the signed refund accumulator. -/
abbrev gas_refund_bound : Int := (199 * (2 ^ 64 - 1))

/-- The signed transaction refund accumulator before capping. -/
abbrev gas_refund := Int

/-- EIP-8037's `SSTORE` storage-clear refund. -/
abbrev amsterdam_storage_clear_refund : Int := 12480

/-- One EIP-8037 `SSTORE` transition's signed refund delta. A transition either
establishes the storage-clear refund or withdraws the one an earlier write
in the same transaction established, so it moves the accumulator by at most
that refund in either direction. The accumulator itself is [gas_refund][],
which this sits well inside. The pre-Amsterdam schedule prices its
transitions from its own constants and is bounded separately. -/
abbrev gas_refund_delta := Int

/-- Net state gas consumed by one execution frame. A credit can make this
negative until transaction settlement clamps the block-level value at
zero. The bounds follow from subtracting two live `uint64` counters and
adding at most one EIP-7825 regular-pool spill. -/
abbrev frame_state_gas_delta := Int

/-- Combined state-gas delta for the Amsterdam authorization and execution
frames. -/
abbrev state_gas_delta := Int

/-- State gas after adding the intrinsic transaction charge and before the
block-level non-negative/u64 checks. -/
abbrev transaction_state_gas_delta := Int

/-- A non-negative transaction state-gas total after settlement has rejected
a negative signed delta. -/
abbrev transaction_state_gas_used := Nat

/-- The pre-London refund divisor: refunds are capped at `gas_used / 2`. -/
abbrev pre_london_refund_divisor : Int := 2

/-- The London refund divisor: EIP-3529 caps refunds at `gas_used / 5`. -/
abbrev post_london_refund_divisor : Int := 5

/-- Whether a value is one of the two protocol refund divisors. -/
def transaction_refund_divisor_value (k_value : Int) : Prop :=
  k_value = pre_london_refund_divisor ∨ k_value = post_london_refund_divisor

/-- The fork-selected transaction-refund divisor: 2 before London, 5 after. -/
abbrev transaction_refund_divisor := Nat

/-- A range in the immutable stateless-input envelope. Its source is carried
by the nominal type rather than a runtime field. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (stateless_input_valid_range k_off k_len) -/
structure StatelessInputSliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def StatelessInputSliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (StatelessInputSliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def StatelessInputSliceFields.len {k_off : Nat} {k_len : Nat}
  (_ : (StatelessInputSliceFields k_off k_len)) : Nat :=
  k_len

/-- A stateless-input range with its coordinate and length packed
existentially. -/
abbrev StatelessInputSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))

/-- A stateless-input range of exactly `'required` bytes. -/
abbrev StatelessInputSliceLength (k_required : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))

/-- A stateless-input range of at least `'minimum` bytes. -/
abbrev StatelessInputSliceAtLeast (k_minimum : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))

/-- A stateless-input range of at most `'maximum` bytes. -/
abbrev StatelessInputSliceAtMost (k_maximum : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))

/-- A range in the executor's reusable scratch arena. Its source is carried by
the nominal type rather than a runtime field. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (scratch_valid_range k_off k_len) -/
structure ScratchSliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def ScratchSliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (ScratchSliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def ScratchSliceFields.len {k_off : Nat} {k_len : Nat}
  (_ : (ScratchSliceFields k_off k_len)) : Nat :=
  k_len

/-- A scratch-arena range with its coordinate and length packed
existentially. -/
abbrev ScratchSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))

/-- A scratch-arena range of exactly `'required` bytes. -/
abbrev ScratchSliceLength (k_required : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))

/-- A scratch-arena range of at least `'minimum` bytes. -/
abbrev ScratchSliceAtLeast (k_minimum : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))

/-- A borrowed range in the shared EVM-memory arena. Its coordinate is absolute
within the arena (an offset in the standard ABI and a stable pointer in the
optimized ABI), so calldata borrowed from a suspended parent remains valid
while a child frame is active. The machine carries only `memory_height`;
this pointer-bearing value exists at derived-view boundaries. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (memory_region_valid_range k_off k_len) -/
structure EvmMemorySliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def EvmMemorySliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (EvmMemorySliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def EvmMemorySliceFields.len {k_off : Nat} {k_len : Nat}
  (_ : (EvmMemorySliceFields k_off k_len)) : Nat :=
  k_len

/-- An EVM-memory range with its coordinate and length packed
existentially. -/
abbrev EvmMemorySlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))

/-- An EVM-memory range of exactly `'required` bytes. -/
abbrev EvmMemorySliceLength (k_required : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))

/-- An EVM-memory range of at least `'minimum` bytes. -/
abbrev EvmMemorySliceAtLeast (k_minimum : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))

/-- A range in the content-addressed executable-code arena. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (code_region_valid_range k_off k_len) -/
structure CodeRegionSliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def CodeRegionSliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (CodeRegionSliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def CodeRegionSliceFields.len {k_off : Nat} {k_len : Nat}
  (_ : (CodeRegionSliceFields k_off k_len)) : Nat :=
  k_len

/-- A code-region range with its coordinate and length packed
existentially. -/
abbrev CodeRegionSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))

/-- A range in the transaction-log data arena. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (log_data_valid_range k_off k_len) -/
structure LogDataSliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def LogDataSliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (LogDataSliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def LogDataSliceFields.len {k_off : Nat} {k_len : Nat}
  (_ : (LogDataSliceFields k_off k_len)) : Nat :=
  k_len

/-- A log-data range with its coordinate and length packed existentially. -/
abbrev LogDataSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))

/-- A log-data range of exactly `'required` bytes. -/
abbrev LogDataSliceLength (k_required : Int) :=
  (Sigma fun (k_off : Nat) => (LogDataSliceFields k_off k_required))

/-- A range in the frame-output buffer. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (output_region_valid_range k_off k_len) -/
structure OutputSliceFields (k_off : Nat) (k_len : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def OutputSliceFields.bytes {k_off : Nat} {k_len : Nat}
  (_ : (OutputSliceFields k_off k_len)) : Nat :=
  k_off
@[simp] def OutputSliceFields.len {k_off : Nat} {k_len : Nat} (_ : (OutputSliceFields k_off k_len))
  : Nat :=
  k_len

/-- A frame-output range with its coordinate and length packed
existentially. -/
abbrev OutputSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))

/-- Calldata is either the immutable top-level transaction input or a frozen
range of the suspended caller's memory. The variants state the only two
protocol-valid provenances instead of exposing the host's region enum. -/
inductive CalldataSlice where
  | InputCalldata (_ : StatelessInputSlice)
  | MemoryCalldata (_ : EvmMemorySlice)
  deriving Inhabited, BEq, Repr
  open CalldataSlice

/-- A log payload retained by the host: either an existing byte slice or the
canonical big-endian bytes of one EVM word used by system logs. -/
inductive LogData where
  | LogDataMemory (_ : EvmMemorySlice)
  | LogDataWord (_ : word)
  deriving Inhabited, BEq, Repr
  open LogData

/-- Exceptional halts (YP §9.4.2): each consumes all remaining gas and
reverts the frame's state changes. -/
inductive ExceptionKind where | StackUnderflow | StackOverflow | OutOfGas | InvalidOpcode | InvalidJump | StaticViolation | CallDepthExceeded | InsufficientBalance | WriteProtection | InitCodeTooLarge | NonceOverflow | AddressCollision
  deriving BEq, Inhabited, Repr
  open ExceptionKind

/-- The reason a block fails validation; one variant per violated
block-validity rule. -/
inductive FatalError where | InvalidConfig | HeaderChainBroken | RlpDecode | InvalidSignature | InvalidGasLimit | GasUsedExceedsLimit | BlobGasLimitExceeded | ExecutionInvalid | InvalidGasUsed | InvalidBlobGasUsed | InvalidExcessBlobGas | InvalidStateRoot | InvalidReceiptsRoot | InvalidLogsBloom | InvalidBlockHash | InvalidParentHash | BlockAccessListTooLarge | InvalidBlockAccessList | InvalidExecutionRequests | WitnessDeficient | NumericOverflow
  deriving BEq, Inhabited, Repr
  open FatalError

/-- Stable identifier for the validation stage that raised a block error.
This diagnostic metadata is a bounded integer, not a protocol bitvector. -/
abbrev validation_stage := Nat

/-- Ordinary frame stops. Only `RETURN` and `REVERT` produce output bytes. -/
inductive HaltKind where
  | HaltStop (_ : Unit)
  | HaltReturn (_ : OutputSlice)
  | HaltRevert (_ : OutputSlice)
  | HaltSelfDestruct (_ : Unit)
  deriving Inhabited, BEq, Repr
  open HaltKind

/-- Per-frame execution status: running, halted normally, or exceptionally
halted. -/
inductive FrameStatus where
  | Running (_ : Unit)
  | Halted (_ : HaltKind)
  | Exceptional (_ : ExceptionKind)
  deriving Inhabited, BEq, Repr
  open FrameStatus

/-- Maximum transactions in the execution-payload SSZ list. Provenance:
Bellatrix `MAX_TRANSACTIONS_PER_PAYLOAD` and Amsterdam
`SszExecutionPayload.transactions`. -/
abbrev transaction_count_bound : Int := (2 ^ 20)

/-- Maximum withdrawals in the execution-payload SSZ list. Provenance:
Capella `MAX_WITHDRAWALS_PER_PAYLOAD` and Amsterdam
`SszExecutionPayload.withdrawals`. -/
abbrev withdrawal_count_bound : Int := (2 ^ 4)

/-- Maximum blob commitments in the stateless-input SSZ list. -/
abbrev blob_commitment_count_bound : Int := (2 ^ 12)

/-- Maximum witness trie nodes in the stateless-input SSZ list. -/
abbrev witness_node_count_bound : Int := (2 ^ 22)

/-- Maximum witness code entries in the stateless-input SSZ list. -/
abbrev witness_code_count_bound : Int := (2 ^ 18)

/-- Maximum witness headers in the stateless-input SSZ list. -/
abbrev witness_header_count_bound : Int := (2 ^ 8)

/-- Maximum witnessed public keys in the stateless-input SSZ list. -/
abbrev public_key_count_bound : Int := (2 ^ 15)

/-- Maximum deposit requests in their stateless-input SSZ list. -/
abbrev deposit_request_count_bound : Int := (2 ^ 13)

/-- Maximum withdrawal requests in their stateless-input SSZ list. -/
abbrev withdrawal_request_count_bound : Int := (2 ^ 4)

/-- Maximum consolidation requests in their stateless-input SSZ list. -/
abbrev consolidation_request_count_bound : Int := (2 ^ 1)

/-- Maximum builder deposit requests in their stateless-input SSZ list. -/
abbrev builder_deposit_request_count_bound : Int := (2 ^ 6)

/-- Maximum builder exit requests in their stateless-input SSZ list. -/
abbrev builder_exit_request_count_bound : Int := (2 ^ 4)

/-- Maximum byte length of execution-payload extra data. Provenance:
consensus `MAX_EXTRA_DATA_BYTES` and Amsterdam
`SszExecutionPayload.extra_data`. -/
abbrev extra_data_length_bound : Int := (2 ^ 5)

/-- Maximum byte length of one encoded transaction envelope. Provenance:
Amsterdam `MAX_BYTES_PER_TRANSACTION` in
`SszExecutionPayload.transactions`. -/
abbrev transaction_length_bound : Int := (2 ^ 30)

/-- Maximum byte length of the block access list. Provenance: Amsterdam
`SszExecutionPayload.block_access_list` uses
`ByteList[MAX_BYTES_PER_TRANSACTION]`. -/
abbrev block_access_list_length_bound : Int := (2 ^ 30)

/-- Maximum byte length of one witnessed trie node. -/
abbrev witness_node_length_bound : Int := (2 ^ 10)

/-- Maximum byte length of one witnessed code entry. -/
abbrev witness_code_length_bound : Int := (2 ^ 16)

/-- Maximum byte length of one witnessed parent header. -/
abbrev witness_header_length_bound : Int := (2 ^ 10)

/-- Number of transactions in a schema-valid execution payload. -/
abbrev transaction_count := Nat

/-- Zero-based transaction position in a non-empty execution payload. -/
abbrev transaction_index := Nat

/-- EIP-7928 change position: pre-execution system calls use zero,
transactions use their one-based position, and post-execution system calls
use `transaction_count + 1`. -/
abbrev block_access_index := Nat

/-- A reference to an SSZ list: its byte span and schema-bounded element
count. -/
/- Type quantifiers: k_maximum : Nat, (source_valid_length k_maximum) -/
structure BoundedSszListRef (k_maximum : Nat) where
  bytes : StatelessInputSlice
  count : Nat
  max_item_length : source_length
  deriving BEq, Inhabited, Repr

/-- A sequential cursor over a variable-element SSZ list. -/
/- Type quantifiers: k_maximum : Nat, (source_valid_length k_maximum) -/
structure BoundedSszListCursor (k_maximum : Nat) where
  items : (BoundedSszListRef k_maximum)
  index : Nat
  current : source_pointer
  deriving BEq, Inhabited, Repr

/-- A schema-bounded source reference to encoded transaction envelopes. -/
abbrev TransactionListRef := (BoundedSszListRef (2 ^ 20))

/-- A sequential cursor over encoded transaction envelopes. -/
abbrev TransactionListCursor := (BoundedSszListCursor (2 ^ 20))

/-- A schema-bounded source reference to withdrawals. -/
abbrev WithdrawalListRef := (BoundedSszListRef (2 ^ 4))

/-- A sequential cursor over withdrawals. -/
abbrev WithdrawalListCursor := (BoundedSszListCursor (2 ^ 4))

/-- A schema-bounded source reference to witnessed trie nodes. -/
abbrev WitnessNodeListRef := (BoundedSszListRef (2 ^ 22))

/-- A sequential cursor over witnessed trie nodes. -/
abbrev WitnessNodeListCursor := (BoundedSszListCursor (2 ^ 22))

/-- A schema-bounded source reference to witnessed code entries. -/
abbrev WitnessCodeListRef := (BoundedSszListRef (2 ^ 18))

/-- A sequential cursor over witnessed code entries. -/
abbrev WitnessCodeListCursor := (BoundedSszListCursor (2 ^ 18))

/-- A schema-bounded source reference to witnessed parent headers. -/
abbrev WitnessHeaderListRef := (BoundedSszListRef (2 ^ 8))

/-- A sequential cursor over witnessed parent headers. -/
abbrev WitnessHeaderListCursor := (BoundedSszListCursor (2 ^ 8))

/-- A reference to a code's completed `JUMPDEST` analysis; zero denotes
the empty bitmap. -/
abbrev jump_table_index := Nat

/-- A source-backed executable byte span. Its length carries the separate
representation invariant needed by program-counter arithmetic; this is
not a protocol deployment-size limit. -/
abbrev CodeSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))

/-- The closed family of Amsterdam opcodes whose instruction encoding carries
one validity-sensitive immediate byte. Keeping this classification in the
specification lets instruction fetch and PUSH-aware code analysis share
one dispatch without introducing a function-valued decoder. -/
inductive DeepStackOperation where | DeepStackDuplicate | DeepStackSwap | DeepStackExchange | NotDeepStackOperation
  deriving BEq, Inhabited, Repr
  open DeepStackOperation

/-- Executable code: its byte address, length, and resolved `JUMPDEST` table.
The flat dependent record keeps the byte address and length relationship
explicit while allowing optimized C to represent both addresses directly
as pointers. The interpreter saves and restores all three together; the
code hash remains the stable code-DB key. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (code_region_valid_range k_off k_len) ∧
  (code_valid_length k_len) -/
structure CodeFields (k_off : Nat) (k_len : Nat) where
  jumpdests : jump_table_index
  deriving BEq, Inhabited, Repr

@[simp] def CodeFields.bytes {k_off : Nat} {k_len : Nat} (_ : (CodeFields k_off k_len)) : Nat :=
  k_off
@[simp] def CodeFields.len {k_off : Nat} {k_len : Nat} (_ : (CodeFields k_off k_len)) : Nat :=
  k_len

/-- Existential executable-code value whose concrete byte address and length
remain correlated inside `CodeFields`. -/
abbrev Code := (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))

/-- Encoded pairing-check status: values `0`/`1` are malformed input and
values `2`/`3` are valid input with a false/true pairing result. -/
abbrev pairing_check_result := Nat

/-- The complete containment invariant for an RLP field reference. `source`
is normalized to the complete encoded item. RLP content is its suffix, so
the content offset is derived as `source.len - content_len`. -/
def rlp_field_ref_valid (k_source_off : Int) (k_source_len : Int) (k_content_len : Int) : Prop :=
  (source_valid_range k_source_off k_source_len) ∧
  0 ≤ k_content_len ∧ k_content_len ≤ k_source_len

/-- The result of decoding one complete field at the head of a cursor. In
addition to the field's own validity, the complete encoding is non-empty
and contained by the cursor. The field's `source.len` is therefore a
witness for the amount a caller may subsequently advance. -/
def rlp_decoded_item_valid
  (k_source_off : Int) (k_source_len : Int) (k_full_len : Int) (k_content_len : Int) : Prop :=
  (rlp_field_ref_valid k_source_off k_full_len k_content_len) ∧
  0 < k_full_len ∧ k_full_len ≤ k_source_len

/-- A positive amount proven to be contained by the cursor it consumes. -/
def rlp_cursor_advance_valid (k_source_len : Int) (k_consumed : Int) : Prop :=
  0 < k_consumed ∧ k_consumed ≤ k_source_len

/-- The witness-carrying fields of a decoded RLP reference. Both the complete
encoding and its content are statically contained by the source slice. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
structure RlpFieldRef (k_source_off : Nat) (k_source_len : Nat) (k_content_len : Nat) where
  source : (StatelessInputSliceFields k_source_off k_source_len)
  is_list : Bool
  deriving BEq, Inhabited, Repr

@[simp] def RlpFieldRef.content_len {k_source_off : Nat} {k_source_len : Nat} {k_content_len : Nat}
  (_ : (RlpFieldRef k_source_off k_source_len k_content_len)) : Nat :=
  k_content_len

/-- A one-pass RLP cursor is the unconsumed suffix of its source. Decoding
yields a field whose length witnesses a valid advance; the caller owns the
corresponding cursor transition. -/
abbrev RlpCursor (k_source_off : Nat) (k_source_len : Nat) :=
  (StatelessInputSliceFields k_source_off k_source_len)

/-- The same RLP framing invariants over a node encoding held in the scratch
arena. Keeping this nominally separate prevents decoded input fields from
acquiring a runtime byte-source tag. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
structure ScratchRlpFieldRef (k_source_off : Nat) (k_source_len : Nat) (k_content_len : Nat) where
  source : (ScratchSliceFields k_source_off k_source_len)
  is_list : Bool
  deriving BEq, Inhabited, Repr

@[simp] def ScratchRlpFieldRef.content_len {k_source_off : Nat} {k_source_len : Nat}
  {k_content_len : Nat} (_ : (ScratchRlpFieldRef k_source_off k_source_len k_content_len)) : Nat :=
  k_content_len

/-- A one-pass RLP cursor over a scratch-arena node encoding, nominally
distinct from the stateless-input cursor. -/
abbrev ScratchRlpCursor (k_source_off : Nat) (k_source_len : Nat) :=
  (ScratchSliceFields k_source_off k_source_len)

/-- The result of applying an RLP field's protocol-level value constraint.
Structurally invalid or non-canonical RLP remains an `InvalidBlock`
exception; this result distinguishes a well-formed value outside the
requested field domain. -/
/- Type quantifiers: k_value : Type -/
inductive RlpResult (k_value : Type) where
  | RlpOk (_ : k_value)
  | RlpInvalidValue (_ : Unit)
  deriving Inhabited, BEq, Repr
  open RlpResult

/-- Every supported protocol and schema fork, in activation order. This is the
sole fork identity in the model: the decoded schema byte selects a
`ProtocolProfile`, which stores one of these values. The bounded semantic
type prevents values outside the supported fork sequence, while each named
constant retains its precise singleton type for dependent profile typing. -/
abbrev Fork := Nat

/-- Singleton indices for the supported fork sequence. Keeping the numeric
identities named lets dependent profile constraints refer to protocol
forks without repeating their wire-order integers. -/
abbrev frontier_fork_value : Int := 0

/-- `Homestead`'s position in the activation order. -/
abbrev homestead_fork_value : Int := 1

/-- `Byzantium`'s position in the activation order. -/
abbrev byzantium_fork_value : Int := 2

/-- `Constantinople`'s position in the activation order. -/
abbrev constantinople_fork_value : Int := 3

/-- `Istanbul`'s position in the activation order. -/
abbrev istanbul_fork_value : Int := 4

/-- `Berlin`'s position in the activation order. -/
abbrev berlin_fork_value : Int := 5

/-- `London`'s position in the activation order. -/
abbrev london_fork_value : Int := 6

/-- `ArrowGlacier`'s position in the activation order. -/
abbrev arrow_glacier_fork_value : Int := 7

/-- `GrayGlacier`'s position in the activation order. -/
abbrev gray_glacier_fork_value : Int := 8

/-- `Paris`'s position in the activation order. -/
abbrev paris_fork_value : Int := 9

/-- `Shanghai`'s position in the activation order. -/
abbrev shanghai_fork_value : Int := 10

/-- First fork at which blob gas is active. Kept as a type-level singleton so
profile equations and the runtime `Cancun` value share one boundary. -/
abbrev first_blob_fork_value : Int := 11

/-- `Prague`'s position in the activation order. -/
abbrev prague_fork_value : Int := 12

/-- `Osaka`'s position in the activation order. -/
abbrev osaka_fork_value : Int := 13

/-- `BPO1`'s position in the activation order. -/
abbrev bpo1_fork_value : Int := 14

/-- `BPO2`'s position in the activation order. -/
abbrev bpo2_fork_value : Int := 15

/-- `Amsterdam`'s position in the activation order. -/
abbrev amsterdam_fork_value : Int := 16

/-- The base-fee update fractions used by some supported blob schedule. -/
def blob_fee_update_fraction_parameter (k_denominator : Int) : Prop :=
  k_denominator = inactive_blob_fee_update_fraction ∨
  k_denominator = cancun_blob_fee_update_fraction ∨
  k_denominator = prague_blob_fee_update_fraction ∨
  k_denominator = bpo1_blob_fee_update_fraction ∨ k_denominator = bpo2_blob_fee_update_fraction

/-- The five protocol-defined blob schedules. Keeping the fields related
excludes cross-products of individually valid constants which are not a
schedule used by any fork. -/
def blob_schedule_parameters (k_target : Int) (k_maximum : Int) (k_denominator : Int) : Prop :=
  (blob_fee_update_fraction_parameter k_denominator) ∧
  k_target = blob_schedule_inactive_count ∧
  k_maximum = blob_schedule_inactive_count ∧ k_denominator = inactive_blob_fee_update_fraction ∨
  k_target = cancun_blob_target_count ∧
  k_maximum = cancun_blob_max_count ∧ k_denominator = cancun_blob_fee_update_fraction ∨
  k_target = prague_blob_target_count ∧
  k_maximum = prague_blob_max_count ∧ k_denominator = prague_blob_fee_update_fraction ∨
  k_target = bpo1_blob_target_count ∧
  k_maximum = bpo1_blob_max_count ∧ k_denominator = bpo1_blob_fee_update_fraction ∨
  k_target = bpo2_blob_target_count ∧
  k_maximum = bpo2_blob_max_count ∧ k_denominator = bpo2_blob_fee_update_fraction

/-- A fork's associated blob parameters (EIP-4844/EIP-7691): target and
maximum blob counts per block, and the base-fee update fraction. The
existential indices retain the selected schedule's equations wherever the
value is consumed. -/
/- Type quantifiers: k_target : Nat, k_maximum : Nat, k_denominator : Nat, (blob_schedule_parameters k_target k_maximum k_denominator) -/
structure BlobScheduleFields (k_target : Nat) (k_maximum : Nat) (k_denominator : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def BlobScheduleFields.target {k_target : Nat} {k_maximum : Nat} {k_denominator : Nat}
  (_ : (BlobScheduleFields k_target k_maximum k_denominator)) : Nat :=
  k_target
@[simp] def BlobScheduleFields.max {k_target : Nat} {k_maximum : Nat} {k_denominator : Nat}
  (_ : (BlobScheduleFields k_target k_maximum k_denominator)) : Nat :=
  k_maximum
@[simp] def BlobScheduleFields.base_fee_update_fraction {k_target : Nat} {k_maximum : Nat}
  {k_denominator : Nat} (_ : (BlobScheduleFields k_target k_maximum k_denominator)) : Nat :=
  k_denominator

/-- An admitted blob schedule with its three parameters packed
existentially. -/
abbrev BlobSchedule :=
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) => (BlobScheduleFields k_target k_maximum k_denominator))))

/-- Exact active-fork schedule types. These aliases make a profile refinement
visible at call sites without reconstructing or copying its schedule. -/
abbrev cancun_blob_schedule := (BlobScheduleFields 3 6 3338477)

/-- The exact schedule type of the Prague fork (EIP-7691). -/
abbrev prague_blob_schedule := (BlobScheduleFields 6 9 5007716)

/-- The exact schedule type of the Osaka fork, which retains Prague's blob
parameters. -/
abbrev osaka_blob_schedule := (BlobScheduleFields 6 9 5007716)

/-- The exact schedule type of the first blob-parameter-only fork. -/
abbrev bpo1_blob_schedule := (BlobScheduleFields 10 15 8346193)

/-- The exact schedule type of the second blob-parameter-only fork. -/
abbrev bpo2_blob_schedule := (BlobScheduleFields 14 21 11684671)

/-- The exact schedule type of the Amsterdam fork, which retains BPO2's blob
parameters. -/
abbrev amsterdam_blob_schedule := (BlobScheduleFields 14 21 11684671)

/-- The protocol parameter tuples admitted by supported forks. Keeping the
fields under one constraint preserves their relationships: consumers know
that they received one real protocol profile rather than an arbitrary
cross-product of individually valid constants. -/
def protocol_profile_parameters
  (k_fork : Int) (k_target : Int) (k_maximum : Int) (k_denominator : Int) (k_code_limit : Int) (k_initcode_limit
  : Int) (k_transaction_total_gas_limit : Int) (k_transaction_regular_gas_limit : Int) (k_transaction_blob_limit
  : Int) (k_refund_divisor : Int) : Prop :=
  k_fork = berlin_fork_value ∧
  k_target = blob_schedule_inactive_count ∧
  k_maximum = blob_schedule_inactive_count ∧
  k_denominator = inactive_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = inactive_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = ssz_uint_bound ∧
  k_transaction_blob_limit = blob_schedule_inactive_count ∧
  k_refund_divisor = pre_london_refund_divisor ∨
  london_fork_value ≤ k_fork ∧ k_fork ≤ paris_fork_value ∧
  k_target = blob_schedule_inactive_count ∧
  k_maximum = blob_schedule_inactive_count ∧
  k_denominator = inactive_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = inactive_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = ssz_uint_bound ∧
  k_transaction_blob_limit = blob_schedule_inactive_count ∧
  k_refund_divisor = post_london_refund_divisor ∨
  k_fork = shanghai_fork_value ∧
  k_target = blob_schedule_inactive_count ∧
  k_maximum = blob_schedule_inactive_count ∧
  k_denominator = inactive_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = ssz_uint_bound ∧
  k_transaction_blob_limit = blob_schedule_inactive_count ∧
  k_refund_divisor = post_london_refund_divisor ∨
  k_fork = first_blob_fork_value ∧
  k_target = cancun_blob_target_count ∧
  k_maximum = cancun_blob_max_count ∧
  k_denominator = cancun_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = ssz_uint_bound ∧
  k_transaction_blob_limit = cancun_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor
  ∨
  k_fork = prague_fork_value ∧
  k_target = prague_blob_target_count ∧
  k_maximum = prague_blob_max_count ∧
  k_denominator = prague_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = ssz_uint_bound ∧
  k_transaction_blob_limit = prague_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor
  ∨
  k_fork = osaka_fork_value ∧
  k_target = prague_blob_target_count ∧
  k_maximum = prague_blob_max_count ∧
  k_denominator = prague_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_regular_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_blob_limit = cancun_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor
  ∨
  k_fork = bpo1_fork_value ∧
  k_target = bpo1_blob_target_count ∧
  k_maximum = bpo1_blob_max_count ∧
  k_denominator = bpo1_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_regular_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_blob_limit = cancun_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor
  ∨
  k_fork = bpo2_fork_value ∧
  k_target = bpo2_blob_target_count ∧
  k_maximum = bpo2_blob_max_count ∧
  k_denominator = bpo2_blob_fee_update_fraction ∧
  k_code_limit = pre_amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = pre_amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_regular_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_blob_limit = cancun_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor
  ∨
  k_fork = amsterdam_fork_value ∧
  k_target = bpo2_blob_target_count ∧
  k_maximum = bpo2_blob_max_count ∧
  k_denominator = bpo2_blob_fee_update_fraction ∧
  k_code_limit = amsterdam_deployed_code_size_limit ∧
  k_initcode_limit = amsterdam_initcode_size_limit ∧
  k_transaction_total_gas_limit = ssz_uint_bound ∧
  k_transaction_regular_gas_limit = eip7825_transaction_gas_limit ∧
  k_transaction_blob_limit = cancun_blob_max_count ∧ k_refund_divisor = post_london_refund_divisor

/-- The reachable excess-blob-gas ceiling derived from a correlated profile.
The type-level equation is also the field's singleton type, so the stored
value cannot disagree with its fork or schedule. -/
abbrev profile_excess_blob_gas_limit
  (k_fork : Int) (k_target : Int) (k_maximum : Int) (k_denominator : Int) := Int

/-- Execution rules and computation bounds selected together by one schema
fork. The singleton indices retain the exact selected parameter tuple. -/
/- Type quantifiers: k_fork : Nat, k_target : Nat, k_maximum : Nat, k_denominator : Nat, k_code_limit
  : Nat, k_initcode_limit : Nat, k_transaction_total_gas_limit : Nat, k_transaction_regular_gas_limit
  : Nat, k_transaction_blob_limit : Nat, k_refund_divisor : Nat, (protocol_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor) -/
structure ProtocolProfileFields
  (k_fork : Nat) (k_target : Nat) (k_maximum : Nat) (k_denominator : Nat) (k_code_limit : Nat) (k_initcode_limit
  : Nat) (k_transaction_total_gas_limit : Nat) (k_transaction_regular_gas_limit : Nat) (k_transaction_blob_limit
  : Nat) (k_refund_divisor : Nat) where
  blob_schedule : (BlobScheduleFields k_target k_maximum k_denominator)
  deriving BEq, Inhabited, Repr

@[simp] def ProtocolProfileFields.fork {k_fork : Nat} {k_target : Nat} {k_maximum : Nat}
  {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_fork
@[simp] def ProtocolProfileFields.excess_blob_gas_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : (profile_excess_blob_gas_limit k_fork k_target k_maximum k_denominator) :=
  (if ( k_fork < 11  : Bool) then 0 else (256 * k_denominator + (k_maximum - k_target) * 2 ^ 17))
@[simp] def ProtocolProfileFields.deployed_code_size_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_code_limit
@[simp] def ProtocolProfileFields.initcode_size_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_initcode_limit
@[simp] def ProtocolProfileFields.transaction_total_gas_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_transaction_total_gas_limit
@[simp] def ProtocolProfileFields.transaction_regular_gas_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_transaction_regular_gas_limit
@[simp] def ProtocolProfileFields.transaction_blob_limit {k_fork : Nat} {k_target : Nat}
  {k_maximum : Nat} {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_transaction_blob_limit
@[simp] def ProtocolProfileFields.refund_divisor {k_fork : Nat} {k_target : Nat} {k_maximum : Nat}
  {k_denominator : Nat} {k_code_limit : Nat} {k_initcode_limit : Nat}
  {k_transaction_total_gas_limit : Nat} {k_transaction_regular_gas_limit : Nat}
  {k_transaction_blob_limit : Nat} {k_refund_divisor : Nat}
  (_ : (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))
  : Nat :=
  k_refund_divisor

/-- A protocol profile with its parameter tuple packed existentially;
unpacking recovers the admitted combination's equations. -/
abbrev ProtocolProfile :=
  (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))

/-- The gas limits which become concrete once a correlated protocol profile
is paired with the executing block header. `transaction_total_limit`
bounds the transaction's complete reservoir; `transaction_regular_limit`
bounds the regular-execution part of that reservoir. Receipt accumulation
is indexed directly by the two block reservoirs and therefore needs no
duplicate scalar limit here. -/
def gas_limits_parameters
  (k_block_limit : Int) (k_profile_total_limit : Int) (k_profile_regular_limit : Int) (k_transaction_total_limit
  : Int) (k_transaction_regular_limit : Int) : Prop :=
  0 ≤ k_block_limit ∧
  k_block_limit ≤ block_gas_limit_bound ∧
  (protocol_transaction_total_gas_limit_value k_profile_total_limit) ∧
  (protocol_transaction_regular_gas_limit_value k_profile_regular_limit) ∧
  k_transaction_total_limit =
  (if ( k_block_limit < k_profile_total_limit  : Bool) then k_block_limit else k_profile_total_limit)
  ∧
  k_transaction_regular_limit =
  (if ( k_transaction_total_limit < k_profile_regular_limit  : Bool) then k_transaction_total_limit else k_profile_regular_limit)

/-- The concrete gas ceilings for the executing block: the header's block
limit, the derived per-transaction total and regular limits, and the
fixed system-call limits. -/
/- Type quantifiers: k_block_limit : Nat, k_profile_total_limit : Nat, k_profile_regular_limit : Nat, k_transaction_total_limit
  : Nat, k_transaction_regular_limit : Nat, (gas_limits_parameters k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit) -/
structure GasLimitsFields
  (k_block_limit : Nat) (k_profile_total_limit : Nat) (k_profile_regular_limit : Nat) (k_transaction_total_limit
  : Nat) (k_transaction_regular_limit : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def GasLimitsFields.block_limit {k_block_limit : Nat} {k_profile_total_limit : Nat}
  {k_profile_regular_limit : Nat} {k_transaction_total_limit : Nat}
  {k_transaction_regular_limit : Nat}
  (_ : (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))
  : Nat :=
  k_block_limit
@[simp] def GasLimitsFields.transaction_total_limit {k_block_limit : Nat}
  {k_profile_total_limit : Nat} {k_profile_regular_limit : Nat} {k_transaction_total_limit : Nat}
  {k_transaction_regular_limit : Nat}
  (_ : (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))
  : Nat :=
  k_transaction_total_limit
@[simp] def GasLimitsFields.transaction_regular_limit {k_block_limit : Nat}
  {k_profile_total_limit : Nat} {k_profile_regular_limit : Nat} {k_transaction_total_limit : Nat}
  {k_transaction_regular_limit : Nat}
  (_ : (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))
  : Nat :=
  k_transaction_regular_limit
@[simp] def GasLimitsFields.system_regular_limit {k_block_limit : Nat} {k_profile_total_limit : Nat}
  {k_profile_regular_limit : Nat} {k_transaction_total_limit : Nat}
  {k_transaction_regular_limit : Nat}
  (_ : (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))
  : Nat :=
  30000000
@[simp] def GasLimitsFields.system_state_limit {k_block_limit : Nat} {k_profile_total_limit : Nat}
  {k_profile_regular_limit : Nat} {k_transaction_total_limit : Nat}
  {k_transaction_regular_limit : Nat}
  (_ : (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))
  : Nat :=
  0

/-- Gas limits with their five indices packed existentially. -/
abbrev GasLimits :=
  (Sigma fun (k_block_limit : Nat) =>
  (Sigma fun (k_profile_total_limit : Nat) =>
  (Sigma fun (k_profile_regular_limit : Nat) =>
  (Sigma fun (k_transaction_total_limit : Nat) =>
  (Sigma fun (k_transaction_regular_limit : Nat) =>
  (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit))))))

/-- A static protocol profile and the concrete limits derived from the current
header, indexed together. Sharing the profile indices with `GasLimits`
prevents independently valid but mutually inconsistent values from being
paired. -/
def execution_profile_parameters
  (k_fork : Int) (k_target : Int) (k_maximum : Int) (k_denominator : Int) (k_code_limit : Int) (k_initcode_limit
  : Int) (k_profile_total_limit : Int) (k_profile_regular_limit : Int) (k_transaction_blob_limit :
  Int) (k_refund_divisor : Int) (k_block_limit : Int) (k_transaction_total_limit : Int) (k_transaction_regular_limit
  : Int) : Prop :=
  (protocol_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)
  ∧
  (gas_limits_parameters k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit)

/-- The static protocol profile paired with the gas limits derived from it
for the executing block, sharing the profile indices so the pair cannot
disagree. -/
/- Type quantifiers: k_fork : Nat, k_target : Nat, k_maximum : Nat, k_denominator : Nat, k_code_limit
  : Nat, k_initcode_limit : Nat, k_profile_total_limit : Nat, k_profile_regular_limit : Nat, k_transaction_blob_limit
  : Nat, k_refund_divisor : Nat, k_block_limit : Nat, k_transaction_total_limit : Nat, k_transaction_regular_limit
  : Nat, (execution_profile_parameters k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor k_block_limit k_transaction_total_limit k_transaction_regular_limit) -/
structure ExecutionProfileFields
  (k_fork : Nat) (k_target : Nat) (k_maximum : Nat) (k_denominator : Nat) (k_code_limit : Nat) (k_initcode_limit
  : Nat) (k_profile_total_limit : Nat) (k_profile_regular_limit : Nat) (k_transaction_blob_limit :
  Nat) (k_refund_divisor : Nat) (k_block_limit : Nat) (k_transaction_total_limit : Nat) (k_transaction_regular_limit
  : Nat) where
  protocol :
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor)
  gas :
  (GasLimitsFields k_block_limit k_profile_total_limit k_profile_regular_limit k_transaction_total_limit k_transaction_regular_limit)
  deriving BEq, Inhabited, Repr

/-- An execution profile with its thirteen indices packed existentially. -/
abbrev ExecutionProfile :=
  (Sigma fun (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_profile_total_limit : Nat) =>
  (Sigma fun (k_profile_regular_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (Sigma fun (k_block_limit : Nat) =>
  (Sigma fun (k_transaction_total_limit : Nat) =>
  (Sigma fun (k_transaction_regular_limit : Nat) =>
  (ExecutionProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_profile_total_limit k_profile_regular_limit k_transaction_blob_limit k_refund_divisor k_block_limit k_transaction_total_limit k_transaction_regular_limit))))))))))))))

/-- The decoded chain configuration for the executing payload. -/
structure ChainConfig where
  chain_id : chain_identifier
  deriving BEq, Inhabited, Repr

/-- Account state (Yellow Paper §4.1, the account tuple σ[a]).
`storage_root` is the account's *pre-state* storage root — the witness
anchor that stateless storage reads walk on a cached-state miss. It is
set when the account is materialized from the witness and is not kept
live during execution (storage mutates in the overlay maps; the
post-state root is computed on demand). Code bytes are not held here —
the account carries only `code_hash`, a content address into the code
store; the code hash is itself observable state (`EXTCODEHASH`,
EIP-1052) and the binding a stateless witness checks code against. -/
structure AccountInfo where
  nonce : account_nonce
  balance : word
  code_hash : hash
  storage_root : hash
  deriving BEq, Inhabited, Repr

/-- An account plus its lifecycle flags: existence, EIP-161 storage
clearing, same-transaction creation (EIP-6780), and selfdestruction. -/
structure Account where
  info : AccountInfo
  present : Bool
  storage_cleared : Bool
  created : Bool
  selfdestructed : Bool
  deriving BEq, Inhabited, Repr

/-- A storage slot's current and original (transaction-start) values —
the pair EIP-2200/EIP-3529 gas and refund rules compare. -/
structure StorageValue where
  curr : word
  orig : word
  deriving BEq, Inhabited, Repr

/-- Result of consulting the transaction storage layer. A clear generation is
not a slot-row hit: it resolves the value to zero, but the caller must
still record the first EIP-7928 read of that slot. -/
inductive StorageTxLookup where
  | StorageTxHit (_ : StorageValue)
  | StorageTxCleared (_ : Unit)
  | StorageTxMiss (_ : Unit)
  deriving Inhabited, BEq, Repr
  open StorageTxLookup

/-- A fully qualified storage key: account address and 256-bit slot. -/
structure StorageKey where
  addr : address
  slot : word
  deriving BEq, Inhabited, Repr

/-- An overlay storage row: the form in which storage entries are
enumerated at transaction-end merge. -/
structure StorageEntry where
  key : StorageKey
  value : StorageValue
  deriving BEq, Inhabited, Repr

/-- A block-layer storage lookup. `value` is meaningful exactly when `found`
is true; the explicit bit keeps a real all-zero row distinct from a miss. -/
structure StorageBlockRow where
  found : Bool
  value : StorageValue
  deriving BEq, Inhabited, Repr

/-- One transaction-layer storage row, or exhaustion of the destructive
transaction-end drain. -/
inductive StorageTxPopResult where
  | StorageTxPopRow (_ : StorageEntry)
  | StorageTxPopExhausted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open StorageTxPopResult

/-- An account's current and original (transaction-start) states. -/
structure AcctValue where
  curr : Account
  orig : Account
  deriving BEq, Inhabited, Repr

/-- An overlay account row: the form in which account entries are
enumerated at transaction-end merge. -/
structure AcctEntry where
  addr : address
  value : AcctValue
  deriving BEq, Inhabited, Repr

/-- An account-layer lookup. `account` is meaningful exactly when `found` is
true; [EMPTY_ACCOUNT][] is the payload sentinel for a miss. -/
structure AccountRow where
  found : Bool
  account : Account
  deriving BEq, Inhabited, Repr

/-- One transaction-layer account row, or exhaustion of the destructive
transaction-end drain. -/
inductive AcctTxPopResult where
  | AcctTxPopRow (_ : AcctEntry)
  | AcctTxPopExhausted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open AcctTxPopResult

/-- A host storage row prepared for secure-trie traversal. The semantic row
remains [StorageEntry][type-StorageEntry]; these cached digests are derived
traversal metadata computed when the witness value is first materialized. -/
structure StorageTrieEntry where
  entry : StorageEntry
  address_hash : hash
  slot_hash : hash
  deriving BEq, Inhabited, Repr

/-- One block-layer storage iterator row, or iterator exhaustion. -/
inductive StorageBlockIterResult where
  | StorageBlockIterRow (_ : StorageTrieEntry)
  | StorageBlockIterExhausted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open StorageBlockIterResult

/-- A host account row prepared for secure-trie traversal. -/
structure AcctTrieEntry where
  entry : AcctEntry
  address_hash : hash
  deriving BEq, Inhabited, Repr

/-- One block-layer account iterator row, or iterator exhaustion. -/
inductive AcctBlockIterResult where
  | AcctBlockIterRow (_ : AcctTrieEntry)
  | AcctBlockIterExhausted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open AcctBlockIterResult

/-- The EIP-2718 envelope type: the single transaction discriminant. Its
closed semantic descriptor below derives fork and feature requirements;
transactions do not store redundant boolean flags. -/
inductive TxType where | LegacyTx | AccessListTx | FeeMarketTx | BlobTx | SetCodeTx
  deriving BEq, Inhabited, Repr
  open TxType

/-- The two transaction-signature encodings. Legacy transactions use the
original/EIP-155 `v` domain; every EIP-2718 typed envelope carries an
explicit zero-or-one parity. -/
inductive TxSignatureScheme where | LegacySignature | TypedSignature
  deriving BEq, Inhabited, Repr
  open TxSignatureScheme

/-- Protocol requirements determined solely by an EIP-2718 envelope type.
Computing this descriptor once prevents validation from repeatedly
dispatching on the same closed transaction-type algebra. -/
structure TxTypeSemantics where
  minimum_fork : Fork
  signature : TxSignatureScheme
  blob : Bool
  set_code : Bool
  deriving BEq, Inhabited, Repr

/-- An EIP-7702 set-code authorization tuple. RLP decoding recovers the
authority and validates the signature (`s <= n/2`, `y_parity`, `r`);
`valid_sig` records that result, and [process_auth][] applies the
chain-id/nonce/code checks. -/
structure Authorization where
  valid_sig : Bool
  authority : address
  address : address
  nonce : account_nonce
  chain_id : word
  deriving BEq, Inhabited, Repr

/-- The source-backed EIP-4844 versioned blob hashes of a transaction. The
limit index records which fork-selected transaction range admitted the
observed count. It is carried by the enclosing transaction rather than
hidden in a nested existential, so consumers retain `count <= limit`. -/
/- Type quantifiers: k_limit : Nat, (transaction_blob_limit_value k_limit) -/
structure BlobHashesFields (k_limit : Nat) where
  bytes : StatelessInputSlice
  count : (transaction_blob_count k_limit)
  deriving BEq, Inhabited, Repr

/-- A byte span contained by one SSZ transaction envelope. This structural
bound is independent of fork-specific calldata and initcode limits. -/
abbrev TransactionInputSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))

/-- A collection count whose entries are encoded inside one transaction. -/
abbrev transaction_item_count := Nat

/-- EIP-7825 bounds a post-Prague transaction to 2^24 regular gas, while
Amsterdam's least expensive authorization costs 7,816 execution gas.
Successful transaction validity therefore proves that no executable
authorization list can contain more entries than this bound. -/
abbrev prepared_authorization_count_bound : Int := (eip7825_transaction_gas_limit / 7816)

/-- The number of authorization tuples admitted by one valid transaction. -/
abbrev prepared_authorization_count := Nat

/-- Authorizations decoded and signature-recovered after transaction validity
but before any world-state mutation. The semantic model retains an
immutable list. Optimized C represents the same ordered collection as a
cursor into a fixed-capacity transaction workspace. -/
structure PreparedAuthorizationList where
  entries : (List Authorization)
  count : prepared_authorization_count
  deriving BEq, Inhabited, Repr

/-- A validated EIP-2930 access-list content span. Entries remain in their
canonical RLP encoding and are consumed with a cursor when prewarming. -/
structure AccessListRef where
  encoded : StatelessInputSlice
  address_count : transaction_item_count
  slot_count : transaction_item_count
  deriving BEq, Inhabited, Repr

/-- A validated EIP-7702 authorization-list content span. Its dependent count
remains visible to execution, so recursive authorization processing can
derive refund bounds from the number of admitted tuples. -/
/- Type quantifiers: k_count : Nat, 0 ≤ k_count ∧ k_count ≤ transaction_length_bound -/
structure AuthorizationListRefFields (k_count : Nat) where
  encoded : StatelessInputSlice
  deriving BEq, Inhabited, Repr

@[simp] def AuthorizationListRefFields.count {k_count : Nat}
  (_ : (AuthorizationListRefFields k_count)) : Nat :=
  k_count

/-- An authorization-list reference packing its admitted tuple count
existentially. -/
abbrev AuthorizationListRef := (Sigma fun (k_count : Nat) => (AuthorizationListRefFields k_count))

/-- A byte length contained by one SSZ transaction envelope. -/
abbrev transaction_byte_length := Nat

/-- The greatest EIP-2028 calldata charge admitted by an SSZ transaction. -/
abbrev transaction_calldata_cost := Nat

/-- The greatest EIP-7623 calldata floor admitted by an SSZ transaction. -/
abbrev transaction_calldata_floor_cost := Nat

/-- The greatest EIP-3860 initcode charge admitted by an SSZ transaction. -/
abbrev transaction_initcode_cost := Nat

/-- A decoded transaction. Covers the EIP-2718 typed envelopes 0–4:
legacy, EIP-2930 (access list), EIP-1559 (fee market), EIP-4844
(blob), and EIP-7702 (set code); the type-specific fields are
validity-relevant per their EIP. -/
/- Type quantifiers: k_blob_limit : Nat, (transaction_blob_limit_value k_blob_limit) -/
structure TransactionFields (k_blob_limit : Nat) where
  tx_type : TxType
  sender : address
  nonce : word
  chain_id : chain_identifier
  gas_limit : transaction_gas
  is_create : Bool
  recipient : address
  value : word
  raw : StatelessInputSlice
  input_src : TransactionInputSlice
  access_list : AccessListRef
  max_fee : word
  max_blob_fee : word
  max_priority_fee : word
  authorizations : AuthorizationListRef
  blob_hashes : (BlobHashesFields k_blob_limit)
  pubkey : StatelessInputSlice
  signing_hash : hash
  sig_v : word
  sig_r : word
  sig_s : word
  deriving BEq, Inhabited, Repr

/-- A decoded transaction retains the fork-selected blob-count limit that was
applied while decoding its envelope. Non-blob transactions carry the
inactive zero limit. -/
abbrev Transaction := (Sigma fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit))

/-- The bounded topic operands of one `LOG0`–`LOG4` instruction. Keeping the
arity in the constructor avoids allocating a Sail list for at most four
stack words. -/
inductive LogTopics where
  | LogTopics0 (_ : Unit)
  | LogTopics1 (_ : word)
  | LogTopics2 (_ : (word × word))
  | LogTopics3 (_ : (word × word × word))
  | LogTopics4 (_ : (word × word × word × word))
  deriving Inhabited, BEq, Repr
  open LogTopics

/-- A host log-store cursor. It is an opaque bounded collection position,
not an EVM quantity; the host rejects positions outside the current
block's retained log series. -/
abbrev log_store_index_bound : Int := (2 ^ 64 - 1)

/-- A position in the block-lifetime host log store. -/
abbrev log_store_index := Nat

/-- A consecutive transaction-local view into the block-lifetime host log
store. Reverted frame records are removed before this view is captured. -/
structure LogSeriesRef where
  start : log_store_index
  count : log_store_index
  deriving BEq, Inhabited, Repr

/-- A transaction receipt. `success` means the top-level frame
returned/stopped without reverting or exceptionally halting (used for
create-tx code deposit and log inclusion). Transaction-invalidity is an
invalid-block exception and therefore never produces a receipt. A valid
transaction that reverts or runs out of gas still has `success = false`
and produces a legitimate receipt. `execution_gas` is the execution-gas
contribution to Amsterdam
block accounting *without* the EIP-3529 refund
(`max(execution_gas_before_refund, calldata_floor)`); it
differs from `gas_used` (the receipt's refunded gas) precisely by the
refund and excludes `state_gas`. -/
def receipt_gas_relation
  (k_limit : Int) (k_regular_limit : Int) (k_gas_used : Int) (k_execution_gas : Int) (k_state_gas :
  Int) : Prop :=
  0 ≤ k_limit ∧
  k_limit ≤ block_gas_limit_bound ∧
  0 ≤ k_regular_limit ∧
  k_regular_limit ≤ k_limit ∧
  0 ≤ k_gas_used ∧
  k_gas_used ≤ k_limit ∧
  0 ≤ k_execution_gas ∧
  k_execution_gas ≤ k_regular_limit ∧
  0 ≤ k_state_gas ∧ k_state_gas ≤ k_limit ∧ k_gas_used ≤ (k_execution_gas + k_state_gas)

/-- The concrete receipt record indexed by its admitted and consumed gas
quantities. -/
/- Type quantifiers: k_limit : Nat, k_regular_limit : Nat, k_gas_used : Nat, k_execution_gas : Nat, k_state_gas
  : Nat, (receipt_gas_relation k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas) -/
structure ReceiptFields
  (k_limit : Nat) (k_regular_limit : Nat) (k_gas_used : Nat) (k_execution_gas : Nat) (k_state_gas :
  Nat) where
  tx_type : TxType
  success : Bool
  logs : LogSeriesRef
  deriving BEq, Inhabited, Repr

@[simp] def ReceiptFields.gas_used {k_limit : Nat} {k_regular_limit : Nat} {k_gas_used : Nat}
  {k_execution_gas : Nat} {k_state_gas : Nat}
  (_ : (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas)) : Nat :=
  k_gas_used
@[simp] def ReceiptFields.execution_gas {k_limit : Nat} {k_regular_limit : Nat} {k_gas_used : Nat}
  {k_execution_gas : Nat} {k_state_gas : Nat}
  (_ : (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas)) : Nat :=
  k_execution_gas
@[simp] def ReceiptFields.state_gas {k_limit : Nat} {k_regular_limit : Nat} {k_gas_used : Nat}
  {k_execution_gas : Nat} {k_state_gas : Nat}
  (_ : (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas)) : Nat :=
  k_state_gas

/-- A receipt retaining the transaction limits that bounded each gas
contribution. -/
abbrev ReceiptWithin (k_limit : Int) (k_regular_limit : Int) :=
  (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))

/-- A receipt whose originating transaction limit is not needed. -/
abbrev Receipt :=
  (Sigma fun (k_state_gas : Nat) =>
  (Sigma fun (k_execution_gas : Nat) =>
  (Sigma fun (k_gas_used : Nat) =>
  (Sigma fun (k_regular_limit : Nat) =>
  (Sigma fun (k_limit : Nat) =>
  (ReceiptFields k_limit k_regular_limit k_gas_used k_execution_gas k_state_gas))))))

/-- The 2048-bit logs bloom filter (YP §4.4.1), as 256 bytes. -/
abbrev LogsBloom := (Vector byte 256)

/-- The payload header's 2048-bit logs bloom in canonical SSZ wire order.
Keeping the authenticated input range by reference lets consumers that
already operate on bytes avoid an eager 256-byte materialization. -/
abbrev LogsBloomRef := (StatelessInputSliceLength 256)

/-- The execution-payload header fields the model reads and validates
(YP §4.4). Scalar wire bounds come from the consensus/Amsterdam SSZ
`ExecutionPayload` schema. `gas_used <= gas_limit` and the active
blob-schedule rules are execution-protocol constraints checked when the
payload is admitted. `extra_data` retains the schema's
`ByteList[MAX_EXTRA_DATA_BYTES]` bound while staying source-backed; it is
RLP-encoded whole for the header hash and never inspected. The fixed
`logs_bloom` commitment likewise stays source-backed until a semantic
consumer explicitly decodes it. -/
structure BlockHeader where
  number : block_number
  timestamp : block_timestamp
  gas_limit : block_gas_limit
  gas_used : block_gas
  prev_randao : word
  base_fee : word
  blob_gas_used : blob_gas_used
  excess_blob_gas : excess_blob_gas
  state_root : hash
  receipts_root : hash
  logs_bloom : LogsBloomRef
  fee_recipient : address
  parent_hash : hash
  parent_beacon_block_root : hash
  slot_number : slot_number
  extra_data : (StatelessInputSliceAtMost (2 ^ 5))
  deriving BEq, Inhabited, Repr

/-- An EIP-4895 beacon-chain withdrawal: index, validator, recipient,
and amount in gwei. -/
structure Withdrawal where
  index : withdrawal_index
  validator_index : validator_index
  address : address
  amount : withdrawal_amount
  deriving BEq, Inhabited, Repr

/-- The block body. The semantic structure is explicit while its
potentially large fields stay source-backed until individual elements
are needed. -/
structure BlockBody where
  transactions : TransactionListRef
  withdrawals : WithdrawalListRef
  block_access_list : (StatelessInputSliceAtMost (2 ^ 30))
  deriving BEq, Inhabited, Repr

/-- A block: header plus body. -/
structure Block where
  header : BlockHeader
  body : BlockBody
  deriving BEq, Inhabited, Repr

/-- The execution payload under validation: the block and the block hash
the consensus layer expects it to commit to. -/
structure ExecutionPayload where
  expected_block_hash : hash
  block' : Block
  deriving BEq, Inhabited, Repr

/-- The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. -/
/- Type quantifiers: k_blob_limit : Nat, (transaction_blob_limit_value k_blob_limit) -/
structure TxEnvFields (k_blob_limit : Nat) where
  origin : address
  gas_price : word
  blob_hashes : (BlobHashesFields k_blob_limit)
  deriving BEq, Inhabited, Repr

/-- A transaction environment packing its fork-selected blob-count limit
existentially. -/
abbrev TxEnv := (Sigma fun (k_blob_limit : Nat) => (TxEnvFields k_blob_limit))

/-- The header/profile-bounded gas allowance attached to one decoded
transaction before stateful validation. The total allowance funds both
gas dimensions; the regular allowance is the portion available to
ordinary EVM execution. -/
/- Type quantifiers: k_total : Nat, k_regular : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_total ∧ k_total ≤ block_gas_limit_bound -/
structure TransactionGasAllowanceFields (k_total : Nat) (k_regular : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def TransactionGasAllowanceFields.total {k_total : Nat} {k_regular : Nat}
  (_ : (TransactionGasAllowanceFields k_total k_regular)) : Nat :=
  k_total
@[simp] def TransactionGasAllowanceFields.regular {k_total : Nat} {k_regular : Nat}
  (_ : (TransactionGasAllowanceFields k_total k_regular)) : Nat :=
  k_regular

/-- A transaction gas allowance with existentially hidden total and regular
budgets. -/
abbrev TransactionGasAllowance :=
  (Sigma fun (k_total : Nat) =>
  (Sigma fun (k_regular : Nat) => (TransactionGasAllowanceFields k_total k_regular)))

/-- The post-intrinsic gas state admitted for a transaction. Keeping the
admitted limit, the regular-gas ceiling, both intrinsic charges, and the
two live reservoirs in one dependent value preserves the conservation
equation established by transaction validation. -/
def transaction_initial_gas_relation
  (k_total : Int) (k_regular : Int) (k_intrinsic_execution : Int) (k_intrinsic_state : Int) (k_calldata_floor
  : Int) (k_execution : Int) (k_state : Int) : Prop :=
  0 ≤ k_intrinsic_execution ∧
  0 ≤ k_intrinsic_state ∧
  0 ≤ k_calldata_floor ∧
  k_intrinsic_execution ≤ k_regular ∧
  k_calldata_floor ≤ k_regular ∧
  k_regular ≤ k_total ∧
  k_total ≤ block_gas_limit_bound ∧
  0 ≤ k_execution ∧
  0 ≤ k_state ∧
  (k_execution + k_state + k_intrinsic_execution + k_intrinsic_state) = k_total ∧
  k_execution ≤ (k_regular - k_intrinsic_execution)

/-- The concrete post-intrinsic gas record whose indices retain the
transaction conservation relation. -/
/- Type quantifiers: k_total : Nat, k_regular : Nat, k_intrinsic_execution : Nat, k_intrinsic_state
  : Nat, k_calldata_floor : Nat, k_execution : Nat, k_state : Nat, (transaction_initial_gas_relation k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state) -/
structure TransactionInitialGasFields
  (k_total : Nat) (k_regular : Nat) (k_intrinsic_execution : Nat) (k_intrinsic_state : Nat) (k_calldata_floor
  : Nat) (k_execution : Nat) (k_state : Nat) where
  deriving BEq, Inhabited, Repr

@[simp] def TransactionInitialGasFields.admitted_limit {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_total
@[simp] def TransactionInitialGasFields.regular_limit {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_regular
@[simp] def TransactionInitialGasFields.intrinsic_execution {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_intrinsic_execution
@[simp] def TransactionInitialGasFields.intrinsic_state {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_intrinsic_state
@[simp] def TransactionInitialGasFields.calldata_floor {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_calldata_floor
@[simp] def TransactionInitialGasFields.execution_remaining {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_execution
@[simp] def TransactionInitialGasFields.state_remaining {k_total : Nat} {k_regular : Nat}
  {k_intrinsic_execution : Nat} {k_intrinsic_state : Nat} {k_calldata_floor : Nat}
  {k_execution : Nat} {k_state : Nat}
  (_ : (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))
  : Nat :=
  k_state

/-- The initial gas state with every conserved quantity hidden
existentially. -/
abbrev TransactionInitialGas :=
  (Sigma fun (k_total : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))

/-- The initial gas state for one concrete admitted transaction limit. This
view hides the internal split while retaining the limit that the block
transaction loop has already checked against its remaining budgets. -/
abbrev TransactionInitialGasFor (k_total : Int) :=
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state)))))))

/-- The initial gas state retaining both the complete and regular execution
allowances checked by the block loop. -/
abbrev TransactionInitialGasForLimits (k_total : Int) (k_regular : Int) :=
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TransactionInitialGasFields k_total k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))

/-- The values established by successful up-front transaction validation
(YP §6.2): sender, pre-state nonce, intrinsic gas, blob fee, and the
EIP-1559 effective gas and priority prices. Invalid transactions terminate
before this value is constructed. -/
/- Type quantifiers: k_limit : Nat, k_regular : Nat, k_intrinsic_execution : Nat, k_intrinsic_state
  : Nat, k_calldata_floor : Nat, k_execution : Nat, k_state : Nat, (transaction_initial_gas_relation k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state) -/
structure TxValidityFields
  (k_limit : Nat) (k_regular : Nat) (k_intrinsic_execution : Nat) (k_intrinsic_state : Nat) (k_calldata_floor
  : Nat) (k_execution : Nat) (k_state : Nat) where
  sender : address
  nonce_before : account_nonce
  gas :
  (TransactionInitialGasFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state)
  blob_fee : word
  gas_price : word
  priority_fee : word
  deriving BEq, Inhabited, Repr

/-- A validity result retaining the transaction's admitted total and regular
gas limits while hiding the internal initial split. -/
abbrev TxValidityForLimits (k_limit : Int) (k_regular : Int) :=
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))

/-- A validity result whose limit is not needed by the consumer. -/
abbrev TxValidity :=
  (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_intrinsic_execution : Nat) =>
  (Sigma fun (k_intrinsic_state : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state))))))))

/-- The transaction gas conserved at the top-level frame boundary. The
snapshot retains both admitted budgets and the exact split consumed by
ordinary execution and state access. Settlement can therefore apply
refunds and the calldata floor without reconstructing or revalidating
either contribution. -/
def tx_frame_gas_snapshot_relation
  (k_limit : Int) (k_regular : Int) (k_calldata_floor : Int) (k_remaining : Int) (k_state_used : Int)
  : Prop :=
  0 ≤ k_regular ∧
  k_regular ≤ k_limit ∧
  k_limit ≤ block_gas_limit_bound ∧
  0 ≤ k_calldata_floor ∧
  k_calldata_floor ≤ k_regular ∧
  0 ≤ k_remaining ∧
  0 ≤ k_state_used ∧
  k_state_used ≤ k_limit ∧
  (k_remaining + k_state_used) ≤ k_limit ∧ (k_limit - k_remaining - k_state_used) ≤ k_regular

/-- The concrete top-level frame gas snapshot whose indices retain the
transaction admission and conservation proof. -/
/- Type quantifiers: k_limit : Nat, k_regular : Nat, k_calldata_floor : Nat, k_remaining : Nat, k_state_used
  : Nat, (tx_frame_gas_snapshot_relation k_limit k_regular k_calldata_floor k_remaining k_state_used) -/
structure TxFrameGasSnapshotFields
  (k_limit : Nat) (k_regular : Nat) (k_calldata_floor : Nat) (k_remaining : Nat) (k_state_used : Nat)
  where
  deriving BEq, Inhabited, Repr

@[simp] def TxFrameGasSnapshotFields.admitted_limit {k_limit : Nat} {k_regular : Nat}
  {k_calldata_floor : Nat} {k_remaining : Nat} {k_state_used : Nat}
  (_ : (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used)) : Nat
  :=
  k_limit
@[simp] def TxFrameGasSnapshotFields.regular_limit {k_limit : Nat} {k_regular : Nat}
  {k_calldata_floor : Nat} {k_remaining : Nat} {k_state_used : Nat}
  (_ : (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used)) : Nat
  :=
  k_regular
@[simp] def TxFrameGasSnapshotFields.calldata_floor {k_limit : Nat} {k_regular : Nat}
  {k_calldata_floor : Nat} {k_remaining : Nat} {k_state_used : Nat}
  (_ : (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used)) : Nat
  :=
  k_calldata_floor
@[simp] def TxFrameGasSnapshotFields.remaining {k_limit : Nat} {k_regular : Nat}
  {k_calldata_floor : Nat} {k_remaining : Nat} {k_state_used : Nat}
  (_ : (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used)) : Nat
  :=
  k_remaining
@[simp] def TxFrameGasSnapshotFields.state_used {k_limit : Nat} {k_regular : Nat}
  {k_calldata_floor : Nat} {k_remaining : Nat} {k_state_used : Nat}
  (_ : (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used)) : Nat
  :=
  k_state_used

/-- A top-level frame gas snapshot with existentially hidden gas totals. -/
abbrev TxFrameGasSnapshot :=
  (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) =>
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_remaining : Nat) =>
  (Sigma fun (k_state_used : Nat) =>
  (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used))))))

/-- A completed top-level frame tied to both admitted transaction limits. -/
abbrev TxFrameGasSnapshotForLimits (k_limit : Int) (k_regular : Int) :=
  (Sigma fun (k_calldata_floor : Nat) =>
  (Sigma fun (k_remaining : Nat) =>
  (Sigma fun (k_state_used : Nat) =>
  (TxFrameGasSnapshotFields k_limit k_regular k_calldata_floor k_remaining k_state_used))))

/-- The result of a transaction's top-level frame: success, the bounded gas
snapshot consumed by settlement, and refunds. -/
/- Type quantifiers: k_limit : Nat, k_regular : Nat, 0 ≤ k_regular ∧
  k_regular ≤ k_limit ∧ k_limit ≤ block_gas_limit_bound -/
structure TxFrameResultFields (k_limit : Nat) (k_regular : Nat) where
  success : Bool
  gas : (TxFrameGasSnapshotForLimits k_limit k_regular)
  refund : gas_refund
  deriving BEq, Inhabited, Repr

/-- A completed top-level frame with existentially hidden admitted limits. -/
abbrev TxFrameResult :=
  (Sigma fun (k_limit : Nat) =>
  (Sigma fun (k_regular : Nat) => (TxFrameResultFields k_limit k_regular)))

/-- The four CALL-family execution modes. `Call` is an ordinary call;
`CallCode` combines the caller's storage with the target's code;
`DelegateCall` additionally inherits the caller and value; and
`StaticCall` enters a read-only frame. -/
inductive CallKind where | Call | CallCode | DelegateCall | StaticCall
  deriving BEq, Inhabited, Repr
  open CallKind

/-- The two contract-creation address schemes. `CreateByNonce` is ordinary
`CREATE`; `CreateBySalt` is EIP-1014 `CREATE2`. Keeping this as a closed
semantic tag prevents callers from encoding an execution mode in an
otherwise unexplained boolean. -/
inductive CreateKind where | CreateByNonce | CreateBySalt
  deriving BEq, Inhabited, Repr
  open CreateKind

/-- The per-frame call message (YP §8, the I tuple): caller, executing
address, code owner, value, calldata length, static flag, and call
depth. -/
structure Message where
  caller : address
  code_address : address
  address : address
  value : word
  state_gas_reservoir : state_gas
  is_static : Bool
  depth : frame_depth
  deriving BEq, Inhabited, Repr

/-- The complete carried state installed after entering or resuming a frame.
Named fields replace the former positional 19-tuple at this cold semantic
boundary; the optimized interpreter immediately unpacks the record into
its hot scalar locals. -/
structure FrameTransition where
  pc : code_pointer
  gas_remaining : gas
  state_gas_remaining : state_gas
  state_gas_spilled : state_gas_spill
  refund : gas_refund
  status : FrameStatus
  stack_top : StackPointer
  memory_base : memory_base
  memory_height : memory_height
  message : Message
  code : Code
  calldata : CalldataSlice
  returndata : OutputSlice
  deriving BEq, Inhabited, Repr

/-- State installed when an executing frame enters an exceptional halt.
Keeping the normalized Amsterdam state-gas values together prevents the
transition from being represented as an anonymous positional tuple. -/
structure ExceptionalStateTransition where
  state_gas_remaining : state_gas
  state_gas_spilled : state_gas_spill
  status : FrameStatus
  deriving BEq, Inhabited, Repr

/-- Lightweight result of one opcode handler. -/
inductive OpcodeOutcome where
  | Continue (_ : Unit)
  | Failed (_ : ExceptionKind)
  deriving Inhabited, BEq, Repr
  open OpcodeOutcome

/-- The suspended parent-frame state restored after nested execution. -/
structure FrameCheckpoint where
  pc : code_pointer
  gas_remaining : gas
  stack_top : StackPointer
  state_gas_remaining : state_gas
  state_gas_spilled : state_gas_spill
  refund : gas_refund
  status : FrameStatus
  message : Message
  code : Code
  calldata : CalldataSlice
  memory_height : memory_height
  deriving BEq, Inhabited, Repr

/-- The suspended parent information needed after a message call returns. -/
structure CallContinuation where
  checkpoint : FrameCheckpoint
  return_offset : memory_base
  return_length : memory_length
  new_account_charged : Bool
  deriving BEq, Inhabited, Repr

/-- The suspended parent information needed after initcode returns. -/
structure CreateContinuation where
  checkpoint : FrameCheckpoint
  address : address
  new_account_charged : Bool
  deriving BEq, Inhabited, Repr

/-- The pending action performed when a child frame finishes. -/
inductive FrameContinuation where
  | Empty (_ : Unit)
  | ResumeCall (_ : CallContinuation)
  | ResumeCreate (_ : CreateContinuation)
  deriving Inhabited, BEq, Repr
  open FrameContinuation

/-- The guest's decoded input: the execution payload and the chain
configuration it executes under. -/
structure StatelessInput where
  payload : ExecutionPayload
  chain_config : ChainConfig
  deriving BEq, Inhabited, Repr

/-- Authenticated facts about the parent block that validation rules
compare the payload header against. -/
structure WitnessContext where
  parent_hash : hash
  parent_state_root : hash
  parent_base_fee_per_gas : word
  parent_blob_gas_used : blob_gas_used
  parent_excess_blob_gas : excess_blob_gas
  deriving BEq, Inhabited, Repr

/-- An RLP byte count that can be materialized in the scratch arena. The
host scratch-region limit is enforced while recursive collection totals
are accumulated. -/
abbrev rlp_scratch_length := Nat

/-- Values accepted by the generic natural-number RLP helpers. Canonical RLP
is unbounded; optimized builds refine values entering the native encoder. -/
def rlp_natural_valid (k_value : Int) : Prop := 0 ≤ k_value

/-- Natural values that can be incremented while counting encoded bytes. -/
def rlp_natural_increment_valid (k_value : Int) : Prop := 0 ≤ k_value

/-- Encoded width returned by the generic natural-number RLP helper. -/
abbrev rlp_natural_size := Nat

/-- One exact-size RLP construction in the shared scratch arena. -/
structure RlpEncoder where
  start : source_pointer
  expected_len : source_length
  deriving BEq, Inhabited, Repr

/-- A decoded access list and the counts needed for intrinsic gas. -/
/- Type quantifiers: k_address_bound : Nat, k_slot_bound : Nat, (source_valid_length k_address_bound)
  ∧ (source_valid_length k_slot_bound) -/
structure AccessListDecode (k_address_bound : Nat) (k_slot_bound : Nat) where
  address_count : Nat
  slot_count : Nat
  deriving BEq, Inhabited, Repr

/-- A prior transient-storage value restored by frame rollback. -/
structure JournalTransientChange where
  address : address
  slot : word
  prior : word
  deriving BEq, Inhabited, Repr

/-- The prior warm epoch of an account. -/
structure JournalWarmAccountChange where
  address : address
  prior_epoch : block_access_index
  deriving BEq, Inhabited, Repr

/-- The prior warm epoch of a storage location. -/
structure JournalWarmStorageChange where
  key : StorageKey
  prior_epoch : block_access_index
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible account balance. -/
structure JournalAccountBalanceChange where
  address : address
  prior : word
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible account nonce. -/
structure JournalAccountNonceChange where
  address : address
  prior : account_nonce
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible account code hash. -/
structure JournalAccountCodeHashChange where
  address : address
  prior : hash
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible account-existence flag. -/
structure JournalAccountExistsChange where
  address : address
  prior : Bool
  deriving BEq, Inhabited, Repr

/-- The prior same-transaction account-creation flag. -/
structure JournalAccountCreatedChange where
  address : address
  prior : Bool
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible selfdestruction flag. -/
structure JournalAccountSelfdestructedChange where
  address : address
  prior : Bool
  deriving BEq, Inhabited, Repr

/-- The prior transaction-visible storage value. -/
structure JournalStorageValueChange where
  key : StorageKey
  prior : word
  deriving BEq, Inhabited, Repr

/-- The prior incarnation attached to one transaction-visible storage row. -/
structure JournalStorageRowGenerationChange where
  key : StorageKey
  prior : storage_generation
  deriving BEq, Inhabited, Repr

/-- The prior account-wide storage incarnation. -/
structure JournalAccountStorageGenerationChange where
  address : address
  prior : storage_generation
  deriving BEq, Inhabited, Repr

/-- One append-only state-journal record. -/
inductive StateJournalEntry where
  | JournalTransientChanged (_ : JournalTransientChange)
  | JournalWarmAccountChanged (_ : JournalWarmAccountChange)
  | JournalWarmStorageChanged (_ : JournalWarmStorageChange)
  | JournalAccountBalanceChanged (_ : JournalAccountBalanceChange)
  | JournalAccountNonceChanged (_ : JournalAccountNonceChange)
  | JournalAccountCodeHashChanged (_ : JournalAccountCodeHashChange)
  | JournalAccountExistsChanged (_ : JournalAccountExistsChange)
  | JournalAccountCreatedChanged (_ : JournalAccountCreatedChange)
  | JournalAccountSelfdestructedChanged (_ : JournalAccountSelfdestructedChange)
  | JournalTransactionAccountListed (_ : Unit)
  | JournalTransactionStorageListed (_ : address)
  | JournalLogAppended (_ : Unit)
  | JournalStorageValueChanged (_ : JournalStorageValueChange)
  | JournalStorageRowGenerationChanged (_ : JournalStorageRowGenerationChange)
  | JournalAccountStorageGenerationChanged (_ : JournalAccountStorageGenerationChange)
  | JournalFrameCheckpointed (_ : Unit)
  | JournalFrameCommitted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open StateJournalEntry

/-- One recorded storage change in canonical host traversal order. -/
structure BalStorageChangeEntry where
  slot : word
  index : block_access_index
  value : word
  deriving BEq, Inhabited, Repr

/-- One recorded balance change in canonical host traversal order. -/
structure BalBalanceChangeEntry where
  index : block_access_index
  value : word
  deriving BEq, Inhabited, Repr

/-- One recorded nonce change in canonical host traversal order. -/
structure BalNonceChangeEntry where
  index : block_access_index
  value : account_nonce
  deriving BEq, Inhabited, Repr

/-- One recorded code change in canonical host traversal order. -/
structure BalCodeChangeEntry where
  index : block_access_index
  code_hash : hash
  deriving BEq, Inhabited, Repr

/-- One event in the canonical account-delimited BAL stream. -/
inductive BalIterEntry where
  | BalAccount (_ : address)
  | BalStorageChange (_ : BalStorageChangeEntry)
  | BalStorageRead (_ : word)
  | BalBalanceChange (_ : BalBalanceChangeEntry)
  | BalNonceChange (_ : BalNonceChangeEntry)
  | BalCodeChange (_ : BalCodeChangeEntry)
  | BalAccountEnd (_ : Unit)
  | BalEmpty (_ : Unit)
  deriving Inhabited, BEq, Repr
  open BalIterEntry

/-- The closed environment-projection algebra interpreted by [k_env][]. Each
opcode supplies one constant member, so the shared projection remains
explicit and first-order for executable and proof backends. -/
inductive EnvField where | F_Number | F_Timestamp | F_Coinbase | F_BaseFee | F_ChainId | F_GasLimit | F_PrevRandao | F_Origin | F_GasPrice | F_SlotNumber
  deriving BEq, Inhabited, Repr
  open EnvField

/-- The number of nibbles in a trie path. -/
abbrev trie_path_len := Nat

/-- A trie path of at most 64 nibbles — secure state and storage keys are
32-byte hashes, and list tries use short RLP indices. `data` is
high-aligned; `len` preserves leading zeroes and prefixes. -/
structure TriePath where
  data : b256
  len : trie_path_len
  deriving BEq, Inhabited, Repr

/-- A self-contained trie-node encoding shorter than 32 bytes. -/
structure InlineNode where
  data : b256
  len : Nat
  deriving BEq, Inhabited, Repr

/-- A reference to a trie node: empty, inline (encodings under 32 bytes),
or by KECCAK-256 hash (YP Appendix D, Eq. 207). -/
inductive NodeRef where
  | EmptyRef (_ : Unit)
  | InputInlineRef (_ : (StatelessInputSliceAtMost 31))
  | ScratchInlineRef (_ : InlineNode)
  | HashRef (_ : hash)
  deriving Inhabited, BEq, Repr
  open NodeRef

/-- The sixteen child references of a branch, indexed by nibble. -/
abbrev BranchRefs := (Vector NodeRef 16)

/-- A four-bit path element (YP Appendix D). -/
abbrev nibble := (BitVec 4)

/-- A decoded authenticated node. Every borrowed field remains a stateless
input slice, including fields reached through an input-inline child. -/
inductive InputTrieNode where
  | InputLeafNode (_ : (TriePath × StatelessInputSlice))
  | InputExtensionNode (_ : (TriePath × NodeRef))
  | InputBranchNode (_ : (BranchRefs × StatelessInputSlice))
  deriving Inhabited, BEq, Repr
  open InputTrieNode

/-- A byte position in a 32-byte secure key. -/
abbrev b256_index := Nat

/-- A cursor at or immediately after a position in a trie path. -/
abbrev trie_path_cursor := Nat

/-- The two correlated lifecycle choices selected once from the active fork.
Passing this descriptor into the merge keeps both the Sail implementation
and optimized host implementation from independently re-dispatching on the
fork or observing impossible feature combinations. -/
structure TransactionMergeSemantics where
  delete_only_created : Bool
  preserve_selfdestruct_balance : Bool
  deriving BEq, Inhabited, Repr

/-- Checks the Yellow Paper stack precondition for one instruction before it
charges gas or performs side effects. `inputs` is the instruction's
required stack height (delta) and `outputs` is the height it contributes
after consuming those inputs (alpha). This is the single stack-bounds
guard: handler bodies consume and produce operands unchecked behind it. -/
inductive StackValidation where | StackValid | StackUnderflowFailure | StackOverflowFailure
  deriving BEq, Inhabited, Repr
  open StackValidation

/-- Returns the number of 32-byte words covering a byte length. Besides the
exact ceiling division, the result exposes its enclosing byte interval so
affordability proofs can establish host-range bounds without a second
runtime size check. -/
def memory_word_count_relation (k_byte_len : Int) (k_words : Int) : Prop :=
  0 ≤ k_byte_len ∧
  k_words = ((k_byte_len + 31) / 32) ∧
  k_byte_len ≤ (32 * k_words) ∧ (32 * k_words) ≤ (k_byte_len + 31)

/-- `C_mem` (YP §9.4.1): the cumulative memory cost of `words` words. Its
singleton result makes the gas-derived memory bound available to the type
checker without introducing a separate memory-size constant. -/
def memory_cost_relation (k_words : Int) (k_cost : Int) : Prop :=
  0 ≤ k_words ∧ k_cost = (3 * k_words + (k_words * k_words) / 512)

/-- The independent effects of one `SSTORE`: execution gas, the signed
transaction refund, state gas charged, and state gas returned. -/
structure SstoreCosts where
  execution : gas_cost
  refund : gas_refund
  state_charge : gas_cost
  state_credit : state_gas_spill
  deriving BEq, Inhabited, Repr

/-- A precompile's outcome: success and the EVM-visible output bytes.
Failure consumes the frame's gas like any exceptional call. -/
structure PrecompileResult where
  success : Bool
  output : OutputSlice
  deriving BEq, Inhabited, Repr

/-- One constructor per opcode. Immediates are carried inline: `PUSH`
holds its byte width (0–32) and value, `DUP`/`SWAP` hold the index
`n`, `LOG` holds its topic count. The constructor groups are labelled
with the opcode byte range they cover. Decoding code bytes into this
AST is [fetch][]'s job; undefined bytes decode to `INVALID` and halt
exceptionally when executed. -/
inductive ast where
  | STOP (_ : Unit)
  | ADD (_ : Unit)
  | MUL (_ : Unit)
  | SUB (_ : Unit)
  | DIV (_ : Unit)
  | SDIV (_ : Unit)
  | MOD (_ : Unit)
  | SMOD (_ : Unit)
  | ADDMOD (_ : Unit)
  | MULMOD (_ : Unit)
  | EXP (_ : Unit)
  | SIGNEXTEND (_ : Unit)
  | LT (_ : Unit)
  | GT (_ : Unit)
  | SLT (_ : Unit)
  | SGT (_ : Unit)
  | EQ (_ : Unit)
  | ISZERO (_ : Unit)
  | AND (_ : Unit)
  | OR (_ : Unit)
  | XOR (_ : Unit)
  | NOT (_ : Unit)
  | BYTE (_ : Unit)
  | SHL (_ : Unit)
  | SHR (_ : Unit)
  | SAR (_ : Unit)
  | CLZ (_ : Unit)
  | KECCAK256 (_ : Unit)
  | ADDRESS (_ : Unit)
  | BALANCE (_ : Unit)
  | ORIGIN (_ : Unit)
  | CALLER (_ : Unit)
  | CALLVALUE (_ : Unit)
  | CALLDATALOAD (_ : Unit)
  | CALLDATASIZE (_ : Unit)
  | CALLDATACOPY (_ : Unit)
  | CODESIZE (_ : Unit)
  | CODECOPY (_ : Unit)
  | GASPRICE (_ : Unit)
  | EXTCODESIZE (_ : Unit)
  | EXTCODECOPY (_ : Unit)
  | RETURNDATASIZE (_ : Unit)
  | RETURNDATACOPY (_ : Unit)
  | EXTCODEHASH (_ : Unit)
  | BLOCKHASH (_ : Unit)
  | COINBASE (_ : Unit)
  | TIMESTAMP (_ : Unit)
  | NUMBER (_ : Unit)
  | PREVRANDAO (_ : Unit)
  | GASLIMIT (_ : Unit)
  | CHAINID (_ : Unit)
  | SELFBALANCE (_ : Unit)
  | BASEFEE (_ : Unit)
  | BLOBHASH (_ : Unit)
  | BLOBBASEFEE (_ : Unit)
  | SLOTNUM (_ : Unit)
  | POP (_ : Unit)
  | MLOAD (_ : Unit)
  | MSTORE (_ : Unit)
  | MSTORE8 (_ : Unit)
  | SLOAD (_ : Unit)
  | SSTORE (_ : Unit)
  | JUMP (_ : Unit)
  | JUMPI (_ : Unit)
  | PC (_ : Unit)
  | MSIZE (_ : Unit)
  | GAS (_ : Unit)
  | JUMPDEST (_ : Unit)
  | TLOAD (_ : Unit)
  | TSTORE (_ : Unit)
  | MCOPY (_ : Unit)
  | PUSH (_ : (push_width × word))
  | DUP (_ : stack_operation_index)
  | SWAP (_ : stack_operation_index)
  | LOG (_ : log_topic_count)
  | DUPN (_ : byte)
  | SWAPN (_ : byte)
  | EXCHANGE (_ : byte)
  | opcode_CREATE (_ : Unit)
  | CALL (_ : Unit)
  | CALLCODE (_ : Unit)
  | RETURN (_ : Unit)
  | DELEGATECALL (_ : Unit)
  | CREATE2 (_ : Unit)
  | STATICCALL (_ : Unit)
  | REVERT (_ : Unit)
  | INVALID (_ : Unit)
  | SELFDESTRUCT (_ : Unit)
  deriving Inhabited, BEq, Repr
  open ast

/-- The storage-owner identity carried by the interpreter. Canonical backends
retain the semantic address; optimized C refines this to the account row
and its storage range/generation. -/
abbrev AccountId := Nat

/-- A stable row identifier in an account's optimized storage table. -/
abbrev StorageId := Nat

/-- The number of storage rows belonging to an optimized account row. -/
abbrev StorageCount := Nat

/-- A generation token that invalidates storage rows after an account clear. -/
abbrev StorageGeneration := Nat

/-- The semantic account identity carried while executing one frame. -/
structure AccountExecutionContext where
  address : address
  deriving BEq, Inhabited, Repr

/-- A decreasing bound for the non-recursive interpreter's complete call tree. -/
abbrev call_tree_steps := Nat

/-- The behavior selected by one member of the closed CALL-family algebra.
Interpreting `CallKind` once keeps operand decoding, value transfer, child
identity, and static-context construction coupled instead of re-matching
the tag independently at every use site. -/
structure CallSemantics where
  takes_value : Bool
  transfers_value : Bool
  uses_target_address : Bool
  inherits_caller_and_value : Bool
  enters_static_context : Bool
  deriving BEq, Inhabited, Repr

/-- The behavior selected by one member of the closed CREATE-family algebra.
Interpreting `CreateKind` once keeps operand decoding, hashing charges, and
address derivation coupled rather than passing an unexplained boolean
through the shared creation path. -/
structure CreateSemantics where
  uses_salt : Bool
  deriving BEq, Inhabited, Repr

/-- The refund available when one EIP-7702 authorization targets an existing
account. -/
abbrev authorization_refund_per_item : Int := 12500

/-- The bounded refund contributed by one EIP-7702 authorization. -/
abbrev authorization_item_refund := Nat

/-- The aggregate EIP-7702 refund admitted by one transaction. -/
abbrev authorization_refund := Nat

/-- Intrinsic transaction charges split into Amsterdam execution gas, state
gas, and the calldata floor. -/
structure IntrinsicGasCost where
  execution : gas_cost
  state : gas_cost
  calldata_floor : gas_cost
  deriving BEq, Inhabited, Repr

/-- The intrinsic, blob, and upfront costs established during validation. -/
structure TransactionCosts where
  intrinsic_execution : gas_cost
  intrinsic_state : gas_cost
  calldata_floor : gas_cost
  blob_gas : transaction_blob_gas
  blob_fee : word
  upfront : word
  deriving BEq, Inhabited, Repr

/-- Values established before entering the top-level transaction frame. -/
structure TxUpfrontResult where
  authorization_refund : authorization_refund
  create_target_prestate_empty : Bool
  deriving BEq, Inhabited, Repr

/-- The maximum Amsterdam recipient-side intrinsic execution charge. -/
abbrev amsterdam_recipient_cost := Nat

/-- The outcome of top-level dispatch preparation: whether the frame is
ready to run, and whether a call recipient delegated, which disables
direct precompile dispatch. -/
structure TransactionPreparation where
  ready : Bool
  delegated : Bool
  deriving BEq, Inhabited, Repr

/-- The depth of a branch node in a fixed 64-nibble secure key. -/
abbrev trie_depth := Nat

/-- A cursor through the at-most-65 positions used by hex-evm_prefix decoding. -/
abbrev hex_prefix_cursor := Nat

/-- A leaf value retained by trie assembly. Authenticated witness and
transaction leaves borrow immutable input bytes; newly encoded state,
receipt, and withdrawal leaves borrow the scratch arena. -/
inductive TrieLeafValue where
  | InputTrieLeaf (_ : StatelessInputSlice)
  | ScratchTrieLeaf (_ : ScratchSlice)
  deriving Inhabited, BEq, Repr
  open TrieLeafValue

/-- The RLP payload of a branch contains sixteen child references of at most
33 bytes and one empty value byte. -/
abbrev branch_content_length := Nat

/-- A compact presence bitset for the sixteen children of a branch. -/
abbrev branch_mask := (BitVec 16)

/-- A decoded node freshly encoded in scratch during canonical rebuilding. -/
inductive ScratchTrieNode where
  | ScratchLeafNode (_ : (TriePath × ScratchSlice))
  | ScratchExtensionNode (_ : (TriePath × NodeRef))
  | ScratchBranchNode (_ : (BranchRefs × ScratchSlice))
  deriving Inhabited, BEq, Repr
  open ScratchTrieNode

/-- A pending change at a trie key: a put of new leaf bytes, or a
delete. -/
inductive TrieChange where
  | TriePut (_ : ScratchSlice)
  | TrieDelete (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieChange

/-- An update: a full-path key and its change. Sources yield updates in
ascending key order. -/
structure TrieUpdate where
  key : TriePath
  change : TrieChange
  deriving BEq, Inhabited, Repr

/-- The closed pull-source algebra for ordered trie updates. Each variant owns
an independently opened host iterator and [trie_update_source_next][] is
its sole interpreter, allowing one cursor and rebuild algorithm without
function-valued callbacks. -/
inductive TrieUpdateSource where
  | StorageTrieUpdates (_ : address)
  | ChangedAccountTrieUpdates (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieUpdateSource

/-- One pull from an ordered update source. `update` is meaningful exactly
when `available` is true. -/
structure TrieUpdateFetch where
  available : Bool
  update : TrieUpdate
  deriving BEq, Inhabited, Repr

/-- The active update's position relative to the subtree currently consuming
it. An under-evm_prefix state carries only the unconsumed key suffix. A
beyond-evm_prefix state carries the absolute common-evm_prefix depth of the update
just consumed and its already loaded successor, allowing recursive callers
to unwind directly to their divergence point. -/
inductive TrieUpdateRelation where
  | UpdateUnderPrefix (_ : TriePath)
  | UpdateBeyondPrefix (_ : trie_path_len)
  | UpdateSourceExhausted (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieUpdateRelation

/-- A consuming cursor over an ordered update source. `current` is the active
item, not a lookahead: it remains owned by the cursor until
[trie_updates_pop][] consumes it and loads its successor exactly once. -/
structure TrieUpdateCursor where
  source : TrieUpdateSource
  current : TrieUpdate
  relation : TrieUpdateRelation
  deriving BEq, Inhabited, Repr

/-- An item's payload: no subtree at all, a live leaf, a known-branch
reference (extension children are always branches), or a subtree
reference of unknown kind. The empty member realizes YP Eq. 207's
`n(I,i) = () if I = {}` directly, so recursive assembly is total; the
reference distinction permits untouched hashes to stay opaque. -/
inductive TrieItemValue where
  | EmptySubtree (_ : Unit)
  | LeafItem (_ : TrieLeafValue)
  | BranchItem (_ : NodeRef)
  | SubtreeItem (_ : NodeRef)
  deriving Inhabited, BEq, Repr
  open TrieItemValue

/-- A sorted-stream item: a path and its payload. -/
structure TrieItem where
  path : TriePath
  value : TrieItemValue
  deriving BEq, Inhabited, Repr

/-- Child references accumulated while one recursive branch is assembled.
`only` retains the structural item when exactly one child survives, so
canonical leaf/extension collapse does not need to reopen an encoded
hash. -/
structure TrieChildren where
  mask : branch_mask
  children : BranchRefs
  only : TrieItem
  count : Nat
  deriving BEq, Inhabited, Repr

/-- The root of the trie anchored at `base_root` after applying the
ordered update stream. This is the only public root computation:
witness-native and fail-closed — the walker resolves every touched
hash reference in the witness node-db and any missing node calls
`fatal_error(WitnessDeficient)`; otherwise the builder recomposes the
emitted stream canonically.

Theorem-shaped remark: restricted to an empty base
(`base_root = EMPTY_TRIE_ROOT`), the walker is the identity on the
live update leaves and `trie_root` computes `TRIE(I)` of Appendix D
directly — an empty base contains no hash references, so the node-db
is never consulted and no failure path can fire. The native
(full-state) backend exercises exactly this restriction: same
implementation, different input. -/
structure TrieRootResult where
  root : hash
  changed : Bool
  deriving BEq, Inhabited, Repr

/-- A supported indexed-trie collection maximum. Transaction lists provide the
largest schema bound among transactions, receipts, and withdrawals. -/
def rlp_index_valid_maximum (k_maximum : Int) : Prop :=
  0 < k_maximum ∧ k_maximum ≤ transaction_count_bound

/-- One numeric index and its canonical trie key. The numeric index addresses
the source collection directly; the key places that item in the trie. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
structure RlpIndexItem (k_maximum : Nat) where
  index : Nat
  key : TriePath
  deriving BEq, Inhabited, Repr

/-- The item count, next canonical-key position, and its cached lookup
descriptor. Each `rlp(index)` key is therefore constructed exactly once.
`current` is meaningful iff `position < count`; the pair already carries
the exhaustion state, so no separate presence wrapper exists. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
structure RlpIndexCursor (k_maximum : Nat) where
  count : Nat
  position : Nat
  current : (RlpIndexItem k_maximum)
  deriving BEq, Inhabited, Repr

/-- The minimal nonzero byte width of a supported RLP list index. -/
abbrev rlp_index_byte_width := Nat

/-- A numeric index admitted by an execution-payload indexed trie. -/
abbrev rlp_index := Nat

/-- Every variable region of the input, resolved once before decoding.
Consumers receive explicit source spans instead of re-reading nested
SSZ offset tables. -/
structure StatelessInputRef where
  protocol : ProtocolProfile
  new_payload_request : StatelessInputSlice
  execution_payload : (StatelessInputSliceAtLeast 540)
  versioned_hashes : StatelessInputSlice
  deposits : StatelessInputSlice
  withdrawal_requests : StatelessInputSlice
  consolidation_requests : StatelessInputSlice
  builder_deposit_requests : StatelessInputSlice
  builder_exit_requests : StatelessInputSlice
  extra_data : (StatelessInputSliceAtMost (2 ^ 5))
  transactions : TransactionListRef
  withdrawals : WithdrawalListRef
  block_access_list : (StatelessInputSliceAtMost (2 ^ 30))
  witness_state : WitnessNodeListRef
  witness_codes : WitnessCodeListRef
  witness_headers : WitnessHeaderListRef
  chain_config : StatelessInputSlice
  public_keys : StatelessInputSlice
  deriving BEq, Inhabited, Repr

/-- A sequential position within one SSZ container's variable region. The
container slice carries the region base and limit, so offsets cannot be
mixed with those of an enclosing container. -/
structure SszContainerCursor where
  bytes : StatelessInputSlice
  current : source_pointer
  deriving BEq, Inhabited, Repr

/-- The parent-header fields required to initialize execution context. -/
structure ParentHeaderFields where
  parent_hash : hash
  state_root : hash
  base_fee : word
  blob_gas_used : blob_gas_used
  excess_blob_gas : excess_blob_gas
  have_parent : Bool
  have_state : Bool
  have_base_fee : Bool
  have_blob_gas : Bool
  have_excess_blob_gas : Bool
  deriving BEq, Inhabited, Repr

/-- A bounded parent-header field position, including the unused-field sentinel. -/
abbrev parent_header_field_cursor := Nat

/-- The running authentication and execution context of the witness header
chain. -/
structure WitnessHeaderIndex where
  cursor : WitnessHeaderListCursor
  previous_hash : hash
  valid : Bool
  parent_state_root : hash
  parent_base_fee_per_gas : word
  parent_blob_gas_used : blob_gas_used
  parent_excess_blob_gas : excess_blob_gas
  parent_fields_valid : Bool
  deriving BEq, Inhabited, Repr

/-- One execution-ordered sequence of length-prefixed encoded receipts in the
scratch arena. The records contain no Sail list or aggregate receipt
values: each push writes an eight-byte little-endian length followed by
the canonical trie value. -/
structure ReceiptRecordsRef where
  bytes : ScratchSlice
  count : transaction_count
  deriving BEq, Inhabited, Repr

/-- The closed source algebra for Ethereum's three index-keyed payload tries.
[indexed_trie_begin][] and [indexed_trie_pop][] interpret it as count and
value operations, allowing one recursive trie builder without storing
function values or duplicating the traversal. -/
inductive IndexedTrieSource where
  | IndexedTransactions (_ : TransactionListRef)
  | IndexedWithdrawals (_ : WithdrawalListRef)
  | IndexedReceipts (_ : ReceiptRecordsRef)
  deriving Inhabited, BEq, Repr
  open IndexedTrieSource

/-- Canonical key traversal plus the receipt-only record state. `receipt_zero`
bridges the sole difference between numeric receipt storage order and
lexical `rlp(index)` order; `receipt_remaining` otherwise advances once. -/
structure IndexedTrieCursor where
  keys : (RlpIndexCursor (2 ^ 20))
  receipt_zero : ScratchSlice
  receipt_remaining : ScratchSlice
  deriving BEq, Inhabited, Repr

/-- Everything block validation needs from a successfully executed body: gas
and blob-gas totals, the post-execution receipts root, and the block's
retained receipt-log range. EIP-7685 requests are validated where they
are collected rather than carried in the result. -/
structure BlockExecutionResult where
  header_gas_used : block_gas
  execution_gas_used : block_gas
  state_gas_used : block_gas
  blob_gas_used : blob_gas_used
  first_tx_recipient : address
  receipts_root : hash
  logs : LogSeriesRef
  deriving BEq, Inhabited, Repr

/-- The two independent block gas dimensions introduced by Amsterdam. Earlier
forks use only `execution`; `state` remains zero. Indexing the accumulator
by the concrete header limit makes an over-limit intermediate
unrepresentable after transaction admission. -/
def block_gas_usage_relation (k_limit : Int) (k_execution : Int) (k_state : Int) (k_receipts : Int)
  : Prop :=
  0 ≤ k_limit ∧
  k_limit ≤ block_gas_limit_bound ∧
  0 ≤ k_execution ∧
  k_execution ≤ k_limit ∧
  0 ≤ k_state ∧
  k_state ≤ k_limit ∧ 0 ≤ k_receipts ∧ k_receipts ≤ (k_execution + k_state)

/-- The exact execution, state, and receipt gas accumulated under one block
header gas limit. -/
/- Type quantifiers: k_limit : Nat, k_execution : Nat, k_state : Nat, k_receipts : Nat, (block_gas_usage_relation k_limit k_execution k_state k_receipts) -/
structure BlockGasUsageFields (k_limit : Nat) (k_execution : Nat) (k_state : Nat) (k_receipts : Nat)
  where
  deriving BEq, Inhabited, Repr

@[simp] def BlockGasUsageFields.execution {k_limit : Nat} {k_execution : Nat} {k_state : Nat}
  {k_receipts : Nat} (_ : (BlockGasUsageFields k_limit k_execution k_state k_receipts)) : Nat :=
  k_execution
@[simp] def BlockGasUsageFields.state {k_limit : Nat} {k_execution : Nat} {k_state : Nat}
  {k_receipts : Nat} (_ : (BlockGasUsageFields k_limit k_execution k_state k_receipts)) : Nat :=
  k_state
@[simp] def BlockGasUsageFields.receipts {k_limit : Nat} {k_execution : Nat} {k_state : Nat}
  {k_receipts : Nat} (_ : (BlockGasUsageFields k_limit k_execution k_state k_receipts)) : Nat :=
  k_receipts

/-- A block gas accumulator existentially hiding its current totals while
retaining their relationship to the concrete header limit. -/
abbrev BlockGasUsageFor (k_limit : Int) :=
  (Sigma fun (k_execution : Nat) =>
  (Sigma fun (k_state : Nat) =>
  (Sigma fun (k_receipts : Nat) => (BlockGasUsageFields k_limit k_execution k_state k_receipts))))

/-- The largest Merkle depth required by the supported execution-layer SSZ
schemas. ByteList[2^30] is the widest one, with 2^25 chunks. -/
abbrev htr_depth := Nat

/-- A leaf count in any supported execution-layer SSZ Merkle tree. -/
abbrev htr_leaf_count := Nat

/-- The closed fixed-width leaf-operation algebra carried through the generic
request-list recursion. [htr_request_leaf][] interprets it at depth zero,
replacing a higher-order leaf function without duplicating merkleization. -/
inductive HtrRequestKind where | HtrDeposit | HtrWithdrawalRequest | HtrConsolidationRequest | HtrBuilderDepositRequest | HtrBuilderExitRequest
  deriving BEq, Inhabited, Repr
  open HtrRequestKind

inductive Register : Type where
  | k_current_transaction_epoch
  | k_tx
  | k_header
  | k_execution_profile
  | k_chain_id
  | k_n_headers
  | k_parent_state_root
  | scratch_arena
  deriving DecidableEq, Hashable, Repr
open Register

abbrev RegisterType : Register → Type
  | .k_current_transaction_epoch => block_access_index
  | .k_tx => TxEnv
  | .k_header => BlockHeader
  | .k_execution_profile => ExecutionProfile
  | .k_chain_id => chain_identifier
  | .k_n_headers => ancestor_hash_count
  | .k_parent_state_root => hash
  | .scratch_arena => ScratchSlice

instance : Inhabited (RegisterRef RegisterType BlockHeader) where
  default := .Reg k_header
instance : Inhabited (RegisterRef RegisterType ExecutionProfile) where
  default := .Reg k_execution_profile
instance : Inhabited (RegisterRef RegisterType ScratchSlice) where
  default := .Reg scratch_arena
instance : Inhabited (RegisterRef RegisterType TxEnv) where
  default := .Reg k_tx
instance : Inhabited (RegisterRef RegisterType ancestor_hash_count) where
  default := .Reg k_n_headers
instance : Inhabited (RegisterRef RegisterType block_access_index) where
  default := .Reg k_current_transaction_epoch
instance : Inhabited (RegisterRef RegisterType chain_identifier) where
  default := .Reg k_chain_id
instance : Inhabited (RegisterRef RegisterType hash) where
  default := .Reg k_parent_state_root
abbrev exception := Unit

abbrev SailM := PreSailM RegisterType trivialChoiceSource exception
abbrev SailME := PreSailME RegisterType trivialChoiceSource exception
