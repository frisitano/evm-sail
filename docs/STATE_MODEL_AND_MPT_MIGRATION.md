# State Model and Authenticated MPT Migration

## Outcome

The optimized guest will execute transactions over a dense block-local semantic
state and will use the authenticated trie only to load original values and to
derive the final post-state root.

```text
untrusted BAL claim                 immutable stateless witness
        |                                      |
        v                                      v
dense block-local schema             authenticated node arena
        |                                      |
        v                                      |
transaction execution                         |
        |                                      |
        v                                      |
final net semantic updates -------------------+
                        |
                        v
          streaming canonical MPT merge
                        |
                        v
                 post-state root
```

The pre-state witness is never mutated. Ordinary execution does not manipulate
trie nodes, secure paths, or node hashes. Post-state construction does not replay
transaction semantics or inspect rollback history.

This document is the implementation plan for that boundary. It replaces the
current optimized design in which account/storage rows combine semantic values,
BAL history, secure hashes, trie terminals, lookup links, and transaction state.

## Optimized FFI hardening task list

The production optimized guest has a stricter implementation contract than the
GMP-backed executable specification. In `ffi/optimized`, initialization binds
typed module pointers into one pre-provisioned guest workspace; it never calls
a general heap allocator or a Sail managed-allocation helper. Native tests use
one zero-filled backing object, Spike maps a dedicated region, and ZisK obtains
the equivalent stable region through its startup platform allocator. Once
bound, module access is ordinary direct pointer indexing.

- [ ] Build only an explicit, reviewed manifest of production C translation
      units. Test and debug sources must be impossible to link by recursive
      source discovery.
- [ ] Compile handwritten optimized C with hidden visibility and warnings as
      errors. Compile generated C with a separately documented warning policy.
- [ ] Add a repository check that rejects `malloc`, `calloc`, `realloc`,
      `free`, `qsort`, production `stdio`, and Sail allocation helpers from the
      optimized backend.
- [ ] Partition one statically reserved workspace into independently bounded
      block, transaction, receipt, trie, EVM, temporary-byte, and output
      regions. Reset live lengths and epochs without clearing unused capacity.
- [ ] Replace the growing EVM memory, operand stack, suspended-frame stack,
      scratch buffer, output buffer, transient store, code store, and
      accelerator buffers with fixed-capacity storage.
- [ ] Compile the authenticated BAL into immutable dense account and storage
      schemas. Ordinary execution must never insert a persistent-state row.
- [ ] Keep block-original and block-current semantic values separate from the
      append-only transaction account/slot logs and their mutable head IDs.
- [ ] Remove sorted state mirrors, insertion `memmove`, general transaction
      merge sorting, hash/scan fallbacks, and production debug reconstruction.
- [ ] Store witness nodes in one contiguous immutable arena. Borrow encoded
      bytes from stateless input and lazily link child `NodeId`s without
      allocating individual nodes.
- [ ] Build post-state roots with fixed update arrays and one explicit
      streaming-merge stack. Receipts and other ordered-root values remain
      borrowed from their owning region.
- [ ] Consolidate forwarding-only translation units and make internal helpers
      `static`. Modules follow ownership boundaries rather than mirroring every
      Sail function with a C file.
- [ ] Define the optimized fixed representations once in the host prelude and
      use semantic `Address`, `Hash32`, `LogsBloom`, and `U256` names throughout
      the handwritten backend. Generated representation names must not leak
      past that boundary.
- [ ] Remove `evmsail_` and redundant `optimized_` prefixes. Exact Sail extern
      names form the small semantic boundary; only `guest_run`, `guest_reset`,
      and optional native-debug entry points have external visibility.
- [ ] Pass the optimized native fixture corpus, the spec native fixture corpus,
      Spike, and ZisK. Record the resulting source/line/function/allocation
      inventory and benchmark delta.

The acceptance condition for the allocation work is a static count of zero,
not merely an execution profile in which a growth path happened not to run.
The standard generated GMP model is outside that assertion; its handwritten
host backend is reviewed independently.

## Non-negotiable invariants

1. The BAL is decoded before execution as an **untrusted block-local schema**.
   It supplies identities and exact allocation counts, but its claimed accesses
   and changes remain subject to final validation.
2. Accounts and persistent storage cells have stable dense IDs for the block.
   No account or persistent-storage row is inserted during transaction execution.
3. Execution lookup uses raw addresses and raw slots. Secure hashes are not
   execution-table keys.
4. Transaction rollback journals contain dense IDs, not repeated addresses and
   slots.
5. Trie witness bytes remain borrowed from the immutable stateless input. They
   are not copied into decoded nodes or child references.
6. Newly encoded trie nodes live in one stable append-only output arena. A
   retained reference must never point into a stack object or relocatable scratch
   storage.
7. A witness node is decoded at most once. A hashed child is resolved at most
   once and its `NodeId` is cached in its parent.
8. Read authentication and post-state updates are separate concerns. Every
   consumed original value is authenticated, while only final net changes enter
   the update stream.
9. Each affected trie is rebuilt once from its immutable pre-state root and its
   complete ordered update stream. There are no intermediate roots or repeated
   per-key walks back to the root.
10. Missing schema entries, missing witness material, invalid cached links, and
    unconsumed updates fail closed. The optimized guest has no scan or hash-table
    fallback hidden behind the fast path.
11. The standard Sail model remains readable and semantic. Dense C layouts and
    borrowed pointers are optimized-backend representations, with high-level
    refinement boundaries rather than pointer operations in the base spec.

## Target semantic state

### Identity and schema

```c
typedef uint32_t AccountId;
typedef uint32_t StorageId;

typedef struct {
    Address address;
    uint32_t storage_begin;
    uint32_t storage_count;
} AccountSchema;

typedef struct {
    U256 raw_slot;
} StorageSchema;
```

`AccountId` identifies one BAL account. `StorageId` identifies one
account-specific storage cell. Storage rows are contiguous per account, so the
owning account does not need to be repeated in every `StorageSchema` entry.
Each storage schema row keeps its raw slot directly.

The BAL decoder will perform two related jobs:

- canonical decode and structural validation of the claimed BAL;
- construction of the immutable account and storage schemas.

Exact decoded counts size the arrays. These are protocol-input-derived sizes,
not prover capacity hints. Any old fixed-capacity or pre-execution sizing-hint
channel is outside this design and will be removed.

### Values and transaction metadata

```c
typedef struct {
    uint64_t nonce;
    U256 balance;
    Hash32 storage_root;
    Hash32 code_hash;
} AccountValue;

typedef struct {
    AccountValue block_original;
    AccountValue block_current;
    bool block_original_exists;
    bool block_current_exists;
    bool block_accessed;
    bool block_dirty;
    uint32_t tx_head;
} AccountEntry;

typedef struct {
    U256 block_original;
    U256 block_current;
    bool block_accessed;
    bool block_dirty;
    uint32_t tx_head;
} StorageEntry;

typedef struct {
    U256 value;
    uint32_t previous;
    uint32_t storage_generation;
} TransactionSlotLog;

typedef struct {
    AccountValue value;
    bool exists;
    bool storage_cleared;
    bool created;
    bool self_destructed;
    uint32_t active_storage_generation;
    uint32_t previous;
} TransactionAccountLog;
```

`AccountValue` contains only the four fields committed by an Ethereum account
leaf. Presence is explicit because a nonexistent account is not the same state
as an existing empty account. Creation, self-destruction, and storage clearing
are transaction-log facts rather than account-leaf fields.

`block_original` is immutable for the block. `block_current` changes only when a
transaction commits and therefore remains the transaction-original value while
that transaction executes. The active transaction state is selected by the
row's `tx_head`: zero falls back to `block_current`; a nonzero ID indexes an
append-only transaction log.

A slot-log record is allocated only when the active rollback scope cannot safely
mutate the current head: the first write in a frame, or the first write after an
account storage-clear generation changes. Repeated writes in the same frame and
generation update the frame-owned record in place. The record's `previous` ID
preserves the value visible to its parent scope.

`block_accessed` and `block_dirty` are ordinary booleans because the schema and
state arrays are initialized once for the block. They control BAL validation and
first insertion into the block worklists. Trie terminals, node digests, BAL
change-history links, and raw lookup-chain links do not belong in either
semantic value.

The block state owns dense worklists:

```c
AccountId *dirty_accounts;
StorageId *dirty_storage;
AccountId *changed_accounts;
StorageId *changed_storage;
```

An ID is appended when its block or rollback-scope membership first becomes
true. Block finalization removes entries whose `block_current` value equals
`block_original`.

### Lookup indexes

The schema owns deterministic local indexes:

```text
raw address                         -> AccountId
AccountId + raw slot                -> StorageId
```

An open-addressed table is appropriate for address and per-account slot lookup,
but it stores only IDs. It does not own semantic rows and it does not maintain a
sorted mirror. A missing entry is an access-list validation failure; it does not
trigger row insertion, a table scan, or a prover hint fallback.

Call frames cache `storage_account_id` and `code_account_id`, so `SLOAD` and
`SSTORE` do not repeatedly resolve the current address.

### Append-only logs, checkpoints, and transaction merge

Each independently revertible frame checkpoints the account- and slot-log
lengths plus the starts of its changed-ID lists. A write records a new head ID
only once per row and rollback scope. Revert restores each listed row's
`previous` head and then truncates the log arenas and worklists. It never looks
up a raw address or slot and never copies a prior `U256` from a separate undo
record.

Storage clearing is account-wide. It advances the active account log's storage
generation in O(1); a slot head is visible only when its generation matches the
account's active generation. The first post-clear write appends a new-generation
slot record whose `previous` link retains the pre-clear record for rollback.
Reverting the account head restores the earlier generation and makes the older
slot heads visible again.

Successful child-frame logs remain linked until transaction completion. A
transaction commit iterates the touched dense IDs, reduces their visible values
into `block_current`, clears their heads, and truncates all transaction arenas.
A full transaction revert clears the heads and truncates without modifying
block state. Pre-block, transaction, and post-block BAL indices annotate these
dense rows; they do not create duplicate state tables or sorted transaction
mirrors.

## State outside persistent accounts and storage

The same separation applies to the other mutable subsystems, without forcing
unrelated data into `BlockState`.

| State | Target representation | Lifetime and authentication |
| --- | --- | --- |
| Warm accounts | Named field in `AccountTransactionState` | Transaction-local metadata over the BAL schema |
| Warm persistent slots | Named field in `StorageTransactionState` | Transaction-local metadata over the BAL schema |
| Transient storage | Dense transaction-local rows keyed by `AccountId` and raw slot | Cleared at transaction end; never enters the MPT |
| Authorization state | `AccountId` plus epoch/authorization metadata | Transaction-local; account mutations still use `AccountState` |
| Self-destruct/create state | Named fields under the row's single transaction epoch | Journaled and reduced to final account semantics |
| Code | Content-addressed code arena keyed by code hash | Account rows store only the code hash; frames cache the code account ID |
| Call frames | Semantic registers plus cached account IDs | Frame stack and rollback checkpoints; no trie metadata |
| BAL histories | Append-only events linked by dense IDs | Compared with the claimed BAL during final validation |
| Logs and receipts | Receipt/output arenas | Not world state and not part of the state MPT |
| Ancestor hashes and block environment | Existing fixed host tables/registers | Read-only execution environment, not block-local semantic state |

Before transient storage is assigned `StorageId`, its BAL coverage must be
confirmed. If the protocol does not require every transient slot in the BAL, it
will use a separate transaction-local `TransientId` namespace rather than
silently widening the persistent-storage schema.

## Immutable witness representation

### Parent-relative witness child references

The current copied `node_reference.bytes[32]` representation will be removed.
A decoded witness child is represented relative to the encoded parent node:

```c
typedef uint32_t NodeId;

typedef struct {
    NodeId node_id;
    uint16_t offset;
    uint8_t len;
} WitnessChildRef;
```

The meaning is:

```text
len == 0       empty child; offset and node_id are ignored
1 <= len < 32  embedded child RLP at parent.encoded + offset
len == 32      child digest at parent.encoded + offset
```

`offset` is relative to the start of `NodeRecord.encoded`, not the stateless
input as a whole. A `uint16_t` is sufficient for canonical account/storage MPT
nodes; the decoder must establish that bound before narrowing. Resolving the
bytes therefore requires one base-plus-offset operation only when the edge is
actually followed.

`len` is also the child-reference tag, so an additional empty/embedded/hashed
enum would duplicate information. The referenced node's decoded MPT kind
(branch, extension, or leaf) belongs to its `NodeRecord` and is not repeated on
every incoming edge.

`node_id == NODE_ID_UNRESOLVED` means that a non-empty edge has not yet been
linked. Once resolved, the ID is written into the edge. An empty edge remains
the all-zero representation; it does not consume a special child `NodeId`.
Consequently a branch may allocate sixteen compact child slots while the host's
zero-initialized memory gives all absent children their correct representation.

Generated merge results use the separate `TrieResult`/generated-arena slice
representation below. They are not forced into a parent-relative witness
reference before their parent has been encoded. This avoids a union, a copied
32-byte buffer, or helper families such as
`mpt_empty_reference`, `mpt_hash_reference`, and `mpt_inline_reference`.

The decoded branch stores sixteen `WitnessChildRef` values directly; it does
not keep a separate parallel `child_ids[16]` array. An extension stores one
`WitnessChildRef`.

The decoder writes the fields directly. Small helpers are retained only where
they establish a non-trivial invariant or own storage; constructors that merely
assign `node_id`, `offset`, and `len` are deleted.

### Node records and sentinels

```c
enum {
    NODE_ID_UNRESOLVED = 0,
    STATE_TRIE_ROOT_ID = 1,
    NODE_ID_EMPTY = UINT32_MAX,
};

typedef struct {
    StatelessInputSlice encoded;
    DecodedTrieNode node;
} NodeRecord;
```

The optimized C representation of `StatelessInputSlice.ptr` is a borrowed
`const uint8_t *`; the base Sail model continues to use a semantic source slice.
For child edges, `len` distinguishes empty from unresolved. `NODE_ID_EMPTY` is
reserved for context roots and cached account storage roots, where there is no
parent edge carrying a length.

The authenticated account-trie root receives record ID 1. When an account is
first authenticated, its decoded storage root is resolved and the resulting
root ID is cached in account authentication metadata. An empty storage root is
`NODE_ID_EMPTY`, never zero. Non-empty unresolved roots and children start at
zero until resolved.

A node record does not store its own digest. A digest is an incoming reference
to an encoded node, not part of the decoded node's semantic contents. The
digest index owns the computed digest key needed for full collision-safe lookup,
and maps it to the borrowed encoding and `NodeId`. A trie context separately
owns its expected root digest. An active merge frame retains the incoming
`WitnessChildRef` together with its parent record when it may need to reuse the
original subtree unchanged.

### Trie contexts and traversal order

The subtree-contiguity rule applies only within one trie context. The global
account trie is one context. Every account storage trie is a separate context.
An interval from the account trie is never compared with a storage interval,
and intervals from two different storage tries are never compared.

`NodeId` remains stable record identity. IDs are reserved in DFS preorder within
one contiguous range for each trie context, even if the corresponding records
are decoded lazily. Account and storage context ranges are disjoint. Each
context therefore owns its local ID range and subtree intervals:

```c
typedef struct {
    NodeId node_id;
    NodeId subtree_end;
    uint8_t depth;
} TrieLayoutEntry;

typedef struct {
    NodeId root_id;
    NodeId node_begin;
    NodeId node_end;
    TrieLayoutEntry *layout;
} TrieContext;
```

Within `[node_begin, node_end)`, increasing `NodeId` is preorder and
`subtree_end` is the first ID after the node's subtree. `depth` is the secure-key
nibble depth at which the node begins. A terminal `NodeId` is meaningful only
with the `TrieContext` supplied to the lookup or merge; the context is not
repeated as a runtime ID in every authentication record.

Discovery order cannot change IDs or ancestry. The witness layout reserves IDs;
lazy decoding only fills their records. If the optimized representation stores
`subtree_end` and `depth` directly beside each occurrence record, the separate
`TrieLayoutEntry` array may be specialized away without changing the interface.

The guest does not reorder or copy encoded witness bytes. The producer may emit
nodes in DFS order, while the guest records only pointers into their original
locations and validates the declared/derived layout.

### Digest index and lazy linking

The witness loader owns one open-addressed map whose entries retain the digest
key outside `NodeRecord`:

```text
node digest -> NodeId and borrowed encoded slice
```

It exists only to resolve authenticated child references and trie roots. Full
32-byte equality is checked after bucket selection. It is not used for ordinary
account or storage execution lookup.

On child traversal:

1. `child.len == 0` returns the empty result.
2. A nonzero `child.node_id` is followed directly.
3. A non-empty child with zero `node_id` resolves the borrowed child digest or embedded encoding,
   validates it, allocates/locates its immutable record, and stores the ID in the
   child reference.

Following traversals use the cached ID. Embedded children are normalized to
their own node records when first traversed, while their original bytes remain
borrowed from the parent encoding.

## Authentication metadata

Trie data is kept beside, not inside, semantic values:

```c
typedef struct {
    NodeId terminal_node_id;
    NodeId storage_root_id;
} AccountAuthentication;

typedef struct {
    NodeId terminal_node_id;
} StorageAuthentication;
```

Authentication stores only the node at which the lookup terminated. The other
candidate terminal fields are derivable and are deliberately not cached:

- preorder and ancestry follow from the context-local `NodeId` layout;
- secure-key depth comes from the node's layout entry;
- branch child follows from the next secure-key nibble;
- branch/extension/leaf kind comes from the decoded node;
- exact match, empty child, leaf mismatch, or extension mismatch follows from
  comparing that decoded node with the locally computed secure key.

`NODE_ID_UNRESOLVED` means that the semantic value has not yet been
authenticated. `NODE_ID_EMPTY` is a valid terminal only for an empty trie root.
An empty branch child terminates at its real parent branch `NodeId`; the child
nibble is derived later from the secure key.

The first implementation has no semantic secure-key/digest cache. Account and
slot secure keys are computed directly from the schema's raw address or raw
slot at the authentication or final-update boundary that needs them. A local
update record may retain the computed key for the duration of one trie merge,
but account and storage rows do not retain it across phases. This keeps the
initial layout simple and makes any later cache an evidence-driven optimization.

The witness digest-to-`NodeId` index described above is different: it is the
authenticated node locator required to resolve a 32-byte MPT child reference,
not a cache of `keccak(address)` or `keccak(slot)`.

The account-trie terminal is interpreted against the single global account
context. Storage authentication records are partitioned by `AccountId`, and
each partition is interpreted only against that account's storage context. The
context comes from the enclosing operation rather than a tag stored in each
record. An empty storage root or empty-root terminal uses the explicit empty
sentinel; zero continues to mean unresolved or unauthenticated.

The first implementation authenticates original values eagerly on the first
schema miss. This is the least disruptive sound boundary: execution receives no
unauthenticated original value, and the resulting terminal `NodeId` is reused at
finalization. A deferred BAL-shaped multiproof pass can be evaluated later, but
is not required for this migration.

## Streaming post-state construction

### Update formation

At block finalization:

1. Iterate `dirty_storage`, discard semantic no-ops, and form PUT/DELETE updates.
2. Group the updates by account.
3. Compute each net update's slot secure key directly from its raw slot.
4. Select that account's storage-trie context and order its updates by terminal
   `NodeId`, which is that context's DFS preorder.
5. Merge each account's immutable storage witness with its complete update
   stream to obtain one post-storage root.
6. Apply those roots to the affected semantic account values.
7. Iterate `dirty_accounts`, discard semantic no-ops, and encode final account
   PUT/DELETE updates.
8. Order account updates by terminal `NodeId` in the global account-trie
   context.
9. Merge once against the immutable account-trie witness.

Every update is consumed exactly once. Read-only authenticated entries never
enter the update stream.

### Merge operation

The semantic operation is:

```text
merge_subtree(original subtree, ordered final updates)
    -> canonical post-state subtree reference
```

For a subtree with no updates, return its borrowed original reference without
descending. For descendant updates, partition the contiguous update range by
affected child, recursively or iteratively merge only those children, reuse all
other original child references, and encode the parent once.

All updates terminating at the same original leaf, empty child, or mismatching
extension are handled as one group. Their remaining secure-key suffixes are
partitioned by nibble with a local radix builder; they are not globally sorted
again.

The optimized implementation uses an explicit stack. The base Sail
specification keeps a recursive `merge_subtree` definition because it expresses
the proof boundary directly.

### Canonical results

The merge returns a generated result, which is deliberately distinct from a
parent-relative witness child reference:

```text
empty                         -> empty result
canonical encoding < 32 bytes -> reference into generated-node arena
canonical encoding >= 32      -> 32-byte digest in generated-node arena
```

The final root boundary returns the required root hash. Parent nodes only see
the final child reference; they do not need to know whether it was reused,
inserted, collapsed, embedded, or hashed.

Normalization is performed at node finalization:

- empty branches disappear;
- a branch with one child and no value collapses;
- adjacent extensions merge;
- a leaf/extension split uses the longest common prefix;
- embedding versus hashing is decided from the final canonical RLP length.

## Implementation sequence

### Phase 0: Lock down semantic and performance baselines

- Record the optimized native full-corpus result and representative ZisK
  profiles.
- Add counters for account/storage probes, row insertions, sorts, `memmove`,
  node decodes, digest resolutions, Keccak calls, reused subtrees, generated
  nodes, and finalized ancestors.
- Add focused synthetic MPT fixtures before changing storage layout.

Acceptance: current output remains byte-exact and the counters explain the old
path's work.

### Phase 1: Fix MPT reference ownership and file hygiene

- Replace copied `node_reference.bytes` with pointer-plus-length child refs.
- Keep witness pointers in stateless input; introduce a stable append-only arena
  only for generated encodings and digests.
- Remove trivial reference-constructor helpers and copied-reference helpers.
- Rename witness/node structures around their role rather than their storage
  history.
- Split the optimized trie implementation into cohesive units:
  `trie_witness.c` for immutable loading/linking and `trie_merge.c` for
  canonical rebuilding, with `mpt.c` retaining the small public boundary.
- Preserve the explicit unresolved/empty/root sentinels.

Acceptance: no witness child/reference byte copy, no retained pointer into
relocatable scratch, and all existing optimized fixtures pass.

### Phase 2: Compile the BAL into a dense schema

- Add a pre-execution canonical BAL schema decode.
- Assign `AccountId` and account-specific `StorageId` values.
- Allocate exact-size schema, semantic, epoch, worklist, journal, and BAL-event
  arrays from decoded counts.
- Build local address and per-account raw-slot indexes.
- Retain the existing final BAL consistency check, rewritten over dense IDs.
- Audit system-call access classes before making a missing schema row fatal.

Acceptance: schema construction trusts no BAL value semantics, execution cannot
insert persistent rows, and extra/missing BAL claims are still rejected.

### Phase 3: Move semantic account and storage execution to dense arrays

- Replace combined account/storage rows with schema, values, and authentication
  arrays.
- Cache account IDs in frames and use direct `StorageId` resolution for storage
  opcodes.
- Move warm-state, lifecycle flags, original/current values, and code identity to
  their designated arrays.
- Keep secure hashes and terminal node IDs out of execution rows.
- Replace sorted raw-key lists and `memmove` insertion with ID worklists.

Acceptance: account/storage hot paths perform no secure hashing, sorting, row
copying, or dynamic persistent-row insertion.

### Phase 4: Replace transaction overlays with ID journals and generations

- Journal first mutations by dense ID.
- Capture transaction originals once per transaction generation.
- Preserve storage generations so account storage clears and their reverts are
  O(1) with respect to the account's slot count.
- Preserve checkpoint generations so each row is snapshotted at most once per
  active frame checkpoint.
- Revert by walking the journal to the frame checkpoint.
- Commit by iterating transaction-touched IDs; remove the C `tx_merge` table
  traversal and sorted iterators.
- Convert BAL event links to dense IDs.

Acceptance: rollback and merge contain no raw-key lookup, and nested-call revert
tests are byte-exact.

### Phase 5: Build the immutable authenticated witness arena

- Index input node digests to borrowed input slices.
- Assign the account root ID and resolve storage root IDs on first authenticated
  account access.
- Decode nodes once, cache child IDs, and distinguish unresolved from empty.
- Reserve per-trie DFS `NodeId` ranges, record subtree intervals/depths, and
  validate their invariants.
- Store only terminal `NodeId`s in the separate account/storage authentication
  arrays.
- Compute semantic secure keys at their use boundaries; do not add an account or
  slot digest cache in the first implementation.

Acceptance: repeated reads follow cached IDs, every consumed original is rooted
in the authenticated pre-state, and no witness encoding is copied.

### Phase 6: Implement the generic streaming merge

- Implement `merge_subtree` in Sail over an immutable base and ordered updates.
- Implement the optimized explicit-stack equivalent in `trie_merge.c`.
- Add equal-terminal local radix construction.
- Add canonical branch/extension/leaf normalization.
- Reuse untouched borrowed references and allocate only changed encodings.
- Assert that the update cursor is fully consumed.

Acceptance: synthetic MPT differential tests cover empty roots, embedded and
hashed children, leaf/extension splits, equal terminals, deletion collapse,
storage clearing, repeated nodes, and untouched-subtree reuse.

### Phase 7: Use one merge engine for storage and account roots

- Form final storage updates directly from `dirty_storage`.
- Compute each affected storage root and attach it to the corresponding final
  account value.
- Form final account updates from `dirty_accounts`.
- Compute the global post-state root with the same merge engine.
- Remove the account/storage block iterators and the temporary
  post-storage-root table channel.

Acceptance: each affected trie is opened/finalized once, unchanged tries return
their original roots, and final output matches EELS across the corpus.

### Phase 8: Migrate the remaining state subsystems

- Convert warm account/slot tracking to dense IDs plus transaction-generation
  fields.
- Convert authorization and lifecycle tracking to account IDs.
- Decide transient-storage ID coverage after the BAL semantics audit.
- Ensure code lookup remains content-addressed while frames use cached account
  IDs.
- Keep receipts/logs and environment data in their existing purpose-specific
  arenas rather than folding them into world state.

Acceptance: no subsystem recreates a parallel raw address/slot state table.

### Phase 9: Delete transitional infrastructure

Delete after the new path is the sole optimized implementation:

- fixed-capacity and pre-execution capacity-hint files and APIs;
- prover account/storage lookup hints;
- sorted raw account/storage arrays, `qsort`, and insertion `memmove` paths;
- dynamic persistent-row insertion;
- duplicate per-row account and slot secure hashes;
- per-row original-leaf fields superseded by terminal node IDs;
- account/storage block iterators used only by old root construction;
- old transaction table merge/drain APIs;
- copied MPT reference buffers and trivial reference constructors;
- runtime fallbacks to scans or legacy hash tables.

A native-only debug oracle may compare the old and new implementations during a
phase, but it must not be linked into the benchmark guest and must be removed
before the phase is considered complete.

## File-level target

| Area | Target responsibility |
| --- | --- |
| `ffi/optimized/state_schema.c` | BAL-derived identities and deterministic raw-key indexes |
| `ffi/optimized/block_state.c` | Dense semantic values, generations, worklists, and lifecycle state |
| `ffi/optimized/state_journal.c` | ID-based checkpoints, undo, commit, and revert |
| `ffi/optimized/trie_witness.c` | Borrowed witness nodes, digest index, child linking, layouts, terminals |
| `ffi/optimized/trie_merge.c` | Ordered-update streaming merge and generated-node arena |
| `ffi/optimized/host/state.c` | High-level optimized Sail ABI boundaries only |
| `sail/lib/mpt/` | Readable recursive immutable merge and canonical MPT semantics |
| `sail/lib/state_trie.sail` | Account/storage value encoding and composition of storage then account roots |
| `sail/host/state.sail` | Semantic state contracts, without C table-layout concepts |
| `sail/optimised/` | High-level overrides selecting the dense C implementation |

Exact filenames may be combined where a unit would otherwise be trivial, but
the ownership boundaries must remain visible. In particular, the new design
must not recreate `state_db.c` as one large structure mixing every concern.

## Validation gates

Every phase must pass the narrowest relevant tests before the next phase starts:

1. Optimized native focused fixtures and synthetic unit tests.
2. Standard native focused fixtures when the shared Sail semantics change.
3. Full retained v0.6.2 corpus on optimized native.
4. Full retained v0.6.2 corpus on standard native after shared-spec changes.
5. A real RISC-V Spike fixture after generated C or ABI changes.
6. Representative ZisK fixtures with a forced guest rebuild.

The performance review records at least:

- total ZisK steps and proving cost;
- Keccak count split between execution reads, witness authentication, storage
  roots, and the account root;
- RLP node decode count;
- digest-map probes and cached-child hits;
- semantic account/storage lookup probes;
- generated versus reused trie nodes;
- finalized ancestor count;
- peak semantic, journal, witness, and generated-node arena sizes.

The expected structural result is more important than a small early cycle win:
ordinary execution should disappear from trie profiles, sorting and row-copying
should disappear from transaction merge, and trie cost should scale with the
authenticated reads plus affected subtree frontier rather than with repeated
per-update root walks.

## Final architecture test

The migration is complete only when this description is literally true:

```text
BAL claim
  -> exact dense identities
  -> semantic execution with ID journals
  -> final net ID worklists
  -> authenticated terminal-node updates
  -> per-account immutable storage-trie merge
  -> immutable account-trie merge
  -> post-state root
```

No stage may require a mutable trie, a secure-hash execution key, a duplicated
witness reference, a sorted transaction overlay, or an unvalidated prover lookup
hint.
