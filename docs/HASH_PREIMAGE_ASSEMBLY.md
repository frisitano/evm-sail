# Composite Hash Preimage Assembly

## Decision

Use a two-layer design:

1. Keep the public Sail model purpose-specific and protocol-readable.
   Fixed-shape composite hashes expose named components rather than a
   `list(Bytes)` or a general segment union.
2. Share the mechanics below that boundary:
   - an exact-sized scratch builder for arbitrary serialized preimages;
   - a C-private fixed-part hashing helper used by optimized whole-operation
     splices.

The primitive cryptographic boundary remains the digest of one contiguous byte
sequence:

```sail
keccak256 : ByteSlice -> hash
sha256    : ByteSlice -> hash
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
the use of a Rust `Vec`. The readable Sail model already follows the same
shape through `rlp_*_size` and `rlp_write_*`. The optimized implementation
should make those sizes effective by preparing one destination cursor and
writing through it without per-field host calls.

## Preimage Inventory

Current hashing sites fall into four classes:

| Class | Current examples | Shape | Design |
| --- | --- | --- | --- |
| Already contiguous | EVM `KECCAK256`, SHA precompile, witness nodes and code, public keys, block-access-list bytes | One `ByteSlice` | Hash the source directly |
| Small fixed composite | Transaction signing, `CREATE2`, per-type request digest, execution-requests hash | Two to five known roles | Purpose-specific Sail helper; optimized C uses shared fixed-part machinery |
| Serialized aggregate | Block header, `CREATE` address, authorization signing, receipts, MPT nodes | RLP encoding with two to 23 or more components | Exact-sized scratch builder, then one contiguous hash |
| Fixed scalar/chunk | Secure account/slot keys, SSZ parent, inline MPT node | Address, word, two hashes, or a short fixed value | Direct fixed-shape helper |

The distinction matters. A fixed-capacity segment value is useful for a
two-to-five-part preimage. It is a poor representation for an Amsterdam block
header with 23 RLP fields or a branch node with 17 RLP elements: those values
must be serialized regardless, and the accelerator still consumes one
contiguous buffer.

## Shared Exact-Sized Builder

Introduce a generic builder that reserves the complete output size before
writing:

```sail
struct PreimageBuilder = {
    start        : source_pointer,
    expected_end : source_pointer,
}

function preimage_begin(total_len : source_length) -> PreimageBuilder
function preimage_finish(builder) -> ByteSlice
```

`preimage_begin` asks the scratch host to reserve
`scratch_cursor + total_len` once. `preimage_finish` asserts that the final
cursor equals `expected_end`.

The builder is therefore a bounded reservation and end-position witness, not
a second cursor threaded through every writer. This avoids changing the
readable RLP equations merely to carry an optimization object. The existing
writers already target the same scratch arena, and their size functions
compute the encoded lengths needed for exact reservation.

Reservation alone only prevents growth or reallocation. If every existing
writer remains a separate Sail-to-C call, each call still performs its own
capacity check and boundary crossing. The optimized backend should therefore
lower a complete builder region to one prepared native cursor, or replace the
whole high-level operation with a C implementation that writes through such a
cursor. The standard build can retain the explicit, readable sequence of RLP
writers.

The reserve operation is a non-semantic capacity operation:

- it does not advance the cursor;
- it does not initialize bytes;
- it does not change the byte sequence denoted by scratch;
- failure raises the existing host-allocation failure path;
- proof-oriented implementations may model successful reservation as a no-op.

## Shared Optimized C Substrate

Optimized fixed-shape wrappers should share a C-private helper:

```c
enum evmsail_hash_algorithm {
  EVMSAIL_KECCAK256,
  EVMSAIL_SHA256,
};

enum evmsail_hash_part_kind {
  EVMSAIL_INLINE_PART,
  EVMSAIL_SOURCE_PART,
};

struct evmsail_hash_part {
  enum evmsail_hash_part_kind kind;
  uint64_t len;
  union {
    const uint8_t *inline_bytes;
    struct {
      uint64_t source;
      uint64_t off;
    } source;
  } value;
};

bool evmsail_hash_parts(enum evmsail_hash_algorithm algorithm,
                        const struct evmsail_hash_part *parts,
                        uint32_t part_count,
                        uint64_t total_len,
                        uint8_t output[32]);
```

This structure is private to `ffi/`; it does not cross the generated Sail ABI.
Each purpose-specific wrapper converts its generated arguments into at most
five local parts. Inline fixed values carry a stable pointer; source-backed
slices carry their source identifier and offset so resolution can be deferred.
The helper:

1. validates the count, component lengths, sum, and overflow;
2. hashes a single already-contiguous part directly;
3. otherwise borrows exactly `total_len` bytes at the scratch high-water mark;
4. copies each non-empty part once in order;
5. invokes the selected accelerator once.

Source-backed parts are resolved after the scratch reservation, because that
reservation may reallocate the backing store. Copies use overlap-safe behavior
when a part can refer to `ScratchSource`.

Do not expose `struct evmsail_hash_part` as a Sail `HashSegment` union. That
would make every standard and proof build construct generated tagged
aggregates merely to reach a C-private pointer-and-length representation.

## Fixed-Shape Public Operations

The first generic substrate users should be:

| Sail operation | Logical parts | Maximum arity |
| --- | --- | ---: |
| Transaction signing hash | type byte?, unsigned RLP prefix, field span, EIP-155 suffix? | 4 |
| `CREATE2` address digest | `0xff`, sender, salt, init-code hash | 4 |
| Per-type request digest | request type, request data | 2 |
| Execution-requests hash | present per-type request digests in ascending type order | 5 |

Each operation retains its own Sail constructor or equation. Their optimized
C wrappers share only `evmsail_hash_parts`.

The following should stay direct rather than enter the generic part helper:

- `keccak256(ByteSlice)`;
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

Consequently, the exact Amsterdam header size is a fixed subtotal plus the
canonical RLP widths of the integer fields and extra data. The list prefix is
also exactly derivable from that content length. The current model correctly:

1. computes the RLP content length;
2. writes the canonical RLP header in field order;
3. hashes the resulting contiguous scratch slice.

The generic improvement is to begin that construction with an exact
reservation:

```sail
let encoded_len = rlp_list_size(content_len);
let builder = preimage_begin(encoded_len);
rlp_write_list_prefix(content_len);
/* the existing explicit header fields */
let encoded = preimage_finish(builder);
keccak256(encoded)
```

Representing all header fields as a fixed segment vector would still require
RLP-encoding every scalar, would create a large generated aggregate, and would
still copy all parts before the one-shot accelerator call. It is therefore
less suitable than the sized RLP builder.

The optimized whole-operation implementation can go further without changing
the standard equation:

- copy `Bytes32`, address, bloom, and extra-data fields directly from their
  validated SSZ source offsets, adding only their known RLP prefixes;
- write the recomputed transaction, withdrawal, request, and block-access-list
  roots from their fixed hash values;
- reverse and trim only the SSZ little-endian integer fields while writing
  their minimal big-endian RLP representations;
- reserve the exact final length once and invoke Keccak once.

This requires `block_header_hash` to retain access to the validated
`StatelessInputRef` (or an equivalent source reference) in addition to the
recomputed roots. It avoids converting fixed input fields into Sail values and
then back into bytes on the hash path; fields that execution or validation
needs may still be decoded normally. The standard Sail body should remain the
explicit field-by-field RLP equation; only the optimized build should splice
this source-aware implementation.

## Transaction Signing Case

Represent a transaction signing preimage as four explicit, fixed-arity
components:

1. an optional EIP-2718 transaction-type byte;
2. the reconstructed unsigned RLP-list prefix;
3. the variable-length source-backed pre-signature field span;
4. the optional legacy EIP-155 `(chain_id, 0, 0)` suffix.

The readable Sail implementation retains the protocol construction. The
optimized build may replace the complete segment-concatenation helper with one
C operation that uses `evmsail_hash_parts`.

This is deliberately not a return to `list(Bytes)`. The arity and role of every
piece are known statically, so the model should not allocate a Sail list,
traverse cons cells, or carry a general-purpose byte-segment union.

## Context

[`tx_signing_hash`](../sail/lib/tx.sail) receives a `ByteSlice` spanning the
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
Keccak absorb operation. Segments therefore improve representation and buffer
allocation, but cannot remove the final concatenation.

## Semantic Model

Use transaction-specific component types rather than a general byte-segment
sum type:

```sail
struct TxSigningTypeSegment = {
    present : bool,
    value   : byte,
}

struct TxSigningPrefixSegment = {
    bytes : vector(9, inc, byte),
    len   : range(1, 9),
}

struct TxSigningSuffixSegment = {
    bytes : vector(35, inc, byte),
    len   : range(0, 35),
}

let TX_SIGNING_PREIMAGE_BOUND = transaction_length_bound + 45
type TxSigningPreimageLength = range(0, TX_SIGNING_PREIMAGE_BOUND)

struct TxSigningSegments = {
    type_segment   : TxSigningTypeSegment,
    list_prefix    : TxSigningPrefixSegment,
    field_span     : ByteSlice,
    eip155_suffix  : TxSigningSuffixSegment,
    total_len      : TxSigningPreimageLength,
}
```

The additional 45 bytes are the maximum one-byte type domain, nine-byte RLP
list prefix, and 35-byte EIP-155 suffix. If the existing transaction bound
already includes any of this envelope overhead, the implementation should
tighten the expression accordingly; it must not weaken the length to an
unbounded `int`.

The segment constructor is pure Sail and owns all protocol decisions:

```sail
function tx_signing_segments(
    tx_type : TxType,
    field_span : ByteSlice,
    v : word,
) -> TxSigningSegments
```

It:

1. determines whether the type segment is present;
2. derives the legacy EIP-155 chain ID when applicable;
3. computes and encodes the suffix;
4. computes the unsigned RLP content length;
5. encodes the correct RLP-list prefix;
6. computes `total_len` as the sum of the four segment lengths.

`tx_signing_hash` remains a small, readable equation:

```sail
function tx_signing_hash(t, field_span, v) =
    keccak256_tx_signing_segments(tx_signing_segments(t, field_span, v))
```

The function name should say “transaction signing” rather than expose a
generic segmented-hash primitive. This keeps the fixed arity and the meaning
of each segment visible in both generated documentation and proof statements.

## Standard Implementation

The standard Sail implementation of `keccak256_tx_signing_segments` should:

1. mark the executor scratch cursor;
2. append each present segment in order;
3. assert that the resulting length equals `total_len`;
4. call the contiguous `keccak256(ByteSlice)` axiom;
5. rewind the cursor.

This implementation is the executable specification. It makes the exact byte
equation visible and keeps the primitive cryptographic axiom as
`keccak256(ByteSlice)`, rather than introducing “segmented Keccak” as a second
cryptographic axiom.

The small fixed buffers are value components, not Sail byte lists. Only their
live prefixes are appended. Unused bytes have no semantic meaning and must not
be hashed.

To preserve that property, introduce fixed-buffer scratch operations alongside
the existing slice operation:

```sail
scratch_push_byte(value)
scratch_push_tx_signing_prefix(prefix)
scratch_push_slice(field_span)
scratch_push_tx_signing_suffix(suffix)
```

Their host contracts copy the inline generated representation directly. They
must not convert the fixed vectors to `list(byte)`. These operations are
ordinary scratch-storage contracts, not hash axioms; their meaning is simply
to append the live bytes in canonical index order. The optimized whole-helper
splice removes the four-call overhead from the production guest.

## Optimized Implementation

The optimized splice may replace the complete
`keccak256_tx_signing_segments` helper with a C binding. The C implementation
must consume the generated model types by including `EVMSAIL_MODEL_H`; it must
not mirror generated aggregate layouts in a hand-written header.

The operation is:

```text
validate component lengths and total
             |
             v
borrow total_len bytes at the scratch high-water mark
             |
             v
copy type? -> prefix -> field span -> suffix?
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
  + prefix_len
  + field_span.len
  + suffix_len
```

It must also reject integer overflow, invalid source ranges, component lengths
outside their Sail bounds, and accelerator failure. Failure handling should
follow the existing hash-boundary convention; it must not silently hash a
partial buffer.

## Why the Known Length Matters

Knowing the length does not make the four memory regions adjacent. It does
allow a cursor-based optimized implementation to avoid:

- repeated scratch-capacity checks;
- geometric growth during a single construction;
- several Sail-to-C scratch append calls;
- a separate segment accumulator and its capacity;
- Sail list allocation and cons-cell traversal.

The variable field span is copied once because the accelerator requires one
contiguous input. Removing that copy would require a compatible
scatter/gather or incremental Keccak API in `zkvm_accelerators.h`, which is
outside this design.

## Scope

The shared exact-sized builder applies to serialized preimages whose encoded
length is already computed. The shared C-private part helper applies to the
four small composite operations listed above. This is generic at the storage
and accelerator boundary without making protocol-facing Sail functions
generic.

The following remain direct operations:

- `keccak256(ByteSlice)` for already-contiguous inputs;
- `keccak256_address(address)`;
- `keccak256_word(word)`;
- `sha256_pair(hash, hash)`.

Receipt and MPT encoders should adopt exact reservation incrementally when
their existing size equations make the final length available. They should not
be converted into large fixed-part aggregates.

Block-header hashing should first adopt the sized builder. A complete
optimized C override is a separate, profile-gated experiment.

### Migration correctness note

The execution-requests preimage is the concatenation of present request
digests in ascending request-type order: `d0 || d1 || d2 || d3 || d4`. Any
migration from prepend-based list construction to forward scratch appends must
preserve that order. The current in-progress scratch rewrite appends the values
in descending order and must be corrected before it is used as validation or
benchmark evidence.

## Extraction and Proof Boundary

Pure and proof-oriented extractions should see:

- the pure `tx_signing_segments` construction;
- the standard Sail body of `keccak256_tx_signing_segments`;
- the explicit Sail equations for `CREATE2`, request digests, and the
  execution-requests hash;
- the exact-sized preimage-builder operations used by RLP serializers;
- the existing contiguous `keccak256(ByteSlice)` cryptographic axiom;
- the existing scratch host contracts.

Only the optimized C build should splice the purpose-specific
segment-materialization helpers. The C-private `evmsail_hash_part` descriptor
does not appear in extraction. This prevents the performance representation
from becoming a new proof axiom and retains each protocol preimage as explicit
Sail semantics.

## Implementation Sequence

1. Correct and test execution-request digest ordering before collecting new
   evidence.
2. Add the non-semantic exact-reserve host contract and the bounded
   `PreimageBuilder`.
3. Add an optimized prepared-cursor lowering or whole-operation C override;
   merely reserving while retaining separate host calls is insufficient.
4. Introduce fixed-capacity transaction RLP-prefix and EIP-155-suffix encoders.
5. Add `TxSigningSegments`, its pure constructor, and its standard Sail hash
   body.
6. Add purpose-specific standard Sail helpers for `CREATE2`, per-type request
   digests, and the ordered execution-requests hash.
7. Add the C-private `evmsail_hash_parts` implementation.
8. Add optimized whole-operation splices for the four small composite
   operations, consuming generated types through `EVMSAIL_MODEL_H`.
9. Move the block header, authorization signing, `CREATE`, receipt, and MPT
   encoders to exact reservation where their size equations are already
   available.
10. Regenerate C output and run formatting/type checks.
11. Run standard and optimized native fixtures byte-exact against EELS.
12. Run the complete retained v0.6.2 fixture corpus on both native builds.
13. Rebuild the production ZisK ELF and compare step counts with the retained
    pre-change guest, Reth, and Ethrex.
14. Profile payload validation before deciding whether a whole
    `block_header_hash` C splice is justified.

## Acceptance Criteria

- Legacy pre-EIP-155, legacy EIP-155, and every supported typed transaction
  produce the same signing digest as EELS.
- `CREATE2`, each per-type request digest, and the execution-requests hash
  produce the same digests as EELS.
- Present execution-request digests are hashed in ascending type order.
- The standard model contains no composite-hash `list(Bytes)` and retains each
  protocol preimage construction in Sail.
- Each optimized small-composite implementation performs one exact reserve,
  one copy per present part, and one accelerator call.
- RLP builders that know their final size reserve once and assert their final
  cursor.
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
optimized C implementation shares a private pointer/source descriptor instead.

### Treat every RLP field as a hash part

Rejected for block headers, receipts, and MPT nodes. The fields first require
canonical RLP encoding, their arity is larger or data-dependent, and the
one-shot accelerator still requires a contiguous result. Exact-sized
serialization is the smaller abstraction.

### Hash the signed transaction in place

Not valid: the signed envelope has a different RLP-list prefix and contains
signature fields absent from the signing preimage.

### Incremental or scatter/gather Keccak

Potentially copy-free, but not available through the required
`zkvm_accelerators.h` interface.
