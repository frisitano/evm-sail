/* ======================================================================== */
/* EVM USER-SPACE instruction semantics: execute(op)                         */
/*                                                                           */
/* Pure compute is done here; every world effect is a kernel syscall (k_*).   */
/* All gas/policy stays in the EVM: the EVM marks-and-prices access via the    */
/* kernel's returned warm bit, decides whether an effect happens, and issues  */
/* the syscall only for real effects (e.g. a no-op SSTORE charges gas but     */
/* issues no host write). Sub-calls/creates delegate to run_call/run_create.  */
/* ======================================================================== */

val run_call : int -> unit /* 0=CALL 1=CALLCODE 2=DELEGATECALL 3=STATICCALL */
val run_create : bool -> unit /* true = CREATE2 */

/* ----------------------------- helpers ---------------------------------- */

function self_addr() -> address = message.msg_address

function guard_static() -> bool =
    if message.msg_is_static then {
        exc_halt(WriteProtection);
        true
    } else {
        false
    }

function do_jump(dest : int) -> unit =
    if dest >= 0 & dest < code_len & hj_valid(get_slice_int(64, dest, 0)) then {
        pc = dest
    } else {
        exc_halt(InvalidJump)
    }

/* ------------------------------- execute -------------------------------- */

function execute(op : ast) -> unit = match op {
    STOP() => frame_status = Halted(HaltStop),
    /* ---- arithmetic (pure) ---- */
    ADD() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_add(a, b))
    },
    MUL() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_mul(a, b))
    },
    SUB() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_sub(a, b))
    },
    DIV() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_div(a, b))
    },
    SDIV() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_sdiv(a, b))
    },
    MOD() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_mod(a, b))
    },
    SMOD() => {
        charge(G_low);
        let a = pop();
        let b = pop();
        push(alu_smod(a, b))
    },
    ADDMOD() => {
        charge(G_mid);
        let a = pop();
        let b = pop();
        let n = pop();
        push(alu_addmod(a, b, n))
    },
    MULMOD() => {
        charge(G_mid);
        let a = pop();
        let b = pop();
        let n = pop();
        push(alu_mulmod(a, b, n))
    },
    EXP() => {
        let a = pop();
        let e = pop();
        charge(exp_gas(e));
        push(alu_exp(a, e))
    },
    SIGNEXTEND() => {
        charge(G_low);
        let bi = pop();
        let v = pop();
        push(alu_signextend(bi, v))
    },
    /* ---- comparison / bitwise (pure) ---- */
    LT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_lt(a, b))
    },
    GT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_gt(a, b))
    },
    SLT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_slt(a, b))
    },
    SGT() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_sgt(a, b))
    },
    EQ() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_eq(a, b))
    },
    ISZERO() => {
        charge(G_verylow);
        let a = pop();
        push(alu_iszero(a))
    },
    AND() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_and(a, b))
    },
    OR() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_or(a, b))
    },
    XOR() => {
        charge(G_verylow);
        let a = pop();
        let b = pop();
        push(alu_xor(a, b))
    },
    NOT() => {
        charge(G_verylow);
        let a = pop();
        push(alu_not(a))
    },
    BYTE() => {
        charge(G_verylow);
        let i = pop();
        let x = pop();
        push(alu_byte(i, x))
    },
    SHL() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_shl(s, v))
    },
    SHR() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_shr(s, v))
    },
    SAR() => {
        /* EIP-145 */
        charge(G_verylow);
        let s = pop();
        let v = pop();
        push(alu_sar(s, v))
    },
    CLZ() => {
        /* EIP-7939 */
        charge(G_low);
        let x = pop();
        push(alu_clz(x))
    },
    /* ---- keccak: hash via kernel primitive ---- */
    KECCAK256() => {
        let off = pop_int();
        let len = pop_int();
        charge(keccak_gas(len));
        charge_expansion(off, len);
        if is_running() then {
            push(mem_keccak(off, len))
        }
    },
    /* ---- environment / account (kernel reads, EVM prices them) ---- */
    ADDRESS() => {
        charge(G_base);
        push(address_to_word(self_addr()))
    },
    ORIGIN() => {
        charge(G_base);
        push(k_env(F_Origin))
    },
    CALLER() => {
        charge(G_base);
        push(address_to_word(message.msg_caller))
    },
    CALLVALUE() => {
        charge(G_base);
        push(message.msg_value)
    },
    GASPRICE() => {
        charge(G_base);
        push(k_env(F_GasPrice))
    },
    CALLDATASIZE() => {
        charge(G_base);
        push(word_of_int(message.msg_data_len))
    },
    CALLDATALOAD() => {
        charge(G_verylow);
        let off = pop_int();
        push(calldata_word(off))
    },
    CALLDATACOPY() => {
        charge(G_verylow);
        let dest = pop_int();
        let off = pop_int();
        let len = pop_int();
        charge(copy_gas(len));
        charge_expansion(dest, len);
        if is_running() then {
            calldata_copy_to_mem(dest, off, len)
        }
    },
    CODESIZE() => {
        charge(G_base);
        push(word_of_int(code_len))
    },
    CODECOPY() => {
        charge(G_verylow);
        let dest = pop_int();
        let off = pop_int();
        let len = pop_int();
        charge(copy_gas(len));
        charge_expansion(dest, len);
        if is_running() then {
            mem_codecopy(dest, off, len)
        }
    },
    BALANCE() => {
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm));
        push(k_get_balance(a))
    },
    SELFBALANCE() => {
        /* EIP-1884 */
        charge(G_low);
        push(k_get_balance(self_addr()))
    },
    EXTCODESIZE() => {
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm));
        push(word_of_int(k_get_code_size(a)))
    },
    EXTCODECOPY() => {
        let a = word_to_address(pop());
        let dest = pop_int();
        let off = pop_int();
        let len = pop_int();
        let warm = k_access_account(a);
        charge(account_cost(warm));
        charge(copy_gas(len));
        charge_expansion(dest, len);
        if is_running() then {
            k_code_copy(a, dest, off, len)
        }
    },
    EXTCODEHASH() => {
        /* EIP-1052 */
        let a = word_to_address(pop());
        let warm = k_access_account(a);
        charge(account_cost(warm));
        push(k_get_codehash(a))
    },
    RETURNDATASIZE() => {
        /* EIP-211 */
        charge(G_base);
        push(word_of_int(returndata_size()))
    },
    RETURNDATACOPY() => {
        /* EIP-211 */
        charge(G_verylow);
        let dest = pop_int();
        let off = pop_int();
        let len = pop_int();
        charge(copy_gas(len));
        charge_expansion(dest, len);
        if off + len > returndata_size() then {
            exc_halt(InvalidOpcode)
        } else {
            if is_running() then {
                returndata_copy_to_mem(dest, off, len)
            }
        }
    },
    /* ---- block (kernel reads) ---- */
    BLOCKHASH() => {
        charge(20);
        let n = pop();
        push(k_blockhash(n))
    },
    COINBASE() => {
        charge(G_base);
        push(k_env(F_Coinbase))
    },
    TIMESTAMP() => {
        charge(G_base);
        push(k_env(F_Timestamp))
    },
    NUMBER() => {
        charge(G_base);
        push(k_env(F_Number))
    },
    PREVRANDAO() => {
        charge(G_base);
        push(k_env(F_PrevRandao))
    },
    GASLIMIT() => {
        charge(G_base);
        push(k_env(F_GasLimit))
    },
    CHAINID() => {
        /* EIP-1344 */
        charge(G_base);
        push(k_env(F_ChainId))
    },
    BASEFEE() => {
        /* EIP-3198 */
        charge(G_base);
        push(k_env(F_BaseFee))
    },
    BLOBBASEFEE() => {
        /* EIP-7516 */
        charge(G_base);
        push(k_env(F_BlobBaseFee))
    },
    BLOBHASH() => {
        /* EIP-4844 */
        charge(G_verylow);
        let i = pop_int();
        push(k_blobhash(i))
    },
    /* ---- stack / memory (pure, user-space) ---- */
    POP() => {
        charge(G_base);
        let _ = pop()
    },
    MLOAD() => {
        charge(G_verylow);
        let off = pop_int();
        charge_expansion(off, 32);
        push(mem_load(off))
    },
    MSTORE() => {
        charge(G_verylow);
        let off = pop_int();
        let v = pop();
        charge_expansion(off, 32);
        mem_store(off, v)
    },
    MSTORE8() => {
        charge(G_verylow);
        let off = pop_int();
        let v = pop();
        charge_expansion(off, 1);
        mem_store_byte(off, v)
    },
    MSIZE() => {
        charge(G_base);
        push(word_of_int(mem_size_words() * 32))
    },
    MCOPY() => {
        /* EIP-5656 */
        charge(G_verylow);
        let dest = pop_int();
        let src = pop_int();
        let len = pop_int();
        charge(copy_gas(len));
        let hi = if dest > src then { dest } else { src };
        charge_expansion(hi, len);
        if is_running() then {
            mem_mcopy(dest, src, len)
        }
    },
    /* ---- storage: gas/decision in EVM, effect in kernel ---- */
    SLOAD() => {
        let s = pop();
        let warm = k_access_slot(self_addr(), s);
        charge(sload_cost(warm));
        push(k_sload(self_addr(), s))
    },
    SSTORE() => {
        if guard_static() then {
            ()
        } else {
            if gas_remaining <= G_callstipend then {
                exc_halt(OutOfGas)
                /* EIP-1706 */
            } else {
                let s = pop();
                let v = pop();
                let warm = k_access_slot(self_addr(), s); /* kernel: access accounting */
                let current = k_sload(self_addr(), s); /* kernel: read current      */
                let original = k_orig(self_addr(), s); /* kernel: tx-original        */
                let (cost, refund) = sstore_gas(original, current, v, not_bool(warm)); /* USER policy */
                charge(cost);
                if refund != 0 then {
                    k_refund_add(refund)
                };
                if current != v then {
                    k_sstore(self_addr(), s, v)
                } /* effect ONLY if it changes */
            }
        }
    },
    TLOAD() => {
        /* EIP-1153 */
        charge(G_warm_access);
        let s = pop();
        push(k_tload(self_addr(), s))
    },
    TSTORE() => {
        /* EIP-1153 */
        if guard_static() then {
            ()
        } else {
            charge(G_warm_access);
            let s = pop();
            let v = pop();
            k_tstore(self_addr(), s, v)
        }
    },
    /* ---- flow (pure over own code) ---- */
    JUMP() => {
        charge(G_mid);
        let dest = pop_int();
        do_jump(dest)
    },
    JUMPI() => {
        charge(G_high);
        let dest = pop_int();
        let cond = pop();
        if word_is_zero(cond) then {
            ()
        } else {
            do_jump(dest)
        }
    },
    PC() => {
        charge(G_base);
        push(word_of_int(pc - 1))
    },
    GAS() => {
        charge(G_base);
        push(word_of_int(gas_remaining))
    },
    JUMPDEST() => charge(G_jumpdest),
    /* ---- push / dup / swap (pure) ---- */
    PUSH(n, v) => {
        /* PUSH0 (n=0): EIP-3855 */
        if n == 0 then {
            charge(G_base)
        } else {
            charge(G_verylow)
        };
        push(v)
    },
    DUP(n) => {
        charge(G_verylow);
        push(peek(n - 1))
    },
    SWAP(n) => {
        charge(G_verylow);
        let top = peek(0);
        let other = peek(n);
        stack_set(0, other);
        stack_set(n, top)
    },
    /* ---- log: EVM checks/prices, kernel records ---- */
    LOG(n) => {
        if guard_static() then {
            ()
        } else {
            let off = pop_int();
            let len = pop_int();
            var topics : list(word) = [||];
            foreach (k from 0 to (n - 1)) {
                topics = list_append(topics, [|pop()|])
            };
            charge(log_gas(n, len));
            charge_expansion(off, len);
            if is_running() then {
                k_log(self_addr(), topics, mem_read_range(off, len))
            }
        }
    },
    /* ---- system ---- */
    CREATE() => run_create(false),
    CREATE2() => /* EIP-1014 */ run_create(true),
    CALL() => run_call(0),
    CALLCODE() => run_call(1),
    DELEGATECALL() => /* EIP-7 */ run_call(2),
    STATICCALL() => /* EIP-214 */ run_call(3),
    RETURN() => {
        let off = pop_int();
        let len = pop_int();
        charge_expansion(off, len);
        if is_running() then {
            returndata_capture(off, len);
            frame_status = Halted(HaltReturn)
        }
    },
    REVERT() => {
        /* EIP-140 */
        let off = pop_int();
        let len = pop_int();
        charge_expansion(off, len);
        if is_running() then {
            returndata_capture(off, len);
            frame_status = Halted(HaltRevert)
        }
    },
    INVALID() => {
        gas_remaining = 0;
        exc_halt(InvalidOpcode)
    },
    SELFDESTRUCT() => {
        if guard_static() then {
            ()
        } else {
            let beneficiary = word_to_address(pop());
            let bal = k_get_balance(self_addr());
            let warm = k_access_account(beneficiary);
            charge(G_selfdestruct);
            if not_bool(warm) then {
                charge(G_cold_account)
            }; /* EIP-2929 */
            if word_nonzero(bal) & k_account_is_empty(beneficiary) then {
                /* new account */
                charge(G_newaccount)
            };
            k_transfer(self_addr(), beneficiary, bal);

            /* EIP-6780: only a created-this-tx originator is registered for
             * deletion, and its balance is forced to 0 after the transfer
             * (burning a self-beneficiary transfer). A pre-existing account
             * just moves its balance and persists. */
            if k_was_created(self_addr()) then {
                k_zero_balance(self_addr());
                k_selfdestruct(self_addr())
            };
            frame_status = Halted(HaltSelfDestruct)
        }
    },
}
