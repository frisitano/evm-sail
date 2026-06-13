/* ======================================================================== */
/* evm-sail: a formal, executable specification of the Ethereum EVM         */
/*                                                                          */
/* Root include, in dependency order. The model is structured as:           */
/*                                                                          */
/*   host/    the HOST: world state, per-frame memory, the kernel-function  */
/*            surface (k_*), and the crypto accelerators. Pure mechanism -- */
/*            no gas, no policy.                                            */
/*   evm/     the TRANSACTION KERNEL (= the EVM): the opcode interpreter,   */
/*            the gas counter and full fork-gated schedule, transaction     */
/*            validity, and the state transition. It operates over user     */
/*            accounts (user space) and touches world state only through    */
/*            the host kernel functions.                                    */
/*   lib/     pure functions: RLP (encode + decode), the block-header hash, */
/*            and the Merkle-Patricia trie (state root + witness).          */
/*            keccak/sha256 themselves live in host/io.sail.                */
/*                                                                          */
/* Reading guide -- the state transition, outermost to innermost:           */
/*                                                                          */
/*   block.sail        process_block: run each tx, then apply withdrawals   */
/*     transaction.sail  process_transaction: validity, intrinsic gas, fee  */
/*                       deduction, nonce; then the top frame; refunds      */
/*       interpreter.sail interpret: the fetch/execute loop; run_call /     */
/*                       run_create build sub-frames; precompile dispatch   */
/*         execute.sail   per-opcode semantics -- pure compute + gas here,  */
/*                       every world effect issued as a k_* host call       */
/*           kernel.sail  the k_* effect/query surface over host/state.sail */
/*                                                                          */
/* gas.sail holds the schedule + charging; machine.sail the per-frame       */
/* registers/stack; exceptions.sail the halt kinds. The post-state is       */
/* authenticated against the block's stateRoot by the Merkle-Patricia trie, */
/* lib/mpt.sail -- which also carries the stateless witness reader (its     */
/* section 3) for proving reads against a committed root.                   */
/* ======================================================================== */

/* shared value model: 256-bit words, byte lists, addresses */
$include "prelude.sail"

/* host I/O: the accelerator FFI binding, host crypto (keccak/sha256 +
 * precompiles), and the stateless SSZ input decoders. Included early because the
 * crypto is used by block_hash/state/kernel/gas/transaction. */
$include "host/io.sail"

/* pure library functions: RLP encode (YP Appendix B), then the block-header
 * hash (keccak256(rlp(header)), YP section 4.3) that seals a block. */
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

/* RLP decode (inverse of lib/rlp.sail), then the Merkle-Patricia trie: the root
 * builder, the Ethereum state trie, and the stateless witness reader (post-state
 * authentication against stateRoot + witness-backed reads). SSZ hash_tree_root
 * (lib/ssz_htr.sail) is a consensus-layer merkleization included only by the
 * zkVM block harness (zkvm/), not by this core EVM model. */
$include "lib/rlp_decode.sail"
$include "lib/mpt.sail"
