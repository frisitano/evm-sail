# Generated Extern Boundary

This file inventories the Sail-to-Coq boundary for the full `evm-sail`
guest. It is the proof-facing contract layer for operations declared as
`impure` in Sail and linked to the native or zkVM host implementation.

The generated Coq may contain no obvious top-level `Axiom`, `Parameter`, or
`Admitted` declarations in `evm.v` / `evm_types.v`. That does not
make the external behavior pure: the Sail backend represents these operations
inside its monad, and useful source theorems still need laws for their host
effects. The initial Coq contract skeleton is
`proof/coq/ExternBoundary.v`.

## Source of the Boundary

The abstract contracts are declared inline beside their Sail users:

```sail
val operation = impure { c: "host_symbol" } : T
```

They live primarily in `sail/host/*.sail`, `sail/primitives/crypto.sail`, and
`sail/host/{code,nodes}.sail`. Proof targets see bodyless operations; executable targets
link the symbols from `ffi/`. There is one Sail specification and no separate
`spec` / `build` interface menu.

Most calls use scalar generated-C types. Three translation units deliberately
handle generated aggregate layouts and are compiled against each build's
generated model header:

| Glue | Generated Sail values | Role |
| --- | --- | --- |
| `ffi/journal_glue.c` | `JEntry`, state rows, and options | Encode/decode structured state and journal values against scalar C stores. |
| `ffi/hash_glue.c` | `list(Bytes)`, `ByteSlice`, topics, and `list(LogEntry)` | Hash or compare a whole segmented byte sequence in one call and translate log records. |
| `ffi/code_glue.c` | `JumpdestBitmap = list(bits(64))`, `option(Code)` | Flatten a Sail-built bitmap for insertion and construct aggregate lookup results. |

No glue file hand-mirrors a generated union, struct, or list layout.

## Contract Groups

| Group | Principal Sail externs | Proof contract |
| --- | --- | --- |
| Private input oracle | `ssz_src_len`, `ssz_src_byte` | Both operations view one bounded byte vector; out-of-range byte reads are zero. Endian helpers are pure Sail functions over these bytes. |
| Output trace | `emit_out` | Emitting one byte appends exactly that byte and preserves byte well-formedness. |
| Segmented hashing | `keccak256_segments`, `sha256_segments` | `BytesList` denotes its list and `BytesSlice` denotes `read_byte_slice(slice)`; a segment list denotes concatenation in order. The host digest equals the corresponding hash of that concatenation. |
| Generic byte slices | `slice_byte_at`, `slice_count_nonzero`, `slice_load_word`, `slice_load_n_word`, `slice_copy_to_memory`, `bytes_segments_equal_slice` | All reads, counting, and segmented equality refine one `ByteSlice` model. A slice has a source, base offset, and length; reads past its length are zero. Word loads are big-endian and copies zero-fill their out-of-bounds suffix. |
| Executor scratch arena | `scratch_store_bytes`, `scratch_store_slice`, `scratch_truncate` | Sail owns the bump cursor. Stores append exactly at that cursor and advance it only after success. Truncation changes only the logical top: bytes below the new top are preserved and bytes above it need not be cleared. A `ScratchSource` slice remains valid until its span is rewound. |
| Memory slices | `mem_establish_absolute` | A slice minted by `host/memory.sail` remains stable while its caller frame is suspended. |
| Code indexing and storage | `code_db_store`, `code_db_lookup`, `jumpdest_ref_contains` | `host/code.sail` computes the PUSH-aware bitmap. Insertion stores the exact bytes under `keccak256(bytes)` and associates that hash with the exact materialized bitmap. Lookup returns one `Code` pairing its code span and packed-table reference. C stores and queries the table but never analyzes instructions. |
| Memory, output arena, and stack | `mem_*`, `output_buffer_*`, `stack_*` | Memory has read-after-write and disjoint-write laws; memory and operand-stack frames are LIFO; the single output arena preserves a returned slice after its producing memory frame is released; copy/move operations refine byte-list operations; the stack has EVM LIFO/peek/set behavior. Sail owns the EVM returndata register. |
| Crypto and precompiles | `accelerator_*`, `secp256k1_verify`, `precompile_ecrecover_hash_sig` | Source-backed precompile execution equals execution over the slice's materialized bytes. Typed outputs carry explicit success information and remain within their declared widths. |
| World state and journal | `transient_*`, `storage_*`, `acct_*`, `warm_*`, `journal_*`, and logs | Account and storage updates refine layered maps with independent undo-log checkpoints; the remaining journal refines ordered transient/warm/log/refund effects. Reverting restores all three cursors, while successful child completion discards only the checkpoint token. |
| Witness and MPT DB | `nodedb_*` | The node table refines authenticated witness spans. From an explicitly supplied authenticated root, every lazy lookup is a hash-chain traversal through `keccak(node)`-keyed entries and fails closed when required material is absent. |

## Byte-Slice Law

`ByteSource` currently has these cases: `StatelessInputSource`, `MemorySource`,
`CodeSource`, `LogDataSource`, `MemoryArenaSource`, `OutputSource`, and
`ScratchSource`.
`byte_source_kind` assigns explicit host ABI tags; generated enum ordinals are
not part of that ABI.

For a well-formed slice `s = (source, off, len)`:

- `read_byte_slice(s)` has exactly `len` bytes;
- byte `i < len` is source byte `off + i`;
- the generic EVM readers return zero when `i >= len`;
- `slice_load` reads 32 bytes big-endian with zero padding;
- `slice_load_n` reads its requested big-endian prefix and right-aligns it in
  an EVM word; and
- `slice_copy` copies the available prefix and zero-fills the rest.

This one law covers calldata and executable code. They are distinct semantic
roles in a frame, but not distinct storage or reader interfaces.

For `ScratchSource`, well-formedness additionally requires that the slice end
does not exceed the Sail-owned scratch cursor. Rewind/reset lowers that cursor
without zeroing or freeing the retained host allocation.

## Code Law

`analyze_code(code)` is a pure Sail pass whose bitmap bit `i` is set exactly
when:

1. `0 <= i < code.len`;
2. `code[i] = 0x5b` (`JUMPDEST`); and
3. `i` is an opcode boundary when scanning from byte zero and skipping the
   immediate bytes of `PUSH1` through `PUSH32`.

The list has `ceil(code.len / 64)` words. Bit `i mod 64` of word
`floor(i / 64)` describes byte `i`.

If `code_db_insert(code)` returns `h`, then:

- `h = keccak256(read_byte_slice(code))`;
- the code DB entry for `h` denotes exactly those bytes;
- its `JumpdestRef` denotes exactly the bitmap computed during that pass; and
- `jumpdest_ref_contains(ref, code_len, i)` is true exactly for an in-range set
  bit in that table.

The `Code` value is the invariant carrier: callers install and
save/restore its byte slice and table reference together. The interpreter
does not reconstruct, rebind, or independently synchronize either half.

## Store Laws

Mutable host stores use the usual refinement laws:

- **read-after-write** — reading a just-written key/span returns the value;
- **disjointness** — a write does not change disjoint keys/spans;
- **default** — unwritten linear bytes read as zero where the EVM requires it;
- **length/bounds** — successful reads and copies respect their declared
  lengths; and
- **frame discipline** — enter/leave and checkpoint/revert operations are
  LIFO and restore the parent view.

Layered account and storage stores additionally need cache/update and
checkpoint/revert laws. A frame checkpoint is the product of their two undo
cursors and the general journal length; successful child completion requires no
host-side compaction. The code store is content-addressed, deduplicated, and
append-only, so frame rollback never removes code or JUMPDEST tables.

## Proof Order

1. Instantiate the private input and output-trace contracts and prove the
   inline `decode_stateless_input_ref` layout checks plus serialization facts.
2. Prove the generic `ByteSlice` reader and segmented-hash refinements.
3. Prove `code_db_insert`'s PUSH-skip bitmap theorem, then the code-DB insertion,
   lookup, and `JumpdestRef` refinement.
4. Add memory/returndata/stack and source-backed precompile contracts.
5. Add world-state, journal, and witness/MPT contracts for transaction and
   state-root theorems.

The source and RISC-V/Islaris proofs should instantiate the same contracts:
generated Coq establishes EVM behavior under this interface, while the runtime
proof establishes that the linked host implementation realizes it.
