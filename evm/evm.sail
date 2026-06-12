/* ======================================================================== */
/* evm-sail: a formal, executable specification of the Ethereum EVM           */
/*                                                                           */
/* Root include, in dependency order. The model is structured as:            */
/*                                                                           */
/*   host/    the HOST: world state, per-frame memory, the kernel-function    */
/*            surface (k_*), and the crypto accelerators. Pure mechanism --   */
/*            no gas, no policy.                                              */
/*   kernel/  the TRANSACTION KERNEL (= the EVM): the opcode interpreter,    */
/*            the gas counter and full fork-gated schedule, transaction       */
/*            validity, and the state transition. It operates over user       */
/*            accounts (user space) and touches world state only through      */
/*            the host kernel functions.                                      */
/*   lib/     pure functions: keccak/sha256, RLP, the Merkle-Patricia trie,   */
/*            state root, block hash, SSZ, and the stateless witness.         */
/* ======================================================================== */

/* shared value model: 256-bit words, byte lists, addresses */
$include "prelude.sail"

/* host crypto accelerators (eth-act zkvm-standards C boundary) */
$include "host/accelerators.sail"

/* pure library functions */
$include "lib/keccak.sail"
$include "lib/rlp.sail"
$include "lib/mpt.sail"
$include "lib/hash_builder.sail"
$include "lib/block_hash.sail"

/* halt / exceptional-halt kinds */
$include "exceptions.sail"

/* the host: world state + the kernel-function surface */
$include "host/state.sail"
$include "host/kernel.sail"
$include "host/memory.sail"

/* the transaction kernel (the EVM) */
$include "kernel/machine.sail"
$include "kernel/gas.sail"
$include "kernel/instructions.sail"
$include "kernel/execute.sail"
$include "kernel/interpreter.sail"
$include "kernel/transaction.sail"
$include "kernel/block.sail"

/* post-state authentication */
$include "lib/state_root.sail"
