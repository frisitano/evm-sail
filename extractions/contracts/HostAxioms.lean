import Evm.Defs

open Sail
open ConcurrencyInterfaceV1
open Evm.Defs

/-!
## Extensional world-state contract

The declarations in `Evm.Functions` below are the raw symbols imported by the
generated Sail model.  This namespace gives those account, persistent-storage,
and transient-storage symbols an implementation-independent meaning.

Nothing here describes the layout of either C backend's `state_db.c` or
`kernel_state.c`:
there are no table rows, generations, undo cursors, physical deletions, or
first-write/copy-on-write rules.  Such devices are permitted only through a
refinement from a backend state to the total maps and logical snapshots below.
-/
namespace Evm.Contracts

deriving instance DecidableEq for word
deriving instance DecidableEq for address

def zeroWord : word := 0

structure PersistentWorld where
  accountAt : address → Account
  storageAt : address → word → word

def accountWithClearedStorage (account : Account) : Account :=
  { account with storage_cleared := true }

def accountWithoutTransactionFlags (account : Account) : Account :=
  { account with created := false, selfdestructed := false }

def zeroAccountNonce : account_nonce :=
  0

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
  value

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

end Evm.Contracts

namespace Evm

/- The executable extraction extends Sail's register state with the mutable
host stores that the C model keeps behind its FFI boundary.  Keeping this
state explicit makes ordinary memory and database operations executable Lean
definitions; only genuine cryptographic accelerators remain external. -/
structure MemoryFrame where
  base : Nat
  established : Nat
  deriving Inhabited

structure JumpdestTable where
  reference : JumpdestRef
  codeLength : code_length
  positions : List code_pointer
  deriving Inhabited

structure JournalSnapshot where
  transient : List (StorageKey × word)
  storageTx : List (StorageKey × StorageValue)
  accountTx : List (address × AcctValue)
  warmAddresses : List address
  warmSlots : List StorageKey
  logs : List LogEntry
  deriving Inhabited

structure BalStorageChangeRecord where
  account : address
  slot : word
  index : block_access_index
  value : word
  deriving Inhabited

structure BalStorageReadRecord where
  account : address
  slot : word
  deriving Inhabited

structure BalBalanceChangeRecord where
  account : address
  index : block_access_index
  value : word
  deriving Inhabited

structure BalNonceChangeRecord where
  account : address
  index : block_access_index
  value : account_nonce
  deriving Inhabited

structure BalCodeChangeRecord where
  account : address
  index : block_access_index
  value : hash
  deriving Inhabited

structure HostState where
  memoryBytes : Array byte
  memoryFrames : List MemoryFrame
  inputBytes : Array byte
  scratchBytes : Array byte
  codeBytes : Array byte
  logBytes : Array byte
  outputBytes : Array byte
  publicOutput : Array byte
  stackFrames : List (List word)
  continuationFrames : List FrameContinuation
  ancestorHashes : Array hash
  jumpdestTables : List JumpdestTable
  codeDb : List (hash × Code)
  delegations : List (hash × address)
  transient : List (StorageKey × word)
  storageTx : List (StorageKey × StorageValue)
  storageBlock : List (StorageKey × StorageValue)
  storageIterator : List StorageEntry
  accountTx : List (address × AcctValue)
  accountBlock : List (address × AcctValue)
  accountIterator : List AcctEntry
  postStorageRoots : List (address × hash)
  warmAddresses : List address
  warmSlots : List StorageKey
  logs : List LogEntry
  checkpoints : List JournalSnapshot
  nodeDb : List (hash × ByteSlice)
  balAccounts : List address
  balStorageChanges : List BalStorageChangeRecord
  balStorageReads : List BalStorageReadRecord
  balBalanceChanges : List BalBalanceChangeRecord
  balNonceChanges : List BalNonceChangeRecord
  balCodeChanges : List BalCodeChangeRecord
  balIterator : List BalIterEntry

def initialHostState : HostState where
  memoryBytes := #[]
  memoryFrames := [{ base := 0, established := 0 }]
  inputBytes := #[]
  scratchBytes := #[]
  codeBytes := #[]
  logBytes := #[]
  outputBytes := #[]
  publicOutput := #[]
  stackFrames := [[]]
  continuationFrames := []
  ancestorHashes := #[]
  jumpdestTables := []
  codeDb := []
  delegations := []
  transient := []
  storageTx := []
  storageBlock := []
  storageIterator := []
  accountTx := []
  accountBlock := []
  accountIterator := []
  postStorageRoots := []
  warmAddresses := []
  warmSlots := []
  logs := []
  checkpoints := []
  nodeDb := []
  balAccounts := []
  balStorageChanges := []
  balStorageReads := []
  balBalanceChanges := []
  balNonceChanges := []
  balCodeChanges := []
  balIterator := []

abbrev SailM (α : Type) :=
  StateT HostState Evm.Defs.SailM α

end Evm

namespace Evm.Functions

private def ensureArraySize
    (bytes : Array byte) (required : Nat) : Array byte :=
  if required ≤ bytes.size then bytes
  else bytes ++ Array.replicate (required - bytes.size) 0

private def writeArrayByte
    (bytes : Array byte) (position : Nat) (value : byte) : Array byte :=
  (ensureArraySize bytes (position + 1)).set! position value

private def writeArrayBytes
    (bytes : Array byte) (position : Nat) (values : List byte) : Array byte :=
  values.zipIdx.foldl
    (fun result pair => writeArrayByte result (position + pair.2) pair.1)
    bytes

private def vectorBytes {size : Nat} (bytes : Vector byte size) : List byte :=
  bytes.toList

def ancestor_hash_write (index : ancestor_index) (value : hash) : SailM Unit :=
  modify fun state =>
    { state with
      ancestorHashes :=
        let hashes :=
          if index < state.ancestorHashes.size then
            state.ancestorHashes
          else
            state.ancestorHashes ++
              Array.replicate (index + 1 - state.ancestorHashes.size) default
        hashes.set! index value }

def ancestor_hash_read (index : ancestor_index) : SailM hash := do
  let state ← get
  pure (state.ancestorHashes.getD index default)

private def currentMemoryFrame (state : HostState) : MemoryFrame :=
  state.memoryFrames.head?.getD default

private def replaceCurrentMemoryFrame
    (state : HostState) (frame : MemoryFrame) : HostState :=
  { state with memoryFrames := frame :: state.memoryFrames.drop 1 }

private def zeroMemoryRange
    (bytes : Array byte) (start count : Nat) : Array byte :=
  (List.range count).foldl
    (fun result index => result.set! (start + index) 0)
    (ensureArraySize bytes (start + count))

private def establishMemory (required : Nat) : SailM MemoryFrame := do
  let state ← get
  let frame := currentMemoryFrame state
  if required ≤ frame.established then
    pure frame
  else
    let bytes :=
      zeroMemoryRange state.memoryBytes
        (frame.base + frame.established) (required - frame.established)
    let frame := { frame with established := required }
    set (replaceCurrentMemoryFrame { state with memoryBytes := bytes } frame)
    pure frame

def mem_read_byte (off : memory_pointer) : SailM byte := do
  let state ← get
  let frame := currentMemoryFrame state
  pure <| if off < frame.established then
    state.memoryBytes.getD (frame.base + off) 0
  else
    0

def mem_write_byte (off : memory_pointer) (value : byte) : SailM Unit := do
  let frame ← establishMemory (off + 1)
  modify fun state =>
    { state with
      memoryBytes := writeArrayByte state.memoryBytes (frame.base + off) value }

def mem_clear (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with memoryFrames := [{ base := 0, established := 0 }] }

def mem_frame_enter (_ : Unit) : SailM source_pointer := do
  let state ← get
  let parent := currentMemoryFrame state
  let base := parent.base + parent.established
  set { state with memoryFrames := { base := base, established := 0 } :: state.memoryFrames }
  pure base

def mem_frame_leave (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with
      memoryFrames :=
        match state.memoryFrames with
        | _ :: parent :: rest => parent :: rest
        | frames => frames }

def mem_expand (required : source_length) : SailM (ByteSliceLength required) := do
  let frame ← establishMemory required
  pure ⟨frame.base, ⟨required,
    { source := ByteSource.EvmMemorySource, off := frame.base, len := required }⟩⟩

private def snapshotMemory
    (bytes : Array byte) (base off len : Nat) : List byte :=
  List.ofFn fun index : Fin len => bytes.getD (base + off + index) 0

def mem_move
    (dst : memory_pointer) (src : memory_pointer) (len : memory_length) :
    SailM Unit := do
  let frame ← establishMemory (max (src + len) (dst + len))
  let state ← get
  let values := snapshotMemory state.memoryBytes frame.base src len
  let indexed := values.zipIdx
  let bytes := indexed.foldl
    (fun result pair =>
      writeArrayByte result (frame.base + dst + pair.2) pair.1)
    state.memoryBytes
  set { state with memoryBytes := bytes }

def mem_load_word (off : memory_pointer) : SailM word := do
  let state ← get
  let frame := currentMemoryFrame state
  let value := (List.range 32).foldl
    (fun result index =>
      let byte :=
        if off + index < frame.established then
          state.memoryBytes.getD (frame.base + off + index) 0
        else
          0
      result * 256 + byte.toNat)
    0
  pure value

def mem_store_word (off : memory_pointer) (value : word) : SailM Unit := do
  let frame ← establishMemory (off + 32)
  modify fun state =>
    let indexed := List.range 32
    let bytes := indexed.foldl
      (fun result index =>
        let shift := 8 * (31 - index)
        let byte := BitVec.ofNat 8 ((value / (2 ^ shift)) % 256)
        writeArrayByte result (frame.base + off + index) byte)
      state.memoryBytes
    { state with memoryBytes := bytes }

def host_scratch_store_bytes
    (off : source_pointer) (bytes : List (BitVec 8)) (len : source_length) :
    SailM (ByteRegionResult (off + len)) := do
  modify fun state =>
    { state with scratchBytes := writeArrayBytes state.scratchBytes off (bytes.take len) }
  pure (.ByteRegionReady ⟨0, ⟨off + len,
    { source := .ScratchSource, off := 0, len := off + len }⟩⟩)

private def sourceArray (state : HostState) (source : ByteSource) : Array byte :=
  match source with
  | .StatelessInputSource => state.inputBytes
  | .CodeSource => state.codeBytes
  | .LogDataSource => state.logBytes
  | .OutputSource => state.outputBytes
  | .ScratchSource => state.scratchBytes
  | .EvmMemorySource => #[]

private def makeByteSlice
    (source : ByteSource) (off len : Nat) : ByteSlice :=
  ⟨off, ⟨len, { source := source, off := off, len := len }⟩⟩

private def readSourceByte
    (state : HostState) (source : ByteSource) (position : Nat) : byte :=
  match source with
  | .EvmMemorySource => state.memoryBytes.getD position 0
  | other => (sourceArray state other).getD position 0

private def readSliceByte
    (state : HostState) (slice : ByteSlice) (position : Nat) : byte :=
  let ⟨_, ⟨_, fields⟩⟩ := slice
  if position < fields.len then
    readSourceByte state fields.source (fields.off + position)
  else
    0

private def materializeSlice
    (state : HostState) (slice : ByteSlice) : List byte :=
  let ⟨_, ⟨_, fields⟩⟩ := slice
  (List.range fields.len).map (readSliceByte state slice)

def host_scratch_store_slice
    {sliceOff sliceLen : Nat}
    (off : source_pointer) (slice : ByteSliceFields sliceOff sliceLen) :
    SailM (ByteRegionResult (off + slice.len)) := do
  let state ← get
  let bytes := (List.range slice.len).map fun index =>
    readSourceByte state slice.source (slice.off + index)
  set { state with scratchBytes := writeArrayBytes state.scratchBytes off bytes }
  pure (.ByteRegionReady ⟨0, ⟨off + slice.len,
    { source := .ScratchSource, off := 0, len := off + slice.len }⟩⟩)

def host_scratch_store_b256
    (off : source_pointer) (bytes : b256) (len : source_length) :
    SailM (ByteRegionResult (off + len)) := do
  modify fun state =>
    { state with
      scratchBytes :=
        writeArrayBytes state.scratchBytes off
          (vectorBytes bytes |>.reverse |>.take len) }
  pure (.ByteRegionReady ⟨0, ⟨off + len,
    { source := .ScratchSource, off := 0, len := off + len }⟩⟩)

def host_scratch_truncate (length : source_pointer) : SailM Unit :=
  modify fun state => { state with scratchBytes := state.scratchBytes.extract 0 length }

private def currentStack (state : HostState) : List word :=
  state.stackFrames.head?.getD []

private def replaceCurrentStack (state : HostState) (stack : List word) : HostState :=
  { state with stackFrames := stack :: state.stackFrames.drop 1 }

private def replaceListAt (values : List α) (index : Nat) (value : α) : List α :=
  match values, index with
  | [], _ => []
  | _ :: rest, 0 => value :: rest
  | head :: rest, index + 1 => head :: replaceListAt rest index value

private def writeListAt (values : List word) (position : Nat) (value : word) : List word :=
  if position < values.length then
    replaceListAt values position value
  else
    replaceListAt
      (values ++ List.replicate (position + 1 - values.length) default)
      position value

/-- The reference reading of the opaque `StackTop` cursor token is the frame
height (the spec C ABI's choice): slot `index` below cursor `top` addresses
position `top - 1 - index` of the active frame's bottom-indexed word list. -/
private def stackSlotPosition (top : StackTop) (index : stack_index) : Nat :=
  top.toNat - 1 - index

def stack_reset (_ : Unit) : SailM StackTop := do
  modify fun state => { state with stackFrames := [[]] }
  pure 0

def operand_stack_push_empty_frame (_ : Unit) : SailM StackTop := do
  modify fun state => { state with stackFrames := [] :: state.stackFrames }
  pure 0

def operand_stack_pop_frame (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with
      stackFrames :=
        match state.stackFrames with
        | _ :: parent :: rest => parent :: rest
        | frames => frames }

def stack_top_height (top : StackTop) : SailM operand_stack_height :=
  pure top.toNat

def stack_slot_read (top : StackTop) (index : stack_index) : SailM word := do
  pure ((currentStack (← get)).getD (stackSlotPosition top index) default)

def stack_slot_write
    (top : StackTop) (index : stack_index) (value : word) : SailM Unit :=
  modify fun state =>
    replaceCurrentStack state
      (writeListAt (currentStack state) (stackSlotPosition top index) value)

def stack_top_advance (top : StackTop) (count : stack_slot_count) : SailM StackTop :=
  pure (top + BitVec.ofNat 64 count)

def stack_top_retreat (top : StackTop) (count : stack_slot_count) : SailM StackTop :=
  pure (top - BitVec.ofNat 64 count)

def frame_stack_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with continuationFrames := [] }

def frame_stack_push (continuation : FrameContinuation) : SailM Unit :=
  modify fun state =>
    { state with continuationFrames := continuation :: state.continuationFrames }

def frame_stack_pop (_ : Unit) : SailM FrameContinuation := do
  let state ← get
  match state.continuationFrames with
  | continuation :: rest =>
      set { state with continuationFrames := rest }
      pure continuation
  | [] => pure (FrameContinuation.Empty ())

def stateless_input (_ : Unit) : SailM ByteSlice := do
  let length := (← get).inputBytes.size
  pure (makeByteSlice .StatelessInputSource 0 length)

def host_slice_byte
    (slice : ByteSlice) (position : source_pointer) : SailM byte := do
  pure (readSliceByte (← get) slice position)

def host_slice_count_nonzero (slice : ByteSlice) : SailM host_access := do
  pure (materializeSlice (← get) slice |>.countP (· != 0))

def host_slice_strided_zero
    (slice : ByteSlice) (start stride width count : host_access) : SailM Bool := do
  let state ← get
  pure <| (List.range count).all fun item =>
    (List.range width).all fun offset =>
      readSliceByte state slice (start + item * stride + offset) == 0

private def bytesToWord (bytes : List byte) : word :=
  bytes.foldl (fun result value => result * 256 + value.toNat) 0

def host_slice_load_word
    (slice : ByteSlice) (position : source_pointer) : SailM word := do
  let state ← get
  pure <| bytesToWord <| (List.range 32).map fun index =>
    readSliceByte state slice (position + index)

def host_slice_load_n_word
    (slice : ByteSlice) (position : source_pointer) (count : host_access) :
    SailM word := do
  let state ← get
  pure <| bytesToWord <| (List.range count).map fun index =>
    readSliceByte state slice (position + index)

def host_slice_copy_to_memory
    (slice : ByteSlice) (dst : memory_pointer)
    (src : source_pointer) (len : memory_length) : SailM Unit := do
  let state ← get
  let values := (List.range len).map fun index =>
    readSliceByte state slice (src + index)
  let frame ← establishMemory (dst + len)
  modify fun state =>
    let bytes := values.zipIdx.foldl
      (fun result pair =>
        writeArrayByte result (frame.base + dst + pair.2) pair.1)
      state.memoryBytes
    { state with memoryBytes := bytes }

private def materializeBytes (state : HostState) (bytes : Bytes) : List byte :=
  match bytes with
  | .BytesList materialized => materialized.data.take materialized.len
  | .BytesSlice slice => materializeSlice state slice
  | .BytesFixed32 fixed =>
      vectorBytes fixed.data |>.reverse |>.take fixed.len

def bytes_segments_equal_slice
    (segments : List Bytes) (slice : ByteSlice) : SailM Bool := do
  let state ← get
  pure (segments.flatMap (materializeBytes state) == materializeSlice state slice)

def byte_slices_equal
    (left right : ByteSlice) : SailM Bool := do
  let state ← get
  pure (materializeSlice state left == materializeSlice state right)

private def wordBytes (value : word) : List byte :=
  (List.range 32).map fun index =>
    BitVec.ofNat 8 ((value / (2 ^ (8 * (31 - index)))) % 256)

private def hashBytes (value : hash) : List byte :=
  (vectorBytes value).reverse

@[extern "lean_evmsail_accelerate_bytes"]
private opaque nativeAccelerateBytes
    (selector : UInt8) (input : ByteArray)
    (first second third : UInt64) : Option ByteArray

private def toNativeBytes (bytes : List byte) : ByteArray :=
  ⟨(bytes.map fun value => UInt8.ofNat value.toNat).toArray⟩

private def fromNativeBytes (bytes : ByteArray) : Array byte :=
  bytes.data.map fun value => BitVec.ofNat 8 value.toNat

-- The accelerator ABI returns wire-order (big-endian) bytes. Sail's
-- `vector(size, dec, byte)` stores index zero at the least-significant end,
-- so fixed hashes and addresses reverse the ABI sequence at this boundary.
private def nativeDecVectorFromBE
    (size : Nat) (bytes : ByteArray) : Vector byte size :=
  Vector.ofFn fun index =>
    BitVec.ofNat 8 (bytes.data.getD (size - 1 - index.val) 0).toNat

private def callNative
    (selector : UInt8) (input : List byte)
    (first second third : Nat) : Option ByteArray :=
  nativeAccelerateBytes selector (toNativeBytes input)
    (UInt64.ofNat first) (UInt64.ofNat second) (UInt64.ofNat third)

private def bytesBENat (bytes : List byte) : Nat :=
  bytes.foldl (fun value next => value * 256 + next.toNat) 0

private def addressSortKey (value : address) : Nat :=
  bytesBENat (vectorBytes value).reverse

private def secureSortKey (bytes : List byte) : Nat :=
  match callNative 0 bytes 0 0 0 with
  | some digest =>
      digest.data.foldl (fun value next => value * 256 + next.toNat) 0
  | none => 0

private def accountSecureSortKey (value : address) : Nat :=
  secureSortKey (vectorBytes value).reverse

private def storageSecureSortKey (value : word) : Nat :=
  secureSortKey (wordBytes value)

private def acceleratorOutput
    (selector : UInt8) (slice : ByteSlice)
    (first second third : Nat) : SailM Bool := do
  let state ← get
  match callNative selector (materializeSlice state slice) first second third with
  | none =>
      set { state with outputBytes := #[] }
      pure false
  | some output =>
      set { state with outputBytes := fromNativeBytes output }
      pure true

private def acceleratorCheck
    (selector : UInt8) (slice : ByteSlice) : SailM Bool := do
  let state ← get
  pure <| match callNative selector (materializeSlice state slice) 0 0 0 with
    | some output => output.data.getD 0 0 != 0
    | none => false

private def acceleratorPairing
    (selector : UInt8) (slice : ByteSlice) : SailM (BitVec 2) := do
  let state ← get
  pure <| match callNative selector (materializeSlice state slice) 0 0 0 with
    | some output =>
        BitVec.ofNat 2 (if output.data.getD 0 0 == 0 then 2 else 3)
    | none => 0

def keccak256_segments (segments : List Bytes) : SailM hash := do
  let state ← get
  let input := segments.flatMap (materializeBytes state)
  pure <| match callNative 0 input 0 0 0 with
    | some output => nativeDecVectorFromBE 32 output
    | none => default

def sha256_segments (segments : List Bytes) : SailM hash := do
  let state ← get
  let input := segments.flatMap (materializeBytes state)
  pure <| match callNative 1 input 0 0 0 with
    | some output => nativeDecVectorFromBE 32 output
    | none => default

def accelerator_ripemd160 (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 2 slice 0 0 0

def accelerator_modexp
    (slice : ByteSlice) (base exponent modulus : Nat) : SailM Bool :=
  acceleratorOutput 3 slice base exponent modulus

def accelerator_bn254_add (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 4 slice 0 0 0

def accelerator_bn254_mul (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 5 slice 0 0 0

def accelerator_bn254_pairing (slice : ByteSlice) : SailM (BitVec 2) :=
  acceleratorPairing 6 slice

def accelerator_blake2f
  (slice : ByteSlice) (rounds : blake2_rounds)
    (finalBlock : y_parity) : SailM Bool :=
  acceleratorOutput 7 slice rounds finalBlock 0

def accelerator_kzg_point_evaluation (slice : ByteSlice) : SailM Bool :=
  acceleratorCheck 8 slice

def accelerator_bls_g1_add (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 9 slice 0 0 0

def accelerator_bls_g1_msm (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 10 slice 0 0 0

def accelerator_bls_g2_add (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 11 slice 0 0 0

def accelerator_bls_g2_msm (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 12 slice 0 0 0

def accelerator_bls_pairing (slice : ByteSlice) : SailM (BitVec 2) :=
  acceleratorPairing 13 slice

def accelerator_bls_map_fp_to_g1 (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 14 slice 0 0 0

def accelerator_bls_map_fp2_to_g2 (slice : ByteSlice) : SailM Bool :=
  acceleratorOutput 15 slice 0 0 0

def accelerator_p256_verify (slice : ByteSlice) : SailM Bool :=
  acceleratorCheck 16 slice

def secp256k1_verify
    (message : hash) (r s x y : word) : SailM Bool :=
  pure <| match callNative 17
      (hashBytes message ++ wordBytes r ++ wordBytes s ++
        wordBytes x ++ wordBytes y) 0 0 0 with
    | some output => output.data.getD 0 0 != 0
    | none => false

def host_ecrecover
    (message : hash) (parity : y_parity) (r s : word) :
    SailM AddressResult :=
  pure <| match callNative 18
      (hashBytes message ++ wordBytes r ++ wordBytes s) parity 0 0 with
    | some output =>
        { success := true, address := nativeDecVectorFromBE 20 output }
    | none => default

def output_buffer_store (slice : ByteSlice) : SailM Bool := do
  let bytes := materializeSlice (← get) slice
  modify fun state => { state with outputBytes := bytes.toArray }
  pure true

def output_buffer_store_word (value : word) : SailM Bool := do
  modify fun state => { state with outputBytes := wordBytes value |>.toArray }
  pure true

def output_buffer_store_words (first second : word) : SailM Bool := do
  modify fun state =>
    { state with outputBytes := (wordBytes first ++ wordBytes second).toArray }
  pure true

def public_output_write (slice : ByteSlice) : SailM Bool := do
  let bytes := materializeSlice (← get) slice
  modify fun state => { state with publicOutput := bytes.toArray }
  pure true

private def assocGet [BEq κ] (entries : List (κ × ν)) (key : κ) :
    Option ν :=
  (entries.find? fun entry => entry.1 == key).map (·.2)

private def assocPut [BEq κ]
    (entries : List (κ × ν)) (key : κ) (newValue : ν) :
    List (κ × ν) :=
  (key, newValue) :: entries.filter fun entry => entry.1 != key

private def codeSliceLength (slice : CodeSlice) : Nat :=
  let ⟨_, ⟨length, _⟩⟩ := slice
  length

def code_db_lookup (key : hash) : SailM (Option Code) := do
  pure (assocGet (← get).codeDb key)

def jumpdest_table_alloc (length : code_length) : SailM JumpdestRef := do
  let state ← get
  let reference := BitVec.ofNat 64 (state.jumpdestTables.length + 1)
  set { state with
    jumpdestTables :=
      { reference := reference, codeLength := length, positions := [] } ::
        state.jumpdestTables }
  pure reference

def jumpdest_table_mark
    (reference : JumpdestRef) (length : code_length)
    (position : code_pointer) : SailM Bool := do
  let state ← get
  match state.jumpdestTables.find? (·.reference == reference) with
  | none => pure false
  | some table =>
      if table.codeLength != length ∨ position >= length then
        pure false
      else
        let table :=
          { table with
            positions := position :: table.positions.filter (· != position) }
        set { state with
          jumpdestTables :=
            table :: state.jumpdestTables.filter (·.reference != reference) }
        pure true

private def storedCodeSlice
    (offset length : Nat) : CodeSlice :=
  ⟨offset, ⟨length,
    { source := .CodeSource, off := offset, len := length }⟩⟩

def code_db_store (slice : CodeSlice) (jumpdests : JumpdestRef) : SailM hash := do
  let state ← get
  let bytes := materializeSlice state slice
  let offset := state.codeBytes.size
  let stored := storedCodeSlice offset bytes.length
  set { state with codeBytes := state.codeBytes ++ bytes.toArray }
  let key ← keccak256_segments [.BytesSlice stored]
  modify fun state =>
    { state with
      codeDb := assocPut state.codeDb key { bytes := stored, jumpdests := jumpdests } }
  pure key

def jumpdest_ref_contains
    (reference : JumpdestRef) (length : code_length) (pc : code_pointer) :
    SailM Bool := do
  let state ← get
  match state.jumpdestTables.find? (·.reference == reference) with
  | none => pure false
  | some table =>
      pure (table.codeLength == length ∧ pc < length ∧ table.positions.contains pc)

def code_intern_delegation
    (target : address) (jumpdests : JumpdestRef) : SailM hash := do
  let state ← get
  let bytes := [0xef, 0x01, 0x00] ++ (vectorBytes target).reverse
  let offset := state.codeBytes.size
  let stored := storedCodeSlice offset bytes.length
  set { state with codeBytes := state.codeBytes ++ bytes.toArray }
  let key ← keccak256_segments [.BytesSlice stored]
  modify fun state =>
    { state with
      codeDb := assocPut state.codeDb key { bytes := stored, jumpdests := jumpdests }
      delegations := assocPut state.delegations key target }
  pure key

def code_db_read_delegation (key : hash) : SailM AddressResult := do
  let state ← get
  match assocGet state.codeDb key with
  | none => pure { success := false, address := default }
  | some code =>
      let bytes := materializeSlice state code.bytes
      if bytes.length == 23 &&
          bytes.getD 0 0 == 0xef &&
          bytes.getD 1 0 == 0x01 &&
          bytes.getD 2 0 == 0x00 then
        pure
          { success := true
            address := Vector.ofFn fun index => bytes.getD (22 - index.val) 0 }
      else
        pure { success := false, address := default }

def transient_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with transient := [] }

def transient_store (account : address) (slot value : word) : SailM Unit :=
  modify fun state =>
    { state with transient := assocPut state.transient { addr := account, slot := slot } value }

def transient_load (account : address) (slot : word) : SailM word := do
  pure <| (assocGet (← get).transient { addr := account, slot := slot }).getD default

def state_checkpoint_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with checkpoints := [] }

def state_checkpoint (_ : Unit) : SailM journal_checkpoint := do
  let state ← get
  let checkpoint := state.checkpoints.length
  let snapshot :=
    { transient := state.transient
      storageTx := state.storageTx
      accountTx := state.accountTx
      warmAddresses := state.warmAddresses
      warmSlots := state.warmSlots
      logs := state.logs }
  set { state with checkpoints := state.checkpoints ++ [snapshot] }
  pure checkpoint

def state_revert (checkpoint : journal_checkpoint) : SailM Unit := do
  let state ← get
  match state.checkpoints[checkpoint]? with
  | none => pure ()
  | some snapshot =>
      set { state with
        transient := snapshot.transient
        storageTx := snapshot.storageTx
        accountTx := snapshot.accountTx
        warmAddresses := snapshot.warmAddresses
        warmSlots := snapshot.warmSlots
        logs := snapshot.logs
        checkpoints := state.checkpoints.take checkpoint }

def storage_tx_update (entry : StorageEntry) : SailM Unit :=
  modify fun state =>
    { state with storageTx := assocPut state.storageTx entry.key entry.value }

def storage_tx_get (key : StorageKey) : SailM (Option StorageValue) := do
  pure (assocGet (← get).storageTx key)

def storage_tx_pop (_ : Unit) : SailM (Option StorageEntry) := do
  let state ← get
  match state.storageTx with
  | [] => pure none
  | (key, value) :: rest =>
      set { state with storageTx := rest }
      pure (some { key := key, value := value })

def storage_tx_clear (account : address) : SailM Unit :=
  modify fun state =>
    { state with storageTx := state.storageTx.filter (·.1.addr != account) }

def storage_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with storageTx := [] }

def storage_has_writes (account : address) : SailM Bool := do
  let state ← get
  pure <| (state.storageTx ++ state.storageBlock).any fun entry =>
    entry.1.addr == account && entry.2.curr != default

def storage_block_get (key : StorageKey) : SailM (Option StorageValue) := do
  pure (assocGet (← get).storageBlock key)

def storage_block_put (entry : StorageEntry) : SailM Unit :=
  modify fun state =>
    let value :=
      match assocGet state.storageBlock entry.key with
      | none => entry.value
      | some prior => { entry.value with orig := prior.orig }
    { state with storageBlock := assocPut state.storageBlock entry.key value }

def storage_block_cache (key : StorageKey) (value : word) : SailM Unit :=
  modify fun state =>
    if (assocGet state.storageBlock key).isSome then state
    else
      { state with
        storageBlock :=
          assocPut state.storageBlock key { curr := value, orig := value } }

def storage_block_clear (account : address) : SailM Unit :=
  modify fun state =>
    { state with storageBlock := state.storageBlock.filter (·.1.addr != account) }

def storage_block_iter_begin (account : address) : SailM Unit :=
  modify fun state =>
    { state with
      storageIterator :=
        ((state.storageBlock.filter (·.1.addr == account)).map fun entry =>
          { key := entry.1, value := entry.2 }).mergeSort fun left right =>
            storageSecureSortKey left.key.slot ≤ storageSecureSortKey right.key.slot }

def storage_block_iter_next (_ : address) : SailM (Option StorageEntry) := do
  let state ← get
  match state.storageIterator with
  | [] => pure none
  | entry :: rest =>
      set { state with storageIterator := rest }
      pure (some entry)

private def accountTxValue
    (state : HostState) (account : address) : AcctValue :=
  match assocGet state.accountTx account with
  | some value => value
  | none =>
      let base := (assocGet state.accountBlock account).map (·.curr) |>.getD default
      { curr := base, orig := base }

def acct_tx_get (account : address) : SailM (Option Account) := do
  pure ((assocGet (← get).accountTx account).map (·.curr))

def acct_tx_update (account : address) (value : Account) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    { state with
      accountTx := assocPut state.accountTx account { prior with curr := value } }

def acct_tx_set_balance (account : address) (value : word) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current := { prior.curr with info := { prior.curr.info with balance := value } }
    { state with
      accountTx := assocPut state.accountTx account { prior with curr := current } }

def acct_tx_set_nonce
    (account : address) (value : account_nonce) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current := { prior.curr with info := { prior.curr.info with nonce := value } }
    { state with
      accountTx := assocPut state.accountTx account { prior with curr := current } }

def acct_tx_set_code_hash (account : address) (value : hash) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current := { prior.curr with info := { prior.curr.info with code_hash := value } }
    { state with
      accountTx := assocPut state.accountTx account { prior with curr := current } }

def acct_tx_pop_ascending (_ : Unit) : SailM (Option AcctEntry) := do
  let state ← get
  match state.accountTx.mergeSort fun left right =>
      addressSortKey left.1 ≤ addressSortKey right.1 with
  | [] => pure none
  | (account, value) :: rest =>
      set { state with accountTx := rest }
      pure (some { addr := account, value := value })

def acct_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with accountTx := [] }

def acct_block_get (account : address) : SailM (Option Account) := do
  pure ((assocGet (← get).accountBlock account).map (·.curr))

def acct_block_write (entry : AcctEntry) : SailM Unit :=
  modify fun state =>
    let value :=
      match assocGet state.accountBlock entry.addr with
      | none => entry.value
      | some prior => { entry.value with orig := prior.orig }
    { state with accountBlock := assocPut state.accountBlock entry.addr value }

def acct_block_cache (account : address) (value : Account) : SailM Unit :=
  modify fun state =>
    if (assocGet state.accountBlock account).isSome then state
    else
      { state with
        accountBlock :=
          assocPut state.accountBlock account { curr := value, orig := value } }

def acct_block_iter_begin (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with
      accountIterator :=
        (state.accountBlock.map fun entry =>
          { addr := entry.1, value := entry.2 }).mergeSort fun left right =>
            accountSecureSortKey left.addr ≤ accountSecureSortKey right.addr }

def acct_block_iter_next (_ : Unit) : SailM (Option AcctEntry) := do
  let state ← get
  match state.accountIterator with
  | [] => pure none
  | entry :: rest =>
      set { state with accountIterator := rest }
      pure (some entry)

def acct_post_storage_root_store
    (account : address) (root : hash) : SailM Unit :=
  modify fun state =>
    { state with postStorageRoots := assocPut state.postStorageRoots account root }

def acct_post_storage_root_read (account : address) : SailM hash := do
  pure ((assocGet (← get).postStorageRoots account).getD default)

private def touchBalAccount (accounts : List address) (account : address) :
    List address :=
  if accounts.contains account then accounts else accounts ++ [account]

private def putBalStorageChange
    (entries : List BalStorageChangeRecord)
    (replacement : BalStorageChangeRecord) :
    List BalStorageChangeRecord :=
  (entries.filter fun entry =>
      !(entry.account == replacement.account &&
        entry.slot == replacement.slot &&
        entry.index == replacement.index)) ++ [replacement]

private def putBalStorageRead
    (entries : List BalStorageReadRecord)
    (replacement : BalStorageReadRecord) :
    List BalStorageReadRecord :=
  if entries.any fun entry =>
      entry.account == replacement.account &&
      entry.slot == replacement.slot then
    entries
  else
    entries ++ [replacement]

private def putBalBalanceChange
    (entries : List BalBalanceChangeRecord)
    (replacement : BalBalanceChangeRecord) :
    List BalBalanceChangeRecord :=
  (entries.filter fun entry =>
      !(entry.account == replacement.account &&
        entry.index == replacement.index)) ++ [replacement]

private def putBalNonceChange
    (entries : List BalNonceChangeRecord)
    (replacement : BalNonceChangeRecord) :
    List BalNonceChangeRecord :=
  (entries.filter fun entry =>
      !(entry.account == replacement.account &&
        entry.index == replacement.index)) ++ [replacement]

private def putBalCodeChange
    (entries : List BalCodeChangeRecord)
    (replacement : BalCodeChangeRecord) :
    List BalCodeChangeRecord :=
  (entries.filter fun entry =>
      !(entry.account == replacement.account &&
        entry.index == replacement.index)) ++ [replacement]

def bal_reset (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := []
      balStorageChanges := []
      balStorageReads := []
      balBalanceChanges := []
      balNonceChanges := []
      balCodeChanges := []
      balIterator := [] }

def bal_account_touch (account : address) : SailM Unit :=
  modify fun state =>
    { state with balAccounts := touchBalAccount state.balAccounts account }

def bal_storage_change
    (index : block_access_index) (account : address) (slot value : word) :
    SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := touchBalAccount state.balAccounts account
      balStorageChanges :=
        putBalStorageChange state.balStorageChanges
          { account := account, slot := slot, index := index, value := value } }

def bal_storage_read (account : address) (slot : word) : SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := touchBalAccount state.balAccounts account
      balStorageReads :=
        putBalStorageRead state.balStorageReads
          { account := account, slot := slot } }

def bal_balance_change
    (index : block_access_index) (account : address) (value : word) :
    SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := touchBalAccount state.balAccounts account
      balBalanceChanges :=
        putBalBalanceChange state.balBalanceChanges
          { account := account, index := index, value := value } }

def bal_nonce_change
    (index : block_access_index) (account : address) (value : account_nonce) :
    SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := touchBalAccount state.balAccounts account
      balNonceChanges :=
        putBalNonceChange state.balNonceChanges
          { account := account, index := index, value := value } }

def bal_code_change
    (index : block_access_index) (account : address) (value : hash) :
    SailM Unit :=
  modify fun state =>
    { state with
      balAccounts := touchBalAccount state.balAccounts account
      balCodeChanges :=
        putBalCodeChange state.balCodeChanges
          { account := account, index := index, value := value } }

private def balEventsForAccount
    (state : HostState) (account : address) : List BalIterEntry :=
  let changes :=
    state.balStorageChanges.filter (·.account == account)
  let storageChanges :=
    changes.map fun entry =>
      BalIterEntry.BalStorageChange
        { slot := entry.slot, index := entry.index, value := entry.value }
  let storageReads :=
    (state.balStorageReads.filter fun entry =>
      entry.account == account &&
      !(changes.any fun change => change.slot == entry.slot)).map fun entry =>
        BalIterEntry.BalStorageRead entry.slot
  let balances :=
    (state.balBalanceChanges.filter (·.account == account)).map fun entry =>
      BalIterEntry.BalBalanceChange { index := entry.index, value := entry.value }
  let nonces :=
    (state.balNonceChanges.filter (·.account == account)).map fun entry =>
      BalIterEntry.BalNonceChange { index := entry.index, value := entry.value }
  let codes :=
    (state.balCodeChanges.filter (·.account == account)).map fun entry =>
      BalIterEntry.BalCodeChange { index := entry.index, code_hash := entry.value }
  [BalIterEntry.BalAccount account] ++ storageChanges ++ storageReads ++
    balances ++ nonces ++ codes ++ [BalIterEntry.BalAccountEnd ()]

def bal_prepare_iter (_ : Unit) : SailM Unit :=
  modify fun state =>
    let prepared :=
      { state with
        balAccounts :=
          state.balAccounts.mergeSort fun left right =>
            addressSortKey left ≤ addressSortKey right
        balStorageChanges :=
          state.balStorageChanges.mergeSort fun left right =>
            if left.account == right.account then
              if left.slot == right.slot then
                left.index ≤ right.index
              else
                left.slot ≤ right.slot
            else
              addressSortKey left.account ≤ addressSortKey right.account
        balStorageReads :=
          state.balStorageReads.mergeSort fun left right =>
            if left.account == right.account then
              left.slot ≤ right.slot
            else
              addressSortKey left.account ≤ addressSortKey right.account
        balBalanceChanges :=
          state.balBalanceChanges.mergeSort fun left right =>
            if left.account == right.account then left.index ≤ right.index
            else addressSortKey left.account ≤ addressSortKey right.account
        balNonceChanges :=
          state.balNonceChanges.mergeSort fun left right =>
            if left.account == right.account then left.index ≤ right.index
            else addressSortKey left.account ≤ addressSortKey right.account
        balCodeChanges :=
          state.balCodeChanges.mergeSort fun left right =>
            if left.account == right.account then left.index ≤ right.index
            else addressSortKey left.account ≤ addressSortKey right.account }
    let events :=
      prepared.balAccounts.foldl
        (fun result account => result ++ balEventsForAccount prepared account)
        []
    { prepared with balIterator := events }

def bal_iter_next (_ : Unit) : SailM BalIterEntry := do
  let state ← get
  match state.balIterator with
  | [] => pure (BalIterEntry.BalEmpty ())
  | entry :: rest =>
      set { state with balIterator := rest }
      pure entry

def warm_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with warmAddresses := [], warmSlots := [] }

def warm_addr_touch (account : address) : SailM Bool := do
  let state ← get
  let warm := state.warmAddresses.contains account
  set { state with warmAddresses := touchBalAccount state.warmAddresses account }
  pure warm

def warm_slot_touch (account : address) (slot : word) : SailM Bool := do
  let state ← get
  let key : StorageKey := { addr := account, slot := slot }
  let warm := state.warmSlots.contains key
  set { state with
    warmSlots := if warm then state.warmSlots else state.warmSlots ++ [key] }
  pure warm

def logs_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with logs := [] }

def log_append (account : address) (topics : List word) (data : Bytes) :
    SailM Unit := do
  let state ← get
  let bytes := materializeBytes state data
  let offset := state.logBytes.size
  let slice := makeByteSlice .LogDataSource offset bytes.length
  let entry : LogEntry :=
    { address := account, topics := topics, data := slice }
  set { state with
    logBytes := state.logBytes ++ bytes.toArray
    logs := state.logs ++ [entry] }

def read_logs (_ : Unit) : SailM (List LogEntry) := do
  pure (← get).logs

def nodedb_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with nodeDb := [] }

def nodedb_insert
    (key : hash) (offset : source_pointer) (length : host_access) :
    SailM Unit :=
  modify fun state =>
    let slice := makeByteSlice .StatelessInputSource offset length
    { state with nodeDb := assocPut state.nodeDb key slice }

def nodedb_lookup (key : hash) : SailM ByteSlice := do
  pure ((assocGet (← get).nodeDb key).getD default)

end Evm.Functions
