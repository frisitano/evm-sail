#!/usr/bin/env python3
"""Guest smoke gate over EEST-generated stateless fixtures.

For every blockchain_test block carrying statelessInputBytes under the given
fixture paths (default: zkvm/fixtures/smoke), bake the input into the spike
guest (REBAKE_ONLY fast path after the first full build), run it, and diff the
guest's emitted output_hex byte-exactly against the fixture's
statelessOutputBytes -- the same oracle zkvm/native-runner/run_fixtures.py
applies to the native build.

Usage: run_guest_smoke.py [fixture.json|dir ...] [--timeout N] [--keep-going]
"""
import argparse, glob, json, os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD_SH = os.path.join(HERE, "build.sh")


def blocks(path):
    with open(path) as f:
        suite = json.load(f)
    for tname, t in suite.items():
        for i, b in enumerate(t.get("blocks", [])):
            sib = b.get("statelessInputBytes")
            if sib:
                yield (
                    f"{os.path.basename(path)}::{tname.split('::')[-1][:48]}#{i}",
                    sib,
                    b.get("statelessOutputBytes"),
                )


def expand(paths):
    for p in paths:
        if os.path.isdir(p):
            yield from sorted(glob.glob(os.path.join(p, "**", "*.json"), recursive=True))
        else:
            yield p


def run_guest(vec_path, rebake, timeout):
    env = dict(os.environ, VEC=vec_path)
    if rebake:
        env["REBAKE_ONLY"] = "1"
    r = subprocess.run([BUILD_SH, "run"], capture_output=True, env=env,
                       timeout=timeout, cwd=HERE)
    out = r.stdout.decode(errors="replace")
    m = re.search(r"^output_hex=([0-9a-f]*)\s*$", out, re.M)
    return r.returncode, (m.group(1) if m else None), out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="*", default=None)
    ap.add_argument("--timeout", type=float, default=900.0)
    ap.add_argument("--keep-going", action="store_true")
    args = ap.parse_args()
    paths = args.paths or [os.path.join(HERE, "fixtures", "smoke")]

    npass = nfail = 0
    rebake = False  # first run does the full build
    for f in expand(paths):
        for cid, sib, want in blocks(f):
            raw = bytes.fromhex(sib.removeprefix("0x"))
            with tempfile.NamedTemporaryFile(suffix=".ssz", delete=False) as tf:
                tf.write(raw)
                vec = tf.name
            try:
                rc, got, console = run_guest(vec, rebake, args.timeout)
            finally:
                os.unlink(vec)
            rebake = True
            exp = (want or "").removeprefix("0x").lower()
            if rc == 0 and got and exp and got == exp:
                print(f"PASS    {cid}  ({len(raw)} B in)")
                npass += 1
            else:
                gv = got[64:66] if got and len(got) >= 66 else "??"
                ev = exp[64:66] if len(exp) >= 66 else "??"
                print(f"FAIL    {cid}: rc={rc} guest_valid_byte={gv} exp_valid_byte={ev}")
                print(f"          guest={got[:80] if got else console[-200:]}")
                print(f"          exp  ={exp[:80]}")
                nfail += 1
                if not args.keep_going:
                    print(f"\n=== guest smoke: {npass} pass, {nfail} fail (stopped) ===")
                    sys.exit(1)
    print(f"\n=== guest smoke: {npass} pass, {nfail} fail ===")
    sys.exit(1 if nfail else 0)


if __name__ == "__main__":
    main()
