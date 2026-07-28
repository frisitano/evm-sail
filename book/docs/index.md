# EVM Sail Specification

An executable specification of the Ethereum execution layer, written in
[Sail](https://github.com/rems-project/sail). It defines the state
transition function — transaction validity and execution, the EVM, gas,
precompiled contracts, and the state trie — together with the stateless
validation of execution payloads. Every rule cites the Yellow Paper
section or EIP it implements; fork-dependent rules are gated on the
[Fork][type-Fork] ordering rather than duplicated per fork.

## The state transition

- [Transactions](reference/sail/evm/transaction.md) — validity, upfront
  effects, execution, and settlement (YP §6).
- [Block execution](reference/sail/executor/block.md) — system calls, the
  transaction loop, withdrawals, and requests.
- [Stateless validation](reference/sail/executor/stateless.md) — the
  commitments an executed block must satisfy.
- [Exceptions](reference/sail/exceptions.md) — exceptional halts and
  block-validation failures.

## The EVM

- [Halting](reference/sail/evm/halt.md) — normal stops, exceptional halts,
  and frame status.
- [The interpreter](reference/sail/evm/interpreter.md) — fetch/decode, the
  run loop, and message calls.
- [Opcode semantics](reference/sail/evm/execute.md) — the single-step
  transition function.
- [The instruction set](reference/sail/evm/instructions.md).
- [The gas schedule](reference/sail/evm/gas.md).
- [Precompiled contracts](reference/sail/evm/precompiles.md).
- [Machine state](reference/sail/evm/machine.md) — the per-message-call
  registers, stack, and memory.

## State and data structures

- [Accounts](reference/sail/primitives/account.md),
  [Transactions](reference/sail/primitives/tx.md),
  [Blocks](reference/sail/primitives/block.md) — the protocol data types.
- [The Merkle-Patricia trie](reference/sail/lib/mpt/trie.md) and the
  [state trie](reference/sail/lib/state_trie.md).
- [RLP](reference/sail/lib/rlp/rlp.md) and
  [SSZ](reference/sail/lib/ssz/ssz.md) serialization;
  [hash_tree_root](reference/sail/lib/htr.md).
- [Cryptographic primitives](reference/sail/primitives/crypto.md).

## Architecture

The model is split into a **user-space machine** (the per-message-call
compute state: program counter, stack, memory, gas) and a **state kernel**
(the world-state semantics for accounts, storage, logs, and transactional
overlays). Every world effect crosses that split as an explicit `k_*`
kernel call, and world rollback is a kernel snapshot/revert. The kernel is
part of the normative Sail specification under `kernel/`.

Beneath both sits the **host interface**, with two facets: **regions** —
the stateless input, frame memory, code, log data, output, and scratch stores,
each addressed by its own nominal `{off, len}` slice type without runtime
source dispatch — and the **accelerator interface**, the cryptographic
functions the implementation computes (hashing, secp256k1, the precompile
accelerators). Pages under `host/` document that interface and are
**non-normative**: they specify the model's internal contracts, not
protocol rules.
