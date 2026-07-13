#!/usr/bin/env python3
"""Run EEST fixtures against the evm-sail Sail EVM -- the single harness for
BOTH executable entries.

Architecture: the Sail model is built ONCE into a shared library and driven
IN-PROCESS via ctypes (dump_state.py). Each fixture case is serialized to the
SSZ SszStatelessInput (ssz_builder.py, under the execution-specs venv) and fed
to the selected entry:

RUNNER (default; libevmsail_runner, EVM_ENTRY=runner): cross-fork state-test
execution. The runner emits NO byte stream: it publishes the cumulative gas +
the recomputed post-state root in the shared zkvm_out_* registers, read
C-side via the snapshot dump (dump_state.result()). The root -- a
cryptographic commitment to the FULL post-state -- IS the pass criterion,
compared to the fixture's `post[fork][i].hash`. Being in-process also lets
--dump inspect the model's live post-run state (the same snapshot) for
analysis.

GUEST (--guest; libevmsail_guest, EVM_ENTRY=guest): the Amsterdam stateless
full-block validator, gated BYTE-EXACT against the EELS reference. Two input
sources, auto-detected per fixture file:
  - state tests: ssz_builder guest mode executes the case through the
    in-process EELS t8n, producing a fully VALID single-tx block input AND the
    reference guest's expected SszStatelessValidationResult bytes;
  - blockchain/stateless fixtures already carrying statelessInputBytes /
    statelessOutputBytes (e.g. corpora under zkvm/.fixtures/): fed directly.
Two execution vehicles: the native in-process ctypes lib (default), or the
REAL RISC-V guest ELF on spike (--spike; input baked into the ELF, full build
once then REBAKE_ONLY per case). This subsumes the old
zkvm/native-runner/run_fixtures*.py and zkvm/run_guest_smoke.py runners.

Usage:
    python3 harness/run.py <test.json|dir> [...] [--fork F] [--limit N]
            [--guest [--spike]] [--rebuild] [--verbose] [--dump]
Requires `sail` on PATH and a C compiler; ssz_builder.py runs under the
execution-specs venv (EXECSPECS_PY).
"""
import argparse, json, os, re, subprocess, sys, tempfile
import dump_state   # in-process ctypes harness for libevmsail_runner + state snapshot

HERE = os.path.dirname(os.path.abspath(__file__))
ELDIR = os.path.abspath(os.path.join(HERE, ".."))

def h2i(x):
    if isinstance(x, int): return x
    x = str(x); return int(x, 16) if x.startswith("0x") else int(x)
def ai(a):
    return int(a.lower().replace("0x", ""), 16) if a else 0

def collect(path, want_fork, limit):
    out = []
    for name, t in json.load(open(path)).items():
        if "post" not in t: continue
        for fork, entries in t["post"].items():
            if want_fork and fork != want_fork: continue
            for pe in entries:
                idx = pe["indexes"]; tx = t["transaction"]
                out.append({
                    "cid": f"{name.split('::')[-1][:46]}|{fork}|d{idx['data']}g{idx['gas']}v{idx['value']}",
                    "env": t["env"], "pre": t["pre"], "tx": tx,
                    "fork": fork, "idx": idx,   # for ssz_builder (fork rules + tx index)
                    "gas": h2i(tx["gasLimit"][idx["gas"]]), "val": h2i(tx["value"][idx["value"]]),
                    "data": tx["data"][idx["data"]], "post": pe["state"], "hash": pe.get("hash"),
                    "al": (tx["accessLists"][idx["data"]] if "accessLists" in tx
                           and idx["data"] < len(tx["accessLists"]) else [])})
                if limit and len(out) >= limit: return out
    return out

# Interpreter for the execution-specs venv: it owns the stateless SSZ types, the
# serializer, and the pre-state MPT builder. run.py's own interpreter lacks
# those deps, so ssz_builder.py (which builds the MPT witness in-process) runs
# there over --serve.
EXECSPECS_PY = "/Users/f/dev/ethereum/execution-specs/.venv/bin/python3"

# Persistent SSZ SszStatelessInput builder (ssz_builder.py --serve), run under the
# execution-specs venv (it owns the stateless SSZ types + serializer + t8n). One
# case JSON per line in, one JSON response per line out ({"input": hex[,
# "expected": hex]} or {"err": msg}).
_ssz_proc = None
def _serve_request(payload):
    global _ssz_proc
    if _ssz_proc is None:
        _ssz_proc = subprocess.Popen(
            [EXECSPECS_PY, os.path.join(HERE, "ssz_builder.py"), "--serve"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    _ssz_proc.stdin.write((json.dumps(payload) + "\n").encode()); _ssz_proc.stdin.flush()
    line = _ssz_proc.stdout.readline()
    if not line:
        raise RuntimeError("ssz_builder --serve exited unexpectedly")
    resp = json.loads(line)
    if "err" in resp:
        raise RuntimeError(resp["err"])
    return resp

def build_ssz_input(case):
    """case (env/pre/tx/fork/idx) -> serialized SszStatelessInput bytes (runner)."""
    resp = _serve_request({"env": case["env"], "pre": case["pre"], "tx": case["tx"],
                           "fork": case["fork"], "idx": case["idx"]})
    return bytes.fromhex(resp["input"])

def build_guest_pair(case):
    """case -> (input bytes, expected SszStatelessValidationResult bytes): a fully
    VALID Amsterdam block built by the in-process EELS t8n, plus the reference
    guest's byte-exact expected output over that exact input."""
    resp = _serve_request({"mode": "guest", "env": case["env"], "pre": case["pre"],
                           "tx": case["tx"], "fork": case["fork"], "idx": case["idx"]})
    return bytes.fromhex(resp["input"]), bytes.fromhex(resp["expected"])

def run_cases(cases, timeout=None):
    """Run each case as one SszStatelessInput through the IN-PROCESS runner lib
    (dump_state.run_once wipes state and runs; the runner emits NO byte stream --
    the (gas, post-state root) result is read from the shared zkvm_out_*
    registers via the snapshot dump, dump_state.result()). Returns
    {index: result}; a build failure maps to an error result. `timeout` is accepted
    for call-site compatibility but unused: the warm in-process worker can't
    interrupt a C call, and the runner is gas-bounded so it terminates."""
    results = {}
    for n, c in enumerate(cases):
        try:
            inp = build_ssz_input(c)
        except Exception as e:
            results[n] = {"root": None, "gas": 0, "err": str(e)}
            continue
        dump_state.run_once(inp)
        ok, gas, root, exc = dump_state.result()
        if ok:
            results[n] = {"root": root, "gas": gas}
        else:
            results[n] = {"root": None, "gas": gas, "exc": exc, "err": exc}
    return results

def collect_stateless(path):
    """Blockchain/stateless fixture blocks carrying statelessInputBytes:
    [(cid, input_bytes, expected_bytes|None)]. Empty for state-test files."""
    out = []
    for name, t in json.load(open(path)).items():
        for i, b in enumerate(t.get("blocks", [])):
            sib = b.get("statelessInputBytes")
            if not sib:
                continue
            want = b.get("statelessOutputBytes")
            out.append((f"{name.split('::')[-1][:48]}#{i}",
                        bytes.fromhex(sib.removeprefix("0x")),
                        bytes.fromhex(want.removeprefix("0x")) if want else None))
    return out


class SpikeGuest:
    """Drive the REAL RISC-V guest ELF on spike, one case at a time, through
    zkvm/build.sh: the input vector is BAKED into the ELF (no host I/O in the
    zkVM model), so each case is a rebake + relink + spike boot -- the first
    case pays the full model build, later ones set REBAKE_ONLY. The build dir
    is per-invocation (ZKVM_BUILD): concurrent gates sharing zkvm/build/ race
    on the baked vector/ELF and byte-exactly report the WRONG fixture. Output
    is the `output_hex=` line the harness prints on the spike console. Same
    run_once(input)->bytes seam as the native ctypes guest, so everything
    above the backend (input sources, reference oracle, reporting) is shared."""

    def __init__(self, timeout):
        self.zkvm = os.path.join(ELDIR, "zkvm")
        self.build_dir = tempfile.mkdtemp(prefix="zkvm-spike-")
        self.rebake = False
        self.timeout = timeout

    def run_once(self, inp):
        with tempfile.NamedTemporaryFile(suffix=".ssz", delete=False) as tf:
            tf.write(inp)
            vec = tf.name
        env = dict(os.environ, VEC=vec, ZKVM_BUILD=self.build_dir)
        if self.rebake:
            env["REBAKE_ONLY"] = "1"
        try:
            r = subprocess.run([os.path.join(self.zkvm, "build.sh"), "run"],
                               capture_output=True, env=env,
                               timeout=self.timeout, cwd=self.zkvm)
        finally:
            os.unlink(vec)
        self.rebake = True
        m = re.search(rb"^output_hex=([0-9a-f]*)\s*$", r.stdout, re.M)
        if r.returncode != 0 or not m:
            raise RuntimeError(f"spike run failed rc={r.returncode}: "
                               f"{r.stderr.decode(errors='replace')[-200:].strip()}")
        return bytes.fromhex(m.group(1).decode())


def run_guest(files, args):
    """Drive the stateless GUEST over every fixture: embedded statelessInputBytes
    blocks run directly against their statelessOutputBytes; state-test cases are
    built into valid Amsterdam blocks + reference outputs by ssz_builder's t8n
    mode. Pass criterion: the guest's output bytes EQUAL the reference's.
    Backend: the native in-process ctypes lib, or the real RISC-V ELF on spike
    (--spike)."""
    if args.spike:
        run_once = SpikeGuest(args.timeout or 900.0).run_once
    else:
        dump_state.load_guest(rebuild=args.rebuild)
        run_once = dump_state.run_once_guest
    npass = ntotal = 0
    fail_reasons = {}
    for f in files:
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
                print(f"FAIL {cid}")
                if args.verbose: print(f"      {exp}")
                continue
            try:
                got = run_once(inp)
            except Exception as e:
                fail_reasons["runerr"] = fail_reasons.get("runerr", 0) + 1
                print(f"FAIL {cid}")
                if args.verbose: print(f"      {e}")
                continue
            if exp is not None and got == exp:
                npass += 1
                if not args.quiet: print(f"PASS {cid}")
            else:
                cat = "noref" if exp is None else "diff"
                fail_reasons[cat] = fail_reasons.get(cat, 0) + 1
                print(f"FAIL {cid}")
                if args.verbose and exp is not None:
                    gv = got[32:33].hex() if len(got) > 32 else "??"
                    ev = exp[32:33].hex() if len(exp) > 32 else "??"
                    print(f"      got_len={len(got)} exp_len={len(exp)} "
                          f"valid_byte got={gv} exp={ev}")
    vehicle = "spike guest" if args.spike else "guest"
    print(f"\n=== {npass}/{ntotal} passed ({vehicle}, byte-exact) ===")
    if fail_reasons: print("fail categories:", dict(sorted(fail_reasons.items(), key=lambda x:-x[1])))
    if ntotal == 0 or npass != ntotal:
        sys.exit(1)



def run_sharded(files, args):
    """Shard fixture files across N self-invocations (one library instance per
    process -- the C world state is process-global, so this is the only safe
    concurrency) and aggregate their summaries."""
    import ast, concurrent.futures as cf
    n = min(args.jobs, len(files))
    shards = [files[i::n] for i in range(n)]
    flags = ["--quiet"] if args.quiet else ([])
    if args.verbose: flags.append("--verbose")
    if args.guest: flags.append("--guest")
    if args.fork: flags += ["--fork", args.fork]
    if args.limit: flags += ["--limit", str(args.limit)]
    if args.timeout is not None: flags += ["--timeout", str(args.timeout)]
    if args.fail_limit: flags += ["--fail-limit", str(args.fail_limit)]
    script = os.path.abspath(__file__)

    def run_shard(shard):
        return subprocess.run([sys.executable, script] + shard + flags,
                              capture_output=True, text=True)

    npass = ntotal = ntimeout = 0
    cats = {}
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
                npass += int(m.group(1)); ntotal += int(m.group(2))
                ntimeout += int(m.group(3) or 0)
                print(f"[shard {futs[fut] + 1}/{n} done: {m.group(1)}/{m.group(2)}]")
            else:
                rc = 1
                print(f"[shard {futs[fut] + 1}/{n} produced no summary; rc={r.returncode}]")
                print("\n".join(out.splitlines()[-10:]))
            mc = re.search(r"fail categories: (\{.*\})", out)
            if mc:
                for k, v in ast.literal_eval(mc.group(1)).items():
                    cats[k] = cats.get(k, 0) + v
    mode = " (guest, byte-exact)" if args.guest else f" ({ntimeout} timeouts)"
    print(f"\n=== {npass}/{ntotal} passed{mode} === [{n} jobs]")
    if cats: print("fail categories:", dict(sorted(cats.items(), key=lambda x: -x[1])))
    return 1 if (rc or ntotal == 0 or npass != ntotal or ntimeout != 0) else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+"); ap.add_argument("--fork", default=None)
    ap.add_argument("--limit", type=int, default=0); ap.add_argument("--rebuild", action="store_true")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--timeout", type=float, default=None,
                    help="per-case wall-clock budget; only meaningful with --spike "
                         "(default 900s there) -- the warm in-process libs cannot "
                         "interrupt a C call and the model is gas-bounded")
    ap.add_argument("--quiet", action="store_true", help="only print failures + summary")
    ap.add_argument("--guest", action="store_true",
                    help="drive the stateless GUEST (byte-exact vs the EELS reference) "
                         "instead of the state-test runner")
    ap.add_argument("--spike", action="store_true",
                    help="with --guest: run the REAL RISC-V guest ELF on spike "
                         "(full build once, then REBAKE_ONLY per case) instead of "
                         "the native in-process lib")
    ap.add_argument("--fail-limit", type=int, default=0,
                    help="print at most this many individual FAIL lines (0 = unlimited)")
    ap.add_argument("--dump", action="store_true",
                    help="on a FAIL, re-run the case and print the post-run state snapshot "
                         "(materialized accounts+storage, stack, memory) for analysis")
    ap.add_argument("--jobs", type=int, default=1,
                    help="shard fixture FILES across N worker processes (each worker "
                         "loads its own library instance, so the process-global C "
                         "world state stays isolated); incompatible with --spike, "
                         "--dump and --rebuild (rebuild once with --jobs 1 first)")
    args = ap.parse_args()

    # expand directories to the .json files within (recursive)
    import glob as _glob
    files = []
    for p in args.files:
        if os.path.isdir(p):
            files += sorted(_glob.glob(os.path.join(p, "**", "*.json"), recursive=True))
        else:
            files.append(p)

    if args.spike and not args.guest:
        ap.error("--spike requires --guest (the runner has no spike build)")
    if args.jobs > 1:
        if args.spike or args.dump or args.rebuild:
            ap.error("--jobs is incompatible with --spike/--dump/--rebuild")
        if len(files) > 1:
            sys.exit(run_sharded(files, args))
        # a single file: nothing to shard, fall through sequentially
    if args.guest:
        run_guest(files, args)
        return
    dump_state.load(rebuild=args.rebuild)

    npass = ntotal = ntimeout = 0
    fail_reasons = {}   # coarse category -> count
    printed_fails = 0
    suppressed_fails = 0
    for f in files:
        allcases = collect(f, args.fork, args.limit)
        if not allcases: continue
        # Run cases in small chunks so one slow file's timeout doesn't take down a
        # whole 100+-case batch: the timeout is per-chunk, not per-file (a single
        # pathological file otherwise dominates the timeout count).
        cases = []; results = {}
        for ci in range(0, len(allcases), 8):
            chunk = allcases[ci:ci + 8]
            rr = run_cases(chunk, args.timeout)
            base = len(cases)
            for n in range(len(chunk)): results[base + n] = rr.get(n, {"root": None, "gas": 0})
            cases.extend(chunk)
        for n, c in enumerate(cases):
            ntotal += 1
            r = results.get(n, {"root": None, "gas": 0})
            if r.get("timeout"):
                ntimeout += 1
                if not args.quiet: print(f"TIMEOUT {c['cid']}")
                continue
            # The post-state root IS the pass criterion: a cryptographic commitment
            # to the ENTIRE post-state (base + overlay), strictly stronger than the
            # fixture's partial per-account list, and it captures effects (SELFDESTRUCT
            # wipes, unchanged base storage) a flat account/storage dump cannot.
            exp, got = c.get("hash"), r.get("root")
            if exp is None:      cat, ok = "nohash", False    # fixture carries no post root
            elif got is None:    # ssz build failure, or a Sail exception escaped the run
                cat, ok = ("exception" if r.get("exc") else "builderr"), False
            else:                ok = (got == h2i(exp)); cat = None if ok else "root"
            if ok:
                npass += 1
                if not args.quiet: print(f"PASS {c['cid']}")
            else:
                fail_reasons[cat] = fail_reasons.get(cat, 0) + 1
                should_print = args.fail_limit <= 0 or printed_fails < args.fail_limit
                if should_print:
                    printed_fails += 1
                    print(f"FAIL {c['cid']}")
                    if args.verbose:
                        if cat == "root":       print(f"      root {hex(got)} != {c['hash']}")
                        elif cat == "builderr": print(f"      {r.get('err', 'runner produced no root')}")
                        else:                   print("      fixture has no expected post-state root")
                    if args.dump and cat != "builderr":
                        try:
                            dump_state.run_once(build_ssz_input(c))
                            snap = dump_state.format_snapshot(dump_state.snapshot(),
                                                              limit=0 if args.verbose else 12)
                            print("\n".join("      " + ln for ln in snap.splitlines()))
                        except Exception as e:
                            print(f"      (dump failed: {e})")
                else:
                    suppressed_fails += 1
    print(f"\n=== {npass}/{ntotal} passed ({ntimeout} timeouts) ===")
    if suppressed_fails:
        print(f"=== suppressed FAIL lines: {suppressed_fails} ===")
    if fail_reasons: print("fail categories:", dict(sorted(fail_reasons.items(), key=lambda x:-x[1])))
    if ntotal == 0 or npass != ntotal or ntimeout != 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
