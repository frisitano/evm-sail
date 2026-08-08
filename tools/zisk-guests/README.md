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
and every executed ELF symbol with its exclusive instruction steps. Reth and
ethrex therefore participate in shared total and operation comparisons without
being assigned zero values for EVM Sail-only semantic tags. A ZisK call-stack
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
