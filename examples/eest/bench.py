#!/usr/bin/env python3
"""Broadened gas-throughput benchmark: gas-heavy EEST fixtures across workload
classes (storage loops, deep calls, memory ops, creates, precompiles, jumps).
Usage: bench.py <model-bin> <fixtures-root> [fork-level]"""
import sys, time, subprocess, json
import run_eest

BIN = sys.argv[1]
FX = sys.argv[2]

# (label, fork-level, fork-name, path) — chosen for high gas + PASSING status
CASES = [
    ("TSTORE loop (jump/push/store)", 1, "Cancun",  "cancun/eip1153_tstore/test_run_until_out_of_gas.json"),
    ("deep reentrant CALLs",          1, "Cancun",  "cancun/eip1153_tstore/test_tstore_reentrancy.json"),
    ("subcall contexts (16.7M gas)",  3, "Osaka",   "osaka/eip7825_transaction_gas_limit_cap/test_tx_gas_limit_cap_subcall_context.json"),
    ("SSTORE refund churn",           3, "Osaka",   "osaka/eip7825_transaction_gas_limit_cap/test_maximum_gas_refund.json"),
    ("MCOPY memory ops",              1, "Cancun",  "cancun/eip5656_mcopy/test_mcopy_memory_expansion.json"),
    ("CREATE w/ 49KB initcode",       0, "Shanghai","shanghai/eip3860_initcode/test_create_opcode_initcode.json"),
    ("blake2f precompile rounds",     1, "Cancun",  "istanbul/eip152_blake2/test_blake2.json"),
]

total_g, total_t = 0, 0.0
for label, lvl, fork, rel in CASES:
    try:
        cases = run_eest.collect(f"{FX}/{rel}", fork, 0)
    except FileNotFoundError:
        print(f"{label:34s} SKIP (missing)")
        continue
    run_eest.FORK_LEVEL = lvl
    tg, tt = 0, 0.0
    for c in cases:
        inp = " ".join(map(str, [lvl, 0, 1, *run_eest.encode(c), 0]))
        t0 = time.time()
        out = subprocess.run([BIN], input=inp, capture_output=True, text=True, timeout=300)
        tt += time.time() - t0
        for l in out.stdout.splitlines():
            if l.startswith("GAS"):
                tg += int(l.split()[1])
    total_g += tg; total_t += tt
    print(f"{label:34s} {tg/1e6:9.1f} Mgas {tt:7.2f}s  {tg/tt/1e6:7.2f} Mgas/s")
print(f"{'TOTAL':34s} {total_g/1e6:9.1f} Mgas {total_t:7.2f}s  {total_g/total_t/1e6:7.2f} Mgas/s")
