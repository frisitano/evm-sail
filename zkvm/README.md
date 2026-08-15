# evm-sail → eth-act zkVM RISC-V standard target

This directory cross-compiles the evm-sail Sail EVM into a **GMP-free, libc-IO-free,
statically-linked RISC-V guest** for the [eth-act zkVM standards](https://github.com/eth-act/zkvm-standards),
and validates it by executing an Ethereum block end-to-end on `spike`.

The proven program is the standalone Sail model in `../sail/` (a real, block-executable
EVM). Here it is lowered Sail → C → `riscv64im_zicclsm-unknown-none-elf` and run on the
bare ISA simulator, with the standard IO interface, guard regions, and termination
semantics wired up.

## Fixture gate

```
$ cd ..
$ python3 -m devtools.harness.cli --spike --limit 1 --quiet \
    zkvm/.fixtures/current-v062-full/blockchain_tests/for_amsterdam/shanghai/eip3855_push0/push0/push0_contracts.json
```

The gate diffs the guest's canonical SSZ `SszStatelessValidationResult` BYTE-EXACT
against the EELS reference: any EEST state test is executed through the
in-process EELS t8n (`devtools/harness/ssz_builder.py`), which produces a fully VALID
Amsterdam block input AND the reference guest's expected output bytes over that
exact input (fixtures already carrying `statelessInputBytes`/
`statelessOutputBytes` run directly). The same harness drives the native
in-process build (drop `--spike`), so the RISC-V guest and the native reference
are held to the identical oracle.

The guest also runs on-guest **keccak + SHA-256 self-checks** before the block, so
exit 0 additionally proves the C accelerators compute correctly on RISC-V through the
freestanding FFI path — not merely that they link.

### Canonical stateless validation (block read from private input)

The guest is a **stateless validator** matching `ethereum/execution-specs`
(`projects/zkevm`, `forks/amsterdam` `stateless_*`): it reads a schema-prefixed SSZ
`SszStatelessInput` from `read_input`, executes the block against an **execution
witness** (a secure-trie node db keyed by `keccak(node)`), recomputes the post-state
root and checks it commits to `payload.state_root`, computes
`new_payload_request_root = hash_tree_root(NewPayloadRequest)` (SSZ/SHA-256), and emits
the canonical `SszStatelessValidationResult` via `write_output`.

```
$ VEC=<raw SszStatelessInput file> ./build.sh run
successful_validation=1          # public output byte 32
--- spike exit code: 0 ---
```

A failed validation is a NORMAL result (`successful_validation=0`, exit 0).
Input vectors are raw `statelessInputBytes` — from EEST-generated fixtures or
built from any state test by `devtools/harness/ssz_builder.py` (in-process EELS t8n);
`devtools.harness.cli --spike` supplies them to the unchanged ELF at runtime. The fixture is
never compiled or linked into the guest. The stateless validator itself is
`../sail/main.sail` over the shared decoders/trie/HTR in `../sail/host/` and
`../sail/lib/`.

## Conformance to the standard target

Target triple: **`riscv64im_zicclsm-unknown-none-elf`**
(RV64I + M + Zicclsm, LP64 soft-float, static ELF, little-endian, flat memory, machine
mode, **no syscalls**).

Verified on the built ELF (`riscv64-unknown-elf-{readelf,objdump}`):

| Requirement | Status |
| --- | --- |
| RV64IM, **C extension excluded** | 0 compressed (`c.*`) instructions |
| Soft-float (F/D excluded), LP64 ABI | 0 floating-point instructions |
| No syscalls / environment calls | 0 `ecall`/`ebreak`/`mret`/`sret` in reachable text |
| Statically linked ELF | single PT_LOAD, no `INTERP`/`DYNAMIC`, no `NEEDED` libs |
| **GMP-free** | no `libgmp`/`mini-gmp`; exact bounded integers and inferred fixed-width values |
| Zicclsm (transparent misaligned data accesses) | run config `spike --misaligned` (see below) |

`spike`'s `--isa` string does not name `Zicclsm`; that extension only mandates transparent
misaligned load/store support, which spike provides via `--misaligned`. So the run config
`--isa=rv64im --misaligned` is the Zicclsm-equivalent. The guest is compiled
`-march=rv64im_zicclsm`.

### IO interface (`io-interface` standard)

`../extractions/c/zkvm_io.h` (the standard header) is implemented for the Spike validation
target in `io-device/guest.c`:

* `read_input(&buf, &size)` — loads the host-provided private input into guest
  memory on its first call, then returns the same cached read-only span on every
  call (`size==0` means the pointer is invalid, as specified). The Spike input
  device receives the fixture path at process launch; no input bytes reside in
  the ELF.
* `write_output(ptr, size)` — accumulates the public output (concatenating across calls)
  and mirrors it to the host console so the result is observable on spike. The result
  is published through `write_output`.

No `stdin`/libc IO is used anywhere.

### Memory layout + guard regions (`memory-safety-guard-regions` standard)

`runtime/link.ld` is the vendor linker script. Two mandatory guard regions are
**unmapped** and enforced:

* **Null-pointer trap [0x0, 0xFFF]** — guest memory starts at `0x80000000`, so the entire
  low range (incl. the first 4 KiB) is unmapped; a null access faults.
* **Stack guard (≥ 4 KiB)** — the stack lives in its own region `[0x90010000, 0x90110000)`;
  the 64 KiB gap `[0x90000000, 0x90010000)` immediately below the stack bottom is left out
  of the simulator's memory, so a stack overflow that decrements past the bottom faults.

The simulator memory ranges must match (the guard gap is deliberately omitted):

```
spike --isa=rv64im --misaligned -m0x80000000:0x10000000,0x90010000:0x00100000 <elf>
```

Guard enforcement was validated with a dedicated trap-test probe (a guest that
deliberately reads address 0x0 / the stack guard gap and must terminate
ABNORMALLY, exit 134). The probe and the HTIF de-risk program were one-time
platform bring-up tools, since deleted — recover `derisk_main.c` /
`traptest_main.c` and their `build.sh` targets from git history if the
platform glue (`start.S` / `link.ld` / `htif.c`) changes.

### Termination semantics (`standard-termination-semantics` + misaligned-instruction)

* **Successful termination** → halt, exit code 0 (HTIF `tohost=1`).
* **Abnormal termination** → halt, **non-zero** exit code 134 (a Type-2 verifier can key on
  it; a Type-1 verifier rejects any proof of a failed execution). Mapped from:
  Sail `assert` / match failure / builtin failure, C `abort()`/`exit(nonzero)`, an uncaught
  Sail exception (surfaced by `model_fini`), a fixture-fact mismatch, and — via the
  machine-mode trap vector in `start.S` — any synchronous fault: guard-region access, null
  dereference, and **instruction-address-misaligned** (per that standard).

## How the GMP-free build works

The model keeps EVM words in `bits(256)`, wire-sized runtime quantities in
`bits(64)`, and exact semantic quantities in Sail `nat`/`int`.
Transaction-controlled words are never silently narrowed: opcode-specific
checked conversion preserves the required overflow result. Proven 64-bit
ranges may lower to native `uint64_t`/`int64_t`; residual mathematical
integers use the exact bounded representation below. The build deliberately
does not use `--Ofixed-int`.

[`runtime/sail256/`](runtime/sail256/) replaces the stock `sail.c` + GMP entirely.
Large bitvectors use the fixed inline representation `{ len, d[4] }`; residual
Sail integers use a normalized sign plus 12 64-bit limbs. The 768-bit width
comes from the pre-Osaka MODEXP gas bound (`< 2^765`), and every operation
traps rather than wraps or saturates on overflow. The runtime has no dynamic
integer allocation and links no `libgmp` or `mini-gmp`. `make runtime-test`
differentially checks it against arbitrary-precision arithmetic.

`runtime/freestanding/` holds minimal `<stdio.h>`/`<stdlib.h>`/`<string.h>`/`<ctype.h>`/
`<assert.h>`/`<time.h>`/`<inttypes.h>` shims so the runtime decouples completely
from newlib; `runtime/runtime.c` supplies the small libc slice they actually use (mem/str
routines, a first-fit + coalescing heap allocator over the linker-defined heap, and the
termination mapping).

## Files

```
zkvm/
  build.sh              driver: guest | run | clean (VEC is runtime-only)
  zkvm_accel_mmio.h     spike MMIO wire protocol (accel_guest.c <-> accel_device.cc)
  zkvm_io_mmio.h        spike-private transport behind extractions/c/zkvm_io.h
  accel-host/           Rust crypto cdylib (the ONE accelerator implementation)
  accel-device/         spike MMIO device dispatching 1:1 into accel-host
  io-device/            spike runtime-input device (host file -> guest buffer)
  native-runner/        host builds: zkvm_native exe + the ctypes libs
                        (libevmsail_guest) over backend-local native_test.c
  runtime/
    link.ld             vendor linker script (null trap + stack guard regions)
    start.S             machine-mode crt0 + trap vector (platform glue; uses Zicsr)
    htif.c/.h           HTIF console + exit (spike host channel; validation only)
    runtime.c           freestanding libc subset + allocator + termination mapping
    harness.c           drives model init → guest main → write_output → terminate
    accel_guest.c       guest half of the accelerator API (MMIO marshalling)
    freestanding/       minimal hosted-header shims (decouple from newlib)
    sail256/            exact bounded-integer runtime
  io-device/
    guest.c             Spike implementation of standard read_input/write_output
```

## Building / running

Requires `riscv64-unknown-elf-gcc` and `spike` on `PATH`. Every repository
target uses the same custom Sail compiler. Its optimized C lowering supports
spliceable type definitions and bound-driven native C representations, while
its standard Lean and Coq backends preserve the model's semantic types. The
build uses `sail` on `PATH` and honours `SAIL=` for a different build; there is
no auto-detection, because silently resolving a stale compiler yields a wrong
model instead of an error. Upstream Sail is not a proof-extraction fallback; the real target build
is the capability check.

```
cd .. && python3 -m devtools.harness.cli --spike <state-test.json> --fork F   # the gate
VEC=<input.ssz> ./build.sh run     # build if needed, supply input, run on spike
./build.sh guest                   # input-free build (build/zkvm_guest.elf)
./build.sh clean
```

## Scope / follow-ups

* **Block-from-input**: DONE — the block + execution witness are deserialized from the
  canonical schema-prefixed SSZ `SszStatelessInput` in the private input, and the guest
  validates an arbitrary input block statelessly. Remaining refinements: block_hash
  recompute, sender-from-`public_keys`, non-empty execution requests, lazy
  arbitrary-access witness reads.
* **All crypto goes through the `c-interface-accelerators` standard.** keccak256, sha256,
  ripemd160, secp256k1, and the EVM precompiles share one boundary: the vendored standard
  header `../extractions/c/zkvm_accelerators.h` (verbatim from eth-act/zkvm-standards). There is a
  SINGLE implementation — the Rust `accel-host` (`k256`/`sha3`/`p256`/…) — served as proven
  precompiles. Native links it directly; the spike guest offloads every op to the host
  accelerator device (`accel-device/accel_device.cc`, linked against the Rust lib), so no
  crypto runs as guest instructions. (The portable-C reference `zkvm_accelerators.c` has
  been removed.) Sail calls focused C adapters directly against that interface:
  `../extractions/c/optimised/contract/src/primitives/crypto.c` for segmented hash axioms,
  `../extractions/c/optimised/contract/src/lib/htr.c` for the optimized whole-request HTR refinement,
  `../extractions/c/optimised/contract/src/host/accelerators.c` for EVM precompile execution, and
  `../extractions/c/optimised/contract/src/host/output.c` for output ownership.
* `start.S`/trap-vector use Zicsr (machine-mode CSRs) — these are **platform/crt0 glue**
  (a vendor responsibility under the memory-layout standard), not the proven STF, which
  stays pure `rv64im_zicclsm`.
* The HTIF console/exit channel is spike's host interface, used only by the validation
  harness; a real zkVM host provides the equivalent.

## Runtime provenance

`runtime/sail256/sail.h` specializes the Sail C runtime ABI to exact bounded
integers and inline bitvectors up to 256 bits. `runtime/sail256/sail.c`
implements the integer and bitvector primitives referenced by the model. No
GMP/`mini-gmp` source is vendored or linked.
