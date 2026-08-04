"""Small, EVM-specific conveniences for the generated Sail model."""

from __future__ import annotations

import evm


WORD_WIDTH = 256
ADDRESS_BYTE_LENGTH = 20
B256_BYTE_LENGTH = 32


def word(value: int | evm.Bits) -> evm.word:
    """Construct an EVM word, reducing integers modulo 2**256."""

    if isinstance(value, evm.Bits):
        if value.width != WORD_WIDTH:
            raise ValueError(f"word requires {WORD_WIDTH} bits, got {value.width}")
        value = int(value)
    return evm.word(value % (1 << WORD_WIDTH))


def word_to_int(value: evm.word) -> int:
    """Return the unsigned integer represented by an exact-width EVM word."""

    return int(value)


def address(value: int | evm.Bits | bytes | bytearray | list[int | evm.Bits]) -> evm.Bytes20:
    """Construct an EELS-compatible 20-byte EVM address."""

    if isinstance(value, evm.Bytes20):
        return value
    if isinstance(value, (bytes, bytearray, list)):
        return evm.Address(value)
    return evm.word_to_address(word(value))


def b256(value: int | evm.Bits | bytes | bytearray | list[int | evm.Bits]) -> evm.Bytes32:
    """Construct an EELS-compatible 32-byte hash value."""

    if isinstance(value, evm.Bytes32):
        return value
    if isinstance(value, (bytes, bytearray, list)):
        return evm.B256(value)
    bits = value if isinstance(value, evm.Bits) else evm.Bits(WORD_WIDTH, value)
    if bits.width != WORD_WIDTH:
        raise ValueError(f"b256 requires {WORD_WIDTH} bits, got {bits.width}")
    return evm.word_to_hash(word(bits))


def b256_to_int(value: evm.Bytes32) -> int:
    """Return the unsigned integer represented by a 32-byte value."""

    return int(evm.hash_to_word(b256(value)))


__all__ = [
    "ADDRESS_BYTE_LENGTH",
    "B256_BYTE_LENGTH",
    "WORD_WIDTH",
    "address",
    "b256",
    "b256_to_int",
    "word",
    "word_to_int",
]
