# EL-IR EVM vs EEST state-test suite — autonomous run report

Goal: maximize EL-IR Sail-EVM pass rate against the EEST `state_tests` corpus
(`crates/revm-fork/test-fixtures/main/develop/state_tests/`). Latitude: refactor
freely when it unblocks a category; keep `make check` + `make fixtures` green;
commit each step; never push.

Harness: compile-once runner (`examples/eest/runner.sail`) reads fixtures from
stdin as an int-stream; `run_eest.py` encodes EEST tests + compares the dumped
post-state to the fixture's explicit `post[fork].state` (no keccak-MPT root).

## Timeline

- Harness built (compile-once, stdin int-stream). Fixed two bugs en route:
  per-tx `pc` reset; O(n^2) trace (`list_append` -> O(1) prepend).
- Phase 1 DONE: real Keccak-256 in Sail (`sail/keccak.sail`, validated against
  known vectors) + real CREATE/CREATE2 address derivation (keccak+RLP, validated).
  Self-contained (no FFI), compiles to the RISC-V target; swappable for the
  zkVM C-accelerator precompile later. Unblocks KECCAK256/EXTCODEHASH/CREATE*
  and keccak-derived storage slots (Solidity mappings/arrays).
- (in progress) Phase 2: keccak-enabled sweep across the corpus + triage.

## Pass-rate log

| stage | suite scope | passed / total | notes |
|---|---|---|---|
| harness gate | eip145 bitwise shift | **21 / 27** | SHL/SHR/SAR across Prague+Shanghai; 6 fails = older-fork divergence |
| post-keccak | ~194-file sample (all forks) | **238 / 715** | fails: balance 298 (gas/fork), storage 75, nonce 25; +79 timeouts (loops/precompiles) |

Biggest lever for pass rate = **balance** fails (gas accounting). To separate
fork divergence from real gas bugs, next sweep should pin one fork (Cancun).
Timeouts are loop/precompile-heavy tests (need a step cap + real ec precompiles).

### Block-authentication track (RLP / MPT / roots / block hash) — VALIDATED
All against real Ethereum data; all self-contained -> compile to the RISC-V target.

| piece | file | validation |
|---|---|---|
| RLP encoding | sail/rlp.sail | standard RLP vectors ✓ |
| Keccak-256 | sail/keccak.sail | keccak("")/("abc") vectors ✓ |
| CREATE/CREATE2 addr | sail/keccak.sail | known nonce-0/1 vectors ✓ |
| MPT trie root | sail/mpt.sail | (leaf/extension/branch + hex-prefix) |
| **post-state root** | sail/state_root.sail | **== real EEST post `hash` 3/3** (Cancun MCOPY) |
| **block hash** | sail/block_hash.sail | **== real genesis block hash** (keccak(rlp(header))) |
| pre/post-state auth | sail/state_root.sail `authenticate_state` | same validated state-root machinery |

- Perf caveat: state-root over storage-heavy states is slow (list state +
  keccak/node); made opt-in (`--root`) for the fast pass-rate sweep.
- Remaining refinements: tx/receipt roots (ordered trie over rlp(index)),
  post-1559/Shanghai/Cancun header fields, stateless MPT branch-proof verify.

Parallel: an independent worker (Agent Deck `fb65323c-1781198297`, branch
`worker/evm-asm-riscv-target`) is compiling the EVM to the eth-act zkVM RISC-V
standard target (`riscv64im_zicclsm-unknown-none-elf`). Progress: full bare-metal
setup under `el-ir/zkvm/` (link.ld, start.S, HTIF for spike, mini-gmp to drop the
libgmp dependency) and a built `derisk.elf`.

## Failure triage (Cancun-pinned, so these are real bugs not fork drift)

Balance fails = gas accounting. Inspecting deltas (GOT vs WANT sender balance):
- **G_newaccount (25000)** recurring under-charge — a CALL/SELFDESTRUCT-to-empty
  path isn't charging the 25000 new-account cost. SELFDESTRUCT in execute.sail
  charges G_selfdestruct + access but NOT G_newaccount when the beneficiary is
  dead — likely culprit. (Highest-frequency single bug.)
- **~48000 over-charge** in some cases — over-charging (double cold? value
  stipend accounting?). Needs a specific repro.
- **small 20/30/90 deltas** — per-op precision: warm/cold edges, memory-expansion
  rounding, keccak/copy word-count rounding.

### Gas instruction set (done) + precise delta checklist (`--gasdbg`)

Gas is now a small instruction set (`GasOp` + `gas_of` in sail/gas.sail) — one
auditable schedule, the structural fix for the long-tail (the revm insight: revm
centralizes gas into spec functions). `run_eest.py --gasdbg` prints the exact
gas delta + disassembled bytecode per balance fail. Cancun-pinned sample:
**91/246**. Delta distribution (the concrete long-tail, each a targeted fix):

| delta | freq | opcodes | rule |
|---|---|---|---|
| FIXED | — | SSTORE | EIP-3529 refund signs inverted + missing clean-clear (+4800) — fixed |
| +4800 | 6 | SSTORE | residual clears-refund case (needs a specific repro) |
| -22800 | 3 | CALL/CALLCODE/DELEGATECALL + SSTORE | sub-call→cold-account + SSTORE under-charge |
| 2400 / 1900 / 2600 | several | (tx) | **EIP-2930 access list** intrinsic + pre-warm (harness/runner ignore accessList) |
| 12 / 18 | 12 | CALLCODE + mem | call args/ret memory-expansion gas |
| huge (±1e7) | few | loops/CREATE | execution divergence (revert/OOG timing), not gas precision |

Next-session order (each isolated, regression-checked against fixtures):
1. EIP-2930 access list: add to Transaction + intrinsic_gas + prewarm + the
   runner/harness int-stream (CAUTION: stream encode/decode order must match).
2. residual SSTORE +4800 (repro a clears test, check the refund cap path).
3. sub-call cold-account/SSTORE -22800 (repro one CALL+SSTORE test).
4. interpreter step cap -> turn the loop timeouts into clean fails.
5. real ec/modexp precompiles; fork-aware gas.

Harness: `run_eest.py <files> --fork Cancun --gasdbg [--root]`.
