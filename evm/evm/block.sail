/* ======================================================================== */
/* BLOCK PROCESSING                                                         */
/*                                                                          */
/* The block-level state transition (Yellow Paper section 11): given a      */
/* block environment, an ordered transaction list, and the consensus-layer  */
/* withdrawals, evolve the world state and return one receipt per           */
/* transaction.                                                             */
/*                                                                          */
/* Transactions are applied in order, each as its own state-transition step */
/* (process_transaction, transaction.sail); withdrawals (EIP-4895) are then */
/* credited at block end. The receipts carry the per-tx success flag, gas   */
/* used, and the logs the header's receiptsRoot/logsBloom commit to.        */
/* ======================================================================== */

/* A consensus-layer withdrawal credited at block end (EIP-4895). w_amount is
 * the value added directly to w_address's balance by apply_withdrawals; the
 * Sail model credits this word as-is (no Gwei->wei scaling is applied here). */
struct Withdrawal = { w_address : address, w_amount : word }

/* A block: its execution environment, ordered transactions, and the
 * post-execution withdrawals (EIP-4895). */
struct Block = { blk_env : BlockEnv, blk_txs : list(Transaction), blk_withdrawals : list(Withdrawal) }

function process_transactions(txs : list(Transaction)) -> list(Receipt) =
    match txs {
        [||]       => [||],
        tx :: rest => {
            let r = process_transaction(tx);
            r :: process_transactions(rest)
        },
    }

function apply_withdrawals(ws : list(Withdrawal)) -> unit =
    match ws {
        [||]      => (),
        w :: rest => {
            k_add_balance(w.w_address, w.w_amount);
            apply_withdrawals(rest)
        },
    }

/* Seed the block environment in the kernel, run the txs, apply withdrawals. */
function process_block(blk : Block) -> list(Receipt) = {
    k_set_block(blk.blk_env);
    let receipts = process_transactions(blk.blk_txs);
    apply_withdrawals(blk.blk_withdrawals);
    receipts
}

/* Cumulative gas used across the block's receipts (the header's gasUsed field). */
function total_gas_used(rs : list(Receipt)) -> gas =
    match rs {
        [||] => 0,
        r :: rest => to_gas(r.rc_gas_used + total_gas_used(rest)),
    }
