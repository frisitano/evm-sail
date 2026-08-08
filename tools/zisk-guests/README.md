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

## Aligned semantic scopes in the reth and ethrex guests

The staged reth and ethrex guests are built with ZisK profile-tag markers
whose phase names align with the evm-sail `EVM_PROFILE` cycle scopes, so the
same tag means the same work in all three guests:

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
most the run's total steps. evm-sail additionally emits its finer-grained
tags (and the coarser `validate_payload`/`validate_result` stage tags) around
and inside these phases; the five phase names above are the cross-guest
comparison surface.

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
