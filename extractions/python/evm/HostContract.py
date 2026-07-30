"""Executable Python implementation of the evm-sail host boundary.

The Sail model calls these functions directly.  Unlike the generic backend
fallback, this module has no string-keyed extern registry: every boundary
operation is a normal Python function over an explicit :class:`HostState`.

Generated evm-sail types are imported inside functions to avoid cycles while
the generated package is being initialized.
"""

from __future__ import annotations

from contextlib import contextmanager
from contextvars import ContextVar
from copy import deepcopy
from dataclasses import dataclass, field
from hashlib import sha256
from typing import Any, Iterator


WORD_BYTES = 32
WORD_MASK = (1 << 256) - 1
DELEGATION_PREFIX = b"\xef\x01\x00"


class AcceleratorContract:
    """Typed extension point for genuine cryptographic accelerators."""

    def ripemd160(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("RIPEMD-160 accelerator is not configured")

    def modexp(
        self,
        state: HostState,
        data: Any,
        base_length: int,
        exponent_length: int,
        modulus_length: int,
    ) -> bool:
        raise NotImplementedError("MODEXP accelerator is not configured")

    def bn254_add(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BN254 add accelerator is not configured")

    def bn254_mul(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BN254 mul accelerator is not configured")

    def bn254_pairing(self, state: HostState, data: Any) -> int:
        raise NotImplementedError("BN254 pairing accelerator is not configured")

    def blake2f(self, state: HostState, data: Any, rounds: int, final: Any) -> bool:
        raise NotImplementedError("BLAKE2F accelerator is not configured")

    def kzg_point_evaluation(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("KZG accelerator is not configured")

    def bls_g1_add(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS G1 add accelerator is not configured")

    def bls_g1_msm(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS G1 MSM accelerator is not configured")

    def bls_g2_add(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS G2 add accelerator is not configured")

    def bls_g2_msm(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS G2 MSM accelerator is not configured")

    def bls_pairing(self, state: HostState, data: Any) -> int:
        raise NotImplementedError("BLS pairing accelerator is not configured")

    def bls_map_fp_to_g1(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS map-to-G1 accelerator is not configured")

    def bls_map_fp2_to_g2(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("BLS map-to-G2 accelerator is not configured")

    def p256_verify(self, state: HostState, data: Any) -> bool:
        raise NotImplementedError("P-256 accelerator is not configured")

    def ecrecover(
        self, state: HostState, digest: Any, y_parity: Any, r: int, s: int
    ) -> Any:
        from evm.prelude import AddressResult

        recovered = _secp256k1_recover_address(
            _fixed_wire_bytes(digest), int(y_parity), int(r), int(s)
        )
        if recovered is None:
            return AddressResult(success=False, address=_bytes20_from_wire(bytes(20)))
        return AddressResult(success=True, address=_bytes20_from_wire(recovered))


@dataclass(slots=True)
class MemoryFrame:
    base: int
    length: int = 0


@dataclass(slots=True)
class JumpdestTable:
    code_length: int
    chunks: dict[int, int] = field(default_factory=dict)


@dataclass(slots=True)
class LogRecord:
    address: Any
    topics: list[int] = field(default_factory=list)
    data_offset: int = 0
    data_length: int = 0


@dataclass(slots=True)
class AuthorizationRecord:
    seen: bool = False
    originally_delegated: bool = False
    delegation_set: bool = False


@dataclass(slots=True)
class StorageBlockRow:
    key: Any
    value: Any
    address_hash: Any | None = None
    slot_hash: Any | None = None


@dataclass(slots=True)
class AccountBlockRow:
    address: Any
    value: Any
    address_hash: Any | None = None


@dataclass(slots=True)
class AccountTxRow:
    current: Any
    original: Any


@dataclass(slots=True)
class JournalSnapshot:
    account_tx: dict[bytes, Any]
    storage_tx: dict[tuple[bytes, int], Any]
    storage_cleared: set[bytes]
    transient: dict[tuple[bytes, int], int]
    warm_addresses: set[bytes]
    warm_slots: set[tuple[bytes, int]]
    logs: list[LogRecord]
    log_data: bytearray
    current_log: int | None


@dataclass(slots=True)
class HostState:
    """All mutable state owned by the Python host implementation."""

    stateless_input_bytes: bytes = b""
    memory_bytes: bytearray = field(default_factory=bytearray)
    memory_frames: list[MemoryFrame] = field(
        default_factory=lambda: [MemoryFrame(base=0)]
    )
    code_bytes: bytearray = field(default_factory=bytearray)
    scratch_bytes: bytearray = field(default_factory=bytearray)
    output_bytes: bytearray = field(default_factory=bytearray)
    public_output: bytearray = field(default_factory=bytearray)
    log_data: bytearray = field(default_factory=bytearray)
    operand_stack: list[int] = field(default_factory=list)
    operand_stack_frames: list[int] = field(default_factory=list)
    continuation_frames: list[Any] = field(default_factory=list)
    ancestor_hashes: dict[int, Any] = field(default_factory=dict)
    jumpdest_tables: dict[int, JumpdestTable] = field(default_factory=dict)
    next_jumpdest_table: int = 1
    code_db: dict[bytes, Any] = field(default_factory=dict)
    node_db: dict[bytes, Any] = field(default_factory=dict)
    transient: dict[tuple[bytes, int], int] = field(default_factory=dict)
    storage_tx: dict[tuple[bytes, int], Any] = field(default_factory=dict)
    storage_cleared: set[bytes] = field(default_factory=set)
    storage_block: dict[tuple[bytes, int], StorageBlockRow] = field(
        default_factory=dict
    )
    storage_iterator: list[StorageBlockRow] = field(default_factory=list)
    account_tx: dict[bytes, Any] = field(default_factory=dict)
    account_block: dict[bytes, AccountBlockRow] = field(default_factory=dict)
    account_iterator: list[AccountBlockRow] = field(default_factory=list)
    post_storage_roots: dict[bytes, Any] = field(default_factory=dict)
    checkpoints: list[JournalSnapshot] = field(default_factory=list)
    warm_addresses: set[bytes] = field(default_factory=set)
    warm_slots: set[tuple[bytes, int]] = field(default_factory=set)
    logs: list[LogRecord] = field(default_factory=list)
    transaction_log_start: int = 0
    current_log: int | None = None
    authorizations: dict[bytes, AuthorizationRecord] = field(default_factory=dict)
    bal_accounts: set[bytes] = field(default_factory=set)
    bal_addresses: dict[bytes, Any] = field(default_factory=dict)
    bal_storage_changes: dict[tuple[bytes, int], tuple[int, int]] = field(
        default_factory=dict
    )
    bal_storage_reads: set[tuple[bytes, int]] = field(default_factory=set)
    bal_balance_changes: dict[bytes, tuple[int, int]] = field(default_factory=dict)
    bal_nonce_changes: dict[bytes, tuple[int, int]] = field(default_factory=dict)
    bal_code_changes: dict[bytes, tuple[int, Any]] = field(default_factory=dict)
    bal_iterator: list[Any] = field(default_factory=list)
    accelerators: AcceleratorContract = field(default_factory=AcceleratorContract)


_CURRENT_STATE: ContextVar[HostState | None] = ContextVar(
    "evm_sail_python_host_state", default=None
)


def get_state() -> HostState:
    state = _CURRENT_STATE.get()
    if state is None:
        state = HostState()
        _CURRENT_STATE.set(state)
    return state


def set_state(state: HostState) -> None:
    _CURRENT_STATE.set(state)


@contextmanager
def use_state(state: HostState) -> Iterator[HostState]:
    token = _CURRENT_STATE.set(state)
    try:
        yield state
    finally:
        _CURRENT_STATE.reset(token)


def reset(state: HostState | None = None) -> None:
    _CURRENT_STATE.set(HostState() if state is None else state)


def finish() -> None:
    pass


def _address_key(value: Any) -> bytes:
    return _fixed_wire_bytes(value)


def _hash_key(value: Any) -> bytes:
    return _fixed_wire_bytes(value)


def _storage_key(value: Any) -> tuple[bytes, int]:
    return (_address_key(value.addr), int(value.slot))


def _address_slot_key(address: Any, slot: int) -> tuple[bytes, int]:
    return (_address_key(address), int(slot))


def _fixed_wire_bytes(value: Any) -> bytes:
    return bytes(int(item) for item in reversed(value))


def _decreasing_vector_wire_bytes(value: Any) -> bytes:
    return bytes(int(item) for item in value)


def _word_bytes(value: int, length: int = WORD_BYTES) -> bytes:
    return (int(value) & WORD_MASK).to_bytes(WORD_BYTES, "big")[-length:]


def _byte(value: int) -> Any:
    from evm._runtime import Bits

    return Bits(8, value)


def _bytes32_from_wire(value: bytes) -> Any:
    from evm._runtime import Bytes32

    return Bytes32(value[::-1])


def _bytes20_from_wire(value: bytes) -> Any:
    from evm._runtime import Bytes20

    return Bytes20(value[::-1])


def _zero_hash() -> Any:
    return _bytes32_from_wire(bytes(32))


def _ensure_length(region: bytearray, required: int) -> None:
    if required > len(region):
        region.extend(bytes(required - len(region)))


def _slice_bytes(region: bytes | bytearray, value: Any) -> bytes:
    start = int(value.off)
    end = start + int(value.len)
    return bytes(region[start:end])


def _current_memory_frame(state: HostState) -> MemoryFrame:
    return state.memory_frames[-1]


def _memory_absolute_offset(state: HostState, offset: int) -> int:
    return _current_memory_frame(state).base + int(offset)


def _memory_slice_bytes(value: Any) -> bytes:
    return _slice_bytes(get_state().memory_bytes, value)


def _region_bytes(kind: str, value: Any) -> bytes:
    state = get_state()
    regions: dict[str, bytes | bytearray] = {
        "input": state.stateless_input_bytes,
        "memory": state.memory_bytes,
        "code": state.code_bytes,
        "scratch": state.scratch_bytes,
        "log": state.log_data,
        "output": state.output_bytes,
    }
    return _slice_bytes(regions[kind], value)


def _region_byte(kind: str, value: Any, offset: int) -> Any:
    data = _region_bytes(kind, value)
    index = int(offset)
    return _byte(data[index] if index < len(data) else 0)


def _load_word(kind: str, value: Any, offset: int, length: int = 32) -> int:
    data = _region_bytes(kind, value)
    start = int(offset)
    width = int(length)
    payload = data[start : start + width]
    return int.from_bytes(payload.ljust(width, b"\x00"), "big")


def _copy_to_memory(
    kind: str, value: Any, destination: int, source: int, length: int
) -> None:
    state = get_state()
    count = int(length)
    payload = _region_bytes(kind, value)[int(source) : int(source) + count]
    payload = payload.ljust(count, b"\x00")
    absolute = _memory_absolute_offset(state, destination)
    _ensure_length(state.memory_bytes, absolute + count)
    state.memory_bytes[absolute : absolute + count] = payload


def _strided_zero(
    kind: str, value: Any, start: int, stride: int, width: int, count: int
) -> bool:
    data = _region_bytes(kind, value)
    for item in range(int(count)):
        offset = int(start) + item * int(stride)
        if any(data[offset : offset + int(width)]):
            return False
    return True


def _append_code(payload: bytes) -> Any:
    from evm.primitives.bytes import code_region_slice

    state = get_state()
    offset = len(state.code_bytes)
    state.code_bytes.extend(payload)
    return code_region_slice(offset, len(payload))


def _scratch_store(offset: int, payload: bytes) -> Any:
    from evm.primitives.bytes import ScratchRegionReady, scratch_slice

    state = get_state()
    start = int(offset)
    end = start + len(payload)
    _ensure_length(state.scratch_bytes, end)
    state.scratch_bytes[start:end] = payload
    return ScratchRegionReady(scratch_slice(0, end))


def _keccak(data: bytes) -> bytes:
    try:
        from ethereum.crypto.hash import keccak256 as eels_keccak256

        return bytes(eels_keccak256(data))
    except ImportError:
        try:
            from Crypto.Hash import keccak
        except ImportError as error:
            raise RuntimeError(
                "Keccak-256 requires ethereum.crypto.hash or pycryptodome"
            ) from error
        digest = keccak.new(digest_bits=256)
        digest.update(data)
        return digest.digest()


_SECP256K1_FIELD = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
_SECP256K1_ORDER = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
_SECP256K1_GENERATOR = (
    0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798,
    0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8,
)
_SecpPoint = tuple[int, int] | None


def _secp256k1_add(left: _SecpPoint, right: _SecpPoint) -> _SecpPoint:
    if left is None:
        return right
    if right is None:
        return left
    left_x, left_y = left
    right_x, right_y = right
    if left_x == right_x:
        if (left_y + right_y) % _SECP256K1_FIELD == 0:
            return None
        slope = (
            (3 * left_x * left_x)
            * pow(2 * left_y, -1, _SECP256K1_FIELD)
        ) % _SECP256K1_FIELD
    else:
        slope = (
            (right_y - left_y)
            * pow(right_x - left_x, -1, _SECP256K1_FIELD)
        ) % _SECP256K1_FIELD
    result_x = (slope * slope - left_x - right_x) % _SECP256K1_FIELD
    result_y = (slope * (left_x - result_x) - left_y) % _SECP256K1_FIELD
    return (result_x, result_y)


def _secp256k1_mul(scalar: int, point: _SecpPoint) -> _SecpPoint:
    result = None
    addend = point
    while scalar:
        if scalar & 1:
            result = _secp256k1_add(result, addend)
        addend = _secp256k1_add(addend, addend)
        scalar >>= 1
    return result


def _secp256k1_recover_address(
    digest: bytes, y_parity: int, r: int, s: int
) -> bytes | None:
    if (
        len(digest) != 32
        or y_parity not in (0, 1)
        or not 1 <= r < _SECP256K1_ORDER
        or not 1 <= s < _SECP256K1_ORDER
        or r >= _SECP256K1_FIELD
    ):
        return None

    alpha = (pow(r, 3, _SECP256K1_FIELD) + 7) % _SECP256K1_FIELD
    y = pow(alpha, (_SECP256K1_FIELD + 1) // 4, _SECP256K1_FIELD)
    if pow(y, 2, _SECP256K1_FIELD) != alpha:
        return None
    if y & 1 != y_parity:
        y = _SECP256K1_FIELD - y
    recovered_r = (r, y)
    if _secp256k1_mul(_SECP256K1_ORDER, recovered_r) is not None:
        return None

    message = int.from_bytes(digest, "big") % _SECP256K1_ORDER
    inverse_r = pow(r, -1, _SECP256K1_ORDER)
    public_key = _secp256k1_mul(
        inverse_r,
        _secp256k1_add(
            _secp256k1_mul(s, recovered_r),
            _secp256k1_mul(
                (-message) % _SECP256K1_ORDER, _SECP256K1_GENERATOR
            ),
        ),
    )
    if public_key is None:
        return None
    public_x, public_y = public_key
    encoded = public_x.to_bytes(32, "big") + public_y.to_bytes(32, "big")
    return _keccak(encoded)[12:]


def _keccak_region(kind: str, value: Any) -> Any:
    return _bytes32_from_wire(_keccak(_region_bytes(kind, value)))


def _sha256_region(kind: str, value: Any) -> Any:
    return _bytes32_from_wire(sha256(_region_bytes(kind, value)).digest())


def stateless_input() -> Any:
    from evm.primitives.bytes import stateless_input_slice

    return stateless_input_slice(0, len(get_state().stateless_input_bytes))


def stateless_input_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("input", value, offset)


def memory_slice_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("memory", value, offset)


def code_region_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("code", value, offset)


def scratch_slice_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("scratch", value, offset)


def log_data_slice_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("log", value, offset)


def output_slice_byte_at(value: Any, offset: int) -> Any:
    return _region_byte("output", value, offset)


def stateless_input_count_nonzero(value: Any) -> int:
    return sum(byte != 0 for byte in _region_bytes("input", value))


def stateless_input_strided_zero(
    value: Any, start: int, stride: int, width: int, count: int
) -> bool:
    return _strided_zero("input", value, start, stride, width, count)


def memory_slice_strided_zero(
    value: Any, start: int, stride: int, width: int, count: int
) -> bool:
    return _strided_zero("memory", value, start, stride, width, count)


def stateless_input_load_word(value: Any, offset: int) -> int:
    return _load_word("input", value, offset)


def memory_slice_load_word(value: Any, offset: int) -> int:
    return _load_word("memory", value, offset)


def code_region_load_word(value: Any, offset: int) -> int:
    return _load_word("code", value, offset)


def scratch_slice_load_word(value: Any, offset: int) -> int:
    return _load_word("scratch", value, offset)


def log_data_slice_load_word(value: Any, offset: int) -> int:
    return _load_word("log", value, offset)


def output_slice_load_word(value: Any, offset: int) -> int:
    return _load_word("output", value, offset)


def stateless_input_load_n_word(value: Any, offset: int, length: int) -> int:
    return _load_word("input", value, offset, length)


def code_region_load_n_word(value: Any, offset: int, length: int) -> int:
    return _load_word("code", value, offset, length)


def scratch_slice_load_n_word(value: Any, offset: int, length: int) -> int:
    return _load_word("scratch", value, offset, length)


def stateless_input_copy_to_memory(
    value: Any, destination: int, source: int, length: int
) -> None:
    _copy_to_memory("input", value, destination, source, length)


def memory_slice_copy_to_memory(
    value: Any, destination: int, source: int, length: int
) -> None:
    _copy_to_memory("memory", value, destination, source, length)


def code_region_copy_to_memory(
    value: Any, destination: int, source: int, length: int
) -> None:
    _copy_to_memory("code", value, destination, source, length)


def output_slice_copy_to_memory(
    value: Any, destination: int, source: int, length: int
) -> None:
    _copy_to_memory("output", value, destination, source, length)


def scratch_input_slices_equal(left: Any, right: Any) -> bool:
    return _region_bytes("scratch", left) == _region_bytes("input", right)


def log_input_slices_equal(left: Any, right: Any) -> bool:
    return _region_bytes("log", left) == _region_bytes("input", right)


def input_code_slices_equal(left: Any, right: Any) -> bool:
    return _region_bytes("input", left) == _region_bytes("code", right)


def mem_read_byte(offset: int) -> Any:
    state = get_state()
    absolute = _memory_absolute_offset(state, offset)
    value = state.memory_bytes[absolute] if absolute < len(state.memory_bytes) else 0
    return _byte(value)


def mem_write_byte(offset: int, value: Any) -> None:
    state = get_state()
    absolute = _memory_absolute_offset(state, offset)
    _ensure_length(state.memory_bytes, absolute + 1)
    state.memory_bytes[absolute] = int(value)
    _current_memory_frame(state).length = max(
        _current_memory_frame(state).length, int(offset) + 1
    )


def mem_clear() -> None:
    state = get_state()
    state.memory_bytes.clear()
    state.memory_frames[:] = [MemoryFrame(base=0)]


def mem_frame_enter() -> int:
    state = get_state()
    base = len(state.memory_bytes)
    state.memory_frames.append(MemoryFrame(base=base))
    return base


def mem_frame_leave() -> None:
    state = get_state()
    if len(state.memory_frames) <= 1:
        raise RuntimeError("cannot leave the root memory frame")
    frame = state.memory_frames.pop()
    del state.memory_bytes[frame.base:]


def mem_expand(required: int) -> Any:
    from evm.primitives.bytes import memory_slice

    state = get_state()
    frame = _current_memory_frame(state)
    length = int(required)
    _ensure_length(state.memory_bytes, frame.base + length)
    frame.length = max(frame.length, length)
    return memory_slice(frame.base, length)


def mem_move(destination: int, source: int, length: int) -> None:
    state = get_state()
    count = int(length)
    source_absolute = _memory_absolute_offset(state, source)
    destination_absolute = _memory_absolute_offset(state, destination)
    payload = bytes(
        state.memory_bytes[source_absolute : source_absolute + count]
    ).ljust(count, b"\x00")
    _ensure_length(state.memory_bytes, destination_absolute + count)
    state.memory_bytes[destination_absolute : destination_absolute + count] = payload


def mem_load_word(offset: int) -> int:
    state = get_state()
    absolute = _memory_absolute_offset(state, offset)
    payload = bytes(state.memory_bytes[absolute : absolute + WORD_BYTES])
    return int.from_bytes(payload.ljust(WORD_BYTES, b"\x00"), "big")


def mem_store_word(offset: int, value: int) -> None:
    state = get_state()
    absolute = _memory_absolute_offset(state, offset)
    _ensure_length(state.memory_bytes, absolute + WORD_BYTES)
    state.memory_bytes[absolute : absolute + WORD_BYTES] = _word_bytes(value)
    _current_memory_frame(state).length = max(
        _current_memory_frame(state).length, int(offset) + WORD_BYTES
    )


def stack_reset() -> None:
    state = get_state()
    state.operand_stack.clear()
    state.operand_stack_frames.clear()


def stack_enter_frame() -> None:
    get_state().operand_stack_frames.append(len(get_state().operand_stack))


def stack_leave_frame() -> None:
    state = get_state()
    if not state.operand_stack_frames:
        raise RuntimeError("cannot leave the root operand-stack frame")
    base = state.operand_stack_frames.pop()
    del state.operand_stack[base:]


def _stack_base(state: HostState) -> int:
    return state.operand_stack_frames[-1] if state.operand_stack_frames else 0


def stack_depth() -> int:
    state = get_state()
    return len(state.operand_stack) - _stack_base(state)


def stack_push_word(value: int) -> None:
    get_state().operand_stack.append(int(value))


def stack_pop_word() -> int:
    state = get_state()
    if len(state.operand_stack) <= _stack_base(state):
        raise RuntimeError("operand stack underflow")
    return state.operand_stack.pop()


def stack_peek_word(index: int) -> int:
    state = get_state()
    position = len(state.operand_stack) - 1 - int(index)
    if position < _stack_base(state):
        raise RuntimeError("operand stack underflow")
    return state.operand_stack[position]


def stack_set_word(index: int, value: int) -> None:
    state = get_state()
    position = len(state.operand_stack) - 1 - int(index)
    if position < _stack_base(state):
        raise RuntimeError("operand stack underflow")
    state.operand_stack[position] = int(value)


def frame_stack_reset() -> None:
    get_state().continuation_frames.clear()


def frame_stack_push(value: Any) -> None:
    get_state().continuation_frames.append(value)


def frame_stack_pop() -> Any:
    from evm.primitives.evm import Empty

    state = get_state()
    return state.continuation_frames.pop() if state.continuation_frames else Empty()


def host_scratch_reserve(offset: int, length: int) -> bool:
    state = get_state()
    _ensure_length(state.scratch_bytes, int(offset) + int(length))
    return True


def host_scratch_store_byte(offset: int, value: Any) -> Any:
    return _scratch_store(offset, bytes([int(value)]))


def host_scratch_store_stateless_input(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _region_bytes("input", value))


def host_scratch_store_scratch(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _region_bytes("scratch", value))


def host_scratch_store_log_data(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _region_bytes("log", value))


def host_scratch_store_output(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _region_bytes("output", value))


def host_scratch_store_address(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _fixed_wire_bytes(value))


def host_scratch_store_b256(offset: int, value: Any, length: int) -> Any:
    return _scratch_store(offset, _fixed_wire_bytes(value)[: int(length)])


def host_scratch_store_fixed_bytes_256(offset: int, value: Any) -> Any:
    return _scratch_store(offset, _decreasing_vector_wire_bytes(value))


def host_scratch_store_word(offset: int, value: int, length: int) -> Any:
    return _scratch_store(offset, _word_bytes(value, int(length)))


def host_scratch_truncate(offset: int) -> None:
    del get_state().scratch_bytes[int(offset):]


def output_buffer_store_memory(value: Any) -> bool:
    get_state().output_bytes[:] = _region_bytes("memory", value)
    return True


def output_buffer_store_input(value: Any) -> bool:
    get_state().output_bytes[:] = _region_bytes("input", value)
    return True


def output_buffer_store_word(value: int) -> bool:
    get_state().output_bytes[:] = _word_bytes(value)
    return True


def output_buffer_store_words(first: int, second: int) -> bool:
    get_state().output_bytes[:] = _word_bytes(first) + _word_bytes(second)
    return True


def public_output_write(value: Any) -> bool:
    get_state().public_output.extend(_region_bytes("scratch", value))
    return True


def stateless_input_keccak256(value: Any) -> Any:
    return _keccak_region("input", value)


def scratch_keccak256(value: Any) -> Any:
    return _keccak_region("scratch", value)


def memory_keccak256(value: Any) -> Any:
    return _keccak_region("memory", value)


def stateless_input_sha256(value: Any) -> Any:
    return _sha256_region("input", value)


def scratch_sha256(value: Any) -> Any:
    return _sha256_region("scratch", value)


def memory_sha256(value: Any) -> Any:
    return _sha256_region("memory", value)


def keccak256_word(value: int) -> Any:
    return _bytes32_from_wire(_keccak(_word_bytes(value)))


def keccak256_address(value: Any) -> Any:
    return _bytes32_from_wire(_keccak(_fixed_wire_bytes(value)))


def sha256_pair(left: Any, right: Any) -> Any:
    payload = _fixed_wire_bytes(left) + _fixed_wire_bytes(right)
    return _bytes32_from_wire(sha256(payload).digest())


def code_region_from_input(value: Any) -> Any:
    return _append_code(_region_bytes("input", value))


def code_region_from_memory(value: Any) -> Any:
    return _append_code(_region_bytes("memory", value))


def code_region_from_output(value: Any) -> Any:
    return _append_code(_region_bytes("output", value))


def jumpdest_table_alloc(code_length: int) -> int:
    state = get_state()
    index = state.next_jumpdest_table
    state.next_jumpdest_table += 1
    state.jumpdest_tables[index] = JumpdestTable(code_length=int(code_length))
    return index


def jumpdest_table_store_chunk(
    table: int, code_length: int, chunk_index: int, chunk: Any
) -> bool:
    state = get_state()
    entry = state.jumpdest_tables.get(int(table))
    if entry is None or entry.code_length != int(code_length):
        return False
    entry.chunks[int(chunk_index)] = int(chunk)
    return True


def jumpdest_ref_contains(table: int, code_length: int, destination: int) -> bool:
    entry = get_state().jumpdest_tables.get(int(table))
    index = int(destination)
    if entry is None or entry.code_length != int(code_length):
        return False
    if index < 0 or index >= entry.code_length:
        return False
    chunk = entry.chunks.get(index // 256, 0)
    return bool((chunk >> (index % 256)) & 1)


def code_db_store(value: Any, jumpdests: int) -> Any:
    from evm.primitives.code import Code

    digest = _keccak(_region_bytes("code", value))
    key = bytes(digest)
    result = _bytes32_from_wire(digest)
    get_state().code_db[key] = Code(bytes=value, jumpdests=jumpdests)
    return result


def code_db_lookup(value: Any) -> Any | None:
    return get_state().code_db.get(_hash_key(value))


def code_intern_delegation(address: Any, jumpdests: int) -> Any:
    value = _append_code(DELEGATION_PREFIX + _fixed_wire_bytes(address))
    return code_db_store(value, jumpdests)


def code_db_read_delegation(value: Any) -> Any:
    from evm.prelude import AddressResult

    code = code_db_lookup(value)
    if code is None:
        return AddressResult(success=False, address=_bytes20_from_wire(bytes(20)))
    payload = _region_bytes("code", code.bytes)
    valid = len(payload) == 23 and payload.startswith(DELEGATION_PREFIX)
    address = payload[3:] if valid else bytes(20)
    return AddressResult(success=valid, address=_bytes20_from_wire(address))


def nodedb_reset() -> None:
    get_state().node_db.clear()


def nodedb_insert(value: Any, offset: int, length: int) -> None:
    from evm.primitives.bytes import stateless_input_slice

    get_state().node_db[_hash_key(value)] = stateless_input_slice(
        int(offset), int(length)
    )


def nodedb_lookup(value: Any) -> Any:
    from evm.primitives.bytes import EMPTY_STATELESS_INPUT_SLICE

    return get_state().node_db.get(_hash_key(value), EMPTY_STATELESS_INPUT_SLICE)


def ancestor_hash_write(index: int, value: Any) -> None:
    get_state().ancestor_hashes[int(index)] = value


def ancestor_hash_read(index: int) -> Any:
    return get_state().ancestor_hashes.get(int(index), _zero_hash())


def transient_reset() -> None:
    get_state().transient.clear()


def transient_store(address: Any, slot: int, value: int) -> None:
    get_state().transient[_address_slot_key(address, slot)] = int(value)


def transient_load(address: Any, slot: int) -> int:
    return get_state().transient.get(_address_slot_key(address, slot), 0)


def state_checkpoint_reset() -> None:
    get_state().checkpoints.clear()


def state_checkpoint() -> int:
    state = get_state()
    checkpoint = len(state.checkpoints)
    state.checkpoints.append(
        JournalSnapshot(
            account_tx=deepcopy(state.account_tx),
            storage_tx=deepcopy(state.storage_tx),
            storage_cleared=set(state.storage_cleared),
            transient=dict(state.transient),
            warm_addresses=set(state.warm_addresses),
            warm_slots=set(state.warm_slots),
            logs=deepcopy(state.logs),
            log_data=bytearray(state.log_data),
            current_log=state.current_log,
        )
    )
    return checkpoint


def state_revert(checkpoint: int) -> None:
    state = get_state()
    index = int(checkpoint)
    if not 0 <= index < len(state.checkpoints):
        return
    snapshot = state.checkpoints[index]
    state.account_tx = deepcopy(snapshot.account_tx)
    state.storage_tx = deepcopy(snapshot.storage_tx)
    state.storage_cleared = set(snapshot.storage_cleared)
    state.transient = dict(snapshot.transient)
    state.warm_addresses = set(snapshot.warm_addresses)
    state.warm_slots = set(snapshot.warm_slots)
    state.logs = deepcopy(snapshot.logs)
    state.log_data = bytearray(snapshot.log_data)
    state.current_log = snapshot.current_log
    del state.checkpoints[index:]


def storage_tx_update(entry: Any) -> None:
    get_state().storage_tx[_storage_key(entry.key)] = entry


def storage_tx_get(key: Any) -> Any:
    from evm.primitives.account import (
        StorageTxCleared,
        StorageTxHit,
        StorageTxMiss,
    )

    state = get_state()
    canonical = _storage_key(key)
    entry = state.storage_tx.get(canonical)
    if entry is not None:
        return StorageTxHit(entry.value)
    if canonical[0] in state.storage_cleared:
        return StorageTxCleared()
    return StorageTxMiss()


def storage_tx_pop() -> Any | None:
    state = get_state()
    if not state.storage_tx:
        return None
    _, entry = state.storage_tx.popitem()
    return entry


def storage_tx_clear(address: Any) -> None:
    state = get_state()
    key = _address_key(address)
    state.storage_tx = {
        storage_key: entry
        for storage_key, entry in state.storage_tx.items()
        if storage_key[0] != key
    }
    state.storage_cleared.add(key)


def storage_tx_reset() -> None:
    state = get_state()
    state.storage_tx.clear()
    state.storage_cleared.clear()


def storage_has_writes(address: Any) -> bool:
    state = get_state()
    key = _address_key(address)
    tx_writes = any(
        storage_key[0] == key and int(entry.value.curr) != 0
        for storage_key, entry in state.storage_tx.items()
    )
    block_writes = any(
        storage_key[0] == key and int(row.value.curr) != 0
        for storage_key, row in state.storage_block.items()
    )
    return tx_writes or block_writes


def storage_block_get(key: Any) -> Any | None:
    row = get_state().storage_block.get(_storage_key(key))
    return None if row is None else row.value


def _storage_row_hashes(key: Any) -> tuple[Any, Any]:
    state = get_state()
    account = state.account_block.get(_address_key(key.addr))
    address_hash = (
        account.address_hash
        if account is not None and account.address_hash is not None
        else keccak256_address(key.addr)
    )
    return address_hash, keccak256_word(key.slot)


def storage_block_put(entry: Any) -> None:
    state = get_state()
    canonical = _storage_key(entry.key)
    prior = state.storage_block.get(canonical)
    if prior is None:
        address_hash, slot_hash = _storage_row_hashes(entry.key)
        state.storage_block[canonical] = StorageBlockRow(
            key=entry.key,
            value=entry.value,
            address_hash=address_hash,
            slot_hash=slot_hash,
        )
        return
    entry.value.orig = prior.value.orig
    prior.key = entry.key
    prior.value = entry.value


def storage_block_cache(key: Any, slot_hash: Any, value: int) -> None:
    from evm.primitives.account import StorageValue

    state = get_state()
    canonical = _storage_key(key)
    if canonical in state.storage_block:
        return
    address_hash, _ = _storage_row_hashes(key)
    state.storage_block[canonical] = StorageBlockRow(
        key=key,
        value=StorageValue(curr=int(value), orig=int(value)),
        address_hash=address_hash,
        slot_hash=slot_hash,
    )


def storage_block_clear(address: Any) -> None:
    state = get_state()
    key = _address_key(address)
    state.storage_block = {
        storage_key: row
        for storage_key, row in state.storage_block.items()
        if storage_key[0] != key
    }


def storage_block_iter_begin(address: Any) -> None:
    state = get_state()
    key = _address_key(address)
    state.storage_iterator = sorted(
        (row for storage_key, row in state.storage_block.items() if storage_key[0] == key),
        key=lambda row: _hash_key(row.slot_hash),
    )


def storage_block_iter_next(address: Any) -> Any | None:
    from evm.primitives.account import StorageEntry, StorageTrieEntry

    state = get_state()
    key = _address_key(address)
    while state.storage_iterator:
        row = state.storage_iterator.pop(0)
        if _address_key(row.key.addr) == key:
            return StorageTrieEntry(
                entry=StorageEntry(key=row.key, value=row.value),
                address_hash=row.address_hash,
                slot_hash=row.slot_hash,
            )
    return None


def acct_tx_get(address: Any) -> Any | None:
    row = get_state().account_tx.get(_address_key(address))
    return None if row is None else row.current


def acct_tx_update(address: Any, value: Any) -> None:
    state = get_state()
    key = _address_key(address)
    row = state.account_tx.get(key)
    if row is not None:
        row.current = value
        return
    block = state.account_block.get(key)
    if block is None:
        raise RuntimeError("transaction account update requires a cached account")
    state.account_tx[key] = AccountTxRow(
        current=value,
        original=deepcopy(block.value.curr),
    )


def _account_tx_required(address: Any) -> AccountTxRow:
    state = get_state()
    key = _address_key(address)
    row = state.account_tx.get(key)
    if row is not None:
        return row
    block = state.account_block.get(key)
    if block is None:
        raise RuntimeError("transaction account update requires a cached account")
    row = AccountTxRow(
        current=deepcopy(block.value.curr),
        original=deepcopy(block.value.curr),
    )
    state.account_tx[key] = row
    return row


def acct_tx_set_balance(address: Any, value: int) -> None:
    _account_tx_required(address).current.info.balance = int(value)


def acct_tx_set_nonce(address: Any, value: int) -> None:
    _account_tx_required(address).current.info.nonce = int(value)


def acct_tx_set_code_hash(address: Any, value: Any) -> None:
    _account_tx_required(address).current.info.code_hash = value


def acct_tx_pop() -> Any | None:
    from evm.primitives.account import AcctEntry, AcctValue

    state = get_state()
    if not state.account_tx:
        return None
    key, row = state.account_tx.popitem()
    block = state.account_block.get(key)
    address = block.address if block is not None else _bytes20_from_wire(key)
    return AcctEntry(
        addr=address,
        value=AcctValue(curr=row.current, orig=row.original),
    )


def acct_tx_reset() -> None:
    get_state().account_tx.clear()


def acct_block_get(address: Any) -> Any | None:
    row = get_state().account_block.get(_address_key(address))
    return None if row is None else row.value.curr


def acct_block_write(entry: Any) -> None:
    state = get_state()
    key = _address_key(entry.addr)
    prior = state.account_block.get(key)
    address_hash = (
        prior.address_hash
        if prior is not None and prior.address_hash is not None
        else keccak256_address(entry.addr)
    )
    state.account_block[key] = AccountBlockRow(
        address=entry.addr, value=entry.value, address_hash=address_hash
    )


def acct_block_cache(address: Any, address_hash: Any, account: Any) -> None:
    from evm.primitives.account import AcctValue

    state = get_state()
    key = _address_key(address)
    if key in state.account_block:
        return
    state.account_block[key] = AccountBlockRow(
        address=address,
        value=AcctValue(curr=account, orig=deepcopy(account)),
        address_hash=address_hash,
    )


def acct_block_iter_begin() -> None:
    state = get_state()
    candidates = dict(state.account_block)
    state.account_iterator = sorted(
        candidates.values(), key=lambda row: _hash_key(row.address_hash)
    )


def acct_block_iter_next() -> Any | None:
    from evm.primitives.account import AcctEntry, AcctTrieEntry

    state = get_state()
    if not state.account_iterator:
        return None
    row = state.account_iterator.pop(0)
    return AcctTrieEntry(
        entry=AcctEntry(addr=row.address, value=row.value),
        address_hash=row.address_hash,
    )


def acct_post_storage_root_store(address: Any, value: Any) -> None:
    get_state().post_storage_roots[_address_key(address)] = value


def acct_post_storage_root_read(address: Any) -> Any:
    from evm.primitives.crypto import EMPTY_TRIE_ROOT

    return get_state().post_storage_roots.get(_address_key(address), EMPTY_TRIE_ROOT)


def bal_reset() -> None:
    state = get_state()
    state.bal_accounts.clear()
    state.bal_addresses.clear()
    state.bal_storage_changes.clear()
    state.bal_storage_reads.clear()
    state.bal_balance_changes.clear()
    state.bal_nonce_changes.clear()
    state.bal_code_changes.clear()
    state.bal_iterator.clear()


def _bal_touch(address: Any) -> bytes:
    state = get_state()
    key = _address_key(address)
    state.bal_accounts.add(key)
    state.bal_addresses[key] = address
    return key


def bal_account_touch(address: Any) -> None:
    _bal_touch(address)


def bal_storage_change(index: int, address: Any, slot: int, value: int) -> None:
    key = _bal_touch(address)
    get_state().bal_storage_changes[(key, int(slot))] = (int(index), int(value))


def bal_storage_read(address: Any, slot: int) -> None:
    key = _bal_touch(address)
    get_state().bal_storage_reads.add((key, int(slot)))


def bal_balance_change(index: int, address: Any, value: int) -> None:
    key = _bal_touch(address)
    get_state().bal_balance_changes[key] = (int(index), int(value))


def bal_nonce_change(index: int, address: Any, value: int) -> None:
    key = _bal_touch(address)
    get_state().bal_nonce_changes[key] = (int(index), int(value))


def bal_code_change(index: int, address: Any, value: Any) -> None:
    key = _bal_touch(address)
    get_state().bal_code_changes[key] = (int(index), value)


def bal_prepare_iter() -> None:
    from evm.host.state import (
        BalAccount,
        BalAccountEnd,
        BalBalanceChange,
        BalBalanceChangeEntry,
        BalCodeChange,
        BalCodeChangeEntry,
        BalNonceChange,
        BalNonceChangeEntry,
        BalStorageChange,
        BalStorageChangeEntry,
        BalStorageRead,
    )

    state = get_state()
    events: list[Any] = []
    for address_key in sorted(state.bal_accounts):
        events.append(BalAccount(state.bal_addresses[address_key]))
        changes = sorted(
            (
                (slot, index, value)
                for (account, slot), (index, value) in state.bal_storage_changes.items()
                if account == address_key
            ),
            key=lambda item: (item[0], item[1]),
        )
        changed_slots = {slot for slot, _, _ in changes}
        for slot, index, value in changes:
            events.append(
                BalStorageChange(
                    BalStorageChangeEntry(slot=slot, index=index, value=value)
                )
            )
        for account, slot in sorted(state.bal_storage_reads):
            if account == address_key and slot not in changed_slots:
                events.append(BalStorageRead(slot))
        balance = state.bal_balance_changes.get(address_key)
        if balance is not None:
            events.append(
                BalBalanceChange(
                    BalBalanceChangeEntry(index=balance[0], value=balance[1])
                )
            )
        nonce = state.bal_nonce_changes.get(address_key)
        if nonce is not None:
            events.append(
                BalNonceChange(BalNonceChangeEntry(index=nonce[0], value=nonce[1]))
            )
        code = state.bal_code_changes.get(address_key)
        if code is not None:
            events.append(
                BalCodeChange(
                    BalCodeChangeEntry(index=code[0], code_hash=code[1])
                )
            )
        events.append(BalAccountEnd())
    state.bal_iterator = events


def bal_iter_next() -> Any:
    from evm.host.state import BalEmpty

    state = get_state()
    return state.bal_iterator.pop(0) if state.bal_iterator else BalEmpty()


def warm_reset() -> None:
    state = get_state()
    state.warm_addresses.clear()
    state.warm_slots.clear()


def warm_addr_touch(address: Any) -> bool:
    state = get_state()
    key = _address_key(address)
    was_warm = key in state.warm_addresses
    state.warm_addresses.add(key)
    return was_warm


def warm_slot_touch(address: Any, slot: int) -> bool:
    state = get_state()
    key = _address_slot_key(address, slot)
    was_warm = key in state.warm_slots
    state.warm_slots.add(key)
    return was_warm


def authorization_tracker_reset(capacity: int) -> None:
    del capacity
    get_state().authorizations.clear()


def authorization_tracker_seen(address: Any) -> bool:
    record = get_state().authorizations.get(_address_key(address))
    return False if record is None else record.seen


def authorization_tracker_originally_delegated(address: Any) -> bool:
    record = get_state().authorizations.get(_address_key(address))
    return False if record is None else record.originally_delegated


def authorization_tracker_delegation_set(address: Any) -> bool:
    record = get_state().authorizations.get(_address_key(address))
    return False if record is None else record.delegation_set


def authorization_tracker_commit(
    address: Any, originally_delegated: bool, delegation_set: bool
) -> None:
    state = get_state()
    key = _address_key(address)
    record = state.authorizations.get(key)
    if record is None:
        state.authorizations[key] = AuthorizationRecord(
            seen=True,
            originally_delegated=bool(originally_delegated),
            delegation_set=bool(delegation_set),
        )
        return
    record.seen = True
    record.delegation_set = record.delegation_set or bool(delegation_set)


def logs_tx_reset() -> None:
    state = get_state()
    state.transaction_log_start = len(state.logs)
    state.current_log = None


def log_begin(address: Any) -> None:
    state = get_state()
    state.logs.append(
        LogRecord(address=address, data_offset=len(state.log_data))
    )
    state.current_log = len(state.logs) - 1


def _current_log_record() -> LogRecord:
    state = get_state()
    if state.current_log is None:
        raise RuntimeError("log data was appended before log_begin")
    return state.logs[state.current_log]


def log_add_topic(value: int) -> None:
    _current_log_record().topics.append(int(value))


def _append_log_data(payload: bytes) -> None:
    state = get_state()
    record = _current_log_record()
    state.log_data.extend(payload)
    record.data_length += len(payload)


def log_add_data_memory(value: Any) -> None:
    _append_log_data(_region_bytes("memory", value))


def log_add_data_word(value: int) -> None:
    _append_log_data(_word_bytes(value))


def logs_tx_start() -> int:
    return get_state().transaction_log_start


def logs_tx_count() -> int:
    state = get_state()
    return len(state.logs) - state.transaction_log_start


def log_address(index: int) -> Any:
    return get_state().logs[int(index)].address


def log_topics_count(index: int) -> int:
    return len(get_state().logs[int(index)].topics)


def log_topic(index: int, topic: int) -> int:
    return get_state().logs[int(index)].topics[int(topic)]


def log_data_offset(index: int) -> int:
    return get_state().logs[int(index)].data_offset


def log_data_length(index: int) -> int:
    return get_state().logs[int(index)].data_length


def accelerator_ripemd160(value: Any) -> bool:
    state = get_state()
    return state.accelerators.ripemd160(state, value)


def accelerator_modexp(
    value: Any, base_length: int, exponent_length: int, modulus_length: int
) -> bool:
    state = get_state()
    return state.accelerators.modexp(
        state, value, base_length, exponent_length, modulus_length
    )


def accelerator_bn254_add(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bn254_add(state, value)


def accelerator_bn254_mul(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bn254_mul(state, value)


def accelerator_bn254_pairing(value: Any) -> int:
    state = get_state()
    return state.accelerators.bn254_pairing(state, value)


def accelerator_blake2f(value: Any, rounds: int, final: Any) -> bool:
    state = get_state()
    return state.accelerators.blake2f(state, value, rounds, final)


def accelerator_kzg_point_evaluation(value: Any) -> bool:
    state = get_state()
    return state.accelerators.kzg_point_evaluation(state, value)


def accelerator_bls_g1_add(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_g1_add(state, value)


def accelerator_bls_g1_msm(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_g1_msm(state, value)


def accelerator_bls_g2_add(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_g2_add(state, value)


def accelerator_bls_g2_msm(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_g2_msm(state, value)


def accelerator_bls_pairing(value: Any) -> int:
    state = get_state()
    return state.accelerators.bls_pairing(state, value)


def accelerator_bls_map_fp_to_g1(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_map_fp_to_g1(state, value)


def accelerator_bls_map_fp2_to_g2(value: Any) -> bool:
    state = get_state()
    return state.accelerators.bls_map_fp2_to_g2(state, value)


def accelerator_p256_verify(value: Any) -> bool:
    state = get_state()
    return state.accelerators.p256_verify(state, value)


def host_ecrecover(digest: Any, y_parity: Any, r: int, s: int) -> Any:
    state = get_state()
    return state.accelerators.ecrecover(state, digest, y_parity, r, s)
