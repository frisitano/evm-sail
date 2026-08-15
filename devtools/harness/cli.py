#!/usr/bin/env python3
"""Run EEST fixtures through evm-sail's single main.sail entry.

Architecture: the Sail model is built ONCE into a shared library and driven
IN-PROCESS via ctypes (devtools.harness.guest). Each fixture case is serialized
to the SSZ SszStatelessInput (devtools.harness.ssz_builder, under the
execution-specs venv) and fed
to main.sail's full-block validator, gated BYTE-EXACT against the EELS
reference. Two input
sources, auto-detected per fixture file:
  - state tests: ssz_builder executes the case through the
    in-process EELS t8n, producing a fully VALID single-tx block input AND the
    reference guest's expected SszStatelessValidationResult bytes;
  - blockchain/stateless fixtures already carrying statelessInputBytes /
    statelessOutputBytes (e.g. corpora under zkvm/.fixtures/): fed directly.
Four execution vehicles: the native in-process ctypes lib (default), the
generated Python extraction (--python), the REAL RISC-V guest ELF on Spike
(--spike), or the production ZisK ELF on ziskemu (--zisk). Both ELF vehicles
build once and then provide each case through the runtime read_input ABI to the
unchanged ELF. This subsumes the old
zkvm/native-runner/run_fixtures*.py and zkvm/run_guest_smoke.py runners.

Usage:
    python3 -m devtools.harness.cli <test.json|dir> [...] [--fork F] [--limit N]
            [--build standard|optimized] [--python|--spike|--zisk] [--rebuild]
            [--verbose] [--debug]
Requires `sail` on PATH and a C compiler; devtools.harness.ssz_builder runs under the
execution-specs venv (EXECSPECS_PY).
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import traceback
from collections.abc import Callable
from dataclasses import dataclass
from typing import Protocol

from devtools.harness import guest as dump_state
from devtools.paths import REPO_ROOT, temporary_root

HERE = os.path.dirname(os.path.abspath(__file__))
ELDIR = str(REPO_ROOT)
TMP_ROOT = str(temporary_root())


class GuestBackend(Protocol):
    """Execution seam shared by native, extracted, and zkVM guests."""

    def run_once(self, inp: bytes) -> bytes: ...


@dataclass(frozen=True)
class FunctionGuest:
    """Adapt an already-loaded native or Lean guest function to the seam."""

    runner: Callable[[bytes], bytes]

    def run_once(self, inp: bytes) -> bytes:
        return self.runner(inp)


def h2i(x):
    if isinstance(x, int):
        return x
    x = str(x)
    return int(x, 16) if x.startswith("0x") else int(x)


def ai(a):
    return int(a.lower().replace("0x", ""), 16) if a else 0


def fixture_items(path):
    """Yield fixtures from regular JSON or EEST worker-shard JSONL."""
    if path.endswith(".jsonl"):
        with open(path) as f:
            for line in f:
                if not line.strip():
                    continue
                row = json.loads(line)
                value = row["v"]
                yield row["k"], json.loads(value) if isinstance(value, str) else value
    else:
        with open(path) as fixture_file:
            yield from json.load(fixture_file).items()


def collect(path, want_fork, limit):
    out = []
    for name, t in fixture_items(path):
        if "post" not in t:
            continue
        for fork, entries in t["post"].items():
            if want_fork and fork != want_fork:
                continue
            for pe in entries:
                idx = pe["indexes"]
                tx = t["transaction"]
                out.append(
                    {
                        "cid": f"{name.split('::')[-1]}|{fork}|d{idx['data']}g{idx['gas']}v{idx['value']}",
                        "env": t["env"],
                        "pre": t["pre"],
                        "tx": tx,
                        "fork": fork,
                        "idx": idx,  # for ssz_builder (fork rules + tx index)
                        "config": t.get("config", {}),
                        "txbytes": pe.get("txbytes"),
                        "gas": h2i(tx["gasLimit"][idx["gas"]]),
                        "val": h2i(tx["value"][idx["value"]]),
                        "data": tx["data"][idx["data"]],
                        "post": pe["state"],
                        "hash": pe.get("hash"),
                        "al": (
                            tx["accessLists"][idx["data"]]
                            if "accessLists" in tx and idx["data"] < len(tx["accessLists"])
                            else []
                        ),
                    }
                )
                if limit and len(out) >= limit:
                    return out
    return out


# Interpreter for the execution-specs venv: it owns the stateless SSZ types, the
# serializer, and the pre-state MPT builder. Default to the sibling checkout,
# with environment overrides for other workspace layouts.
EXECSPECS_ROOT = os.environ.get(
    "EXECSPECS_ROOT", os.path.abspath(os.path.join(ELDIR, "..", "execution-specs"))
)
EXECSPECS_PY = os.environ.get(
    "EXECSPECS_PY", os.path.join(EXECSPECS_ROOT, ".venv", "bin", "python3")
)

# Persistent SSZ SszStatelessInput builder (devtools.harness.ssz_builder --serve), run under the
# execution-specs venv (it owns the stateless SSZ types + serializer + t8n). One
# case JSON per line in, one JSON response per line out ({"input": hex[,
# "expected": hex]} or {"err": msg}).
_ssz_proc = None


def _serve_request(payload):
    global _ssz_proc
    if _ssz_proc is None:
        _ssz_proc = subprocess.Popen(
            [EXECSPECS_PY, "-m", "devtools.harness.ssz_builder", "--serve"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
        )
    assert _ssz_proc.stdin is not None
    assert _ssz_proc.stdout is not None
    _ssz_proc.stdin.write((json.dumps(payload) + "\n").encode())
    _ssz_proc.stdin.flush()
    line = _ssz_proc.stdout.readline()
    if not line:
        raise RuntimeError("ssz_builder --serve exited unexpectedly")
    resp = json.loads(line)
    if "err" in resp:
        raise RuntimeError(resp["err"])
    return resp


def build_guest_pair(case):
    """case -> (input bytes, expected SszStatelessValidationResult bytes): a fully
    VALID Amsterdam block built by the in-process EELS t8n, plus the reference
    guest's byte-exact expected output over that exact input."""
    resp = _serve_request(
        {
            "env": case["env"],
            "pre": case["pre"],
            "tx": case["tx"],
            "fork": case["fork"],
            "idx": case["idx"],
            "config": case["config"],
            "txbytes": case["txbytes"],
        }
    )
    return bytes.fromhex(resp["input"]), bytes.fromhex(resp["expected"])


def collect_stateless(path):
    """Blockchain/stateless fixture blocks carrying statelessInputBytes:
    [(cid, input_bytes, expected_bytes|None)]. Empty for state-test files."""
    out = []
    for name, t in fixture_items(path):
        for i, b in enumerate(t.get("blocks", [])):
            sib = b.get("statelessInputBytes")
            if not sib:
                continue
            want = b.get("statelessOutputBytes")
            out.append(
                (
                    f"{name.split('::')[-1]}#{i}",
                    bytes.fromhex(sib.removeprefix("0x")),
                    bytes.fromhex(want.removeprefix("0x")) if want else None,
                )
            )
    return out


class SpikeGuest:
    """Drive the REAL RISC-V guest ELF on spike, one case at a time, through
    zkvm/build.sh. The first case builds an input-agnostic ELF; every case then
    supplies its bytes to the same ELF through the Spike implementation of the
    standard read_input ABI. The build dir is per invocation (ZKVM_BUILD), so
    concurrent gates cannot race on generated objects. Output is the
    `output_hex=` line the harness prints on the Spike console. Same
    run_once(input)->bytes seam as the native ctypes guest, so everything above
    the backend (input sources, reference oracle, reporting) is shared."""

    def __init__(self, timeout, profile=False, rebuild=False):
        self.zkvm = os.path.join(ELDIR, "zkvm")
        self.build_dir = tempfile.mkdtemp(prefix="zkvm-spike-", dir=TMP_ROOT)
        self.built = False
        self.timeout = timeout
        self.profile = profile
        self.rebuild = rebuild

    def run_once(self, inp):
        self.rebuild = False
        with tempfile.NamedTemporaryFile(suffix=".ssz", delete=False, dir=TMP_ROOT) as tf:
            tf.write(inp)
            vec = tf.name
        env = dict(os.environ, VEC=vec, ZKVM_BUILD=self.build_dir)
        env["EVM_PROFILE"] = "on" if self.profile else "off"
        if self.built:
            env["RUN_ONLY"] = "1"
        try:
            r = subprocess.run(
                [os.path.join(self.zkvm, "build.sh"), "run"],
                capture_output=True,
                env=env,
                timeout=self.timeout,
                cwd=self.zkvm,
            )
        finally:
            os.unlink(vec)
        if r.returncode == 0:
            self.built = True
        m = re.search(rb"^output_hex=([0-9a-f]*)\s*$", r.stdout, re.M)
        if r.returncode != 0 or not m:
            raise RuntimeError(
                f"spike run failed rc={r.returncode}: "
                f"{r.stderr.decode(errors='replace')[-200:].strip()}"
            )
        return bytes.fromhex(m.group(1).decode())


class ZiskGuest:
    """Drive the production ZisK ELF on ziskemu through zkvm_io.h.

    ZisK stdin is an eight-byte little-endian payload length followed by the
    payload and zero padding to an eight-byte boundary. Public output is a
    fixed array of 64 little-endian u32 slots; run_fixtures compares only the
    reference-length prefix and separately requires the unused suffix to be
    zero.
    """

    def __init__(self, timeout, profile=False, rebuild=False):
        self.build_script = os.path.join(ELDIR, "zkvm", "zisk", "build.sh")
        self.build_dir = tempfile.mkdtemp(prefix="zkvm-zisk-", dir=TMP_ROOT)
        self.ziskemu = os.environ.get("ZISKEMU", os.path.expanduser("~/.zisk/bin/ziskemu"))
        self._check_emulator_version()
        self.built = False
        self.timeout = timeout
        self.profile = profile
        self.rebuild = rebuild

    @staticmethod
    def _locked_zisk_version():
        lock_path = os.path.join(ELDIR, "zkvm", "zisk", "Cargo.lock")
        with open(lock_path) as f:
            lock = f.read()
        match = re.search(
            r'\[\[package\]\]\s+name = "ziskos"\s+version = "([^"]+)"',
            lock,
        )
        if not match:
            raise RuntimeError(f"cannot find the locked ziskos version in {lock_path}")
        return match.group(1)

    def _check_emulator_version(self):
        required = self._locked_zisk_version()
        try:
            result = subprocess.run(
                [self.ziskemu, "--version"],
                capture_output=True,
                text=True,
                timeout=10,
            )
        except OSError as exc:
            raise RuntimeError(f"cannot execute ZisK emulator {self.ziskemu!r}: {exc}") from exc
        output = (result.stdout + result.stderr).strip()
        match = re.search(r"\bziskemu\s+([^\s]+)", output)
        if result.returncode != 0 or not match:
            raise RuntimeError(
                f"cannot determine the version of ZisK emulator {self.ziskemu!r}: {output}"
            )
        if match.group(1) != required:
            raise RuntimeError(
                f"ZisK emulator {match.group(1)} is incompatible with the "
                f"guest's locked ziskos {required}; set ZISKEMU to a "
                f"matching ziskemu binary"
            )

    @staticmethod
    def _frame_input(inp):
        framed = len(inp).to_bytes(8, "little") + inp
        return framed + bytes((-len(framed)) % 8)

    def _build(self):
        env = dict(os.environ, ZKVM_BUILD=self.build_dir)
        env["EVM_PROFILE"] = "on" if self.profile else "off"
        r = subprocess.run(
            [self.build_script, "guest"],
            capture_output=True,
            env=env,
            timeout=self.timeout,
            cwd=ELDIR,
        )
        if r.returncode != 0:
            tail = (r.stdout + r.stderr).decode(errors="replace")[-1000:].strip()
            raise RuntimeError(f"ZisK guest build failed rc={r.returncode}: {tail}")
        self.built = True

    def run_once(self, inp):
        self.rebuild = False
        if not self.built:
            self._build()
        elf = os.path.join(self.build_dir, "stateless-validator-evm-sail-zisk.elf")
        with tempfile.TemporaryDirectory(prefix="case-", dir=self.build_dir) as case_dir:
            input_path = os.path.join(case_dir, "input.bin")
            output_path = os.path.join(case_dir, "output.bin")
            with open(input_path, "wb") as f:
                f.write(self._frame_input(inp))
            r = subprocess.run(
                [
                    self.ziskemu,
                    "--elf",
                    elf,
                    "--inputs",
                    input_path,
                    "--output",
                    output_path,
                    "--steps",
                ],
                capture_output=True,
                timeout=self.timeout,
                cwd=ELDIR,
            )
            if os.environ.get("EVM_DEBUG") == "on":
                # ziskemu currently forwards guest fd 2 through its stdout while
                # keeping emulator diagnostics on stderr. Preserve both streams in
                # debug builds without making production fixture output noisy.
                for stream in (r.stdout, r.stderr):
                    if stream:
                        sys.stderr.buffer.write(stream)
                sys.stderr.buffer.flush()
            if r.returncode != 0 or not os.path.exists(output_path):
                tail = (r.stdout + r.stderr).decode(errors="replace")[-1000:].strip()
                raise RuntimeError(f"ziskemu run failed rc={r.returncode}: {tail}")
            with open(output_path, "rb") as f:
                return f.read()


class PythonGuest:
    """Run the generated, typed Python extraction against the guest ABI."""

    def __init__(self):
        extraction_root = os.path.join(ELDIR, "extractions", "python", "src")
        if extraction_root not in sys.path:
            sys.path.insert(0, extraction_root)
        try:
            import evm
            from evm import HostContract
        except ImportError as exc:
            raise RuntimeError(
                "cannot import the generated Python extraction; run "
                "`make extract-python` with its Python dependencies installed"
            ) from exc
        self.evm = evm
        self.host = HostContract

    def run_once(self, inp):
        # Reset Sail registers first; reset() also replaces the current host
        # state, so install this case's explicit input state afterwards.
        self.evm.reset()
        state = self.host.HostState(stateless_input_bytes=bytes(inp))
        self.host.set_state(state)
        try:
            self.evm.main()
        except self.evm.SailExit:
            # Sail's fatal_error is `exit(())`: it leaves the guest without
            # writing a result. The native backends bind it to a host contract
            # that emits the invalid-block output first, so the Python vehicle
            # does the same here and a rejected block stays comparable.
            self.write_invalid_result()
        return bytes(state.public_output)

    def write_invalid_result(self):
        try:
            input_ref = self.evm.decode_stateless_input_ref(self.host.stateless_input())
        except BaseException:
            self.evm.write_invalid_result()
            return
        self.evm.write_validation_result(input_ref, False)


def output_matches(got, expected, zisk=False):
    """Compare one public result without discarding meaningful zero bytes."""
    if expected is None:
        return False
    if not zisk:
        return got == expected
    return got[: len(expected)] == expected and not any(got[len(expected) :])


def run_fixtures(files, args):
    """Drive main.sail over every fixture: embedded statelessInputBytes
    blocks run directly against their statelessOutputBytes; state-test cases are
    built into valid Amsterdam blocks + reference outputs by ssz_builder's t8n
    mode. Pass criterion: the guest's output bytes EQUAL the reference's.
    Backend: native in-process ctypes, the generated Python extraction,
    executable Lean extraction, the real RISC-V ELF on Spike, or the production
    ZisK ELF on ziskemu."""
    if args.spike:
        backend: GuestBackend = SpikeGuest(
            args.timeout or 900.0,
            profile=args.profile,
            rebuild=args.rebuild,
        )
    elif args.zisk:
        backend = ZiskGuest(
            args.timeout or 900.0,
            profile=args.profile,
            rebuild=args.rebuild,
        )
    elif args.lean:
        dump_state.load_lean_guest(rebuild=args.rebuild)
        backend = FunctionGuest(dump_state.run_once_guest)
    elif args.python:
        backend = PythonGuest()
    else:
        dump_state.load_guest(
            rebuild=args.rebuild,
            profile=args.profile,
            build_mode=args.build_mode,
        )
        backend = FunctionGuest(dump_state.run_once_guest)
    npass = ntotal = 0
    fail_reasons: dict[str, int] = {}
    printed_fails = 0
    suppressed_fails = 0
    for f in files:
        if args._trace_files:
            print(f"TRACE_FILE {f}", flush=True)
        work = [(cid, inp, exp) for cid, inp, exp in collect_stateless(f)]
        if not work:
            for c in collect(f, args.fork or "Amsterdam", args.limit):
                try:
                    inp, exp = build_guest_pair(c)
                except Exception as e:
                    work.append((c["cid"], None, str(e)))
                    continue
                work.append((c["cid"], inp, exp))
        for cid, inp, exp in work:
            ntotal += 1
            if inp is None:
                fail_reasons["builderr"] = fail_reasons.get("builderr", 0) + 1
                if args.fail_limit <= 0 or printed_fails < args.fail_limit:
                    printed_fails += 1
                    print(f"FAIL {cid} [{f}]")
                    if args.verbose:
                        print(f"      {exp}")
                else:
                    suppressed_fails += 1
                continue
            try:
                got = backend.run_once(inp)
            except Exception as e:
                fail_reasons["runerr"] = fail_reasons.get("runerr", 0) + 1
                if args.fail_limit <= 0 or printed_fails < args.fail_limit:
                    printed_fails += 1
                    print(f"FAIL {cid} [{f}]")
                    if args.verbose:
                        detail = "".join(
                            traceback.format_exception(type(e), e, e.__traceback__)
                        ).rstrip()
                        print("\n".join(f"      {line}" for line in detail.splitlines()))
                else:
                    suppressed_fails += 1
                continue
            if output_matches(got, exp, zisk=args.zisk):
                npass += 1
                if not args.quiet:
                    print(f"PASS {cid}")
            else:
                cat = "noref" if exp is None else "diff"
                fail_reasons[cat] = fail_reasons.get(cat, 0) + 1
                if args.fail_limit <= 0 or printed_fails < args.fail_limit:
                    printed_fails += 1
                    print(f"FAIL {cid} [{f}]")
                    if args.verbose and exp is not None:
                        gv = got[32:33].hex() if len(got) > 32 else "??"
                        ev = exp[32:33].hex() if len(exp) > 32 else "??"
                        print(
                            f"      got_len={len(got)} exp_len={len(exp)} "
                            f"valid_byte got={gv} exp={ev}"
                        )
                    if args.debug and not (args.spike or args.zisk):
                        snap = dump_state.format_snapshot(
                            dump_state.snapshot(), limit=0 if args.verbose else 12
                        )
                        print("\n".join("      " + ln for ln in snap.splitlines()))
                else:
                    suppressed_fails += 1
    if args.spike:
        vehicle = "spike guest"
    elif args.zisk:
        vehicle = "ZisK guest"
    elif args.lean:
        vehicle = "Lean extraction"
    elif args.python:
        vehicle = "Python extraction"
    else:
        vehicle = f"{args.build_mode} guest"
    print(f"\n=== {npass}/{ntotal} passed ({vehicle}, byte-exact) ===")
    if suppressed_fails:
        print(f"=== suppressed FAIL lines: {suppressed_fails} ===")
    if fail_reasons:
        print("fail categories:", dict(sorted(fail_reasons.items(), key=lambda x: -x[1])))
    if ntotal == 0 or npass != ntotal:
        sys.exit(1)


def run_sharded(files, args):
    """Shard fixture files across N self-invocations (one library instance per
    process -- the C world state is process-global, so this is the only safe
    concurrency) and aggregate their summaries."""
    import ast
    import concurrent.futures as cf

    n = min(args.jobs, len(files))
    shards = [files[i::n] for i in range(n)]
    flags = ["--quiet"] if args.quiet else ([])
    # A native signal kills a worker before Python can report the active case.
    # Keep a flushed file breadcrumb in sharded children so such failures are
    # attributable without adding noise to successful parent output.
    flags.append("--_trace-files")
    if args.verbose:
        flags.append("--verbose")
    if args.profile:
        flags.append("--profile")
    if args.lean:
        flags.append("--lean")
    if args.python:
        flags.append("--python")
    flags += ["--build", args.build_mode]
    if args.fork:
        flags += ["--fork", args.fork]
    if args.limit:
        flags += ["--limit", str(args.limit)]
    if args.timeout is not None:
        flags += ["--timeout", str(args.timeout)]
    if args.fail_limit:
        flags += ["--fail-limit", str(args.fail_limit)]

    def run_shard(shard):
        return subprocess.run(
            [sys.executable, "-m", "devtools.harness.cli", *shard, *flags],
            capture_output=True,
            text=True,
        )

    npass = ntotal = ntimeout = 0
    cats: dict[str, int] = {}
    rc = 0
    with cf.ThreadPoolExecutor(n) as ex:
        futs = {ex.submit(run_shard, sh): i for i, sh in enumerate(shards)}
        for fut in cf.as_completed(futs):
            r = fut.result()
            out = r.stdout + r.stderr
            for ln in out.splitlines():
                if ln.startswith(("FAIL", "TIMEOUT")):
                    print(ln)
            m = re.search(r"=== (\d+)/(\d+) passed(?: \((\d+) timeouts\))?", out)
            if m:
                npass += int(m.group(1))
                ntotal += int(m.group(2))
                ntimeout += int(m.group(3) or 0)
                print(f"[shard {futs[fut] + 1}/{n} done: {m.group(1)}/{m.group(2)}]")
            else:
                rc = 1
                if r.returncode < 0:
                    import signal

                    try:
                        cause = signal.Signals(-r.returncode).name
                    except ValueError:
                        cause = f"signal {-r.returncode}"
                else:
                    cause = f"rc={r.returncode}"
                print(f"[shard {futs[fut] + 1}/{n} produced no summary; {cause}]")
                print("\n".join(out.splitlines()[-10:]))
            mc = re.search(r"fail categories: (\{.*\})", out)
            if mc:
                for k, v in ast.literal_eval(mc.group(1)).items():
                    cats[k] = cats.get(k, 0) + v
    print(
        f"\n=== {npass}/{ntotal} passed "
        f"({'Lean extraction' if args.lean else 'Python extraction' if args.python else args.build_mode + ' guest'}, "
        f"byte-exact) === [{n} jobs]"
    )
    if cats:
        print("fail categories:", dict(sorted(cats.items(), key=lambda x: -x[1])))
    return 1 if (rc or ntotal == 0 or npass != ntotal or ntimeout != 0) else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--fork", default=None)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--rebuild", action="store_true")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument(
        "--build",
        dest="build_mode",
        choices=("standard", "optimized"),
        default="optimized",
        help="native C build mode (default: optimized); standard omits the "
        "sail/optimised module overrides",
    )
    ap.add_argument(
        "--timeout",
        type=float,
        default=None,
        help="per-case wall-clock budget; only meaningful with --spike/--zisk "
        "(default 900s there) -- warm in-process libs cannot "
        "interrupt a C call and the model is gas-bounded",
    )
    ap.add_argument("--quiet", action="store_true", help="only print failures + summary")
    ap.add_argument(
        "--spike",
        action="store_true",
        help="run the REAL RISC-V guest ELF on spike "
        "(full build once, runtime input per case) instead of "
        "the native in-process lib",
    )
    ap.add_argument(
        "--zisk",
        action="store_true",
        help="run the production ZisK guest ELF on ziskemu "
        "(full build once, runtime input per case) instead of "
        "the native in-process lib",
    )
    ap.add_argument(
        "--lean",
        action="store_true",
        help="run the executable Lean extraction in process through the shared fixture-harness ABI",
    )
    ap.add_argument(
        "--python",
        action="store_true",
        help="run the generated typed Python extraction in process "
        "through the shared fixture-harness ABI",
    )
    ap.add_argument(
        "--profile", action="store_true", help="build with optional zkVM cycle-scope markers"
    )
    ap.add_argument(
        "--fail-limit",
        type=int,
        default=0,
        help="print at most this many individual FAIL lines (0 = unlimited)",
    )
    ap.add_argument(
        "--debug",
        "--dump",
        dest="debug",
        action="store_true",
        help="on a native FAIL, print the on-demand post-run state dump",
    )
    ap.add_argument(
        "--jobs",
        type=int,
        default=1,
        help="shard fixture FILES across N worker processes (each worker "
        "loads its own library instance, so the process-global C "
        "world state stays isolated); incompatible with --spike/--zisk, "
        "--debug and --rebuild (rebuild once with --jobs 1 first)",
    )
    ap.add_argument(
        "--_trace-files", dest="_trace_files", action="store_true", help=argparse.SUPPRESS
    )
    args = ap.parse_args()
    if sum((args.spike, args.zisk, args.lean, args.python)) > 1:
        ap.error("--python, --spike, --zisk, and --lean are mutually exclusive")
    if (args.spike or args.zisk) and args.build_mode != "optimized":
        ap.error("--spike and --zisk support only --build optimized")
    if args.lean and args.profile:
        ap.error("--profile is not available for the Lean extraction")
    if args.debug and (args.spike or args.zisk or args.python):
        ap.error("--debug is available only for native and Lean runs")

    # Expand regular fixtures and EEST worker-shard JSONL files recursively.
    import glob as _glob

    files = []
    for p in args.files:
        if os.path.isdir(p):
            files += sorted(_glob.glob(os.path.join(p, "**", "*.json"), recursive=True))
            files += sorted(_glob.glob(os.path.join(p, "**", "*.jsonl"), recursive=True))
        else:
            files.append(p)

    if args.jobs > 1:
        if args.spike or args.zisk or args.debug or args.rebuild:
            ap.error("--jobs is incompatible with --spike/--zisk/--debug/--rebuild")
        if len(files) > 1:
            if args.lean:
                dump_state.load_lean_guest()
            elif args.python:
                PythonGuest()
            else:
                dump_state.load_guest(profile=args.profile, build_mode=args.build_mode)
            sys.exit(run_sharded(files, args))
        # a single file: nothing to shard, fall through sequentially
    run_fixtures(files, args)


if __name__ == "__main__":
    main()
