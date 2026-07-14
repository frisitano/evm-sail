# evm-sail → eth-act zkVM RISC-V standard target

This directory cross-compiles the evm-sail Sail EVM into a **GMP-free, libc-IO-free,
statically-linked RISC-V guest** for the [eth-act zkVM standards](https://github.com/eth-act/zkvm-standards),
and validates it by executing an Ethereum block end-to-end on `spike`.

The proven program is the standalone Sail model in `../sail/` (a real, block-executable
EVM). Here it is lowered Sail → C → `riscv64im_zicclsm-unknown-none-elf` and run on the
bare ISA simulator, with the standard IO interface, guard regions, and termination
semantics wired up.

## Result (validated)

```
$ python3 ../harness/run.py --spike --fork Shanghai --quiet \
    ../harness/fixtures/eels/shanghai_push0/state_tests/for_shanghai/shanghai/eip3855_push0/push0/push0_contracts.json
=== 2/2 passed (spike guest, byte-exact) ===
```

The gate diffs the guest's canonical SSZ `SszStatelessValidationResult` BYTE-EXACT
against the EELS reference: any EEST state test is executed through the
in-process EELS t8n (`harness/ssz_builder.py`), which produces a fully VALID
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
built from any state test by `harness/ssz_builder.py` (in-process EELS t8n);
`run.py --spike` supplies them per fixture. The stateless
validator itself is `../sail/main.sail` over the shared decoders/trie/HTR in
`../sail/host/` and `../sail/lib/`.

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
| **GMP-free** | no `libgmp`/`mini-gmp`/`mpz`; fixed-width `sailfix` runtime (512-bit int, 256-bit lbits) |
| Zicclsm (transparent misaligned data accesses) | run config `spike --misaligned` (see below) |

`spike`'s `--isa` string does not name `Zicclsm`; that extension only mandates transparent
misaligned load/store support, which spike provides via `--misaligned`. So the run config
`--isa=rv64im --misaligned` is the Zicclsm-equivalent. The guest is compiled
`-march=rv64im_zicclsm`.

### IO interface (`io-interface` standard)

`zkvm_io.h` (the standard header) is implemented in `runtime/zkvm_io.c`:

* `read_input(&buf, &size)` — returns the private-input region (`__zkvm_input`,
  idempotent; `size==0` ⇒ buffer invalid, as specified). The harness calls it and reports
  `input_size`. For this milestone the block is built inside the Sail model, so input is
  empty; loading the block *from* the private input is a documented follow-up.
* `write_output(ptr, size)` — accumulates the public output (concatenating across calls)
  and mirrors it to the host console so the result is observable on spike. The result
  facts (`gas_used`, `storage0`, …) are published through `write_output`.

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

The stock Sail C backend emits the GMP arbitrary-precision ABI (`mpz_t` integers, `lbits`
bitvectors backed by `mpz`). The EVM uses 256-bit words (`bits(256)`), which exceed 64
bits, so the toolchain's `nostd` value-ABI runtime (which caps `lbits` at 64 bits) is
**not** usable, and width-bounding `sail_int` to 128 bits (`-DSAIL_INT128`) would silently
truncate general 256-bit values.

Instead we link a **fixed-width runtime, [`runtime/sailfix/`](runtime/sailfix/)**, that
replaces the stock `sail.c` + GMP entirely: `sail_int` is a **512-bit sign-magnitude**
integer (declared as an array-of-1 so it keeps `mpz_t`'s pass-as-pointer ABI — the
*unchanged* Sail-generated C recompiles against it), and `lbits` is a **256-bit inline**
`{ len, d[4] }` (256 is the max bitvector width in the EVM). 512 bits covers every integer
the EVM produces — MUL/MULMOD/EXP form `unsigned(a)*unsigned(b) <= 2^512` before
truncation. It implements only the ~40 runtime functions the guest references (schoolbook
512-bit multiply + bit-by-bit division with a 64-bit fast path), so there is **no `libgmp`,
no `mini-gmp`, and no `mpz` in the binary**. Correctness is checked on-guest by
`arith_selfcheck` (256-bit MUL/DIV/MOD, 512-bit MULMOD, EXP, signed SDIV/SMOD vs Python
ground truth) and by the full stateless run.

`runtime/freestanding/` holds minimal `<stdio.h>`/`<stdlib.h>`/`<string.h>`/`<ctype.h>`/
`<assert.h>`/`<time.h>`/`<inttypes.h>` shims so the runtime decouples completely
from newlib; `runtime/runtime.c` supplies the small libc slice they actually use (mem/str
routines, a first-fit + coalescing heap allocator over the linker-defined heap, and the
termination mapping).

## Files

```
zkvm/
  build.sh              driver: guest | run | clean (VEC = the baked input vector)
  zkvm_io.h             the standard IO header (verbatim from zkvm-standards)
  zkvm_input.h          guest extern decls injected into the generated model C
  zkvm_accel_mmio.h     spike MMIO wire protocol (accel_guest.c <-> accel_device.cc)
  accel-host/           Rust crypto cdylib (the ONE accelerator implementation)
  accel-device/         spike MMIO device dispatching 1:1 into accel-host
  native-runner/        host builds: zkvm_native exe + the ctypes libs
                        (libevmsail_guest) over test_utils.c
  runtime/
    link.ld             vendor linker script (null trap + stack guard regions)
    start.S             machine-mode crt0 + trap vector (platform glue; uses Zicsr)
    htif.c/.h           HTIF console + exit (spike host channel; validation only)
    runtime.c           freestanding libc subset + allocator + termination mapping
    zkvm_io.c           read_input / write_output (io-interface standard)
    zkvm_input.c        private-input plumbing (baked vector / ere read_input)
    harness.c           drives model init → guest main → write_output → terminate
    accel_guest.c       guest half of the accelerator API (MMIO marshalling)
    freestanding/       minimal hosted-header shims (decouple from newlib)
    sailfix/            GMP-free fixed-width Sail runtime (512-bit int, 256-bit lbits)
    sail256/            host-optimized GMP-free runtime (native exe + ctypes libs)
```

## Building / running

Requires `sail` (opam), `riscv64-unknown-elf-gcc`, and `spike` on `PATH`
(`eval $(opam env --root=$HOME/.opam --switch=sail)` for sail).

```
python3 ../harness/run.py --spike <state-test.json> --fork F   # the gate
VEC=<input.ssz> ./build.sh run     # one vector: build the guest, run on spike
VEC=<input.ssz> ./build.sh guest   # build only (produces build/zkvm_guest.elf)
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
  header `../ffi/zkvm_accelerators.h` (verbatim from eth-act/zkvm-standards). There is a
  SINGLE implementation — the Rust `accel-host` (`k256`/`sha3`/`p256`/…) — served as proven
  precompiles. Native links it directly; the spike guest offloads every op to the host
  accelerator device (`accel-device/accel_device.cc`, linked against the Rust lib), so no
  crypto runs as guest instructions. (The portable-C reference `zkvm_accelerators.c` has
  been removed.) Sail calls explicit C adapters instead of a catch-all shim:
  `../ffi/host_crypto.c` for direct hash pointer/length calls,
  `../ffi/precompiles.c` for EVM precompile execution,
  and `../ffi/output.c` for output ownership. keccak/sha256 are exercised + asserted on-guest by
  `keccak_selfcheck`/`sha256_selfcheck`.
* `start.S`/trap-vector use Zicsr (machine-mode CSRs) — these are **platform/crt0 glue**
  (a vendor responsibility under the memory-layout standard), not the proven STF, which
  stays pure `rv64im_zicclsm`.
* The HTIF console/exit channel is spike's host interface, used only by the validation
  harness; a real zkVM host provides the equivalent.

## Runtime provenance

`runtime/sailfix/sail.h` is the stock Sail C runtime header with the GMP types swapped for
the fixed-width `sail_int`/`lbits` above; `runtime/sailfix/sail.c` is an original GMP-free
implementation of the int/bits primitives. No GMP/`mini-gmp` source is vendored or linked.
