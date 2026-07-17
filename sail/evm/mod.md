# The EVM

The Ethereum Virtual Machine (YP §9): the quasi-Turing-complete stack
machine that executes account code. Execution proceeds in frames; each
frame owns an operand stack, a byte-addressed memory, a program counter,
and a gas counter, and message calls and contract creations push new
frames. Every operation is metered — a frame halts exceptionally the
moment its gas is exhausted — so execution is total by construction.

Read the pages in order:

- [Machine state](machine.md) — the frame: operand stack, memory,
  program counter, gas, and the halting discipline.
- [Halting](halt.md) — normal stops, exceptional halts, and frame status.
- [The instruction set](instructions.md) — the opcode space and its
  fork-gated availability.
- [The interpreter](interpreter.md) — the fetch–decode–dispatch run
  loop.
- [Opcode semantics](execute.md) — the operational rules of every
  instruction.
- [The gas schedule](gas.md) — intrinsic costs, memory expansion, and
  access-list pricing (EIP-2929).
- [Precompiled contracts](precompiles.md) — the address-mapped native
  contracts and their gas rules.
- [The transaction state transition](transaction.md) — validity,
  intrinsic gas, the fee market, and the frame that a transaction
  ultimately runs (YP §6).
