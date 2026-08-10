import Evm.Defs

open Sail
open ConcurrencyInterfaceV1
open Evm.Defs

/-!
## Extensional world-state contract

The declarations in `Evm.Functions` below are the raw symbols imported by the
generated Sail model.  This namespace gives those account, persistent-storage,
transient-storage, warm-set, journal, log-store and authorization symbols an
implementation-independent meaning.

Nothing here describes the layout of either C backend's `state_db.c` or
`kernel_state.c`:
there are no table rows, generations-as-cursors, undo cursors, physical
deletions, or first-write/copy-on-write rules.  Such devices are permitted only
through a refinement from a backend state to the total maps, epoch stamps and
ordered journal below.
-/
namespace Evm.Contracts

/-- A retained log record's semantic content.  `LogEntry` is no longer a
generated type: the host builds a record field by field (`log_begin`,
`log_add_topic`, `log_add_data_memory` / `log_add_data_word`) and then exposes
it by index (`log_addr`, `log_topic_count`, `log_topic`, `log_data_off`,
`log_data_len`).  The contract keeps the assembled record as one value so the
builder's obligation can be stated without arena offsets. -/
structure LogRecord where
  address : address
  topics : List word
  data : List byte
  deriving BEq, Inhabited

/-- EIP-2929 warmth is an epoch stamp, not a boolean, and EIP-7928 positions
share the same counter. -/
abbrev AccessIndex := block_access_index

/-- The storage incarnation counter behind account-wide storage clears. -/
abbrev Generation := storage_generation

def zeroWord : word := 0

structure PersistentWorld where
  accountAt : address → Account
  storageAt : address → word → word

def accountWithBalance (value : word) (account : Account) : Account :=
  { account with info := { account.info with balance := value } }

def accountWithNonce (value : account_nonce) (account : Account) : Account :=
  { account with info := { account.info with nonce := value } }

def accountWithCodeHash (value : hash) (account : Account) : Account :=
  { account with info := { account.info with code_hash := value } }

def accountWithPresent (value : Bool) (account : Account) : Account :=
  { account with present := value }

def accountWithCreated (value : Bool) (account : Account) : Account :=
  { account with created := value }

def accountWithSelfdestructed (value : Bool) (account : Account) : Account :=
  { account with selfdestructed := value }

def accountWithClearedStorage (account : Account) : Account :=
  { account with storage_cleared := true }

def accountWithoutTransactionFlags (account : Account) : Account :=
  { account with created := false, selfdestructed := false }

def zeroAccountNonce : account_nonce :=
  0

/-- The end-of-transaction deletion form.  `preserveBalance` is the fork
policy's decision, applied here extensionally. -/
def deletedAccount
    (preserveBalance : Bool) (emptyCodeHash : hash) (account : Account) :
    Account :=
  { account with
    info :=
      { account.info with
        nonce := zeroAccountNonce
        balance := if preserveBalance then account.info.balance else zeroWord
        code_hash := emptyCodeHash }
    present := false
    storage_cleared := true }

def worldWriteAccount
    (world : PersistentWorld) (target : address) (value : Account) :
    PersistentWorld :=
  { accountAt := fun candidate =>
      if candidate == target then value else world.accountAt candidate
    storageAt := world.storageAt }

def worldUpdateAccount
    (world : PersistentWorld) (target : address) (update : Account → Account) :
    PersistentWorld :=
  worldWriteAccount world target (update (world.accountAt target))

def worldWriteStorage
    (world : PersistentWorld) (target : address) (targetSlot value : word) :
    PersistentWorld :=
  { accountAt := world.accountAt
    storageAt := fun candidate candidateSlot =>
      if candidate == target && candidateSlot == targetSlot then
        value
      else
        world.storageAt candidate candidateSlot }

abbrev TransientState := address → word → word
/-- Warmth is the last epoch at which a location was stamped. -/
abbrev WarmAddressState := address → AccessIndex
abbrev WarmSlotState := address → word → AccessIndex
/-- A storage row is visible only while its incarnation matches its account's. -/
abbrev AccountGenerationState := address → Generation
abbrev RowGenerationState := address → word → Generation

def emptyTransientState : TransientState :=
  fun _ _ => zeroWord

def emptyWarmAddressState : WarmAddressState := fun _ => 0
def emptyWarmSlotState : WarmSlotState := fun _ _ => 0
def emptyAccountGenerationState : AccountGenerationState := fun _ => 0
def emptyRowGenerationState : RowGenerationState := fun _ _ => 0

def transientWrite
    (transient : TransientState) (target : address) (targetSlot value : word) :
    TransientState :=
  fun candidate candidateSlot =>
    if candidate == target && candidateSlot == targetSlot then
      value
    else
      transient candidate candidateSlot

def warmAddressStamp
    (warm : WarmAddressState) (target : address) (epoch : AccessIndex) :
    WarmAddressState :=
  fun candidate => if candidate == target then epoch else warm candidate

def warmSlotStamp
    (warm : WarmSlotState) (target : address) (targetSlot : word)
    (epoch : AccessIndex) : WarmSlotState :=
  fun candidate candidateSlot =>
    if candidate == target && candidateSlot == targetSlot then epoch
    else warm candidate candidateSlot

def accountGenerationWrite
    (generations : AccountGenerationState) (target : address)
    (value : Generation) : AccountGenerationState :=
  fun candidate =>
    if candidate == target then value else generations candidate

def rowGenerationWrite
    (generations : RowGenerationState) (target : address)
    (targetSlot : word) (value : Generation) : RowGenerationState :=
  fun candidate candidateSlot =>
    if candidate == target && candidateSlot == targetSlot then value
    else generations candidate candidateSlot

def storageListRemoveLast
    (lists : address → List word) (target : address) : address → List word :=
  fun candidate =>
    if candidate == target then (lists candidate).dropLast else lists candidate

/- `blockStart` is the authenticated parent state and `blockCurrent`
accumulates successful transactions.  `txStart` is the EIP-2200 original-value
snapshot; `txCurrent` is the live state of the executing transaction.  The
worklists record the transaction overlay's insertion order, and `logTxStart`
opens the current transaction's window into the block-lifetime log store. -/
structure ReferenceWorldState where
  blockStart : PersistentWorld
  blockCurrent : PersistentWorld
  txStart : PersistentWorld
  txCurrent : PersistentWorld
  transient : TransientState
  warmEpoch : AccessIndex
  warmAddresses : WarmAddressState
  warmSlots : WarmSlotState
  accountGenerations : AccountGenerationState
  rowGenerations : RowGenerationState
  txAccounts : List address
  txStorage : address → List word
  logs : List LogRecord
  logTxStart : Nat

/-- The semantic content of one open frame: exactly the frame-revertible
sub-state.  Block layers, the warm epoch counter and the log-window start are
transaction-scoped and therefore deliberately absent. -/
structure TransactionSnapshot where
  txCurrent : PersistentWorld
  transient : TransientState
  warmAddresses : WarmAddressState
  warmSlots : WarmSlotState
  accountGenerations : AccountGenerationState
  rowGenerations : RowGenerationState
  txAccounts : List address
  txStorage : address → List word
  logs : List LogRecord

def persistentWorldEquivalent (left right : PersistentWorld) : Prop :=
  (∀ account, left.accountAt account = right.accountAt account) ∧
  (∀ account slot, left.storageAt account slot = right.storageAt account slot)

def transientStateEquivalent (left right : TransientState) : Prop :=
  ∀ account slot, left account slot = right account slot

def warmAddressStateEquivalent (left right : WarmAddressState) : Prop :=
  ∀ account, left account = right account

def warmSlotStateEquivalent (left right : WarmSlotState) : Prop :=
  ∀ account slot, left account slot = right account slot

def accountGenerationStateEquivalent
    (left right : AccountGenerationState) : Prop :=
  ∀ account, left account = right account

def rowGenerationStateEquivalent (left right : RowGenerationState) : Prop :=
  ∀ account slot, left account slot = right account slot

def storageListsEquivalent (left right : address → List word) : Prop :=
  ∀ account, left account = right account

def referenceWorldStateEquivalent
    (left right : ReferenceWorldState) : Prop :=
  persistentWorldEquivalent left.blockStart right.blockStart ∧
  persistentWorldEquivalent left.blockCurrent right.blockCurrent ∧
  persistentWorldEquivalent left.txStart right.txStart ∧
  persistentWorldEquivalent left.txCurrent right.txCurrent ∧
  transientStateEquivalent left.transient right.transient ∧
  left.warmEpoch = right.warmEpoch ∧
  warmAddressStateEquivalent left.warmAddresses right.warmAddresses ∧
  warmSlotStateEquivalent left.warmSlots right.warmSlots ∧
  accountGenerationStateEquivalent left.accountGenerations
    right.accountGenerations ∧
  rowGenerationStateEquivalent left.rowGenerations right.rowGenerations ∧
  left.txAccounts = right.txAccounts ∧
  storageListsEquivalent left.txStorage right.txStorage ∧
  left.logs = right.logs ∧
  left.logTxStart = right.logTxStart

def transactionSnapshotEquivalent
    (left right : TransactionSnapshot) : Prop :=
  persistentWorldEquivalent left.txCurrent right.txCurrent ∧
  transientStateEquivalent left.transient right.transient ∧
  warmAddressStateEquivalent left.warmAddresses right.warmAddresses ∧
  warmSlotStateEquivalent left.warmSlots right.warmSlots ∧
  accountGenerationStateEquivalent left.accountGenerations
    right.accountGenerations ∧
  rowGenerationStateEquivalent left.rowGenerations right.rowGenerations ∧
  left.txAccounts = right.txAccounts ∧
  storageListsEquivalent left.txStorage right.txStorage ∧
  left.logs = right.logs

def referenceCheckpoint (state : ReferenceWorldState) : TransactionSnapshot :=
  { txCurrent := state.txCurrent
    transient := state.transient
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    accountGenerations := state.accountGenerations
    rowGenerations := state.rowGenerations
    txAccounts := state.txAccounts
    txStorage := state.txStorage
    logs := state.logs }

def referenceRevert
    (state : ReferenceWorldState) (snapshot : TransactionSnapshot) :
    ReferenceWorldState :=
  { state with
    txCurrent := snapshot.txCurrent
    transient := snapshot.transient
    warmAddresses := snapshot.warmAddresses
    warmSlots := snapshot.warmSlots
    accountGenerations := snapshot.accountGenerations
    rowGenerations := snapshot.rowGenerations
    txAccounts := snapshot.txAccounts
    txStorage := snapshot.txStorage
    logs := snapshot.logs }

/-!
## The state journal: a closed operation algebra

`sail/host/journal.sail` gives every frame-revertible mutation exactly the
semantic field needed to restore its predecessor, and encodes the frame nesting
in the same ordered stream.  The four externs are all `unit -> unit`: no handle
crosses the boundary.  The generated `StateJournalEntry` union is reused
verbatim here, so a variant cannot drift away from the contract.  The list is
newest-first.
-/

abbrev StateJournal := List StateJournalEntry

def journalEntryIsMarker (entry : StateJournalEntry) : Prop :=
  entry = StateJournalEntry.JournalFrameCheckpointed () ∨
  entry = StateJournalEntry.JournalFrameCommitted ()

def journalEntrySemantic (entry : StateJournalEntry) : Prop :=
  ¬ journalEntryIsMarker entry

/-- Applies the inverse of one journal entry.  Structural markers have no
semantic inverse.  There is deliberately no storage-root variant: transaction
writes cannot change an account's witness anchor, and the algebra's closedness
makes that a boundary obligation rather than a convention. -/
def applyInverse
    (entry : StateJournalEntry) (state : ReferenceWorldState) :
    ReferenceWorldState :=
  match entry with
  | .JournalTransientChanged change =>
      { state with
        transient :=
          transientWrite state.transient change.address change.slot
            change.prior }
  | .JournalWarmAccountChanged change =>
      { state with
        warmAddresses :=
          warmAddressStamp state.warmAddresses change.address
            change.prior_epoch }
  | .JournalWarmStorageChanged change =>
      { state with
        warmSlots :=
          warmSlotStamp state.warmSlots change.key.addr change.key.slot
            change.prior_epoch }
  | .JournalAccountBalanceChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithBalance change.prior) }
  | .JournalAccountNonceChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithNonce change.prior) }
  | .JournalAccountCodeHashChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithCodeHash change.prior) }
  | .JournalAccountExistsChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithPresent change.prior) }
  | .JournalAccountCreatedChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithCreated change.prior) }
  | .JournalAccountSelfdestructedChanged change =>
      { state with
        txCurrent :=
          worldUpdateAccount state.txCurrent change.address
            (accountWithSelfdestructed change.prior) }
  | .JournalTransactionAccountListed () =>
      { state with txAccounts := state.txAccounts.dropLast }
  | .JournalTransactionStorageListed account =>
      { state with txStorage := storageListRemoveLast state.txStorage account }
  | .JournalLogAppended () =>
      { state with logs := state.logs.dropLast }
  | .JournalStorageValueChanged change =>
      { state with
        txCurrent :=
          worldWriteStorage state.txCurrent change.key.addr change.key.slot
            change.prior }
  | .JournalStorageRowGenerationChanged change =>
      { state with
        rowGenerations :=
          rowGenerationWrite state.rowGenerations change.key.addr
            change.key.slot change.prior }
  | .JournalAccountStorageGenerationChanged change =>
      { state with
        accountGenerations :=
          accountGenerationWrite state.accountGenerations change.address
            change.prior }
  | .JournalFrameCheckpointed () => state
  | .JournalFrameCommitted () => state

/-- The backwards frame-marker scan: walking the journal newest-first, a
`JournalFrameCommitted` marker closes one earlier checkpoint and the innermost
open `JournalFrameCheckpointed` marker splits the stream.  The suffix
(everything newer than that marker, committed child frames included) is what
revert replays; the base is what remains once the marker is consumed. -/
def splitAtOpenCheckpointFrom :
    StateJournal → Nat → Option (List StateJournalEntry × StateJournal)
  | [], _ => none
  | StateJournalEntry.JournalFrameCheckpointed () :: rest, 0 => some ([], rest)
  | StateJournalEntry.JournalFrameCheckpointed () :: rest, closed + 1 =>
      match splitAtOpenCheckpointFrom rest closed with
      | none => none
      | some (suffix, base) =>
          some (StateJournalEntry.JournalFrameCheckpointed () :: suffix, base)
  | StateJournalEntry.JournalFrameCommitted () :: rest, closed =>
      match splitAtOpenCheckpointFrom rest (closed + 1) with
      | none => none
      | some (suffix, base) =>
          some (StateJournalEntry.JournalFrameCommitted () :: suffix, base)
  | entry :: rest, closed =>
      match splitAtOpenCheckpointFrom rest closed with
      | none => none
      | some (suffix, base) => some (entry :: suffix, base)

def splitAtOpenCheckpoint (journal : StateJournal) :
    Option (List StateJournalEntry × StateJournal) :=
  splitAtOpenCheckpointFrom journal 0

def journalReplay
    (suffix : List StateJournalEntry) (state : ReferenceWorldState) :
    ReferenceWorldState :=
  suffix.foldl (fun current entry => applyInverse entry current) state

/-- `state_journal_checkpoint`: appends the frame marker. -/
def journalCheckpoint (journal : StateJournal) : StateJournal :=
  StateJournalEntry.JournalFrameCheckpointed () :: journal

/-- `state_journal_commit`: appends a `JournalFrameCommitted` for the innermost
open checkpoint.  The committed frame's mutations stay in the journal and are
therefore still revertible by a parent.  Committing with no open checkpoint is
a host failure. -/
def journalCommit (journal : StateJournal) : Option StateJournal :=
  match splitAtOpenCheckpoint journal with
  | none => none
  | some _ => some (StateJournalEntry.JournalFrameCommitted () :: journal)

/-- `state_journal_revert`: replays entries backwards to the innermost open
checkpoint, then removes its marker. -/
def referenceJournalRevert
    (state : ReferenceWorldState) (journal : StateJournal) :
    Option (ReferenceWorldState × StateJournal) :=
  match splitAtOpenCheckpoint journal with
  | none => none
  | some (suffix, base) => some (journalReplay suffix state, base)

/-! ## Reference lifecycle and reads -/

def referenceStartBlock (initial : PersistentWorld) : ReferenceWorldState :=
  { blockStart := initial
    blockCurrent := initial
    txStart := initial
    txCurrent := initial
    transient := emptyTransientState
    warmEpoch := 1
    warmAddresses := emptyWarmAddressState
    warmSlots := emptyWarmSlotState
    accountGenerations := emptyAccountGenerationState
    rowGenerations := emptyRowGenerationState
    txAccounts := []
    txStorage := fun _ => []
    logs := []
    logTxStart := 0 }

/-- The `k_tx_reset` bundle: transient storage, the worklists, the generations,
the log window and the journal reset together.  `warm_reset` installs the
transaction's block-access epoch, so the new current epoch is `epoch + 1` and
nothing stamped by an earlier transaction stays warm. -/
def referenceBeginTransaction
    (epoch : AccessIndex) (state : ReferenceWorldState) : ReferenceWorldState :=
  { blockStart := state.blockStart
    blockCurrent := state.blockCurrent
    txStart := state.blockCurrent
    txCurrent := state.blockCurrent
    transient := emptyTransientState
    warmEpoch := epoch + 1
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    accountGenerations := emptyAccountGenerationState
    rowGenerations := emptyRowGenerationState
    txAccounts := []
    txStorage := fun _ => []
    logs := state.logs
    logTxStart := state.logs.length }

def referenceReadAccount
    (state : ReferenceWorldState) (account : address) : Account :=
  state.txCurrent.accountAt account

/-- Storage reads are generation-gated: a row is visible only while its
incarnation matches its account's.  A stale row reads as zero — the semantic
content of an account-wide storage clear. -/
def referenceStorageVisible
    (state : ReferenceWorldState) (account : address) (slot : word) : Bool :=
  state.rowGenerations account slot == state.accountGenerations account

def referenceReadStorageCurrent
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  if referenceStorageVisible state account slot then
    state.txCurrent.storageAt account slot
  else
    zeroWord

/- SSTORE's original value is fixed by transaction entry, not by the first
insertion into an implementation table. -/
def referenceReadStorageOriginal
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  state.txStart.storageAt account slot

/-- A transaction-layer lookup distinguishes an actual visible row from a slot
made known-zero by an account storage clear; the generated `StorageTxLookup`
union names the same three cases. -/
def referenceIsAccountWarm
    (state : ReferenceWorldState) (account : address) : Bool :=
  state.warmEpoch ≤ state.warmAddresses account

def referenceIsStorageWarm
    (state : ReferenceWorldState) (account : address) (slot : word) : Bool :=
  state.warmEpoch ≤ state.warmSlots account slot

def referenceReadTransient
    (state : ReferenceWorldState) (account : address) (slot : word) : word :=
  state.transient account slot

def referenceWriteAccount
    (state : ReferenceWorldState) (account : address) (value : Account) :
    ReferenceWorldState :=
  { state with txCurrent := worldWriteAccount state.txCurrent account value }

def referenceWriteStorage
    (state : ReferenceWorldState) (account : address) (slot value : word) :
    ReferenceWorldState :=
  { state with
    txCurrent := worldWriteStorage state.txCurrent account slot value }

/-- An account-wide storage clear bumps the account incarnation; every stale
row then reads as zero without being enumerated. -/
def referenceClearStorage
    (state : ReferenceWorldState) (account : address) : ReferenceWorldState :=
  { state with
    txCurrent :=
      worldUpdateAccount state.txCurrent account accountWithClearedStorage
    accountGenerations :=
      accountGenerationWrite state.accountGenerations account
        (state.accountGenerations account + 1) }

def referenceDeleteAccount
    (preserveBalance : Bool) (emptyCodeHash : hash)
    (state : ReferenceWorldState) (account : address) : ReferenceWorldState :=
  { state with
    txCurrent :=
      worldUpdateAccount state.txCurrent account
        (deletedAccount preserveBalance emptyCodeHash)
    accountGenerations :=
      accountGenerationWrite state.accountGenerations account
        (state.accountGenerations account + 1) }

def referenceWriteTransient
    (state : ReferenceWorldState) (account : address) (slot value : word) :
    ReferenceWorldState :=
  { state with transient := transientWrite state.transient account slot value }

def referenceMarkWarmAccount
    (state : ReferenceWorldState) (account : address) : ReferenceWorldState :=
  { state with
    warmAddresses :=
      warmAddressStamp state.warmAddresses account state.warmEpoch }

def referenceMarkWarmSlot
    (state : ReferenceWorldState) (account : address) (slot : word) :
    ReferenceWorldState :=
  { state with
    warmSlots := warmSlotStamp state.warmSlots account slot state.warmEpoch }

def referenceAppendLog
    (state : ReferenceWorldState) (record : LogRecord) : ReferenceWorldState :=
  { state with logs := state.logs ++ [record] }

def finalizedWorld
    (preserveBalance : Bool) (emptyCodeHash : hash)
    (deleteAtEnd : address → Bool) (world : PersistentWorld) :
    PersistentWorld :=
  { accountAt := fun account =>
      accountWithoutTransactionFlags
        (if deleteAtEnd account then
          deletedAccount preserveBalance emptyCodeHash (world.accountAt account)
        else
          world.accountAt account)
    storageAt := fun account slot =>
      if deleteAtEnd account then zeroWord
      else world.storageAt account slot }

/-- Committing materializes the generation-gated storage view into plain maps
before the fork policy's deletions are applied. -/
def materializedTxWorld (state : ReferenceWorldState) : PersistentWorld :=
  { accountAt := state.txCurrent.accountAt
    storageAt := fun account slot =>
      referenceReadStorageCurrent state account slot }

/- `deleteAtEnd` and `preserveBalance` are computed by the pure Sail fork
policy.  The host transition only applies that semantic decision and clears
transaction-local flags and worklists. -/
def referenceCommitTransaction
    (preserveBalance : Bool) (emptyCodeHash : hash)
    (deleteAtEnd : address → Bool) (state : ReferenceWorldState) :
    ReferenceWorldState :=
  let committed :=
    finalizedWorld preserveBalance emptyCodeHash deleteAtEnd
      (materializedTxWorld state)
  { blockStart := state.blockStart
    blockCurrent := committed
    txStart := committed
    txCurrent := committed
    transient := state.transient
    warmEpoch := state.warmEpoch
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    accountGenerations := emptyAccountGenerationState
    rowGenerations := emptyRowGenerationState
    txAccounts := []
    txStorage := fun _ => []
    logs := state.logs
    logTxStart := state.logTxStart }

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

/-- Fixed vectors index byte zero at the leading (most significant) protocol
byte, matching the generated `word_to_hash` / `word_to_address` equations. -/
def addressBytes (bytes : address) : List byte :=
  bytes.toList

def bytesBEToNat (bytes : List byte) : Nat :=
  bytes.foldl (fun value byte => value * 256 + byte.toNat) 0

def b256ToNat (bytes : b256) : Nat :=
  bytesBEToNat bytes.toList

abbrev PureKeccak := List byte → hash

def accountSecureKey (keccak : PureKeccak) (account : address) : hash :=
  keccak (addressBytes account)

def storageSecureKey (keccak : PureKeccak) (slot : word) : hash :=
  keccak (fixedBEBytes 32 (wordToNat slot))

def worldDeltaSecurelyOrdered
    (keccak : PureKeccak) (delta : WorldDelta) : Prop :=
  strictlySortedBy (fun account => b256ToNat (accountSecureKey keccak account))
    delta.accounts ∧
  ∀ account,
    strictlySortedBy (fun slot => b256ToNat (storageSecureKey keccak slot))
      (delta.storage account)

/- The raw account/storage/transient/warm/journal/log externs collectively
implement this operation-level interface.  `journalDenotes` is a ghost relation
between a backend state and the ordered journal it denotes; no handle crosses
the boundary, so `checkpoint`, `commitFrame` and `revertFrame` are all plain
state steps.  Numeric token encodings, registries, row encodings, active
lengths, and drain cursors remain backend-private. -/
structure WorldStateContract where
  BackendState : Type
  observe : BackendState → ReferenceWorldState
  journalDenotes : BackendState → StateJournal → Prop
  startBlock : PersistentWorld → BackendState
  beginTransaction : AccessIndex → BackendState → BackendState
  readAccount : BackendState → address → Account
  readStorageCurrent : BackendState → address → word → word
  readStorageOriginal : BackendState → address → word → word
  writeAccount : BackendState → address → Account → BackendState
  writeStorage : BackendState → address → word → word → BackendState
  clearStorage : BackendState → address → BackendState
  deleteAccount : Bool → hash → BackendState → address → BackendState
  checkpoint : BackendState → BackendState
  commitFrame : BackendState → Option BackendState
  revertFrame : BackendState → Option BackendState
  commitTransaction :
    Bool → hash → (address → Bool) → BackendState → BackendState
  cacheAccount : BackendState → address → hash → Account → BackendState
  cacheStorage : BackendState → address → word → hash → word → BackendState
  readTransient : BackendState → address → word → word
  writeTransient : BackendState → address → word → word → BackendState
  accountIsWarm : BackendState → address → Bool
  storageIsWarm : BackendState → address → word → Bool
  markWarmAccount : BackendState → address → BackendState
  markWarmStorage : BackendState → address → word → BackendState
  appendLog : BackendState → LogRecord → BackendState
  readLogs : BackendState → List LogRecord
  /-- `log_begin` / `log_add_topic` / `log_add_data_*` assemble exactly one
  `LogRecord`, and the indexed readers (`log_addr`, `log_topic_count`,
  `log_topic`, `log_data_off`, `log_data_len`) recover its fields. -/
  logRecordBuilder : Prop
  transactionDelta : BackendState → WorldDelta
  blockDelta : BackendState → WorldDelta

/-- The closed-algebra ghost law: a mutating step appends exactly the semantic
(marker-free) entries whose inverse replay restores the prior observation.
Observation-preserving steps append nothing. -/
def journalStepRecorded
    (contract : WorldStateContract)
    (before after : contract.BackendState) : Prop :=
  ∀ journal,
    contract.journalDenotes before journal →
    ∃ appended,
      (∀ entry ∈ appended, journalEntrySemantic entry) ∧
      contract.journalDenotes after (appended ++ journal) ∧
      referenceWorldStateEquivalent
        (journalReplay appended (contract.observe after))
        (contract.observe before)

/- The canonical semantic instantiation pairs the reference world with the
reference journal itself; a native backend may use any physical journal storage
related to it by `journalDenotes`.  Delta enumerators are supplied explicitly
because finite support cannot be computed constructively from arbitrary total
functions. -/
abbrev ReferenceWorldPair := ReferenceWorldState × StateJournal

def referenceWorldStateContract
    (transactionDelta blockDelta : ReferenceWorldState → WorldDelta)
    (logRecordBuilder : Prop) :
    WorldStateContract where
  BackendState := ReferenceWorldPair
  observe := Prod.fst
  journalDenotes := fun state journal => state.2 = journal
  startBlock := fun initial => (referenceStartBlock initial, [])
  beginTransaction := fun epoch state =>
    (referenceBeginTransaction epoch state.1, [])
  readAccount := fun state => referenceReadAccount state.1
  readStorageCurrent := fun state => referenceReadStorageCurrent state.1
  readStorageOriginal := fun state => referenceReadStorageOriginal state.1
  writeAccount := fun state account value =>
    (referenceWriteAccount state.1 account value,
     StateJournalEntry.JournalAccountBalanceChanged
       { address := account
         prior := (referenceReadAccount state.1 account).info.balance } ::
       state.2)
  writeStorage := fun state account slot value =>
    (referenceWriteStorage state.1 account slot value,
     StateJournalEntry.JournalStorageValueChanged
       { key := { addr := account, slot := slot }
         prior := referenceReadStorageCurrent state.1 account slot } :: state.2)
  clearStorage := fun state account =>
    (referenceClearStorage state.1 account,
     StateJournalEntry.JournalAccountStorageGenerationChanged
       { address := account, prior := state.1.accountGenerations account } ::
       state.2)
  deleteAccount := fun preserveBalance emptyCodeHash state account =>
    (referenceDeleteAccount preserveBalance emptyCodeHash state.1 account,
     StateJournalEntry.JournalAccountStorageGenerationChanged
       { address := account, prior := state.1.accountGenerations account } ::
       state.2)
  checkpoint := fun state => (state.1, journalCheckpoint state.2)
  commitFrame := fun state =>
    (journalCommit state.2).map fun journal => (state.1, journal)
  revertFrame := fun state =>
    (referenceJournalRevert state.1 state.2).map fun result =>
      (result.1, result.2)
  commitTransaction := fun preserveBalance emptyCodeHash deleteAtEnd state =>
    (referenceCommitTransaction preserveBalance emptyCodeHash deleteAtEnd
      state.1, [])
  cacheAccount := fun state _ _ _ => state
  cacheStorage := fun state _ _ _ _ => state
  readTransient := fun state => referenceReadTransient state.1
  writeTransient := fun state account slot value =>
    (referenceWriteTransient state.1 account slot value,
     StateJournalEntry.JournalTransientChanged
       { address := account
         slot := slot
         prior := referenceReadTransient state.1 account slot } :: state.2)
  accountIsWarm := fun state => referenceIsAccountWarm state.1
  storageIsWarm := fun state => referenceIsStorageWarm state.1
  markWarmAccount := fun state account =>
    (referenceMarkWarmAccount state.1 account,
     StateJournalEntry.JournalWarmAccountChanged
       { address := account
         prior_epoch := state.1.warmAddresses account } :: state.2)
  markWarmStorage := fun state account slot =>
    (referenceMarkWarmSlot state.1 account slot,
     StateJournalEntry.JournalWarmStorageChanged
       { key := { addr := account, slot := slot }
         prior_epoch := state.1.warmSlots account slot } :: state.2)
  appendLog := fun state record =>
    (referenceAppendLog state.1 record,
     StateJournalEntry.JournalLogAppended () :: state.2)
  readLogs := fun state => state.1.logs
  logRecordBuilder := logRecordBuilder
  transactionDelta := fun state => transactionDelta state.1
  blockDelta := fun state => blockDelta state.1

def worldStateBoundary
    (keccak : PureKeccak) (contract : WorldStateContract) : Prop :=
  (∀ initial,
    let state := contract.startBlock initial
    referenceWorldStateEquivalent (contract.observe state)
      (referenceStartBlock initial) ∧
    contract.journalDenotes state []) ∧
  (∀ epoch state,
    let next := contract.beginTransaction epoch state
    referenceWorldStateEquivalent (contract.observe next)
      (referenceBeginTransaction epoch (contract.observe state)) ∧
    contract.journalDenotes next []) ∧
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
    journalStepRecorded contract state next) ∧
  (∀ state account slot value,
    let next := contract.writeStorage state account slot value
    referenceWorldStateEquivalent (contract.observe next)
      (referenceWriteStorage (contract.observe state) account slot value) ∧
    journalStepRecorded contract state next) ∧
  (∀ state account,
    let next := contract.clearStorage state account
    (∀ slot, contract.readStorageCurrent next account slot = zeroWord) ∧
    journalStepRecorded contract state next) ∧
  (∀ preserveBalance emptyCodeHash state account,
    let next :=
      contract.deleteAccount preserveBalance emptyCodeHash state account
    (contract.readAccount next account).present = false ∧
    (∀ slot, contract.readStorageCurrent next account slot = zeroWord) ∧
    journalStepRecorded contract state next) ∧
  (∀ state journal,
    contract.journalDenotes state journal →
    let next := contract.checkpoint state
    referenceWorldStateEquivalent (contract.observe next)
      (contract.observe state) ∧
    contract.journalDenotes next (journalCheckpoint journal)) ∧
  (∀ state journal,
    contract.journalDenotes state journal →
    ∀ committed, journalCommit journal = some committed →
      ∃ next,
        contract.commitFrame state = some next ∧
        referenceWorldStateEquivalent (contract.observe next)
          (contract.observe state) ∧
        contract.journalDenotes next committed) ∧
  (∀ state journal,
    contract.journalDenotes state journal →
    ∀ reverted,
      referenceJournalRevert (contract.observe state) journal = some reverted →
        ∃ next,
          contract.revertFrame state = some next ∧
          referenceWorldStateEquivalent (contract.observe next) reverted.1 ∧
          contract.journalDenotes next reverted.2) ∧
  (∀ preserveBalance emptyCodeHash deleteAtEnd state,
    let next :=
      contract.commitTransaction preserveBalance emptyCodeHash deleteAtEnd state
    referenceWorldStateEquivalent (contract.observe next)
      (referenceCommitTransaction preserveBalance emptyCodeHash deleteAtEnd
        (contract.observe state)) ∧
    contract.journalDenotes next []) ∧
  (∀ state account digest,
    let next :=
      contract.cacheAccount state account digest
        (contract.readAccount state account)
    referenceWorldStateEquivalent (contract.observe next)
      (contract.observe state) ∧
    journalStepRecorded contract state next) ∧
  (∀ state account slot digest,
    let next := contract.cacheStorage state account slot digest
      (contract.readStorageCurrent state account slot)
    referenceWorldStateEquivalent (contract.observe next)
      (contract.observe state) ∧
    journalStepRecorded contract state next) ∧
  (∀ state account slot,
    contract.readTransient state account slot =
      referenceReadTransient (contract.observe state) account slot) ∧
  (∀ state account slot value,
    let next := contract.writeTransient state account slot value
    referenceWorldStateEquivalent (contract.observe next)
      (referenceWriteTransient (contract.observe state) account slot value) ∧
    journalStepRecorded contract state next) ∧
  (∀ state account,
    contract.accountIsWarm state account =
      referenceIsAccountWarm (contract.observe state) account) ∧
  (∀ state account slot,
    contract.storageIsWarm state account slot =
      referenceIsStorageWarm (contract.observe state) account slot) ∧
  (∀ state account,
    let next := contract.markWarmAccount state account
    contract.accountIsWarm next account = true ∧
    journalStepRecorded contract state next) ∧
  (∀ state account slot,
    let next := contract.markWarmStorage state account slot
    contract.storageIsWarm next account slot = true ∧
    journalStepRecorded contract state next) ∧
  (∀ state record,
    let next := contract.appendLog state record
    (contract.observe next).logs = (contract.observe state).logs ++ [record] ∧
    journalStepRecorded contract state next) ∧
  (∀ state, contract.readLogs state = (contract.observe state).logs) ∧
  contract.logRecordBuilder ∧
  (∀ state,
    worldDeltaDescribes
      (contract.observe state).txStart
      (materializedTxWorld (contract.observe state))
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

/-!
## EIP-7702 authorization tracker

`authorization_tracker_*` is transaction-local authority bookkeeping.  It sits
deliberately outside the journal's scope — no `StateJournalEntry` variant
records it, so tracker state is not frame-revertible.
-/
structure AuthorizationTrackerContract where
  TrackerState : Type
  seen : TrackerState → address → Bool
  originallyDelegated : TrackerState → address → Bool
  delegationSet : TrackerState → address → Bool
  commit : TrackerState → address → Bool → Bool → TrackerState
  reset : Nat → TrackerState
  resetClears :
    ∀ size account,
      seen (reset size) account = false ∧
      delegationSet (reset size) account = false
  commitMarksSeen :
    ∀ state account originallyDelegated delegationSet,
      seen (commit state account originallyDelegated delegationSet) account =
        true
  /-- The originally-delegated flag records the state before the authority's
  FIRST successful tuple; later commits must not revise it. -/
  firstCommitWins : Prop

/-!
## EIP-7928 block-access-list recorder

The canonical account-delimited event stream the recorder canonicalizes into.
Multiplicity matters: each `(index, value)` change is one event, so two
transactions changing the same slot yield two `BalStorageChange` events.
-/
structure BalRecorderContract where
  RecorderState : Type
  observe : RecorderState → List BalIterEntry
  streamAccountDelimited : Prop
  streamSecurelyOrdered : Prop
  readsSurviveRollback : Prop
  changesFollowRollback : Prop
  readShadowedByChange : Prop

/-!
## Typed region access and the scratch arena

Six nominal region families.  The nominal Sail type of a slice selects the
backing region; no runtime source tag crosses the boundary.  Every region
operation (`*_byte_at`, `*_load_word`, `*_load_n_word`, `*_copy_to_memory`,
`*_strided_zero`, `stateless_input_count_nonzero`, the three provenance-pair
equalities, and the per-region hash entry points) must refine the one
materialized byte sequence this contract denotes for its region.
-/
inductive Region where
  | StatelessInputRegion
  | EvmMemoryRegion
  | CodeRegion
  | ScratchRegion
  | LogDataRegion
  | OutputRegion

structure RegionAccessContract where
  RegionState : Type
  regionBytes : RegionState → Region → List byte
  byteAt : Prop
  loadWord : Prop
  loadNWord : Prop
  copyToMemory : Prop
  stridedZero : Prop
  countNonzero : Prop
  slicesEqual : Prop
  hashRefinement : Prop

/-- The Sail-cursor-owned executor scratch arena.  Composite hash preimages and
RLP payloads are written in order at an explicit offset and then cross the
boundary as one scratch-backed slice. -/
structure ScratchArenaContract where
  ScratchState : Type
  scratchBytes : ScratchState → List byte
  reserve : Prop
  store : Prop
  truncate : Prop

/-- `nodedb_*`: the node table plus the Sail-side authenticated parent-root
register is the whole lazy-witness model.  `nodedb_lookup` is total — the empty
span denotes an unwitnessed node. -/
structure WitnessDbContract where
  WitnessState : Type
  nodeLookup : WitnessState → hash → Nat × Nat
  witnessBytes : WitnessState → Nat × Nat → List byte
  resetClears : Prop
  insertSelect : Prop
  spanBounds : Prop
  authenticatedFrontier : Prop

/-- The complete host boundary the guest is verified against. -/
structure GuestExternContract where
  regions : RegionAccessContract
  scratch : ScratchArenaContract
  worldState : ValidWorldStateContract
  balRecorder : BalRecorderContract
  authorizations : AuthorizationTrackerContract
  witnessDb : WitnessDbContract

/-- Every named obligation of every sub-contract appears here exactly once, so
an obligation cannot silently drop out of the top-level boundary. -/
def guestBoundary (contract : GuestExternContract) : Prop :=
  contract.regions.byteAt ∧
  contract.regions.loadWord ∧
  contract.regions.loadNWord ∧
  contract.regions.copyToMemory ∧
  contract.regions.stridedZero ∧
  contract.regions.countNonzero ∧
  contract.regions.slicesEqual ∧
  contract.regions.hashRefinement ∧
  contract.scratch.reserve ∧
  contract.scratch.store ∧
  contract.scratch.truncate ∧
  worldStateBoundary contract.worldState.keccak contract.worldState.operations ∧
  contract.balRecorder.streamAccountDelimited ∧
  contract.balRecorder.streamSecurelyOrdered ∧
  contract.balRecorder.readsSurviveRollback ∧
  contract.balRecorder.changesFollowRollback ∧
  contract.balRecorder.readShadowedByChange ∧
  contract.authorizations.firstCommitWins ∧
  contract.witnessDb.resetClears ∧
  contract.witnessDb.insertSelect ∧
  contract.witnessDb.spanBounds ∧
  contract.witnessDb.authenticatedFrontier

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

/-- One record of the block-lifetime log store.  The payload stays in the log
data arena; the row keeps only its span. -/
structure LogRecordRow where
  address : address
  topics : List word
  dataOffset : log_data_pointer
  dataLength : log_data_length
  deriving Inhabited

/-- The frame-revertible sub-state saved by `state_journal_checkpoint`.  This
is a snapshot refinement of the ordered journal specified in
`Evm.Contracts`: pushing a frame records the predecessor of every field the
journal's semantic variants can restore, so revert and commit have the same
observable effect as inverse replay to the innermost open checkpoint. -/
structure JournalFrame where
  transient : List (StorageKey × word)
  storageTx : List (StorageKey × StorageValue)
  storageCleared : List address
  accountTx : List (address × AcctValue)
  warmAddresses : List (address × block_access_index)
  warmSlots : List (StorageKey × block_access_index)
  logs : Array LogRecordRow
  logBytes : Array byte
  deriving Inhabited

structure HostState where
  inputBytes : Array byte
  memoryBytes : Array byte
  memoryFrames : List MemoryFrame
  scratchBytes : Array byte
  codeBytes : Array byte
  logBytes : Array byte
  outputBytes : Array byte
  publicOutput : Array byte
  stackFrames : List (List word)
  continuationFrames : List FrameContinuation
  ancestorHashes : Array hash
  /-- A JUMPDEST analysis is just the set of marked program counters; the
  code length crossing the boundary is a caller-supplied bound, not state. -/
  jumpdestTables : List (jump_table_index × List code_pointer)
  codeDb : List (hash × Code)
  transient : List (StorageKey × word)
  storageTx : List (StorageKey × StorageValue)
  storageCleared : List address
  storageBlock : List (StorageKey × StorageValue)
  storageIterator : List StorageTrieEntry
  accountTx : List (address × AcctValue)
  accountBlock : List (address × AcctValue)
  accountIterator : List AcctTrieEntry
  warmEpoch : block_access_index
  warmAddresses : List (address × block_access_index)
  warmSlots : List (StorageKey × block_access_index)
  /-- Per authority: whether it was already delegated before its first
  successful tuple, and whether this transaction set a delegation. -/
  authorizations : List (address × (Bool × Bool))
  logs : Array LogRecordRow
  logsTxStart : log_store_index
  journal : List JournalFrame
  nodeDb : List (hash × (stateless_input_pointer × stateless_input_length))
  /-- Touched accounts in first-touch order, and the recorded events tagged
  with the account they belong to.  Canonical ordering is imposed once by
  `bal_prepare_iter`. -/
  balAccounts : List address
  balRecords : List (address × BalIterEntry)
  balIterator : List BalIterEntry

def initialHostState : HostState where
  inputBytes := #[]
  memoryBytes := #[]
  memoryFrames := [{ base := 0, established := 0 }]
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
  transient := []
  storageTx := []
  storageCleared := []
  storageBlock := []
  storageIterator := []
  accountTx := []
  accountBlock := []
  accountIterator := []
  warmEpoch := 1
  warmAddresses := []
  warmSlots := []
  authorizations := []
  logs := #[]
  logsTxStart := 0
  journal := []
  nodeDb := []
  balAccounts := []
  balRecords := []
  balIterator := []

/-- The generated model runs in Sail's register monad extended with the host
stores above.  `Evm.SailM` shadows `Evm.Defs.SailM` inside `Evm.Functions`, so
every generated function threads this state. -/
abbrev SailM (α : Type) :=
  StateT HostState Evm.Defs.SailM α

end Evm

namespace Evm.Functions

open Defs

/-! ### Byte-array helpers -/

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

private def readArrayBytes
    (bytes : Array byte) (base len : Nat) : List byte :=
  (List.range len).map fun index => bytes.getD (base + index) 0

private def bytesToWord (bytes : List byte) : word :=
  bytes.foldl (fun result value => result * 256 + value.toNat) 0

private def wordBytes (value : word) : List byte :=
  (List.range 32).map fun index =>
    BitVec.ofNat 8 ((value / (2 ^ (8 * (31 - index)))) % 256)

private def wordBytesLow (value : word) (len : Nat) : List byte :=
  (List.range len).map fun index =>
    BitVec.ofNat 8 ((value / (2 ^ (8 * (len - 1 - index)))) % 256)

/-- An `inc` fixed vector -- `address`, `b256` and everything built on them --
indexes byte zero at the leading protocol byte, so a canonical wire sequence is
the vector's own element order.  A `dec` vector reverses that, and its callers
reverse this result. -/
private def vectorBytes {size : Nat} (bytes : Vector byte size) : List byte :=
  bytes.toList

private def bytesToVector (size : Nat) (bytes : List byte) : Vector byte size :=
  Vector.ofFn fun index => bytes.getD index.val 0

private def assocGet [BEq κ] (entries : List (κ × ν)) (key : κ) : Option ν :=
  (entries.find? fun entry => entry.1 == key).map (·.2)

private def assocPut [BEq κ]
    (entries : List (κ × ν)) (key : κ) (newValue : ν) : List (κ × ν) :=
  (key, newValue) :: entries.filter fun entry => entry.1 != key

/-! ### Native accelerator boundary -/

@[extern "lean_evmsail_accelerate_bytes"]
private opaque nativeAccelerateBytes
    (selector : UInt8) (input : ByteArray)
    (first second third : UInt64) : Option ByteArray

private def toNativeBytes (bytes : List byte) : ByteArray :=
  ⟨(bytes.map fun value => UInt8.ofNat value.toNat).toArray⟩

private def fromNativeBytes (bytes : ByteArray) : Array byte :=
  bytes.data.map fun value => BitVec.ofNat 8 value.toNat

private def nativeFixedVector
    (size : Nat) (bytes : ByteArray) : Vector byte size :=
  Vector.ofFn fun index => BitVec.ofNat 8 (bytes.data.getD index.val 0).toNat

private def callNative
    (selector : UInt8) (input : List byte)
    (first second third : Nat) : Option ByteArray :=
  nativeAccelerateBytes selector (toNativeBytes input)
    (UInt64.ofNat first) (UInt64.ofNat second) (UInt64.ofNat third)

private def keccakOfBytes (input : List byte) : hash :=
  match callNative 0 input 0 0 0 with
  | some digest => nativeFixedVector 32 digest
  | none => default

private def sha256OfBytes (input : List byte) : hash :=
  match callNative 1 input 0 0 0 with
  | some digest => nativeFixedVector 32 digest
  | none => default

private def bytesBENat (bytes : List byte) : Nat :=
  bytes.foldl (fun value next => value * 256 + next.toNat) 0

private def accountSecureSortKey (value : address) : Nat :=
  bytesBENat (vectorBytes (keccakOfBytes (vectorBytes value)))

private def storageSecureSortKey (value : word) : Nat :=
  bytesBENat (vectorBytes (keccakOfBytes (wordBytes value)))

private def addressSortKey (value : address) : Nat :=
  bytesBENat (vectorBytes value)

/-! ### Region materialization

Every host-backed span carries only `{bytes, len}`; its nominal Sail type
selects the region, so there is no runtime source tag to dispatch on. -/

private def inputSpan (state : HostState) (base len : Nat) : List byte :=
  readArrayBytes state.inputBytes base len

def stateless_input (_ : Unit) : SailM StatelessInputSlice := do
  let length := (← get).inputBytes.size
  pure ⟨0, ⟨length, {}⟩⟩

def stateless_input_byte_at
    (s : StatelessInputSlice) (off : stateless_input_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).inputBytes.getD (s.bytes + off) 0)

def memory_slice_byte_at
    (s : EvmMemorySlice) (off : memory_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).memoryBytes.getD (s.bytes + off) 0)

def code_region_byte_at
    (s : CodeRegionSlice) (off : code_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).codeBytes.getD (s.bytes + off) 0)

def scratch_slice_byte_at
    (s : ScratchSlice) (off : scratch_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).scratchBytes.getD (s.bytes + off) 0)

def log_data_slice_byte_at
    (s : LogDataSlice) (off : log_data_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).logBytes.getD (s.bytes + off) 0)

def output_slice_byte_at
    (s : OutputSlice) (off : output_length) : SailM byte := do
  let ⟨_, ⟨_, s⟩⟩ := s
  pure ((← get).outputBytes.getD (s.bytes + off) 0)

private def inputBytesOf
    (state : HostState) (s : StatelessInputSlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.inputBytes s.bytes s.len

private def memoryBytesOf
    (state : HostState) (s : EvmMemorySlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.memoryBytes s.bytes s.len

private def codeBytesOf
    (state : HostState) (s : CodeRegionSlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.codeBytes s.bytes s.len

private def scratchBytesOf
    (state : HostState) (s : ScratchSlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.scratchBytes s.bytes s.len

private def logBytesOf (state : HostState) (s : LogDataSlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.logBytes s.bytes s.len

private def outputBytesOf (state : HostState) (s : OutputSlice) : List byte :=
  let ⟨_, ⟨_, s⟩⟩ := s
  readArrayBytes state.outputBytes s.bytes s.len

private def calldataBytesOf
    (state : HostState) (s : CalldataSlice) : List byte :=
  match s with
  | .InputCalldata bytes => inputBytesOf state bytes
  | .MemoryCalldata bytes => memoryBytesOf state bytes

private def spanWord (bytes : List byte) (position count : Nat) : word :=
  bytesToWord ((List.range count).map fun index =>
    bytes.getD (position + index) 0)

private def stridedZero
    (bytes : List byte) (start stride width count : Nat) (limit : Nat) : Bool :=
  (List.range count).all fun item =>
    let base := start + item * stride
    base + width ≤ limit &&
      (List.range width).all fun offset => bytes.getD (base + offset) 0 == 0

def stateless_input_count_nonzero
    (s : StatelessInputSlice) : SailM stateless_input_length := do
  pure ((inputBytesOf (← get) s).countP (· != 0))

def stateless_input_strided_zero
    (s : StatelessInputSlice)
    (start stride width count : stateless_input_length) : SailM Bool := do
  let bytes := inputBytesOf (← get) s
  pure (stridedZero bytes start stride width count bytes.length)

def memory_slice_strided_zero
    (s : EvmMemorySlice) (start stride width count : memory_length) :
    SailM Bool := do
  let bytes := memoryBytesOf (← get) s
  pure (stridedZero bytes start stride width count bytes.length)

def stateless_input_load_word
    (s : StatelessInputSlice) (off : stateless_input_length) : SailM word := do
  pure (spanWord (inputBytesOf (← get) s) off 32)

def memory_slice_load_word
    (s : EvmMemorySlice) (off : memory_length) : SailM word := do
  pure (spanWord (memoryBytesOf (← get) s) off 32)

def code_region_load_word
    (s : CodeRegionSlice) (off : code_length) : SailM word := do
  pure (spanWord (codeBytesOf (← get) s) off 32)

def scratch_slice_load_word
    (s : ScratchSlice) (off : scratch_length) : SailM word := do
  pure (spanWord (scratchBytesOf (← get) s) off 32)

def log_data_slice_load_word
    (s : LogDataSlice) (off : log_data_length) : SailM word := do
  pure (spanWord (logBytesOf (← get) s) off 32)

def output_slice_load_word
    (s : OutputSlice) (off : output_length) : SailM word := do
  pure (spanWord (outputBytesOf (← get) s) off 32)

def stateless_input_load_n_word
    (s : StatelessInputSlice) (off : stateless_input_length)
    (n : word_byte_count) : SailM word := do
  pure (spanWord (inputBytesOf (← get) s) off n)

def code_region_load_n_word
    (s : CodeRegionSlice) (off : code_length) (n : word_byte_count) :
    SailM word := do
  pure (spanWord (codeBytesOf (← get) s) off n)

def scratch_slice_load_n_word
    (s : ScratchSlice) (off : scratch_length) (n : word_byte_count) :
    SailM word := do
  pure (spanWord (scratchBytesOf (← get) s) off n)

/-! ### EVM memory -/

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

private def copyIntoMemory
    (values : List byte) (dst : memory_pointer) : SailM Unit := do
  let frame ← establishMemory (dst + values.length)
  modify fun state =>
    { state with
      memoryBytes :=
        writeArrayBytes state.memoryBytes (frame.base + dst) values }

private def copySpanIntoMemory
    (bytes : List byte) (dst : memory_pointer) (src len : Nat) : SailM Unit :=
  copyIntoMemory ((List.range len).map fun index =>
    bytes.getD (src + index) 0) dst

def stateless_input_copy_to_memory
    (s : StatelessInputSlice) (dst : memory_pointer)
    (off : stateless_input_length) (len : memory_length) : SailM Unit := do
  copySpanIntoMemory (inputBytesOf (← get) s) dst off len

def memory_slice_copy_to_memory
    (s : EvmMemorySlice) (dst : memory_pointer) (off : memory_length)
    (len : memory_length) : SailM Unit := do
  copySpanIntoMemory (memoryBytesOf (← get) s) dst off len

def code_region_copy_to_memory
    (s : CodeRegionSlice) (dst : memory_pointer) (off : code_length)
    (len : memory_length) : SailM Unit := do
  copySpanIntoMemory (codeBytesOf (← get) s) dst off len

def output_slice_copy_to_memory
    (s : OutputSlice) (dst : memory_pointer) (off : output_length)
    (len : memory_length) : SailM Unit := do
  copySpanIntoMemory (outputBytesOf (← get) s) dst off len

def scratch_input_slices_equal
    (left : ScratchSlice) (right : StatelessInputSlice) : SailM Bool := do
  let state ← get
  pure (scratchBytesOf state left == inputBytesOf state right)

def log_input_slices_equal
    (left : LogDataSlice) (right : StatelessInputSlice) : SailM Bool := do
  let state ← get
  pure (logBytesOf state left == inputBytesOf state right)

def input_code_slices_equal
    (left : StatelessInputSlice) (right : CodeRegionSlice) : SailM Bool := do
  let state ← get
  pure (inputBytesOf state left == codeBytesOf state right)

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

def mem_frame_enter (_ : Unit) : SailM memory_pointer := do
  let state ← get
  let parent := currentMemoryFrame state
  let base := parent.base + parent.established
  set { state with
    memoryFrames := { base := base, established := 0 } :: state.memoryFrames }
  pure base

def mem_frame_leave (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with
      memoryFrames :=
        match state.memoryFrames with
        | _ :: parent :: rest => parent :: rest
        | frames => frames }

def mem_expand (required : Nat) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => EvmMemorySliceFields k_off k_len) := do
  let frame ← establishMemory required
  pure ⟨frame.base, ⟨required, {}⟩⟩

def mem_move
    (dst : memory_pointer) (src : memory_pointer) (len : memory_length) :
    SailM Unit := do
  let frame ← establishMemory (max (src + len) (dst + len))
  let state ← get
  let values := readArrayBytes state.memoryBytes (frame.base + src) len
  set { state with
    memoryBytes :=
      writeArrayBytes state.memoryBytes (frame.base + dst) values }

def mem_load_word (off : memory_pointer) : SailM word := do
  let state ← get
  let frame := currentMemoryFrame state
  pure <| bytesToWord <| (List.range 32).map fun index =>
    if off + index < frame.established then
      state.memoryBytes.getD (frame.base + off + index) 0
    else
      0

def mem_store_word (off : memory_pointer) (value : word) : SailM Unit := do
  let frame ← establishMemory (off + 32)
  modify fun state =>
    { state with
      memoryBytes :=
        writeArrayBytes state.memoryBytes (frame.base + off) (wordBytes value) }

/-! ### The scratch arena

Sail owns the allocation cursor; the host only stores bytes at the explicit
offset it is given and reports the resulting high-water mark. -/

private def scratchResult (length : Nat) :
    Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len :=
  ⟨0, ⟨length, {}⟩⟩

private def scratchStore (off : Nat) (values : List byte) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) := do
  modify fun state =>
    { state with scratchBytes := writeArrayBytes state.scratchBytes off values }
  pure (scratchResult (off + values.length))

def host_scratch_reserve (off : Nat) (len : Nat) : SailM Bool := do
  modify fun state =>
    { state with scratchBytes := ensureArraySize state.scratchBytes (off + len) }
  pure true

def host_scratch_store_byte (off : Nat) (value : byte) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) :=
  scratchStore off [value]

def host_scratch_store_stateless_input
    {k_off k_len : Nat} (dst : Nat) (data : StatelessInputSliceFields k_off k_len) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) := do
  let state ← get
  scratchStore dst (readArrayBytes state.inputBytes data.bytes data.len)

def host_scratch_store_scratch
    {k_off k_len : Nat} (dst : Nat) (data : ScratchSliceFields k_off k_len) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) := do
  let state ← get
  scratchStore dst (readArrayBytes state.scratchBytes data.bytes data.len)

def host_scratch_store_log_data
    {k_off k_len : Nat} (dst : Nat) (data : LogDataSliceFields k_off k_len) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) := do
  let state ← get
  scratchStore dst (readArrayBytes state.logBytes data.bytes data.len)

def host_scratch_store_output
    {k_off k_len : Nat} (dst : Nat) (data : OutputSliceFields k_off k_len) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) := do
  let state ← get
  scratchStore dst (readArrayBytes state.outputBytes data.bytes data.len)

def host_scratch_store_address (off : Nat) (value : address) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) :=
  scratchStore off (vectorBytes value)

def host_scratch_store_b256 (off : Nat) (value : b256) (len : Nat) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) :=
  scratchStore off ((vectorBytes value).take len)

/-- The logs bloom is the model's one `dec` vector: element zero is its
trailing wire byte, unlike `address` and `b256`, which are `inc`.  Appending it
in canonical wire order therefore walks the vector backwards. -/
def host_scratch_store_fixed_bytes_256
    (off : Nat) (value : Vector byte 256) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) :=
  scratchStore off (vectorBytes value).reverse

def host_scratch_store_word (off : Nat) (value : word) (len : Nat) :
    SailM (Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => ScratchSliceFields k_off k_len) :=
  scratchStore off (wordBytesLow value len)

def host_scratch_truncate (length : scratch_pointer) : SailM Unit :=
  modify fun state =>
    { state with scratchBytes := state.scratchBytes.extract 0 length }

/-! ### Hashing -/

def stateless_input_keccak256 (s : StatelessInputSlice) : SailM hash := do
  pure (keccakOfBytes (inputBytesOf (← get) s))

def scratch_keccak256 (s : ScratchSlice) : SailM hash := do
  pure (keccakOfBytes (scratchBytesOf (← get) s))

def memory_keccak256 (s : EvmMemorySlice) : SailM hash := do
  pure (keccakOfBytes (memoryBytesOf (← get) s))

def code_keccak256 (s : CodeRegionSlice) : SailM hash := do
  pure (keccakOfBytes (codeBytesOf (← get) s))

def output_keccak256 (s : OutputSlice) : SailM hash := do
  pure (keccakOfBytes (outputBytesOf (← get) s))

def log_data_keccak256 (s : LogDataSlice) : SailM hash := do
  pure (keccakOfBytes (logBytesOf (← get) s))

def stateless_input_sha256 (s : StatelessInputSlice) : SailM hash := do
  pure (sha256OfBytes (inputBytesOf (← get) s))

def scratch_sha256 (s : ScratchSlice) : SailM hash := do
  pure (sha256OfBytes (scratchBytesOf (← get) s))

def memory_sha256 (s : EvmMemorySlice) : SailM hash := do
  pure (sha256OfBytes (memoryBytesOf (← get) s))

def keccak256_word (value : word) : SailM hash :=
  pure (keccakOfBytes (wordBytes value))

def keccak256_address (value : address) : SailM hash :=
  pure (keccakOfBytes (vectorBytes value))

def sha256_pair (left right : hash) : SailM hash :=
  pure (sha256OfBytes (vectorBytes left ++ vectorBytes right))

/-! ### Accelerators -/

private def acceleratorOutput
    (selector : UInt8) (s : CalldataSlice)
    (first second third : Nat) : SailM Bool := do
  let state ← get
  match callNative selector (calldataBytesOf state s) first second third with
  | none =>
      set { state with outputBytes := #[] }
      pure false
  | some output =>
      set { state with outputBytes := fromNativeBytes output }
      pure true

private def acceleratorCheck
    (selector : UInt8) (s : CalldataSlice) : SailM Bool := do
  let state ← get
  pure <| match callNative selector (calldataBytesOf state s) 0 0 0 with
    | some output => output.data.getD 0 0 != 0
    | none => false

private def acceleratorPairing
    (selector : UInt8) (s : CalldataSlice) : SailM pairing_check_result := do
  let state ← get
  pure <| match callNative selector (calldataBytesOf state s) 0 0 0 with
    | some output => if output.data.getD 0 0 == 0 then 2 else 3
    | none => 0

def accelerator_ripemd160 (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 2 s 0 0 0

def accelerator_modexp
    (s : CalldataSlice) (base exponent modulus : calldata_length) :
    SailM Bool :=
  acceleratorOutput 3 s base exponent modulus

def accelerator_bn254_add (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 4 s 0 0 0

def accelerator_bn254_mul (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 5 s 0 0 0

def accelerator_bn254_pairing (s : CalldataSlice) :
    SailM pairing_check_result :=
  acceleratorPairing 6 s

def accelerator_blake2f
    (s : CalldataSlice) (rounds : blake2_rounds) (finalBlock : y_parity) :
    SailM Bool :=
  acceleratorOutput 7 s rounds finalBlock 0

def accelerator_kzg_point_evaluation (s : CalldataSlice) : SailM Bool :=
  acceleratorCheck 8 s

def accelerator_bls_g1_add (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 9 s 0 0 0

def accelerator_bls_g1_msm (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 10 s 0 0 0

def accelerator_bls_g2_add (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 11 s 0 0 0

def accelerator_bls_g2_msm (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 12 s 0 0 0

def accelerator_bls_pairing (s : CalldataSlice) : SailM pairing_check_result :=
  acceleratorPairing 13 s

def accelerator_bls_map_fp_to_g1 (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 14 s 0 0 0

def accelerator_bls_map_fp2_to_g2 (s : CalldataSlice) : SailM Bool :=
  acceleratorOutput 15 s 0 0 0

def accelerator_p256_verify (s : CalldataSlice) : SailM Bool :=
  acceleratorCheck 16 s

def host_ecrecover
    (message : hash) (parity : y_parity) (r s : word) :
    SailM AddressResult :=
  pure <| match callNative 18
      (vectorBytes message ++ wordBytes r ++ wordBytes s) parity 0 0 with
    | some output =>
        { success := true, address := nativeFixedVector 20 output }
    | none => default

/-! ### The output buffer -/

def output_buffer_store_memory (s : EvmMemorySlice) : SailM Bool := do
  let bytes := memoryBytesOf (← get) s
  modify fun state => { state with outputBytes := bytes.toArray }
  pure true

def output_buffer_store_input (s : StatelessInputSlice) : SailM Bool := do
  let bytes := inputBytesOf (← get) s
  modify fun state => { state with outputBytes := bytes.toArray }
  pure true

def output_buffer_store_word (value : word) : SailM Bool := do
  modify fun state => { state with outputBytes := (wordBytes value).toArray }
  pure true

def output_buffer_store_words (first second : word) : SailM Bool := do
  modify fun state =>
    { state with
      outputBytes := (wordBytes first ++ wordBytes second).toArray }
  pure true

def public_output_write (s : ScratchSlice) : SailM Bool := do
  let bytes := scratchBytesOf (← get) s
  modify fun state =>
    { state with publicOutput := state.publicOutput ++ bytes.toArray }
  pure true

/-! ### Ancestor hashes -/

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
  pure ((← get).ancestorHashes.getD index default)

/-! ### The operand stack and the suspended-frame stack -/

private def currentStack (state : HostState) : List word :=
  state.stackFrames.head?.getD []

private def replaceCurrentStack
    (state : HostState) (stack : List word) : HostState :=
  { state with stackFrames := stack :: state.stackFrames.drop 1 }

private def replaceListAt (values : List α) (index : Nat) (value : α) :
    List α :=
  match values, index with
  | [], _ => []
  | _ :: rest, 0 => value :: rest
  | head :: rest, index + 1 => head :: replaceListAt rest index value

private def writeListAt
    (values : List word) (position : Nat) (value : word) : List word :=
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

def stack_top_advance
    (top : StackTop) (count : stack_slot_count) : SailM StackTop :=
  pure (top + BitVec.ofNat 64 count)

def stack_top_retreat
    (top : StackTop) (count : stack_slot_count) : SailM StackTop :=
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

/-! ### Code storage -/

private def codeRegion (offset length : Nat) :
    Sigma fun (k_off : Nat) =>
      Sigma fun (k_len : Nat) => CodeRegionSliceFields k_off k_len :=
  ⟨offset, ⟨length, {}⟩⟩

private def internCode (bytes : List byte) : SailM CodeRegionSlice := do
  let state ← get
  let offset := state.codeBytes.size
  set { state with codeBytes := state.codeBytes ++ bytes.toArray }
  pure (codeRegion offset bytes.length)

def code_region_from_input (s : StatelessInputSlice) :
    SailM CodeRegionSlice := do
  internCode (inputBytesOf (← get) s)

def code_region_from_memory (s : EvmMemorySlice) : SailM CodeRegionSlice := do
  internCode (memoryBytesOf (← get) s)

def code_region_from_output (s : OutputSlice) : SailM CodeRegionSlice := do
  internCode (outputBytesOf (← get) s)

def code_region_from_delegation (target : address) :
    SailM CodeRegionSlice := do
  internCode ([0xef, 0x01, 0x00] ++ vectorBytes target)

def code_db_lookup (key : hash) : SailM Code := do
  match assocGet (← get).codeDb key with
  | some code => pure code
  | none => pure ⟨0, ⟨0, { jumpdests := 0 }⟩⟩

def jumpdest_table_alloc (_ : CodeSlice) : SailM jump_table_index := do
  let state ← get
  let reference := state.jumpdestTables.length + 1
  set { state with
    jumpdestTables := (reference, []) :: state.jumpdestTables }
  pure reference

def jumpdest_table_mark
    (reference : jump_table_index) (length : code_length)
    (position : code_pointer) : SailM Bool := do
  let state ← get
  match assocGet state.jumpdestTables reference with
  | none => pure false
  | some positions =>
      if position ≥ length then
        pure false
      else
        set { state with
          jumpdestTables :=
            assocPut state.jumpdestTables reference (position :: positions) }
        pure true

def code_db_store (code : Code) : SailM hash := do
  let state ← get
  let ⟨_, ⟨_, fields⟩⟩ := code
  let bytes := readArrayBytes state.codeBytes fields.bytes fields.len
  let key := keccakOfBytes bytes
  set { state with codeDb := assocPut state.codeDb key code }
  pure key

def jumpdest_ref_contains
    (reference : jump_table_index) (length : code_length)
    (pc : code_pointer) : SailM Bool := do
  match assocGet (← get).jumpdestTables reference with
  | none => pure false
  | some positions => pure (pc < length && positions.contains pc)

def code_db_read_delegation (key : hash) : SailM AddressResult := do
  let state ← get
  match assocGet state.codeDb key with
  | none => pure { success := false, address := default }
  | some code =>
      let ⟨_, ⟨_, fields⟩⟩ := code
      let bytes := readArrayBytes state.codeBytes fields.bytes fields.len
      if bytes.length == 23 && bytes.getD 0 0 == 0xef &&
          bytes.getD 1 0 == 0x01 && bytes.getD 2 0 == 0x00 then
        pure
          { success := true
            address := bytesToVector 20 (bytes.drop 3) }
      else
        pure { success := false, address := default }

/-! ### The witness node database -/

def nodedb_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with nodeDb := [] }

def nodedb_insert
    (key : hash) (offset : stateless_input_pointer)
    (length : stateless_input_length) : SailM Unit :=
  modify fun state =>
    { state with nodeDb := assocPut state.nodeDb key (offset, length) }

def nodedb_lookup (key : hash) : SailM StatelessInputSlice := do
  match assocGet (← get).nodeDb key with
  | some (offset, length) =>
      pure ⟨offset, ⟨length, {}⟩⟩
  | none => pure ⟨0, ⟨0, {}⟩⟩

/-! ### Transient storage -/

def transient_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with transient := [] }

def transient_store (account : address) (slot value : word) : SailM Unit :=
  modify fun state =>
    { state with
      transient :=
        assocPut state.transient { addr := account, slot := slot } value }

def transient_load (account : address) (slot : word) : SailM word := do
  pure ((assocGet (← get).transient { addr := account, slot := slot }).getD
    default)

/-! ### The state journal

The frame-revertible sub-state is saved as one snapshot per open checkpoint.
`state_journal_commit` closes the innermost checkpoint while keeping the
frame's mutations live, so a parent checkpoint still restores them. -/

private def journalFrameOf (state : HostState) : JournalFrame :=
  { transient := state.transient
    storageTx := state.storageTx
    storageCleared := state.storageCleared
    accountTx := state.accountTx
    warmAddresses := state.warmAddresses
    warmSlots := state.warmSlots
    logs := state.logs
    logBytes := state.logBytes }

def state_journal_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with journal := [] }

def state_journal_checkpoint (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with journal := journalFrameOf state :: state.journal }

def state_journal_revert (_ : Unit) : SailM Unit := do
  let state ← get
  match state.journal with
  | [] => pure ()
  | frame :: rest =>
      set { state with
        transient := frame.transient
        storageTx := frame.storageTx
        storageCleared := frame.storageCleared
        accountTx := frame.accountTx
        warmAddresses := frame.warmAddresses
        warmSlots := frame.warmSlots
        logs := frame.logs
        logBytes := frame.logBytes
        journal := rest }

def state_journal_commit (_ : Unit) : SailM Unit :=
  modify fun state => { state with journal := state.journal.drop 1 }

/-! ### The transaction and block storage overlays -/

def storage_tx_update (entry : StorageEntry) : SailM Unit :=
  modify fun state =>
    { state with storageTx := assocPut state.storageTx entry.key entry.value }

def storage_tx_get (key : StorageKey) : SailM StorageTxLookup := do
  let state ← get
  match assocGet state.storageTx key with
  | some value => pure (StorageTxLookup.StorageTxHit value)
  | none =>
      if state.storageCleared.contains key.addr then
        pure (StorageTxLookup.StorageTxCleared ())
      else
        pure (StorageTxLookup.StorageTxMiss ())

def storage_tx_pop (_ : Unit) : SailM StorageTxPopResult := do
  let state ← get
  match state.storageTx with
  | [] => pure (StorageTxPopResult.StorageTxPopExhausted ())
  | (key, value) :: rest =>
      set { state with storageTx := rest }
      pure (StorageTxPopResult.StorageTxPopRow { key := key, value := value })

def storage_tx_clear (account : address) : SailM Unit :=
  modify fun state =>
    { state with
      storageTx := state.storageTx.filter (·.1.addr != account)
      storageCleared :=
        if state.storageCleared.contains account then state.storageCleared
        else account :: state.storageCleared }

def storage_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with storageTx := [], storageCleared := [] }

def storage_has_writes (account : address) : SailM Bool := do
  let state ← get
  pure <| state.storageTx.any fun entry =>
    entry.1.addr == account && entry.2.curr != default

def storage_block_get (key : StorageKey) : SailM StorageBlockRow := do
  match assocGet (← get).storageBlock key with
  | some value => pure { found := true, value := value }
  | none => pure { found := false, value := default }

def storage_block_put (entry : StorageEntry) : SailM Unit :=
  modify fun state =>
    let value :=
      match assocGet state.storageBlock entry.key with
      | none => entry.value
      | some prior => { entry.value with orig := prior.orig }
    { state with storageBlock := assocPut state.storageBlock entry.key value }

/-- The supplied digest is only a precomputed secure key; the reference host
rederives it when a traversal needs it, so the cache stores the value alone. -/
def storage_block_cache
    (key : StorageKey) (_slotHash : hash) (value : word) : SailM Unit :=
  modify fun state =>
    if (assocGet state.storageBlock key).isSome then state
    else
      { state with
        storageBlock :=
          assocPut state.storageBlock key { curr := value, orig := value } }

def storage_block_clear (account : address) : SailM Unit :=
  modify fun state =>
    { state with
      storageBlock := state.storageBlock.filter (·.1.addr != account) }

def storage_block_iter_begin (account : address) : SailM Unit :=
  modify fun state =>
    let addressHash := keccakOfBytes (vectorBytes account)
    { state with
      storageIterator :=
        ((state.storageBlock.filter (·.1.addr == account)).map fun row =>
          { entry := { key := row.1, value := row.2 }
            address_hash := addressHash
            slot_hash := keccakOfBytes (wordBytes row.1.slot) }).mergeSort
          fun left right =>
            storageSecureSortKey left.entry.key.slot ≤
              storageSecureSortKey right.entry.key.slot }

def storage_block_iter_next (_ : address) : SailM StorageBlockIterResult := do
  let state ← get
  match state.storageIterator with
  | [] => pure (StorageBlockIterResult.StorageBlockIterExhausted ())
  | entry :: rest =>
      set { state with storageIterator := rest }
      pure (StorageBlockIterResult.StorageBlockIterRow entry)

/-! ### The transaction and block account overlays -/

/-- A transaction-layer row's original value is the block layer's current value
at first touch. -/
private def accountTxValue
    (state : HostState) (account : address) : AcctValue :=
  match assocGet state.accountTx account with
  | some value => value
  | none =>
      let base := (assocGet state.accountBlock account).map (·.curr) |>.getD default
      { curr := base, orig := base }

def acct_tx_get (account : address) : SailM AccountRow := do
  match assocGet (← get).accountTx account with
  | some value => pure { found := true, account := value.curr }
  | none => pure { found := false, account := default }

def acct_tx_update (account : address) (value : Account) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    { state with
      accountTx := assocPut state.accountTx account { prior with curr := value } }

def acct_tx_set_balance (account : address) (value : word) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current :=
      { prior.curr with info := { prior.curr.info with balance := value } }
    { state with
      accountTx :=
        assocPut state.accountTx account { prior with curr := current } }

def acct_tx_set_nonce
    (account : address) (value : account_nonce) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current :=
      { prior.curr with info := { prior.curr.info with nonce := value } }
    { state with
      accountTx :=
        assocPut state.accountTx account { prior with curr := current } }

def acct_tx_set_code_hash (account : address) (value : hash) : SailM Unit :=
  modify fun state =>
    let prior := accountTxValue state account
    let current :=
      { prior.curr with info := { prior.curr.info with code_hash := value } }
    { state with
      accountTx :=
        assocPut state.accountTx account { prior with curr := current } }

def acct_tx_pop (_ : Unit) : SailM AcctTxPopResult := do
  let state ← get
  match state.accountTx with
  | [] => pure (AcctTxPopResult.AcctTxPopExhausted ())
  | (account, value) :: rest =>
      set { state with accountTx := rest }
      pure (AcctTxPopResult.AcctTxPopRow { addr := account, value := value })

def acct_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with accountTx := [] }

def acct_block_get (account : address) : SailM AccountRow := do
  match assocGet (← get).accountBlock account with
  | some value => pure { found := true, account := value.curr }
  | none => pure { found := false, account := default }

def acct_block_write (entry : AcctEntry) : SailM Unit :=
  modify fun state =>
    let value :=
      match assocGet state.accountBlock entry.addr with
      | none => entry.value
      | some prior => { entry.value with orig := prior.orig }
    { state with accountBlock := assocPut state.accountBlock entry.addr value }

/-- As for storage, the supplied digest is a precomputed secure key only. -/
def acct_block_cache
    (account : address) (_addressHash : hash) (value : Account) : SailM Unit :=
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
        (state.accountBlock.map fun row =>
          { entry := { addr := row.1, value := row.2 }
            address_hash := keccakOfBytes (vectorBytes row.1) }).mergeSort
          fun left right =>
            accountSecureSortKey left.entry.addr ≤
              accountSecureSortKey right.entry.addr }

def acct_block_iter_next (_ : Unit) : SailM AcctBlockIterResult := do
  let state ← get
  match state.accountIterator with
  | [] => pure (AcctBlockIterResult.AcctBlockIterExhausted ())
  | entry :: rest =>
      set { state with accountIterator := rest }
      pure (AcctBlockIterResult.AcctBlockIterRow entry)

/-! ### The EIP-7928 block-access-list recorder

Recording keeps one flat, account-tagged event list in record order.  All
canonicalization -- EIP-7928's raw ascending address and slot order, per-kind
grouping, last-write-wins per key, and dropping reads shadowed by a change --
happens once in `bal_prepare_iter`.  This list is encoded and hashed as it
stands, so it is ordered by the raw key rather than by the secure key that
orders trie traversal. -/

private def touchBalAccount
    (accounts : List address) (account : address) : List address :=
  if accounts.contains account then accounts else accounts ++ [account]

private def recordBal
    (state : HostState) (account : address) (event : BalIterEntry) : HostState :=
  { state with
    balAccounts := touchBalAccount state.balAccounts account
    balRecords := state.balRecords ++ [(account, event)] }

/-- Keeps the last occurrence of each key, in record order: a later record for
the same key replaces the earlier one. -/
private def keepLastByKey [BEq k] (key : a -> k) (items : List a) : List a :=
  (items.reverse.foldl
    (fun (seen, kept) item =>
      if seen.contains (key item) then (seen, kept)
      else (key item :: seen, item :: kept))
    (([] : List k), ([] : List a))).2

def bal_reset (_ : Unit) : SailM Unit :=
  modify fun state =>
    { state with balAccounts := [], balRecords := [], balIterator := [] }

def bal_account_touch (account : address) : SailM Unit :=
  modify fun state =>
    { state with balAccounts := touchBalAccount state.balAccounts account }

def bal_storage_change
    (index : block_access_index) (account : address) (slot value : word) :
    SailM Unit :=
  modify fun state =>
    recordBal state account
      (BalIterEntry.BalStorageChange
        { slot := slot, index := index, value := value })

def bal_storage_read (account : address) (slot : word) : SailM Unit :=
  modify fun state =>
    recordBal state account (BalIterEntry.BalStorageRead slot)

def bal_balance_change
    (index : block_access_index) (account : address) (value : word) :
    SailM Unit :=
  modify fun state =>
    recordBal state account
      (BalIterEntry.BalBalanceChange { index := index, value := value })

def bal_nonce_change
    (index : block_access_index) (account : address) (value : account_nonce) :
    SailM Unit :=
  modify fun state =>
    recordBal state account
      (BalIterEntry.BalNonceChange { index := index, value := value })

def bal_code_change
    (index : block_access_index) (account : address) (value : hash) :
    SailM Unit :=
  modify fun state =>
    recordBal state account
      (BalIterEntry.BalCodeChange { index := index, code_hash := value })

private def balEventsForAccount
    (state : HostState) (account : address) : List BalIterEntry :=
  let events := (state.balRecords.filter (·.1 == account)).map (·.2)
  let changes :=
    keepLastByKey (fun entry : BalStorageChangeEntry => (entry.slot, entry.index))
      (events.filterMap fun event =>
        match event with
        | .BalStorageChange entry => some entry
        | _ => none)
  let changedSlots := changes.map (·.slot)
  let reads :=
    (events.filterMap fun event =>
      match event with
      | .BalStorageRead slot => some slot
      | _ => none).filter fun slot => !changedSlots.contains slot
  let balances :=
    keepLastByKey (fun entry : BalBalanceChangeEntry => entry.index)
      (events.filterMap fun event =>
        match event with
        | .BalBalanceChange entry => some entry
        | _ => none)
  let nonces :=
    keepLastByKey (fun entry : BalNonceChangeEntry => entry.index)
      (events.filterMap fun event =>
        match event with
        | .BalNonceChange entry => some entry
        | _ => none)
  let codes :=
    keepLastByKey (fun entry : BalCodeChangeEntry => entry.index)
      (events.filterMap fun event =>
        match event with
        | .BalCodeChange entry => some entry
        | _ => none)
  let orderedChanges :=
    changes.mergeSort fun left right =>
      if left.slot == right.slot then left.index <= right.index
      else left.slot <= right.slot
  let orderedReads :=
    (keepLastByKey id reads).mergeSort fun left right => left <= right
  [BalIterEntry.BalAccount account] ++
    orderedChanges.map BalIterEntry.BalStorageChange ++
    orderedReads.map BalIterEntry.BalStorageRead ++
    balances.map BalIterEntry.BalBalanceChange ++
    nonces.map BalIterEntry.BalNonceChange ++
    codes.map BalIterEntry.BalCodeChange ++
    [BalIterEntry.BalAccountEnd ()]

def bal_prepare_iter (_ : Unit) : SailM Unit :=
  modify fun state =>
    let accounts :=
      state.balAccounts.mergeSort fun left right =>
        addressSortKey left <= addressSortKey right
    { state with
      balIterator :=
        accounts.foldl
          (fun result account => result ++ balEventsForAccount state account)
          [] }


def bal_iter_next (_ : Unit) : SailM BalIterEntry := do
  let state ← get
  match state.balIterator with
  | [] => pure (BalIterEntry.BalEmpty ())
  | entry :: rest =>
      set { state with balIterator := rest }
      pure entry

/-! ### The EIP-2929 warm sets

Warmth is an epoch stamp rather than a boolean: `warm_reset` installs the
transaction's block-access epoch, marking stamps the current epoch, and a
location is warm exactly while its stamp has reached it. -/

def warm_reset (epoch : block_access_index) : SailM Unit :=
  modify fun state => { state with warmEpoch := epoch + 1 }

def account_is_warm (account : address) : SailM Bool := do
  let state ← get
  pure (state.warmEpoch ≤ (assocGet state.warmAddresses account).getD 0)

def account_mark_warm (account : address) : SailM Unit :=
  modify fun state =>
    { state with
      warmAddresses :=
        assocPut state.warmAddresses account state.warmEpoch }

def storage_is_warm (account : address) (slot : word) : SailM Bool := do
  let state ← get
  let key : StorageKey := { addr := account, slot := slot }
  pure (state.warmEpoch ≤ (assocGet state.warmSlots key).getD 0)

def storage_mark_warm (account : address) (slot : word) : SailM Unit :=
  modify fun state =>
    { state with
      warmSlots :=
        assocPut state.warmSlots { addr := account, slot := slot }
          state.warmEpoch }

/-! ### The EIP-7702 authorization tracker

Transaction-local authority bookkeeping, deliberately outside the journal's
scope: no `StateJournalEntry` variant records it, so it is not
frame-revertible.  The originally-delegated flag is fixed by the authority's
first successful tuple. -/

def authorization_tracker_reset
    (_ : prepared_authorization_count) : SailM Unit :=
  modify fun state => { state with authorizations := [] }

def authorization_tracker_seen (authority : address) : SailM Bool := do
  pure ((assocGet (← get).authorizations authority).isSome)

def authorization_tracker_originally_delegated
    (authority : address) : SailM Bool := do
  pure (((assocGet (← get).authorizations authority).map (·.1)).getD false)

def authorization_tracker_delegation_set (authority : address) : SailM Bool := do
  pure (((assocGet (← get).authorizations authority).map (·.2)).getD false)

def authorization_tracker_commit
    (authority : address) (originallyDelegated delegationSet : Bool) :
    SailM Unit :=
  modify fun state =>
    match assocGet state.authorizations authority with
    | some prior =>
        { state with
          authorizations :=
            assocPut state.authorizations authority
              (prior.1, prior.2 || delegationSet) }
    | none =>
        { state with
          authorizations :=
            assocPut state.authorizations authority
              (originallyDelegated, delegationSet) }

/-! ### The transaction log store

A log record is assembled field by field into the block-lifetime store, and the
indexed readers recover it. -/

def logs_tx_reset (_ : Unit) : SailM Unit :=
  modify fun state => { state with logsTxStart := state.logs.size }

def log_begin (account : address) : SailM Unit :=
  modify fun state =>
    { state with
      logs := state.logs.push
        { address := account
          topics := []
          dataOffset := state.logBytes.size
          dataLength := 0 } }

private def modifyLastLog
    (state : HostState) (update : LogRecordRow → LogRecordRow) : HostState :=
  if state.logs.size == 0 then state
  else
    { state with
      logs := state.logs.set! (state.logs.size - 1)
        (update (state.logs.getD (state.logs.size - 1) default)) }

def log_add_topic (topic : word) : SailM Unit :=
  modify fun state =>
    modifyLastLog state fun record =>
      { record with topics := record.topics ++ [topic] }

private def appendLogData (values : List byte) : SailM Unit :=
  modify fun state =>
    let appended := { state with logBytes := state.logBytes ++ values.toArray }
    modifyLastLog appended fun record =>
      { record with dataLength := record.dataLength + values.length }

def log_add_data_memory (s : EvmMemorySlice) : SailM Unit := do
  appendLogData (memoryBytesOf (← get) s)

def log_add_data_word (value : word) : SailM Unit :=
  appendLogData (wordBytes value)

def logs_tx_start (_ : Unit) : SailM log_store_index := do
  pure (← get).logsTxStart

def logs_tx_count (_ : Unit) : SailM log_store_index := do
  let state ← get
  pure (state.logs.size - state.logsTxStart)

private def logRow (state : HostState) (index : Nat) : LogRecordRow :=
  state.logs.getD index default

def log_address (index : log_store_index) : SailM address := do
  pure (logRow (← get) index).address

def log_topics_count (index : log_store_index) : SailM log_store_index := do
  pure (logRow (← get) index).topics.length

def log_topic
    (index : log_store_index) (topic : log_store_index) : SailM word := do
  pure ((logRow (← get) index).topics.getD topic default)

def log_data_offset (index : log_store_index) : SailM log_data_pointer := do
  pure (logRow (← get) index).dataOffset

def log_data_length (index : log_store_index) : SailM log_data_length := do
  pure (logRow (← get) index).dataLength

end Evm.Functions
