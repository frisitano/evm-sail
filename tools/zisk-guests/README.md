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
catalog initially and fetches a selected fixture's case shard on demand.
Selectors cascade by corpus type, target, source fork, and feature/EIP; long
fixture and case option lists require search after the first 200 matches.
