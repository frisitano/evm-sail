/* ======================================================================== */
/* Block processing: execute a full Ethereum block over the kernel state      */
/* ======================================================================== */

/* A consensus-layer withdrawal credited at block end (EIP-4895); w_amount is
 * in Gwei and is scaled to wei when applied (apply_withdrawals). */
struct Withdrawal = { w_address : address, w_amount : word }

/* A block: its environment, ordered transactions, and withdrawals. */
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

function total_gas_used(rs : list(Receipt)) -> gas =
    match rs {
        [||] => 0,
        r :: rest => to_gas(r.rc_gas_used + total_gas_used(rest)),
    }
