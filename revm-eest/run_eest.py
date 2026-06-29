#!/usr/bin/env python3
"""Run EEST state-test fixtures against the evm-sail Sail EVM.

Compile-once architecture: the Sail `runner` (runner.sail) is built into a single
binary ONCE, then every test is fed to it at runtime as a flat int-stream on
stdin (the fixture data is INPUT, not generated code). The runner dumps the
resulting accounts + storage; this script compares them to the fixture's explicit
`post[fork][i].state`. No keccak-MPT state root needed.

Usage:
    python3 revm-eest/run_eest.py <test.json> [...] [--fork F] [--limit N]
            [--rebuild] [--verbose]
Requires `sail` on PATH, a C compiler, and libgmp.
"""
import json, os, subprocess, sys, argparse

HERE = os.path.dirname(os.path.abspath(__file__))
ELDIR = os.path.abspath(os.path.join(HERE, ".."))
BIN = os.path.join(HERE, ".runner_bin")

def h2i(x):
    if isinstance(x, int): return x
    x = str(x); return int(x, 16) if x.startswith("0x") else int(x)
def ai(a):
    return int(a.lower().replace("0x", ""), 16) if a else 0
def hexbytes(s):
    s = (s or "").lower().replace("0x", "")
    return bytes.fromhex(s) if s else b""

def build_runner(rebuild=False):
    if os.path.exists(BIN) and not rebuild:
        return
    if os.path.exists(BIN): os.remove(BIN)   # never leave a stale binary if the build fails
    lib = subprocess.check_output(["sh","-c","dirname $(command -v sail)"]).decode().strip() + "/../share/sail/lib"
    cache = os.path.join(HERE, ".rtcache"); os.makedirs(cache, exist_ok=True)
    # GMP-free fixed-width runtimes: SAILFIX = the guest-shared baseline,
    # SAIL256 = the host-optimized variant (sized limb ops, Knuth-D division).
    sail256 = os.environ.get("SAIL256")
    sailfix = os.environ.get("SAILFIX") or sail256
    sfdir = os.path.join(ELDIR, "zkvm", "runtime", "sail256" if sail256 else "sailfix")
    print("# building runner (one-time%s)..." % (" [sailfix]" if sailfix else ""), file=sys.stderr)
    # Build the host accelerator crate (industry crypto: ecrecover/bn254/modexp/...)
    # and link its cdylib in place of the C reference impl. acc_shim.c still marshals.
    accel = os.path.join(ELDIR, "zkvm", "accel-host")
    subprocess.check_call(["cargo", "build", "--release", "--offline", "--target-dir", "target"], cwd=accel)
    accel_lib = os.path.join(accel, "target", "release")
    accel_flags = [f"-L{accel_lib}", "-lzkvm_accel_host", f"-Wl,-rpath,{accel_lib}"]
    # 512MB main-thread stack (arm64 ld cap): recursive Sail list ops over multi-MB
    # calldata (e.g. the 1.2MB EIP-2537 multi-inf pairing input) overflow the 8MB default.
    stack_flags = (["-Wl,-stack_size,0x20000000"] if sys.platform == "darwin"
                   else ["-Wl,-z,stacksize=0x20000000"])
    if sailfix:
        # GMP-free: sailfix sail.c + sail_native.c (string/print/rts) + sail_failure.c.
        objs = []
        for src in (os.path.join(sfdir,"sail.c"), os.path.join(sfdir,"sail_native.c"), os.path.join(sfdir,"sail_failure.c")):
            o = os.path.join(cache, "sf_" + os.path.basename(src)[:-2] + ".o")
            subprocess.check_call(["cc","-O2","-c",f"-I{sfdir}",f"-I{lib}",src,"-o",o])
            objs.append(o)
        subprocess.check_call(["sail","-c","-O","--c-include","runner_ffi.h",
                               "sail/runner.sail","-o",BIN+"_gen"], cwd=ELDIR)
        subprocess.check_call(["cc","-O2",f"-I{sfdir}",f"-I{lib}",
                               BIN+"_gen.c", os.path.join(HERE,"runner_ffi.c"),
                               os.path.join(ELDIR,"ffi","acc_shim.c"),
                               os.path.join(ELDIR,"ffi","host_mem.c"),
                               os.path.join(ELDIR,"ffi","host_map.c"),
                               os.path.join(ELDIR,"ffi","host_stack.c"),
                               os.path.join(ELDIR,"ffi","host_word.c"),
                               os.path.join(ELDIR,"ffi","host_code.c"),
                               os.path.join(ELDIR,"ffi","host_nodedb.c"),
                               os.path.join(ELDIR,"ffi","host_acctmap.c"),
                               os.path.join(ELDIR,"ffi","host_preimage.c"),
                               *objs, *accel_flags, *stack_flags, "-o",BIN])
    else:
        objs = []
        for c in ("sail.c","rts.c","sail_failure.c","sail_config.c","elf.c","cJSON.c"):
            o = os.path.join(cache, c[:-2] + ".o")
            if not os.path.exists(o):
                subprocess.check_call(["cc","-O2","-c",f"-I{lib}","-I/opt/homebrew/include",f"{lib}/{c}","-o",o])
            objs.append(o)
        subprocess.check_call(["sail","-c","-O","--c-include","runner_ffi.h",
                               "sail/runner.sail","-o",BIN+"_gen"], cwd=ELDIR)
        subprocess.check_call(["cc","-O2",f"-I{lib}","-I/opt/homebrew/include","-L/opt/homebrew/lib",
                               BIN+"_gen.c", os.path.join(HERE,"runner_ffi.c"),
                               os.path.join(ELDIR,"ffi","acc_shim.c"),
                               os.path.join(ELDIR,"ffi","host_mem.c"),
                               os.path.join(ELDIR,"ffi","host_map.c"),
                               os.path.join(ELDIR,"ffi","host_stack.c"),
                               os.path.join(ELDIR,"ffi","host_word.c"),
                               os.path.join(ELDIR,"ffi","host_code.c"),
                               os.path.join(ELDIR,"ffi","host_nodedb.c"),
                               os.path.join(ELDIR,"ffi","host_acctmap.c"),
                               os.path.join(ELDIR,"ffi","host_preimage.c"),
                               *objs, *accel_flags, *stack_flags, "-lgmp","-o",BIN])
    for p in (BIN+"_gen.c", BIN+"_gen.h"):
        if os.path.exists(p): os.remove(p)

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
                    "gas": h2i(tx["gasLimit"][idx["gas"]]), "val": h2i(tx["value"][idx["value"]]),
                    "data": tx["data"][idx["data"]], "post": pe["state"], "hash": pe.get("hash"),
                    "al": (tx["accessLists"][idx["data"]] if "accessLists" in tx
                           and idx["data"] < len(tx["accessLists"]) else [])})
                if limit and len(out) >= limit: return out
    return out

def vbytes(ints):
    """Serialize a flat list of non-negative ints as varints: [len][big-endian]."""
    out = bytearray()
    for v in ints:
        v = int(v)
        L = (v.bit_length() + 7) // 8
        out.append(L)
        out += v.to_bytes(L, "big")
    return bytes(out)

def encode(c):
    """Encode one case into the runner's int-stream (must match runner.sail's read order)."""
    env, pre, tx = c["env"], c["pre"], c["tx"]
    s = [len(pre)]
    for a, acc in pre.items():
        code = hexbytes(acc.get("code"))
        s += [ai(a), h2i(acc.get("nonce","0x0")), h2i(acc.get("balance","0x0")), len(code), *code]
        sto = acc.get("storage", {})
        s += [len(sto)]
        for k, v in sto.items(): s += [h2i(k), h2i(v)]
    base = h2i(env.get("currentBaseFee","0x0"))
    excess = h2i(env.get("currentExcessBlobGas","0x0"))
    s += [h2i(env.get("currentNumber","0x0")), h2i(env.get("currentTimestamp","0x0")),
          h2i(env.get("currentGasLimit","0x0")), ai(env["currentCoinbase"]), base,
          h2i(env.get("currentRandom", env.get("currentDifficulty","0x0"))), 1,
          blob_gas_price(excess), h2i(env.get("slotNumber","0x0"))]  # EIP-7843 SLOTNUM
    gp = h2i(tx["gasPrice"]) if "gasPrice" in tx else min(
        h2i(tx.get("maxFeePerGas","0x0")), base + h2i(tx.get("maxPriorityFeePerGas","0x0")))
    is_create = 0 if tx.get("to") else 1
    data = hexbytes(c["data"])
    # priority fee, saturating at 0 (matches revm-eest's gp.saturating_sub(base));
    # gp < base means an invalid tx, judged from gp vs base, not a negative tip.
    s += [ai(tx["sender"]), h2i(tx.get("nonce","0x0")), gp, max(0, gp-base), c["gas"], is_create,
          0 if is_create else ai(tx["to"]), c["val"], len(data), *data]
    # EIP-2930 access list: n_addrs addresses, then n_keys (addr, slot) pairs
    al = c.get("al") or []
    addrs = [ai(e["address"]) for e in al]
    keys = [(ai(e["address"]), h2i(k)) for e in al for k in e.get("storageKeys", [])]
    s += [len(addrs), *addrs, len(keys)]
    for a, k in keys: s += [a, k]
    # EIP-4844 num blobs; then fee caps (max_fee_per_gas, max_fee_per_blob_gas) for validity
    max_fee = h2i(tx["gasPrice"]) if "gasPrice" in tx else h2i(tx.get("maxFeePerGas","0x0"))
    s += [len(tx.get("blobVersionedHashes", [])), max_fee, h2i(tx.get("maxFeePerBlobGas","0x0")),
          h2i(tx.get("maxPriorityFeePerGas", tx.get("gasPrice","0x0")))]
    # EIP-7702 type-4 envelope flag (an EMPTY auth list is still a type-4 tx -> invalid)
    s += [1 if tx.get("authorizationList") is not None else 0]
    # EIP-7702 authorization list: each (valid_sig, authority, delegate, nonce, chainId).
    # The runner pre-validates the signature (recoverable signer, EIP-2 s<=n/2, r/yParity).
    SECP_N = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
    auths = tx.get("authorizationList") or []
    s += [len(auths)]
    for a in auths:
        signer = a.get("signer")
        sv = h2i(a.get("s","0x0")); rv = h2i(a.get("r","0x0"))
        yp = h2i(a.get("yParity", a.get("v","0x0")))
        valid = 1 if (signer and 1 <= sv <= SECP_N//2 and 1 <= rv < SECP_N
                      and yp in (0,1) and h2i(a.get("nonce","0x0")) < 2**64-1) else 0
        s += [valid, ai(signer) if signer else 0, ai(a["address"]),
              h2i(a.get("nonce","0x0")), h2i(a.get("chainId","0x0"))]
    # EIP-4844 blob section: is_blob flag (type-3 marker), then the actual 32-byte
    # versioned hashes as decimal words (BLOBHASH operands + version validity).
    hashes = [h2i(h) for h in tx.get("blobVersionedHashes", [])]
    is_blob = 1 if ("maxFeePerBlobGas" in tx or hashes) else 0
    s += [is_blob, len(hashes), *hashes]
    return s


def blob_gas_price(excess):
    """EIP-4844 fake_exponential(1, excess, 3338477)."""
    denom, i, output, num = 3338477, 1, 0, 3338477
    while num > 0:
        output += num
        num = num * excess // (denom * i)
        i += 1
    return output // denom

def compare(expected, accounts, storage):
    fails = []
    for a, acc in expected.items():
        ad = ai(a); n, b = accounts.get(ad, (0, 0))
        if "nonce" in acc and n != h2i(acc["nonce"]):     fails.append(f"{a[:10]} nonce {n}!={h2i(acc['nonce'])}")
        if "balance" in acc and b != h2i(acc["balance"]): fails.append(f"{a[:10]} bal {b}!={h2i(acc['balance'])}")
        for k, v in acc.get("storage", {}).items():
            got = storage.get((ad, h2i(k)), 0)
            if got != h2i(v): fails.append(f"{a[:8]}[{k}] {got}!={h2i(v)}")
    return fails

def fork_level(name):
    """Map an EEST fork name to a chronological level for fork-gated gas rules."""
    name = (name or "").lower()
    for k, v in (("amsterdam", 4), ("osaka", 3), ("prague", 2), ("cancun", 1), ("shanghai", 0)):
        if k in name:
            return v
    return 1   # default: pre-Prague (no calldata floor / EIP-7883)

FORK_LEVEL = 1   # set from --fork in main()

def run_cases(cases, timeout, want_root=False):
    """Stream a batch of cases through the runner; return parsed per-case state.
    On timeout, returns None (caller marks the whole batch as timeout)."""
    stream = [FORK_LEVEL, 1 if want_root else 0]   # fork level; then state-root flag
    for c in cases: stream += [1, *encode(c)]
    stream += [0]
    inp = vbytes(stream)
    try:
        out = subprocess.run([BIN], input=inp, capture_output=True, timeout=timeout).stdout.decode()
    except subprocess.TimeoutExpired:
        return None
    cur, results = -1, {}
    for line in out.splitlines():
        p = line.split()
        if not p: continue
        if p[0] == "CASE": cur += 1; results[cur] = {"acc": {}, "sto": {}, "root": None}
        elif p[0] == "ACC" and cur >= 0: results[cur]["acc"][int(p[1])] = (int(p[2]), int(p[3]))
        elif p[0] == "STO" and cur >= 0: results[cur]["sto"][(int(p[1]), int(p[2]))] = int(p[3])
        elif p[0] == "ROOT" and cur >= 0: results[cur]["root"] = int(p[1], 16)
        elif p[0] == "GAS" and cur >= 0: results[cur]["gas"] = int(p[1])
    return results

# minimal EVM disassembler for gas debugging
_OPS = {0:"STOP",1:"ADD",2:"MUL",3:"SUB",4:"DIV",5:"SDIV",6:"MOD",7:"SMOD",8:"ADDMOD",9:"MULMOD",
 10:"EXP",11:"SIGNEXTEND",16:"LT",17:"GT",18:"SLT",19:"SGT",20:"EQ",21:"ISZERO",22:"AND",23:"OR",
 24:"XOR",25:"NOT",26:"BYTE",27:"SHL",28:"SHR",29:"SAR",32:"KECCAK256",48:"ADDRESS",49:"BALANCE",
 50:"ORIGIN",51:"CALLER",52:"CALLVALUE",53:"CALLDATALOAD",54:"CALLDATASIZE",55:"CALLDATACOPY",
 56:"CODESIZE",57:"CODECOPY",58:"GASPRICE",59:"EXTCODESIZE",60:"EXTCODECOPY",61:"RETURNDATASIZE",
 62:"RETURNDATACOPY",63:"EXTCODEHASH",64:"BLOCKHASH",65:"COINBASE",66:"TIMESTAMP",67:"NUMBER",
 68:"PREVRANDAO",69:"GASLIMIT",70:"CHAINID",71:"SELFBALANCE",72:"BASEFEE",73:"BLOBHASH",74:"BLOBBASEFEE",
 80:"POP",81:"MLOAD",82:"MSTORE",83:"MSTORE8",84:"SLOAD",85:"SSTORE",86:"JUMP",87:"JUMPI",88:"PC",
 89:"MSIZE",90:"GAS",91:"JUMPDEST",92:"TLOAD",93:"TSTORE",94:"MCOPY",240:"CREATE",241:"CALL",
 242:"CALLCODE",243:"RETURN",244:"DELEGATECALL",245:"CREATE2",250:"STATICCALL",253:"REVERT",
 254:"INVALID",255:"SELFDESTRUCT"}
def disasm(codehex):
    b = hexbytes(codehex); out=[]; i=0
    while i < len(b):
        op=b[i]
        if 0x60<=op<=0x7f:
            n=op-0x5f; out.append(f"PUSH{n} 0x{b[i+1:i+1+n].hex()}"); i+=1+n
        elif 0x80<=op<=0x8f: out.append(f"DUP{op-0x7f}"); i+=1
        elif 0x90<=op<=0x9f: out.append(f"SWAP{op-0x8f}"); i+=1
        elif 0xa0<=op<=0xa4: out.append(f"LOG{op-0xa0}"); i+=1
        else: out.append(_OPS.get(op,f"0x{op:02x}")); i+=1
    return out

def expected_gas(c, post_accounts):
    """gas_used inferred from the coinbase fee: (post-pre coinbase bal)/priority_fee."""
    env=c["env"]; cb=ai(env["currentCoinbase"]); base=h2i(env.get("currentBaseFee","0x0"))
    tx=c["tx"]; gp=h2i(tx["gasPrice"]) if "gasPrice" in tx else min(h2i(tx.get("maxFeePerGas","0x0")),base+h2i(tx.get("maxPriorityFeePerGas","0x0")))
    prio=gp-base
    if prio<=0: return None
    pre_cb=h2i(c["pre"].get(env["currentCoinbase"],{}).get("balance","0x0"))
    # find coinbase in post.state
    for a,acc in c["post"].items():
        if ai(a)==cb and "balance" in acc:
            return (h2i(acc["balance"])-pre_cb)//prio
    return None

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+"); ap.add_argument("--fork", default=None)
    ap.add_argument("--limit", type=int, default=0); ap.add_argument("--rebuild", action="store_true")
    ap.add_argument("--verbose", action="store_true"); ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--quiet", action="store_true", help="only print failures + summary")
    ap.add_argument("--root", action="store_true", help="also compute+check the state root (slow)")
    ap.add_argument("--gasdbg", action="store_true", help="for balance fails: show gas delta + disasm")
    args = ap.parse_args()
    global FORK_LEVEL; FORK_LEVEL = fork_level(args.fork)
    build_runner(args.rebuild)

    # expand directories to the .json files within (recursive), like run_witness.py
    import glob as _glob
    files = []
    for p in args.files:
        if os.path.isdir(p):
            files += sorted(_glob.glob(os.path.join(p, "**", "*.json"), recursive=True))
        else:
            files.append(p)

    npass = ntotal = ntimeout = nroot = nroot_have = 0
    fail_reasons = {}   # coarse category -> count
    for f in files:
        allcases = collect(f, args.fork, args.limit)
        if not allcases: continue
        # Run cases in small chunks so one slow file's timeout doesn't take down a
        # whole 100+-case batch: the timeout is per-chunk, not per-file (a single
        # pathological file otherwise dominates the timeout count).
        cases = []; results = {}
        for ci in range(0, len(allcases), 8):
            chunk = allcases[ci:ci + 8]
            rr = run_cases(chunk, args.timeout, want_root=args.root)
            if rr is None:
                ntimeout += len(chunk); ntotal += len(chunk)
                print(f"TIMEOUT ({len(chunk)} cases) {os.path.basename(f)}")
                continue
            base = len(cases)
            for n in range(len(chunk)): results[base + n] = rr.get(n, {"acc": {}, "sto": {}, "root": None})
            cases.extend(chunk)
        for n, c in enumerate(cases):
            ntotal += 1
            r = results.get(n, {"acc": {}, "sto": {}, "root": None})
            fails = compare(c["post"], r["acc"], r["sto"])
            # state-root check (post-state authentication)
            if args.root and c.get("hash") and r["root"] is not None:
                nroot_have += 1
                if r["root"] == h2i(c["hash"]): nroot += 1
                elif args.verbose: print(f"      ROOT {hex(r['root'])[:14]}.. != {c['hash'][:14]}..")
            if not fails:
                npass += 1
                if not args.quiet: print(f"PASS {c['cid']}")
            else:
                cat = "storage" if any("[" in x for x in fails) else ("balance" if any("bal" in x for x in fails) else "nonce")
                fail_reasons[cat] = fail_reasons.get(cat, 0) + 1
                print(f"FAIL {c['cid']}")
                if args.verbose:
                    for fdesc in fails[:6]: print("      " + fdesc)
                if args.gasdbg and cat == "balance":
                    tx = c["tx"]; sender = ai(tx["sender"])
                    base = h2i(c["env"].get("currentBaseFee", "0x0"))
                    gp = h2i(tx["gasPrice"]) if "gasPrice" in tx else min(
                        h2i(tx.get("maxFeePerGas","0x0")), base + h2i(tx.get("maxPriorityFeePerGas","0x0")))
                    want_bal = next((h2i(acc["balance"]) for a, acc in c["post"].items()
                                     if ai(a) == sender and "balance" in acc), None)
                    got_bal = r["acc"].get(sender, (0, 0))[1]
                    if want_bal is not None and gp > 0:
                        # +ve delta = we over-charged gas (sender kept less than expected)
                        print(f"      gasdelta={(want_bal - got_bal)//gp} got_gas={r.get('gas')}")
                    to = tx.get("to")
                    code = c["pre"].get(to, {}).get("code", "0x") if to else "0x"
                    if code != "0x": print("      code: " + " ".join(disasm(code)[:20]))
    print(f"\n=== {npass}/{ntotal} passed ({ntimeout} timeouts) ===")
    print(f"=== state-root matches: {nroot}/{nroot_have} ===")
    if fail_reasons: print("fail categories:", dict(sorted(fail_reasons.items(), key=lambda x:-x[1])))

if __name__ == "__main__":
    main()
