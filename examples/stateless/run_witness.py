#!/usr/bin/env python3
"""Witness-reader coverage harness for the EL-IR Sail EVM.

For each EEST fixture `pre` alloc, build a REAL secure-trie execution witness
(py-trie, via zkvm/gen_vector.py), wrap it as a schema-prefixed SSZ
`SszStatelessInput`, and feed it to the native `witness_probe` binary, which
builds the witness node-db and re-roots the state trie via lib/mpt.sail's
witness reader (`build_node_db` + `witness_reroot`). We then diff the Sail
reader's REROOT against the trusted py-trie root.

`witness_reroot` walks the entire witness trie and rebuilds it, so a match
certifies the reader parsed every node (branch/extension/leaf, hex-prefix,
embedded children) correctly. The 4748 EEST pre-states give the trie-shape
diversity that the single baked spike vector cannot — the coverage execution-
specs gets by running EEST through stateless.py / witness_state.py.

Usage:
    python3 examples/stateless/run_witness.py <test.json|dir> [...] [--limit N]
            [--rebuild] [--verbose]
Requires `sail` on PATH, a C compiler, and the gen_vector venv deps.
"""
import argparse
import glob
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ELDIR = os.path.abspath(os.path.join(HERE, "..", ".."))
EEST = os.path.join(ELDIR, "examples", "eest")
BIN = os.path.join(HERE, ".witness_probe_bin")
sys.path.insert(0, os.path.join(ELDIR, "zkvm"))
import gen_vector as gv  # noqa: E402


def build_probe(rebuild=False):
    if os.path.exists(BIN) and not rebuild:
        return
    if os.path.exists(BIN):
        os.remove(BIN)
    lib = subprocess.check_output(
        ["sh", "-c", "dirname $(command -v sail)"]).decode().strip() \
        + "/../share/sail/lib"
    cache = os.path.join(HERE, ".rtcache")
    os.makedirs(cache, exist_ok=True)
    sail256 = os.environ.get("SAIL256")
    sailfix = os.environ.get("SAILFIX") or sail256
    sfdir = os.path.join(ELDIR, "zkvm", "runtime",
                         "sail256" if sail256 else "sailfix")
    print("# building witness_probe (one-time%s)..." %
          (" [sailfix]" if sailfix else ""), file=sys.stderr)
    accel = os.path.join(ELDIR, "zkvm", "accel-host")
    subprocess.check_call(
        ["cargo", "build", "--release", "--offline", "--target-dir", "target"],
        cwd=accel)
    accel_lib = os.path.join(accel, "target", "release")
    accel_flags = [f"-L{accel_lib}", "-lzkvm_accel_host",
                   f"-Wl,-rpath,{accel_lib}"]
    stack_flags = (["-Wl,-stack_size,0x20000000"] if sys.platform == "darwin"
                   else ["-Wl,-z,stacksize=0x20000000"])
    host_c = [os.path.join(ELDIR, "ffi", f"{m}.c") for m in
              ("acc_shim", "host_mem", "host_map", "host_stack",
               "host_word", "host_code")]
    ffi_c = os.path.join(EEST, "runner_ffi.c")  # ssz_src over buffered stdin
    src = os.path.join(HERE, "witness_probe.sail")
    subprocess.check_call(
        ["sail", "-c", "-O", "--c-include", "runner_ffi.h", src, "-o",
         BIN + "_gen"], cwd=ELDIR)
    if sailfix:
        objs = []
        for s in ("sail.c", "sail_native.c", "sail_failure.c"):
            o = os.path.join(cache, "sf_" + s[:-2] + ".o")
            subprocess.check_call(
                ["cc", "-O2", "-c", f"-I{sfdir}", f"-I{lib}",
                 os.path.join(sfdir, s), "-o", o])
            objs.append(o)
        subprocess.check_call(
            ["cc", "-O2", f"-I{sfdir}", f"-I{lib}", f"-I{EEST}",
             BIN + "_gen.c", ffi_c, *host_c, *objs, *accel_flags,
             *stack_flags, "-o", BIN])
    else:
        objs = []
        for c in ("sail.c", "rts.c", "sail_failure.c", "sail_config.c",
                  "elf.c", "cJSON.c"):
            o = os.path.join(cache, c[:-2] + ".o")
            if not os.path.exists(o):
                subprocess.check_call(
                    ["cc", "-O2", "-c", f"-I{lib}", "-I/opt/homebrew/include",
                     f"{lib}/{c}", "-o", o])
            objs.append(o)
        subprocess.check_call(
            ["cc", "-O2", f"-I{lib}", "-I/opt/homebrew/include",
             "-L/opt/homebrew/lib", f"-I{EEST}",
             BIN + "_gen.c", ffi_c, *host_c, *objs, *accel_flags,
             *stack_flags, "-lgmp", "-o", BIN])
    for p in (BIN + "_gen.c", BIN + "_gen.h"):
        if os.path.exists(p):
            os.remove(p)


def reroot(pre_alloc, timeout):
    """Build a witness from `pre`, probe the Sail reader, return (parent, reroot)
    hex strings, or None on probe failure/timeout."""
    state_root, nodes, codes = gv.build_witness_from_alloc(pre_alloc)
    blob = gv.make_stateless_blob(state_root, nodes, codes)
    try:
        out = subprocess.run([BIN], input=blob, capture_output=True,
                             timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, state_root
    parent = rr = None
    for line in out.stdout.decode(errors="replace").splitlines():
        if line.startswith("PARENT "):
            parent = line.split()[1]
        elif line.startswith("REROOT "):
            rr = line.split()[1]
    if rr is None:
        return ("ERR:" + out.stderr.decode(errors="replace")[-200:]), state_root
    return (parent, rr), state_root


def collect_pres(path, limit):
    """Yield (name, pre) per test; dedup identical pre allocs within a file."""
    seen = set()
    for name, t in json.load(open(path)).items():
        pre = t.get("pre")
        if not pre:
            continue
        key = json.dumps(pre, sort_keys=True)
        if key in seen:
            continue
        seen.add(key)
        yield (name.split("::")[-1][:60], pre)
        if limit and len(seen) >= limit:
            return


def expand(paths):
    for p in paths:
        if os.path.isdir(p):
            yield from sorted(glob.glob(os.path.join(p, "**", "*.json"),
                                        recursive=True))
        else:
            yield p


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="+")
    ap.add_argument("--limit", type=int, default=0,
                    help="max distinct pre-states per file")
    ap.add_argument("--rebuild", action="store_true")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--timeout", type=float, default=30.0)
    args = ap.parse_args()
    build_probe(args.rebuild)

    npass = nfail = 0
    fail_examples = []
    for f in expand(args.paths):
        for name, pre in collect_pres(f, args.limit):
            res, sroot = reroot(pre, args.timeout)
            want = "0x" + sroot.hex()
            if res is None:
                nfail += 1
                fail_examples.append(f"{name}: TIMEOUT")
                continue
            if isinstance(res, str):  # probe error
                nfail += 1
                fail_examples.append(f"{name}: {res}")
                continue
            parent, rr = res
            # compare as integers: Sail hex_str emits minimal-width hex (drops
            # leading-zero nibbles), so a root with a leading zero byte differs
            # only in formatting, not value.
            w = int(want, 16)
            if int(rr, 16) == w and int(parent, 16) == w:
                npass += 1
                if args.verbose:
                    print(f"PASS {name} {want[:18]}..")
            else:
                nfail += 1
                fail_examples.append(
                    f"{name}: reroot={rr[:18]} parent={parent[:18]} want={want[:18]}")
                print(f"FAIL {name}: reroot={rr} want={want}")
    print(f"\n=== witness reroot: {npass}/{npass + nfail} matched ===")
    if fail_examples:
        print("first failures:")
        for e in fail_examples[:15]:
            print("  " + e)


if __name__ == "__main__":
    main()
