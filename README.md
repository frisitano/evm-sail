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

## Architecture: a host, and the transaction kernel over user space

- **The host** (`evm/host/`): world state — user accounts, storage, transient
  storage, warm sets, logs, refunds, snapshots, the block/tx environment —
  plus per-frame memory and the crypto accelerators. Pure mechanism: no gas,
  no policy. State is reachable **only** through the host's kernel functions:

```
Storage:  k_access_slot  k_sload  k_sstore  k_tload  k_tstore  k_orig
Account:  k_access_account  k_get_balance/nonce/code/codehash  k_transfer
          k_bump_nonce  k_set_code  k_add_balance  k_sub_balance
Env:      k_env(field)  k_blockhash  k_blobhash  k_coinbase
Prim:     k_keccak  k_create_addr  k_create2_addr  k_precompile
World:    k_snapshot  k_commit  k_revert  k_refund_add  k_log
```

- **The transaction kernel = the EVM** (`evm/evm/`): the opcode
  interpreter, the gas counter, and **all** policy — the full fork-gated gas
  schedule, the EIP-2929/2200/3529 rules, transaction validity
  (EIP-1559/2930/3860/4844/7623/7702/7825), and *the decision of whether an
  effect happens* (a no-op SSTORE charges gas in the transaction kernel but
  performs no host write). It operates over user accounts — user space.

The host-call boundary is what makes the model a natural front end for proof
systems (each kernel function is an interface channel), for stateless
validation (the host fails closed on state absent from a witness), and for
symbolic engines (the world is an explicit, finite interface).

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
evm/         the specification (evm.sail is the root include)
  host/
    state.sail        world state: accounts, storage overlays, warm sets,
                      logs, journal, block/tx environment
    kernel.sail       the kernel functions (k_*): the only state interface
    memory.sail       per-frame byte memory (C-backed, O(1))
    accelerators.sail crypto host functions (eth-act zkvm-standards)
  evm/                THE TRANSACTION KERNEL (= the EVM)
    machine.sail      frame registers, gas counter, stack, code cursor
    gas.sail          the complete gas schedule (fork-gated)
    instructions.sail the opcode AST
    execute.sail      per-opcode semantics (policy here, effects via k_*)
    interpreter.sail  fetch/decode, run loop, CALL*/CREATE*, precompiles
    transaction.sail  tx validity + the state transition + refunds
    block.sail        whole-block execution (txs + withdrawals)
  lib/
    keccak.sail  sha256.sail  rlp.sail  rlp_decode.sail  mpt.sail
    hash_builder.sail  state_root.sail  block_hash.sail  ssz.sail  ssz_htr.sail
    mpt_witness.sail  stateless witness: feed, re-root, fail-closed lookups
ffi/         C backends: host_mem.c (memory), host_map.c (overlay maps),
             acc_shim.c + zkvm_accelerators.h (eth-act zkvm-standards crypto)
examples/    runnable examples + the EEST harness (run_eest.py, runner.sail)
revm-eest/   parallel EEST runner (Rust): drives the model on all cores
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

Type-check the specification and run the example block + fixture suite
(needs a C compiler; the GMP build additionally needs libgmp):

```sh
make check
make run-example                            # execute a block + print the EL trace
make run EX=examples/fixtures.sail          # 8 block fixtures
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
checked facts → Constraint IR/AIR → WHIR proof backend). The host kernel
functions correspond one-to-one with that proof system's AIR interface
channels.
