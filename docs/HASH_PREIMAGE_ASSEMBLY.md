# Composite Hash Preimage Assembly

## Decision

Use one shared assembly workspace:

1. Keep the public Sail model purpose-specific and protocol-readable.
   Fixed-shape composite hashes expose named components rather than a
   `list(Bytes)` or a general segment union.
2. Assemble every non-contiguous RLP or composite preimage in the reusable
   executor scratch arena. A scratch window has a reserved capacity and a
   logical live span; only that span is passed to the hash or parent encoder.
3. Keep already-contiguous inputs and true fixed-input operations direct.
   Scratch is an assembly workspace, not a mandatory staging step for every
   cryptographic call.

The primitive cryptographic boundary remains the digest of one contiguous byte
sequence. Each region has an exact axiom rather than a generic source-tagged
slice:

```sail
stateless_input_keccak256 : StatelessInputSlice -> hash
scratch_keccak256         : ScratchSlice -> hash
memory_keccak256          : EvmMemorySlice -> hash
```

A segmented representation is an assembly technique, not a second
cryptographic meaning.

## Reth/REVM Reference Pattern

The Reth benchmark guest uses REVM for EVM execution, but REVM is not the
owner of block-header or transaction RLP encoding. Its relevant encoders are:

- `alloy-rlp 0.3.16` and `alloy-consensus 2.1.1` for headers and
  transactions;
- the guest's `zeth-mpt` dependency for MPT node encoding.

Alloy's general pattern is a semantic `Encodable` interface with separate
`length()` and `encode(out)` operations:

```rust
let mut out = Vec::with_capacity(value.length());
value.encode(&mut out);
```

This is a length pass followed by one write pass. Compound values can inspect
their fields twice, but each encoded byte is written only once. For types with
a compile-time maximum, `encode_fixed_size` writes into an uninitialized
`ArrayVec` backing array and records only the initialized prefix.

The concrete benchmark paths are not all equally optimized:

| Path | Reservation behavior |
| --- | --- |
| Transaction signing | Computes `payload_len_for_signature()`, allocates exactly that many bytes, encodes once, then hashes |
| MPT nodes | Computes the payload length, reserves the full node encoding, encodes once; node references of at most 33 bytes use fixed-capacity `ArrayVec` storage |
| Block header | Computes `header_payload_length()` while encoding, but `Header::hash_slow()` starts from `Vec::new()` rather than reserving `Header::length()` |

The guest also currently computes the reconstructed header hash in
`seal_slow()` during payload conversion, discards that seal when converting
back to an unsealed block, and computes `block.hash_slow()` again during
public-key recovery. That duplicate header serialization and Keccak is a guest
integration cost, not an inherent REVM requirement.

The transferable idea is therefore the `length + encode(out)` contract, not
the use of a Rust `Vec` or `ArrayVec`. EVM Sail can reuse one scratch
allocation across every construction. Protocol bounds determine how much of
that arena to reserve; they do not require a separate Sail vector type or a
per-operation C stack buffer. The readable Sail model already exposes the
required size equations through `rlp_*_size`; the optimized implementation
should write forward through one checked scratch cursor without per-field host
calls.

## Preimage Inventory

Current hashing sites fall into four classes:

| Class | Current examples | Shape | Design |
| --- | --- | --- | --- |
| Already contiguous | EVM `KECCAK256`, SHA precompile, witness nodes and code, public keys, block-access-list bytes | One nominal region slice | Hash that region directly |
| Small fixed composite | `CREATE2`, per-type request digest, execution-requests hash | Two to five known roles | Purpose-specific Sail helper; optimized C uses shared fixed-part machinery |
| Bounded serialization | Block header, `CREATE` address, authorization signing, inline MPT node/reference | RLP encoding with a useful protocol maximum | Reserve that maximum in scratch, write forward, consume the live span |
| Input-sized serialization | Transactions, receipts, and complete MPT material | RLP encoding whose practical capacity follows the input or block | Reserve the computed/input-derived capacity in scratch, then write forward |
| Fixed scalar/chunk | Secure account/slot keys, SSZ parent, inline MPT node | Address, word, two hashes, or a short fixed value | Direct fixed-shape helper |

The distinction is about reservation policy, not storage type. An Amsterdam
block header has a small fixed maximum, whereas receipts and complete tries
scale with the input. Both use the same scratch cursor and return a
`ScratchSlice`.

## Shared Scratch Windows

Represent one assembly lifetime as a reserved scratch window:

```sail
struct PreimageBuilder = {
    mark          : source_pointer,
    content_start : source_pointer,
    capacity_end  : source_pointer,
}

function preimage_begin(capacity, prefix_headroom) -> PreimageBuilder
function preimage_finish(builder, prefix_len) -> ScratchSlice
```

`preimage_begin` reserves `capacity` bytes beyond the current scratch mark and
positions the prepared write cursor after `prefix_headroom`.
`preimage_finish` checks the cursor against `capacity_end`, writes the outer
prefix into its reserved headroom, and returns the live `ScratchSource`
sub-slice. Unused reserved bytes have no semantic meaning and must never be
read, copied, or hashed.

RLP encoders write forward. They must not build output by prepending bytes to
a Sail list and later reversing or materializing it. Scalar encoders first
compute their canonical byte width and then emit exactly that many bytes in
canonical order.

### Prefix headroom

When an outer RLP prefix depends on the content length, reserve its maximum
prefix width at the front and begin content writes after that headroom:

```text
mark                       content_start                 cursor       capacity_end
 |                                 |                       |               |
 v                                 v                       v               v
 | unused prefix headroom | content written forward ... | unused capacity |
```

After the content is complete:

1. derive `content_len = cursor - content_start`;
2. encode the prefix immediately before `content_start`;
3. set `start = content_start - prefix_len`;
4. set `length = prefix_len + content_len`.

This neither moves the content nor deletes unused headroom. The consumer is
given a scratch slice starting at `start` with `length` bytes.

For a statically known outer-prefix width, reserve exactly that width. For a
generic RLP list bounded by a 64-bit host length, nine bytes of headroom are
enough. The abstract Sail implementation may retain a wider bound where its
length domain requires it.

A bounded encoding passes its protocol maximum as `capacity`. An input-sized
encoding passes an exact or checked input-derived capacity. The underlying
arena grows only when its high-water mark is insufficient and is then reused
by later operations.

Reservation is non-semantic:

- it does not advance the Sail-owned cursor;
- it does not initialize unused bytes;
- it does not change the byte sequence denoted by scratch;
- failure follows the existing host-allocation failure path;
- proof-oriented implementations may model successful reservation as a no-op.

Reservation must happen before native code borrows the scratch pointer because
growth may reallocate the arena. Between borrowing the pointer and consuming
the completed slice, no operation may grow scratch or invalidate the mark.

Reservation alone only prevents growth. If each field remains a separate
Sail-to-C call, each call still performs a capacity check and boundary
crossing. The optimized backend should lower a complete builder region to one
borrowed native cursor or splice the whole high-level operation to C. The
standard model keeps the explicit forward field writes.

### Canonical scalar encoding

Computing an RLP scalar's encoded width remains necessary for canonical
encoding even when scratch capacity is already reserved. For a 256-bit word:

```text
payload_width = ceil(bit_length(value) / 8)
```

The encoder then writes only the selected big-endian bytes. It must not write
all 32 bytes and subsequently delete or shift leading zeroes. In optimized C,
finding the first non-zero byte can inspect at most four 64-bit limbs before
the forward write. This is bounded control work, not a second serialization
pass.

The same rule applies to fixed Ethereum values. Address and hash vectors are
already canonical byte sequences; encoders should read `value[i]` directly
and add only the required RLP prefix. They should not convert those vectors to
Sail lists or round-trip them through words.

### Storage selection

| Encoding | Storage | Allocation policy |
| --- | --- | --- |
| Already-contiguous nominal slice | Its named region | Hash directly |
| Fixed input requiring no serialization, such as two SHA roots | Direct fixed-shape helper | C-local implementation where appropriate |
| Any RLP or composite preimage assembly | Reusable scratch window | Reserve a bound once, write through one cursor, rewind after consumption |

This deliberately avoids dynamic Sail vectors, per-operation fixed byte
vectors, and separate C-local serialization arrays. Scratch owns capacity;
the returned `ScratchSlice` owns the live-span description. Nested constructions
use properly nested marks and must consume or copy an inner result before its
mark is rewound.

## Shared Optimized C Substrate

Optimized fixed-shape wrappers should share a C-private helper:

```c
enum evmsail_hash_algorithm {
  EVMSAIL_KECCAK256,
  EVMSAIL_SHA256,
};

struct evmsail_hash_part {
  const uint8_t *bytes;
  uint64_t len;
};

bool evmsail_hash_parts(enum evmsail_hash_algorithm algorithm,
                        const struct evmsail_hash_part *parts,
                        uint32_t part_count,
                        uint64_t total_len,
                        uint8_t output[32]);
```

This structure is private to `ffi/`; it does not cross the generated Sail ABI.
Each purpose-specific wrapper reserves any required scratch capacity first,
then resolves its exact nominal slice arguments and converts them into at most
five local pointer/length parts. The helper:

1. validates the count, component lengths, sum, and overflow;
2. hashes a single already-contiguous part directly;
3. otherwise writes into the already-reserved scratch window;
4. copies each non-empty part once in order;
5. invokes the selected accelerator once.

Region-backed parts are resolved after the scratch reservation, because that
reservation may reallocate the backing store. Copies use overlap-safe behavior
when a part can refer to the scratch arena.

Do not expose `struct evmsail_hash_part` as a Sail `HashSegment` union. That
would make every standard and proof build construct generated tagged
aggregates merely to reach a C-private pointer-and-length representation.

## Fixed-Shape Public Operations

The first generic substrate users should be:

| Sail operation | Logical parts | Maximum arity |
| --- | --- | ---: |
| `CREATE2` address digest | `0xff`, sender, salt, init-code hash | 4 |
| Per-type request digest | request type, request data | 2 |
| Execution-requests hash | present per-type request digests in ascending type order | 5 |

Each operation retains its own Sail constructor or equation. Their optimized
C wrappers share only `evmsail_hash_parts`.

Transaction signing deliberately bypasses this generic part helper. Its RLP
list prefix and optional EIP-155 suffix are generated directly into the
reserved scratch window, so neither becomes an intermediate fixed buffer that
must then be copied into a second output.

The following should stay direct rather than enter the generic part helper:

- the provenance-specific single-slice `keccak256` overloads;
- `keccak256_address(address)`;
- `keccak256_word(word)`;
- `sha256_pair(hash, hash)`.

Those operations already have one fixed input representation and do not
benefit from part assembly.

## Block Header Hashing

The block header is not naturally a small fixed-part hash. In Amsterdam it
contains 23 RLP fields, including the large fixed logs bloom and variable
extra data. The SSZ transport gives most source fields fixed widths and stable
offsets. Their destination RLP widths split into three groups:

| Field shape | SSZ source width | Canonical RLP width |
| --- | ---: | ---: |
| `Bytes32` root or hash | 32 | 33 |
| Fee recipient | 20 | 21 |
| Logs bloom | 256 | 259 |
| Post-merge nonce | constant 8 | 9 |
| Post-merge difficulty | constant zero | 1 |
| `uint64` quantity | 8, little-endian | 1–9, minimal big-endian |
| Base fee (`uint256`) | 32, little-endian | 1–33, minimal big-endian |
| Extra data | variable, at most 32 | 1–33 |

Consequently, an Amsterdam header has:

- 620 bytes of fixed RLP field material;
- 629–749 bytes of RLP list content after the variable canonical widths are
  included;
- a three-byte outer list prefix throughout that range;
- a total encoded size of at most 752 bytes.

The current model correctly:

1. computes the RLP content length;
2. writes the canonical RLP header in field order;
3. hashes the resulting contiguous scratch slice.

This gives the scratch builder a fixed reservation bound even though the
encoded value retains its actual length. The standard equation can state the
bounded construction explicitly:

```sail
let builder = preimage_begin(752, 3);
/* the existing explicit header fields */
let encoded = preimage_finish_rlp_list(builder);
let result = keccak256(encoded);
scratch_rewind(builder.mark);
result
```

The standard model's explicit RLP writers target scratch exactly as they do
today; the builder adds one reservation, prefix headroom, and an end-bound
check. It introduces no Sail vector and no second output representation.

Representing all header fields as a fixed vector of segment unions would still
create a large generated aggregate and require a second concatenation. The
fixed quantity here is the maximum scratch reservation, not a per-field
intermediate or a fixed-length encoded value.

The optimized whole-operation implementation can avoid intermediate Sail
materialization without changing the standard equation:

- reserve 752 bytes at the current scratch high-water mark before borrowing
  the backing pointer;
- copy `Bytes32`, address, bloom, and extra-data fields directly from their
  validated SSZ source offsets, adding only their known RLP prefixes;
- write the recomputed transaction, withdrawal, request, and block-access-list
  roots from their fixed hash values;
- reverse and trim only the SSZ little-endian integer fields while writing
  their minimal big-endian RLP representations;
- write the three-byte list prefix into the reserved headroom;
- invoke Keccak once over the live scratch slice and rewind the mark.

The optimized splice writes through the borrowed scratch pointer. It does not
allocate a local `uint8_t[752]`, clear the reserved capacity, or advance
through per-field host calls. Every byte in the live slice is written before
the accelerator reads it; unused reserved bytes remain outside that slice.
No scratch operation that can grow the arena may occur while the native
pointer is borrowed.

This requires `block_header_hash` to retain access to the validated
`StatelessInputRef` (or an equivalent source reference) in addition to the
recomputed roots. It avoids converting fixed input fields into Sail values and
then back into bytes on the hash path; fields that execution or validation
needs may still be decoded normally. The standard Sail body remains the
explicit field-by-field RLP equation; only the optimized build splices this
source-aware implementation.

## Transaction Signing Case

Represent a transaction signing preimage as one scratch construction with four
ordered logical regions:

1. an optional EIP-2718 transaction-type byte;
2. the reconstructed unsigned RLP-list prefix;
3. the variable-length source-backed pre-signature field span;
4. the optional legacy EIP-155 `(chain_id, 0, 0)` suffix.

The readable Sail implementation retains the protocol construction. The
optimized build may replace the complete operation with one C implementation
that reserves scratch once and writes all four regions through one cursor.

This is deliberately not a return to `list(Bytes)` or to fixed prefix/suffix
vectors. The model records the decisions and lengths needed to write the
canonical bytes directly into scratch.

## Context

[`tx_signing_hash`](../sail/lib/tx.sail) receives a
`StatelessInputSlice` spanning the
RLP-encoded pre-signature fields. That span is already contiguous in the
stateless input and its length is known before hashing. It is not, however, the
complete signing preimage:

- typed transactions prepend their EIP-2718 type byte;
- the unsigned field list needs a newly encoded RLP-list prefix because the
  signed envelope's prefix includes `v`, `r`, and `s`;
- legacy EIP-155 transactions append the RLP encoding of
  `(chain_id, 0, 0)`.

The accelerator interface in
[`zkvm_accelerators.h`](../ffi/zkvm_accelerators.h) accepts one contiguous
pointer and length. It does not expose scatter/gather input or an incremental
Keccak absorb operation. The final concatenation therefore remains necessary,
but it can be the only materialization.

## Semantic Model

Use a transaction-specific plan containing protocol decisions and lengths, not
materialized prefix or suffix byte vectors:

```sail
let TX_SIGNING_PREIMAGE_BOUND = transaction_length_bound + 45
type TxSigningPreimageLength = range(0, TX_SIGNING_PREIMAGE_BOUND)

struct TxSigningPlan = {
    type_present         : bool,
    type_byte            : byte,
    field_span           : StatelessInputSlice,
    eip155_present       : bool,
    eip155_chain_id      : word,
    unsigned_content_len : rlp_scratch_length,
    total_len            : TxSigningPreimageLength,
}
```

The additional 45 bytes are the maximum one-byte type domain, nine-byte RLP
list prefix, and 35-byte EIP-155 suffix. If the existing transaction bound
already includes any of this envelope overhead, the implementation should
tighten the expression accordingly; it must not weaken the length to an
unbounded `int`.

The planner is pure Sail and owns all protocol decisions:

```sail
function tx_signing_plan(
    tx_type : TxType,
    field_span : StatelessInputSlice,
    v : word,
) -> TxSigningPlan
```

It:

1. determines whether the type byte is present;
2. derives the legacy EIP-155 chain ID when applicable;
3. computes the canonical suffix length without materializing the suffix;
4. computes the unsigned RLP content and list-prefix lengths;
5. computes `total_len` as the sum of the four logical regions.

`tx_signing_hash` remains a small, readable equation:

```sail
function tx_signing_hash(t, field_span, v) =
    keccak256_tx_signing_plan(tx_signing_plan(t, field_span, v))
```

The function name should say “transaction signing” rather than expose a
generic segmented-hash primitive. This keeps the protocol decisions visible in
both generated documentation and proof statements.

## Standard Implementation

The standard Sail implementation of `keccak256_tx_signing_plan` should:

1. mark the executor scratch cursor;
2. reserve `total_len`;
3. write the optional type byte and canonical unsigned-list prefix directly;
4. append `field_span`;
5. write the optional EIP-155 suffix directly from `eip155_chain_id`;
6. assert that the resulting length equals `total_len`;
7. call `scratch_keccak256` on the live `ScratchSlice` and rewind the cursor.

This implementation is the executable specification. It makes the exact byte
equation visible and keeps the primitive cryptographic boundary as a
single-region hash, rather than introducing “segmented Keccak” as a second
cryptographic axiom.

The writers target scratch directly:

```sail
scratch_push_byte(value)
rlp_write_list_prefix(unsigned_content_len)
scratch_push_slice(field_span)
rlp_write_eip155_suffix(eip155_chain_id)
```

These operations are ordinary scratch-storage contracts, not hash axioms. The
optimized whole-operation splice removes their individual boundary crossings
from the production guest.

## Optimized Implementation

The optimized splice may replace the complete
`keccak256_tx_signing_plan` helper with a C binding. The C implementation
must consume the generated model types by including `EVMSAIL_MODEL_H`; it must
not mirror generated aggregate layouts in a hand-written header.

The operation is:

```text
validate plan lengths and total
             |
             v
borrow total_len bytes at the scratch high-water mark
             |
             v
write type? -> prefix -> copy field span -> write suffix?
             |
             v
zkvm_keccak256(buffer, total_len, output)
```

The buffer should come from the existing reusable scratch backing store:

- reserve exactly `total_len` once;
- do not advance the Sail-owned scratch cursor;
- resolve `field_span` only after reservation, because reservation may move the
  scratch backing allocation;
- use `memmove` when a source can itself be `ScratchSource`;
- call the accelerator before the borrowed region can be reused;
- perform no per-call heap allocation after the scratch capacity has reached
  its high-water mark.

The C side must independently verify:

```text
total_len ==
    (type_present ? 1 : 0)
  + rlp_list_prefix_len(unsigned_content_len)
  + field_span.len
  + canonical_eip155_suffix_len
```

It must also reject integer overflow, invalid source ranges, component lengths
outside their Sail bounds, and accelerator failure. Failure handling should
follow the existing hash-boundary convention; it must not silently hash a
partial buffer.

## Why the Known Length Matters

Capacity and live length serve different purposes:

- a proven worst-case or input-derived capacity makes one scratch reservation
  possible;
- the live length selects the canonical bytes passed to RLP's parent or the
  hash accelerator;
- an exact final length can tighten a reservation, while a checked protocol
  maximum is sufficient for bounded constructions.

Knowing the live length does not make the four transaction-signing regions
adjacent. It does allow a cursor-based optimized implementation to avoid:

- repeated scratch-capacity checks;
- geometric growth during a single construction;
- several Sail-to-C scratch append calls;
- a separate segment accumulator and its capacity;
- Sail list allocation and cons-cell traversal;
- per-operation heap or stack serialization buffers.

The variable field span is copied once because the accelerator requires one
contiguous input. Removing that copy would require a compatible
scatter/gather or incremental Keccak API in `zkvm_accelerators.h`, which is
outside this design.

## Scope

Shared scratch windows apply to every non-contiguous serialized or composite
preimage. Bounded values reserve their protocol maximum; input-sized values
reserve an exact or checked derived capacity. The shared C-private part helper
applies to the three small fixed-composite operations listed above and uses
the same scratch arena internally. This is generic at the storage and
accelerator boundary without making protocol-facing Sail functions generic.

The following remain direct operations:

- the exact nominal-slice `keccak256` overload for already-contiguous inputs;
- `keccak256_address(address)`;
- `keccak256_word(word)`;
- `sha256_pair(hash, hash)`.

Receipt and MPT encoders should adopt one forward cursor and one reservation
incrementally when their existing size equations make the final length
available. Inline MPT encodings reserve their existing small fixed bounds;
complete receipt/trie material reserves an input-derived capacity. Neither
should be converted into a large fixed-part aggregate.

Block-header hashing should reserve 752 bytes in scratch directly. A complete
optimized C override should borrow that reservation and preserve the same
capacity and live-span invariant.

### Migration correctness note

The execution-requests preimage is the concatenation of present request
digests in ascending request-type order: `d0 || d1 || d2 || d3 || d4`. Any
migration from prepend-based list construction to forward scratch appends must
preserve that order. The current in-progress scratch rewrite appends the values
in descending order and must be corrected before it is used as validation or
benchmark evidence.

## Extraction and Proof Boundary

Pure and proof-oriented extractions should see:

- the pure `tx_signing_plan` construction;
- the standard Sail body of `keccak256_tx_signing_plan`;
- the explicit Sail equations for `CREATE2`, request digests, and the
  execution-requests hash;
- the protocol capacity bounds and explicit forward scratch writes;
- the same scratch-window operations for bounded and input-sized serializers;
- the existing provenance-specific contiguous cryptographic axioms;
- the existing scratch host contracts.

Only the optimized C build should splice the purpose-specific assembly
helpers. The C-private `evmsail_hash_part` descriptor does not appear in
extraction. This prevents the performance representation from becoming a new
proof axiom and retains each protocol preimage as explicit Sail semantics.

## Implementation Sequence

1. Correct and test execution-request digest ordering before collecting new
   evidence.
2. Introduce one scratch-window abstraction with nested marks, prefix
   headroom, checked capacity, and a live `ScratchSlice` result.
3. Replace RLP output built by prepending to Sail `list(byte)` with canonical
   width calculation and forward writes into scratch. Do not change list-based
   decoding in this step.
4. Add `TxSigningPlan`, its pure constructor, and its standard Sail scratch
   construction. Write its prefix and suffix directly rather than
   materializing fixed intermediate vectors.
5. Add the non-semantic reserve operation and a prepared scratch cursor.
   Merely reserving while retaining separate host calls is insufficient.
6. Add purpose-specific standard Sail helpers for `CREATE2`, per-type request
   digests, and the ordered execution-requests hash.
7. Add the C-private `evmsail_hash_parts` implementation and optimized
   whole-operation splices for the three fixed-shape composite operations,
   consuming generated types through `EVMSAIL_MODEL_H`. Transaction signing
   uses its dedicated plan-driven scratch splice from step 4.
8. Convert `block_header_hash` to a 752-byte scratch reservation in both the
   standard equation and optimized splice. Write the three-byte list prefix
   into reserved headroom and hash the live span.
9. Convert authorization signing, `CREATE`, transactions, receipts, and MPT
   construction to the same scratch builder, selecting fixed or input-derived
   reservation bounds as appropriate.
10. Audit fixed Ethereum types so address/hash bytes are emitted directly by
    canonical index, without word or Sail-list materialization.
11. Regenerate C output and run formatting/type checks.
12. Run standard and optimized native fixtures byte-exact against EELS.
13. Run the complete retained v0.6.2 fixture corpus on both native builds.
14. Rebuild the production ZisK ELF and compare step counts with the retained
    pre-change guest, Reth, and Ethrex.
15. Profile the affected hash and serialization scopes before selecting the
    next whole-operation splice.

## Acceptance Criteria

- Legacy pre-EIP-155, legacy EIP-155, and every supported typed transaction
  produce the same signing digest as EELS.
- `CREATE2`, each per-type request digest, and the execution-requests hash
  produce the same digests as EELS.
- Present execution-request digests are hashed in ascending type order.
- The standard model contains no composite-hash `list(Bytes)` and retains each
  protocol preimage construction in Sail.
- No RLP encoder constructs output by prepending to a Sail `list(byte)` or
  converts a fixed byte vector to a list merely to hash or append it.
- Every scratch-window consumer checks that its live span lies within the
  reservation and consumes only that span.
- No serializer uses a separate dynamic Sail vector, per-operation fixed byte
  vector, or C-local output array.
- Optimized encoders reserve before borrowing scratch, do not grow the arena
  while borrowed, and neither clear unused capacity nor move encoded content
  to close prefix headroom.
- Each optimized small-composite implementation performs one exact reserve,
  one copy per present part, and one accelerator call.
- Every RLP builder reserves once and asserts its final cursor.
- The optimized Amsterdam header reserves at most 752 scratch bytes and makes
  one Keccak call over its actual encoded length.
- A single-slice hash still hashes its source directly without scratch.
- Direct address, word, and SSZ-pair helpers do not use segmented machinery.
- Standard and optimized native builds pass the complete retained fixture
  corpus byte-exact.
- The rebuilt ZisK guest does not regress the five-case workload; any
  measurable change is recorded in
  [`OPTIMIZATIONS.md`](OPTIMIZATIONS.md).

## Deferred Alternatives

### General `list(Bytes)`

Rejected because it allocates and traverses generated Sail list and union
objects for a statically bounded operation.

### General fixed-capacity segment vector

Rejected as the public Sail interface because every slot still needs a segment
union, the generated ABI must copy or initialize the full aggregate, and
transaction, request, and address-derivation roles become less clear. The
optimized C implementation shares a private pointer/source descriptor only
where the final preimage necessarily combines discontiguous sources.

### Per-operation fixed byte vectors or C-local output arrays

Rejected because they duplicate the existing scratch workspace, make ordinary
Sail initialize fixed vectors, and create a second lifetime/capacity discipline
for optimized C. A fixed protocol maximum remains useful as the scratch
reservation bound without becoming a distinct storage representation.

### Treat every RLP field as a hash part

Rejected for block headers, receipts, and MPT nodes. The fields first require
canonical RLP encoding, their arity is larger or data-dependent, and the
one-shot accelerator still requires a contiguous result. A forward scratch
window is the smaller abstraction.

### Hash the signed transaction in place

Not valid: the signed envelope has a different RLP-list prefix and contains
signature fields absent from the signing preimage.

### Incremental or scatter/gather Keccak

Potentially copy-free, but not available through the required
`zkvm_accelerators.h` interface.
