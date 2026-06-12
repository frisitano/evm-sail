/* ======================================================================== */
/* EL-IR : a standalone, block-executable EVM in Sail, structured as a        */
/*         USER-SPACE / KERNEL split.                                         */
/*                                                                           */
/*   EVM (user space)  : the opcode interpreter + ALL gas/policy + the gas    */
/*                       counter. Pure compute; decides whether effects happen.*/
/*   EL  (kernel/host) : the privileged world state, reached only via k_*     */
/*                       syscalls, each recorded into `el_trace`.             */
/*                                                                           */
/* The EVM issues a kernel syscall ONLY for a real effect (e.g. a no-op       */
/* SSTORE charges gas but issues no EL_SStore). `el_trace` is the resulting   */
/* sequence of authenticated effects -- the artifact that lowers to the AIR   */
/* interface channels.                                                        */
/*                                                                           */
/* Build:  make check        Run a block:  make run-example                   */
/* ======================================================================== */

/* shared: value model + 256-bit ALU + list/byte helpers */
$include "prelude_evm.sail"

/* zkVM crypto accelerators (eth-act zkvm-standards): keccak/sha256/precompiles */
$include "accelerators.sail"

/* real Keccak-256 (Ethereum variant) + CREATE/CREATE2 address derivation */
$include "keccak.sail"

/* RLP encoding (for accounts, headers, tries, block hash) */
$include "rlp.sail"

/* Merkle-Patricia trie root (state/storage/tx/receipt roots) */
$include "mpt.sail"

/* stack-based MPT root builder (alloy-trie HashBuilder; partial-trie aware) */
$include "hash_builder.sail"

/* block header RLP + block hash */
$include "block_hash.sail"

/* halt / exception outcomes (shared) */
$include "exceptions.sail"

/* ---------------------------- KERNEL (host) ----------------------------- */
/* world state + the EL syscall ISA + the recorded effect trace */
$include "el_kernel.sail"

/* --------------------------- EVM (user space) --------------------------- */
/* per-frame memory (frame-local scratch) */
$include "memory.sail"
/* frame registers + gas counter + operand stack + saved-frame stack */
$include "machine.sail"
/* gas schedule + policy helpers (pure) */
$include "gas.sail"
/* the opcode instruction surface */
$include "instructions.sail"
/* per-opcode semantics (gas/policy in EVM, effects via k_* syscalls) */
$include "execute.sail"
/* fetch/decode, run loop, calls/creates, precompiles */
$include "interpreter.sail"
/* intrinsic gas, state transition, refunds, receipt */
$include "transaction.sail"
/* execute a whole block */
$include "block.sail"
/* post-state root: secure state trie over the kernel accounts (block auth) */
$include "state_root.sail"
