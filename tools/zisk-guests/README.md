# Local ZisK benchmark guests

This directory is the stable repository-local home for the three guest ELFs
used by `tools/benchmark_zisk.py`:

- `stateless-validator-evm-sail-zisk.elf`
- `stateless-validator-reth-zisk.elf`
- `stateless-validator-ethrex-zisk.elf`

Named experimental variants may be kept alongside them without replacing the
staged baseline; for example,
`stateless-validator-evm-sail-zisk-c-lto.elf`.

The ELFs and generated `SHA256SUMS` are machine-local build artifacts and are
ignored by Git. Stage freshly built guests with:

```sh
bash tools/stage_zisk_guests.sh \
  path/to/stateless-validator-evm-sail-zisk.elf \
  path/to/stateless-validator-reth-zisk \
  path/to/stateless-validator-ethrex-zisk
```

Once staged, the standard comparison needs no `--guest` arguments:

```sh
python3 tools/benchmark_zisk.py path/to/fixture.json
```

The runner validates every public output before recording a step count.

## Build provenance

The runner records what each guest ELF was built from into `results.json`
(`guests.{name}.build`) and the dashboard catalog. Provide it either as an
explicit argument (`--guest-build reth="reth v1.11.0"`) or as a sidecar next
to the staged ELF, written when the guest is built:

```sh
echo '{"version": "reth v1.11.0", "commit": "<reth commit>"}' \
  > tools/zisk-guests/stateless-validator-reth-zisk.elf.build.json
```

The `evm-sail` guest defaults to the repository HEAD commit (suffixed
`-dirty` for local changes). Unknown provenance is recorded as `null` and the
dashboard shows it as unknown.

## MkDocs scope dashboard

The dashboard at `book/docs/performance.md` needs an EVM Sail guest built with
ZisK profile tags enabled. Keep this ELF separate from the uninstrumented
benchmark baseline:

```sh
EVM_PROFILE=on \
ZKVM_BUILD=.agent-tmp/build-zisk-profile \
bash zkvm/zisk/build.sh guest
```

Generate the comparative retained-corpus dashboard dataset with two exact,
output-checked emulator invocations per guest and embedded block: an SDK pass
for semantic tags and a full statistics/disassembly pass for comprehensive
operation and executed-function data.

```sh
python3 tools/benchmark_zisk.py \
  --guest evm-sail=.agent-tmp/build-zisk-profile/stateless-validator-evm-sail-zisk.elf \
  --guest reth=tools/zisk-guests/stateless-validator-reth-zisk.elf \
  --guest ethrex=tools/zisk-guests/stateless-validator-ethrex-zisk.elf \
  --baseline reth \
  --profile sdk \
  --dashboard-only \
  --dashboard-dir book/docs/assets/generated/performance \
  --output-dir .agent-tmp/zisk-dashboard-run \
  zkvm/.fixtures/current-v062-full

make docs-site
```

`--dashboard-only` uses the SDK pass as the benchmark measurement and parses
total cost, steps, and any inclusive semantic scope tags from it. The
supplemental full pass supplies every costed ZisK operation used by the input
and every executed ELF symbol with its exclusive instruction steps.

## Semantic scopes in the reth and ethrex guests

The staged reth and ethrex guests are built with ZisK profile-tag markers
mirroring the evm-sail `EVM_PROFILE` cycle-scope vocabulary. There are three
tiers.

**Tier 1 — aligned phases.** The five phase tags keep their exact original
boundaries and remain the cross-guest comparison tier:

- `decode_input` — serialized input bytes to native block structures (SSZ
  deserialization plus payload-to-engine-type conversion).
- `index_witness` — witness parsing/indexing/authentication, including
  ancestor-header decoding and (for reth/ethrex) sparse-trie construction and
  pre-state-root verification.
- `execute_block` — transaction signature/public-key verification plus block
  execution (system calls and transactions), matching evm-sail, which decodes
  and recovers transactions inside its execution scope.
- `state_root` — post-execution state-root computation (for ethrex this also
  covers applying account updates to the trie, the work reth performs inside
  `calculate_state_root`).
- `receipts_root` — receipts/commitments validation: receipts root, logs
  bloom, requests hash, and block-access-list hash. evm-sail's
  `receipts_root` tag covers the receipts-trie reduction plus the
  receipts-root, logs-bloom, and block-access-list checks (the finer
  `block_access_list` tag nests inside it); its EIP-7685 request
  authentication is system-call driven and therefore reports under
  `execute_block`.

In all three guests the five tags are emitted as mutually non-overlapping
phases, so their inclusive step totals can be compared directly and sum to at
most the run's total steps. The five phase names above are the cross-guest
comparison surface.

**Tier 2 — aligned `sig_recovery`.** Verification of every transaction
signature against its supplied public key and derivation of the sender
addresses. In reth this is `recover_block_with_public_keys`; in ethrex it is
the guest program's public-key verification loop plus levm's
`get_transactions_with_sender`. Both sit inside `execute_block`. evm-sail
performs this work inside its `tx_decode` scope, so `sig_recovery` compares
against (a subset of) evm-sail's `tx_decode`.

**Tier 3 — per-guest drill-down.** Every remaining tag the evm-sail profile
overlay emits was mapped onto the semantically corresponding work in each
guest and wrapped with the identical tag name, regardless of its position in
that engine's call structure. Nesting and ordering do NOT align across
guests; comparisons are scope-by-scope inclusive totals. Work unique to one
engine carries engine-natural names instead.

ziskemu keeps a strict LIFO stack of open scopes keyed by tag name and
reports inclusive totals per tag: same-name sibling segments accumulate, and
nesting differently-named scopes does not change the enclosing tag's total.
Never nest a tag inside itself — its inner span would be double-counted.

### evm-sail tag mapping

Presence of each overlay-emitted evm-sail tag in the peer guests ("—" = no
separately measurable counterpart; the note says where that work is counted):

| evm-sail tag | meaning in evm-sail | reth | ethrex |
| --- | --- | --- | --- |
| `stateless_validation` | whole-guest validation | guest entrypoint | guest entrypoint |
| `decode_input` | input-decode phase | phase (unchanged) | phase (unchanged) |
| `index_witness` | witness-indexing phase | phase (unchanged) | phase (unchanged) |
| `validate_payload` | payload/header validation | ancestor-hash linkage + pre-execution consensus checks | versioned-hash check; witness block-hash init/validation; block-body and pre-execution header checks |
| `execute_block` | execution phase | phase (unchanged) | phase (unchanged) |
| `validate_result` | post-execution validation (gas, roots, bloom, BAL) | two segments: BAL checks + `validate_block_post_execution`, then the state-root check | two segments: gas/receipts/requests/BAL validation, then the final state-root check |
| `compute_output_root` | SSZ hash-tree-root of the `NewPayloadRequest` | guest `hash_tree_root` | guest `hash_tree_root` |
| `serialize_output` | building/committing the output | guest `to_ssz` | guest `to_ssz` |
| `block_start` | block-start system calls | — (inside pinned reth-evm, not vendored) | `prepare_block` (beacon root, block-hash history) |
| `block_transactions` | whole per-transaction loop | — (inside pinned reth-evm) | the tx loop in `LEVM::execute_block` |
| `tx_decode` | per-tx decode incl. public-key verification | — (decode under `decode_input`, recovery under `sig_recovery`) | — (same split) |
| `tx_reset` | per-tx state reset | — (inside revm) | per-tx env + root-frame setup |
| `tx_validate` | transaction validity checks | — | merged into `tx_upfront` (levm's prepare hooks validate and apply upfront effects in one pass) |
| `tx_upfront` | upfront transaction effects | — | `prepare_execution` |
| `tx_frame` | root call-frame interpretation | — | `run_execution` |
| `tx_settle` | transaction settlement | — | `finalize_execution` |
| `receipts_root` | receipts-root reduction + result checks | phase (unchanged) | phase (unchanged) |
| `block_end_state` | block-end state changes (withdrawals) | — (inside pinned reth-evm) | `process_withdrawals` |
| `block_end_requests` | execution-requests validation | — (inside pinned reth-evm/consensus) | `extract_all_requests_levm` |
| `state_root` | post-state-root phase | phase (unchanged) | phase (unchanged) |
| `block_access_list` | BAL validation | BAL item-limit check + BAL hash computation | BAL hash validation |
| `index_witness_nodes` | hashing/indexing witness trie nodes | node hashing in `SparseState::new` | node decode+hash in `into_execution_witness` |
| `index_witness_codes` | hashing/indexing witness bytecode | bytecode hashing in `SparseState::new` | code hashing in `GuestProgramState::from_witness` |
| `index_witness_headers` | decoding witness headers | ancestor-header decode | header decode + chain-linkage check (guest program and `from_witness`) |
| `request_withdrawals` | withdrawal-request system call | — | `read_withdrawal_requests` |
| `request_consolidations` | consolidation-request system call | — | `dequeue_consolidation_requests` |
| `request_builder_deposits` | builder-deposit-request system call | — | `read_builder_deposit_requests` |
| `request_builder_exits` | builder-exit-request system call | — | `dequeue_builder_exit_requests` |
| `tx_merge` | merging the tx write-set at epoch end | — | — (levm writes through its cache; no separate merge step) |
| `system_call_interpret` | running one system contract | — | the system-contract `VM::execute` in `generic_system_contract_levm` |
| `system_call_merge` | merging system-call effects | — | — (no separate merge step) |

The evm-sail runtime declares further tag ids (`htr_*`, `account_*`,
`storage_*`, `bal_*`, `account_mutation`, `storage_mutation`,
`tx_merge_accounts`, `tx_merge_storage`) that the current profile overlay
never emits; they are excluded from the mapping. The reth "—" rows exist
because that work happens inside the pinned reth-evm/revm/consensus crates,
which are not vendored; only `stateless`/`tries`/`zeth-mpt` and the guest
crate are instrumentable for reth.

Engine-natural drill-down scopes (not part of the evm-sail vocabulary):

- reth: `run_validation`, `deserialize_input`, `to_reth_input`,
  `new_payload_request_root`, `sparse_trie_build` (witness trie reveal in
  `SparseState::new`), `hashed_post_state` and `calculate_state_root` (inside
  `state_root`), `read_input`, `write_output`, `validate_chain_config`.
- ethrex: `run_validation`, `deserialize_input`, `to_ethrex_input`,
  `new_payload_request_root`, `witness_to_rpc`, `build_execution_witness`,
  `embed_witness_tries`, `hash_witness_tries`, `pre_state_root`, `execute_tx`
  (per-tx umbrella around the `tx_*` stages), `get_state_transitions`,
  `apply_account_updates`, `state_trie_root`, `receipts_bloom`,
  `requests_hash`, `read_input`, `write_output`, `validate_chain_config`.

System calls run through the same levm `VM::execute` as transactions but do
not emit the `tx_*` stage scopes (matching evm-sail, whose system calls
report only under `system_call_interpret`).

They are built from `.agent-tmp/ere-guests-scoped/`, a copy of the local
`~/dev/ethereum/ere-guests` checkout whose zisk bin workspaces `[patch]` the
pinned `stateless` and `ethrex` revisions with vendored copies carrying the
markers (`vendor/stateless/`, `vendor/ethrex/`). Rebuild either guest with:

```sh
cd .agent-tmp/ere-guests-scoped/bin/stateless-validator-reth/zisk  # or ethrex
RUSTUP_TOOLCHAIN=zisk RUSTFLAGS="-C passes=lower-atomic" \
  cargo build --release --target riscv64ima-zisk-zkvm-elf --features cycle-scope
```

The markers are profile syscalls interpreted only by the emulator; plain and
`--stats --sdk` runs produce byte-identical public outputs. An ELF containing
them cannot be proved by the ZisK ASM prover, so keep these staged guests for
emulator benchmarking and profiling only. A ZisK call-stack
mismatch prevents inclusive call attribution, but does not affect the complete
exclusive function inventory. Successful raw input, output, disassembly, and
text-report artifacts are discarded; `results.json`, the benchmark report, and
the compact comparative dashboard catalog/shards remain. The generated
dashboard data is ignored by Git and `make clean` removes it, so regenerate it
before publishing a fresh site.

For a quick UI check, add `--limit 1`. The browser loads only the fixture
catalog initially and fetches a selected fixture's case shard on demand. The
fixture is the dashboard's only selector; long fixture option lists require
search after the first 200 matches. If per-phase step attributions
(`measurements[].phases`, see the runner docstring) are present, the
dashboard adds a stacked per-phase view per guest.

An existing `results.json` can be re-exported without re-running any
emulator:

```sh
python3 tools/benchmark_zisk.py \
  --regenerate-dashboard .agent-tmp/zisk-dashboard-run/results.json \
  --dashboard-dir book/docs/assets/generated/performance
```
