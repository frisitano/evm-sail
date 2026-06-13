/* ======================================================================== */
/* evm-sail: a formal, executable specification of the Ethereum EVM           */
/*                                                                           */
/* Root include, in dependency order. The model is structured as:            */
/*                                                                           */
/*   host/    the HOST: world state, per-frame memory, the kernel-function    */
/*            surface (k_*), and the crypto accelerators. Pure mechanism --   */
/*            no gas, no policy.                                              */
/*   evm/     the TRANSACTION KERNEL (= the EVM): the opcode interpreter,     */
/*            the gas counter and full fork-gated schedule, transaction       */
/*            validity, and the state transition. It operates over user       */
/*            accounts (user space) and touches world state only through      */
/*            the host kernel functions.                                      */
/*   lib/     pure functions: keccak/sha256, RLP, the Merkle-Patricia trie,   */
/*            state root, block hash, SSZ, and the stateless witness.         */
/*                                                                           */
/* Reading guide -- the state transition, outermost to innermost:            */
/*                                                                           */
/*   block.sail        process_block: run each tx, then apply withdrawals     */
/*     transaction.sail  process_transaction: validity, intrinsic gas, fee    */
/*                       deduction, nonce; then the top-level frame; refunds   */
/*       interpreter.sail interpret: the fetch/execute loop; run_call /        */
/*                       run_create build sub-frames; precompile dispatch      */
/*         execute.sail   per-opcode semantics -- pure compute + gas here,     */
/*                       every world effect issued as a k_* host call          */
/*           kernel.sail  the k_* effect/query surface over host/state.sail    */
/*                                                                           */
/* gas.sail holds the schedule + charging; machine.sail the per-frame         */
/* registers/stack; exceptions.sail the halt kinds. The post-state is         */
/* authenticated by lib/mpt.sail (and, statelessly, lib/mpt_witness.sail). */
/* ======================================================================== */

/* shared value model: 256-bit words, byte lists, addresses */
$include "prelude.sail"

/* host crypto accelerators (eth-act zkvm-standards C boundary) */
$include "host/ffi.sail"
$include "host/crypto.sail"

/* pure library functions */
$include "lib/rlp.sail"
$include "lib/block_hash.sail"

/* halt / exceptional-halt kinds */
$include "exceptions.sail"

/* the host: world state + the kernel-function surface */
$include "host/state.sail"
$include "host/kernel.sail"
$include "host/memory.sail"

/* the transaction kernel (the EVM) */
$include "evm/machine.sail"
$include "evm/gas.sail"
$include "evm/instructions.sail"
$include "evm/execute.sail"
$include "evm/interpreter.sail"
$include "evm/transaction.sail"
$include "evm/block.sail"

/* post-state authentication */
$include "lib/mpt.sail"
