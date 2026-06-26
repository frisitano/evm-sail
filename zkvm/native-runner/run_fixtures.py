#!/usr/bin/env python3
"""Drive the native zkVM stateless guest over EEST stateless.py fixtures.

Each EEST blockchain_test (for_amsterdam / tests-zkevm) block carries a
`statelessInputBytes` (the schema-prefixed SSZ SszStatelessInput the guest
consumes) and a `statelessOutputBytes` (the canonical SszStatelessValidationResult
the reference produces). For each block we hex-decode the input to a temp file,
run the single-process native runner over it, and diff the guest's hex output
against the expected output.

SERIAL on purpose: each child reserves a 512 MB stack, so we run exactly one at
a time. No fan-out.

Usage: run_fixtures.py --bin <zkvm_native> <fixture.json|dir> [...]
"""
import argparse, glob, json, os, subprocess, sys, tempfile


def blocks(path):
    with open(path) as f:
        suite = json.load(f)
    for tname, t in suite.items():
        for i, b in enumerate(t.get("blocks", [])):
            sib = b.get("statelessInputBytes")
            if sib:
                yield (f"{os.path.basename(path)}::{tname.split('::')[-1][:48]}#{i}",
                       sib, b.get("statelessOutputBytes"))


def expand(paths):
    for p in paths:
        if os.path.isdir(p):
            yield from sorted(glob.glob(os.path.join(p, "**", "*.json"), recursive=True))
        else:
            yield p


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True)
    ap.add_argument("paths", nargs="+")
    ap.add_argument("--timeout", type=float, default=60.0)
    args = ap.parse_args()

    npass = nfail = nerr = 0
    for f in expand(args.paths):
        for cid, sib, want in blocks(f):
            raw = bytes.fromhex(sib.removeprefix("0x"))
            with tempfile.NamedTemporaryFile(suffix=".ssz", delete=False) as tf:
                tf.write(raw)
                tpath = tf.name
            try:
                r = subprocess.run([args.bin, tpath], capture_output=True,
                                   timeout=args.timeout)
            except subprocess.TimeoutExpired:
                print(f"TIMEOUT {cid}")
                nerr += 1
                os.unlink(tpath)
                continue
            os.unlink(tpath)
            got = r.stdout.decode(errors="replace").strip()
            if r.returncode != 0 or not got:
                print(f"ERROR   {cid}: rc={r.returncode} "
                      f"stderr={r.stderr.decode(errors='replace')[-120:].strip()}")
                nerr += 1
                continue
            exp = (want or "").removeprefix("0x").lower()
            if exp and got == exp:
                print(f"PASS    {cid}  ({len(raw)} B in)")
                npass += 1
            else:
                # surface the mismatch shape: validation byte is at offset 32
                gv = got[64:66] if len(got) >= 66 else "??"
                ev = exp[64:66] if len(exp) >= 66 else "??"
                print(f"DIFF    {cid}: guest_len={len(got)//2} exp_len={len(exp)//2} "
                      f"guest_valid_byte={gv} exp_valid_byte={ev}")
                print(f"          guest={got[:80]}")
                print(f"          exp  ={exp[:80]}")
                nfail += 1
    print(f"\n=== stateless fixtures: {npass} pass, {nfail} diff, {nerr} error ===")


if __name__ == "__main__":
    main()
