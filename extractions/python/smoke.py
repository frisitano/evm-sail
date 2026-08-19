"""Executable numeric-boundary smoke test for the full EVM extraction."""

from __future__ import annotations

import ast
import builtins
import inspect
from pathlib import Path
import symtable
import typing

from ethereum_types.bytes import Bytes20, Bytes32

import adapter
import evm
import evm.HostContract as host_contract
from evm.kernel import environment
from evm.host.state import BalAccount, BalAccountEnd, BalEmpty, BalStorageChange
from evm.primitives.account import StorageKey, StorageTxCleared, StorageTxMiss


def expect_raises(error: type[BaseException], action) -> None:
    try:
        action()
    except error:
        return
    raise AssertionError(f"expected {error.__name__}")


def generated_code_checks() -> None:
    generated_root = Path(evm.__file__).parent
    objects = {**evm.__sail_functions__, **evm.__sail_types__}
    modules = {evm}
    if (generated_root / "_model.py").exists():
        raise AssertionError("generated package still contains _model.py")
    failures = []
    direct_extern_targets = set()
    for value in objects.values():
        owner = inspect.getmodule(value)
        if owner is not None and (owner.__name__ == "evm" or owner.__name__.startswith("evm.")):
            modules.add(owner)

    builtin_names = set(dir(builtins))
    for source_path in sorted(generated_root.rglob("*.py")):
        source = source_path.read_text()
        source_tree = ast.parse(source)
        host_imports = {
            imported.asname or imported.name: imported.name
            for node in ast.walk(source_tree)
            if isinstance(node, ast.ImportFrom) and node.module == "evm.HostContract"
            for imported in node.names
        }
        direct_extern_targets.update(host_imports.values())
        for node in ast.walk(source_tree):
            if (
                isinstance(node, ast.Call)
                and isinstance(node.func, ast.Name)
                and node.func.id.startswith("_host_")
                and node.func.id not in host_imports
            ):
                failures.append(
                    f"{source_path.relative_to(generated_root)}: "
                    f"unlinked host call {node.func.id}"
                )
        if "_sail_extern." in source:
            failures.append(
                f"{source_path.relative_to(generated_root)}: legacy extern module gateway"
            )
        if "_sail_module_" in source:
            failures.append(f"{source_path.relative_to(generated_root)}: legacy module alias")
        if any(
            (isinstance(node, ast.Name) and node.id == "_model")
            or (isinstance(node, ast.alias) and node.name == "_model")
            for node in ast.walk(source_tree)
        ):
            failures.append(f"{source_path.relative_to(generated_root)}: central model reference")
        if any(
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Name)
            and node.func.id.startswith("coerce_")
            for node in ast.walk(source_tree)
        ):
            failures.append(f"{source_path.relative_to(generated_root)}: generated coercion call")

    for target in sorted(direct_extern_targets):
        if not callable(getattr(host_contract, target, None)):
            failures.append("HostContract: missing direct extern " + target)

    types_tree = ast.parse((generated_root / "_types.py").read_text())
    if any(isinstance(node, (ast.ClassDef, ast.FunctionDef, ast.Assign, ast.AnnAssign)) for node in types_tree.body):
        failures.append("evm._types: expected imports-only compatibility aggregator")
    type_imports = {
        (node.module, imported.name)
        for node in types_tree.body
        if isinstance(node, ast.ImportFrom)
        for imported in node.names
    }
    for expected_import in {
        ("primitives.rlp", "RlpFieldRef"),
    }:
        if expected_import not in type_imports:
            failures.append(f"evm._types: missing source-local re-export {expected_import}")

    address_tree = ast.parse((generated_root / "lib" / "address.py").read_text())
    address_imports = {
        (node.module, imported.name)
        for node in ast.walk(address_tree)
        if isinstance(node, ast.ImportFrom)
        for imported in node.names
    }
    for expected_import in {
        ("evm.primitives.bytes", "WORD_BYTE_LENGTH"),
        ("evm.kernel.scratch", "scratch_push_address"),
    }:
        if expected_import not in address_imports:
            failures.append(f"evm.lib.address: missing exact import {expected_import}")

    for module in sorted(modules, key=lambda item: item.__name__):
        try:
            typing.get_type_hints(module, include_extras=True)
        except Exception as error:
            failures.append(f"module {module.__name__}: {type(error).__name__}: {error}")

        source_path = Path(getattr(module, "__file__", None) or "")
        if not source_path.is_file():
            continue
        table = symtable.symtable(source_path.read_text(), str(source_path), "exec")
        module_names = set(module.__dict__) | {
            symbol.get_name()
            for symbol in table.get_symbols()
            if symbol.is_assigned() or symbol.is_imported() or symbol.is_namespace()
        }

        def visit(scope: symtable.SymbolTable) -> None:
            if scope is not table:
                for symbol in scope.get_symbols():
                    name = symbol.get_name()
                    if symbol.is_referenced() and symbol.is_global() and name not in module_names | builtin_names:
                        failures.append(f"{module.__name__}:{scope.get_name()}: undefined global {name}")
            for child in scope.get_children():
                visit(child)

        visit(table)

    for sail_name, value in sorted(objects.items()):
        if not (inspect.isfunction(value) or inspect.isclass(value)):
            continue
        try:
            typing.get_type_hints(value, include_extras=True)
        except Exception as error:
            failures.append(f"{sail_name}: {type(error).__name__}: {error}")

    for function_name in (
        "word_add_word",
        "word_sub_word",
        "word_shift_left",
    ):
        function = getattr(evm, function_name)
        if not any(
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Name)
            and node.func.id == "int"
            for node in ast.walk(ast.parse(inspect.getsource(function)))
        ):
            failures.append(f"{function_name}: missing integer arithmetic boundary")

    assert not failures, "generated Python audit failed:\n" + "\n".join(failures)


def main() -> None:
    generated_code_checks()

    assert evm.U256.__module__ == "ethereum_types.numeric"
    assert int(evm.U256(7)) == 7
    assert evm.word_add_word.__module__ == "evm.prelude"
    assert evm.process_transaction.__module__ == "evm.evm.transaction"
    assert evm.StatelessInputSliceFields.__module__ == "evm.primitives.bytes"
    assert evm.RlpFieldRef.__module__ == "evm.primitives.rlp"
    assert evm.Bytes20 is Bytes20
    assert evm.Bytes32 is Bytes32
    assert evm.word is evm.U256

    runtime_source = (Path(evm.__file__).parent / "_runtime.py").read_text()
    assert "from ethereum_types.numeric import (" in runtime_source
    assert "from ethereum_types.bytes import (" in runtime_source
    for source_path in Path(evm.__file__).parent.rglob("*.py"):
        if source_path.name != "_runtime.py":
            assert "from ethereum_types" not in source_path.read_text()

    zero_word = evm.ZERO_WORD
    previous_host_state = host_contract.get_state()
    environment.k_chain_id = 7
    assert evm.k_chain_id == 7
    evm.reset()
    assert host_contract.get_state() is not previous_host_state
    assert environment.k_chain_id == 1
    assert evm.k_chain_id == 1
    assert evm.ZERO_WORD is zero_word
    assert type(zero_word) is evm.U256

    outer_host_state = host_contract.get_state()
    isolated_host_state = host_contract.HostState()
    with host_contract.use_state(isolated_host_state):
        assert host_contract.get_state() is isolated_host_state

        host_contract.mem_store_word(0, 0x1234)
        assert host_contract.mem_load_word(0) == 0x1234
        host_contract.mem_frame_enter()
        host_contract.mem_store_word(0, 0xABCD)
        assert host_contract.mem_load_word(0) == 0xABCD
        host_contract.mem_frame_leave()
        assert host_contract.mem_load_word(0) == 0x1234

        parent_top = host_contract.stack_reset_host()
        host_contract.stack_slot_write_next_host(parent_top, 7)
        parent_top = host_contract.stack_top_advance_host(parent_top, 1)
        child_top = host_contract.operand_stack_push_empty_frame_host()
        assert host_contract.stack_top_height_host(child_top) == 0
        host_contract.stack_slot_write_next_host(child_top, 11)
        child_top = host_contract.stack_top_advance_host(child_top, 1)
        assert host_contract.stack_top_height_host(child_top) == 1
        assert host_contract.stack_slot_read_host(child_top, 0) == 11
        child_top = host_contract.stack_top_retreat_host(child_top, 1)
        host_contract.operand_stack_pop_frame()
        assert host_contract.stack_top_height_host(parent_top) == 1
        assert host_contract.stack_slot_read_host(parent_top, 0) == 7
        host_contract.stack_top_retreat_host(parent_top, 1)

        isolated_host_state.stateless_input_bytes = b"\x05\x06\x07"
        input_slice = host_contract.stateless_input()
        assert input_slice.len == 3
        assert int(host_contract.stateless_input_byte_at(input_slice, 2)) == 7
        stored = host_contract.host_scratch_store_stateless_input(0, input_slice)
        assert host_contract.stateless_input_keccak256(
            input_slice
        ) == host_contract.scratch_keccak256(stored)

        state_address = Bytes20(b"\x11" * 20)
        assert host_contract.account_is_warm(state_address) is False
        host_contract.account_mark_warm(state_address)
        assert host_contract.account_is_warm(state_address) is True
        host_contract.transient_store(state_address, 3, 5)
        host_contract.state_journal_checkpoint()
        host_contract.transient_store(state_address, 3, 9)
        assert host_contract.transient_load(state_address, 3) == 9
        cold_address = Bytes20(b"\x22" * 20)
        host_contract.account_mark_warm(cold_address)
        host_contract.storage_mark_warm(cold_address, 4)
        assert host_contract.account_is_warm(cold_address) is True
        assert host_contract.storage_is_warm(cold_address, 4) is True
        slot_key = StorageKey(addr=cold_address, slot=4)
        assert isinstance(host_contract.storage_tx_get(slot_key), StorageTxMiss)
        host_contract.storage_tx_clear(cold_address)
        assert isinstance(host_contract.storage_tx_get(slot_key), StorageTxCleared)
        host_contract.state_journal_revert()
        assert host_contract.transient_load(state_address, 3) == 5
        assert host_contract.account_is_warm(state_address) is True
        assert host_contract.account_is_warm(cold_address) is False
        assert host_contract.storage_is_warm(cold_address, 4) is False
        assert isinstance(host_contract.storage_tx_get(slot_key), StorageTxMiss)

        host_contract.bal_reset()
        host_contract.bal_storage_change(1, state_address, 6, 7)
        host_contract.bal_storage_change(2, state_address, 6, 8)
        host_contract.bal_prepare_iter()
        assert isinstance(host_contract.bal_iter_next(), BalAccount)
        first = host_contract.bal_iter_next()
        assert isinstance(first, BalStorageChange)
        assert (first.value.index, first.value.value) == (1, 7)
        second = host_contract.bal_iter_next()
        assert isinstance(second, BalStorageChange)
        assert (second.value.index, second.value.value) == (2, 8)
        assert isinstance(host_contract.bal_iter_next(), BalAccountEnd)
        assert isinstance(host_contract.bal_iter_next(), BalEmpty)

        host_contract.logs_tx_reset()
        host_contract.log_begin(state_address)
        host_contract.log_add_topic(17)
        host_contract.log_add_data_word(19)
        assert host_contract.logs_tx_count() == 1
        assert host_contract.log_address(0) == state_address
        assert host_contract.log_topics_count(0) == 1
        assert host_contract.log_topic(0, 0) == 17
        assert host_contract.log_data_length(0) == 32

    assert host_contract.get_state() is outer_host_state

    word_max = adapter.word(-1)
    assert type(word_max) is evm.U256
    assert adapter.word_to_int(word_max) == (1 << adapter.WORD_WIDTH) - 1

    wrapped = evm.word_add_word(word_max, adapter.word(1))
    assert type(wrapped) is evm.U256
    assert adapter.word_to_int(wrapped) == 0

    shifted = evm.word_shift_left(adapter.word(1 << 255), 1)
    assert type(shifted) is evm.U256
    assert adapter.word_to_int(shifted) == 0

    shifted_right = evm.word_shift_right(adapter.word(1 << 255), 255)
    assert type(shifted_right) is evm.U256
    assert int(shifted_right) == 1

    digest = adapter.b256(0x1234)
    assert isinstance(digest, Bytes32)
    assert len(digest) == adapter.B256_BYTE_LENGTH
    assert all(isinstance(byte, int) and 0 <= byte <= 0xFF for byte in digest)
    assert adapter.b256_to_int(digest) == 0x1234
    assert type(evm.hash_to_word(digest)) is evm.U256
    assert evm.hash is evm.b256

    account = adapter.address(0x1234)
    assert isinstance(account, Bytes20)
    assert len(account) == adapter.ADDRESS_BYTE_LENGTH
    assert evm.address_to_word(account) == adapter.word(0x1234)

    cursor_validity = evm.RlpIndexCursorValidity(maximum=16)
    populated_cursor = evm.rlp_index_cursor(8, 16)
    initial_item = populated_cursor.current
    cursor = evm.RlpIndexCursor(
        validity=cursor_validity,
        count=evm.Uint(8),
        position=evm.Uint(0),
        current=initial_item,
    )
    assert cursor.validity.maximum == 16
    assert populated_cursor.validity == cursor.validity
    assert populated_cursor.count == cursor.count
    assert populated_cursor.position == cursor.position
    assert populated_cursor.current == cursor.current
    expect_raises(ValueError, lambda: evm.RlpIndexCursorValidity(maximum=0))
    expect_raises(ValueError, lambda: evm.RlpIndexCursorValidity(maximum="16"))
    expect_raises(
        ValueError,
        lambda: evm.RlpIndexCursor(
            validity=cursor_validity,
            count=evm.Uint(17),
            position=evm.Uint(0),
            current=initial_item,
        ),
    )

    def invalidate_cursor() -> None:
        cursor.count = evm.Uint(17)

    expect_raises(ValueError, invalidate_cursor)
    cursor = evm.RlpIndexCursor(
        validity=cursor_validity,
        count=evm.Uint(8),
        position=evm.Uint(0),
        current=initial_item,
    )

    def invalidate_cursor_validity() -> None:
        cursor.validity = evm.RlpIndexCursorValidity(maximum=4)

    expect_raises(ValueError, invalidate_cursor_validity)

    print("Python EVM extraction smoke: ok")


if __name__ == "__main__":
    main()
