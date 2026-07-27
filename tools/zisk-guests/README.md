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
