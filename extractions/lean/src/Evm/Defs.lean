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

/-- The supported protocol forks, in activation order. Blob-parameter-only
schema forks collapse to their base execution fork in
[protocol_profile][]. -/
inductive Fork where | Frontier | Homestead | Byzantium | Constantinople | Istanbul | Berlin | London | Paris | Shanghai | Cancun | Prague | Osaka | Amsterdam
  deriving BEq, Inhabited, Repr
  open Fork

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

/-- A 20-byte account address (YP §4.1). -/
abbrev address := (Vector byte 20)

/-- A KECCAK-256 / storage-key sized digest. -/
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

/-- An execution block number. The Yellow Paper header scalar and EIP-1559
arithmetic impose no fixed-width semantic bound; the stateless-input SSZ
transport performs its own `uint64` decoding. -/
abbrev block_number := Nat

/-- An execution block timestamp in seconds. As with the Yellow Paper header
scalar, the canonical semantic value is a natural; SSZ width is checked at
the input boundary rather than becoming the protocol type. -/
abbrev block_timestamp := Nat

/-- Largest per-block blob count among the supported schedules: BPO2's
`MAX_BLOBS_PER_BLOCK = 21` (EIP-8135). -/
abbrev blob_count_bound : Int := 21

/-- Largest per-transaction blob count among supported forks: Prague's
`MAX_BLOB_GAS_PER_BLOCK / GAS_PER_BLOB = 9` (EIP-7691). -/
abbrev transaction_blob_count_bound : Int := 9

/-- Largest target blob count among supported schedules: BPO2's target of 14
blobs (EIP-8135). -/
abbrev blob_target_count_bound : Int := 14

/-- Blob gas charged per blob, `2^17` (EIP-4844). -/
abbrev gas_per_blob_value : Int := (2 ^ 17)

/-- A blob count in any block schedule supported by this model. -/
abbrev blob_count := Nat

/-- A target blob count in a supported block schedule. -/
abbrev blob_target_count := Nat

/-- A blob count carried by one EIP-4844 transaction. -/
abbrev transaction_blob_count := Nat

/-- Blob gas used by one supported block. EIP-4844 requires this to be a
multiple of `GAS_PER_BLOB`; the fork-specific maximum is checked against
the active schedule. -/
abbrev blob_gas_used := Nat

/-- Blob gas contributed by one transaction. -/
abbrev transaction_blob_gas := Nat

/-- The accumulating EIP-4844 `excess_blob_gas` header field, encoded as
`uint64` and not bounded by a single block's blob count. -/
abbrev excess_blob_gas_bound : Int := (2 ^ 64 - 1)

/-- An accumulated excess-blob-gas value carried by a block header. -/
abbrev excess_blob_gas := Nat

/-- Largest fee-update fraction in the supported blob schedules: BPO2's
`BLOB_BASE_FEE_UPDATE_FRACTION = 11684671` (EIP-8135). -/
abbrev blob_fee_update_fraction_bound : Int := 11684671

/-- The positive denominator governing excess-blob-gas fee adjustment. -/
abbrev blob_fee_update_fraction := Nat

/-- Largest chain identifier admitted by the typed-transaction wire decoder. -/
abbrev chain_identifier_bound : Int := (2 ^ 64 - 1)

/-- A chain identifier. Typed-transaction chain identifiers and the stateless
chain configuration are decoded as unsigned 64-bit integers. -/
abbrev chain_identifier := Nat

/-- The ordinal of a fork in this model's thirteen-member [Fork][type-Fork]
enumeration. This is a model structural bound, not a wire constraint. -/
abbrev protocol_fork_index := Nat

/-- A beacon-chain slot number, explicitly a `uint64` in EIP-7843. -/
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

/-- The number of entries in a host-backed collection without a tighter
schema-specific bound. Host tables are addressed by 64-bit ordinals in
every executable build; protocol and structural counts use their own
tighter singleton/range types instead. -/
abbrev item_count := Nat

/-- An index into a host-backed collection without a tighter schema bound. -/
abbrev item_index := Nat

/-- The nesting depth of an execution frame. -/
abbrev frame_depth := Nat

/-- The immediate-byte width of a PUSH instruction. -/
abbrev push_width := Nat

/-- The number of words on an operand stack. -/
abbrev operand_stack_height := Nat

/-- A zero-based index from the top of the operand stack. -/
abbrev stack_index := Nat

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

/-- The largest one-based precompile identifier admitted by the model. -/
abbrev precompile_id_bound : Int := 256

/-- The one-based identifier of a precompiled contract. -/
abbrev precompile_id := Nat

/-- An active precompile identifier, or zero for an ordinary address. -/
abbrev precompile_selector := Nat

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

/-- An index into the 256 most recent ancestor block hashes. -/
abbrev ancestor_index := Nat

/-- A small positive divisor used for exact protocol arithmetic. -/
abbrev protocol_divisor := Nat

/-- Whether an offset/length pair is representable by the host. This is
vacuous for the canonical natural-number model; optimized builds strengthen
it with their concrete address-space bound. -/
def host_valid_range (k_off : Int) (k_len : Int) : Prop := 0 ≤ k_off ∧ 0 ≤ k_len

/-- Whether a source-backed byte range is representable by the selected
model. This is a semantic representation invariant, not an FFI type. -/
def source_valid_range (k_off : Int) (k_len : Int) : Prop := 0 ≤ k_off ∧ 0 ≤ k_len

/-- Whether one host byte quantity is representable. Unlike
`host_valid_range`, this does not assert that two quantities can be added
without overflow. -/
def host_valid_access (k_value : Int) : Prop := 0 ≤ k_value

/-- Whether a semantic source length is representable by the selected model.
The canonical specification is unbounded; optimized builds strengthen
this representation invariant independently of protocol limits. -/
def source_valid_length (k_value : Int) : Prop := 0 ≤ k_value

/-- A position representable by the host byte-store interface. The canonical
model is unbounded; production builds refine this representation. -/
abbrev host_access := Nat

/-- An absolute byte position in a named source region. -/
abbrev source_pointer := Nat

/-- A byte length or regular-layout count derived from a source region. -/
abbrev source_length := Nat

/-- An opaque ordinal issued by the host rollback journal. The model may store
and return this complete transaction-state snapshot cursor, but cannot
inspect subsystem journals or replay the host's rollback mechanism. -/
abbrev journal_checkpoint := Nat

/-- An absolute byte position in the current EVM memory frame. -/
abbrev memory_pointer := Nat

/-- A length or allocated size in EVM memory. -/
abbrev memory_length := Nat

/-- The invariant carried by every canonical EVM memory range. -/
def memory_valid_range (k_off : Int) (k_len : Int) : Prop := 0 ≤ k_off ∧ 0 ≤ k_len

/-- The indexed fields of an EVM memory range. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (memory_valid_range k_off k_len) -/
structure MemoryRangeFields (k_off : Nat) (k_len : Nat) where
  off : Nat
  len : Nat
  deriving BEq, Inhabited, Repr

/-- A memory range retaining its offset, length, and containment proof. -/
abbrev MemoryRange :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))

/-- An absolute byte position in the code arena. -/
abbrev code_pointer := Nat

/-- A contract-code length. -/
abbrev code_length := Nat

/-- The representation invariant required of executable code. The canonical
model is mathematically unbounded; optimized builds strengthen this with
enough cursor headroom for a complete PUSH32 immediate. -/
def code_valid_length (k_len : Int) : Prop := 0 ≤ k_len

/-- An index into the 256-byte chunks of a JUMPDEST bitmap. -/
abbrev code_chunk_index := Nat

/-- The maximum block gas limit admitted by the execution-payload SSZ
`uint64` field. -/
abbrev block_gas_limit_bound : Int := (2 ^ 64 - 1)

/-- Available gas in a running EVM frame. -/
abbrev gas := Nat

/-- Representation invariant for a value copied from the live frame gas
counter.  The canonical model requires only non-negativity; production
splices may strengthen it from the admitted input and counter lifecycle. -/
def live_gas_valid (k_value : Int) : Prop := 0 ≤ k_value

/-- Gas supplied by a transaction before fork-specific validation. -/
abbrev transaction_gas := Nat

/-- A transient computed charge. Canonically this is an exact natural;
optimized builds use a native representation only after the computation's
semantic bound has been established. -/
abbrev gas_cost := Nat

/-- Intermediate MODEXP affordability factors.  The canonical model keeps
their exact natural values; production splices bound them from the live
gas counter and the at-most-255-bit exponent-head contribution. -/
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

/-- Execution gas temporarily consumed by Amsterdam state charges. EIP-8037
draws spill only from the regular-gas pool, which is capped by EIP-7825. -/
abbrev state_gas_spill := Nat

/-- A block header's gas limit. Its SSZ execution-payload field is `uint64`;
`GASLIMIT` widens this bounded natural into an EVM word when it pushes the
value onto the stack (Yellow Paper equation 147). -/
abbrev block_gas_limit := Nat

/-- Gas consumed by a block. It remains an exact natural during execution and
is subsequently checked against the word-bounded header gas limit. -/
abbrev block_gas := Nat

/-- A fixed gas-schedule value used as an opcode or transaction base cost. -/
abbrev gas_constant := Nat

/-- Absolute lifecycle bound for the signed refund accumulator. -/
abbrev gas_refund_bound : Int := (199 * (2 ^ 64 - 1))

/-- The signed transaction refund accumulator before capping. -/
abbrev gas_refund := Int

/-- Net state gas consumed by one execution frame. A credit can make this
negative until transaction settlement clamps the block-level value at
zero. The canonical model remains mathematically unbounded; production
splices refine this type from the live-counter lifecycle. -/
abbrev frame_state_gas_delta := Int

/-- Combined state-gas delta for the Amsterdam authorization and execution
frames. -/
abbrev state_gas_delta := Int

/-- State gas after adding the intrinsic transaction charge and before the
block-level non-negative/u64 checks. -/
abbrev transaction_state_gas_delta := Int

/-- A small positive divisor used by the gas schedule. -/
abbrev gas_divisor := Nat

/-- The regions of the host interface a [EvmByteSlice][type-EvmByteSlice] may
reference. `StatelessInputSource` is the SSZ stateless-input envelope;
`EvmMemorySource` is the current frame's memory; `CodeSource` is stored
contract code; `LogDataSource` retains log payloads for receipts;
`OutputSource` is frame output frozen at halt; and `ScratchSource` is
the executor's scratch arena. -/
inductive ByteSource where | StatelessInputSource | EvmMemorySource | CodeSource | LogDataSource | OutputSource | ScratchSource
  deriving BEq, Inhabited, Repr
  open ByteSource

/-- The witness-carrying fields of an unmaterialized byte range. -/
/- Type quantifiers: k_off : Nat, k_len : Nat, (source_valid_range k_off k_len) -/
structure EvmByteSliceFields (k_off : Nat) (k_len : Nat) where
  source : ByteSource
  off : Nat
  len : Nat
  deriving BEq, Inhabited, Repr

/-- An unmaterialized byte range: a region, an offset, and a length. A
slice denotes the byte sequence it references; reads resolve in O(1)
without copying. -/
abbrev EvmByteSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))

/-- A source-backed byte range whose length is known statically. -/
abbrev EvmByteSliceLength (k_required : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))

/-- A source-backed byte range whose validated length is at least `minimum`. -/
abbrev EvmByteSliceAtLeast (k_minimum : Int) :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))

/-- The result of making a requested evm_prefix of a byte region available.
Success carries a slice of exactly the requested length. This shared type
deliberately imposes no global region maximum: each region-specific
producer may refine the admissible length independently. -/
/- Type quantifiers: k_required : Nat, 0 ≤ k_required -/
inductive ByteRegionResult (k_required : Nat) where
  | ByteRegionReady (_ : (EvmByteSliceLength k_required))
  | ByteRegionFailed (_ : Unit)
  deriving Inhabited, BEq, Repr
  open ByteRegionResult

/-- A finite list of bytes paired with its construction-time length. -/
structure MaterializedBytes where
  data : (List byte)
  len : source_length
  deriving BEq, Inhabited, Repr

/-- A evm_prefix of a fixed 32-byte vector, in wire order. -/
structure FixedBytes32 where
  data : b256
  len : Nat
  deriving BEq, Inhabited, Repr

/-- A byte-sequence segment: materialized bytes or a region-backed span.
A `list(Bytes)` denotes the concatenation of its segments.

!!! note "Implementation"
    A segment list crosses the host interface whole
    (`ffi/hash_glue.c`); no per-byte crossings occur. -/
inductive Bytes where
  | BytesList (_ : MaterializedBytes)
  | BytesSlice (_ : EvmByteSlice)
  | BytesFixed32 (_ : FixedBytes32)
  deriving Inhabited, BEq, Repr
  open Bytes

/-- Exceptional halts (YP §9.4.2): each consumes all remaining gas and
reverts the frame's state changes. -/
inductive ExceptionKind where | StackUnderflow | StackOverflow | OutOfGas | InvalidOpcode | InvalidJump | StaticViolation | CallDepthExceeded | InsufficientBalance | WriteProtection | InitCodeTooLarge | NonceOverflow | AddressCollision
  deriving BEq, Inhabited, Repr
  open ExceptionKind

/-- The reason a block fails validation; one variant per violated
block-validity rule. -/
inductive BlockError where | InvalidConfig | HeaderChainBroken | RlpDecode | InvalidSignature | InvalidGasLimit | GasUsedExceedsLimit | BlobGasLimitExceeded | ExecutionInvalid | InvalidGasUsed | InvalidBlobGasUsed | InvalidExcessBlobGas | InvalidStateRoot | InvalidReceiptsRoot | InvalidLogsBloom | InvalidBlockHash | InvalidParentHash | BlockAccessListTooLarge | InvalidBlockAccessList | InvalidExecutionRequests | WitnessDeficient
  deriving BEq, Inhabited, Repr
  open BlockError

/-- The single Sail exception type. Block validation throws
`InvalidBlock(reason)` at the failure point, and `main` catches it
once — any failure yields an unsuccessful validation. Sail's C backend
unwinds via `have_exception` return-checks (no `longjmp`), so it is
zkVM-guest safe. -/
inductive exception where
  | InvalidBlock (_ : BlockError)
  deriving Inhabited, BEq, Repr
  open exception

/-- Ordinary frame stops. Only `RETURN` and `REVERT` produce output bytes. -/
inductive HaltKind where
  | HaltStop (_ : Unit)
  | HaltReturn (_ : EvmByteSlice)
  | HaltRevert (_ : EvmByteSlice)
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

/-- Maximum transactions in the execution-payload SSZ list. -/
abbrev transaction_count_bound : Int := (2 ^ 20)

/-- Maximum withdrawals in the execution-payload SSZ list. -/
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

/-- Maximum byte length of execution-payload extra data. -/
abbrev extra_data_length_bound : Int := (2 ^ 5)

/-- Maximum byte length of one encoded transaction envelope. -/
abbrev transaction_length_bound : Int := (2 ^ 30)

/-- Maximum byte length of the block access list. -/
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
  bytes : EvmByteSlice
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

/-- A 256-byte segment of a PUSH-aware `JUMPDEST` bitmap. -/
abbrev JumpdestChunk := (BitVec 256)

/-- A reference to a code's completed `JUMPDEST` analysis; zero denotes
the empty bitmap. -/
abbrev JumpdestRef := (BitVec 64)

/-- A source-backed executable byte span. Its length carries the separate
representation invariant needed by program-counter arithmetic; this is
not a protocol deployment-size limit. -/
abbrev CodeSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))

/-- Executable code: its byte span and resolved `JUMPDEST` table. The
interpreter saves and restores both together; the code hash remains
the stable code-DB key. -/
structure Code where
  bytes : CodeSlice
  jumpdests : JumpdestRef
  deriving BEq, Inhabited, Repr

/-- The complete containment invariant for an RLP field reference. `source`
is normalized to the complete encoded item. RLP content is its suffix, so
the content offset is derived as `source.len - content_len`. -/
def rlp_field_ref_valid (k_source_off : Int) (k_source_len : Int) (k_content_len : Int) : Prop :=
  (source_valid_range k_source_off k_source_len) ∧
  0 ≤ k_content_len ∧ k_content_len ≤ k_source_len

/-- The result of popping one complete field from a cursor. In addition to
the field's own validity, the complete encoding is non-empty and contained
by the cursor being consumed. -/
def rlp_cursor_pop_valid
  (k_source_off : Int) (k_source_len : Int) (k_full_len : Int) (k_content_len : Int) : Prop :=
  (rlp_field_ref_valid k_source_off k_full_len k_content_len) ∧
  0 < k_full_len ∧ k_full_len ≤ k_source_len

/-- The witness-carrying fields of a decoded RLP reference. Both the complete
encoding and its content are statically contained by the source slice. -/
/- Type quantifiers: k_source_off : Nat, k_source_len : Nat, k_content_len : Nat, (rlp_field_ref_valid k_source_off k_source_len k_content_len) -/
structure RlpFieldRef (k_source_off : Nat) (k_source_len : Nat) (k_content_len : Nat) where
  source : (EvmByteSliceFields k_source_off k_source_len)
  is_list : Bool
  content_len : Nat
  deriving BEq, Inhabited, Repr

/-- A one-pass RLP cursor is the unconsumed suffix of its source. Popping an
item advances both its offset and its decreasing remaining length without
carrying a second position field. -/
abbrev RlpCursor (k_source_off : Nat) (k_source_len : Nat) :=
  (EvmByteSliceFields k_source_off k_source_len)

/-- A fork's blob parameters (EIP-4844/EIP-7691): target and maximum blob
counts per block, and the base-fee update fraction. -/
structure BlobSchedule where
  target : blob_target_count
  max : blob_count
  base_fee_update_fraction : blob_fee_update_fraction
  deriving BEq, Inhabited, Repr

/-- Execution rules and protocol constants selected by the stable fork byte
in the stateless-input schema identifier. -/
structure ProtocolProfile where
  fork : Fork
  blob_schedule : BlobSchedule
  deriving BEq, Inhabited, Repr

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

/-- The EIP-2718 envelope type: the single transaction discriminant
The type-derived predicates at the end of this module read it — no
boolean flags are stored. -/
inductive TxType where | LegacyTx | AccessListTx | FeeMarketTx | BlobTx | SetCodeTx
  deriving BEq, Inhabited, Repr
  open TxType

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

/-- The source-backed EIP-4844 versioned blob hashes of a transaction. -/
structure BlobHashes where
  bytes : EvmByteSlice
  count : transaction_blob_count
  deriving BEq, Inhabited, Repr

/-- A byte span contained by one SSZ transaction envelope. This structural
bound is independent of fork-specific calldata and initcode limits. -/
abbrev TransactionEvmByteSlice :=
  (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))

/-- A collection count whose entries are encoded inside one transaction. -/
abbrev transaction_item_count := Nat

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
structure Transaction where
  tx_type : TxType
  sender : address
  nonce : word
  chain_id : chain_identifier
  gas_limit : transaction_gas
  is_create : Bool
  recipient : address
  value : word
  raw : EvmByteSlice
  input_src : TransactionEvmByteSlice
  access_list_addresses : (List address)
  access_list_address_count : transaction_item_count
  access_list_slots : (List StorageKey)
  access_list_slot_count : transaction_item_count
  max_fee : word
  max_blob_fee : word
  max_priority_fee : word
  authorizations : (List Authorization)
  authorization_count : transaction_item_count
  blob_hashes : BlobHashes
  pubkey : EvmByteSlice
  signing_hash : hash
  sig_v : word
  sig_r : word
  sig_s : word
  deriving BEq, Inhabited, Repr

/-- A log record (YP §4.4.1, the log series): emitting address, 0–4
topics (`LOG0`–`LOG4`), and the data bytes. -/
structure LogEntry where
  address : address
  topics : (List word)
  data : EvmByteSlice
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
structure Receipt where
  tx_type : TxType
  success : Bool
  gas_used : gas
  execution_gas : gas
  state_gas : gas
  logs : (List LogEntry)
  deriving BEq, Inhabited, Repr

/-- The 2048-bit logs bloom filter (YP §4.4.1), as 256 bytes. -/
abbrev LogsBloom := (Vector byte 256)

/-- The execution-payload header fields the model reads and validates
(YP §4.4). `extra_data` stays a reference into the payload — it is
RLP-encoded whole for the header hash and never inspected. -/
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
  logs_bloom : LogsBloom
  fee_recipient : address
  parent_hash : hash
  parent_beacon_block_root : hash
  slot_number : slot_number
  extra_data : EvmByteSlice
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
  block_access_list : EvmByteSlice
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

/-- Raw EIP-7685 request bodies. The request-type bytes are added only
while computing the header commitment; the SSZ input carries one field
per type. -/
structure ExecutionRequests where
  deposits : EvmByteSlice
  withdrawals : EvmByteSlice
  consolidations : EvmByteSlice
  builder_deposits : EvmByteSlice
  builder_exits : EvmByteSlice
  deriving BEq, Inhabited, Repr

/-- The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. -/
structure TxEnv where
  origin : address
  gas_price : word
  blob_hashes : BlobHashes
  deriving BEq, Inhabited, Repr

/-- The values established by successful up-front transaction validation
(YP §6.2): sender, pre-state nonce, intrinsic gas, blob fee, and the
EIP-1559 effective gas and priority prices. Invalid transactions throw
before this value is constructed. -/
structure TxValidity where
  sender : address
  nonce_before : account_nonce
  gas_limit : block_gas_limit
  intrinsic_execution_gas : gas_cost
  intrinsic_state_gas : gas_cost
  calldata_floor : gas_cost
  blob_fee : word
  gas_price : word
  priority_fee : word
  deriving BEq, Inhabited, Repr

/-- The result of a transaction's top-level frame: success and remaining
gas. -/
structure TxFrameResult where
  success : Bool
  execution_gas_remaining : gas
  state_gas_remaining : gas
  state_gas_used : state_gas_delta
  refund : gas_refund
  deriving BEq, Inhabited, Repr

/-- The four CALL-family execution modes. `Call` is an ordinary call;
`CallCode` combines the caller's storage with the target's code;
`DelegateCall` additionally inherits the caller and value; and
`StaticCall` enters a read-only frame. -/
inductive CallKind where | Call | CallCode | DelegateCall | StaticCall
  deriving BEq, Inhabited, Repr
  open CallKind

/-- The per-frame call message (YP §8, the I tuple): caller, executing
address, code owner, value, calldata length, static flag, and call
depth. -/
structure Message where
  caller : address
  code_address : address
  address : address
  value : word
  state_gas_reservoir : gas
  is_static : Bool
  depth : frame_depth
  deriving BEq, Inhabited, Repr

/-- The suspended parent-frame state restored after nested execution. -/
structure FrameCheckpoint where
  state : journal_checkpoint
  pc : code_pointer
  gas_remaining : gas
  state_gas_remaining : gas
  state_gas_spilled : state_gas_spill
  refund : gas_refund
  status : FrameStatus
  message : Message
  call_depth : frame_depth
  code : Code
  calldata : EvmByteSlice
  memory : EvmByteSlice
  deriving BEq, Inhabited, Repr

/-- The suspended parent information needed after a message call returns. -/
structure CallContinuation where
  checkpoint : FrameCheckpoint
  return_offset : memory_pointer
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

/-- The partial JUMPDEST bitmap and its location while code is scanned. -/
structure CodeAnalysis where
  chunk : JumpdestChunk
  chunk_index : code_chunk_index
  chunk_offset : Nat
  deriving BEq, Inhabited, Repr

/-- An RLP byte count that can be materialized in the scratch arena. The
canonical model is unbounded; optimized builds refine this representation
and validate recursive collection totals at this boundary. -/
abbrev rlp_scratch_length := Nat

/-- Values accepted by the generic natural-number RLP helpers. Canonical RLP
is unbounded; optimized builds refine values entering the native encoder. -/
def rlp_natural_valid (k_value : Int) : Prop := 0 ≤ k_value

/-- Natural values that can be incremented while counting encoded bytes. -/
def rlp_natural_increment_valid (k_value : Int) : Prop := 0 ≤ k_value

/-- Encoded width returned by the generic natural-number RLP helper. -/
abbrev rlp_natural_size := Nat

/-- A decoded access list and the counts needed for intrinsic gas. -/
/- Type quantifiers: k_address_bound : Nat, k_slot_bound : Nat, (source_valid_length k_address_bound)
  ∧ (source_valid_length k_slot_bound) -/
structure AccessListDecode (k_address_bound : Nat) (k_slot_bound : Nat) where
  addresses : (List address)
  storage_slots : (List StorageKey)
  address_count : Nat
  slot_count : Nat
  deriving BEq, Inhabited, Repr

/-- A decoded EIP-7702 authorization sequence and its item count. -/
/- Type quantifiers: k_bound : Nat, (source_valid_length k_bound) -/
structure AuthorizationDecode (k_bound : Nat) where
  authorizations : (List Authorization)
  count : Nat
  deriving BEq, Inhabited, Repr

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

/-- The environment fields opcodes read through [k_env][]. -/
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
  | InlineRef (_ : InlineNode)
  | HashRef (_ : hash)
  deriving Inhabited, BEq, Repr
  open NodeRef

/-- The sixteen child references of a branch, indexed by nibble. -/
abbrev BranchRefs := (Vector NodeRef 16)

/-- A four-bit path element (YP Appendix D). -/
abbrev nibble := (BitVec 4)

/-- A decoded trie node. Malformed node bytes throw
`InvalidBlock(RlpDecode)`. -/
inductive TrieNode where
  | LeafNode (_ : (TriePath × EvmByteSlice))
  | ExtensionNode (_ : (TriePath × NodeRef))
  | BranchNode (_ : (BranchRefs × EvmByteSlice))
  deriving Inhabited, BEq, Repr
  open TrieNode

/-- A byte position in a 32-byte secure key. -/
abbrev b256_index := Nat

/-- A cursor at or immediately after a position in a trie path. -/
abbrev trie_path_cursor := Nat

/-- A strictly positive Taylor-series index in the fake-exponential equation. -/
abbrev fake_exponential_index := Nat

/-- The fractional numerator of a denominator-scaled blob-fee value. -/
abbrev blob_fee_remainder := Nat

/-- An exact denominator-scaled blob-fee value split into whole and remainder. -/
structure ScaledBlobValue where
  whole : word
  remainder : blob_fee_remainder
  deriving BEq, Inhabited, Repr

/-- An exact quotient and remainder in the EVM-word domain. -/
structure BlobProductDivMod where
  quotient : word
  remainder : word
  deriving BEq, Inhabited, Repr

/-- A single memory operand coupled to the high-water mark and gas cost that
make the range materializable. -/
/- Type quantifiers: k_available : Nat, (live_gas_valid k_available) -/
structure MemoryExpansion (k_available : Nat) where
  range : MemoryRange
  required_size : memory_length
  cost : Nat
  deriving BEq, Inhabited, Repr

/-- Two memory operands sharing one expansion high-water mark and gas cost. -/
/- Type quantifiers: k_available : Nat, (live_gas_valid k_available) -/
structure MemoryPairExpansion (k_available : Nat) where
  left : MemoryRange
  right : MemoryRange
  required_size : memory_length
  cost : Nat
  deriving BEq, Inhabited, Repr

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
  output : EvmByteSlice
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
  | CREATE (_ : Unit)
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
  authorization_refund : gas_refund
  create_target_prestate_empty : Bool
  deriving BEq, Inhabited, Repr

/-- Transaction-local bookkeeping for Amsterdam EIP-7702 authorization
charges. The lists contain only successfully validated authorities. -/
structure AmsterdamAuthorizationState where
  seen_valid_authorities : (List address)
  originally_delegated : (List address)
  delegation_set_for : (List address)
  deriving BEq, Inhabited, Repr

/-- The maximum Amsterdam recipient-side intrinsic execution charge. -/
abbrev amsterdam_recipient_cost := Nat

/-- The depth of a branch node in a fixed 64-nibble secure key. -/
abbrev trie_depth := Nat

/-- A cursor through the at-most-65 positions used by hex-evm_prefix decoding. -/
abbrev hex_prefix_cursor := Nat

/-- The RLP payload of a branch contains sixteen child references of at most
33 bytes and one empty value byte. -/
abbrev branch_content_length := Nat

/-- A pending change at a trie key: a put of new leaf bytes, or a
delete. -/
inductive TrieChange where
  | TriePut (_ : EvmByteSlice)
  | TrieDelete (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieChange

/-- An update: a full-path key and its change. Sources yield updates in
ascending key order. -/
structure TrieUpdate where
  key : TriePath
  change : TrieChange
  deriving BEq, Inhabited, Repr

/-- A pull source for ordered trie updates. Each variant owns an independently
opened host iterator. -/
inductive TrieUpdateSource where
  | StorageTrieUpdates (_ : address)
  | ChangedAccountTrieUpdates (_ : Unit)
  | CachedAccountTrieUpdates (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieUpdateSource

/-- One-item lookahead over an ordered update source. -/
structure TrieUpdateCursor where
  source : TrieUpdateSource
  pending : (Option TrieUpdate)
  deriving BEq, Inhabited, Repr

/-- An item's payload: a live leaf, a known-branch reference (extension
children are always branches), or a subtree reference of unknown kind.
The distinction permits untouched hashes to stay opaque. -/
inductive TrieItemValue where
  | LeafItem (_ : EvmByteSlice)
  | BranchItem (_ : NodeRef)
  | SubtreeItem (_ : NodeRef)
  deriving Inhabited, BEq, Repr
  open TrieItemValue

/-- A sorted-stream item: a path and its payload. -/
structure TrieItem where
  path : TriePath
  value : TrieItemValue
  deriving BEq, Inhabited, Repr

/-- One open branch in the incremental canonical trie builder. -/
structure TrieBranchFrame where
  depth : trie_depth
  mask : (BitVec 16)
  children : BranchRefs
  deriving BEq, Inhabited, Repr

/-- The open branch stack and completed root of a streaming trie build. -/
structure TrieBuilder where
  frames : (List TrieBranchFrame)
  root : NodeRef
  complete : Bool
  deriving BEq, Inhabited, Repr

/-- A one-item lookahead buffer feeding the incremental trie builder. -/
structure TrieItemSink where
  builder : TrieBuilder
  pending : (Option TrieItem)
  deriving BEq, Inhabited, Repr

/-- A supported indexed-trie collection maximum. Transaction lists provide the
largest schema bound among transactions, receipts, and withdrawals. -/
def rlp_index_valid_maximum (k_maximum : Int) : Prop :=
  0 < k_maximum ∧ k_maximum ≤ transaction_count_bound

/-- The item count and next canonical-key position of a schema-bounded indexed
trie. Both retain the enclosing collection's semantic maximum. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
structure RlpIndexCursor (k_maximum : Nat) where
  count : Nat
  position : Nat
  deriving BEq, Inhabited, Repr

/-- One numeric index, its trie key, and the following key when present. -/
/- Type quantifiers: k_maximum : Nat, (rlp_index_valid_maximum k_maximum) -/
structure RlpIndexItem (k_maximum : Nat) where
  index : Nat
  key : TriePath
  next_key : (Option TriePath)
  deriving BEq, Inhabited, Repr

/-- The minimal nonzero byte width of a supported RLP list index. -/
abbrev rlp_index_byte_width := Nat

/-- The root of the trie anchored at `base_root` after applying the
ordered update stream. This is the only public root computation:
witness-native and fail-closed — the walker resolves every touched
hash reference in the witness node-db and any missing node throws
`InvalidBlock(WitnessDeficient)`; otherwise the builder recomposes the
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

/-- Every variable region of the input, resolved once before decoding.
Consumers receive explicit source spans instead of re-reading nested
SSZ offset tables. -/
structure StatelessInputRef where
  protocol : ProtocolProfile
  new_payload_request : EvmByteSlice
  execution_payload : (EvmByteSliceAtLeast 540)
  versioned_hashes : EvmByteSlice
  deposits : EvmByteSlice
  withdrawal_requests : EvmByteSlice
  consolidation_requests : EvmByteSlice
  builder_deposit_requests : EvmByteSlice
  builder_exit_requests : EvmByteSlice
  extra_data : EvmByteSlice
  transactions : TransactionListRef
  withdrawals : WithdrawalListRef
  block_access_list : EvmByteSlice
  witness_state : WitnessNodeListRef
  witness_codes : WitnessCodeListRef
  witness_headers : WitnessHeaderListRef
  chain_config : EvmByteSlice
  public_keys : EvmByteSlice
  deriving BEq, Inhabited, Repr

/-- A sequential position within one SSZ container's variable region. The
container slice carries the region base and limit, so offsets cannot be
mixed with those of an enclosing container. -/
structure SszContainerCursor where
  bytes : EvmByteSlice
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

/-- A receipt retained until its lexicographic trie successor is known. -/
structure PendingReceipt where
  index : transaction_count
  cumulative_gas_used : block_gas
  receipt : Receipt
  deriving BEq, Inhabited, Repr

/-- The streaming receipts-trie builder and block-wide receipt aggregates. -/
structure ReceiptAccumulator where
  builder : TrieBuilder
  first : (Option PendingReceipt)
  pending : (Option PendingReceipt)
  count : transaction_count
  cumulative_gas_used : block_gas
  bloom : LogsBloom
  deriving BEq, Inhabited, Repr

/-- Everything block validation needs from a successfully executed body: gas
and blob-gas totals, receipts, and the collected EIP-7685 requests. -/
structure BlockExecutionResult where
  header_gas_used : gas
  execution_gas_used : gas
  state_gas_used : gas
  blob_gas_used : blob_gas_used
  first_tx_recipient : address
  receipts_root : hash
  logs_bloom : LogsBloom
  deposits : EvmByteSlice
  requests : ExecutionRequests
  deriving BEq, Inhabited, Repr

/-- A validation failure: the pipeline stage it occurred in and the
violated rule. -/
structure StatelessValidationFailure where
  scope : (BitVec 8)
  reason : BlockError
  deriving BEq, Inhabited, Repr

/-- The verdict of stateless payload verification. -/
inductive StatelessValidationResult where
  | StatelessPayloadValid (_ : Unit)
  | StatelessPayloadInvalid (_ : StatelessValidationFailure)
  deriving Inhabited, BEq, Repr
  open StatelessValidationResult

/-- One level of the Merkle frontier: empty, or holding the pending left
subtree hash for that level. -/
inductive MerkleSlot where
  | EmptyMerkleSlot (_ : Unit)
  | OccupiedMerkleSlot (_ : hash)
  deriving Inhabited, BEq, Repr
  open MerkleSlot

/-- The largest Merkle depth required by the supported execution-layer SSZ
schemas. ByteList[2^30] is the widest one, with 2^25 chunks. -/
abbrev htr_depth := Nat

/-- A leaf count in any supported execution-layer SSZ Merkle tree. -/
abbrev htr_leaf_count := Nat

/-- A streaming Merkle frontier together with the number of leaves already
incorporated. -/
structure MerkleAccumulator where
  frontier : (List MerkleSlot)
  count : htr_leaf_count
  depth : htr_depth
  deriving BEq, Inhabited, Repr

/-- A decoded input paired with its validation verdict. -/
structure GuestValidation where
  input_ref : StatelessInputRef
  valid : Bool
  deriving BEq, Inhabited, Repr

inductive Register : Type where
  | evm_memory
  | returndata
  | calldata
  | frame_code
  | call_depth
  | message
  | frame_status
  | frame_refund
  | state_gas_spilled
  | state_gas_remaining
  | gas_remaining
  | pc
  | k_block_access_index
  | k_tx
  | k_header
  | k_blob_schedule
  | k_fork
  | k_chain_id
  | k_n_headers
  | k_parent_state_root
  | scratch_arena
  deriving DecidableEq, Hashable, Repr
open Register

abbrev RegisterType : Register → Type
  | .evm_memory => EvmByteSlice
  | .returndata => EvmByteSlice
  | .calldata => EvmByteSlice
  | .frame_code => Code
  | .call_depth => frame_depth
  | .message => Message
  | .frame_status => FrameStatus
  | .frame_refund => gas_refund
  | .state_gas_spilled => state_gas_spill
  | .state_gas_remaining => gas
  | .gas_remaining => gas
  | .pc => code_pointer
  | .k_block_access_index => block_access_index
  | .k_tx => TxEnv
  | .k_header => BlockHeader
  | .k_blob_schedule => BlobSchedule
  | .k_fork => Fork
  | .k_chain_id => chain_identifier
  | .k_n_headers => item_count
  | .k_parent_state_root => hash
  | .scratch_arena => EvmByteSlice

instance : Inhabited (RegisterRef RegisterType BlobSchedule) where
  default := .Reg k_blob_schedule
instance : Inhabited (RegisterRef RegisterType BlockHeader) where
  default := .Reg k_header
instance : Inhabited (RegisterRef RegisterType EvmByteSlice) where
  default := .Reg scratch_arena
instance : Inhabited (RegisterRef RegisterType Code) where
  default := .Reg frame_code
instance : Inhabited (RegisterRef RegisterType Fork) where
  default := .Reg k_fork
instance : Inhabited (RegisterRef RegisterType FrameStatus) where
  default := .Reg frame_status
instance : Inhabited (RegisterRef RegisterType Message) where
  default := .Reg message
instance : Inhabited (RegisterRef RegisterType TxEnv) where
  default := .Reg k_tx
instance : Inhabited (RegisterRef RegisterType block_access_index) where
  default := .Reg k_block_access_index
instance : Inhabited (RegisterRef RegisterType chain_identifier) where
  default := .Reg k_chain_id
instance : Inhabited (RegisterRef RegisterType code_pointer) where
  default := .Reg pc
instance : Inhabited (RegisterRef RegisterType frame_depth) where
  default := .Reg call_depth
instance : Inhabited (RegisterRef RegisterType gas) where
  default := .Reg gas_remaining
instance : Inhabited (RegisterRef RegisterType gas_refund) where
  default := .Reg frame_refund
instance : Inhabited (RegisterRef RegisterType hash) where
  default := .Reg k_parent_state_root
instance : Inhabited (RegisterRef RegisterType item_count) where
  default := .Reg k_n_headers
instance : Inhabited (RegisterRef RegisterType state_gas_spill) where
  default := .Reg state_gas_spilled
abbrev SailM := PreSailM RegisterType trivialChoiceSource exception
abbrev SailME := PreSailME RegisterType trivialChoiceSource exception

