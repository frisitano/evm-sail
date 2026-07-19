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

abbrev bit := (BitVec 1)

/-- An exact byte offset, length, or size after representability checks. -/
inductive byte_quantity where
  | ByteQuantity (_ : Nat)
  deriving Inhabited, BEq, Repr
  open byte_quantity

abbrev bits k_n := (BitVec k_n)

/-- A fixed gas-schedule value used as an opcode or transaction base cost. -/
inductive gas_constant where
  | GasConstant (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas_constant

/-- An exact non-negative cost that may exceed all available gas. -/
inductive gas_cost where
  | GasCost (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas_cost

/-- Available frame, transaction, or block gas below the protocol limit. -/
inductive gas where
  | Gas (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas

/-- The signed transaction refund accumulator before capping. -/
inductive gas_refund where
  | GasRefund (_ : Int)
  deriving Inhabited, BEq, Repr
  open gas_refund

/-- The supported protocol forks, in activation order. Blob-parameter-only
forks are not distinct members; they collapse to their base fork (see
[fork_of_protocol_index][]). -/
inductive Fork where | Frontier | Homestead | Byzantium | Constantinople | Istanbul | Berlin | London | Paris | Shanghai | Cancun | Prague | Osaka | Amsterdam
  deriving BEq, Inhabited, Repr
  open Fork

/-- An unsigned quantity carried by an Ethereum protocol field. -/
structure protocol_quantity where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace protocol_quantity
def Valid (x : protocol_quantity) : Prop :=
  0 ≤ x.value ∧ x.value ≤ (2 ^ 64 - 1)
end protocol_quantity

/-- The ordinal assigned to a supported protocol fork. -/
abbrev protocol_fork_index := protocol_quantity

/- Type quantifiers: k_a : Type -/
inductive option (k_a : Type) where
  | Some (_ : k_a)
  | None (_ : Unit)
  deriving Inhabited, BEq, Repr
  open option

/-- An 8-bit byte. -/
abbrev byte := (BitVec 8)

/-- The EVM 256-bit machine word (YP §9.1), nominally separated from
other 256-bit protocol values. -/
inductive word where
  | U256 (_ : (BitVec 256))
  deriving Inhabited, BEq, Repr
  open word

/-- A 20-byte account address (YP §4.1). -/
inductive address where
  | Address (_ : (Vector byte 20))
  deriving Inhabited, BEq, Repr
  open address

/-- A KECCAK-256 / storage-key sized digest. -/
inductive b256 where
  | B256 (_ : (Vector byte 32))
  deriving Inhabited, BEq, Repr
  open b256

/-- The common digest type used by trie, code, and block hashes. -/
abbrev hash := b256

/-- A fixed-width 64-bit limb used to define word arithmetic. -/
abbrev limb := (BitVec 64)

/-- A host address operation's success flag and nominal address result. -/
structure AddressResult where
  success : Bool
  address : address
  deriving BEq, Inhabited, Repr

/-- The quotient and remainder of one limb division. -/
structure LimbDivMod where
  quotient : limb
  remainder : limb
  deriving BEq, Inhabited, Repr

/-- The quotient and remainder of one unsigned word division. -/
structure WordDivMod where
  quotient : word
  remainder : word
  deriving BEq, Inhabited, Repr

/-- A bit count within one limb. -/
structure limb_bit_count where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace limb_bit_count
def Valid (x : limb_bit_count) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 64
end limb_bit_count

/-- A bit count within one EVM word. -/
structure word_bit_count where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace word_bit_count
def Valid (x : word_bit_count) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 256
end word_bit_count

/-- An account transaction-count nonce. -/
abbrev account_nonce := protocol_quantity

/-- An execution block number. -/
abbrev block_number := protocol_quantity

/-- An execution block timestamp in seconds. -/
abbrev block_timestamp := protocol_quantity

/-- A quantity of blob gas. -/
abbrev blob_gas := protocol_quantity

/-- A number of blobs. -/
abbrev blob_count := protocol_quantity

/-- The denominator governing excess-blob-gas fee adjustment. -/
abbrev blob_fee_update_fraction := protocol_quantity

/-- An EIP-155 chain identifier. -/
abbrev chain_identifier := protocol_quantity

/-- A beacon-chain slot number. -/
abbrev slot_number := protocol_quantity

/-- An index in the withdrawal sequence. -/
abbrev withdrawal_index := protocol_quantity

/-- A consensus-layer validator index. -/
abbrev validator_index := protocol_quantity

/-- A withdrawal amount denominated in gwei. -/
abbrev withdrawal_amount := protocol_quantity

/-- The number of entries in a source-backed collection. -/
abbrev item_count := protocol_quantity

/-- An index into a source-backed collection. -/
abbrev item_index := protocol_quantity

/-- The nesting depth of an execution frame. -/
structure frame_depth where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace frame_depth
def Valid (x : frame_depth) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 1024
end frame_depth

/-- The immediate-byte width of a PUSH instruction. -/
structure push_width where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace push_width
def Valid (x : push_width) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 32
end push_width

/-- The number of words on an operand stack. -/
structure operand_stack_height where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace operand_stack_height
def Valid (x : operand_stack_height) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 1024
end operand_stack_height

/-- An index into the top sixteen operand-stack entries. -/
structure stack_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace stack_index
def Valid (x : stack_index) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 16
end stack_index

/-- A nonzero operand-stack position used by DUP and SWAP. -/
structure stack_operation_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace stack_operation_index
def Valid (x : stack_operation_index) : Prop :=
  1 ≤ x.value ∧ x.value ≤ 16
end stack_operation_index

/-- The number of indexed topics attached to one log. -/
structure log_topic_count where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace log_topic_count
def Valid (x : log_topic_count) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 4
end log_topic_count

/-- The parity bit used by transaction signatures. -/
structure y_parity where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace y_parity
def Valid (x : y_parity) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 1
end y_parity

/-- A depth in a 64-level binary Merkle tree. -/
structure merkle_depth where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace merkle_depth
def Valid (x : merkle_depth) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 64
end merkle_depth

/-- An EVM instruction byte. -/
structure opcode where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace opcode
def Valid (x : opcode) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 255
end opcode

/-- The one-based identifier of a precompiled contract. -/
structure precompile_id where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace precompile_id
def Valid (x : precompile_id) : Prop :=
  1 ≤ x.value ∧ x.value ≤ 256
end precompile_id

/-- The round count supplied to the BLAKE2 compression precompile. -/
structure blake2_rounds where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace blake2_rounds
def Valid (x : blake2_rounds) : Prop :=
  0 ≤ x.value ∧ x.value ≤ (2 ^ 32 - 1)
end blake2_rounds

/-- A fixed-point discount factor used by BLS precompile pricing. -/
structure bls_discount where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace bls_discount
def Valid (x : bls_discount) : Prop :=
  0 ≤ x.value ∧ x.value ≤ (2 ^ 16 - 1)
end bls_discount

/-- A bit position in the 2048-bit log bloom. -/
structure bloom_bit_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace bloom_bit_index
def Valid (x : bloom_bit_index) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 2047
end bloom_bit_index

/-- A 64-bit limb position in the log bloom. -/
structure bloom_limb_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace bloom_limb_index
def Valid (x : bloom_limb_index) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 31
end bloom_limb_index

/-- A bit position within a log-bloom limb. -/
structure bloom_limb_bit where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace bloom_limb_bit
def Valid (x : bloom_limb_bit) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 63
end bloom_limb_bit

/-- An index into the 256 most recent ancestor block hashes. -/
structure ancestor_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace ancestor_index
def Valid (x : ancestor_index) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 255
end ancestor_index

/-- A small positive divisor used for exact protocol arithmetic. -/
structure protocol_divisor where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace protocol_divisor
def Valid (x : protocol_divisor) : Prop :=
  1 ≤ x.value ∧ x.value ≤ 256
end protocol_divisor

/-- An absolute byte position in a named source region. -/
abbrev source_pointer := byte_quantity

/-- A byte-sequence length. -/
abbrev byte_length := byte_quantity

/-- An absolute byte position in the current EVM memory frame. -/
abbrev memory_pointer := byte_quantity

/-- A length or allocated size in EVM memory. -/
abbrev memory_length := byte_quantity

/-- An absolute byte position in the code arena. -/
abbrev code_pointer := byte_quantity

/-- A contract-code length. -/
abbrev code_length := byte_quantity

/-- A small positive divisor used by the gas schedule. -/
structure gas_divisor where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace gas_divisor
def Valid (x : gas_divisor) : Prop :=
  1 ≤ x.value ∧ x.value ≤ 2000
end gas_divisor

/-- The regions of the host interface a [EvmByteSlice][type-EvmByteSlice] may
reference. `StatelessInputSource` is the SSZ stateless-input envelope;
`EvmMemorySource` is the current frame's memory; `CodeSource` is stored
contract code; `LogDataSource` retains log payloads for receipts;
`OutputSource` is frame output frozen at halt; and `ScratchSource` is
the executor's scratch arena. -/
inductive ByteSource where | StatelessInputSource | EvmMemorySource | CodeSource | LogDataSource | OutputSource | ScratchSource
  deriving BEq, Inhabited, Repr
  open ByteSource

/-- An unmaterialized byte range: a region, an offset, and a length. A
slice denotes the byte sequence it references; reads resolve in O(1)
without copying. -/
structure EvmByteSlice where
  source : ByteSource
  off : byte_quantity
  len : byte_quantity
  deriving BEq, Inhabited, Repr

/-- A finite list of bytes paired with its construction-time length. -/
structure MaterializedBytes where
  data : (List byte)
  len : byte_length
  deriving BEq, Inhabited, Repr

/-- A byte-sequence segment: materialized bytes or a region-backed span.
A `list(Bytes)` denotes the concatenation of its segments.

!!! note "Implementation"
    A segment list crosses the host interface whole
    (`ffi/hash_glue.c`); no per-byte crossings occur. -/
inductive Bytes where
  | BytesList (_ : MaterializedBytes)
  | BytesSlice (_ : EvmByteSlice)
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

/-- A reference to an SSZ list: its byte span and element count. -/
structure SszListRef where
  bytes : EvmByteSlice
  count : item_count
  deriving BEq, Inhabited, Repr

/-- A sequential cursor over a variable-element SSZ list. -/
structure SszListCursor where
  items : SszListRef
  index : item_index
  current : source_pointer
  deriving BEq, Inhabited, Repr

/-- A 256-byte segment of a PUSH-aware `JUMPDEST` bitmap. -/
abbrev JumpdestChunk := (BitVec 256)

/-- A reference to a code's completed `JUMPDEST` analysis; zero denotes
the empty bitmap. -/
abbrev JumpdestRef := (BitVec 64)

/-- Executable code: its byte span and resolved `JUMPDEST` table. The
interpreter saves and restores both together; the code hash remains
the stable code-DB key. -/
structure Code where
  bytes : EvmByteSlice
  jumpdests : JumpdestRef
  deriving BEq, Inhabited, Repr

/-- A decoded RLP field: its source, list/string tag, and the offsets and
lengths of both the full encoding and its content. -/
structure RlpFieldRef where
  source : EvmByteSlice
  is_list : Bool
  full_off : source_pointer
  full_len : byte_length
  content_off : source_pointer
  content_len : byte_length
  deriving BEq, Inhabited, Repr

/-- A one-pass view over the immediate children of an RLP list. -/
structure RlpCursor where
  source : EvmByteSlice
  current : source_pointer
  stop : source_pointer
  valid : Bool
  deriving BEq, Inhabited, Repr

/-- A fork's blob parameters (EIP-4844/EIP-7691): target and maximum blob
counts per block, and the base-fee update fraction. -/
structure BlobSchedule where
  target : blob_count
  max : blob_count
  base_fee_update_fraction : blob_fee_update_fraction
  deriving BEq, Inhabited, Repr

/-- The decoded chain configuration for the executing payload. -/
structure ChainConfig where
  chain_id : chain_identifier
  fork_index : protocol_fork_index
  fork : Fork
  activation_active : Bool
  blob_schedule : (Option BlobSchedule)
  blob_schedule_shape_valid : Bool
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

/-- An opaque host-issued handle for a semantic transaction-state snapshot.
Its representation and rollback mechanism are deliberately unobservable. -/
inductive StateCheckpoint where
  | StateCheckpoint (_ : protocol_quantity)
  deriving Inhabited, BEq, Repr
  open StateCheckpoint

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
  count : blob_count
  deriving BEq, Inhabited, Repr

/-- A decoded transaction. Covers the EIP-2718 typed envelopes 0–4:
legacy, EIP-2930 (access list), EIP-1559 (fee market), EIP-4844
(blob), and EIP-7702 (set code); the type-specific fields are
validity-relevant per their EIP. -/
structure Transaction where
  tx_type : TxType
  sender : address
  nonce : word
  chain_id : chain_identifier
  gas_limit : gas
  is_create : Bool
  recipient : address
  value : word
  raw : EvmByteSlice
  input_src : EvmByteSlice
  access_list_addresses : (List address)
  access_list_address_count : item_count
  access_list_slots : (List StorageKey)
  access_list_slot_count : item_count
  max_fee : word
  max_blob_fee : word
  max_priority_fee : word
  authorizations : (List Authorization)
  authorization_count : item_count
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
create-tx code deposit and log inclusion). `valid` means the tx passed
transaction validity (nonce/balance/intrinsic/floor/fee/blob/EIP-7702
rules) and was applied to the block — a valid tx that reverts or runs
out of gas still has `valid = true`; it is a legitimate block member
(gas charged, nonce bumped). Block validity folds `valid`, not
`success`: only an invalid, inapplicable tx makes the containing block
invalid. `block_gas` is EIP-7778 block-gas accounting *without* the
EIP-3529 refund (`max(gas_used_before_refund, calldata_floor)`); it
differs from `gas_used` (the receipt's refunded gas) precisely by the
refund. -/
structure Receipt where
  tx_type : TxType
  success : Bool
  valid : Bool
  gas_used : gas
  block_gas : gas
  logs : (List LogEntry)
  deriving BEq, Inhabited, Repr

/-- The 2048-bit logs bloom filter (YP §4.4.1), as 256 bytes. -/
abbrev LogsBloom := (Vector limb 32)

/-- The execution-payload header fields the model reads and validates
(YP §4.4). `extra_data` stays a reference into the payload — it is
RLP-encoded whole for the header hash and never inspected. -/
structure BlockHeader where
  number : block_number
  timestamp : block_timestamp
  gas_limit : gas
  gas_used : gas
  prev_randao : word
  base_fee : word
  blob_gas_used : blob_gas
  excess_blob_gas : blob_gas
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
  transactions : SszListRef
  withdrawals : SszListRef
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
  deriving BEq, Inhabited, Repr

/-- The per-transaction environment: `ORIGIN`/`GASPRICE` (YP I_o, I_p)
plus the EIP-4844 blob versioned hashes the `BLOBHASH` opcode reads. -/
structure TxEnv where
  origin : address
  gas_price : word
  blob_hashes : BlobHashes
  deriving BEq, Inhabited, Repr

/-- The outcome of up-front transaction validation (YP §6.2): validity,
recovered sender, pre-state nonce, intrinsic gas, blob fee, and the
EIP-1559 effective gas and priority prices. -/
structure TxValidity where
  valid : Bool
  sender : address
  nonce_before : account_nonce
  intrinsic_gas : gas_cost
  calldata_floor : gas_cost
  blob_fee : word
  gas_price : word
  priority_fee : word
  deriving BEq, Inhabited, Repr

/-- The result of a transaction's top-level frame: success and remaining
gas. -/
structure TxFrameResult where
  success : Bool
  gas_remaining : gas
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
  is_static : Bool
  depth : frame_depth
  deriving BEq, Inhabited, Repr

/-- The suspended parent-frame state restored after nested execution. -/
structure FrameCheckpoint where
  state : StateCheckpoint
  pc : code_pointer
  gas_remaining : gas
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
  deriving BEq, Inhabited, Repr

/-- The suspended parent information needed after initcode returns. -/
structure CreateContinuation where
  checkpoint : FrameCheckpoint
  address : address
  deriving BEq, Inhabited, Repr

/-- The pending action performed when a child frame finishes. -/
inductive FrameContinuation where
  | ResumeCall (_ : CallContinuation)
  | ResumeCreate (_ : CreateContinuation)
  deriving Inhabited, BEq, Repr
  open FrameContinuation

/-- The protocol-bounded stack of suspended parent continuations. -/
abbrev FrameStack := (Vector FrameContinuation 1024)

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
  parent_blob_gas_used : blob_gas
  parent_excess_blob_gas : blob_gas
  deriving BEq, Inhabited, Repr

/-- The partial JUMPDEST bitmap and its location while code is scanned. -/
structure CodeAnalysis where
  chunk : JumpdestChunk
  chunk_index : code_pointer
  chunk_offset : Nat
  deriving BEq, Inhabited, Repr

/-- A decoded access list and the counts needed for intrinsic gas. -/
structure AccessListDecode where
  addresses : (List address)
  storage_slots : (List StorageKey)
  address_count : item_count
  slot_count : item_count
  deriving BEq, Inhabited, Repr

/-- A decoded EIP-7702 authorization sequence and its item count. -/
structure AuthorizationDecode where
  authorizations : (List Authorization)
  count : item_count
  deriving BEq, Inhabited, Repr

/-- The environment fields opcodes read through [k_env][]. -/
inductive EnvField where | F_Number | F_Timestamp | F_Coinbase | F_BaseFee | F_ChainId | F_GasLimit | F_PrevRandao | F_Origin | F_GasPrice | F_SlotNumber
  deriving BEq, Inhabited, Repr
  open EnvField

/-- The number of nibbles in a trie path. -/
structure trie_path_len where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace trie_path_len
def Valid (x : trie_path_len) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 64
end trie_path_len

/-- A trie path of at most 64 nibbles — secure state and storage keys are
32-byte hashes, and list tries use short RLP indices. `data` is
high-aligned; `len` preserves leading zeroes and prefixes. -/
structure TriePath where
  data : b256
  len : trie_path_len
  deriving BEq, Inhabited, Repr

/-- The sixteen child fields of a decoded branch node. -/
abbrev BranchChildren := (Vector RlpFieldRef 16)

/-- A four-bit path element (YP Appendix D). -/
abbrev nibble := (BitVec 4)

/-- A byte position in a 32-byte secure key. -/
structure b256_index where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace b256_index
def Valid (x : b256_index) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 31
end b256_index

/-- A cursor at or immediately after a position in a trie path. -/
structure trie_path_cursor where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace trie_path_cursor
def Valid (x : trie_path_cursor) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 64
end trie_path_cursor

/-- A decoded branch: sixteen child fields and the value field. -/
structure BranchNodeData where
  children : BranchChildren
  value : RlpFieldRef
  deriving BEq, Inhabited, Repr

/-- A decoded extension: its shared path and child field. -/
structure ExtensionNodeData where
  path : TriePath
  child : RlpFieldRef
  deriving BEq, Inhabited, Repr

/-- A decoded leaf: its remaining path and value field. -/
structure LeafNodeData where
  path : TriePath
  value : RlpFieldRef
  deriving BEq, Inhabited, Repr

/-- A decoded trie node, or `InvalidNode` when the bytes are not a
well-formed node. -/
inductive TrieNode where
  | LeafNode (_ : LeafNodeData)
  | ExtensionNode (_ : ExtensionNodeData)
  | BranchNode (_ : BranchNodeData)
  | InvalidNode (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieNode

/-- A self-contained trie-node encoding shorter than 32 bytes. -/
structure InlineNode where
  data : (BitVec 256)
  len : byte_length
  deriving BEq, Inhabited, Repr

/-- A reference to a trie node: empty, inline (encodings under 32 bytes),
or by KECCAK-256 hash (YP Appendix D, Eq. 207). -/
inductive NodeRef where
  | EmptyRef (_ : Unit)
  | InlineRef (_ : InlineNode)
  | HashRef (_ : hash)
  deriving Inhabited, BEq, Repr
  open NodeRef

/-- A non-negative blob-fee Taylor term scaled by a protocol denominator. -/
structure ScaledBlobValue where
  whole : word
  remainder : protocol_quantity
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

/-- The intrinsic, blob, and upfront costs established during validation. -/
structure TransactionCosts where
  intrinsic : gas_cost
  calldata_floor : gas_cost
  blob_gas : blob_gas
  blob_fee : word
  upfront : word
  deriving BEq, Inhabited, Repr

/-- The depth of a branch node in a fixed 64-nibble secure key. -/
structure trie_depth where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace trie_depth
def Valid (x : trie_depth) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 63
end trie_depth

/-- A cursor through the at-most-65 positions used by hex-evm_prefix decoding. -/
structure hex_prefix_cursor where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace hex_prefix_cursor
def Valid (x : hex_prefix_cursor) : Prop :=
  0 ≤ x.value ∧ x.value ≤ 65
end hex_prefix_cursor

/-- The sixteen child references of a branch, indexed by nibble. -/
abbrev BranchRefs := (Vector NodeRef 16)

/-- A pending change at a trie key: a put of new leaf bytes, or a
delete. -/
inductive TrieChange where
  | TriePut (_ : EvmByteSlice)
  | TrieDelete (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieChange

/-- An update: a full-path key and its change. Update lists are kept
sorted by key. -/
structure TrieUpdate where
  key : TriePath
  change : TrieChange
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

/-- The item count and next canonical-key position of an indexed trie. -/
structure RlpIndexCursor where
  count : item_count
  position : item_index
  deriving BEq, Inhabited, Repr

/-- One numeric index, its trie key, and the following key when present. -/
structure RlpIndexItem where
  index : item_index
  key : TriePath
  next_key : (Option TriePath)
  deriving BEq, Inhabited, Repr

/-- The minimal nonzero byte width of a supported RLP list index. -/
structure rlp_index_byte_width where
  value : Nat
  deriving Inhabited, BEq, Repr

namespace rlp_index_byte_width
def Valid (x : rlp_index_byte_width) : Prop :=
  1 ≤ x.value ∧ x.value ≤ 8
end rlp_index_byte_width

/-- Every variable region of the input, resolved once before decoding.
Consumers receive explicit source spans instead of re-reading nested
SSZ offset tables. -/
structure StatelessInputRef where
  new_payload_request : EvmByteSlice
  execution_payload : EvmByteSlice
  versioned_hashes : EvmByteSlice
  deposits : EvmByteSlice
  withdrawal_requests : EvmByteSlice
  consolidation_requests : EvmByteSlice
  extra_data : EvmByteSlice
  transactions : SszListRef
  withdrawals : SszListRef
  block_access_list : EvmByteSlice
  witness_state : SszListRef
  witness_codes : SszListRef
  witness_headers : SszListRef
  chain_config : EvmByteSlice
  public_keys : EvmByteSlice
  deriving BEq, Inhabited, Repr

/-- The parent-header fields required to initialize execution context. -/
structure ParentHeaderFields where
  parent_hash : hash
  state_root : hash
  base_fee : word
  blob_gas_used : blob_gas
  excess_blob_gas : blob_gas
  have_parent : Bool
  have_state : Bool
  have_base_fee : Bool
  have_blob_gas : Bool
  have_excess_blob_gas : Bool
  deriving BEq, Inhabited, Repr

/-- The running authentication and execution context of the witness header
chain. -/
structure WitnessHeaderIndex where
  cursor : SszListCursor
  previous_hash : hash
  valid : Bool
  parent_state_root : hash
  parent_base_fee_per_gas : word
  parent_blob_gas_used : blob_gas
  parent_excess_blob_gas : blob_gas
  parent_fields_valid : Bool
  deriving BEq, Inhabited, Repr

/-- A receipt retained until its lexicographic trie successor is known. -/
structure PendingReceipt where
  index : item_index
  cumulative_gas_used : gas
  receipt : Receipt
  deriving BEq, Inhabited, Repr

/-- The streaming receipts-trie builder and block-wide receipt aggregates. -/
structure ReceiptAccumulator where
  builder : TrieBuilder
  first : (Option PendingReceipt)
  pending : (Option PendingReceipt)
  count : item_count
  cumulative_gas_used : gas
  bloom : LogsBloom
  deriving BEq, Inhabited, Repr

/-- The reconstructed BAL: its RLP and its item count (bounded by
`gas_limit / 2000`). -/
structure EncodedBlockAccessList where
  bytes : EvmByteSlice
  item_count : item_count
  deriving BEq, Inhabited, Repr

/-- An encoded-content length paired with the next table cursor. -/
structure BalContentCursor where
  content_len : byte_length
  cursor : item_index
  deriving BEq, Inhabited, Repr

/-- An encoded-content length paired with its logical BAL item count. -/
structure BalContentCount where
  content_len : byte_length
  count : item_count
  deriving BEq, Inhabited, Repr

/-- The encoded length and logical item count contributed by one account. -/
structure BalAccountSize where
  encoded_len : byte_length
  item_count : item_count
  deriving BEq, Inhabited, Repr

/-- The end and maximum value of one equal-index nonce run. -/
structure BalNonceRun where
  cursor : item_index
  maximum : account_nonce
  deriving BEq, Inhabited, Repr

/-- Everything block validation needs from an executed body: overall
success, gas and blob-gas totals, receipts, and the collected
EIP-7685 requests. -/
structure BlockExecutionResult where
  all_ok : Bool
  gas_acc : gas
  blob_gas_acc : blob_gas
  first_tx_recipient : address
  block_gas_overflow : Bool
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

/-- A streaming Merkle frontier together with the number of leaves already
incorporated. -/
structure MerkleAccumulator where
  frontier : (List MerkleSlot)
  count : item_count
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
  | frame_stack_top
  | frame_stack
  | call_depth
  | message
  | frame_status
  | frame_refund
  | gas_remaining
  | pc
  | k_tx
  | k_header
  | k_blob_schedule
  | k_fork
  | k_chain_id
  | k_n_headers
  | k_parent_state_root
  | scratch_cursor
  deriving DecidableEq, Hashable, Repr
open Register

abbrev RegisterType : Register → Type
  | .evm_memory => EvmByteSlice
  | .returndata => EvmByteSlice
  | .calldata => EvmByteSlice
  | .frame_code => Code
  | .frame_stack_top => Nat
  | .frame_stack => (Vector FrameContinuation 1024)
  | .call_depth => Nat
  | .message => Message
  | .frame_status => FrameStatus
  | .frame_refund => gas_refund
  | .gas_remaining => gas
  | .pc => byte_quantity
  | .k_tx => TxEnv
  | .k_header => BlockHeader
  | .k_blob_schedule => BlobSchedule
  | .k_fork => Fork
  | .k_chain_id => Nat
  | .k_n_headers => Nat
  | .k_parent_state_root => b256
  | .scratch_cursor => byte_quantity

instance : Inhabited (RegisterRef RegisterType BlobSchedule) where
  default := .Reg k_blob_schedule
instance : Inhabited (RegisterRef RegisterType BlockHeader) where
  default := .Reg k_header
instance : Inhabited (RegisterRef RegisterType EvmByteSlice) where
  default := .Reg calldata
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
instance : Inhabited (RegisterRef RegisterType b256) where
  default := .Reg k_parent_state_root
instance : Inhabited (RegisterRef RegisterType byte_quantity) where
  default := .Reg scratch_cursor
instance : Inhabited (RegisterRef RegisterType gas) where
  default := .Reg gas_remaining
instance : Inhabited (RegisterRef RegisterType gas_refund) where
  default := .Reg frame_refund
instance : Inhabited (RegisterRef RegisterType Nat) where
  default := .Reg call_depth
instance : Inhabited (RegisterRef RegisterType Nat) where
  default := .Reg k_n_headers
instance : Inhabited (RegisterRef RegisterType (Vector FrameContinuation 1024)) where
  default := .Reg frame_stack
abbrev SailM := PreSailM RegisterType trivialChoiceSource exception
abbrev SailME := PreSailME RegisterType trivialChoiceSource exception

