# zkVM Optimisation Backlog

This note records performance work separately from the readable Sail
specification. Optimisations should preserve exact output bytes, remain valid
for both standard and optimised native builds, and avoid introducing
backend-specific model structure unless profiling demonstrates that it is
necessary.

The primary comparison metric is ZisK step count. Wall-clock timings are useful
for development feedback, but are not the acceptance criterion.

## Current Baseline

On the five-case Amsterdam workload used on 2026-07-27, three repeated
uninstrumented production runs produced identical step counts:

| Guest | Total steps | Relative to Reth |
| --- | ---: | ---: |
| Reth | 2,615,733 | 1.00x |
| Ethrex | 5,988,191 | 2.29x |
| EVM Sail, before BAL iterator work | 29,934,461 | 11.44x |
| EVM Sail, before HTR padding work | 29,533,176 | 11.29x |
| EVM Sail, after implicit-zero skipping | 27,202,698 | 10.40x |
| EVM Sail, whole-request HTR refinement | 20,550,344 | 7.86x |
| EVM Sail, raw state/BAL keys | 18,313,327 | 7.00x |
| EVM Sail, fixed-stack state-root refinement | 13,506,185 | 5.16x |
| EVM Sail, before C code indexing | 9,903,399 | 3.79x |
| EVM Sail, C code indexing | 8,553,870 | 3.27x |
| EVM Sail, whole C state access | 7,396,499 | 2.83x |
| EVM Sail, explicit optimized result unions | 7,467,773 | 2.85x |
| EVM Sail, compact active indexes and direct root view | 7,480,593 | 2.86x |
| EVM Sail, high-level C interpreter | 6,285,821 | 2.40x |
| EVM Sail, raw-opcode leaf dispatch | 6,097,003 | 2.33x |
| EVM Sail, direct word/fixed-byte conversions | 5,806,021 | 2.22x |

The implicit-zero change removes 2,330,478 steps (7.89%) from the immediately
preceding EVM Sail build. Replacing the whole new-payload HTR operation in the
optimized C splice then removes another 6,652,354 steps (24.46%). Keying the
native state and BAL tables directly by address and slot subsequently reduced
the guest to 18,313,327 steps.

The fixed-stack state-root refinement removes a further 4,807,142 steps
(26.25%) from that immediately preceding build.

On the same workload, moving complete witness-code indexing to C then removed
1,349,529 steps (13.63%), from 9,903,399 to 8,553,870. Moving the complete
`k_aload`, `k_sload`, `k_sstore`, `store_account`, and `store_account_info`
optimized paths to C removed another 1,157,371 steps (13.53%). Together these
two changes remove 2,506,900 steps (25.31%) from the pre-code-index build. The
explicit result-union refinement then adds 71,274 steps (0.96%) while removing
the optimized backend's direct mutation of generated Sail exception state:
fallible C operations now return `Optimized*Result`, and the Sail wrapper
performs the protocol `throw`.

The subsequent shared-table implementation initially regressed the five-case
workload to 7,703,331 steps. One-probe reads, direct merge worklists, compact
active-row indexes, and direct state-root consumption recovered that regression
and retained the high-cardinality improvement. Replacing the optimized
interpreter at the complete fetch/decode/frame-loop boundary then removed
1,194,772 steps (15.97%), taking the guest from 7,480,593 to 6,285,821. The
subsequent raw-opcode leaf dispatch removed 188,818 steps, and direct
word/fixed-byte conversions removed another 290,982 steps (4.77%). The current
guest is 2.22x Reth and 3.04% cheaper than Ethrex. The current production
comparison report is
`.agent-tmp/zisk-benchmark-word-bytes-fresh/report.md`.

An earlier experiment lowered only the scalar HTR helpers across the Sail/C
boundary. It regressed the workload to 29,930,468 steps, 10.03% above the
implicit-zero build, because it preserved the generated aggregate and
ownership overhead around the calls. This motivated moving the refinement to
the semantically meaningful whole-operation boundary.

The latest profile-enabled guest predates the raw-opcode leaf-dispatch and
direct word/fixed-byte refinements. It executes 6,295,066 steps, 9,245 more
than its corresponding 6,285,821-step uninstrumented guest. Its scoped report is
`.agent-tmp/zisk-profile-scopes-interpreter-c/report.md`. The SDK profiler
still reports a call-stack mismatch, so explicit nested scope tags remain the
source of truth for attribution.

Across the five cases, the important tagged scopes are:

| Scope | Steps | Share of tagged stateless validation |
| --- | ---: | ---: |
| Stateless validation | 5,948,006 | 100.0% |
| Execute block | 3,202,995 | 53.8% |
| Block transactions | 1,552,992 | 26.1% |
| Block-end request system calls | 1,235,913 | 20.8% |
| Validate result | 1,189,958 | 20.0% |
| System-call interpreter | 1,171,190 | 19.7% |
| Transaction frame execution | 780,797 | 13.1% |
| Block-access-list validation | 712,563 | 12.0% |
| Decode input | 579,715 | 9.7% |
| Validate payload | 578,868 | 9.7% |
| State root | 453,365 | 7.6% |
| Block start | 405,975 | 6.8% |
| Index witness | 306,235 | 5.1% |
| Compute output HTR | 209,045 | 3.5% |
| Transaction merge | 73,583 | 1.2% |

Scope tags nest and therefore must not be summed. In particular, the four
request-builder scopes are children of block-end request execution, and
block-access-list validation and state-root construction are children of
result validation.

The high-level interpreter refinement removed 1,194,747 tagged stateless
validation steps: approximately 964,910 from block-end request execution,
137,712 from transaction-frame execution, and 92,300 from block start. The
saving is therefore attributable to the intended boundary rather than to an
unrelated table or root change.

The largest remaining interpreter opportunity is the fixed system-call
workload: `system_call_interpret` costs 1,171,190 steps across the five cases,
about 234,000 per block. Result validation is now equally material at 1,189,958
steps, including 712,563 for BAL validation and 453,365 for the state root.
These are EVM-Sail internal scopes rather than matched Reth scopes, so they are
opportunity bounds rather than proof that every tagged step is avoidable.

A retained high-cardinality Amsterdam fixture provides a more realistic stress
case for the BAL backend:
`block_2d_gas_valid_when_cumulative_exceeds_limit.json` contains 2,047 account
rows, 3,086 account/slot items, and 4,082 change records. The current
uninstrumented EVM Sail guest executes 211,014,796 steps versus Reth's
79,672,575 (2.65x), improving the pre-interpreter result of 217,939,203 by
6,924,407 steps (3.18%);
both outputs are exact. Ethrex's current guest exhausts its ZisK bump-allocator
heap on this input, so no Ethrex ratio is reported. In the EVM Sail profile,
the earlier pre-shared-table build attributed 28,033,023 steps to BAL
validation and 9,223,926 to transaction merge. Those scope values are retained
as historical evidence only; the current compact-index build has not yet been
re-profiled on this large fixture.

BAL construction is not yet isolated by a scope: account touches and storage
reads occur inside whole optimized state-access calls, while change recording
occurs inside transaction merge. The current implementation no longer
maintains sorted BAL vectors on insertion. It activates each shared state row
once per BAL epoch, appends compact row indexes and change histories, overwrites
same-index history tails in place, and sorts only active row indexes in
`bal_prepare_iter`.

### Shared-table lookup recovery

Combining the transaction and cumulative projections into one physical account
table and one physical storage table made optimized C `tx_merge` substantially
cheaper, but the first whole-program result is mixed:

| Workload | Before shared tables | Shared tables + C merge | Change |
| --- | ---: | ---: | ---: |
| Five-case Amsterdam | 7,467,773 | 7,703,331 | +3.15% |
| High-cardinality BAL | 231,194,171 | 228,085,975 | -1.34% |

The merge scope itself improves by 34.93% on the five-case workload and 60.23%
on the high-cardinality fixture. The regression is therefore outside merge.
The first confirmed cause is redundant semantic-layer lookup: optimized
`k_aload` calls the transaction probe and then the cumulative probe, and
optimized `k_sload` does the same. Both probes search the same physical table;
the storage point comparator and bucket hash deliberately ignore the
storage-clear generation because generation is row metadata, not identity.

The implemented recovery stages are:

1. One-probe optimized account and storage reads select transaction,
   cumulative, cleared-generation, or miss state after one table search.
2. Transaction merge and cache propagation consume stable row indexes from
   their worklists instead of rediscovering rows through public lookup paths.
3. Compact active-row indexes name only live account/storage updates.
4. The optimized state-root operation consumes the indexed active view
   directly, with canonical ordering performed only at the root/iteration
   boundary.

The final five-case count is 7,480,593, recovering 222,738 of the initial
shared-table regression. The high-cardinality count fell from 231,194,171
before shared tables to 217,939,203, a 5.73% improvement. Transaction merge is
now only 73,583 tagged steps on the five-case profile, so changing the hash
mixer, introducing per-account slot tables, or further restructuring active
rows is deferred until a new profile shows table lookup as material.

### High-level C interpreter refinement

The readable interpreter in `sail/evm/interpreter.sail` remains the normative
implementation for standard native builds and extraction. The optimized splice
replaces `interpret` as one semantically complete operation. Its C
implementation owns the active-frame loop, code-source resolution, opcode
fetch/decode, PUSH-immediate assembly, and continuation dispatch. It calls the
generated Sail opcode-family handlers for instruction semantics, gas
accounting, state access, and protocol errors, so it does not duplicate the EVM
instruction definitions.

The active code source is resolved once per frame instead of once per fetched
byte. PUSH values are assembled directly into the optimized fixed-width word,
and the existing C-private continuation stack is reused. The generated handler
ABI still receives one instruction aggregate per dispatched opcode; removing
that final aggregate would require either scalar entry points for every handler
or a second C implementation of opcode semantics, neither of which is justified
by the current result.

This boundary removed 1,194,772 production ZisK steps (15.97%) on the five-case
workload and improved every individual case by 216,000 to 305,000 steps. EVM
Sail now also beats Ethrex on the recursive-call case: 1,837,744 versus
2,137,764 steps. The experimental custom-compiler AST copy/match changes were
reverted; this optimization is explicit in the optimized guest and does not
alter general Sail C-backend behavior.

### Direct word/fixed-byte conversions

The standard Sail model uses direct canonical-byte concatenation and fixed
slices for
`hash_to_word`, `word_to_hash`, `hash_lt`, `word_to_address`, and
`address_to_word`. This exposes the canonical big-endian protocol-byte
interpretation clearly to standard native builds and proof extraction without
allocating a little-endian temporary vector or executing a reversal loop.

The optimized C splice replaces those complete conversion functions with
header-only operations over the optimized representations: canonical fixed
bytes and least-significant-limb-first `sail_u256`. The implementation performs
direct big-endian loads, stores, and comparisons. It does not allocate or
initialize temporary vectors and does not execute byte-reversal loops. This
whole-function boundary also avoids the generated vector ownership and indexed
update machinery around the otherwise simple equations.

On the five-case workload, the change reduced the guest from 6,097,003 to
5,806,021 steps, saving 290,982 steps (4.77%). Every case improved, by 49,520
to 87,992 steps. The retained ELF produces byte-exact output and is 182,170
steps (3.04%) cheaper than the staged Ethrex guest on this workload.

### Fixed-stack state-root refinement

The readable Sail specification remains the normative state-root equation. The
optimized build alone replaces `compute_state_root` at the whole-operation
boundary and links `ffi/mpt_glue.c`, which consumes the same account,
storage, and authenticated witness-node host contracts.

The implementation uses one statically allocated workspace containing at most
64 branch frames, with 16 child references stored inline in each frame. It
first computes and caches the post-storage root for every changed account,
resetting and reusing that workspace between accounts. It then resets and
reuses the same workspace for the account/state trie. Storage and state trie
construction are sequential, so this avoids duplicate stacks without changing
semantics or introducing shared mutable traversal state between simultaneous
operations. No generated Sail list/vector aggregate or per-frame heap
allocation crosses this optimized-only boundary.

### Matched HTR scope

A profile-enabled comparison against `ere-guests` commit `9d3d4b9` measures
only the new-payload-request HTR operation after input decoding. EVM Sail's
`compute_output_root` tag wraps the call on the validated `StatelessInputRef`;
Reth and Ethrex's `new_payload_request_root` tag wraps the corresponding
operation on their decoded Rust input. All three guests produced the exact
expected output for all five inputs.

| Guest | Five-case HTR steps | Mean per case | Relative to Reth |
| --- | ---: | ---: | ---: |
| Reth | 320,606 | 64,121 | 1.00x |
| Ethrex | 336,177 | 67,235 | 1.05x |
| EVM Sail, optimized | 473,297 | 94,659 | 1.48x |

EVM Sail's HTR section is therefore 47.6% more expensive than Reth's and 40.8%
more expensive than Ethrex's on this workload. Its per-case ratio ranges from
1.42x to 1.50x Reth, so the remaining overhead is consistent across the
different payload shapes rather than isolated to one fixture.

## Ranked Work

1. **Block-end request execution.**
   The generic interpreter refinement reduced this scope from 2,200,823 to
   1,235,913 steps, of which 1,171,190 remain in the interpreter. The next
   candidate is a whole checked-system-call operation, or exact-code-hash-gated
   specialization with the normal interpreter as a fail-closed fallback.
   Address alone is not a sufficient fast-path condition.
2. **Result and BAL validation.**
   Result validation costs 1,189,958 steps, of which BAL validation accounts
   for 712,563 and the already optimized state root for 453,365. The next BAL
   experiment should refine `validate_block_access_list` at the whole-operation
   boundary: parse canonical RLP and compare directly against the C-private
   recorder, return an explicit `OptimizedUnitResult`, and let the Sail wrapper
   throw. The standard Sail decoder and iterator remain normative.
3. **Input decoding and payload validation.**
   `decode_input` and `validate_payload` each cost about 579,000 tagged steps.
   Split SSZ offset/list validation from aggregate construction and separate
   payload-header checks from root calculations before choosing a lowering
   boundary.
4. **Generic interpreter and frame execution — complete, monitor only.**
   The high-level optimized C loop is in place and removed 15.97% of the
   complete workload. It retains generated handlers for instruction semantics.
   Reopen this boundary only if a representative opcode-heavy workload shows
   the remaining per-opcode aggregate ABI to be material.
5. **HTR specialization — complete, monitor only.**
   The optimized whole-request implementation preserves the validated
   `StatelessInputRef` boundary and the standard model's explicit SSZ equation.
   A raw-pointer experiment saved only 1,835 whole-program steps and was
   reverted. The latest `compute_output_root` scope is 209,045 steps, so its
   remaining matched overhead is not the largest opportunity.
6. **State access, transaction merge, and state-root construction — complete.**
   Whole account/storage access, direct C merge, compact active-row indexes,
   and the reusable fixed-stack root implementation are in place. Transaction
   merge is now 73,583 tagged steps; further table topology work is not
   justified by the current five-case profile.

## Next Optimization Program

### Outcome and constraints

The next milestone is to move the optimized guest below the current Ethrex
baseline of 5,988,191 steps, then re-profile against Reth. The standard Sail
build remains the normative readable specification. Optimized implementations
may replace a semantically complete operation in the C splice, but must not
leak C-specific representations or control flow into the standard model.

Each experiment changes one attributable boundary and produces a new
uninstrumented ELF. Retain it only when it:

- preserves exact output and failure behavior in focused native tests;
- saves at least 1% of the five-case total, or at least 5% on the targeted
  recursive/high-cardinality workload without regressing the five-case total;
- does not regress either comparison workload by more than measurement noise;
- has a clear path through both complete native fixture suites; and
- does not rely on an address-only protocol fast path.

Smaller changes may be retained only when they are required infrastructure for
a measured larger optimization. Re-baseline after every retained stage rather
than stacking unmeasured changes.

### Phase 0: make the remaining costs attributable

1. Add profile-only subdivisions without changing production behavior:
   - block-start and checked-system-call code lookup, frame preparation,
     output capture, revert, and merge;
   - BAL iterator preparation, RLP traversal, value decoding, and comparison;
   - header/chain-config decoding;
   - transaction root, withdrawal root, requests hash, BAL hash, and header
     hash inside payload validation.
2. Do not place ZisK start/end markers around every opcode: that would distort
   the interpreter being measured. Instead add profile-only counters for
   opcode count, PUSH-immediate loads, frame pushes/resumes, memory growth,
   state accesses, and byte-source resolutions.
3. Investigate the profiler's call-stack mismatch with a diagnostic-only build:
   test frame pointers and disabled sibling-call optimization, then identify
   hand-written assembly or nonstandard return edges if the mismatch remains.
   Do not change the production ABI merely to satisfy the profiler; explicit
   scopes remain the fallback source of truth.
4. Preserve four workload views:
   - the five-case Amsterdam comparison;
   - the recursive-call case;
   - the high-cardinality BAL case;
   - focused blocks with empty and non-empty request queues and high log count.

Exit this phase with a checked-in scope/counter map and one script-generated
summary that compares every later ELF to the same production baseline.

### Phase 1: block system calls

Treat block-start and block-end calls separately because their profiles have
different shapes.

1. **Block-end requests.** Confirm the current result that
   `system_call_checked` spends almost all of its current 1,235,913 steps in
   `interpret`, while merge is negligible. Record the exact canonical code
   hashes and opcode/state-access counts for withdrawal, consolidation,
   builder-deposit, and builder-exit contracts.
2. **Complete:** the generic optimized fetch/dispatch path resolves the code
   source once, reads opcode/PUSH bytes without repeated `ByteSlice` source
   dispatch, and preserves the generated Sail execution handlers. It improves
   both request-contract and transaction execution.
3. If generic fetch still leaves the fixed contracts dominant, prototype one
   whole-operation C implementation for one canonical request contract. Gate
   it on the exact authenticated code hash, use the existing journal, state,
   BAL, scratch, and result-union contracts, and fall back to the ordinary
   interpreter for every other hash. Verify missing code, modified code,
   empty/non-empty queue, success, revert, output, storage, and BAL behavior.
4. Extend the code-hash-gated path to the remaining request contracts only
   after the first contract produces a material whole-program win.
5. **Block-start calls.** Use the new subdivisions to distinguish authenticated
   code lookup, 32-byte calldata preparation, interpreter work, and state
   writes. Optimize the measured dominant complete operation; do not assume
   the block-end solution applies.

The target for this phase is a 500,000-step or larger five-case reduction.
That is deliberately below the 2.20-million-step upper bound: protocol state
access and output production remain real work.

### Phase 2: generic interpreter and frame execution

The high-level C interpreter completed the generic fetch/decode and
continuation-loop part of this phase. The remaining items are diagnostic
follow-ups rather than the next default optimization:

1. Re-profile transaction execution after any system-call specialization so
   fixed block overhead no longer obscures the transaction slope.
2. Split targeted benchmarks by opcode family, PUSH density, memory copying,
   storage access, precompile use, and call depth. Use the recursive fixture as
   the frame-continuation acceptance case.
3. Review the generated fetch/decode path before duplicating the EVM in C:
   compare a cached compact code index, an optimized C `fetch` returning the
   existing AST, and compiler specialization of the opcode match. Choose the
   smallest boundary that removes repeated decode/source work.
4. Measure frame pushes and resumes before changing `frame_stack_glue.c`. The
   optimized ABI already uses one lazily allocated fixed-capacity array and
   structure assignment; it does not perform a heap allocation per frame.
   Change continuation storage only if counters show copying is material.
5. Attribute `slice_load_n_word_source`, `evmsail_resolve_byte_source`,
   `memcpy`, and `copy_zast` to opcode fetch, calldata, memory, hashing, or
   receipts. Cache a resolved code pointer for the active frame or construct
   words directly from the resolved bytes only where source lifetime permits.

Retain generic interpreter changes only when they improve the five-case and
recursive workloads. A request-contract-only improvement belongs in Phase 1,
not in the generic interpreter.

### Phase 3: result and BAL validation

1. Re-profile the current compact-index build on the high-cardinality fixture;
   the retained 28-million-step BAL figure predates the latest storage model.
2. Implement an optimized whole-operation
   `validate_block_access_list` candidate in C. It should:
   - drive canonical RLP cursors directly from the input slice;
   - compare against the already prepared C-private active-row/history view;
   - derive list bounds from canonical RLP lengths and enforce the protocol
     gas-derived maximum;
   - avoid generated recursive lists and aggregate option/union copies; and
   - return `OptimizedUnitResult`, leaving the Sail wrapper to throw.
3. Keep the existing Sail RLP decoder and `bal_iter_next` algorithm unchanged
   as the standard specification and differential oracle.
4. Compare empty, duplicate-invalid, ordering-invalid, maximum-bound, and
   high-cardinality BAL fixtures before running the complete corpus.

State-root work remains closed unless this phase shows that active-row
preparation shared with the root is again material.

### Phase 4: input decoding and payload validation

1. Split the roughly 579,000-step decode scope into fixed header extraction,
   256-byte logs-bloom extraction, chain-config validation, and kernel install.
   The full profile's `ssz_logs_bloom` hotspot makes bulk fixed-byte extraction
   the first candidate, either as a general compiler fixed-vector
   specialization or a whole header-decoding optimized boundary.
2. Split payload validation into its existing commitments. Transaction,
   withdrawal, receipt, and state roots should continue to share the optimized
   MPT machinery rather than grow separate implementations.
3. If fixed-field decoding dominates, add a validated-reference C operation
   that reads fields directly from `StatelessInputRef` and returns an explicit
   decoded-header result. Keep all offset, length, and range checks equivalent
   to the Sail decoder.
4. If root/header assembly dominates, lower the whole payload-validation
   operation instead of adding many scalar C calls. Preserve exact exception
   ordering and fall back to the readable Sail implementation in the standard
   build.

The current 209,045-step output HTR scope is monitor-only. Reopen HTR only if
these measurements show shared padding or source resolution dominating another
phase.

### Phase 5: allocations, copies, and logs bloom

This is a cross-cutting cleanup after the semantic phases, not an invitation to
optimize every `malloc` or `memcpy`.

1. Add profile-only allocation counters by call site: call count, requested
   bytes, peak live bytes, and reset/reuse frequency.
2. Map the full-profile `malloc`, `memcpy`, aggregate-copy, byte-source, and
   logs-bloom PCs back to the scopes above.
3. Reuse scratch/arena storage only when lifetime is already bounded by a
   transaction, frame, or block operation. Prefer bulk fixed-byte operations
   over generated element loops.
4. Keep the C ABI ownership rules intact for the standard GMP build. Apply
   POD moves, pointer views, or C-private arenas only to optimized
   representations whose ownership is explicit.
5. Remove an allocation/copy path only when its enclosing scope and complete
   workload both improve; reject local instruction-count wins that move cost
   into lookup, conversion, or finalization.

### Validation and handoff after every phase

For exploratory variants, run focused standard and optimized native fixtures,
then build an unprofiled ZisK ELF and measure the relevant workload. For every
retained phase:

1. run `make check`;
2. run the complete retained 26,104-fixture corpus on standard and optimized
   native builds;
3. rebuild with `EVM_PROFILE=off EVM_DEBUG=off`;
4. stage the ELF under `tools/zisk-guests/`;
5. run one warmup and three measured repetitions against the same Reth and
   Ethrex ELFs;
6. run the recursive and high-cardinality guards; and
7. update this note with before/after steps, ELF hash, correctness result, and
   the next measured bottleneck.

Lean and Coq extraction repair remains deferred during this performance
program. Do not change the standard Sail semantics or host-axiom boundary in a
way that makes later proof extraction less direct.

## Validation Gate

Each optimisation must pass:

1. Standard and optimised native builds.
2. Byte-exact fixture comparison against EELS, with the complete retained
   `tests-zkevm` v0.6.2 corpus before landing.
3. A rebuilt, uninstrumented production ZisK ELF.
4. Repeated step-count comparison on the same workload against the saved Reth
   and Ethrex ELFs.

Profile-enabled ELFs are diagnostic only and must not be used for the final
step-count comparison.

The implicit-zero skipping, whole-request HTR, raw state/BAL key,
fixed-stack state-root, code-indexing, whole state-access, explicit optimized
result unions, shared state tables, direct C merge, compact active indexes, and
direct optimized root view passed all 26,104 retained fixtures byte-exact in
both standard and optimised native builds on 2026-07-27. The high-level C
interpreter override, raw-opcode leaf dispatch, and direct word/fixed-byte
conversion refinement subsequently passed the same 26,104/26,104 standard and
26,104/26,104 optimized runs. The standard build retains the Sail interpreter
and direct endian equations; only the optimized build links
`interpreter_glue.c` and injects `word_bytes_glue.h`.
Inspection confirmed that the standard generated C retains the explicit Sail
HTR and state-root implementations and does not link either optimized glue
object. The optimized generated C makes one replacement call at each
whole-operation boundary.

The current production ZisK ELF was rebuilt with `EVM_PROFILE=off` and
`EVM_DEBUG=off` and passed the six-case ZisK byte-exact fixture. It was staged
at `tools/zisk-guests/stateless-validator-evm-sail-zisk.elf` and produced exact
outputs for both the five-case comparison against the saved Reth and Ethrex
ELFs and the high-cardinality comparison against Reth.
Its SHA-256 is
`505096c9438a678cb29771c8ab5ceef1db42add5fb4f94d0a6dea91fad2c15c3`.
The comparison used the host's default Rayon thread count; thread count does
not affect the deterministic ZisK step count.
