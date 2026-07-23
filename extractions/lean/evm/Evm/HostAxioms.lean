import Evm.Defs

open Sail
open ConcurrencyInterfaceV1
open Evm.Defs

/-!
## Extensional world-state contract

The declarations in `Evm.Functions` below are the raw symbols imported by the
generated Sail model.  This namespace gives those account, persistent-storage,
and transient-storage symbols an implementation-independent meaning.

Nothing here describes the layout of `ffi/state_db.c` or `ffi/kernel_state.c`:
there are no table rows, generations, undo cursors, physical deletions, or
first-write/copy-on-write rules.  Such devices are permitted only through a
refinement from a backend state to the total maps and logical snapshots below.
-/
namespace Evm.Contracts

deriving instance DecidableEq for word
deriving instance DecidableEq for address

def zeroWord : word := ⟨0⟩

structure PersistentWorld where
  accountAt : address → Account
  storageAt : address → word → word

def accountWithClearedStorage (account : Account) : Account :=
  { account with storage_cleared := true }

def accountWithoutTransactionFlags (account : Account) : Account :=
  { account with created := false, selfdestructed := false }

def zeroAccountNonce : account_nonce :=
  ⟨0⟩

def deletedAccount (emptyCodeHash : hash) (account : Account) : Account :=
  { account with
    info :=
      { account.info with
        nonce := zeroAccountNonce
        balance := zeroWord
        code_hash := emptyCodeHash }
    present := false
    storage_cleared := true }

def worldWriteAccount
    (world : PersistentWorld) (target : address) (value : Account) :
    PersistentWorld :=
  { accountAt := fun candidate =>
      if candidate = target then value else world.accountAt candidate
    storageAt := world.storageAt }

def worldWriteStorage
    (world : PersistentWorld) (target : address) (targetSlot value : word) :
    PersistentWorld :=
  { accountAt := world.accountAt
    storageAt := fun candidate candidateSlot =>
      if candidate = target ∧ candidateSlot = targetSlot then
        value
      else
        world.storageAt candidate candidateSlot }

def worldClearStorage
    (world : PersistentWorld) (target : address) : PersistentWorld :=
  { accountAt := fun candidate =>
      if candidate = target then
        accountWithClearedStorage (world.accountAt candidate)
      else
        world.accountAt candidate
    storageAt := fun candidate candidateSlot =>
      if candidate = target then zeroWord
      else world.storageAt candidate candidateSlot }

def worldDeleteAccount
    (emptyCodeHash : hash) (world : PersistentWorld) (target : address) :
    PersistentWorld :=
  { accountAt := fun candidate =>
      if candidate = target then
        deletedAccount emptyCodeHash (world.accountAt candidate)
      else
        world.accountAt candidate
    storageAt := fun candidate candidateSlot =>
      if candidate = target then zeroWord
      else world.storageAt candidate candidateSlot }

abbrev TransientState := address → word → word
abbrev WarmAddressState := address → Bool
abbrev WarmSlotState := address → word → Bool

def emptyTransientState : TransientState :=
  fun _ _ => zeroWord

def emptyWarmAddressState : WarmAddressState := fun _ => false
def emptyWarmSlotState : WarmSlotState := fun _ _ => false

def transientWrite
    (transient : TransientState) (target : address) (targetSlot value : word) :
    TransientState :=
  fun candidate candidateSlot =>
    if candidate = target ∧ candidateSlot = targetSlot then
      value
    else
      transient candidate candidateSlot

def warmAddressWrite
    (warm : WarmAddressState) (target : address) : WarmAddressState :=
  fun candidate => if candidate = target then true else warm candidate

def warmSlotWrite
    (warm : WarmSlotState) (target : address) (targetSlot : word) :
    WarmSlotState :=
  fun candidate candidateSlot =>
    if candidate = target ∧ candidateSlot = targetSlot then true
    else warm candidate candidateSlot

/- `blockStart` is the authenticated parent state and `blockCurrent`
accumulates successful transactions.  `txStart` is the EIP-2200 original-value
snapshot; `txCurrent` is the live state of the executing transaction. -/
structure ReferenceWorldState where
  blockStart : PersistentWorld
  blockCurrent : PersistentWorld
  txStart : PersistentWorld
  txCurrent : PersistentWorld
  transient : TransientState
  warmAddresses : WarmAddressState
  warmSlots : WarmSlotState
  logs : List LogEntry

structure TransactionSnapshot where
  txCurrent : PersistentWorld
  transient : TransientState
  warmAddresses : WarmAddressState
  warmSlots : WarmSlotState
  logs : List LogEntry

def persistentWorldEquivalent (left right : PersistentWorld) : Prop :=
  (∀ account, left.accountAt account = right.accountAt account) ∧
  (∀ account slot, left.storageAt account slot = right.storageAt account slot)

def transientStateEquivalent (left right : TransientState) : Prop :=
  ∀ account slot, left account slot = right account slot

def warmAddressStateEquivalent (left right : WarmAddressState) : Prop :=
  ∀ account, left account = right account

def warmSlotStateEquivalent (left right : WarmSlotState) : Prop :=
  ∀ account slot, left account slot = right account slot

def referenceWorldStateEquivalent
    (left right : ReferenceWorldState) : Prop :=
  persistentWorldEquivalent left.blockStart right.blockStart ∧
  persistentWorldEquivalent left.blockCurrent right.blockCurrent ∧
  persistentWorldEquivalent left.txStart right.txStart ∧
  persistentWorldEquivalent left.txCurrent right.txCurrent ∧
  transientStateEquivalent left.transient right.transient ∧
  warmAddressStateEquivalent left.warmAddresses right.warmAddresses ∧
  warmSlotStateEquivalent left.warmSlots right.warmSlots ∧
  left.logs = right.logs

def transactionSnapshotEquivalent
    (left right : TransactionSnapshot) : Prop :=
  persistentWorldEquivalent left.txCurrent right.txCurrent ∧
  transientStateEquivalent left.transient right.transient ∧
  warmAddressStateEquivalent left.warmAddresses right.warmAddresses ∧
  warmSlotStateEquivalent left.warmSlots right.warmSlots ∧
  left.logs = right.logs

def referenceStartBlock (initial : PersistentWorld) : ReferenceWorldState :=
  { blockStart := initial
    blockCurrent := initial
    txStart := initial
    txCurrent := initial
    transient := emptyTransientState
    warmAddresses := emptyWarmAddressState
    warmSlots := emptyWarmSlotState
    logs := [] }

def referenceBeginTransaction
    (state : ReferenceWorldState) : ReferenceWorldState :=
  { blockStart := state.blockStart
    blockCurrent := state.blockCurrent
    txStart := state.blockCurrent
    txCurrent := state.blockCurrent
    transient := emptyTransientState
    warmAddresses := emptyWarmAddressState
    warmSlots := emptyWarmSlotState
    logs := [] }

def referenceReadAccount
    (state : ReferenceWorldState) (account : address) : Account :=
  state.txCurrent.accountAt account

def referenceReadStorageCurrent
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  state.txCurrent.storageAt account slot

/- SSTORE's original value is fixed by transaction entry, not by the first
insertion into an implementation table. -/
def referenceReadStorageOriginal
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  state.txStart.storageAt account slot

def referenceWriteAccount
    (state : ReferenceWorldState) (account : address) (value : Account) :
    ReferenceWorldState :=
  { state with txCurrent := worldWriteAccount state.txCurrent account value }

def referenceWriteStorage
    (state : ReferenceWorldState) (account : address) (slot value : word) :
    ReferenceWorldState :=
  { state with txCurrent := worldWriteStorage state.txCurrent account slot value }

def referenceClearStorage
    (state : ReferenceWorldState) (account : address) : ReferenceWorldState :=
  { state with txCurrent := worldClearStorage state.txCurrent account }

def referenceDeleteAccount
    (emptyCodeHash : hash) (state : ReferenceWorldState) (account : address) :
    ReferenceWorldState :=
  { state with
    txCurrent := worldDeleteAccount emptyCodeHash state.txCurrent account }

def referenceReadTransient
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  state.transient account slot

def referenceWriteTransient
    (state : ReferenceWorldState) (account : address) (slot value : word) :
    ReferenceWorldState :=
  { state with transient := transientWrite state.transient account slot value }

def referenceTouchWarmAddress
    (state : ReferenceWorldState) (account : address) :
    Bool × ReferenceWorldState :=
  (state.warmAddresses account,
   { state with warmAddresses := warmAddressWrite state.warmAddresses account })

def referenceTouchWarmSlot
    (state : ReferenceWorldState) (account : address) (slot : word) :
    Bool × ReferenceWorldState :=
  (state.warmSlots account slot,
   { state with warmSlots := warmSlotWrite state.warmSlots account slot })

def referenceAppendLog
    (state : ReferenceWorldState) (entry : LogEntry) : ReferenceWorldState :=
  { state with logs := state.logs ++ [entry] }

def referenceCheckpoint (state : ReferenceWorldState) : TransactionSnapshot :=
  { txCurrent := state.txCurrent
    transient := state.transient
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    logs := state.logs }

def referenceRevert
    (state : ReferenceWorldState) (snapshot : TransactionSnapshot) :
    ReferenceWorldState :=
  { state with
    txCurrent := snapshot.txCurrent
    transient := snapshot.transient
    warmAddresses := snapshot.warmAddresses
    warmSlots := snapshot.warmSlots
    logs := snapshot.logs }

def finalizedWorld
    (emptyCodeHash : hash) (deleteAtEnd : address → Bool)
    (world : PersistentWorld) : PersistentWorld :=
  { accountAt := fun account =>
      accountWithoutTransactionFlags
        (if deleteAtEnd account then
          deletedAccount emptyCodeHash (world.accountAt account)
        else
          world.accountAt account)
    storageAt := fun account slot =>
      if deleteAtEnd account then zeroWord
      else world.storageAt account slot }

/- `deleteAtEnd` is computed by the pure Sail fork policy.  The host transition
only applies that semantic decision and clears transaction-local flags. -/
def referenceCommitTransaction
    (emptyCodeHash : hash) (deleteAtEnd : address → Bool)
    (state : ReferenceWorldState) : ReferenceWorldState :=
  let committed := finalizedWorld emptyCodeHash deleteAtEnd state.txCurrent
  { blockStart := state.blockStart
    blockCurrent := committed
    txStart := committed
    txCurrent := committed
    transient := state.transient
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    logs := state.logs }

/- A delta is a finite extensional witness for every changed key, not a cache
or materialization log. -/
structure WorldDelta where
  accounts : List address
  storage : address → List word

def worldDeltaDescribes
    (before after : PersistentWorld) (delta : WorldDelta) : Prop :=
  delta.accounts.Nodup ∧
  (∀ account,
    account ∈ delta.accounts ↔
      before.accountAt account ≠ after.accountAt account ∨
      ∃ slot, before.storageAt account slot ≠ after.storageAt account slot) ∧
  (∀ account,
    (delta.storage account).Nodup ∧
    ∀ slot,
      slot ∈ delta.storage account ↔
        before.storageAt account slot ≠ after.storageAt account slot)

def keyBeforeAll {α : Type} (key : α → Nat) (first : α) : List α → Prop
  | [] => True
  | next :: tail => key first < key next ∧ keyBeforeAll key first tail

def strictlySortedBy {α : Type} (key : α → Nat) : List α → Prop
  | [] => True
  | first :: rest =>
      keyBeforeAll key first rest ∧ strictlySortedBy key rest

def fixedBEBytes (width value : Nat) : List byte :=
  (List.range width).map fun index =>
    BitVec.ofNat 8 ((value / (256 ^ (width - 1 - index))) % 256)

def wordToNat (value : word) : Nat :=
  value.value

def addressBytesBE (bytes : address) : List byte :=
  bytes.toList.reverse

def bytesBEToNat (bytes : List byte) : Nat :=
  bytes.foldl (fun value byte => value * 256 + byte.toNat) 0

def b256ToNat (bytes : b256) : Nat :=
  bytesBEToNat bytes.toList.reverse

abbrev PureKeccak := List byte → hash

def accountSecureKey (keccak : PureKeccak) (account : address) : hash :=
  keccak (addressBytesBE account)

def storageSecureKey (keccak : PureKeccak) (slot : word) : hash :=
  keccak (fixedBEBytes 32 (wordToNat slot))

def worldDeltaSecurelyOrdered
    (keccak : PureKeccak) (delta : WorldDelta) : Prop :=
  strictlySortedBy (fun account => b256ToNat (accountSecureKey keccak account))
    delta.accounts ∧
  ∀ account,
    strictlySortedBy (fun slot => b256ToNat (storageSecureKey keccak slot))
      (delta.storage account)

/- The raw account/storage/transient/warm/log externs collectively implement
this operation-level interface.  `CheckpointHandle` models Sail's opaque
`StateCheckpoint`; `checkpointDenotes` is a ghost relation between a handle and
the semantic snapshot it names.  Numeric token encodings, registries, row
encodings, active lengths, and drain cursors remain backend-private. -/
structure WorldStateContract where
  CheckpointHandle : Type
  BackendState : Type
  observe : BackendState → ReferenceWorldState
  startBlock : PersistentWorld → BackendState
  beginTransaction : BackendState → BackendState
  readAccount : BackendState → address → Account
  readStorageCurrent : BackendState → address → word → word
  readStorageOriginal : BackendState → address → word → word
  writeAccount : BackendState → address → Account → BackendState
  writeStorage : BackendState → address → word → word → BackendState
  clearStorage : BackendState → address → BackendState
  deleteAccount : hash → BackendState → address → BackendState
  checkpoint : BackendState → CheckpointHandle × BackendState
  checkpointDenotes : BackendState → CheckpointHandle → TransactionSnapshot → Prop
  revert : BackendState → CheckpointHandle → Option BackendState
  commitTransaction : hash → (address → Bool) → BackendState → BackendState
  cacheAccount : BackendState → address → Account → BackendState
  cacheStorage : BackendState → address → word → word → BackendState
  readTransient : BackendState → address → word → word
  writeTransient : BackendState → address → word → word → BackendState
  touchWarmAddress : BackendState → address → Bool × BackendState
  touchWarmSlot : BackendState → address → word → Bool × BackendState
  appendLog : BackendState → LogEntry → BackendState
  readLogs : BackendState → List LogEntry
  transactionDelta : BackendState → WorldDelta
  blockDelta : BackendState → WorldDelta

/- The canonical semantic instantiation uses the reference world directly and
uses a `TransactionSnapshot` itself as the checkpoint handle.  Native backends
may instead use any opaque handle related to the same snapshot by
`checkpointDenotes`.  Delta enumerators are supplied explicitly because finite
support cannot be computed constructively from arbitrary total functions. -/
def referenceWorldStateContract
    (transactionDelta blockDelta : ReferenceWorldState → WorldDelta) :
    WorldStateContract where
  CheckpointHandle := TransactionSnapshot
  BackendState := ReferenceWorldState
  observe := id
  startBlock := referenceStartBlock
  beginTransaction := referenceBeginTransaction
  readAccount := referenceReadAccount
  readStorageCurrent := referenceReadStorageCurrent
  readStorageOriginal := referenceReadStorageOriginal
  writeAccount := referenceWriteAccount
  writeStorage := referenceWriteStorage
  clearStorage := referenceClearStorage
  deleteAccount := referenceDeleteAccount
  checkpoint := fun state => (referenceCheckpoint state, state)
  checkpointDenotes := fun _ handle snapshot =>
    transactionSnapshotEquivalent handle snapshot
  revert := fun state snapshot => some (referenceRevert state snapshot)
  commitTransaction := referenceCommitTransaction
  cacheAccount := fun state _ _ => state
  cacheStorage := fun state _ _ _ => state
  readTransient := referenceReadTransient
  writeTransient := referenceWriteTransient
  touchWarmAddress := referenceTouchWarmAddress
  touchWarmSlot := referenceTouchWarmSlot
  appendLog := referenceAppendLog
  readLogs := ReferenceWorldState.logs
  transactionDelta := transactionDelta
  blockDelta := blockDelta

def checkpointDenotationsPreserved
    (contract : WorldStateContract)
    (before after : contract.BackendState) : Prop :=
  ∀ checkpoint snapshot,
    contract.checkpointDenotes before checkpoint snapshot →
    contract.checkpointDenotes after checkpoint snapshot

/- Ordinary call-frame mutations preserve live checkpoint denotations.
Transaction boundaries may invalidate every handle and therefore deliberately
have no such obligation. -/

def worldStateBoundary
    (keccak : PureKeccak) (contract : WorldStateContract) : Prop :=
  (∀ initial,
    let state := contract.startBlock initial
    referenceWorldStateEquivalent (contract.observe state)
      (referenceStartBlock initial)) ∧
  (∀ state,
    let next := contract.beginTransaction state
    referenceWorldStateEquivalent (contract.observe next)
      (referenceBeginTransaction (contract.observe state))) ∧
  (∀ state account,
    contract.readAccount state account =
      referenceReadAccount (contract.observe state) account) ∧
  (∀ state account slot,
    contract.readStorageCurrent state account slot =
      referenceReadStorageCurrent (contract.observe state) account slot) ∧
  (∀ state account slot,
    contract.readStorageOriginal state account slot =
      referenceReadStorageOriginal (contract.observe state) account slot) ∧
  (∀ state account value,
    let next := contract.writeAccount state account value
    referenceWorldStateEquivalent (contract.observe next)
      (referenceWriteAccount (contract.observe state) account value) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state account slot value,
    let next := contract.writeStorage state account slot value
    referenceWorldStateEquivalent (contract.observe next)
      (referenceWriteStorage (contract.observe state) account slot value) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state account,
    let next := contract.clearStorage state account
    referenceWorldStateEquivalent (contract.observe next)
      (referenceClearStorage (contract.observe state) account) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ emptyCodeHash state account,
    let next := contract.deleteAccount emptyCodeHash state account
    referenceWorldStateEquivalent (contract.observe next)
      (referenceDeleteAccount emptyCodeHash (contract.observe state) account) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state,
    let result := contract.checkpoint state
    referenceWorldStateEquivalent (contract.observe result.2)
      (contract.observe state) ∧
    contract.checkpointDenotes result.2 result.1
      (referenceCheckpoint (contract.observe state)) ∧
    checkpointDenotationsPreserved contract state result.2) ∧
  (∀ state checkpoint snapshot,
    contract.checkpointDenotes state checkpoint snapshot →
    ∃ next,
      contract.revert state checkpoint = some next ∧
      referenceWorldStateEquivalent (contract.observe next)
        (referenceRevert (contract.observe state) snapshot)) ∧
  (∀ emptyCodeHash deleteAtEnd state,
    let next := contract.commitTransaction emptyCodeHash deleteAtEnd state
    referenceWorldStateEquivalent (contract.observe next)
      (referenceCommitTransaction
        emptyCodeHash deleteAtEnd (contract.observe state))) ∧
  (∀ state account,
    let next := contract.cacheAccount state account (contract.readAccount state account)
    referenceWorldStateEquivalent (contract.observe next) (contract.observe state) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state account slot,
    let next := contract.cacheStorage state account slot
      (contract.readStorageCurrent state account slot)
    referenceWorldStateEquivalent (contract.observe next) (contract.observe state) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state account slot,
    contract.readTransient state account slot =
      referenceReadTransient (contract.observe state) account slot) ∧
  (∀ state account slot value,
    let next := contract.writeTransient state account slot value
    referenceWorldStateEquivalent (contract.observe next)
      (referenceWriteTransient (contract.observe state) account slot value) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state account,
    let result := contract.touchWarmAddress state account
    let expected := referenceTouchWarmAddress (contract.observe state) account
    result.1 = expected.1 ∧
    referenceWorldStateEquivalent (contract.observe result.2) expected.2 ∧
    checkpointDenotationsPreserved contract state result.2) ∧
  (∀ state account slot,
    let result := contract.touchWarmSlot state account slot
    let expected := referenceTouchWarmSlot (contract.observe state) account slot
    result.1 = expected.1 ∧
    referenceWorldStateEquivalent (contract.observe result.2) expected.2 ∧
    checkpointDenotationsPreserved contract state result.2) ∧
  (∀ state entry,
    let next := contract.appendLog state entry
    referenceWorldStateEquivalent (contract.observe next)
      (referenceAppendLog (contract.observe state) entry) ∧
    checkpointDenotationsPreserved contract state next) ∧
  (∀ state, contract.readLogs state = (contract.observe state).logs) ∧
  (∀ state,
    worldDeltaDescribes
      (contract.observe state).txStart
      (contract.observe state).txCurrent
      (contract.transactionDelta state)) ∧
  (∀ state,
    worldDeltaDescribes
      (contract.observe state).blockStart
      (contract.observe state).blockCurrent
      (contract.blockDelta state)) ∧
  (∀ state, worldDeltaSecurelyOrdered keccak (contract.blockDelta state))

structure ValidWorldStateContract where
  keccak : PureKeccak
  operations : WorldStateContract
  valid : worldStateBoundary keccak operations

/- The extracted model sees the exact EVM host-memory interface declared in
`sail/host/memory.sail`.  Keeping these operations in one concrete contract
prevents proofs from silently substituting an unrelated generic memory model.
The `Evm.Functions` declarations below are only the generated-name/extern
adapters for this contract. -/
structure HostMemoryContract where
  readByte : memory_pointer → SailM (BitVec 8)
  writeByte : memory_pointer → BitVec 8 → SailM Unit
  clear : Unit → SailM Unit
  frameEnter : Unit → SailM source_pointer
  frameLeave : Unit → SailM Unit
  expand : (required : source_length) → SailM (EvmByteSliceLength required)
  move : memory_pointer → memory_pointer → memory_length → SailM Unit
  loadWord : memory_pointer → SailM word
  storeWord : memory_pointer → word → SailM Unit
  copySlice :
    EvmByteSlice → memory_pointer → source_pointer → memory_length → SailM Unit

axiom hostMemoryContract : HostMemoryContract

end Evm.Contracts

namespace Evm.Functions

@[extern "lean_evmsail_ancestor_hash_write"]
axiom ancestor_hash_write : ancestor_index → hash → SailM Unit
@[extern "lean_evmsail_ancestor_hash_read"]
axiom ancestor_hash_read : ancestor_index → SailM hash

@[extern "lean_evmsail_keccak256_segments"]
axiom keccak256_segments : List Bytes → SailM hash
@[extern "lean_evmsail_sha256_segments"]
axiom sha256_segments : List Bytes → SailM hash

@[extern "lean_evmsail_mem_read_byte"]
def mem_read_byte : memory_pointer → SailM (BitVec 8) :=
  Evm.Contracts.hostMemoryContract.readByte
@[extern "lean_evmsail_mem_write_byte"]
def mem_write_byte : memory_pointer → BitVec 8 → SailM Unit :=
  Evm.Contracts.hostMemoryContract.writeByte
@[extern "lean_evmsail_mem_clear"]
def mem_clear : Unit → SailM Unit :=
  Evm.Contracts.hostMemoryContract.clear
@[extern "lean_evmsail_mem_frame_enter"]
def mem_frame_enter : Unit → SailM source_pointer :=
  Evm.Contracts.hostMemoryContract.frameEnter
@[extern "lean_evmsail_mem_frame_leave"]
def mem_frame_leave : Unit → SailM Unit :=
  Evm.Contracts.hostMemoryContract.frameLeave
@[extern "lean_evmsail_mem_expand"]
def mem_expand (required : source_length) : SailM (EvmByteSliceLength required) :=
  Evm.Contracts.hostMemoryContract.expand required
@[extern "lean_evmsail_mem_move"]
def mem_move : memory_pointer → memory_pointer → memory_length → SailM Unit :=
  Evm.Contracts.hostMemoryContract.move
@[extern "lean_evmsail_mem_load_word"]
def mem_load_word : memory_pointer → SailM word :=
  Evm.Contracts.hostMemoryContract.loadWord
@[extern "lean_evmsail_mem_store_word"]
def mem_store_word : memory_pointer → word → SailM Unit :=
  Evm.Contracts.hostMemoryContract.storeWord

@[extern "lean_evmsail_host_scratch_store_bytes"]
axiom host_scratch_store_bytes
    (off : source_pointer) (bytes : List (BitVec 8)) (len : source_length) :
    SailM (ByteRegionResult (off + len))
@[extern "lean_evmsail_host_scratch_store_slice"]
axiom host_scratch_store_slice
    {sliceOff sliceLen : Nat}
    (off : source_pointer) (slice : EvmByteSliceFields sliceOff sliceLen) :
    SailM (ByteRegionResult (off + slice.len))
@[extern "lean_evmsail_host_scratch_store_b256"]
axiom host_scratch_store_b256
    (off : source_pointer) (bytes : b256) (len : source_length) :
    SailM (ByteRegionResult (off + len))
@[extern "lean_evmsail_host_scratch_truncate"]
axiom host_scratch_truncate : source_pointer → SailM Unit

@[extern "lean_evmsail_accelerator_ripemd160"]
axiom accelerator_ripemd160 : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_modexp"]
axiom accelerator_modexp : EvmByteSlice → byte_length → byte_length → byte_length → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_add"]
axiom accelerator_bn254_add : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_mul"]
axiom accelerator_bn254_mul : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bn254_pairing"]
axiom accelerator_bn254_pairing : EvmByteSlice → SailM (BitVec 2)
@[extern "lean_evmsail_accelerator_blake2f"]
axiom accelerator_blake2f : EvmByteSlice → blake2_rounds → y_parity → SailM Bool
@[extern "lean_evmsail_accelerator_kzg_point_evaluation"]
axiom accelerator_kzg_point_evaluation : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g1_add"]
axiom accelerator_bls_g1_add : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g1_msm"]
axiom accelerator_bls_g1_msm : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g2_add"]
axiom accelerator_bls_g2_add : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_g2_msm"]
axiom accelerator_bls_g2_msm : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_pairing"]
axiom accelerator_bls_pairing : EvmByteSlice → SailM (BitVec 2)
@[extern "lean_evmsail_accelerator_bls_map_fp_to_g1"]
axiom accelerator_bls_map_fp_to_g1 : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_bls_map_fp2_to_g2"]
axiom accelerator_bls_map_fp2_to_g2 : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_accelerator_p256_verify"]
axiom accelerator_p256_verify : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_secp256k1_verify"]
axiom secp256k1_verify : hash → word → word → word → word → SailM Bool
@[extern "lean_evmsail_host_ecrecover"]
axiom host_ecrecover : hash → y_parity → word → word → SailM AddressResult

@[extern "lean_evmsail_stack_reset"]
axiom stack_reset : Unit → SailM Unit
@[extern "lean_evmsail_stack_enter_frame"]
axiom stack_enter_frame : Unit → SailM Unit
@[extern "lean_evmsail_stack_leave_frame"]
axiom stack_leave_frame : Unit → SailM Unit
@[extern "lean_evmsail_stack_depth"]
axiom stack_depth : Unit → SailM operand_stack_height
@[extern "lean_evmsail_stack_push_word"]
axiom stack_push_word : word → SailM Unit
@[extern "lean_evmsail_stack_pop_word"]
axiom stack_pop_word : Unit → SailM word
@[extern "lean_evmsail_stack_peek_word"]
axiom stack_peek_word : stack_index → SailM word
@[extern "lean_evmsail_stack_set_word"]
axiom stack_set_word : stack_index → word → SailM Unit

@[extern "lean_evmsail_stateless_input"]
axiom stateless_input : Unit → SailM EvmByteSlice
@[extern "lean_evmsail_host_slice_byte"]
axiom host_slice_byte : EvmByteSlice → source_pointer → SailM (BitVec 8)
@[extern "lean_evmsail_host_slice_count_nonzero"]
axiom host_slice_count_nonzero : EvmByteSlice → SailM host_access
@[extern "lean_evmsail_host_slice_strided_zero"]
axiom host_slice_strided_zero :
  EvmByteSlice → source_pointer → host_access → host_access → host_access → SailM Bool
@[extern "lean_evmsail_host_slice_load_word"]
axiom host_slice_load_word : EvmByteSlice → source_pointer → SailM word
@[extern "lean_evmsail_host_slice_load_n_word"]
axiom host_slice_load_n_word : EvmByteSlice → source_pointer → host_access → SailM word
@[extern "lean_evmsail_host_slice_copy_to_memory"]
def host_slice_copy_to_memory :
    EvmByteSlice → memory_pointer → source_pointer → memory_length → SailM Unit :=
  Evm.Contracts.hostMemoryContract.copySlice
@[extern "lean_evmsail_bytes_segments_equal_slice"]
axiom bytes_segments_equal_slice : List Bytes → EvmByteSlice → SailM Bool

@[extern "lean_evmsail_output_buffer_store"]
axiom output_buffer_store : EvmByteSlice → SailM Bool
@[extern "lean_evmsail_output_buffer_store_word"]
axiom output_buffer_store_word : word → SailM Bool
@[extern "lean_evmsail_output_buffer_store_words"]
axiom output_buffer_store_words : word → word → SailM Bool
@[extern "lean_evmsail_public_output_write"]
axiom public_output_write : EvmByteSlice → SailM Bool

@[extern "lean_evmsail_code_db_lookup"]
axiom code_db_lookup : hash → SailM (Option Code)
@[extern "lean_evmsail_jumpdest_table_alloc"]
axiom jumpdest_table_alloc : code_length → SailM JumpdestRef
@[extern "lean_evmsail_jumpdest_table_store_chunk"]
axiom jumpdest_table_store_chunk :
  JumpdestRef → code_length → code_chunk_index → JumpdestChunk → SailM Bool
@[extern "lean_evmsail_code_db_store"]
axiom code_db_store : CodeSlice → JumpdestRef → SailM hash
@[extern "lean_evmsail_jumpdest_ref_contains"]
axiom jumpdest_ref_contains : JumpdestRef → code_length → code_pointer → SailM Bool
@[extern "lean_evmsail_code_intern_delegation"]
axiom code_intern_delegation : address → JumpdestRef → SailM hash
@[extern "lean_evmsail_code_db_read_delegation"]
axiom code_db_read_delegation : hash → SailM AddressResult

@[extern "lean_evmsail_transient_reset"]
axiom transient_reset : Unit → SailM Unit
@[extern "lean_evmsail_transient_store"]
axiom transient_store : address → word → word → SailM Unit
@[extern "lean_evmsail_transient_load"]
axiom transient_load : address → word → SailM word

@[extern "lean_evmsail_state_checkpoint_reset"]
axiom state_checkpoint_reset : Unit → SailM Unit
@[extern "lean_evmsail_state_checkpoint"]
axiom state_checkpoint : Unit → SailM StateCheckpoint
@[extern "lean_evmsail_state_revert"]
axiom state_revert : StateCheckpoint → SailM Unit

@[extern "lean_evmsail_storage_tx_update"]
axiom storage_tx_update : StorageEntry → SailM Unit
@[extern "lean_evmsail_storage_tx_get"]
axiom storage_tx_get : StorageKey → SailM (Option StorageValue)
@[extern "lean_evmsail_storage_tx_pop"]
axiom storage_tx_pop : Unit → SailM (Option StorageEntry)
@[extern "lean_evmsail_storage_tx_clear"]
axiom storage_tx_clear : address → SailM Unit
@[extern "lean_evmsail_storage_tx_reset"]
axiom storage_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_storage_has_writes"]
axiom storage_has_writes : address → SailM Bool
@[extern "lean_evmsail_storage_block_get"]
axiom storage_block_get : StorageKey → SailM (Option StorageValue)
@[extern "lean_evmsail_storage_block_put"]
axiom storage_block_put : StorageEntry → SailM Unit
@[extern "lean_evmsail_storage_block_cache"]
axiom storage_block_cache : StorageKey → word → SailM Unit
@[extern "lean_evmsail_storage_block_clear"]
axiom storage_block_clear : address → SailM Unit
@[extern "lean_evmsail_storage_block_iter_begin"]
axiom storage_block_iter_begin : address → SailM Unit
@[extern "lean_evmsail_storage_block_iter_next"]
axiom storage_block_iter_next : address → SailM (Option StorageEntry)

@[extern "lean_evmsail_acct_tx_get"]
axiom acct_tx_get : address → SailM (Option Account)
@[extern "lean_evmsail_acct_tx_update"]
axiom acct_tx_update : address → Account → SailM Unit
@[extern "lean_evmsail_acct_tx_set_balance"]
axiom acct_tx_set_balance : address → word → SailM Unit
@[extern "lean_evmsail_acct_tx_set_nonce"]
axiom acct_tx_set_nonce : address → account_nonce → SailM Unit
@[extern "lean_evmsail_acct_tx_set_code_hash"]
axiom acct_tx_set_code_hash : address → hash → SailM Unit
@[extern "lean_evmsail_acct_tx_pop_ascending"]
axiom acct_tx_pop_ascending : Unit → SailM (Option AcctEntry)
@[extern "lean_evmsail_acct_tx_reset"]
axiom acct_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_acct_block_get"]
axiom acct_block_get : address → SailM (Option Account)
@[extern "lean_evmsail_acct_block_write"]
axiom acct_block_write : AcctEntry → SailM Unit
@[extern "lean_evmsail_acct_block_cache"]
axiom acct_block_cache : address → Account → SailM Unit
@[extern "lean_evmsail_acct_block_iter_begin"]
axiom acct_block_iter_begin : Unit → SailM Unit
@[extern "lean_evmsail_acct_block_iter_next"]
axiom acct_block_iter_next : Unit → SailM (Option AcctEntry)
@[extern "lean_evmsail_acct_post_storage_root_store"]
axiom acct_post_storage_root_store : address → hash → SailM Unit
@[extern "lean_evmsail_acct_post_storage_root_read"]
axiom acct_post_storage_root_read : address → SailM hash

@[extern "lean_evmsail_bal_reset"]
axiom bal_reset : Unit → SailM Unit
@[extern "lean_evmsail_bal_set_index"]
axiom bal_set_index : Nat → SailM Unit
@[extern "lean_evmsail_bal_account_touch"]
axiom bal_account_touch : address → SailM Unit
@[extern "lean_evmsail_bal_storage_change"]
axiom bal_storage_change : address → word → word → SailM Unit
@[extern "lean_evmsail_bal_storage_read"]
axiom bal_storage_read : address → word → SailM Unit
@[extern "lean_evmsail_bal_balance_change"]
axiom bal_balance_change : address → word → SailM Unit
@[extern "lean_evmsail_bal_nonce_change"]
axiom bal_nonce_change : address → account_nonce → SailM Unit
@[extern "lean_evmsail_bal_code_change"]
axiom bal_code_change : address → hash → SailM Unit
@[extern "lean_evmsail_bal_prepare"]
axiom bal_prepare : Unit → SailM Unit
@[extern "lean_evmsail_bal_account_count"]
axiom bal_account_count : Unit → SailM item_count
@[extern "lean_evmsail_bal_account_address"]
axiom bal_account_address : item_index → SailM address
@[extern "lean_evmsail_bal_storage_change_count"]
axiom bal_storage_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_storage_change_slot"]
axiom bal_storage_change_slot : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_storage_change_index"]
axiom bal_storage_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_storage_change_value"]
axiom bal_storage_change_value : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_storage_read_count"]
axiom bal_storage_read_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_storage_read_slot"]
axiom bal_storage_read_slot : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_balance_change_count"]
axiom bal_balance_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_balance_change_index"]
axiom bal_balance_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_balance_change_value"]
axiom bal_balance_change_value : item_index → item_index → SailM word
@[extern "lean_evmsail_bal_nonce_change_count"]
axiom bal_nonce_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_nonce_change_index"]
axiom bal_nonce_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_nonce_change_value"]
axiom bal_nonce_change_value : item_index → item_index → SailM account_nonce
@[extern "lean_evmsail_bal_code_change_count"]
axiom bal_code_change_count : item_index → SailM item_count
@[extern "lean_evmsail_bal_code_change_index"]
axiom bal_code_change_index : item_index → item_index → SailM item_index
@[extern "lean_evmsail_bal_code_change_hash"]
axiom bal_code_change_hash : item_index → item_index → SailM hash

@[extern "lean_evmsail_warm_reset"]
axiom warm_reset : Unit → SailM Unit
@[extern "lean_evmsail_warm_addr_touch"]
axiom warm_addr_touch : address → SailM Bool
@[extern "lean_evmsail_warm_slot_touch"]
axiom warm_slot_touch : address → word → SailM Bool

@[extern "lean_evmsail_logs_tx_reset"]
axiom logs_tx_reset : Unit → SailM Unit
@[extern "lean_evmsail_log_append"]
axiom log_append : address → List word → Bytes → SailM Unit
@[extern "lean_evmsail_read_logs"]
axiom read_logs : Unit → SailM (List LogEntry)

@[extern "lean_evmsail_nodedb_reset"]
axiom nodedb_reset : Unit → SailM Unit
@[extern "lean_evmsail_nodedb_insert"]
axiom nodedb_insert : hash → source_pointer → byte_length → SailM Unit
@[extern "lean_evmsail_nodedb_lookup"]
axiom nodedb_lookup : hash → SailM EvmByteSlice

end Evm.Functions
