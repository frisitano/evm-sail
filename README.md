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
Utils:    k_state_checkpoint  k_revert  k_log
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

To *run* the model — the generated C compiled against `sail256` —
the host's mechanism is backed by C FFI; the Sail definitions stay the
specification while these provide the data structures underneath it.
Performance-critical state lives behind C FFI with O(1) operations — EVM
memory, generic byte-slice sources, and the output arena (`ffi/memory.c`,
`ffi/output.c`), direct accelerator-backed hash and precompile adapters
(`ffi/hash_glue.c`, `ffi/htr_glue.c`, `ffi/precompiles.c`), the operand stack
(`ffi/stack.c`), and the content-addressed code arena plus packed JUMPDEST tables
(`ffi/code_db.c`). Sail performs PUSH-aware code analysis before insertion;
each active frame holds one `Code` pairing the code slice with its
resolved table reference. Transient
storage (`ffi/transient_storage.c`, with frame rollback driven by the Sail
journal), and account plus persistent storage state (`ffi/state_db.c`, sorted
cache/update backends keyed by keccak(address) and keccak(slot)). In-memory representations keep raw keys
alongside secure trie keys; hashing happens at state access setup, the write
boundary (the account's cached codeHash), and the commitment boundary (state
root, witness authentication). The Sail semantics is unchanged by these
backends.

## Performance

Native and RISC-V builds now use one GMP-free runtime. EVM words remain inline
four-limb `bits(256)` values; wire-sized lengths, offsets, and cursors are
explicit `bits(64)` quantities with checked operations. Exact semantic gas
costs and signed refunds remain Sail `nat`/`int`. Residual mathematical
integers use a fixed 12-limb, 768-bit sign-magnitude runtime selected from the
model-wide maximum intermediate; proven 64-bit ranges can still lower to
native scalars. Operations trap rather than truncate if that audited bound is
violated.

The evidence-backed zkVM optimisation backlog, current measurements, and
validation requirements are tracked in
[`docs/OPTIMIZATIONS.md`](docs/OPTIMIZATIONS.md).

## Layout

```
sail/        the specification (evm.sail_project defines the single build)
  main.sail           the single stateless block executable entry
  host/
    state.sail        world state: accounts, storage overlays, warm sets,
                      logs, journal, block/tx environment
    kernel/           the kernel functions (k_*): the only state interface
    memory.sail       per-frame byte memory (C-backed, O(1))
    byte_slice.sail   generic calldata/code views + Sail JUMPDEST analysis
    scratch.sail      executor scratch-arena FFI contract
    code.sail         content-addressed code/JUMPDEST FFI contract
    nodes.sail        witness trie node DB FFI contract
    accelerators.sail crypto/precompile accelerator FFI contract
    output.sail       persistent frame output + public guest output contract
  evm/                THE TRANSACTION KERNEL (= the EVM)
    machine.sail      frame registers, gas counter, stack, code descriptors
    gas.sail          the complete gas schedule (fork-gated)
    instructions.sail the opcode AST
    execute.sail      per-opcode semantics (policy here, effects via k_*)
    interpreter.sail  fetch/decode, run loop, CALL*/CREATE*, precompiles
    transaction.sail  tx validity + the state transition + refunds
  executor/
    block.sail        whole-block execution (txs + withdrawals)
    payload.sail      transaction/withdrawal MPT roots for payload validation
    system_calls.sail protocol system-call orchestration
    block_access_list.sail EIP-7928 validation
  lib/
    rlp/rlp.sail       RLP decoding and encoding
    ssz/
      ssz.sail         generic source-backed SSZ readers and list navigation
      stateless_input.sail concrete SszStatelessInput refs and decoder
    htr.sail           SSZ hash-tree-root computation
    mpt/               generic MPT implementation
      primitives.sail  trie paths + hex-prefix encoding
      nodes.sail       node refs, RLP encoding/decoding + C-backed node-db
      updates.sail     ordered updates + canonical trie rebuilding
      trie.sail        witness overlay, roots + fail-closed lookup
ffi/         C backends: memory.c (memory/generic byte slices), scratch.c
             (Sail-cursor-owned executor scratch arena), transient_storage.c
             (transient storage), state_db.c (account and persistent storage
             cache/update maps plus distinct keyed EIP-7928 read/change tables
             exposed as one sorted event stream), stack.c (operand stack), code_db.c
             (content-addressed code + packed JUMPDEST arenas), trie_node_db.c
             (witness node-db), hash_glue.c + precompiles.c +
             zkvm_accelerators.h (eth-act zkvm-standards crypto), and the
             optimized-C-only htr_glue.c whole-request refinement
harness/     the EEST harness: run.py drives main.sail in-process and gates its
             canonical output byte-exactly against EELS; state tests are first
             materialized as valid stateless blocks by the in-process t8n
extractions/ maintained C, Coq, and Lean model generation plus extern contracts
zkvm/        RISC-V zkVM guest targets (Spike and production ZisK stateless
             block validation)
  runtime/sail256     shared GMP-free runtime for bounded integers
  accel-host/         host crypto cdylib (blst, k256, c-kzg, aurora-modexp, p256)
```

## Build and run

The Sail toolchain installs via **opam** (do not `brew install sail` — that is
an unrelated tool):

```sh
opam init --bare -y && opam switch create sail 5.2.0 && eval "$(opam env --switch=sail)"
opam install -y rocq-sail-stdpp.0.20.2
# Build Sail 0.20.2 from source, put its `sail` binary on PATH, then:
sail --version
```

The compiler and `rocq-sail-stdpp` package must have compatible Sail releases.

All repository targets use one custom Sail compiler, selected by
`zkvm/resolve_optimized_sail.sh`: an explicit `SAIL` takes precedence, followed
by the adjacent feature worktree and then `sail` on `PATH`. The compiler
supports the standard Sail backends plus spliceable type definitions and
bound-driven native C representations. Those extensions affect only optimized
C lowering; Lean and Coq use the same compiler without the C-only splice and
therefore retain the model's ordinary semantic types. Upstream Sail is not a
supported fallback for checks or proof extraction.

Type-check the specification (block execution is validated by the EEST harness
and the zkVM guest, below):

```sh
make check                                  # type-check sail/evm.sail_project
make lint                                   # sail --all-warnings + source hygiene
make fmt-check                              # verify sail --fmt formatting
make extract-c                              # generate + compile-check optimized C
make extract-coq                            # generate the complete Coq model
make extract-lean                           # generate and compile the Lean model
make extract                                # run all maintained extractions
```

`make extract-c` owns the tracked source-aligned model under
`extractions/c/evm/`. Every active file below `sail/` has a corresponding C
unity fragment, while `evm.c` remains the compilation entry point and `evm.h`
the complete generated interface. The extraction retains Sail's default C
name mangling and uses the same specialization and `c_optimized.sail` splice as
the optimized native and zkVM builds. That splice lowers the complete
`htr_new_payload_request` operation to `ffi/htr_glue.c`, allowing production C
to hash the already validated `StatelessInputRef` slices without constructing
the intermediate Sail values. Standard native C and Lean/Coq extraction do not
load this C-only replacement and retain the explicit, readable equations in
`sail/lib/htr.sail`; the refinement therefore does not add a proof axiom.
Machine-local generator scratch, editor metadata, and object files remain
untracked. See
[`extractions/README.md`](extractions/README.md) for the ownership contract.

`make all` runs `check` + `lint` + `fmt-check`. `make lint`
enforces a warning-clean model and basic `*.sail` hygiene (no trailing
whitespace, no tabs, final newline); `make fmt-check` enforces that every
`*.sail` is formatted with the official `sail --fmt` (canonical 4-space style).
`make fmt` reformats in place.

Run the conformance suite against the workspace-local
`tests-zkevm@v0.6.2` corpus:

```sh
cd harness
python3 run.py --rebuild --limit 1 --quiet \
  ../zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
python3 run.py --jobs 12 --quiet ../zkvm/.fixtures/current-v062-full
```

The sole pass criterion is byte-exact output agreement with the EELS reference
guest. `--debug` prints an on-demand native post-run state dump on failure;
`--profile` enables optional zkVM cycle scopes without embedding them in normal
builds. `--zisk` builds the production, input-agnostic ZisK ELF and runs the
same fixtures through `ziskemu`. The emulator version must match the `ziskos`
version locked in `zkvm/zisk/Cargo.lock`; set `ZISKEMU` when the matching
binary is not at `~/.zisk/bin/ziskemu`:

```sh
ZISKEMU=/path/to/ziskemu python3 run.py --zisk --limit 1 --quiet \
  ../zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
```

The crypto
(keccak/secp256k1/bn254/BLS12-381/KZG/modexp/blake2f/P-256) runs through the
eth-act zkvm-standards accelerator boundary, backed by the industry libraries
in `zkvm/accel-host` (blst, k256, c-kzg, aurora-engine-modexp, p256).

## License

Dedicated to the public domain under [CC0-1.0](LICENSE), mirroring
[`ethereum/execution-specs`](https://github.com/ethereum/execution-specs) and
the EIPs — so this specification of the Ethereum protocol can be reused,
modified, and built upon without restriction.
