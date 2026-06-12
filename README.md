# evm-sail — a formal, executable specification of the Ethereum EVM

`evm-sail` is a specification of the Ethereum Virtual Machine written in
[Sail](https://github.com/rems-project/sail), the ISA-specification language
behind the official RISC-V model and the Arm/CHERI/x86 academic models. It is
**formal** — one small, typed, total source of truth with semantics suitable
for theorem provers and symbolic engines — and it is **executable**: the same
specification compiles to native code, runs real Ethereum blocks, and passes
**100% of the execution-spec-tests state suites for the modern forks**:

| Suite (execution-spec-tests state fixtures) | Result |
|---|---|
| Cancun | 1427/1427 |
| Shanghai | 88/88 |
| Prague | 2010/2010 |
| Osaka | 1223/1223 |

Why Sail, rather than prose (the Yellow Paper) or a reference client (EELS,
revm)?

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

## Architecture: EVM user space / EL kernel

- **EVM = user space** (`sail/execute.sail`, `interpreter.sail`, `gas.sail`,
  `transaction.sail`): the opcode interpreter, the gas counter, and **all**
  policy — the full gas schedule, the EIP-2929/2200/3529 rules, transaction
  validity (EIP-1559/2930/3860/4844/7623/7702/7825), and *the decision of
  whether an effect happens*. Pure compute.
- **EL = kernel / host** (`sail/el_kernel.sail`): the privileged world state —
  accounts, storage, transient storage, warm sets, logs, refunds, snapshots,
  block/tx environment — reachable **only** through the EL syscall ISA:

```
Storage:  k_access_slot  k_sload  k_sstore  k_tload  k_tstore  k_orig
Account:  k_access_account  k_get_balance/nonce/code/codehash  k_transfer
          k_bump_nonce  k_set_code  k_add_balance  k_sub_balance
Env:      k_env(field)  k_blockhash  k_blobhash  k_coinbase
Prim:     k_keccak  k_create_addr  k_create2_addr  k_precompile
World:    k_snapshot  k_commit  k_revert  k_refund_add  k_log
```

The EVM issues a syscall **only for a real effect** (a no-op SSTORE charges
gas in user space but emits no kernel write — see
`examples/syscall_demo.sail`). An execution is recorded as an interleaved
stream of opcode steps and authenticated effects (`ir_trace`). That boundary
is what makes the model a natural front end for proof systems (the effect
stream is an AIR interface-channel stream), for stateless validation (the
kernel fails closed on state absent from a witness), and for symbolic engines
(the world is an explicit, finite interface).

Performance-critical state lives behind C FFI with O(1) operations — EVM
memory (`ffi/el_mem.c`) and the overlay-layered storage / transient storage /
warm sets (`ffi/el_map.c`: frame revert = discard the top layer; the EIP-2200
"original" value = the overlay's base layer). The Sail semantics is unchanged
by these backends.

## Performance

Three host runtimes compile the same generated C:

| Runtime | 256-bit words | Throughput* |
|---|---|---|
| GMP (stock Sail) | heap `mpz_t` | ~5.6 Mgas/s |
| `sailfix` | inline 512-bit (guest-shared, zkVM) | below GMP |
| `sail256` | inline 512-bit, host-optimized | **~22 Mgas/s** |

*Execution-bound benchmark (a TSTORE loop run until out-of-gas, 360M gas). A
30–45M-gas mainnet block executes in ~1.5–2s under `sail256` — within ~17× of
revm, which is a production interpreter, not a specification.

## Layout

```
sail/        the specification (el_ir.sail is the root include)
  el_kernel.sail      EL kernel: world state + the syscall ISA + ir_trace
  machine.sail        frame registers, gas counter, stack, code cursor
  memory.sail         per-frame byte memory (C-backed, O(1))
  gas.sail            the complete gas schedule (fork-gated)
  instructions.sail   the opcode AST
  execute.sail        per-opcode semantics (policy here, effects via k_*)
  interpreter.sail    fetch/decode, run loop, CALL*/CREATE*, precompiles
  transaction.sail    tx validity + the state transition + refunds
  block.sail          whole-block execution (txs + withdrawals)
  keccak.sail  rlp.sail  mpt.sail  hash_builder.sail  state_root.sail
  mpt_witness.sail    stateless witness: feed, re-root, fail-closed lookups
ffi/         C backends: el_mem.c (memory), el_map.c (overlay maps),
             acc_shim.c + zkvm_accelerators.h (eth-act zkvm-standards crypto)
examples/    runnable examples + the EEST harness (run_eest.py, runner.sail)
revm-eest/   parallel EEST runner (Rust): drives the model on all cores
zkvm/        RISC-V zkVM guest target (riscv64im, stateless block validation)
  runtime/sailfix     GMP-free fixed-width Sail runtime (guest-shared)
  runtime/sail256     host-optimized variant (sized limbs, Knuth-D division)
  accel-host/         host crypto cdylib (blst, k256, c-kzg, aurora-modexp, p256)
mapping/     the EL-syscall <-> AIR-channel and Sail <-> Rust mappings
```

## Build and run

The Sail toolchain installs via **opam** (do not `brew install sail` — that is
an unrelated tool):

```sh
opam init --bare -y && opam switch create sail 5.2.0 && eval "$(opam env --switch=sail)"
opam install -y sail && sail --version
```

Type-check the specification and run the example block + fixture suite
(needs a C compiler; the GMP build additionally needs libgmp):

```sh
make check
make run-example                            # execute a block + print the EL trace
make run EX=examples/fixtures.sail          # 8 block fixtures
make run EX=examples/syscall_demo.sail      # no-op SSTORE emits no kernel write
```

Run the conformance suite against a local
[execution-spec-tests](https://github.com/ethereum/execution-spec-tests)
state-fixtures checkout:

```sh
cd examples/eest
SAIL256=1 python3 run_eest.py --rebuild --fork Cancun <fixtures>/state_tests/cancun

# or across all cores, with the Rust runner:
cd ../../revm-eest && cargo build --release
./target/release/revm-eest --bin ../examples/eest/.runner_bin \
    --fork Cancun --timeout 60 <fixtures>/state_tests/cancun
```

`run_eest.py --verbose` prints per-account balance/nonce/storage diffs (the
debugging loop); `revm-eest` is the bulk measurement tool; `--root`
additionally computes and checks the post-state MPT root. The crypto
(keccak/secp256k1/bn254/BLS12-381/KZG/modexp/blake2f/P-256) runs through the
eth-act zkvm-standards accelerator boundary, backed by the industry libraries
in `zkvm/accel-host` (blst, k256, c-kzg, aurora-engine-modexp, p256).

## Provenance

Extracted from the `evm-asm` research workspace, where this model is the
semantic front end of a specification-driven EVM AIR compiler (Event IR →
checked facts → Constraint IR/AIR → WHIR proof backend). The internal name of
the syscall ISA — **EL-IR** — is retained: the `TSyscall` effect stream is
that proof system's AIR interface-channel stream, and the correspondence is
documented in [`mapping/MAPPING.md`](mapping/MAPPING.md).
