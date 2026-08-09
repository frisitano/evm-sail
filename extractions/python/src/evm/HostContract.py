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
STORAGE_INITIAL_GENERATION = 1


#: Encoded ``pairing_check_result``: below two is malformed input, and the
#: low-order parity of a valid value is the pairing outcome.
PAIRING_MALFORMED = 0
PAIRING_VALID_FALSE = 2
PAIRING_VALID_TRUE = 3

#: ``ZKVM_EOK`` from ``extractions/c/zkvm_accelerators.h``.
ZKVM_EOK = 0

#: EIP-2537 carries each Fp in a 64-byte word with 16 leading zero bytes; the
#: accelerator ABI takes the compact 48-byte value.  Sail validates the zero
#: padding before these adapters run.
FP_WIRE = 64
FP_COMPACT = 48
FP_PAD = FP_WIRE - FP_COMPACT


class AcceleratorContract:
    """The shared cryptographic accelerators, bound to the canonical library.

    These operations are axioms of the model, so rather than reimplement them
    this binds the same Rust accelerator cdylib that the C and ZisK guests
    link (``zkvm/accel-host``).  There is one crypto implementation across
    every executable target, and the Python extraction exercises it rather
    than a parallel one.

    Only the wire adaptation lives here, mirroring
    ``extractions/c/spec/contract/precompiles.c``: EVM calldata is
    zero-extended on short reads, and BLS12-381 field elements are narrowed
    from their 64-byte EIP-2537 words to the accelerator's compact 48-byte
    form and widened again on the way out.
    """

    def ripemd160(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        digest = _out_buffer(32)
        if _accel().zkvm_ripemd160(raw, len(raw), digest) != ZKVM_EOK:
            return False
        _store_output(state, digest.raw)
        return True

    def modexp(
        self,
        state: HostState,
        data: Any,
        base_length: int,
        exponent_length: int,
        modulus_length: int,
    ) -> bool:
        base_length = int(base_length)
        exponent_length = int(exponent_length)
        modulus_length = int(modulus_length)
        # The three 32-byte length fields precede the operands.
        raw = _buffer_read(
            _calldata_bytes(data),
            0,
            96 + base_length + exponent_length + modulus_length,
        )
        base = 96
        exponent = base + base_length
        modulus = exponent + exponent_length
        result = _out_buffer(modulus_length)
        if (
            _accel().zkvm_modexp(
                raw[base:exponent],
                base_length,
                raw[exponent:modulus],
                exponent_length,
                raw[modulus:],
                modulus_length,
                result,
            )
            != ZKVM_EOK
        ):
            return False
        _store_output(state, result.raw)
        return True

    def bn254_add(self, state: HostState, data: Any) -> bool:
        raw = _buffer_read(_calldata_bytes(data), 0, 128)
        result = _out_buffer(64)
        if _accel().zkvm_bn254_g1_add(raw[:64], raw[64:], result) != ZKVM_EOK:
            return False
        _store_output(state, result.raw)
        return True

    def bn254_mul(self, state: HostState, data: Any) -> bool:
        raw = _buffer_read(_calldata_bytes(data), 0, 96)
        result = _out_buffer(64)
        if _accel().zkvm_bn254_g1_mul(raw[:64], raw[64:], result) != ZKVM_EOK:
            return False
        _store_output(state, result.raw)
        return True

    def bn254_pairing(self, state: HostState, data: Any) -> int:
        raw = _calldata_bytes(data)
        if len(raw) % 192 != 0:
            return PAIRING_MALFORMED
        # The accelerator pair layout is the wire layout for BN254.
        return _pairing_status(
            _accel().zkvm_bn254_pairing, raw, len(raw) // 192
        )

    def blake2f(self, state: HostState, data: Any, rounds: int, final: Any) -> bool:
        raw = _calldata_bytes(data)
        rounds = int(rounds)
        final = int(final)
        if len(raw) != 213 or rounds > 0xFFFFFFFF or final > 1:
            return False
        # The state word is updated in place and is the result.
        h = _out_buffer(64, _buffer_read(raw, 4, 64))
        message = _buffer_read(raw, 68, 128)
        offset = _buffer_read(raw, 196, 16)
        if _accel().zkvm_blake2f(rounds, h, message, offset, final) != ZKVM_EOK:
            return False
        _store_output(state, h.raw)
        return True

    def kzg_point_evaluation(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 192:
            return False
        # The Sail caller owns the constant success payload, so this reports
        # only whether the proof verified.
        verified = _bool_out()
        status = _accel().zkvm_kzg_point_eval(
            _buffer_read(raw, 96, 48),
            _buffer_read(raw, 32, 32),
            _buffer_read(raw, 64, 32),
            _buffer_read(raw, 144, 48),
            verified,
        )
        return status == ZKVM_EOK and bool(verified.value)

    def bls_g1_add(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 256:
            return False
        result = _out_buffer(96)
        if (
            _accel().zkvm_bls12_g1_add(
                _compact_g1(raw, 0), _compact_g1(raw, 128), result
            )
            != ZKVM_EOK
        ):
            return False
        _store_output(state, _pad_g1(result.raw))
        return True

    def bls_g2_add(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 512:
            return False
        result = _out_buffer(192)
        if (
            _accel().zkvm_bls12_g2_add(
                _compact_g2(raw, 0), _compact_g2(raw, 256), result
            )
            != ZKVM_EOK
        ):
            return False
        _store_output(state, _pad_g2(result.raw))
        return True

    def bls_g1_msm(self, state: HostState, data: Any) -> bool:
        return _bls_msm(
            state, data, 160, 128, 96, _compact_g1, _pad_g1,
            _accel().zkvm_bls12_g1_msm,
        )

    def bls_g2_msm(self, state: HostState, data: Any) -> bool:
        return _bls_msm(
            state, data, 288, 256, 192, _compact_g2, _pad_g2,
            _accel().zkvm_bls12_g2_msm,
        )

    def bls_pairing(self, state: HostState, data: Any) -> int:
        raw = _calldata_bytes(data)
        if len(raw) == 0 or len(raw) % 384 != 0:
            return PAIRING_MALFORMED
        count = len(raw) // 384
        pairs = bytearray()
        for index in range(count):
            base = index * 384
            pairs += _compact_g1(raw, base)
            pairs += _compact_g2(raw, base + 128)
        return _pairing_status(
            _accel().zkvm_bls12_pairing, bytes(pairs), count
        )

    def bls_map_fp_to_g1(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 64:
            return False
        result = _out_buffer(96)
        if _accel().zkvm_bls12_map_fp_to_g1(_compact_fp(raw, 0), result) != ZKVM_EOK:
            return False
        _store_output(state, _pad_g1(result.raw))
        return True

    def bls_map_fp2_to_g2(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 128:
            return False
        fp2 = _compact_fp(raw, 0) + _compact_fp(raw, 64)
        result = _out_buffer(192)
        if _accel().zkvm_bls12_map_fp2_to_g2(fp2, result) != ZKVM_EOK:
            return False
        _store_output(state, _pad_g2(result.raw))
        return True

    def p256_verify(self, state: HostState, data: Any) -> bool:
        raw = _calldata_bytes(data)
        if len(raw) != 160:
            return False
        verified = _bool_out()
        status = _accel().zkvm_secp256r1_verify(
            raw[0:32], raw[32:96], raw[96:160], verified
        )
        return status == ZKVM_EOK and bool(verified.value)

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


_accel_library: Any = None


def _accel() -> Any:
    """The canonical accelerator cdylib, loaded once.

    This is the very library the native and ZisK guests link, so every
    executable target shares one cryptographic implementation.  Override the
    location with ``EVMSAIL_ACCEL_LIB`` when it is not in the build tree.
    """
    global _accel_library
    if _accel_library is None:
        import ctypes
        import os

        override = os.environ.get("EVMSAIL_ACCEL_LIB")
        if override:
            candidates = [override]
        else:
            # .../extractions/python/src/evm/HostContract.py -> repository root
            root = os.path.abspath(
                os.path.join(os.path.dirname(__file__), *[os.pardir] * 4)
            )
            built = os.path.join(
                root, "zkvm", "accel-host", "target", "release"
            )
            candidates = [
                os.path.join(built, f"libzkvm_accel_host.{suffix}")
                for suffix in ("dylib", "so")
            ]
        for candidate in candidates:
            if os.path.exists(candidate):
                _accel_library = _declare_accel(ctypes.CDLL(candidate))
                break
        else:
            raise RuntimeError(
                "cannot locate the accelerator library "
                f"(tried {', '.join(candidates)}); build it with "
                "zkvm/native-runner/build.sh or set EVMSAIL_ACCEL_LIB"
            )
    return _accel_library


def _declare_accel(lib: Any) -> Any:
    """Pin argument types so ctypes never guesses at the ABI."""
    import ctypes

    data = ctypes.c_char_p
    size = ctypes.c_size_t
    out = ctypes.c_void_p
    flag = ctypes.POINTER(ctypes.c_bool)
    signatures = {
        "zkvm_ripemd160": [data, size, out],
        "zkvm_modexp": [data, size, data, size, data, size, out],
        "zkvm_bn254_g1_add": [data, data, out],
        "zkvm_bn254_g1_mul": [data, data, out],
        "zkvm_bn254_pairing": [data, size, flag],
        "zkvm_blake2f": [ctypes.c_uint32, out, data, data, ctypes.c_uint8],
        "zkvm_kzg_point_eval": [data, data, data, data, flag],
        "zkvm_bls12_g1_add": [data, data, out],
        "zkvm_bls12_g2_add": [data, data, out],
        "zkvm_bls12_g1_msm": [data, size, out],
        "zkvm_bls12_g2_msm": [data, size, out],
        "zkvm_bls12_pairing": [data, size, flag],
        "zkvm_bls12_map_fp_to_g1": [data, out],
        "zkvm_bls12_map_fp2_to_g2": [data, out],
        "zkvm_secp256r1_verify": [data, data, data, flag],
    }
    for name, argtypes in signatures.items():
        entry = getattr(lib, name)
        entry.argtypes = argtypes
        entry.restype = ctypes.c_int
    return lib


def _out_buffer(length: int, initial: bytes = b"") -> Any:
    import ctypes

    return ctypes.create_string_buffer(
        initial.ljust(length, b"\x00") if initial else bytes(length), length
    )


def _bool_out() -> Any:
    import ctypes

    return ctypes.c_bool(False)


def _pairing_status(entry: Any, pairs: bytes, count: int) -> int:
    """Run a pairing check and encode it as ``pairing_check_result``."""
    verified = _bool_out()
    if entry(pairs, count, verified) != ZKVM_EOK:
        return PAIRING_MALFORMED
    return PAIRING_VALID_TRUE if verified.value else PAIRING_VALID_FALSE


def _compact_fp(raw: bytes, offset: int) -> bytes:
    """Narrow one 64-byte EIP-2537 word to its compact 48-byte value."""
    return _buffer_read(raw, offset + FP_PAD, FP_COMPACT)


def _pad_fp(compact: bytes) -> bytes:
    return bytes(FP_PAD) + compact


def _compact_g1(raw: bytes, offset: int) -> bytes:
    return _compact_fp(raw, offset) + _compact_fp(raw, offset + FP_WIRE)


def _pad_g1(compact: bytes) -> bytes:
    return _pad_fp(compact[:FP_COMPACT]) + _pad_fp(compact[FP_COMPACT:])


def _compact_g2(raw: bytes, offset: int) -> bytes:
    # The accelerator G2 layout matches EIP-2537 component order (c0 then c1),
    # so the four field elements narrow straight through.
    return b"".join(
        _compact_fp(raw, offset + index * FP_WIRE) for index in range(4)
    )


def _pad_g2(compact: bytes) -> bytes:
    return b"".join(
        _pad_fp(compact[index * FP_COMPACT : (index + 1) * FP_COMPACT])
        for index in range(4)
    )


def _bls_msm(
    state: HostState,
    data: Any,
    wire_stride: int,
    point_wire: int,
    point_compact: int,
    compact: Any,
    pad: Any,
    entry: Any,
) -> bool:
    """Shared body for the BLS12-381 multi-scalar multiplications."""
    raw = _calldata_bytes(data)
    if len(raw) == 0 or len(raw) % wire_stride != 0:
        return False
    count = len(raw) // wire_stride
    pairs = bytearray()
    for index in range(count):
        base = index * wire_stride
        pairs += compact(raw, base)
        pairs += _buffer_read(raw, base + point_wire, WORD_BYTES)
    result = _out_buffer(point_compact)
    if entry(bytes(pairs), count, result) != ZKVM_EOK:
        return False
    _store_output(state, pad(result.raw))
    return True


def _calldata_bytes(value: Any) -> bytes:
    """Materialize a ``CalldataSlice`` whichever provenance it carries."""
    from evm import InputCalldata, MemoryCalldata

    match value:
        case InputCalldata(inner):
            return _region_bytes("input", inner)
        case MemoryCalldata(inner):
            return _region_bytes("memory", inner)
        case _:
            raise TypeError(f"not a CalldataSlice: {value!r}")


def _buffer_read(data: bytes, start: int, length: int) -> bytes:
    """EVM buffer semantics: a read past the end is zero-filled."""
    chunk = data[start : start + length]
    return bytes(chunk) + bytes(length - len(chunk))


def _store_output(state: HostState, payload: bytes) -> None:
    state.output_bytes[:] = payload


@dataclass(slots=True)
class MemoryFrame:
    base: int
    length: int = 0


@dataclass(slots=True)
class JumpdestTable:
    code_length: int
    positions: set[int] = field(default_factory=set)


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
class StorageBlockCacheRow:
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
class StorageTxRow:
    key: Any
    value: Any
    generation: int


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
    storage_tx: dict[tuple[bytes, int], StorageTxRow] = field(default_factory=dict)
    storage_generations: dict[bytes, int] = field(default_factory=dict)
    storage_next_generation: int = STORAGE_INITIAL_GENERATION
    storage_block: dict[tuple[bytes, int], StorageBlockCacheRow] = field(
        default_factory=dict
    )
    storage_iterator: list[StorageBlockCacheRow] = field(default_factory=list)
    account_tx: dict[bytes, Any] = field(default_factory=dict)
    account_block: dict[bytes, AccountBlockRow] = field(default_factory=dict)
    account_iterator: list[AccountBlockRow] = field(default_factory=list)
    journal: list[Any] = field(default_factory=list)
    warm_addresses: dict[bytes, int] = field(default_factory=dict)
    warm_slots: dict[tuple[bytes, int], int] = field(default_factory=dict)
    current_warm_epoch: int = 1
    logs: list[LogRecord] = field(default_factory=list)
    transaction_log_start: int = 0
    current_log: int | None = None
    authorizations: dict[bytes, AuthorizationRecord] = field(default_factory=dict)
    bal_accounts: set[bytes] = field(default_factory=set)
    bal_addresses: dict[bytes, Any] = field(default_factory=dict)
    bal_storage_changes: dict[tuple[bytes, int], dict[int, int]] = field(
        default_factory=dict
    )
    bal_storage_reads: set[tuple[bytes, int]] = field(default_factory=set)
    bal_balance_changes: dict[bytes, dict[int, int]] = field(default_factory=dict)
    bal_nonce_changes: dict[bytes, dict[int, int]] = field(default_factory=dict)
    bal_code_changes: dict[bytes, dict[int, Any]] = field(default_factory=dict)
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
    start = int(value.bytes)
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
    from evm.primitives.bytes import scratch_slice

    state = get_state()
    start = int(offset)
    end = start + len(payload)
    _ensure_length(state.scratch_bytes, end)
    state.scratch_bytes[start:end] = payload
    return scratch_slice(0, end)


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
    from evm.primitives.bytes import evm_memory_slice

    state = get_state()
    frame = _current_memory_frame(state)
    length = int(required)
    _ensure_length(state.memory_bytes, frame.base + length)
    frame.length = max(frame.length, length)
    return evm_memory_slice(frame.base, length)


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


# The reference reading of the opaque StackTop cursor token is the frame
# height (the spec C ABI's choice): slot `index` below cursor `top` addresses
# position `top - 1 - index` of the active frame's word storage.  The cursor
# is threaded by value through the generated interpreter; this module owns
# only the word storage and the frame-of-frames structure.

_STACK_TOP_WIDTH = 64
_STACK_TOP_MASK = (1 << _STACK_TOP_WIDTH) - 1


def _stack_top(value: int) -> Any:
    """`StackTop` is `bits(64)`, so the cursor crosses as an exact bitvector."""
    from evm._runtime import Bits

    return Bits(_STACK_TOP_WIDTH, int(value) & _STACK_TOP_MASK)


def stack_reset() -> Any:
    state = get_state()
    state.operand_stack.clear()
    state.operand_stack_frames.clear()
    return _stack_top(0)


def operand_stack_push_empty_frame() -> Any:
    get_state().operand_stack_frames.append(len(get_state().operand_stack))
    return _stack_top(0)


def operand_stack_pop_frame() -> None:
    state = get_state()
    if not state.operand_stack_frames:
        raise RuntimeError("cannot pop the root operand-stack frame")
    base = state.operand_stack_frames.pop()
    del state.operand_stack[base:]


def _stack_base(state: HostState) -> int:
    return state.operand_stack_frames[-1] if state.operand_stack_frames else 0


def _stack_slot_position(state: HostState, top: int, index: int) -> int:
    return _stack_base(state) + int(top) - 1 - int(index)


def stack_top_height(top: Any) -> int:
    return int(top)


def stack_slot_read(top: Any, index: int) -> int:
    state = get_state()
    position = _stack_slot_position(state, top, index)
    if position < _stack_base(state) or position >= len(state.operand_stack):
        raise RuntimeError("operand stack slot out of range")
    return state.operand_stack[position]


def stack_slot_write(top: Any, index: int, value: int) -> None:
    state = get_state()
    position = _stack_slot_position(state, top, index)
    if position < _stack_base(state):
        raise RuntimeError("operand stack slot out of range")
    while len(state.operand_stack) <= position:
        state.operand_stack.append(0)
    state.operand_stack[position] = int(value)


def stack_top_advance(top: Any, count: int) -> Any:
    return _stack_top(int(top) + int(count))


def stack_top_retreat(top: Any, count: int) -> Any:
    return _stack_top(int(top) - int(count))


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


def code_keccak256(value: Any) -> Any:
    return _keccak_region("code", value)


def output_keccak256(value: Any) -> Any:
    return _keccak_region("output", value)


def log_data_keccak256(value: Any) -> Any:
    return _keccak_region("log", value)


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


def code_region_from_delegation(address: Any) -> Any:
    return _append_code(DELEGATION_PREFIX + _fixed_wire_bytes(address))


def jumpdest_table_alloc(code: Any) -> int:
    state = get_state()
    index = state.next_jumpdest_table
    state.next_jumpdest_table += 1
    state.jumpdest_tables[index] = JumpdestTable(code_length=int(code.len))
    return index


def jumpdest_table_mark(table: int, code_length: int, position: int) -> bool:
    state = get_state()
    entry = state.jumpdest_tables.get(int(table))
    if entry is None or entry.code_length != int(code_length):
        return False
    index = int(position)
    if index < 0 or index >= entry.code_length:
        return False
    entry.positions.add(index)
    return True


def jumpdest_ref_contains(table: int, code_length: int, destination: int) -> bool:
    entry = get_state().jumpdest_tables.get(int(table))
    index = int(destination)
    if entry is None or entry.code_length != int(code_length):
        return False
    if index < 0 or index >= entry.code_length:
        return False
    return index in entry.positions


def code_db_store(code: Any) -> Any:
    from evm.primitives.code import code_bytes

    digest = _keccak(_region_bytes("code", code_bytes(code)))
    get_state().code_db[bytes(digest)] = code
    return _bytes32_from_wire(digest)


def _code_db_get(value: Any) -> Any | None:
    return get_state().code_db.get(_hash_key(value))


def code_db_lookup(value: Any) -> Any:
    from evm.primitives.code import EMPTY_CODE

    code = _code_db_get(value)
    return EMPTY_CODE if code is None else code


def code_db_read_delegation(value: Any) -> Any:
    from evm.prelude import AddressResult
    from evm.primitives.code import code_bytes

    code = _code_db_get(value)
    if code is None:
        return AddressResult(success=False, address=_bytes20_from_wire(bytes(20)))
    payload = _region_bytes("code", code_bytes(code))
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
    from evm.host.journal import JournalTransientChange, JournalTransientChanged

    state = get_state()
    key = _address_slot_key(address, slot)
    state.journal.append(
        JournalTransientChanged(
            JournalTransientChange(
                address=address, slot=int(slot), prior=state.transient.get(key, 0)
            )
        )
    )
    state.transient[key] = int(value)


def transient_load(address: Any, slot: int) -> int:
    return get_state().transient.get(_address_slot_key(address, slot), 0)


def _journal_account_row(state: HostState, address: Any) -> AccountTxRow:
    row = state.account_tx.get(_address_key(address))
    if row is None:
        raise RuntimeError("journal entry references a missing transaction account")
    return row


def _journal_restore(state: HostState, entry: Any) -> None:
    from evm.host.journal import (
        JournalAccountBalanceChanged,
        JournalAccountCodeHashChanged,
        JournalAccountCreatedChanged,
        JournalAccountExistsChanged,
        JournalAccountNonceChanged,
        JournalAccountSelfdestructedChanged,
        JournalAccountStorageGenerationChanged,
        JournalFrameCheckpointed,
        JournalFrameCommitted,
        JournalLogAppended,
        JournalStorageRowGenerationChanged,
        JournalStorageValueChanged,
        JournalTransactionAccountListed,
        JournalTransactionStorageListed,
        JournalTransientChanged,
        JournalWarmAccountChanged,
        JournalWarmStorageChanged,
    )

    if isinstance(entry, JournalTransientChanged):
        change = entry.value
        state.transient[_address_slot_key(change.address, change.slot)] = int(
            change.prior
        )
    elif isinstance(entry, JournalWarmAccountChanged):
        state.warm_addresses[_address_key(entry.value.address)] = int(
            entry.value.prior_epoch
        )
    elif isinstance(entry, JournalWarmStorageChanged):
        state.warm_slots[_storage_key(entry.value.key)] = int(entry.value.prior_epoch)
    elif isinstance(entry, JournalAccountBalanceChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.info.balance = entry.value.prior
    elif isinstance(entry, JournalAccountNonceChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.info.nonce = entry.value.prior
    elif isinstance(entry, JournalAccountCodeHashChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.info.code_hash = entry.value.prior
    elif isinstance(entry, JournalAccountExistsChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.present = entry.value.prior
    elif isinstance(entry, JournalAccountCreatedChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.created = entry.value.prior
    elif isinstance(entry, JournalAccountSelfdestructedChanged):
        row = _journal_account_row(state, entry.value.address)
        row.current.selfdestructed = entry.value.prior
    elif isinstance(entry, JournalTransactionAccountListed):
        if not state.account_tx:
            raise RuntimeError("journal account listing has no transaction row")
        state.account_tx.popitem()
    elif isinstance(entry, JournalTransactionStorageListed):
        key = _address_key(entry.value)
        listed = None
        for storage_key in reversed(state.storage_tx):
            if storage_key[0] == key:
                listed = storage_key
                break
        if listed is None:
            raise RuntimeError("journal storage listing has no transaction row")
        del state.storage_tx[listed]
    elif isinstance(entry, JournalLogAppended):
        record = state.logs.pop()
        del state.log_data[record.data_offset:]
        state.current_log = None
    elif isinstance(entry, JournalStorageValueChanged):
        state.storage_tx[_storage_key(entry.value.key)].value.curr = entry.value.prior
    elif isinstance(entry, JournalStorageRowGenerationChanged):
        state.storage_tx[_storage_key(entry.value.key)].generation = int(
            entry.value.prior
        )
    elif isinstance(entry, JournalAccountStorageGenerationChanged):
        key = _address_key(entry.value.address)
        prior = int(entry.value.prior)
        state.storage_generations[key] = prior
        row = state.account_tx.get(key)
        if row is not None:
            row.current.storage_cleared = (
                row.original.storage_cleared or prior != STORAGE_INITIAL_GENERATION
            )
    elif isinstance(entry, (JournalFrameCheckpointed, JournalFrameCommitted)):
        pass
    else:
        raise RuntimeError("unknown state-journal entry")


def _open_checkpoint_index(state: HostState) -> int:
    from evm.host.journal import JournalFrameCheckpointed, JournalFrameCommitted

    closed = 0
    for index in range(len(state.journal) - 1, -1, -1):
        entry = state.journal[index]
        if isinstance(entry, JournalFrameCommitted):
            closed += 1
        elif isinstance(entry, JournalFrameCheckpointed):
            if closed == 0:
                return index
            closed -= 1
    raise RuntimeError("state journal has no open checkpoint")


def state_journal_reset() -> None:
    get_state().journal.clear()


def state_journal_checkpoint() -> None:
    from evm.host.journal import JournalFrameCheckpointed

    get_state().journal.append(JournalFrameCheckpointed())


def state_journal_revert() -> None:
    state = get_state()
    marker = _open_checkpoint_index(state)
    for index in range(len(state.journal) - 1, marker, -1):
        _journal_restore(state, state.journal[index])
    del state.journal[marker:]


def state_journal_commit() -> None:
    from evm.host.journal import JournalFrameCommitted

    state = get_state()
    _open_checkpoint_index(state)
    state.journal.append(JournalFrameCommitted())


def _storage_generation(state: HostState, key: bytes) -> int:
    return state.storage_generations.get(key, STORAGE_INITIAL_GENERATION)


def storage_tx_update(entry: Any) -> None:
    from evm.host.journal import (
        JournalStorageRowGenerationChange,
        JournalStorageRowGenerationChanged,
        JournalStorageValueChange,
        JournalStorageValueChanged,
        JournalTransactionStorageListed,
    )
    from evm.primitives.account import StorageValue

    state = get_state()
    canonical = _storage_key(entry.key)
    generation = _storage_generation(state, canonical[0])
    row = state.storage_tx.get(canonical)
    if row is None:
        state.journal.append(JournalTransactionStorageListed(entry.key.addr))
        state.storage_tx[canonical] = StorageTxRow(
            key=entry.key,
            value=StorageValue(curr=entry.value.curr, orig=entry.value.orig),
            generation=generation,
        )
        return
    if int(row.value.curr) != int(entry.value.curr):
        state.journal.append(
            JournalStorageValueChanged(
                JournalStorageValueChange(key=row.key, prior=row.value.curr)
            )
        )
        row.value.curr = entry.value.curr
    if row.generation != generation:
        state.journal.append(
            JournalStorageRowGenerationChanged(
                JournalStorageRowGenerationChange(key=row.key, prior=row.generation)
            )
        )
        row.generation = generation


def storage_tx_get(key: Any) -> Any:
    from evm.primitives.account import (
        StorageTxCleared,
        StorageTxHit,
        StorageTxMiss,
    )

    state = get_state()
    canonical = _storage_key(key)
    generation = _storage_generation(state, canonical[0])
    row = state.storage_tx.get(canonical)
    if row is not None and row.generation == generation:
        return StorageTxHit(row.value)
    if generation != STORAGE_INITIAL_GENERATION:
        return StorageTxCleared()
    return StorageTxMiss()


def storage_tx_pop() -> Any:
    from evm.primitives.account import (
        StorageEntry,
        StorageTxPopExhausted,
        StorageTxPopRow,
    )

    state = get_state()
    while state.storage_tx:
        (address_key, _), row = state.storage_tx.popitem()
        if row.generation == _storage_generation(state, address_key):
            return StorageTxPopRow(StorageEntry(key=row.key, value=row.value))
    return StorageTxPopExhausted()


def storage_tx_clear(address: Any) -> None:
    from evm.host.journal import (
        JournalAccountStorageGenerationChange,
        JournalAccountStorageGenerationChanged,
    )

    state = get_state()
    key = _address_key(address)
    state.journal.append(
        JournalAccountStorageGenerationChanged(
            JournalAccountStorageGenerationChange(
                address=address, prior=_storage_generation(state, key)
            )
        )
    )
    state.storage_next_generation += 1
    state.storage_generations[key] = state.storage_next_generation


def storage_tx_reset() -> None:
    state = get_state()
    state.storage_tx.clear()
    state.storage_generations.clear()
    state.storage_next_generation = STORAGE_INITIAL_GENERATION


def storage_has_writes(address: Any) -> bool:
    state = get_state()
    key = _address_key(address)
    generation = _storage_generation(state, key)
    tx_writes = any(
        storage_key[0] == key
        and row.generation == generation
        and int(row.value.curr) != 0
        for storage_key, row in state.storage_tx.items()
    )
    if tx_writes:
        return True
    if generation != STORAGE_INITIAL_GENERATION:
        return False
    return any(
        storage_key[0] == key and int(row.value.curr) != 0
        for storage_key, row in state.storage_block.items()
    )


def storage_block_get(key: Any) -> Any:
    from evm.prelude import ZERO_WORD
    from evm.primitives.account import StorageBlockRow, StorageValue

    row = get_state().storage_block.get(_storage_key(key))
    if row is None:
        return StorageBlockRow(
            found=False, value=StorageValue(curr=ZERO_WORD, orig=ZERO_WORD)
        )
    return StorageBlockRow(found=True, value=row.value)


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
    from evm.primitives.account import StorageValue

    state = get_state()
    canonical = _storage_key(entry.key)
    prior = state.storage_block.get(canonical)
    if prior is None:
        address_hash, slot_hash = _storage_row_hashes(entry.key)
        state.storage_block[canonical] = StorageBlockCacheRow(
            key=entry.key,
            value=StorageValue(curr=entry.value.curr, orig=entry.value.orig),
            address_hash=address_hash,
            slot_hash=slot_hash,
        )
        return
    prior.key = entry.key
    prior.value = StorageValue(curr=entry.value.curr, orig=prior.value.orig)


def storage_block_cache(key: Any, slot_hash: Any, value: int) -> None:
    from evm.primitives.account import StorageValue

    state = get_state()
    canonical = _storage_key(key)
    if canonical in state.storage_block:
        return
    address_hash, _ = _storage_row_hashes(key)
    state.storage_block[canonical] = StorageBlockCacheRow(
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


def storage_block_iter_next(address: Any) -> Any:
    from evm.primitives.account import (
        StorageBlockIterExhausted,
        StorageBlockIterRow,
        StorageEntry,
        StorageTrieEntry,
    )

    state = get_state()
    key = _address_key(address)
    while state.storage_iterator:
        row = state.storage_iterator.pop(0)
        if _address_key(row.key.addr) == key:
            return StorageBlockIterRow(
                StorageTrieEntry(
                    entry=StorageEntry(key=row.key, value=row.value),
                    address_hash=row.address_hash,
                    slot_hash=row.slot_hash,
                )
            )
    return StorageBlockIterExhausted()


def acct_tx_get(address: Any) -> Any:
    from evm.primitives.account import EMPTY_ACCOUNT, AccountRow

    row = get_state().account_tx.get(_address_key(address))
    if row is None:
        return AccountRow(found=False, account=deepcopy(EMPTY_ACCOUNT))
    return AccountRow(found=True, account=row.current)


def acct_tx_update(address: Any, value: Any) -> None:
    from evm.host.journal import (
        JournalAccountBalanceChange,
        JournalAccountBalanceChanged,
        JournalAccountCodeHashChange,
        JournalAccountCodeHashChanged,
        JournalAccountCreatedChange,
        JournalAccountCreatedChanged,
        JournalAccountExistsChange,
        JournalAccountExistsChanged,
        JournalAccountNonceChange,
        JournalAccountNonceChanged,
        JournalAccountSelfdestructedChange,
        JournalAccountSelfdestructedChanged,
        JournalTransactionAccountListed,
    )

    state = get_state()
    key = _address_key(address)
    row = state.account_tx.get(key)
    if row is None:
        block = state.account_block.get(key)
        if block is None:
            raise RuntimeError("transaction account update requires a cached account")
        state.journal.append(JournalTransactionAccountListed())
        state.account_tx[key] = AccountTxRow(
            current=value,
            original=deepcopy(block.value.curr),
        )
        return
    prior = row.current
    if int(prior.info.balance) != int(value.info.balance):
        state.journal.append(
            JournalAccountBalanceChanged(
                JournalAccountBalanceChange(address=address, prior=prior.info.balance)
            )
        )
    if int(prior.info.nonce) != int(value.info.nonce):
        state.journal.append(
            JournalAccountNonceChanged(
                JournalAccountNonceChange(address=address, prior=prior.info.nonce)
            )
        )
    if prior.info.code_hash != value.info.code_hash:
        state.journal.append(
            JournalAccountCodeHashChanged(
                JournalAccountCodeHashChange(
                    address=address, prior=prior.info.code_hash
                )
            )
        )
    if prior.present != value.present:
        state.journal.append(
            JournalAccountExistsChanged(
                JournalAccountExistsChange(address=address, prior=prior.present)
            )
        )
    if prior.created != value.created:
        state.journal.append(
            JournalAccountCreatedChanged(
                JournalAccountCreatedChange(address=address, prior=prior.created)
            )
        )
    if prior.selfdestructed != value.selfdestructed:
        state.journal.append(
            JournalAccountSelfdestructedChanged(
                JournalAccountSelfdestructedChange(
                    address=address, prior=prior.selfdestructed
                )
            )
        )
    row.current = value


def _account_tx_required(address: Any) -> AccountTxRow:
    from evm.host.journal import JournalTransactionAccountListed

    state = get_state()
    key = _address_key(address)
    row = state.account_tx.get(key)
    if row is not None:
        return row
    block = state.account_block.get(key)
    if block is None:
        raise RuntimeError("transaction account update requires a cached account")
    state.journal.append(JournalTransactionAccountListed())
    row = AccountTxRow(
        current=deepcopy(block.value.curr),
        original=deepcopy(block.value.curr),
    )
    state.account_tx[key] = row
    return row


def acct_tx_set_balance(address: Any, value: int) -> None:
    from evm.host.journal import (
        JournalAccountBalanceChange,
        JournalAccountBalanceChanged,
    )

    state = get_state()
    row = _account_tx_required(address)
    if int(row.current.info.balance) != int(value):
        state.journal.append(
            JournalAccountBalanceChanged(
                JournalAccountBalanceChange(
                    address=address, prior=row.current.info.balance
                )
            )
        )
        row.current.info.balance = int(value)


def acct_tx_set_nonce(address: Any, value: int) -> None:
    from evm.host.journal import (
        JournalAccountNonceChange,
        JournalAccountNonceChanged,
    )

    state = get_state()
    row = _account_tx_required(address)
    if int(row.current.info.nonce) != int(value):
        state.journal.append(
            JournalAccountNonceChanged(
                JournalAccountNonceChange(
                    address=address, prior=row.current.info.nonce
                )
            )
        )
        row.current.info.nonce = int(value)


def acct_tx_set_code_hash(address: Any, value: Any) -> None:
    from evm.host.journal import (
        JournalAccountCodeHashChange,
        JournalAccountCodeHashChanged,
    )

    state = get_state()
    row = _account_tx_required(address)
    if row.current.info.code_hash != value:
        state.journal.append(
            JournalAccountCodeHashChanged(
                JournalAccountCodeHashChange(
                    address=address, prior=row.current.info.code_hash
                )
            )
        )
        row.current.info.code_hash = value


def acct_tx_pop() -> Any:
    from evm.primitives.account import (
        AcctEntry,
        AcctTxPopExhausted,
        AcctTxPopRow,
        AcctValue,
    )

    state = get_state()
    if not state.account_tx:
        return AcctTxPopExhausted()
    key, row = state.account_tx.popitem()
    block = state.account_block.get(key)
    address = block.address if block is not None else _bytes20_from_wire(key)
    return AcctTxPopRow(
        AcctEntry(
            addr=address,
            value=AcctValue(curr=row.current, orig=row.original),
        )
    )


def acct_tx_reset() -> None:
    get_state().account_tx.clear()


def acct_block_get(address: Any) -> Any:
    from evm.primitives.account import EMPTY_ACCOUNT, AccountRow

    row = get_state().account_block.get(_address_key(address))
    if row is None:
        return AccountRow(found=False, account=deepcopy(EMPTY_ACCOUNT))
    return AccountRow(found=True, account=row.value.curr)


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
    for storage_key in state.storage_block:
        if storage_key[0] not in candidates:
            raise RuntimeError("storage update candidate lacks a cached account")
    state.account_iterator = sorted(
        candidates.values(), key=lambda row: _hash_key(row.address_hash)
    )


def acct_block_iter_next() -> Any:
    from evm.primitives.account import (
        AcctBlockIterExhausted,
        AcctBlockIterRow,
        AcctEntry,
        AcctTrieEntry,
    )

    state = get_state()
    if not state.account_iterator:
        return AcctBlockIterExhausted()
    row = state.account_iterator.pop(0)
    return AcctBlockIterRow(
        AcctTrieEntry(
            entry=AcctEntry(addr=row.address, value=row.value),
            address_hash=row.address_hash,
        )
    )


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
    changes = get_state().bal_storage_changes.setdefault((key, int(slot)), {})
    changes[int(index)] = int(value)


def bal_storage_read(address: Any, slot: int) -> None:
    key = _bal_touch(address)
    get_state().bal_storage_reads.add((key, int(slot)))


def bal_balance_change(index: int, address: Any, value: int) -> None:
    key = _bal_touch(address)
    get_state().bal_balance_changes.setdefault(key, {})[int(index)] = int(value)


def bal_nonce_change(index: int, address: Any, value: int) -> None:
    key = _bal_touch(address)
    get_state().bal_nonce_changes.setdefault(key, {})[int(index)] = int(value)


def bal_code_change(index: int, address: Any, value: Any) -> None:
    key = _bal_touch(address)
    get_state().bal_code_changes.setdefault(key, {})[int(index)] = value


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
        slot_changes = {
            slot: changes
            for (account, slot), changes in state.bal_storage_changes.items()
            if account == address_key
        }
        for slot in sorted(slot_changes):
            for index in sorted(slot_changes[slot]):
                events.append(
                    BalStorageChange(
                        BalStorageChangeEntry(
                            slot=slot, index=index, value=slot_changes[slot][index]
                        )
                    )
                )
        for account, slot in sorted(state.bal_storage_reads):
            if account == address_key and slot not in slot_changes:
                events.append(BalStorageRead(slot))
        balances = state.bal_balance_changes.get(address_key, {})
        for index in sorted(balances):
            events.append(
                BalBalanceChange(
                    BalBalanceChangeEntry(index=index, value=balances[index])
                )
            )
        nonces = state.bal_nonce_changes.get(address_key, {})
        for index in sorted(nonces):
            events.append(
                BalNonceChange(BalNonceChangeEntry(index=index, value=nonces[index]))
            )
        codes = state.bal_code_changes.get(address_key, {})
        for index in sorted(codes):
            events.append(
                BalCodeChange(
                    BalCodeChangeEntry(index=index, code_hash=codes[index])
                )
            )
        events.append(BalAccountEnd())
    state.bal_iterator = events


def bal_iter_next() -> Any:
    from evm.host.state import BalEmpty

    state = get_state()
    return state.bal_iterator.pop(0) if state.bal_iterator else BalEmpty()


def warm_reset(epoch: int) -> None:
    get_state().current_warm_epoch = int(epoch) + 1


def account_is_warm(address: Any) -> bool:
    state = get_state()
    return state.warm_addresses.get(_address_key(address), 0) >= state.current_warm_epoch


def account_mark_warm(address: Any) -> None:
    from evm.host.journal import JournalWarmAccountChange, JournalWarmAccountChanged

    state = get_state()
    key = _address_key(address)
    stamp = state.warm_addresses.get(key, 0)
    if stamp < state.current_warm_epoch:
        state.journal.append(
            JournalWarmAccountChanged(
                JournalWarmAccountChange(address=address, prior_epoch=stamp)
            )
        )
        state.warm_addresses[key] = state.current_warm_epoch


def storage_is_warm(address: Any, slot: int) -> bool:
    state = get_state()
    return (
        state.warm_slots.get(_address_slot_key(address, slot), 0)
        >= state.current_warm_epoch
    )


def storage_mark_warm(address: Any, slot: int) -> None:
    from evm.host.journal import JournalWarmStorageChange, JournalWarmStorageChanged
    from evm.primitives.account import StorageKey

    state = get_state()
    key = _address_slot_key(address, slot)
    stamp = state.warm_slots.get(key, 0)
    if stamp < state.current_warm_epoch:
        state.journal.append(
            JournalWarmStorageChanged(
                JournalWarmStorageChange(
                    key=StorageKey(addr=address, slot=int(slot)), prior_epoch=stamp
                )
            )
        )
        state.warm_slots[key] = state.current_warm_epoch


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
    from evm.host.journal import JournalLogAppended

    state = get_state()
    state.journal.append(JournalLogAppended())
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
