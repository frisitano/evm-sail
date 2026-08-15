#!/usr/bin/env python3
"""In-process ctypes harness for main.sail and its native-only debug dump.

The stateless guest is built once as a shared library and driven in-process.
Normal runs return only main.sail's canonical SSZ validation result. On demand,
guest_debug_dump serializes the live post-run host state for failure analysis;
that utility is never linked into the real RISC-V guest.

Note: execution is gas-bounded. The optimized native backend converts its
fail-closed host invariant failures into per-case Python exceptions at a
test-only thread boundary; faults outside that boundary can still terminate a
warm in-process worker.
"""

import contextlib
import ctypes
import os
import resource
import subprocess
import sys

from devtools.paths import REPO_ROOT

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = str(REPO_ROOT)
_NR = os.path.join(ROOT, "zkvm", "native-runner")
_LEAN_RUNNER = os.path.join(ROOT, "extractions", "lean", "runner")
_EXT = "dylib" if sys.platform == "darwin" else "so"

_guest = None
_guest_profile = None
_guest_build_mode = None


class GuestExecutionError(RuntimeError):
    """A native guest run stopped at its test-only failure boundary."""


def _build_paths(build_mode):
    if build_mode not in ("standard", "optimized"):
        raise ValueError("build_mode must be 'standard' or 'optimized'")
    build_dir = os.path.join(_NR, f".build-{build_mode}")
    return (
        build_dir,
        os.path.join(build_dir, f"libevmsail_guest.{_EXT}"),
        os.path.join(build_dir, "libevmsail_guest.profile"),
    )


def _bind(path):
    """dlopen a test_utils.c-shaped lib and bind the shared ctypes signatures."""
    try:
        lib = ctypes.CDLL(path)
    except OSError as error:
        raise GuestExecutionError(f"could not load native guest library {path}: {error}") from error
    P = ctypes.POINTER(ctypes.c_ubyte)
    lib.guest_init.restype = None
    lib.guest_init.argtypes = []
    lib.guest_fini.restype = None
    lib.guest_fini.argtypes = []
    lib.guest_reset.restype = None
    lib.guest_reset.argtypes = []
    lib.guest_run.restype = ctypes.c_ulong
    lib.guest_run.argtypes = [ctypes.c_char_p, ctypes.c_ulong, ctypes.POINTER(P)]
    lib.guest_debug_dump.restype = ctypes.c_ulong
    lib.guest_debug_dump.argtypes = [ctypes.POINTER(P)]
    # Lean's independently implemented test ABI does not expose the optimized
    # C failure boundary. Keep that backend loadable while requiring all C
    # native builds to use the common accessor.
    try:
        lib.guest_last_error.restype = ctypes.c_char_p
        lib.guest_last_error.argtypes = []
    except AttributeError:
        pass
    lib.guest_init()
    return lib


def _last_error(lib):
    accessor = getattr(lib, "guest_last_error", None)
    if accessor is None:
        return ""
    message = accessor()
    return message.decode(errors="replace") if message else ""


def load_guest(rebuild=False, profile=False, build_mode="optimized"):
    """Load main.sail's native shared library, building it if needed."""
    global _guest, _guest_profile, _guest_build_mode
    build_dir, guest_lib, profile_marker = _build_paths(build_mode)
    wanted = "on" if profile else "off"
    if _guest is not None:
        if _guest_profile != wanted or _guest_build_mode != build_mode:
            raise RuntimeError(
                "cannot switch EVM_PROFILE or build mode after loading the guest library"
            )
        return _guest
    try:
        with open(profile_marker) as f:
            built_profile = f.read().strip()
    except FileNotFoundError:
        built_profile = None
    if rebuild or not os.path.exists(guest_lib) or built_profile != wanted:
        print(f"# building {build_mode} guest lib (one-time)...", file=sys.stderr)
        env = dict(
            os.environ,
            EVM_BUILD_MODE=build_mode,
            EVM_PROFILE=wanted,
            NATIVE_BUILD=build_dir,
        )
        if build_mode == "standard":
            env["EXTRA_PRESERVE"] = "debug_account_storage_root"
        subprocess.check_call([os.path.join(_NR, "build_lib.sh")], env=env)
        with open(profile_marker, "w") as f:
            f.write(wanted + "\n")
    _guest = _bind(guest_lib)
    _guest_profile = wanted
    _guest_build_mode = build_mode
    return _guest


def load_lean_guest(rebuild=False):
    """Load the executable Lean extraction through the shared harness ABI."""
    global _guest, _guest_profile, _guest_build_mode
    soft_stack, hard_stack = resource.getrlimit(resource.RLIMIT_STACK)
    wanted_stack = 64 * 1024 * 1024
    if hard_stack != resource.RLIM_INFINITY:
        wanted_stack = min(wanted_stack, hard_stack)
    if soft_stack != resource.RLIM_INFINITY and soft_stack < wanted_stack:
        with contextlib.suppress(OSError, ValueError):
            resource.setrlimit(resource.RLIMIT_STACK, (wanted_stack, hard_stack))
    guest_lib = os.path.join(
        _LEAN_RUNNER,
        ".lake",
        "build",
        "lib",
        f"libevmsail_lean_guest.{_EXT}",
    )
    if _guest is not None:
        if _guest_build_mode != "lean":
            raise RuntimeError("cannot switch guest backend after loading a guest library")
        return _guest
    if rebuild or not os.path.exists(guest_lib):
        print("# building Lean guest lib (one-time)...", file=sys.stderr)
        subprocess.check_call([os.path.join(_LEAN_RUNNER, "build_lib.sh")])
    _guest = _bind(guest_lib)
    _guest_profile = "off"
    _guest_build_mode = "lean"
    return _guest


def _run(lib, inp):
    outp = ctypes.POINTER(ctypes.c_ubyte)()
    try:
        lib.guest_reset()
        n = lib.guest_run(inp, len(inp), ctypes.byref(outp))
    except (OSError, ctypes.ArgumentError) as error:
        raise GuestExecutionError(f"native guest call failed: {error}") from error
    host_error = _last_error(lib)
    if host_error:
        raise GuestExecutionError(host_error)
    if n and not outp:
        raise GuestExecutionError(f"native guest returned {n} output bytes through a null pointer")
    return ctypes.string_at(outp, n) if n else b""


def run_once_guest(inp):
    """Wipe state, run one SszStatelessInput through main.sail, returning its
    canonical SSZ SszStatelessValidationResult bytes."""
    lib = _guest if _guest is not None else load_guest()
    return _run(lib, inp)


def _debug_dump_bytes():
    lib = _guest if _guest is not None else load_guest()
    sp = ctypes.POINTER(ctypes.c_ubyte)()
    m = lib.guest_debug_dump(ctypes.byref(sp))
    return ctypes.string_at(sp, m)


# BlockError enum names, in sail/exceptions.sail declaration order (= the
# generated C enum values the dump's err byte carries).
BLOCK_ERRORS = [
    "InvalidConfig",
    "HeaderChainBroken",
    "RlpDecode",
    "InvalidSignature",
    "InvalidGasLimit",
    "GasUsedExceedsLimit",
    "BlobGasLimitExceeded",
    "ExecutionInvalid",
    "InvalidGasUsed",
    "InvalidBlobGasUsed",
    "InvalidExcessBlobGas",
    "InvalidStateRoot",
    "InvalidReceiptsRoot",
    "InvalidLogsBloom",
    "InvalidBlockHash",
    "InvalidParentHash",
    "BlockAccessListTooLarge",
    "InvalidBlockAccessList",
    "InvalidExecutionRequests",
    "WitnessDeficient",
]

VALIDATION_SCOPES = [
    "stateless-validation",
    "decode-input",
    "index-witness",
    "validate-payload",
    "execute-block",
    "validate-result",
    "compute-output-root",
    "serialize-output",
]


def _parse_result(b, p=0):
    """Parse the debug dump's 'G' section -> ((ok, root, exc), p').
    exc is None on a clean run, else 'InvalidBlock(<BlockError>) @ <throw
    location>'; in that case root is meaningless (zeros)."""
    assert b[p : p + 1] == b"G", "bad snapshot: missing result section"
    p += 1
    ok = b[p] == 1
    p += 1
    root, p = _w(b, p)
    exc = None
    if not ok:
        err = b[p]
        p += 1
        n = int.from_bytes(b[p : p + 2], "big")
        p += 2
        loc = b[p : p + n].decode(errors="replace")
        p += n
        name = BLOCK_ERRORS[err] if err < len(BLOCK_ERRORS) else f"?{err}"
        exc = f"InvalidBlock({name})" + (f" @ {loc}" if loc else "")
    return (ok, root, exc), p


def snapshot():
    """Decode an on-demand dump of the live post-run native state."""
    return decode_snapshot(_debug_dump_bytes())


# --------------------------- snapshot wire format ---------------------------
def _u32(b, p):
    return int.from_bytes(b[p : p + 4], "big"), p + 4


def _u64(b, p):
    return int.from_bytes(b[p : p + 8], "big"), p + 8


def _w(b, p):
    return int.from_bytes(b[p : p + 32], "big"), p + 32


def decode_snapshot(b):
    """Blob (see test_utils.c guest_debug_dump) -> native debug state.
    accounts: {acct_hash_int: {address, nonce, bal, sroot, computed_sroot,
                               chash, storage:{slot:val}}}
    (materialized state = what execution touched; unchanged witness-base values are not
    enumerable here -- that is what the state root commits to). stack: [word,...] top-first."""
    (ok, root, exc), p = _parse_result(b)
    assert b[p : p + 1] == b"O", "bad snapshot: missing output section"
    p += 1
    output_len, p = _u32(b, p)
    output = b[p : p + output_len]
    p += output_len
    assert b[p : p + 1] == b"V", "bad snapshot: missing validation section"
    p += 1
    validation_failure = None
    if b[p] == 1:
        p += 1
        scope, reason = b[p], b[p + 1]
        p += 2
        location_len = int.from_bytes(b[p : p + 2], "big")
        p += 2
        location = b[p : p + location_len].decode("utf-8", errors="replace")
        p += location_len
        scope_name = VALIDATION_SCOPES[scope] if scope < len(VALIDATION_SCOPES) else f"?{scope}"
        reason_name = BLOCK_ERRORS[reason] if reason < len(BLOCK_ERRORS) else f"?{reason}"
        validation_failure = {
            "scope": scope_name,
            "reason": reason_name,
            "location": location,
        }
    else:
        p += 1
    assert b[p : p + 1] == b"B", "bad snapshot: missing block gas section"
    p += 1
    block_gas = None
    if b[p] == 1:
        p += 1
        actual, p = _u64(b, p)
        expected, p = _u64(b, p)
        execution, p = _u64(b, p)
        state, p = _u64(b, p)
        block_gas = {
            "actual": actual,
            "expected": expected,
            "execution": execution,
            "state": state,
        }
    else:
        p += 1
    assert b[p : p + 1] == b"A", "bad snapshot: missing accounts section"
    p += 1
    na, p = _u32(b, p)
    accounts = {}
    for _ in range(na):
        hk, p = _w(b, p)
        address = int.from_bytes(b[p : p + 20], "big")
        p += 20
        nonce, p = _u64(b, p)
        bal, p = _w(b, p)
        sroot, p = _w(b, p)
        computed_sroot, p = _w(b, p)
        chash, p = _w(b, p)
        ns, p = _u32(b, p)
        sto = {}
        for _ in range(ns):
            slot, p = _w(b, p)
            val, p = _w(b, p)
            sto[slot] = val
        accounts[hk] = {
            "address": address,
            "nonce": nonce,
            "bal": bal,
            "sroot": sroot,
            "computed_sroot": computed_sroot,
            "chash": chash,
            "storage": sto,
        }
    assert b[p : p + 1] == b"S", "bad snapshot: missing stack section"
    p += 1
    sd, p = _u32(b, p)
    stack = []
    for _ in range(sd):
        w, p = _w(b, p)
        stack.append(w)
    assert b[p : p + 1] == b"M", "bad snapshot: missing memory section"
    p += 1
    md, p = _u32(b, p)
    assert b[p : p + 1] == b"E", "bad snapshot: missing end marker"
    return {
        "ok": ok,
        "root": root,
        "exc": exc,
        "output": output,
        "validation_failure": validation_failure,
        "block_gas": block_gas,
        "accounts": accounts,
        "stack": stack,
        "mem_frame_depth": md,
    }


def format_snapshot(snap, limit=0):
    """Human-readable summary of a decoded snapshot (for FAIL analysis)."""
    valid = snap["output"][32] if len(snap["output"]) > 32 else None
    rejected = snap["validation_failure"]
    exc = "" if snap["ok"] or rejected is not None else f"  [ESCAPED: {snap['exc']}]"
    failure = (
        ""
        if rejected is None
        else (
            f"  [REJECTED: {rejected['reason']} during {rejected['scope']}"
            f" at {rejected['location']}]"
        )
    )
    lines = [
        f"state_root={snap['root']:#066x} validation={valid}{failure}{exc}",
        f"accounts (materialized): {len(snap['accounts'])}",
    ]
    if snap["block_gas"] is not None:
        gas = snap["block_gas"]
        lines.insert(
            1,
            "block gas: "
            f"actual={gas['actual']} expected={gas['expected']} "
            f"execution={gas['execution']} state={gas['state']}",
        )
    for i, (hk, a) in enumerate(snap["accounts"].items()):
        if limit and i >= limit:
            lines.append(f"  ... (+{len(snap['accounts']) - limit} more)")
            break
        lines.append(
            f"  {hk:#066x} address={a['address']:#042x} "
            f"nonce={a['nonce']} bal={a['bal']} "
            f"base_sroot={a['sroot']:#066x} "
            f"computed_sroot={a['computed_sroot']:#066x} "
            f"chash={a['chash']:#066x}"
        )
        for slot, val in a["storage"].items():
            lines.append(f"      [{slot:#x}] = {val:#x}")
    if snap["stack"]:
        lines.append(
            f"stack depth={len(snap['stack'])}: " + " ".join(hex(w) for w in snap["stack"][:8])
        )
    lines.append(f"mem frame depth={snap['mem_frame_depth']}")
    return "\n".join(lines)
