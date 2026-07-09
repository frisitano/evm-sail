#!/usr/bin/env python3
"""In-process ctypes harness for the EEST runner + post-run state snapshot decoder.

The runner is built as a shared library (zkvm/native-runner/build_runner_lib.sh ->
libevmsail_runner.dylib) linking test_utils.c, so run.py drives it in-process
instead of forking a subprocess per case. Being in-process is what lets us read
the model's live state AFTER a run: the runner emits NO byte stream at all --
evmsail_dump_snapshot marshals the run RESULT (the shared zkvm_out_gas /
zkvm_out_root registers) plus the write-set accounts+storage, the stack, and the
memory frame depth into a self-describing blob; result() reads just the (gas,
root) pair, decode_snapshot() the whole thing.

Note: the runner is gas-bounded so it terminates, but a warm in-process worker has
no per-case timeout/crash isolation (unlike the old subprocess) -- a pathological
case would take down the whole run. The full corpus is gas-bounded in practice.
"""
import ctypes, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, ".."))
_NR = os.path.join(ROOT, "zkvm", "native-runner")
_EXT = "dylib" if sys.platform == "darwin" else "so"
LIB = os.path.join(_NR, ".build", f"libevmsail_runner.{_EXT}")
GUEST_LIB = os.path.join(_NR, ".build", f"libevmsail_guest.{_EXT}")

_lib = None
_guest = None

def build():
    """(Re)build libevmsail_runner via build_runner_lib.sh; needs `sail` on PATH."""
    print("# building runner lib (one-time)...", file=sys.stderr)
    subprocess.check_call([os.path.join(_NR, "build_runner_lib.sh")])

def _bind(path):
    """dlopen a test_utils.c-shaped lib and bind the shared ctypes signatures."""
    lib = ctypes.CDLL(path)
    P = ctypes.POINTER(ctypes.c_ubyte)
    lib.evmsail_run_once.restype = ctypes.c_ulong
    lib.evmsail_run_once.argtypes = [ctypes.c_char_p, ctypes.c_ulong, ctypes.POINTER(P)]
    lib.evmsail_dump_snapshot.restype = ctypes.c_ulong
    lib.evmsail_dump_snapshot.argtypes = [ctypes.POINTER(P)]
    lib.evmsail_lib_init()
    return lib

def load(rebuild=False):
    """Load (building if needed) the runner lib and bind the ctypes signatures."""
    global _lib
    if _lib is not None and not rebuild:
        return _lib
    if rebuild or not os.path.exists(LIB):
        build()
    _lib = _bind(LIB)
    return _lib

def load_guest(rebuild=False):
    """Load (building if needed) libevmsail_guest -- the stateless guest
    (EVM_ENTRY=guest, main.sail full-block validator) behind the SAME
    test_utils.c harness ABI as the runner lib (build_lib.sh)."""
    global _guest
    if _guest is not None and not rebuild:
        return _guest
    if rebuild or not os.path.exists(GUEST_LIB):
        print("# building guest lib (one-time)...", file=sys.stderr)
        subprocess.check_call([os.path.join(_NR, "build_lib.sh")])
    _guest = _bind(GUEST_LIB)
    return _guest

def _run(lib, inp):
    lib.evmsail_clear_memory()
    outp = ctypes.POINTER(ctypes.c_ubyte)()
    n = lib.evmsail_run_once(inp, len(inp), ctypes.byref(outp))
    return ctypes.string_at(outp, n) if n else b""

def run_once(inp):
    """Wipe state, run one SszStatelessInput through the RUNNER. The runner
    emits no byte stream: read the (gas, root) result via result() and the full
    post-run state via snapshot() (both valid until the next run_once/clear)."""
    return _run(load(), inp)

def run_once_guest(inp):
    """Wipe state, run one SszStatelessInput through the GUEST, return its
    canonical SSZ SszStatelessValidationResult bytes."""
    return _run(load_guest(), inp)

def _snapshot_bytes():
    lib = load()
    sp = ctypes.POINTER(ctypes.c_ubyte)()
    m = lib.evmsail_dump_snapshot(ctypes.byref(sp))
    return ctypes.string_at(sp, m)

# BlockError enum names, in sail/exceptions.sail declaration order (= the
# generated C enum values the dump's err byte carries).
BLOCK_ERRORS = [
    "InvalidConfig", "HeaderChainBroken", "RlpDecode", "InvalidSignature",
    "GasUsedExceedsLimit", "BlobGasLimitExceeded", "ExecutionInvalid",
    "InvalidGasUsed", "InvalidBlobGasUsed", "InvalidExcessBlobGas",
    "InvalidStateRoot", "InvalidReceiptsRoot", "InvalidLogsBloom",
    "InvalidBlockHash", "InvalidParentHash", "BlockAccessListTooLarge",
    "WitnessDeficient",
]

def _parse_result(b, p=0):
    """Parse the 'G' result section at offset p -> ((ok, gas, root, exc), p').
    exc is None on a clean run, else 'InvalidBlock(<BlockError>) @ <throw
    location>' for the exception that escaped the run (the runner catches
    nothing), in which case root is meaningless (zeros)."""
    assert b[p:p + 1] == b"G", "bad snapshot: missing result section"; p += 1
    ok = b[p] == 1; p += 1
    gas, p = _u64(b, p)
    root, p = _w(b, p)
    exc = None
    if not ok:
        err = b[p]; p += 1
        n = int.from_bytes(b[p:p + 2], "big"); p += 2
        loc = b[p:p + n].decode(errors="replace"); p += n
        name = BLOCK_ERRORS[err] if err < len(BLOCK_ERRORS) else f"?{err}"
        exc = f"InvalidBlock({name})" + (f" @ {loc}" if loc else "")
    return (ok, gas, root, exc), p

def result():
    """(ok, gas, root, exc) of the last run, via the snapshot's 'G' section."""
    (r, _) = _parse_result(_snapshot_bytes())
    return r

def snapshot():
    """Decode the live post-run FFI state (valid until the next run_once/clear)."""
    return decode_snapshot(_snapshot_bytes())

# --------------------------- snapshot wire format ---------------------------
def _u32(b, p): return int.from_bytes(b[p:p + 4], "big"), p + 4
def _u64(b, p): return int.from_bytes(b[p:p + 8], "big"), p + 8
def _w(b, p):   return int.from_bytes(b[p:p + 32], "big"), p + 32

def decode_snapshot(b):
    """Blob (see test_utils.c evmsail_dump_snapshot) -> {gas, root, accounts, stack,
    mem_frame_depth}.
    accounts: {acct_hash_int: {nonce, bal, sroot, chash, base_exists, storage:{slot:val}}}
    (write-set union = what execution touched; unchanged witness-base values are not
    enumerable here -- that is what the state root commits to). stack: [word,...] top-first."""
    (ok, gas, root, exc), p = _parse_result(b)
    assert b[p:p + 1] == b"A", "bad snapshot: missing accounts section"; p += 1
    na, p = _u32(b, p)
    accounts = {}
    for _ in range(na):
        hk, p = _w(b, p); nonce, p = _u64(b, p); bal, p = _w(b, p)
        sroot, p = _w(b, p); chash, p = _w(b, p)
        base = b[p]; p += 1
        ns, p = _u32(b, p)
        sto = {}
        for _ in range(ns):
            slot, p = _w(b, p); val, p = _w(b, p); sto[slot] = val
        accounts[hk] = {"nonce": nonce, "bal": bal, "sroot": sroot, "chash": chash,
                        "base_exists": bool(base), "storage": sto}
    assert b[p:p + 1] == b"S", "bad snapshot: missing stack section"; p += 1
    sd, p = _u32(b, p)
    stack = []
    for _ in range(sd):
        w, p = _w(b, p); stack.append(w)
    assert b[p:p + 1] == b"M", "bad snapshot: missing memory section"; p += 1
    md, p = _u32(b, p)
    assert b[p:p + 1] == b"E", "bad snapshot: missing end marker"
    return {"ok": ok, "gas": gas, "root": root, "exc": exc,
            "accounts": accounts, "stack": stack, "mem_frame_depth": md}

def format_snapshot(snap, limit=0):
    """Human-readable summary of a decoded snapshot (for FAIL analysis)."""
    exc = "" if snap["ok"] else f"  [ESCAPED: {snap['exc']}]"
    lines = [f"gas={snap['gas']} root={snap['root']:#066x}{exc}",
             f"accounts (write-set): {len(snap['accounts'])}"]
    for i, (hk, a) in enumerate(snap["accounts"].items()):
        if limit and i >= limit:
            lines.append(f"  ... (+{len(snap['accounts']) - limit} more)"); break
        lines.append(f"  {hk:#066x} nonce={a['nonce']} bal={a['bal']}"
                     f"{' [new]' if not a['base_exists'] else ''}")
        for slot, val in a["storage"].items():
            lines.append(f"      [{slot:#x}] = {val:#x}")
    if snap["stack"]:
        lines.append(f"stack depth={len(snap['stack'])}: " +
                     " ".join(hex(w) for w in snap["stack"][:8]))
    lines.append(f"mem frame depth={snap['mem_frame_depth']}")
    return "\n".join(lines)
