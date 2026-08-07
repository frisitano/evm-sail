
/*md
# Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [memory_expansion_cost][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][].
*/
/*! The call-family runner (0 = `CALL`, 1 = `CALLCODE`,
    2 = `DELEGATECALL`, 3 = `STATICCALL`); defined with the
    interpreter. */
val run_call : CallKind -> unit
/*! The contract-creation runner; defined with the interpreter. */
val run_create : CreateKind -> unit

/*md
## Helpers
*/
/*! The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
    `SELFDESTRUCT` act on this account. */
function self_addr() -> address = message.address

/*! EIP-214 write protection: any state-modifying opcode in a
    `STATICCALL` frame halts exceptionally. State-changing opcodes call
    this first; `true` means already halted. */
function guard_static() -> bool =
    if message.is_static then {
        exc_halt(WriteProtection);
        true
    } else {
        false
    }

/*! `JUMP`/`JUMPI` target validity: the destination must be in code range
    and land on a `JUMPDEST` (the precomputed valid-destination set,
    YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
    bytes that happen to equal `0x5b` are not valid. */
function do_jump(destination_value : word) -> unit = {
    let code_length = frame_code_len();
    if destination_value < code_length then {
        let destination : code_pointer = destination_value;
        let valid_destination = frame_jumpdest_valid(destination);
        if valid_destination then {
            pc = destination
        } else {
            exc_halt(InvalidJump)
        }
    } else {
        exc_halt(InvalidJump)
    }
}

/*! Pops `count` log topics from the stack into its bounded representation. */
function pop_log_topics(count : log_topic_count) -> LogTopics =
    match count {
        0 => LogTopics0(),
        1 => {
            let t0 = pop();
            LogTopics1(t0)
        },
        2 => {
            let t0 = pop();
            let t1 = pop();
            LogTopics2((t0, t1))
        },
        3 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            LogTopics3((t0, t1, t2))
        },
        4 => {
            let t0 = pop();
            let t1 = pop();
            let t2 = pop();
            let t3 = pop();
            LogTopics4((t0, t1, t2, t3))
        },
        _ => LogTopics0(),
    }

/*md
## The opcode bodies

Each semantic opcode constructor has a named body. The standard decoder
validates the instruction's stack effect before dispatching its `ast` to these
functions below. Optimized C applies the same validation at its raw-byte
dispatch boundary before calling a generated body, without materializing an
`ast`.
*/
/*! Implements `ADD`. */
function execute_add() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_add(a, b);
    push(result)
}

/*! Implements `MUL`. */
function execute_mul() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_mul(a, b);
    push(result)
}

/*! Implements `SUB`. */
function execute_sub() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_sub(a, b);
    push(result)
}

/*! Implements unsigned `DIV`. */
function execute_div() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_div(a, b);
    push(result)
}

/*! Implements signed `SDIV`. */
function execute_sdiv() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_sdiv(a, b);
    push(result)
}

/*! Implements unsigned `MOD`. */
function execute_mod() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_mod(a, b);
    push(result)
}

/*! Implements signed `SMOD`. */
function execute_smod() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_smod(a, b);
    push(result)
}

/*! Implements `ADDMOD`. */
function execute_addmod() -> unit = {
    let gas_charged = charge(G_mid);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let n = pop();
    let result = alu_addmod(a, b, n);
    push(result)
}

/*! Implements `MULMOD`. */
function execute_mulmod() -> unit = {
    let gas_charged = charge(G_mid);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let n = pop();
    let result = alu_mulmod(a, b, n);
    push(result)
}

/*! Implements `EXP`, including exponent-dependent gas. */
function execute_exp() -> unit = {
    let a = pop();
    let e = pop();
    let gas_cost = exp_gas(e);
    let gas_charged = charge(gas_cost);
    if not_bool(gas_charged) then {
        return ()
    };
    let result = alu_exp(a, e);
    push(result)
}

/*! Implements `SIGNEXTEND`. */
function execute_signextend() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let bi = pop();
    let v = pop();
    let result = alu_signextend(bi, v);
    push(result)
}

/*! Implements unsigned `LT`. */
function execute_lt() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_lt(a, b);
    push(result)
}

/*! Implements unsigned `GT`. */
function execute_gt() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_gt(a, b);
    push(result)
}

/*! Implements signed `SLT`. */
function execute_slt() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_slt(a, b);
    push(result)
}

/*! Implements signed `SGT`. */
function execute_sgt() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_sgt(a, b);
    push(result)
}

/*! Implements `EQ`. */
function execute_eq() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_eq(a, b);
    push(result)
}

/*! Implements `ISZERO`. */
function execute_iszero() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let result = alu_iszero(a);
    push(result)
}

/*! Implements bitwise `AND`. */
function execute_and() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_and(a, b);
    push(result)
}

/*! Implements bitwise `OR`. */
function execute_or() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_or(a, b);
    push(result)
}

/*! Implements bitwise `XOR`. */
function execute_xor() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let b = pop();
    let result = alu_xor(a, b);
    push(result)
}

/*! Implements bitwise `NOT`. */
function execute_not() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let a = pop();
    let result = alu_not(a);
    push(result)
}

/*! Implements `BYTE`. */
function execute_byte() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let i = pop();
    let x = pop();
    let result = alu_byte(i, x);
    push(result)
}

/*! Implements logical left shift `SHL` (EIP-145). */
function execute_shl() -> unit = {
    /* EIP-145 */
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let s = pop();
    let v = pop();
    let result = alu_shl(s, v);
    push(result)
}

/*! Implements logical right shift `SHR` (EIP-145). */
function execute_shr() -> unit = {
    /* EIP-145 */
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let s = pop();
    let v = pop();
    let result = alu_shr(s, v);
    push(result)
}

/*! Implements arithmetic right shift `SAR` (EIP-145). */
function execute_sar() -> unit = {
    /* EIP-145 */
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let s = pop();
    let v = pop();
    let result = alu_sar(s, v);
    push(result)
}

/*! Implements count-leading-zeroes `CLZ` (EIP-7939). */
function execute_clz() -> unit = {
    /* EIP-7939 */
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let x = pop();
    let result = alu_clz(x);
    push(result)
}

/*! Implements `KECCAK256` over an expanded memory range. */
function execute_keccak256() -> unit = {
    let offset_word = pop();
    let length_word = pop();
    let hashing_gas_charged = charge_keccak_gas(length_word);
    if not_bool(hashing_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, length_word);
    expand_memory(access.required_size);
    let digest = mem_keccak(access.range);
    push(digest)
}

/*! Implements `ADDRESS`. */
function execute_address() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let address = self_addr();
    let address_word = address_to_word(address);
    push(address_word)
}

/*! Implements `ORIGIN`. */
function execute_origin() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let origin = k_env(F_Origin);
    push(origin)
}

/*! Implements `CALLER`. */
function execute_caller() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let caller = address_to_word(message.caller);
    push(caller)
}

/*! Implements `CALLVALUE`. */
function execute_callvalue() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    push(message.value)
}

/*! Implements `GASPRICE`. */
function execute_gasprice() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let gas_price = k_env(F_GasPrice);
    push(gas_price)
}

/*! Implements `CALLDATASIZE`. */
function execute_calldatasize() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let input = calldata;
    let input_length = region_slice_length(input);
    let length_word = word_of_byte_count(input_length);
    push(length_word)
}

/*! Implements `CALLDATALOAD`. */
function execute_calldataload() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let offset_word = pop();
    let value = slice_load_word_offset(calldata, offset_word);
    push(value)
}

/*! Implements `CALLDATACOPY`. */
function execute_calldatacopy() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let copy_gas_charged = charge_copy_gas(length_word);
    if not_bool(copy_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(destination_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    slice_copy_word_offset(calldata, range.off, source_word, range.len)
}

/*! Implements `CODESIZE`. */
function execute_codesize() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let code_length = frame_code_len();
    let length_word = word_of_byte_count(code_length);
    push(length_word)
}

/*! Implements `CODECOPY`. */
function execute_codecopy() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let copy_gas_charged = charge_copy_gas(length_word);
    if not_bool(copy_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(destination_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    let code = frame_code;
    let bytes = code_bytes(code);
    slice_copy_word_offset(bytes, range.off, source_word, range.len)
}

/*! Implements `BALANCE`, including warm/cold account access. */
function execute_balance() -> unit = {
    let address_word = pop();
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let gas_charged = charge(gas_cost);
    if not_bool(gas_charged) then {
        return ()
    };
    k_account_mark_warm(a);
    let balance = k_get_balance(a);
    push(balance)
}

/*! Implements `SELFBALANCE`. */
function execute_selfbalance() -> unit = {
    let gas_charged = charge(G_low);
    if not_bool(gas_charged) then {
        return ()
    };
    let address = self_addr();
    let balance = k_get_balance(address);
    push(balance)
}

/*! Implements `EXTCODESIZE`, including warm/cold account access. */
function execute_extcodesize() -> unit = {
    let address_word = pop();
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let gas_charged = charge(access_cost + read_cost);
    if not_bool(gas_charged) then {
        return ()
    };
    k_account_mark_warm(a);
    let code_size = k_get_code_size(a);
    let size_word = word_of_byte_count(code_size);
    push(size_word)
}

/*! Implements `EXTCODECOPY`, including access and copy gas. */
function execute_extcodecopy() -> unit = {
    let address_word = pop();
    let a = word_to_address(address_word);
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let warm = k_account_is_warm(a);
    let access_cost = account_cost(warm);
    let read_cost = external_code_read_cost();
    let access_gas_charged = charge(access_cost + read_cost);
    if not_bool(access_gas_charged) then {
        return ()
    };
    let copy_gas_charged = charge_copy_gas(length_word);
    if not_bool(copy_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(destination_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    k_account_mark_warm(a);
    k_code_copy(a, range.off, source_word, range.len)
}

/*! Implements `EXTCODEHASH`, including warm/cold account access. */
function execute_extcodehash() -> unit = {
    let address_word = pop();
    let a = word_to_address(address_word);
    let warm = k_account_is_warm(a);
    let gas_cost = account_cost(warm);
    let gas_charged = charge(gas_cost);
    if not_bool(gas_charged) then {
        return ()
    };
    k_account_mark_warm(a);
    let code_hash = k_get_codehash(a);
    let hash_word = hash_to_word(code_hash);
    push(hash_word)
}

/*! Implements `RETURNDATASIZE`. */
function execute_returndatasize() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let return_data_size = returndata_size();
    let size_word = word_of_byte_count(return_data_size);
    push(size_word)
}

/*! Implements bounds-checked `RETURNDATACOPY`. */
function execute_returndatacopy() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let copy_gas_charged = charge_copy_gas(length_word);
    if not_bool(copy_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(destination_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(destination_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    returndata_copy_words(range.off, source_word, length_word)
}

/*! Implements `BLOCKHASH`. */
function execute_blockhash() -> unit = {
    let gas_charged = charge(20);
    if not_bool(gas_charged) then {
        return ()
    };
    let block_number = pop();
    let block_hash = k_blockhash(block_number);
    let hash_word = hash_to_word(block_hash);
    push(hash_word)
}

/*! Implements `COINBASE`. */
function execute_coinbase() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let coinbase = k_env(F_Coinbase);
    push(coinbase)
}

/*! Implements `TIMESTAMP`. */
function execute_timestamp() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let timestamp = k_env(F_Timestamp);
    push(timestamp)
}

/*! Implements `NUMBER`. */
function execute_number() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let block_number = k_env(F_Number);
    push(block_number)
}

/*! Implements `SLOTNUM`. */
function execute_slotnum() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let slot_number = k_env(F_SlotNumber);
    push(slot_number)
}

/*! Implements `PREVRANDAO`. */
function execute_prevrandao() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let prev_randao = k_env(F_PrevRandao);
    push(prev_randao)
}

/*! Implements `GASLIMIT`. */
function execute_gaslimit() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let gas_limit = k_env(F_GasLimit);
    push(gas_limit)
}

/*! Implements `CHAINID`. */
function execute_chainid() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let chain_id = k_env(F_ChainId);
    push(chain_id)
}

/*! Implements `BASEFEE`. */
function execute_basefee() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let base_fee = k_env(F_BaseFee);
    push(base_fee)
}

/*! Implements `BLOBBASEFEE`. */
function execute_blobbasefee() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let execution_profile = k_execution_profile;
    let base_fee = blob_base_fee(execution_profile.protocol, k_header.excess_blob_gas);
    push(base_fee)
}

/*! Implements `BLOBHASH`. */
function execute_blobhash() -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let index = pop();
    let blob_hash = k_blobhash(index);
    push(blob_hash)
}

/*! Implements `POP`. */
function execute_pop() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let _ = pop()
}

/*! Implements `MLOAD`. */
function execute_mload() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let offset_word = pop();
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, word_size);
    expand_memory(access.required_size);
    let value = mem_load(access.range.off);
    push(value)
}

/*! Implements `MSTORE`. */
function execute_mstore() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let offset_word = pop();
    let v = pop();
    let word_size = u256(32);
    let required_size = memory_required_size(offset_word, word_size);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, word_size);
    expand_memory(access.required_size);
    mem_store(access.range.off, v)
}

/*! Implements `MSTORE8`. */
function execute_mstore8() -> unit = {
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let offset_word = pop();
    let v = pop();
    let required_size = memory_required_size(offset_word, WORD_ONE);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, WORD_ONE);
    expand_memory(access.required_size);
    mem_store_byte(access.range.off, v)
}

/*! Implements `MSIZE`. */
function execute_msize() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let high_water = evm_memory_high_water();
    let words = memory_word_count(high_water);
    let size = word_of_nat_byte_count(words * 32);
    push(size)
}

/*! Implements overlapping memory copy `MCOPY` (EIP-5656). */
function execute_mcopy() -> unit = {
    /* EIP-5656 */
    let base_gas_charged = charge(G_verylow);
    if not_bool(base_gas_charged) then {
        return ()
    };
    let destination_word = pop();
    let source_word = pop();
    let length_word = pop();
    let copy_gas_charged = charge_copy_gas(length_word);
    if not_bool(copy_gas_charged) then {
        return ()
    };
    let destination_required = memory_required_size(destination_word, length_word);
    let source_required = memory_required_size(source_word, length_word);
    let required_size =
        if destination_required < source_required then source_required else destination_required;
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let destination = memory_access(destination_word, length_word);
    let source = memory_access(source_word, length_word);
    let materialized_required_size =
        if destination.required_size < source.required_size then source.required_size else destination.required_size;
    expand_memory(materialized_required_size);
    mem_mcopy(destination.range.off, source.range.off, destination.range.len)
}

/*! Implements `SLOAD`, including warm/cold access gas. */
function execute_sload() -> unit = {
    /* EIP-2929: cold (2100) vs warm (100) by the slot's accessed-set bit */
    let s = pop();
    let address = self_addr();
    let warm = k_slot_is_warm(address, s);
    let gas_cost = sload_cost(warm);
    let gas_charged = charge(gas_cost);
    if not_bool(gas_charged) then {
        return ()
    };
    k_slot_mark_warm(address, s);
    let entry = k_sload(address, s);
    push(entry.curr)
}

/*! Implements `SSTORE`, including fork-specific gas and refunds. */
function execute_sstore() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let write_protected = guard_static();
    if write_protected then {
        return ()
    };
    if (profile.fork < Amsterdam) & (gas_remaining <= G_callstipend) then {
        exc_halt(OutOfGas);
        return ()
    };
    let s = pop();
    let v = pop();
    let address = self_addr();
    let warm = k_slot_is_warm(address, s);
    let cold = not_bool(warm);
    if profile.fork >= Amsterdam then {
        let sentry_cost = sstore_sentry_cost(cold);
        let sentry_affordable = check_execution_gas(sentry_cost);
        if not_bool(sentry_affordable) then {
            return ()
        }
    };
    k_slot_mark_warm(address, s);
    let entry = k_sload(address, s);
    let costs = sstore_costs(entry.orig, entry.curr, v, cold);
    if costs.state_credit != 0 then {
        credit_state_gas_refund(costs.state_credit)
    };
    let execution_gas_charged = charge(costs.execution);
    if not_bool(execution_gas_charged) then {
        return ()
    };
    let state_gas_charged = charge_state_gas(costs.state_charge);
    if not_bool(state_gas_charged) then {
        return ()
    };
    if not_bool(costs.refund == GAS_REFUND_ZERO) then {
        record_refund(costs.refund)
    };
    if entry.curr != v then {
        k_sstore(address, s, struct { curr = v, orig = entry.orig })
    }
}

/*! Implements transient-storage load `TLOAD` (EIP-1153). */
function execute_tload() -> unit = {
    /* EIP-1153 */
    let gas_charged = charge(G_warm_access);
    if not_bool(gas_charged) then {
        return ()
    };
    let s = pop();
    let address = self_addr();
    let value = k_tload(address, s);
    push(value)
}

/*! Implements transient-storage write `TSTORE` (EIP-1153). */
function execute_tstore() -> unit = {
    /* EIP-1153 */
    let write_protected = guard_static();
    if write_protected then {
        return ()
    };
    let gas_charged = charge(G_warm_access);
    if not_bool(gas_charged) then {
        return ()
    };
    let s = pop();
    let v = pop();
    let address = self_addr();
    k_tstore(address, s, v)
}

/*! Implements unconditional `JUMP`. */
function execute_jump() -> unit = {
    let gas_charged = charge(G_mid);
    if not_bool(gas_charged) then {
        return ()
    };
    let dest = pop();
    do_jump(dest)
}

/*! Implements conditional `JUMPI`. */
function execute_jumpi() -> unit = {
    let gas_charged = charge(G_high);
    if not_bool(gas_charged) then {
        return ()
    };
    let dest = pop();
    let cond = pop();
    let condition_is_zero = word_is_zero(cond);
    if condition_is_zero then {
        return ()
    };
    do_jump(dest)
}

/*! Implements `PC`, returning the current opcode position. */
function execute_pc() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    let next_pc = word_of_byte_count(pc);
    let opcode_pc = alu_sub(next_pc, WORD_ONE);
    push(opcode_pc)
}

/*! Implements `GAS`, returning gas remaining after its own charge. */
function execute_gas() -> unit = {
    let gas_charged = charge(G_base);
    if not_bool(gas_charged) then {
        return ()
    };
    push_gas(gas_remaining)
}

/*! Implements `JUMPDEST`. */
function execute_jumpdest() -> unit = {
    let gas_charged = charge(G_jumpdest);
    if not_bool(gas_charged) then {
        return ()
    }
}

/*! Implements the `PUSH0` through `PUSH32` family. */
function execute_push(n : push_width, v : word) -> unit = {
    if n == 0 then {
        let gas_charged = charge(G_base);
        if not_bool(gas_charged) then {
            return ()
        }
    } else {
        let gas_charged = charge(G_verylow);
        if not_bool(gas_charged) then {
            return ()
        }
    };
    push(v)
}

/*! Implements the `DUP1` through `DUP16` family. */
function execute_dup(n : stack_operation_index) -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let value = peek(n - 1);
    push(value)
}

/*! Implements the `SWAP1` through `SWAP16` family. */
function execute_swap(n : stack_operation_index) -> unit = {
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let top = peek(0);
    let other = peek(n);
    stack_set(0, other);
    stack_set(n, top)
}

/*! Implements immediate deep-stack duplication `DUPN`. */
function execute_dupn(immediate : byte) -> unit = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        exc_halt(InvalidOpcode)
    } else {
        let gas_charged = charge(G_verylow);
        if not_bool(gas_charged) then {
            return ()
        };
        let n = decode_single_stack_index(immediate);
        let value = peek(n - 1);
        push(value)
    }
}

/*! Implements immediate deep-stack exchange `SWAPN`. */
function execute_swapn(immediate : byte) -> unit = {
    let valid_immediate = deep_stack_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        exc_halt(InvalidOpcode)
    } else {
        let gas_charged = charge(G_verylow);
        if not_bool(gas_charged) then {
            return ()
        };
        let n = decode_single_stack_index(immediate);
        let top = peek(0);
        let other = peek(n);
        stack_set(0, other);
        stack_set(n, top)
    }
}

/*! Implements immediate pairwise deep-stack `EXCHANGE`. */
function execute_exchange(immediate : byte) -> unit = {
    let valid_immediate = exchange_immediate_valid(immediate);
    if not_bool(valid_immediate) then {
        exc_halt(InvalidOpcode);
        return ()
    };
    let gas_charged = charge(G_verylow);
    if not_bool(gas_charged) then {
        return ()
    };
    let (n, m) = decode_exchange_stack_indices(immediate);
    let first = peek(n);
    let second = peek(m);
    stack_set(n, second);
    stack_set(m, first)
}

/*! Implements the `LOG0` through `LOG4` family. */
function execute_log(n : log_topic_count) -> unit = {
    let write_protected = guard_static();
    if write_protected then {
        return ()
    };
    let offset_word = pop();
    let length_word = pop();
    let topics = pop_log_topics(n);
    let log_gas_charged = charge_log_gas(n, length_word);
    if not_bool(log_gas_charged) then {
        return ()
    };
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    let data = active_memory_slice(range.off, range.len);
    let address = self_addr();
    let memory_slice = evm_memory_slice(data.bytes, data.len);
    let log_data = LogDataMemory(memory_slice);
    k_log(address, topics, log_data)
}

/*! Implements normal `STOP`. */
function execute_stop() -> unit = {
    let reason = HaltStop();
    frame_status = Halted(reason)
}

/*! Implements successful `RETURN`. */
function execute_return() -> unit = {
    let offset_word = pop();
    let length_word = pop();
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    let data = active_memory_slice(range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltReturn(output);
    frame_status = Halted(reason)
}

/*! Implements rollback-carrying `REVERT` (EIP-140). */
function execute_revert() -> unit = {
    /* EIP-140 */
    let offset_word = pop();
    let length_word = pop();
    let required_size = memory_required_size(offset_word, length_word);
    let expansion_cost = memory_expansion_cost(required_size);
    let expansion_gas_charged = charge(expansion_cost);
    if not_bool(expansion_gas_charged) then {
        return ()
    };
    let access = memory_access(offset_word, length_word);
    expand_memory(access.required_size);
    let range = access.range;
    refill_frame_state_gas();
    let data = active_memory_slice(range.off, range.len);
    let output = freeze_output(data);
    let reason = HaltRevert(output);
    frame_status = Halted(reason)
}

/*! Implements exceptional invalid-opcode termination. */
function execute_invalid() -> unit = {
    gas_remaining = GAS_ZERO;
    exc_halt(InvalidOpcode)
}

/*! Implements `SELFDESTRUCT` with fork-specific semantics. */
function execute_selfdestruct() -> unit = {
    let execution_profile = k_execution_profile;
    let profile = execution_profile.protocol;
    let write_protected = guard_static();
    if write_protected then {
        return ()
    };
    let beneficiary_word = pop();
    let beneficiary = word_to_address(beneficiary_word);
    let address = self_addr();
    if profile.fork >= Amsterdam then {
        let warm = k_account_is_warm(beneficiary);
        let access_cost = 0 + G_selfdestruct + (if warm then G_zero else G_amsterdam_cold_account_access);
        let execution_affordable = check_execution_gas(access_cost);
        if not_bool(execution_affordable) then {
            return ()
        };
        k_account_mark_warm(beneficiary);
        let bal = k_get_balance(address);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        let creates_account = nonzero_balance & beneficiary_empty;
        let execution_cost =
            if creates_account then access_cost + G_amsterdam_account_write else access_cost;
        let execution_gas_charged = charge(execution_cost);
        if not_bool(execution_gas_charged) then {
            return ()
        };
        if creates_account then {
            let state_gas_charged = charge_state_gas(G_amsterdam_state_new_account);
            if not_bool(state_gas_charged) then {
                return ()
            }
        };
        k_transfer(address, beneficiary, bal);
        let created = k_was_created(address);
        if created then {
            k_selfdestruct(address)
        };
        let reason = HaltSelfDestruct();
        frame_status = Halted(reason)
    } else {
        let bal = k_get_balance(address);
        let warm = k_account_is_warm(beneficiary);
        let selfdestruct_gas_charged = charge(G_selfdestruct);
        if not_bool(selfdestruct_gas_charged) then {
            return ()
        };
        if not_bool(warm) then {
            let cold_account_gas_charged = charge(G_cold_account);
            if not_bool(cold_account_gas_charged) then {
                return ()
            }
        };
        k_account_mark_warm(beneficiary);
        let nonzero_balance = word_nonzero(bal);
        let beneficiary_empty = k_account_is_empty(beneficiary);
        if nonzero_balance & beneficiary_empty then {
            let new_account_gas_charged = charge(G_newaccount);
            if not_bool(new_account_gas_charged) then {
                return ()
            }
        };
        let is_selfdestructed = k_is_selfdestructed(address);
        let first_selfdestruct = not_bool(is_selfdestructed);
        if (profile.fork < London) & first_selfdestruct then {
            record_refund(R_selfdestruct_pre_london)
        };
        k_transfer(address, beneficiary, bal);
        if profile.fork < Cancun then {
            k_zero_balance(address);
            k_selfdestruct(address)
        } else {
            let created = k_was_created(address);
            if created then {
                k_zero_balance(address);
                k_selfdestruct(address)
            }
        };
        let reason = HaltSelfDestruct();
        frame_status = Halted(reason)
    }
}

/*! Implements `CREATE` through the common create runner. */
function execute_create() -> unit = run_create(CreateByNonce)
/*! Implements `CREATE2` through the common create runner. */
function execute_create2() -> unit = run_create(CreateBySalt)
/*! Implements `CALL` through the common call runner. */
function execute_call() -> unit = run_call(Call)
/*! Implements `CALLCODE` through the common call runner. */
function execute_callcode() -> unit = run_call(CallCode)
/*! Implements `DELEGATECALL` through the common call runner. */
function execute_delegatecall() -> unit = run_call(DelegateCall)
/*! Implements `STATICCALL` through the common call runner. */
function execute_staticcall() -> unit = run_call(StaticCall)

/*! The Yellow Paper stack effect `(delta, alpha)` for every decoded
    instruction. Invalid deep-stack immediates have no stack effect because
    their opcode body raises `InvalidOpcode` before accessing the stack. */
function opcode_stack_effect(op : ast) -> (operand_stack_height, operand_stack_height) = match op {
    STOP() => (0, 0),
    ADD() => (2, 1),
    MUL() => (2, 1),
    SUB() => (2, 1),
    DIV() => (2, 1),
    SDIV() => (2, 1),
    MOD() => (2, 1),
    SMOD() => (2, 1),
    ADDMOD() => (3, 1),
    MULMOD() => (3, 1),
    EXP() => (2, 1),
    SIGNEXTEND() => (2, 1),
    LT() => (2, 1),
    GT() => (2, 1),
    SLT() => (2, 1),
    SGT() => (2, 1),
    EQ() => (2, 1),
    ISZERO() => (1, 1),
    AND() => (2, 1),
    OR() => (2, 1),
    XOR() => (2, 1),
    NOT() => (1, 1),
    BYTE() => (2, 1),
    SHL() => (2, 1),
    SHR() => (2, 1),
    SAR() => (2, 1),
    CLZ() => (1, 1),
    KECCAK256() => (2, 1),
    ADDRESS() => (0, 1),
    BALANCE() => (1, 1),
    ORIGIN() => (0, 1),
    CALLER() => (0, 1),
    CALLVALUE() => (0, 1),
    CALLDATALOAD() => (1, 1),
    CALLDATASIZE() => (0, 1),
    CALLDATACOPY() => (3, 0),
    CODESIZE() => (0, 1),
    CODECOPY() => (3, 0),
    GASPRICE() => (0, 1),
    EXTCODESIZE() => (1, 1),
    EXTCODECOPY() => (4, 0),
    RETURNDATASIZE() => (0, 1),
    RETURNDATACOPY() => (3, 0),
    EXTCODEHASH() => (1, 1),
    BLOCKHASH() => (1, 1),
    COINBASE() => (0, 1),
    TIMESTAMP() => (0, 1),
    NUMBER() => (0, 1),
    PREVRANDAO() => (0, 1),
    GASLIMIT() => (0, 1),
    CHAINID() => (0, 1),
    SELFBALANCE() => (0, 1),
    BASEFEE() => (0, 1),
    BLOBHASH() => (1, 1),
    BLOBBASEFEE() => (0, 1),
    SLOTNUM() => (0, 1),
    POP() => (1, 0),
    MLOAD() => (1, 1),
    MSTORE() => (2, 0),
    MSTORE8() => (2, 0),
    SLOAD() => (1, 1),
    SSTORE() => (2, 0),
    JUMP() => (1, 0),
    JUMPI() => (2, 0),
    PC() => (0, 1),
    MSIZE() => (0, 1),
    GAS() => (0, 1),
    JUMPDEST() => (0, 0),
    TLOAD() => (1, 1),
    TSTORE() => (2, 0),
    MCOPY() => (3, 0),
    PUSH(_, _) => (0, 1),
    DUP(n) => (n, n + 1),
    SWAP(n) => (n + 1, n + 1),
    DUPN(immediate) => {
        let valid_immediate = deep_stack_immediate_valid(immediate);
        if valid_immediate then {
            let n = decode_single_stack_index(immediate);
            (n, n + 1)
        } else {
            (0, 0)
        }
    },
    SWAPN(immediate) => {
        let valid_immediate = deep_stack_immediate_valid(immediate);
        if valid_immediate then {
            let n = decode_single_stack_index(immediate);
            (n + 1, n + 1)
        } else {
            (0, 0)
        }
    },
    EXCHANGE(immediate) => {
        let valid_immediate = exchange_immediate_valid(immediate);
        if valid_immediate then {
            let (_, higher) = decode_exchange_stack_indices(immediate);
            (higher + 1, higher + 1)
        } else {
            (0, 0)
        }
    },
    LOG(n) => (n + 2, 0),
    opcode_CREATE() => (3, 1),
    CALL() => (7, 1),
    CALLCODE() => (7, 1),
    RETURN() => (2, 0),
    DELEGATECALL() => (6, 1),
    CREATE2() => (4, 1),
    STATICCALL() => (6, 1),
    REVERT() => (2, 0),
    INVALID() => (0, 0),
    SELFDESTRUCT() => (1, 0),
}

/*! Standard decoded-opcode dispatcher. Each arm invokes the same narrow
    semantic function used directly by the optimized raw-byte interpreter. */
function execute_opcode(op : ast) -> unit = match op {
    STOP() => execute_stop(),
    /* ---- arithmetic (pure) ---- */
    ADD() => execute_add(),
    MUL() => execute_mul(),
    SUB() => execute_sub(),
    DIV() => execute_div(),
    SDIV() => execute_sdiv(),
    MOD() => execute_mod(),
    SMOD() => execute_smod(),
    ADDMOD() => execute_addmod(),
    MULMOD() => execute_mulmod(),
    EXP() => execute_exp(),
    SIGNEXTEND() => execute_signextend(),
    /* ---- comparison / bitwise (pure) ---- */
    LT() => execute_lt(),
    GT() => execute_gt(),
    SLT() => execute_slt(),
    SGT() => execute_sgt(),
    EQ() => execute_eq(),
    ISZERO() => execute_iszero(),
    AND() => execute_and(),
    OR() => execute_or(),
    XOR() => execute_xor(),
    NOT() => execute_not(),
    BYTE() => execute_byte(),
    SHL() => execute_shl(),
    SHR() => execute_shr(),
    SAR() => execute_sar(),
    CLZ() => execute_clz(),
    /* ---- keccak: hash via kernel primitive ---- */
    KECCAK256() => execute_keccak256(),
    /* ---- environment / account (kernel reads, EVM prices them) ---- */
    ADDRESS() => execute_address(),
    ORIGIN() => execute_origin(),
    CALLER() => execute_caller(),
    CALLVALUE() => execute_callvalue(),
    GASPRICE() => execute_gasprice(),
    CALLDATASIZE() => execute_calldatasize(),
    CALLDATALOAD() => execute_calldataload(),
    CALLDATACOPY() => execute_calldatacopy(),
    CODESIZE() => execute_codesize(),
    CODECOPY() => execute_codecopy(),
    BALANCE() => execute_balance(),
    SELFBALANCE() => execute_selfbalance(),
    EXTCODESIZE() => execute_extcodesize(),
    EXTCODECOPY() => execute_extcodecopy(),
    EXTCODEHASH() => execute_extcodehash(),
    RETURNDATASIZE() => execute_returndatasize(),
    RETURNDATACOPY() => execute_returndatacopy(),
    /* ---- block (kernel reads) ---- */
    BLOCKHASH() => execute_blockhash(),
    COINBASE() => execute_coinbase(),
    TIMESTAMP() => execute_timestamp(),
    NUMBER() => execute_number(),
    SLOTNUM() => execute_slotnum(),
    PREVRANDAO() => execute_prevrandao(),
    GASLIMIT() => execute_gaslimit(),
    CHAINID() => execute_chainid(),
    BASEFEE() => execute_basefee(),
    BLOBBASEFEE() => execute_blobbasefee(),
    BLOBHASH() => execute_blobhash(),
    /* ---- stack / memory (pure, user-space) ---- */
    POP() => execute_pop(),
    MLOAD() => execute_mload(),
    MSTORE() => execute_mstore(),
    MSTORE8() => execute_mstore8(),
    MSIZE() => execute_msize(),
    MCOPY() => execute_mcopy(),
    /* ---- storage: gas/decision in EVM, effect in kernel ---- */
    SLOAD() => execute_sload(),
    SSTORE() => execute_sstore(),
    TLOAD() => execute_tload(),
    TSTORE() => execute_tstore(),
    /* ---- flow (pure over own code) ---- */
    JUMP() => execute_jump(),
    JUMPI() => execute_jumpi(),
    PC() => execute_pc(),
    GAS() => execute_gas(),
    JUMPDEST() => execute_jumpdest(),
    /* ---- push / dup / swap (pure) ---- */
    PUSH(n, v) => execute_push(n, v),
    DUP(n) => execute_dup(n),
    SWAP(n) => execute_swap(n),
    DUPN(immediate) => execute_dupn(immediate),
    SWAPN(immediate) => execute_swapn(immediate),
    EXCHANGE(immediate) => execute_exchange(immediate),
    /* ---- log: EVM checks/prices, kernel records ---- */
    LOG(n) => execute_log(n),
    /* ---- system ---- */
    opcode_CREATE() => execute_create(),
    CREATE2() => execute_create2(),
    CALL() => execute_call(),
    CALLCODE() => execute_callcode(),
    DELEGATECALL() => execute_delegatecall(),
    STATICCALL() => execute_staticcall(),
    RETURN() => execute_return(),
    REVERT() => execute_revert(),
    INVALID() => execute_invalid(),
    SELFDESTRUCT() => execute_selfdestruct(),
}

/*! The single-step transition function over the standard decoded AST. Stack
    validity is established before gas charging or any instruction side
    effect, as part of the Yellow Paper exceptional-halt predicate. */
function execute(op : ast) -> unit = {
    let (inputs, outputs) = opcode_stack_effect(op);
    let stack_valid = validate_stack(inputs, outputs);
    if stack_valid then {
        execute_opcode(op)
    }
}
