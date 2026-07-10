# evm-sail

**A formal, executable specification of the Ethereum Virtual Machine (WIP)**

![EVM Sail hero image](assets/evm-sail-hero-1280x720.jpg)

`evm-sail` is a specification of the Ethereum Virtual Machine written in
[Sail](https://github.com/rems-project/sail), the ISA-specification language
behind the official RISC-V model and the Arm/CHERI/x86 academic models. It is
**formal** — one small, typed, total source of truth with semantics suitable
for theorem provers and symbolic engines — and it is **executable**: the same
specification compiles to native code, riscv, runs real Ethereum blocks, and passes
**~100% of the execution-spec-tests state suites for the modern forks** (Berlin onwards).
This formal specification is inspired by and intended as a complement to [`evm-asm`](https://github.com/Verified-zkEVM/evm-asm).

Objectives:

- **Complete & objective.** The specification is complete — it defines the EVM,
  the host kernel it runs over, and the interface between them. And it is
  objective: the semantics are fixed by executable code, not by a
  natural-language description, and the conformance suite runs against the same
  model you reason about.
- **One artifact, many backends.** Sail definitions export to Coq,
  Isabelle/HOL, HOL4 and Lean, generate SMT obligations, and compile to C — so
  the *same* model that passes the conformance suite is the one you reason
  about. Nothing is lost between a paper spec and an implementation.
- **Conducive to formal verification.** The model is small, typed (bitvector
  widths checked at definition time), and total: `execute` never gets stuck —
  every error sets `frame_status` to a halt/exception. Properties (gas
  monotonicity, revert atomicity, stack bounds) are stated against the actual
  interpreter, not a re-formalization of it.
- **Conducive to advanced / symbolic testing.** The interpreter is a pure
  function over an explicit machine state, with the world behind a small
  syscall interface — exactly where symbolic execution, differential fuzzing,
  and witness-based (stateless) execution attach.

## Architecture: a host kernel, and the EVM as user space

- **The host** (`sail/host/`): Ethereum world state — user accounts, storage, transient
  storage, warm sets, logs, refunds, snapshots, the block/tx environment —
  plus per-frame memory and the crypto accelerators. Pure mechanism: no gas,
  no policy. State is reachable **only** through the host's kernel functions:

```
Storage:  k_access_slot  k_sload  k_sstore  k_tload  k_tstore
Account:  k_access_account  k_get_balance/nonce/code/codehash  k_transfer
          k_bump_nonce  k_set_code  k_get_code_size  k_code_copy
          k_deleg_target  k_seed_account
Env:      k_env(field)  k_blockhash  k_blobhash  k_coinbase
Prim:     k_create_addr  k_create2_addr  k_precompile
Utils:    k_snapshot  k_commit  k_revert  k_refund_add  k_log
```

- **The EVM** (`sail/evm/`): the opcode
  interpreter, the gas counter, and **all** policy — the full fork-gated gas
  schedule, the EIP-2929/2200/3529 rules, transaction validity
  (EIP-1559/2930/3860/4844/7623/7702/7825), and *the decision of whether an
  effect happens* (a no-op SSTORE charges gas in the EVM but performs no
  host write). User space: it holds no world state of its own and reaches
  it only through the k_* syscalls.

The host-call boundary is what makes the model a natural front end for proof
systems (each kernel function is an interface channel), for stateless
validation (the host fails closed on state absent from a witness), and for
symbolic engines (the world is an explicit, finite interface).

### Native execution: C FFI backends

To *run* the model — the generated C compiled natively (`sail256`/`sailfix`) —
the host's mechanism is backed by C FFI; the Sail definitions stay the
specification while these provide the data structures underneath it.
Performance-critical state lives behind C FFI with O(1) operations — EVM
memory, calldata and returndata (`ffi/memory.c`, `ffi/returndata.c`), direct
host crypto and precompile adapters (`ffi/host_crypto.c`, `ffi/precompiles.c`), the
operand stack (`ffi/stack.c`), the account code store + per-frame code
descriptors with prebuilt JUMPDEST bitmaps (`ffi/code_db.c`), transient
storage (`ffi/transient_storage.c`, with frame rollback driven by the Sail
journal), and account plus persistent storage state (`ffi/state_db.c`, sorted
cache/update backends keyed by keccak(address) and keccak(slot)). In-memory representations keep raw keys
alongside secure trie keys; hashing happens at state access setup, the write
boundary (the account's cached codeHash), and the commitment boundary (state
root, witness authentication). The Sail semantics is unchanged by these
backends.

## Performance

Three host runtimes compile the same generated C:

| Runtime | 256-bit words | Throughput* |
|---|---|---|
| GMP (stock Sail) | heap `mpz_t` | ~5.6 Mgas/s |
| `sailfix` | inline 512-bit (guest-shared, zkVM) | below GMP |
| `sail256` | inline 512-bit, host-optimized | **~28–35 Mgas/s** |

*Workload mix: ~28 Mgas/s on an execution-bound
TSTORE loop, ~35 Mgas/s across the mix, with calls/creates/precompiles well
above (deep reentrant CALLs ~940, 49KB-initcode CREATE ~170, blake2f ~100).
A 30–45M-gas mainnet block executes in ~1–1.5s under `sail256` — within ~13×
of revm, which is a production interpreter, not a specification. Calling a
24KB-code contract costs the same as calling a 1-byte one (O(1) frame entry).

## Layout

```
sail/        the specification (evm.sail_project selects core/entry files)
  runner.sail         single-block / EEST runner entry point (reads input, runs)
  host/
    state.sail        world state: accounts, storage overlays, warm sets,
                      logs, journal, block/tx environment
    kernel.sail       the kernel functions (k_*): the only state interface
    memory.sail       per-frame byte memory (C-backed, O(1))
    io.sail           host I/O: accelerator FFI binding + keccak256/sha256 +
                      EVM precompiles + CREATE derivation + the stateless SSZ
                      input decoder (eth-act zkvm-standards C boundary)
  evm/                THE TRANSACTION KERNEL (= the EVM)
    machine.sail      frame registers, gas counter, stack, code descriptors
    gas.sail          the complete gas schedule (fork-gated)
    instructions.sail the opcode AST
    execute.sail      per-opcode semantics (policy here, effects via k_*)
    interpreter.sail  fetch/decode, run loop, CALL*/CREATE*, precompiles
    transaction.sail  tx validity + the state transition + refunds
    block.sail        whole-block execution (txs + withdrawals)
  lib/
    rlp.sail  ssz_htr.sail
    mpt.sail           MPT root builder + state trie + stateless witness reads
                       (feed, re-root, fail-closed lookups; C-backed node-db)
ffi/         C backends: memory.c (memory/calldata), transient_storage.c
             (transient storage), state_db.c (account and persistent storage
             cache/update maps), stack.c (operand stack), code_db.c
             (code store + frame descriptors + JUMPDEST bitmaps), trie_node_db.c
             (witness node-db), host_crypto.c + precompiles.c +
             zkvm_accelerators.h (eth-act zkvm-standards crypto)
harness/   the EEST harness: run.py drives BOTH entries in-process
             (EVM_ENTRY=runner state tests by post-state root; --guest gates the
             stateless guest byte-exact vs the EELS reference via in-process t8n)
zkvm/        RISC-V zkVM guest target (riscv64im, stateless block validation)
  runtime/sailfix     GMP-free fixed-width Sail runtime (guest-shared)
  runtime/sail256     host-optimized variant (sized limbs, Knuth-D division)
  accel-host/         host crypto cdylib (blst, k256, c-kzg, aurora-modexp, p256)
```

## Build and run

The Sail toolchain installs via **opam** (do not `brew install sail` — that is
an unrelated tool):

```sh
opam init --bare -y && opam switch create sail 5.2.0 && eval "$(opam env --switch=sail)"
opam install -y sail && sail --version
```

Type-check the specification (block execution is validated by the EEST harness
and the zkVM guest, below):

```sh
make check                                  # type-check sail/evm.sail_project
make lint                                   # sail --all-warnings + source hygiene
make fmt-check                              # verify sail --fmt formatting
```

`make all` runs `check` + `lint` + `fmt-check`. `make lint`
enforces a warning-clean model and basic `*.sail` hygiene (no trailing
whitespace, no tabs, final newline); `make fmt-check` enforces that every
`*.sail` is formatted with the official `sail --fmt` (canonical 4-space style).
`make fmt` reformats in place.

Run the conformance suite against a local
[execution-spec-tests](https://github.com/ethereum/execution-spec-tests)
state-fixtures checkout:

```sh
cd harness
SAIL256=1 python3 run.py --rebuild --fork Cancun <fixtures>/state_tests/cancun

# gate the stateless guest byte-exact against the EELS reference:
python3 run.py --guest --fork Cancun fixtures/eels/cancun_selfdestruct/state_tests/for_cancun
python3 run.py --guest --fork Cancun <fixtures>/state_tests/cancun
```

The pass criterion is the recomputed post-state MPT root (a commitment to the
full post-state) for the runner, and byte-exact output agreement with the EELS
reference guest for `--guest`; `--verbose` + `--dump` print diagnostics and the
model's live post-run state on a failure (the debugging loop). The crypto
(keccak/secp256k1/bn254/BLS12-381/KZG/modexp/blake2f/P-256) runs through the
eth-act zkvm-standards accelerator boundary, backed by the industry libraries
in `zkvm/accel-host` (blst, k256, c-kzg, aurora-engine-modexp, p256).

## License

Dedicated to the public domain under [CC0-1.0](LICENSE), mirroring
[`ethereum/execution-specs`](https://github.com/ethereum/execution-specs) and
the EIPs — so this specification of the Ethereum protocol can be reused,
modified, and built upon without restriction.
