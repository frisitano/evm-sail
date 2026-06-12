# EL-IR Sail EVM → eth-act zkVM RISC-V standard target

This directory cross-compiles the evm-sail Sail EVM into a **GMP-free, libc-IO-free,
statically-linked RISC-V guest** for the [eth-act zkVM standards](https://github.com/eth-act/zkvm-standards),
and validates it by executing an Ethereum block end-to-end on `spike`.

The proven program is the standalone Sail model in `../evm/` (a real, block-executable
EVM). Here it is lowered Sail → C → `riscv64im_zicclsm-unknown-none-elf` and run on the
bare ISA simulator, with the standard IO interface, guard regions, and termination
semantics wired up.

## Result (validated)

```
$ ./build.sh run
[zkvm] EL-IR Sail EVM guest on riscv64im_zicclsm (GMP-free)
input_size=0
gas_used=43106
storage0=42
withdrawal_balance=500
coinbase_balance=43106
trace_len=29
tx_success=1
[zkvm] block executed: all fixture facts verified. SUCCESS.
--- spike exit code: 0 ---
```

`gas_used=43106`, `storage0=42`, `withdrawal_balance=500`, `tx_success=1` match the
native `make -C el-ir run-example` fixture exactly — the block (a 1-tx block: `PUSH1 0x2a;
PUSH1 0x00; SSTORE; STOP`, plus a 500-wei withdrawal) produces identical state and gas on
the RISC-V guest as in the Sail/C reference. A mismatch is an abnormal termination, so a
wrong execution cannot exit 0.

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
$ ./build.sh run                 # default fixture vector (valid block)
successful_validation=1          # public output byte 32; public_output_bytes=105
--- spike exit code: 0 ---
$ VEC=vectors/fixture_block_bad.ssz ./build.sh run   # tampered state_root
successful_validation=0          # a failed validation is a NORMAL result
--- spike exit code: 0 ---
```

The full design + stage-by-stage validation is in `STATELESS.md`. Vectors are produced
host-side by `gen_vector.py` (`--bad` for the fail path); the SSZ codec, RLP decode,
witness-MPT walk, and SSZ `hash_tree_root` live in `../evm/{ssz,rlp_decode,mpt_witness,
sha256,ssz_htr}.sail` (model-level, not pulled into `evm.sail`).

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
**unmapped** and enforced (validated — see `./build.sh traptest`):

* **Null-pointer trap [0x0, 0xFFF]** — guest memory starts at `0x80000000`, so the entire
  low range (incl. the first 4 KiB) is unmapped; a null access faults.
* **Stack guard (≥ 4 KiB)** — the stack lives in its own region `[0x90010000, 0x90110000)`;
  the 64 KiB gap `[0x90000000, 0x90010000)` immediately below the stack bottom is left out
  of the simulator's memory, so a stack overflow that decrements past the bottom faults.

The simulator memory ranges must match (the guard gap is deliberately omitted):

```
spike --isa=rv64im --misaligned -m0x80000000:0x10000000,0x90010000:0x00100000 <elf>
```

Validated guard enforcement:

```
$ ./build.sh traptest 0       # null-pointer read at 0x0
[zkvm] ABNORMAL TERMINATION (trap): load access fault ... mcause=0x5 mtval=0x0
--- spike exit code: 134 ---
$ ./build.sh traptest 1       # read into the stack guard gap
[zkvm] ABNORMAL TERMINATION (trap): load access fault ... mtval=0x9000ffc0
--- spike exit code: 134 ---
```

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
  build.sh              driver: derisk | guest | run | traptest [0|1] | clean
  zkvm_block.sail       Sail guest entry (runs the block; exposes result facts in registers)
  zkvm_io.h             the standard IO header (verbatim from zkvm-standards)
  runtime/
    link.ld             vendor linker script (null trap + stack guard regions)
    start.S             machine-mode crt0 + trap vector (platform glue; uses Zicsr)
    htif.c/.h           HTIF console + exit (spike host channel; validation only)
    runtime.c           freestanding libc subset + allocator + termination mapping
    zkvm_io.c           read_input / write_output (io-interface standard)
    harness.c           drives model_init → zkvm_run → write_output → terminate
    derisk_main.c       minimal HTIF smoke test
    traptest_main.c     guard-region enforcement test
    freestanding/       minimal hosted-header shims (decouple from newlib)
    sailfix/            GMP-free fixed-width Sail runtime (512-bit int, 256-bit lbits)
```

## Building / running

Requires `sail` (opam), `riscv64-unknown-elf-gcc`, and `spike` on `PATH`
(`eval $(opam env --root=$HOME/.opam --switch=sail)` for sail).

```
./build.sh run            # build the GMP-free guest and execute the block on spike
./build.sh guest          # build only (produces build/zkvm_guest.elf)
./build.sh traptest 0     # validate the null-pointer guard region
./build.sh traptest 1     # validate the stack guard region
./build.sh derisk         # HTIF harness smoke test
./build.sh clean
```

## Scope / follow-ups

* **Block-from-input**: DONE — the block + execution witness are deserialized from the
  canonical schema-prefixed SSZ `SszStatelessInput` in the private input, and the guest
  validates an arbitrary input block statelessly. See `STATELESS.md` for the design and
  the remaining refinements (block_hash recompute, sender-from-`public_keys`, non-empty
  execution requests, lazy arbitrary-access witness reads).
* **All crypto goes through the `c-interface-accelerators` standard.** keccak256, sha256,
  ripemd160, and the EVM precompiles share one boundary: the vendored standard header
  `../ffi/zkvm_accelerators.h` (verbatim from eth-act/zkvm-standards), implemented by the
  GMP-free, freestanding reference `../ffi/zkvm_accelerators.c` (`zkvm_keccak256`/`zkvm_sha256`/
  `zkvm_ripemd160` real; ecrecover/modexp/bn254/blake2f/KZG return `ZKVM_EFAIL` pending impl).
  The Sail model calls these via a thin marshalling shim (`../ffi/acc_shim.c`, externs
  injected with `sail -c --c-include acc_shim.h`), since Sail cannot form a `(ptr,len)` call.
  keccak/sha256 are exercised + asserted on-guest by `keccak_selfcheck`/`sha256_selfcheck`.
  A real zkVM host swaps `zkvm_accelerators.c` for its native precompiles behind the
  unchanged standard header. (The Sail-shim staging adds a buffer pass vs. a direct C
  `(ptr,len)` call — a marshalling artifact that a C guest or precompile-circuit host avoids.)
* `start.S`/trap-vector use Zicsr (machine-mode CSRs) — these are **platform/crt0 glue**
  (a vendor responsibility under the memory-layout standard), not the proven STF, which
  stays pure `rv64im_zicclsm`.
* The HTIF console/exit channel is spike's host interface, used only by the validation
  harness; a real zkVM host provides the equivalent.

## Runtime provenance

`runtime/sailfix/sail.h` is the stock Sail C runtime header with the GMP types swapped for
the fixed-width `sail_int`/`lbits` above; `runtime/sailfix/sail.c` is an original GMP-free
implementation of the int/bits primitives. No GMP/`mini-gmp` source is vendored or linked.
