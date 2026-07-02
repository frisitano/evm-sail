# HolBA Equivalence Assessment

Date: 2026-07-02

Scope: local repository inspection only. I did not build HolBA or run its examples in this pass.

## Local Snapshots

- HolBA: `/Users/f/dev/ethereum/holba`, `21be5becd87644a956d77a653b2d9838b451922c`, branch `master`
- Sail: `/Users/f/dev/ethereum/sail`, `7ed92aaec7f9c59718aeef6d9385f851c2fc240e`, branch `sail2`
- ere-guests: `/Users/f/dev/ethereum/ere-guests`, `afc3f4e0fdd8f23353469fdc088af51c3a765194`
- evm-sail worktree: `/Users/f/dev/ethereum/evm-sail/.worktrees/refactor-evm-sail-formal-verification-alignment`

## Executive Conclusion

HolBA is useful for this project, but not as a replacement for evm-sail or as an immediate whole-guest equivalence engine.

The best role for HolBA is as a proof-producing binary-side contract engine:

1. Lift selected RISC-V binary slices into BIR.
2. Prove BIR contracts by symbolic execution or weakest precondition.
3. Backlift those contracts into the HOL4 RISC-V machine state.
4. Prove that the resulting binary contract refines a contract derived from evm-sail, which remains the source of truth.

The main blocker is ISA alignment. HolBA's RISC-V examples are documented around RV64G and use word64-heavy contracts. The relevant Reth zkVM guest artifacts in `ere-guests` appear RV32IM-oriented, especially OpenVM and RISC0. Unless we can compile the exact guest slice as RV64G, or confirm HolBA can lift the exact RV32IM instructions we need, HolBA should be treated as a promising spike path rather than the primary proof plan.

## What HolBA Provides

HolBA is a HOL4 binary analysis library. Its useful components for us are:

- L3 ISA semantics for ARMv8, Cortex-M0, and RISC-V.
- A proof-producing lifter from machine code/disassembly into BIR, HolBA's Binary Intermediate Representation.
- Symbolic execution and weakest-precondition tooling over BIR.
- SMT integration through HOL4/HolSmt with Z3 support.
- A proof-producing backlifter that can turn BIR contracts into ISA-level RISC-V contracts.
- Example-driven workflows for proving contracts about RISC-V functions.

The relevant local files are:

- `/Users/f/dev/ethereum/holba/README.md`
- `/Users/f/dev/ethereum/holba/src/tools/lifter/README`
- `/Users/f/dev/ethereum/holba/src/tools/lifter/bir_lifter_interfaceLib.sml`
- `/Users/f/dev/ethereum/holba/src/tools/backlifter/bir_backlifterLib.sig`
- `/Users/f/dev/ethereum/holba/src/tools/symbexec/bir_symbLib.sig`
- `/Users/f/dev/ethereum/holba/src/tools/wp/bir_wp_interfaceLib.sig`
- `/Users/f/dev/ethereum/holba/examples/riscv/README.md`
- `/Users/f/dev/ethereum/holba/examples/riscv/incr`
- `/Users/f/dev/ethereum/holba/examples/riscv/aes`

## The HolBA RISC-V Workflow

The documented RISC-V workflow is function-contract oriented.

First, compile a program and disassemble it into a `.da` file. The examples use objdump-style input such as:

```text
0000000000010488 <incr>:
   10488: 00150513 addi a0,a0,1
   1048c: 00008067 ret
```

Then lift an address range:

```sml
lift_da_and_store "incr" "incr.da" da_riscv ((0x10488), (0x10498));
```

This creates BIR program definitions and a lifting theorem tying the BIR program to the RISC-V machine-code bytes.

Then define:

- A RISC-V precondition over `riscv_state`.
- A RISC-V postcondition over `riscv_state`.
- A BIR precondition.
- A BIR postcondition.
- Bridge theorems from the RISC-V predicates to the BIR predicates.

The BIR contract can be discharged by symbolic execution:

```sml
bir_symb_analysis_thm
  bir_incr_prog_def
  incr_init_addr_def
  [incr_end_addr_def]
  bspec_incr_pre_def
  incr_birenvtyl_def
```

Finally, the backlifter produces a theorem of the form:

```sml
riscv_cont bir_incr_progbin incr_init_addr {incr_end_addr}
  (riscv_incr_pre pre_x10)
  (riscv_incr_post pre_x10)
```

This is exactly the style of theorem we would want for compiled guest slices, provided the ISA and ABI match the real guest binary.

## Compatibility Risks

### 1. RV64G Examples Versus RV32IM zkVM Guests

The biggest practical issue is that HolBA's RISC-V examples state that programs should be provided in `.da` format for the RV64G instruction set. The examples also use `word64` registers and addresses pervasively.

The Reth guest artifacts in `ere-guests` appear to target zkVM-specific RV32 variants:

- `bin/stateless-validator-reth/openvm/openvm.toml` has RV32I and RV32M configuration.
- `bin/stateless-validator-reth/openvm/Cargo.toml` depends on `openvm-rv32im-guest`.
- `bin/stateless-validator-reth/risc0/Cargo.lock` contains `risc0-circuit-rv32im`.

HolBA's generated RISC-V model has architecture constructors for `RV32I`, `RV64I`, and `RV128I`, so RV32 is not conceptually absent. The risk is that the end-to-end lifter/backlifter examples, helper libraries, and proof scripts we would rely on may currently be exercised mainly on RV64G.

This must be tested before we make HolBA central.

### 2. HolBA Uses L3/HOL4 Semantics, Not Sail Semantics

HolBA proves facts against its L3 RISC-V semantics and BIR. evm-sail is the source of truth for EVM semantics. That means there is no direct theorem of the shape:

```text
RISC-V ELF implements evm-sail
```

The realistic theorem stack is:

```text
HolBA theorem:
  selected RISC-V code satisfies binary contract C_bin

Bridge theorem:
  C_bin implies C_evm

evm-sail theorem or trusted extraction:
  C_evm is the evm-sail source-of-truth behavior for the selected EVM operation
```

The bridge theorem is the important work. HolBA can help prove the binary side, but it does not remove the need to state evm-sail contracts precisely.

### 3. Whole-Guest Proof Is Not Practical

The Reth guest is large, compiled Rust. A monolithic proof that the whole ELF implements evm-sail is not a realistic first target.

The practical target is modular:

- Prove small, stable guest functions or subsystems.
- Use debug symbols and intentionally compiled proof builds to keep function boundaries visible.
- Treat zkVM platform APIs, host I/O, precompile calls, and allocator behavior as explicit contracts.
- Compose those contracts toward larger execution-level invariants.

This is compatible with HolBA's examples, which are function-contract oriented.

### 4. Disassembly Is an Input Assumption

HolBA's examples lift from objdump-style `.da` files and manual address ranges. For a secure proof workflow, we need to control:

- Which ELF was compiled.
- Which exact bytes are in the proof target.
- Which disassembler produced the `.da` file.
- Which symbol and address range were lifted.
- Whether the lifted bytes are checked against the ELF bytes.

The lifting theorem includes machine-code bytes, which is good, but the surrounding artifact pipeline still needs hardening.

## Recommended Architecture

HolBA should be used as a binary proof backend, not as the semantic source.

```text
                  evm-sail
                     |
        source-of-truth EVM contract
                     |
       HOL4/Sail-exported or hand-written
          operation-level predicate
                     |
                bridge theorem
                     |
       HolBA RISC-V/BIR binary contract
                     |
       selected Reth guest ELF function
```

The evm-sail side should define operation-level contracts such as:

- Given opcode inputs and EVM state witness, the post-state and outputs match evm-sail.
- Given a precompile input buffer, the return buffer and gas behavior match evm-sail.
- Given a trie/state witness operation, account and storage updates match the evm-sail state transition.

The HolBA side should prove that selected compiled functions satisfy corresponding low-level contracts over registers, memory, and return values.

The bridge theorem maps:

- RISC-V memory layout to Sail values.
- Guest ABI registers to function arguments and return values.
- Serialized buffers to EVM objects.
- Guest error codes to evm-sail failure modes.
- Platform calls to assumed contracts.

## Where Sail Can Help

The local Sail repo documents theorem-prover generation for Coq, Isabelle, and HOL4 through Lem. That suggests a possible route for moving selected evm-sail contracts into HOL4:

1. Keep evm-sail as the source.
2. Carve out small pure Sail functions or adapter predicates.
3. Try Sail-to-Lem-to-HOL4 export for those small pieces.
4. Prove HolBA binary contracts imply the exported HOL4 Sail contract.

This should be a spike, not an assumption. Full evm-sail HOL4 export may be too large or may need code-shape restrictions. The safer first step is a small adapter contract, for example a precompile buffer transformation or a simple opcode state transition.

## Suggested Spikes

### Spike 1: Build HolBA Examples

Goal: confirm the local HolBA checkout, HOL4 dependency, and Z3 setup actually work.

Steps:

1. Install or locate the HOL4 version HolBA expects.
2. Set `HOL4_Z3_EXECUTABLE`.
3. Build HolBA.
4. Build `examples/riscv/incr`.
5. Confirm the final `riscv_cont` theorem is produced.

Success criterion: the unmodified `incr` proof builds locally.

### Spike 2: Test Exact ISA Compatibility

Goal: determine whether HolBA can lift the same ISA profile used by the Reth zkVM guest.

Steps:

1. Compile a tiny function with the same target/toolchain as the selected `ere-guests` backend.
2. Preserve symbols and disable inlining for the function under test.
3. Run `file`, `readelf -h`, and objdump on the ELF.
4. Try lifting the function's `.da` range with HolBA.

Success criterion: HolBA lifts the real guest-target instruction subset and produces a lifting theorem.

Failure criterion: the lifter rejects RV32IM instructions or requires RV64G-only assumptions. If this happens, HolBA should not be used as the main path until the lifter is extended.

### Spike 3: Prove One Guest Slice Contract

Goal: prove a small compiled Rust guest function satisfies a contract.

Candidate slices:

- A simple byte-buffer copy or length check.
- A precompile input parser with no host calls.
- A small state-key hashing wrapper, if the hash itself is stubbed.
- A simple return-buffer encoding function.

Avoid first:

- Full block execution.
- Full transaction execution.
- Keccak internals.
- Trie recursion.
- Allocator-heavy or panic-heavy Rust paths.

Success criterion: HolBA proves a BIR contract and backlifts it to a RISC-V contract for a function compiled from the Reth guest tree.

### Spike 4: Bridge One Contract To evm-sail

Goal: prove that a binary-side contract implies a small evm-sail-side contract.

Two viable variants:

- Hand-write the evm-sail-side predicate in HOL4 for one small function-shaped behavior.
- Export a small Sail adapter through Sail's HOL4 path and prove against that.

Success criterion: a HOL4 theorem states that the RISC-V contract implies the evm-sail-derived contract for the chosen slice.

## Decision

HolBA is worth using as a complementary proof track.

It is strongest where we need a high-assurance theorem about concrete RISC-V machine code. It is weaker as a whole-system equivalence strategy because:

- The current guest target appears RV32IM while HolBA's ready-made examples are RV64G.
- evm-sail is not in HolBA's semantic universe by default.
- Whole compiled Rust guest verification is too large without modular contracts.

The near-term plan should be:

1. Keep Sail/Isla as the primary spec-side and ISA-semantics exploration path.
2. Add HolBA as a binary-side spike for selected RISC-V functions.
3. First prove that HolBA can lift the exact Reth guest ISA profile.
4. Only then invest in evm-sail-to-HOL4 bridge work.

If Spike 2 succeeds, HolBA becomes a serious candidate for proof-producing modular compliance theorems. If Spike 2 fails, HolBA remains useful as a reference architecture for binary proofs, but not as the immediate tool for the current Reth zkVM ELF.
