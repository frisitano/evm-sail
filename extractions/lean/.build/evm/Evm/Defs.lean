import Evm.Sail.Sail
import Evm.Sail.BitVec

open PreSail

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

abbrev bit := (BitVec 1)

inductive byte_quantity where
  | ByteQuantity (_ : Nat)
  deriving Inhabited, BEq, Repr
  open byte_quantity

inductive gas_constant where
  | GasConstant (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas_constant

inductive gas_cost where
  | GasCost (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas_cost

inductive gas where
  | Gas (_ : Nat)
  deriving Inhabited, BEq, Repr
  open gas

inductive gas_refund where
  | GasRefund (_ : Int)
  deriving Inhabited, BEq, Repr
  open gas_refund

inductive Fork where | Frontier | Homestead | Byzantium | Constantinople | Istanbul | Berlin | London | Paris | Shanghai | Cancun | Prague | Osaka | Amsterdam
  deriving BEq, Inhabited, Repr
  open Fork

abbrev bits k_n := (BitVec k_n)

/-- Type quantifiers: k_a : Type -/
inductive option (k_a : Type) where
  | Some (_ : k_a)
  | None (_ : Unit)
  deriving Inhabited, BEq, Repr
  open option

abbrev word := (BitVec 256)

abbrev address := (BitVec 160)

abbrev hash := (BitVec 256)

abbrev byte := (BitVec 8)

abbrev limb := (BitVec 64)

structure LimbDivMod where
  quotient : limb
  remainder : limb
  deriving BEq, Inhabited, Repr

structure WordDivMod where
  quotient : word
  remainder : word
  deriving BEq, Inhabited, Repr

abbrev limb_bit_count := Nat

abbrev word_bit_count := Nat

abbrev protocol_quantity := Nat

abbrev account_nonce := protocol_quantity

abbrev block_number := protocol_quantity

abbrev block_timestamp := protocol_quantity

abbrev blob_gas := protocol_quantity

abbrev blob_count := protocol_quantity

abbrev blob_fee_update_fraction := protocol_quantity

abbrev chain_identifier := protocol_quantity

abbrev protocol_fork_index := protocol_quantity

abbrev slot_number := protocol_quantity

abbrev withdrawal_index := protocol_quantity

abbrev validator_index := protocol_quantity

abbrev withdrawal_amount := protocol_quantity

abbrev item_count := protocol_quantity

abbrev item_index := protocol_quantity

abbrev frame_depth := Nat

abbrev push_width := Nat

abbrev operand_stack_height := Nat

abbrev stack_index := Nat

abbrev stack_operation_index := Nat

abbrev log_topic_count := Nat

abbrev y_parity := Nat

abbrev merkle_depth := Nat

abbrev opcode := Nat

abbrev precompile_id := Nat

abbrev blake2_rounds := Nat

abbrev bls_discount := Nat

abbrev bloom_bit_index := Nat

abbrev bloom_limb_index := Nat

abbrev bloom_limb_bit := Nat

abbrev ancestor_index := Nat

abbrev protocol_divisor := Nat

abbrev source_pointer := byte_quantity

abbrev byte_length := byte_quantity

abbrev memory_pointer := byte_quantity

abbrev memory_length := byte_quantity

abbrev code_pointer := byte_quantity

abbrev code_length := byte_quantity

abbrev gas_divisor := Nat

inductive ByteSource where | StatelessInputSource | EvmMemorySource | CodeSource | LogDataSource | OutputSource | ScratchSource
  deriving BEq, Inhabited, Repr
  open ByteSource

structure ByteSlice where
  source : ByteSource
  off : byte_quantity
  len : byte_quantity
  deriving BEq, Inhabited, Repr

structure MaterializedBytes where
  data : (List byte)
  len : byte_length
  deriving BEq, Inhabited, Repr

inductive Bytes where
  | BytesList (_ : MaterializedBytes)
  | BytesSlice (_ : ByteSlice)
  deriving Inhabited, BEq, Repr
  open Bytes

inductive HaltKind where
  | HaltStop (_ : Unit)
  | HaltReturn (_ : ByteSlice)
  | HaltRevert (_ : ByteSlice)
  | HaltSelfDestruct (_ : Unit)
  deriving Inhabited, BEq, Repr
  open HaltKind

inductive ExceptionKind where | StackUnderflow | StackOverflow | OutOfGas | InvalidOpcode | InvalidJump | StaticViolation | CallDepthExceeded | InsufficientBalance | WriteProtection | InitCodeTooLarge | NonceOverflow | AddressCollision
  deriving BEq, Inhabited, Repr
  open ExceptionKind

inductive FrameStatus where
  | Running (_ : Unit)
  | Halted (_ : HaltKind)
  | Exceptional (_ : ExceptionKind)
  deriving Inhabited, BEq, Repr
  open FrameStatus

inductive BlockError where | InvalidConfig | HeaderChainBroken | RlpDecode | InvalidSignature | InvalidGasLimit | GasUsedExceedsLimit | BlobGasLimitExceeded | ExecutionInvalid | InvalidGasUsed | InvalidBlobGasUsed | InvalidExcessBlobGas | InvalidStateRoot | InvalidReceiptsRoot | InvalidLogsBloom | InvalidBlockHash | InvalidParentHash | BlockAccessListTooLarge | InvalidBlockAccessList | InvalidExecutionRequests | WitnessDeficient
  deriving BEq, Inhabited, Repr
  open BlockError

inductive exception where
  | InvalidBlock (_ : BlockError)
  deriving Inhabited, BEq, Repr
  open exception

structure SszListRef where
  bytes : ByteSlice
  count : item_count
  deriving BEq, Inhabited, Repr

structure SszListCursor where
  items : SszListRef
  index : item_index
  current : source_pointer
  deriving BEq, Inhabited, Repr

abbrev JumpdestChunk := (BitVec 256)

abbrev JumpdestRef := (BitVec 64)

structure Code where
  bytes : ByteSlice
  jumpdests : JumpdestRef
  deriving BEq, Inhabited, Repr

structure RlpFieldRef where
  source : ByteSlice
  is_list : Bool
  full_off : source_pointer
  full_len : byte_length
  content_off : source_pointer
  content_len : byte_length
  deriving BEq, Inhabited, Repr

structure RlpCursor where
  source : ByteSlice
  current : source_pointer
  stop : source_pointer
  valid : Bool
  deriving BEq, Inhabited, Repr

structure BlobSchedule where
  target : blob_count
  max : blob_count
  base_fee_update_fraction : blob_fee_update_fraction
  deriving BEq, Inhabited, Repr

structure ChainConfig where
  chain_id : chain_identifier
  fork_index : protocol_fork_index
  fork : Fork
  activation_active : Bool
  blob_schedule : (Option BlobSchedule)
  blob_schedule_shape_valid : Bool
  deriving BEq, Inhabited, Repr

structure AccountInfo where
  nonce : account_nonce
  balance : word
  code_hash : hash
  storage_root : hash
  deriving BEq, Inhabited, Repr

structure Account where
  info : AccountInfo
  exists : Bool
  storage_cleared : Bool
  created : Bool
  selfdestructed : Bool
  deriving BEq, Inhabited, Repr

structure StorageValue where
  curr : word
  orig : word
  deriving BEq, Inhabited, Repr

structure StorageKey where
  addr : address
  slot : word
  deriving BEq, Inhabited, Repr

structure StorageEntry where
  key : StorageKey
  value : StorageValue
  deriving BEq, Inhabited, Repr

structure AcctValue where
  curr : Account
  orig : Account
  deriving BEq, Inhabited, Repr

structure AcctEntry where
  addr : address
  value : AcctValue
  deriving BEq, Inhabited, Repr

abbrev JournalCheckpoint := protocol_quantity

abbrev AccountCheckpoint := protocol_quantity

abbrev StorageCheckpoint := protocol_quantity

abbrev LogCheckpoint := protocol_quantity

structure StateCheckpoint where
  journal : JournalCheckpoint
  accounts : AccountCheckpoint
  storage : StorageCheckpoint
  logs : LogCheckpoint
  deriving BEq, Inhabited, Repr

inductive JEntry where
  | JTran (_ : (address × word × word))
  | JWarmA (_ : address)
  | JWarmS (_ : (address × word))
  deriving Inhabited, BEq, Repr
  open JEntry

inductive TxType where | LegacyTx | AccessListTx | FeeMarketTx | BlobTx | SetCodeTx
  deriving BEq, Inhabited, Repr
  open TxType

structure Authorization where
  valid_sig : Bool
  authority : address
  address : address
  nonce : account_nonce
  chain_id : word
  deriving BEq, Inhabited, Repr

structure BlobHashes where
  bytes : ByteSlice
  count : blob_count
  deriving BEq, Inhabited, Repr

structure Transaction where
  tx_type : TxType
  sender : address
  nonce : word
  chain_id : chain_identifier
  gas_limit : gas
  is_create : Bool
  recipient : address
  value : word
  raw : ByteSlice
  input_src : ByteSlice
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
  pubkey : ByteSlice
  signing_hash : hash
  sig_v : word
  sig_r : word
  sig_s : word
  deriving BEq, Inhabited, Repr

structure LogEntry where
  address : address
  topics : (List word)
  data : ByteSlice
  deriving BEq, Inhabited, Repr

structure Receipt where
  tx_type : TxType
  success : Bool
  valid : Bool
  gas_used : gas
  block_gas : gas
  logs : (List LogEntry)
  deriving BEq, Inhabited, Repr

abbrev LogsBloom := (Vector limb 32)

structure BlockHeader where
  number : block_number
  timestamp : block_timestamp
  gas_limit : gas
  gas_used : gas
  prev_randao : word
  base_fee : word
  blob_gas_used : blob_gas
  excess_blob_gas : blob_gas
  state_root : (BitVec 256)
  receipts_root : (BitVec 256)
  logs_bloom : LogsBloom
  fee_recipient : address
  parent_hash : (BitVec 256)
  parent_beacon_block_root : (BitVec 256)
  slot_number : slot_number
  extra_data : ByteSlice
  deriving BEq, Inhabited, Repr

structure Withdrawal where
  index : withdrawal_index
  validator_index : validator_index
  address : address
  amount : withdrawal_amount
  deriving BEq, Inhabited, Repr

structure BlockBody where
  transactions : SszListRef
  withdrawals : SszListRef
  block_access_list : ByteSlice
  deriving BEq, Inhabited, Repr

structure Block where
  header : BlockHeader
  body : BlockBody
  deriving BEq, Inhabited, Repr

structure ExecutionPayload where
  expected_block_hash : hash
  block : Block
  deriving BEq, Inhabited, Repr

structure ExecutionRequests where
  deposits : ByteSlice
  withdrawals : ByteSlice
  consolidations : ByteSlice
  deriving BEq, Inhabited, Repr

structure TxEnv where
  origin : address
  gas_price : word
  blob_hashes : BlobHashes
  deriving BEq, Inhabited, Repr

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

structure TxFrameResult where
  success : Bool
  gas_remaining : gas
  refund : gas_refund
  deriving BEq, Inhabited, Repr

inductive CallKind where | Call | CallCode | DelegateCall | StaticCall
  deriving BEq, Inhabited, Repr
  open CallKind

structure Message where
  caller : address
  address : address
  code_address : address
  value : word
  is_static : Bool
  depth : frame_depth
  deriving BEq, Inhabited, Repr

structure FrameCheckpoint where
  state : StateCheckpoint
  pc : code_pointer
  gas_remaining : gas
  refund : gas_refund
  status : FrameStatus
  message : Message
  call_depth : frame_depth
  code : Code
  calldata : ByteSlice
  memory : ByteSlice
  deriving BEq, Inhabited, Repr

structure StatelessInput where
  payload : ExecutionPayload
  chain_config : ChainConfig
  deriving BEq, Inhabited, Repr

structure WitnessContext where
  parent_hash : hash
  parent_state_root : hash
  parent_base_fee_per_gas : word
  parent_blob_gas_used : blob_gas
  parent_excess_blob_gas : blob_gas
  deriving BEq, Inhabited, Repr

structure CodeAnalysis where
  chunk : JumpdestChunk
  chunk_index : code_pointer
  chunk_offset : Nat
  deriving BEq, Inhabited, Repr

structure AccessListDecode where
  addresses : (List address)
  storage_slots : (List StorageKey)
  address_count : item_count
  slot_count : item_count
  deriving BEq, Inhabited, Repr

structure AuthorizationDecode where
  authorizations : (List Authorization)
  count : item_count
  deriving BEq, Inhabited, Repr

inductive EnvField where | F_Number | F_Timestamp | F_Coinbase | F_BaseFee | F_ChainId | F_GasLimit | F_PrevRandao | F_Origin | F_GasPrice | F_SlotNumber
  deriving BEq, Inhabited, Repr
  open EnvField

abbrev trie_path_len := Nat

structure TriePath where
  data : (BitVec 256)
  len : trie_path_len
  deriving BEq, Inhabited, Repr

abbrev BranchChildren := (Vector RlpFieldRef 16)

abbrev nibble := (BitVec 4)

structure BranchNodeData where
  children : BranchChildren
  value : RlpFieldRef
  deriving BEq, Inhabited, Repr

structure ExtensionNodeData where
  path : TriePath
  child : RlpFieldRef
  deriving BEq, Inhabited, Repr

structure LeafNodeData where
  path : TriePath
  value : RlpFieldRef
  deriving BEq, Inhabited, Repr

inductive TrieNode where
  | LeafNode (_ : LeafNodeData)
  | ExtensionNode (_ : ExtensionNodeData)
  | BranchNode (_ : BranchNodeData)
  | InvalidNode (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieNode

structure InlineNode where
  data : (BitVec 256)
  len : byte_length
  deriving BEq, Inhabited, Repr

inductive NodeRef where
  | EmptyRef (_ : Unit)
  | InlineRef (_ : InlineNode)
  | HashRef (_ : hash)
  deriving Inhabited, BEq, Repr
  open NodeRef

abbrev trie_path_cursor := Nat

abbrev trie_fuel := Nat

structure ScaledBlobValue where
  whole : word
  remainder : protocol_quantity
  deriving BEq, Inhabited, Repr

structure PrecompileResult where
  success : Bool
  output : ByteSlice
  deriving BEq, Inhabited, Repr

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

structure TransactionCosts where
  intrinsic : gas_cost
  calldata_floor : gas_cost
  blob_gas : blob_gas
  blob_fee : word
  upfront : word
  deriving BEq, Inhabited, Repr

abbrev trie_depth := Nat

abbrev BranchRefs := (Vector NodeRef 16)

inductive TrieChange where
  | TriePut (_ : ByteSlice)
  | TrieDelete (_ : Unit)
  deriving Inhabited, BEq, Repr
  open TrieChange

structure TrieUpdate where
  key : TriePath
  change : TrieChange
  deriving BEq, Inhabited, Repr

inductive TrieItemValue where
  | LeafItem (_ : ByteSlice)
  | BranchItem (_ : NodeRef)
  | SubtreeItem (_ : NodeRef)
  deriving Inhabited, BEq, Repr
  open TrieItemValue

structure TrieItem where
  path : TriePath
  value : TrieItemValue
  deriving BEq, Inhabited, Repr

structure TrieBranchFrame where
  depth : trie_depth
  mask : (BitVec 16)
  children : BranchRefs
  deriving BEq, Inhabited, Repr

structure TrieBuilder where
  frames : (List TrieBranchFrame)
  root : NodeRef
  complete : Bool
  deriving BEq, Inhabited, Repr

structure TrieItemSink where
  builder : TrieBuilder
  pending : (Option TrieItem)
  deriving BEq, Inhabited, Repr

structure RlpIndexCursor where
  count : item_count
  position : item_index
  deriving BEq, Inhabited, Repr

structure RlpIndexItem where
  index : item_index
  key : TriePath
  next_key : (Option TriePath)
  deriving BEq, Inhabited, Repr

abbrev rlp_index_byte_width := Nat

structure StatelessInputRef where
  new_payload_request : ByteSlice
  execution_payload : ByteSlice
  versioned_hashes : ByteSlice
  deposits : ByteSlice
  withdrawal_requests : ByteSlice
  consolidation_requests : ByteSlice
  extra_data : ByteSlice
  transactions : SszListRef
  withdrawals : SszListRef
  block_access_list : ByteSlice
  witness_state : SszListRef
  witness_codes : SszListRef
  witness_headers : SszListRef
  chain_config : ByteSlice
  public_keys : ByteSlice
  deriving BEq, Inhabited, Repr

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

structure PendingReceipt where
  index : item_index
  cumulative_gas_used : gas
  receipt : Receipt
  deriving BEq, Inhabited, Repr

structure ReceiptAccumulator where
  builder : TrieBuilder
  first : (Option PendingReceipt)
  pending : (Option PendingReceipt)
  count : item_count
  cumulative_gas_used : gas
  bloom : LogsBloom
  deriving BEq, Inhabited, Repr

structure EncodedBlockAccessList where
  bytes : ByteSlice
  item_count : item_count
  deriving BEq, Inhabited, Repr

structure BalContentCursor where
  content_len : byte_length
  cursor : item_index
  deriving BEq, Inhabited, Repr

structure BalContentCount where
  content_len : byte_length
  count : item_count
  deriving BEq, Inhabited, Repr

structure BalAccountSize where
  encoded_len : byte_length
  item_count : item_count
  deriving BEq, Inhabited, Repr

structure BalNonceRun where
  cursor : item_index
  maximum : account_nonce
  deriving BEq, Inhabited, Repr

structure BlockExecutionResult where
  all_ok : Bool
  gas_acc : gas
  blob_gas_acc : blob_gas
  first_tx_recipient : address
  block_gas_overflow : Bool
  blob_gas_overflow : Bool
  receipts_root : hash
  logs_bloom : LogsBloom
  deposits : ByteSlice
  requests : ExecutionRequests
  deriving BEq, Inhabited, Repr

structure StatelessValidationFailure where
  scope : (BitVec 8)
  reason : BlockError
  deriving BEq, Inhabited, Repr

inductive StatelessValidationResult where
  | StatelessPayloadValid (_ : Unit)
  | StatelessPayloadInvalid (_ : StatelessValidationFailure)
  deriving Inhabited, BEq, Repr
  open StatelessValidationResult

inductive MerkleSlot where
  | EmptyMerkleSlot (_ : Unit)
  | OccupiedMerkleSlot (_ : hash)
  deriving Inhabited, BEq, Repr
  open MerkleSlot

structure MerkleAccumulator where
  frontier : (List MerkleSlot)
  count : item_count
  deriving BEq, Inhabited, Repr

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
  | .evm_memory => ByteSlice
  | .returndata => ByteSlice
  | .calldata => ByteSlice
  | .frame_code => Code
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
  | .k_parent_state_root => (BitVec 256)
  | .scratch_cursor => byte_quantity

instance : Inhabited (RegisterRef RegisterType BlobSchedule) where
  default := .Reg k_blob_schedule
instance : Inhabited (RegisterRef RegisterType BlockHeader) where
  default := .Reg k_header
instance : Inhabited (RegisterRef RegisterType ByteSlice) where
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
instance : Inhabited (RegisterRef RegisterType (BitVec 256)) where
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
abbrev SailM := PreSailM RegisterType trivialChoiceSource exception
abbrev SailME := PreSailME RegisterType trivialChoiceSource exception

