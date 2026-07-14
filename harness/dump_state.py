#!/usr/bin/env python3
"""In-process ctypes harness for main.sail and its native-only debug dump.

The stateless guest is built once as a shared library and driven in-process.
Normal runs return only main.sail's canonical SSZ validation result. On demand,
evmsail_debug_dump serializes the live post-run host state for failure analysis;
that utility is never linked into the real RISC-V guest.

Note: execution is gas-bounded, but a warm in-process worker has
no per-case timeout/crash isolation (unlike the old subprocess) -- a pathological
case would take down the whole run. The full corpus is gas-bounded in practice.
"""
import ctypes, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, ".."))
_NR = os.path.join(ROOT, "zkvm", "native-runner")
_EXT = "dylib" if sys.platform == "darwin" else "so"
GUEST_LIB = os.path.join(_NR, ".build", f"libevmsail_guest.{_EXT}")
PROFILE_MARKER = os.path.join(_NR, ".build", "libevmsail_guest.profile")

_guest = None
_guest_profile = None

def _bind(path):
    """dlopen a test_utils.c-shaped lib and bind the shared ctypes signatures."""
    lib = ctypes.CDLL(path)
    P = ctypes.POINTER(ctypes.c_ubyte)
    lib.evmsail_run_once.restype = ctypes.c_ulong
    lib.evmsail_run_once.argtypes = [ctypes.c_char_p, ctypes.c_ulong, ctypes.POINTER(P)]
    lib.evmsail_debug_dump.restype = ctypes.c_ulong
    lib.evmsail_debug_dump.argtypes = [ctypes.POINTER(P)]
    lib.evmsail_lib_init()
    return lib

def load_guest(rebuild=False, profile=False):
    """Load main.sail's native shared library, building it if needed."""
    global _guest, _guest_profile
    wanted = "on" if profile else "off"
    if _guest is not None:
        if _guest_profile != wanted:
            raise RuntimeError("cannot switch EVM_PROFILE after loading the guest library")
        return _guest
    try:
        with open(PROFILE_MARKER) as f:
            built_profile = f.read().strip()
    except FileNotFoundError:
        built_profile = None
    if rebuild or not os.path.exists(GUEST_LIB) or built_profile != wanted:
        print("# building guest lib (one-time)...", file=sys.stderr)
        env = dict(os.environ, EVM_PROFILE=wanted)
        subprocess.check_call([os.path.join(_NR, "build_lib.sh")], env=env)
        with open(PROFILE_MARKER, "w") as f:
            f.write(wanted + "\n")
    _guest = _bind(GUEST_LIB)
    _guest_profile = wanted
    return _guest

def _run(lib, inp):
    lib.evmsail_clear_memory()
    outp = ctypes.POINTER(ctypes.c_ubyte)()
    n = lib.evmsail_run_once(inp, len(inp), ctypes.byref(outp))
    return ctypes.string_at(outp, n) if n else b""

def run_once_guest(inp):
    """Wipe state, run one SszStatelessInput through main.sail, returning its
    canonical SSZ SszStatelessValidationResult bytes."""
    return _run(_guest if _guest is not None else load_guest(), inp)

def _debug_dump_bytes():
    lib = _guest if _guest is not None else load_guest()
    sp = ctypes.POINTER(ctypes.c_ubyte)()
    m = lib.evmsail_debug_dump(ctypes.byref(sp))
    return ctypes.string_at(sp, m)

# BlockError enum names, in sail/exceptions.sail declaration order (= the
# generated C enum values the dump's err byte carries).
BLOCK_ERRORS = [
    "InvalidConfig", "HeaderChainBroken", "RlpDecode", "InvalidSignature",
    "GasUsedExceedsLimit", "BlobGasLimitExceeded", "ExecutionInvalid",
    "InvalidGasUsed", "InvalidBlobGasUsed", "InvalidExcessBlobGas",
    "InvalidStateRoot", "InvalidReceiptsRoot", "InvalidLogsBloom",
    "InvalidBlockHash", "InvalidParentHash", "BlockAccessListTooLarge",
    "InvalidBlockAccessList",
    "InvalidExecutionRequests",
    "WitnessDeficient",
]

VALIDATION_SCOPES = [
    "stateless-validation", "decode-input", "index-witness",
    "validate-payload", "execute-block", "validate-result",
    "compute-output-root", "serialize-output",
]

def _parse_result(b, p=0):
    """Parse the debug dump's 'G' section -> ((ok, root, exc), p').
    exc is None on a clean run, else 'InvalidBlock(<BlockError>) @ <throw
    location>'; in that case root is meaningless (zeros)."""
    assert b[p:p + 1] == b"G", "bad snapshot: missing result section"; p += 1
    ok = b[p] == 1; p += 1
    root, p = _w(b, p)
    exc = None
    if not ok:
        err = b[p]; p += 1
        n = int.from_bytes(b[p:p + 2], "big"); p += 2
        loc = b[p:p + n].decode(errors="replace"); p += n
        name = BLOCK_ERRORS[err] if err < len(BLOCK_ERRORS) else f"?{err}"
        exc = f"InvalidBlock({name})" + (f" @ {loc}" if loc else "")
    return (ok, root, exc), p

def snapshot():
    """Decode an on-demand dump of the live post-run native state."""
    return decode_snapshot(_debug_dump_bytes())

# --------------------------- snapshot wire format ---------------------------
def _u32(b, p): return int.from_bytes(b[p:p + 4], "big"), p + 4
def _u64(b, p): return int.from_bytes(b[p:p + 8], "big"), p + 8
def _w(b, p):   return int.from_bytes(b[p:p + 32], "big"), p + 32

def decode_snapshot(b):
    """Blob (see test_utils.c evmsail_debug_dump) -> native debug state.
    accounts: {acct_hash_int: {nonce, bal, sroot, chash, storage:{slot:val}}}
    (materialized state = what execution touched; unchanged witness-base values are not
    enumerable here -- that is what the state root commits to). stack: [word,...] top-first."""
    (ok, root, exc), p = _parse_result(b)
    assert b[p:p + 1] == b"O", "bad snapshot: missing output section"; p += 1
    output_len, p = _u32(b, p)
    output = b[p:p + output_len]; p += output_len
    assert b[p:p + 1] == b"V", "bad snapshot: missing validation section"; p += 1
    validation_failure = None
    if b[p] == 1:
        p += 1
        scope, reason = b[p], b[p + 1]; p += 2
        scope_name = VALIDATION_SCOPES[scope] if scope < len(VALIDATION_SCOPES) else f"?{scope}"
        reason_name = BLOCK_ERRORS[reason] if reason < len(BLOCK_ERRORS) else f"?{reason}"
        validation_failure = {"scope": scope_name, "reason": reason_name}
    else:
        p += 1
    assert b[p:p + 1] == b"A", "bad snapshot: missing accounts section"; p += 1
    na, p = _u32(b, p)
    accounts = {}
    for _ in range(na):
        hk, p = _w(b, p); nonce, p = _u64(b, p); bal, p = _w(b, p)
        sroot, p = _w(b, p); chash, p = _w(b, p)
        ns, p = _u32(b, p)
        sto = {}
        for _ in range(ns):
            slot, p = _w(b, p); val, p = _w(b, p); sto[slot] = val
        accounts[hk] = {"nonce": nonce, "bal": bal, "sroot": sroot, "chash": chash,
                        "storage": sto}
    assert b[p:p + 1] == b"S", "bad snapshot: missing stack section"; p += 1
    sd, p = _u32(b, p)
    stack = []
    for _ in range(sd):
        w, p = _w(b, p); stack.append(w)
    assert b[p:p + 1] == b"M", "bad snapshot: missing memory section"; p += 1
    md, p = _u32(b, p)
    assert b[p:p + 1] == b"E", "bad snapshot: missing end marker"
    return {"ok": ok, "root": root, "exc": exc, "output": output,
            "validation_failure": validation_failure,
            "accounts": accounts, "stack": stack, "mem_frame_depth": md}

def format_snapshot(snap, limit=0):
    """Human-readable summary of a decoded snapshot (for FAIL analysis)."""
    exc = "" if snap["ok"] else f"  [ESCAPED: {snap['exc']}]"
    valid = snap["output"][32] if len(snap["output"]) > 32 else None
    rejected = snap["validation_failure"]
    failure = "" if rejected is None else (
        f"  [REJECTED: {rejected['reason']} during {rejected['scope']}]"
    )
    lines = [f"state_root={snap['root']:#066x} validation={valid}{failure}{exc}",
             f"accounts (materialized): {len(snap['accounts'])}"]
    for i, (hk, a) in enumerate(snap["accounts"].items()):
        if limit and i >= limit:
            lines.append(f"  ... (+{len(snap['accounts']) - limit} more)"); break
        lines.append(f"  {hk:#066x} nonce={a['nonce']} bal={a['bal']}")
        for slot, val in a["storage"].items():
            lines.append(f"      [{slot:#x}] = {val:#x}")
    if snap["stack"]:
        lines.append(f"stack depth={len(snap['stack'])}: " +
                     " ".join(hex(w) for w in snap["stack"][:8]))
    lines.append(f"mem frame depth={snap['mem_frame_depth']}")
    return "\n".join(lines)
