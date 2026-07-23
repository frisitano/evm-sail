#!/usr/bin/env python3
"""Differential tests for the bounded, GMP-free Sail integer runtime."""

from __future__ import annotations

import ctypes
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile


LIMBS = 12
WIDTH = LIMBS * 64
MASK64 = (1 << 64) - 1
RANDOM = random.Random(0xE7A5A11)
HERE = Path(__file__).resolve().parent


class SailInt(ctypes.Structure):
    _fields_ = [
        ("negative", ctypes.c_uint8),
        ("length", ctypes.c_uint8),
        ("padding", ctypes.c_uint8 * 6),
        ("limbs", ctypes.c_uint64 * LIMBS),
    ]


INT_PTR = ctypes.POINTER(SailInt)


def encode(value: int) -> SailInt:
    magnitude = abs(value)
    assert magnitude < 1 << WIDTH
    result = SailInt()
    result.negative = value < 0
    for index in range(LIMBS):
        result.limbs[index] = magnitude & MASK64
        magnitude >>= 64
        if result.limbs[index] != 0:
            result.length = index + 1
    return result


def decode(value: SailInt) -> int:
    assert value.length <= LIMBS
    magnitude = sum(
        int(value.limbs[index]) << (64 * index)
        for index in range(value.length)
    )
    assert value.length == 0 or value.limbs[value.length - 1] != 0
    assert value.length != 0 or not value.negative
    return -magnitude if value.negative else magnitude


def signed_random(max_bits: int = 765) -> int:
    value = RANDOM.getrandbits(RANDOM.randrange(max_bits + 1))
    return -value if RANDOM.choice((False, True)) else value


def trunc_div(lhs: int, rhs: int) -> int:
    quotient = abs(lhs) // abs(rhs)
    return -quotient if (lhs < 0) != (rhs < 0) else quotient


def build_runtime(output: Path) -> None:
    compiler = os.environ.get("CC", "cc")
    command = [
        compiler,
        "-std=c11",
        "-O2",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-fPIC",
        "-shared",
        f"-I{HERE}",
        str(HERE / "sail.c"),
        "-o",
        str(output),
    ]
    subprocess.run(command, check=True)


def load_runtime(path: Path) -> ctypes.CDLL:
    runtime = ctypes.CDLL(str(path))
    for name in (
        "add_int",
        "sub_int",
        "sub_nat",
        "mult_int",
        "tdiv_int",
        "tmod_int",
        "ediv_int",
        "emod_int",
        "fdiv_int",
        "fmod_int",
        "max_int",
        "min_int",
        "shl_int",
        "shr_int",
        "pow_int",
    ):
        function = getattr(runtime, name)
        function.argtypes = [INT_PTR, INT_PTR, INT_PTR]
        function.restype = None
    runtime.pow2.argtypes = [INT_PTR, INT_PTR]
    runtime.pow2.restype = None
    runtime.create_sail_int_of_sail_string.argtypes = [INT_PTR, ctypes.c_char_p]
    runtime.create_sail_int_of_sail_string.restype = None
    for name in ("eq_int", "lt", "gt", "lteq", "gteq"):
        function = getattr(runtime, name)
        function.argtypes = [INT_PTR, INT_PTR]
        function.restype = ctypes.c_bool
    runtime.convert_mach_uint_of_sail_int.argtypes = [INT_PTR]
    runtime.convert_mach_uint_of_sail_int.restype = ctypes.c_uint64
    runtime.convert_sail_int_of_mach_uint.argtypes = [INT_PTR, ctypes.c_uint64]
    runtime.convert_sail_int_of_mach_uint.restype = None
    return runtime


def call_binary(runtime: ctypes.CDLL, name: str, lhs: int, rhs: int) -> int:
    left = encode(lhs)
    right = encode(rhs)
    result = encode(0)
    getattr(runtime, name)(
        ctypes.byref(result), ctypes.byref(left), ctypes.byref(right)
    )
    return decode(result)


def check_arithmetic(runtime: ctypes.CDLL) -> None:
    edge_values = [
        0,
        1,
        -1,
        (1 << 63) - 1,
        -(1 << 63),
        (1 << 255) - 1,
        -(1 << 255),
        (1 << 764) - 1,
        -((1 << 764) - 1),
    ]
    pairs = [(lhs, rhs) for lhs in edge_values for rhs in edge_values]
    pairs.extend((signed_random(763), signed_random(763)) for _ in range(500))
    for lhs, rhs in pairs:
        if abs(lhs + rhs) < 1 << WIDTH:
            assert call_binary(runtime, "add_int", lhs, rhs) == lhs + rhs
        if abs(lhs - rhs) < 1 << WIDTH:
            assert call_binary(runtime, "sub_int", lhs, rhs) == lhs - rhs
        assert call_binary(runtime, "max_int", lhs, rhs) == max(lhs, rhs)
        assert call_binary(runtime, "min_int", lhs, rhs) == min(lhs, rhs)
        left = encode(lhs)
        right = encode(rhs)
        assert runtime.eq_int(ctypes.byref(left), ctypes.byref(right)) == (lhs == rhs)
        assert runtime.lt(ctypes.byref(left), ctypes.byref(right)) == (lhs < rhs)
        assert runtime.gt(ctypes.byref(left), ctypes.byref(right)) == (lhs > rhs)
        assert runtime.lteq(ctypes.byref(left), ctypes.byref(right)) == (lhs <= rhs)
        assert runtime.gteq(ctypes.byref(left), ctypes.byref(right)) == (lhs >= rhs)

    for _ in range(500):
        lhs_bits = RANDOM.randrange(383)
        rhs_bits = RANDOM.randrange(383)
        lhs = signed_random(lhs_bits)
        rhs = signed_random(rhs_bits)
        assert call_binary(runtime, "mult_int", lhs, rhs) == lhs * rhs

    for _ in range(200):
        lhs = RANDOM.getrandbits(RANDOM.randrange(766))
        rhs = RANDOM.getrandbits(RANDOM.randrange(766))
        assert call_binary(runtime, "sub_nat", lhs, rhs) == max(lhs - rhs, 0)


def check_division(runtime: ctypes.CDLL) -> None:
    pairs = [
        (5, 2),
        (-5, 2),
        (5, -2),
        (-5, -2),
        (1 << 764, (1 << 255) + 17),
        (-(1 << 764), (1 << 255) + 17),
    ]
    for _ in range(350):
        lhs = signed_random()
        rhs = 0
        while rhs == 0:
            rhs = signed_random()
        pairs.append((lhs, rhs))

    for lhs, rhs in pairs:
        truncated = trunc_div(lhs, rhs)
        assert call_binary(runtime, "tdiv_int", lhs, rhs) == truncated
        assert call_binary(runtime, "tmod_int", lhs, rhs) == lhs - truncated * rhs

        euclidean_remainder = lhs % abs(rhs)
        euclidean_quotient = (lhs - euclidean_remainder) // rhs
        assert call_binary(runtime, "ediv_int", lhs, rhs) == euclidean_quotient
        assert call_binary(runtime, "emod_int", lhs, rhs) == euclidean_remainder

        floor_quotient = lhs // rhs
        assert call_binary(runtime, "fdiv_int", lhs, rhs) == floor_quotient
        assert call_binary(runtime, "fmod_int", lhs, rhs) == lhs - floor_quotient * rhs


def check_shifts_and_powers(runtime: ctypes.CDLL) -> None:
    for _ in range(400):
        value = signed_random(700)
        shift = RANDOM.randrange(769)
        if abs(value << shift) < 1 << WIDTH:
            assert call_binary(runtime, "shl_int", value, shift) == value << shift
        assert call_binary(runtime, "shr_int", value, shift) == value >> shift

    for exponent in range(768):
        result = encode(0)
        power = encode(exponent)
        runtime.pow2(ctypes.byref(result), ctypes.byref(power))
        assert decode(result) == 1 << exponent

    for base in range(-12, 13):
        for exponent in range(13):
            result = call_binary(runtime, "pow_int", base, exponent)
            assert result == base**exponent


def check_aliasing(runtime: ctypes.CDLL) -> None:
    cases = [
        ("add_int", (1 << 511) + 9, -(1 << 255) + 3),
        ("sub_int", -(1 << 511) + 9, (1 << 255) + 3),
        ("mult_int", (1 << 320) + 9, -(1 << 255) + 3),
        ("tdiv_int", -(1 << 700) + 9, (1 << 255) + 3),
        ("tmod_int", -(1 << 700) + 9, (1 << 255) + 3),
    ]
    for name, lhs, rhs in cases:
        expected = call_binary(runtime, name, lhs, rhs)
        left = encode(lhs)
        right = encode(rhs)
        getattr(runtime, name)(
            ctypes.byref(left), ctypes.byref(left), ctypes.byref(right)
        )
        assert decode(left) == expected
        left = encode(lhs)
        right = encode(rhs)
        getattr(runtime, name)(
            ctypes.byref(right), ctypes.byref(left), ctypes.byref(right)
        )
        assert decode(right) == expected


def check_strings_and_bitvectors(runtime: ctypes.CDLL) -> None:
    values = [0, 1, -1, (1 << 255) - 1, -(1 << 700) + 17]
    for value in values:
        result = encode(0)
        runtime.create_sail_int_of_sail_string(
            ctypes.byref(result), str(value).encode()
        )
        assert decode(result) == value

    for value in (0, 1, (1 << 63) - 1, 1 << 63, (1 << 64) - 1):
        encoded = encode(value)
        assert runtime.convert_mach_uint_of_sail_int(ctypes.byref(encoded)) == value
        result = encode(0)
        runtime.convert_sail_int_of_mach_uint(ctypes.byref(result), value)
        assert decode(result) == value

def main() -> None:
    suffix = ".dylib" if sys.platform == "darwin" else ".so"
    with tempfile.TemporaryDirectory(prefix="evmsail-sail256-") as directory:
        library = Path(directory) / f"runtime{suffix}"
        build_runtime(library)
        runtime = load_runtime(library)
        check_arithmetic(runtime)
        check_division(runtime)
        check_shifts_and_powers(runtime)
        check_aliasing(runtime)
        check_strings_and_bitvectors(runtime)
    print("sail256 runtime: differential tests passed")


if __name__ == "__main__":
    main()
