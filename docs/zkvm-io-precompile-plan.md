# zkVM IO And Precompile Buffer Plan

## Context

The zkVM standards expose private input and public output as pointer/length
spans:

- `read_input(const uint8_t **buf_ptr, size_t *buf_size)` returns the private
  witness/input bytes.
- `write_output(const uint8_t *output, size_t size)` appends public output
  bytes.

The model should preserve that shape internally. Bytes that already live in a C
buffer should not be streamed through Sail one byte at a time unless Sail is
actually inspecting those bytes as model data.

## Current Shape

- `zkvm/runtime/zkvm_input.c` already keeps the standards input as a stable
  pointer/length span. The Sail SSZ decoder reads scalar bytes and small
  64-bit chunks through FFI because Sail cannot dereference host pointers.
- `ffi/memory.c` owns EVM memory, calldata views, transaction input views, and
  returndata slots. Range copies such as `CALLDATACOPY`, `CODECOPY`,
  `RETURNDATACOPY`, and `MCOPY` are C-side bulk copies.
- `ffi/acc_shim.c` stages accelerator input in `ACC_in`, writes results into a
  global `ACC_out`, and then `hr_capture_acc` copies `ACC_out` into pending
  returndata.
- Memory-sourced `KECCAK256` is already staged with `acc_begin_mem`, avoiding a
  Sail list for the input range.

## Decisions

- Keep the zkVM standard IO surface as pointer/length only. Do not bake a byte
  protocol into the standards layer.
- Keep Sail responsible for specification decisions: which bytes are hashed,
  precompile gas/accounting rules, success/failure semantics, and returndata
  adoption timing.
- Keep C responsible for bulk byte movement at host boundaries: memory spans,
  calldata spans, transaction input spans, accelerator input, accelerator
  output, and public output flushes.
- Do not use one global accelerator output buffer as permanent EVM returndata.
  Later accelerators could overwrite it before `RETURNDATACOPY`.

## Planned Refactor

1. Split accelerator output roles:
   - Hash/list APIs keep a small scratch output readable through `acc_word` and
     `acc_out`.
   - Staged precompile execution writes directly into pending returndata.
2. Add an accelerator execution path for staged precompiles that receives the
   pending returndata buffer as its output destination. `hr_capture_acc` then
   becomes a finalizer for the pending length instead of copying bytes.
3. Preserve the current snapshot semantics for precompile input. A precompile
   input slice must not alias memory that later receives the precompile output.
4. Expand direct memory-span reads where safe:
   - Hashes can read memory in place.
   - Precompile inputs should continue to snapshot unless a specific operation
     is proven non-overlapping and non-mutating.
5. Buffer public zkVM output once for all non-spike guest paths, then call
   `write_output(ptr, len)` once from the wrapper. The spike harness may keep
   its observability-oriented output handling, but the canonical result should
   remain a contiguous buffer.

## Validation

After C/FFI changes, run:

```sh
rtk make check
rtk git diff --check
cd revm-eest
rtk python3 run_eest.py fixtures/smoke/state_root_transfer.json fixtures/smoke/state_root_precompile.json --fork Cancun --quiet --timeout 30 --root --rebuild
rtk python3 run_eest.py fixtures/eels/shanghai_push0/state_tests/for_shanghai --fork Shanghai --quiet --timeout 30 --root --rebuild
rtk python3 run_eest.py fixtures/eels/cancun_selfdestruct/state_tests/for_cancun --fork Cancun --quiet --timeout 30 --root --rebuild
```
