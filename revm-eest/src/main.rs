//! Parallel EEST state-test runner for the evm-sail Sail EVM model.
//!
//! Parses execution-spec-tests state fixtures (self-contained schema below),
//! encodes each case into the model binary's stdin varint byte stream (matching
//! evm/runner.sail and run_eest.py), runs the model across all cores,
//! and compares the dumped post-state. The model binary is the Sail EVM
//! compiled to native C; this runner only drives it in parallel.
//!
//! Usage: revm-eest --bin <model-bin> [--fork Cancun] [--timeout 20] <files-or-dirs...>

use alloy_primitives::{Address, Bytes, B256, U256};
use rayon::prelude::*;
use serde::{Deserialize, Deserializer};
use std::collections::{BTreeMap, HashMap};
use std::io::{Read, Write};
use std::process::{Command, Stdio};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::mpsc;
use std::time::Duration;

/* ----------------------------- fixture schema ---------------------------- */
/* The execution-spec-tests state-test JSON shape (the same fields revm's
 * statetest-types reads), defined locally so the runner is dependency-light. */

type AddressMap<T> = HashMap<Address, T>;

#[derive(Deserialize)]
struct Unit {
    env: Env,
    pre: AddressMap<AccountInfo>,
    transaction: TransactionParts,
    post: BTreeMap<String, Vec<PostEntry>>,
}

#[derive(Deserialize)]
struct PostEntry {
    indexes: TxPartIndices,
    #[serde(default)]
    state: AddressMap<AccountInfo>,
}

#[derive(Deserialize)]
struct TxPartIndices {
    data: usize,
    gas: usize,
    value: usize,
}

#[derive(Deserialize, Default)]
#[serde(rename_all = "camelCase")]
struct AccountInfo {
    #[serde(default)]
    balance: U256,
    #[serde(default)]
    code: Bytes,
    #[serde(default)]
    nonce: U256,
    #[serde(default)]
    storage: HashMap<U256, U256>,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct Env {
    current_coinbase: Address,
    #[serde(default)]
    current_difficulty: U256,
    #[allow(dead_code)]
    current_gas_limit: U256,
    current_number: U256,
    current_timestamp: U256,
    #[serde(default)]
    current_base_fee: Option<U256>,
    #[serde(default)]
    current_random: Option<B256>,
    #[serde(default)]
    current_excess_blob_gas: Option<U256>,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct TransactionParts {
    #[serde(default)]
    data: Vec<Bytes>,
    gas_limit: Vec<U256>,
    #[serde(default)]
    gas_price: Option<U256>,
    #[serde(default)]
    nonce: U256,
    #[serde(default)]
    sender: Option<Address>,
    #[serde(default, deserialize_with = "maybe_empty_addr")]
    to: Option<Address>,
    value: Vec<U256>,
    #[serde(default)]
    max_fee_per_gas: Option<U256>,
    #[serde(default)]
    max_priority_fee_per_gas: Option<U256>,
    #[serde(default)]
    max_fee_per_blob_gas: Option<U256>,
    #[serde(default)]
    access_lists: Vec<Option<Vec<AccessItem>>>,
    #[serde(default)]
    authorization_list: Option<Vec<TestAuthorization>>,
    #[serde(default)]
    blob_versioned_hashes: Vec<B256>,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct AccessItem {
    address: Address,
    #[serde(default)]
    storage_keys: Vec<B256>,
}

/// EIP-7702 authorization tuple as fixtures carry it. The fixture provides the
/// recovered `signer`; validity mirrors run_eest.py (EIP-2 s-bound, r-range,
/// y-parity, nonce bound) -- no in-runner signature recovery needed.
#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct TestAuthorization {
    #[serde(default)]
    chain_id: U256,
    address: Address,
    #[serde(default)]
    nonce: U256,
    #[serde(default)]
    v: Option<U256>,
    #[serde(default)]
    y_parity: Option<U256>,
    #[serde(default)]
    r: U256,
    #[serde(default)]
    s: U256,
    #[serde(default)]
    signer: Option<Address>,
}

/// `to` may be "" (contract creation) or absent.
fn maybe_empty_addr<'de, D: Deserializer<'de>>(d: D) -> Result<Option<Address>, D::Error> {
    let s: Option<String> = Option::deserialize(d)?;
    match s {
        Some(x) if !x.is_empty() => x.parse().map(Some).map_err(serde::de::Error::custom),
        _ => Ok(None),
    }
}

/* ------------------------------- encoding -------------------------------- */

fn addr_int(a: &Address) -> U256 {
    U256::from_be_slice(a.as_slice())
}
fn b256_int(b: &B256) -> U256 {
    U256::from_be_slice(b.as_slice())
}

/// One concrete (test, fork, tx-index) case, pre-encoded into the int-stream.
struct Case {
    cid: String,
    encoded: Vec<String>,
    post: Vec<(U256, Option<u64>, Option<U256>, Vec<(U256, U256)>)>, // (addr, nonce?, balance?, storage)
}

fn push_u256(s: &mut Vec<String>, v: U256) {
    s.push(v.to_string());
}

/// Encode one case the way runner.sail reads it (see run_eest.py::encode).
fn encode(unit: &Unit, entry: &PostEntry) -> Option<Vec<String>> {
    let mut s: Vec<String> = Vec::new();
    s.push(unit.pre.len().to_string());
    for (addr, acc) in unit.pre.iter() {
        push_u256(&mut s, addr_int(addr));
        s.push(acc.nonce.to_string());
        push_u256(&mut s, acc.balance);
        let code: &[u8] = acc.code.as_ref();
        s.push(code.len().to_string());
        for b in code {
            s.push(b.to_string());
        }
        s.push(acc.storage.len().to_string());
        for (k, v) in acc.storage.iter() {
            push_u256(&mut s, *k);
            push_u256(&mut s, *v);
        }
    }
    let env = &unit.env;
    let base = env.current_base_fee.unwrap_or(U256::ZERO);
    push_u256(&mut s, env.current_number);
    push_u256(&mut s, env.current_timestamp);
    push_u256(&mut s, env.current_gas_limit);
    push_u256(&mut s, addr_int(&env.current_coinbase));
    push_u256(&mut s, base);
    let prevrandao = env
        .current_random
        .map(|r| b256_int(&r))
        .unwrap_or(env.current_difficulty);
    push_u256(&mut s, prevrandao);
    s.push("1".into()); // chain id
    // EIP-4844 blob gas price = fake_exponential(1, excess_blob_gas, 3338477)
    let excess = env.current_excess_blob_gas.unwrap_or(U256::ZERO);
    push_u256(&mut s, blob_gas_price(excess));

    let tx = &unit.transaction;
    let idx = &entry.indexes;
    let gp = if let Some(g) = tx.gas_price {
        g
    } else {
        let mf = tx.max_fee_per_gas.unwrap_or(U256::ZERO);
        let mp = tx.max_priority_fee_per_gas.unwrap_or(U256::ZERO);
        std::cmp::min(mf, base + mp)
    };
    let is_create = tx.to.is_none();
    let data = tx.data.get(idx.data)?;
    let gas = *tx.gas_limit.get(idx.gas)?;
    let value = *tx.value.get(idx.value)?;
    push_u256(&mut s, addr_int(&tx.sender.unwrap_or_default()));
    push_u256(&mut s, tx.nonce);
    push_u256(&mut s, gp);
    push_u256(&mut s, gp.saturating_sub(base)); // priority fee
    push_u256(&mut s, gas);
    s.push(if is_create { "1" } else { "0" }.into());
    if is_create {
        s.push("0".into());
    } else {
        push_u256(&mut s, addr_int(&tx.to.unwrap()));
    }
    push_u256(&mut s, value);
    s.push(data.len().to_string());
    for b in data.iter() {
        s.push(b.to_string());
    }

    // EIP-2930 access list (for the selected data index)
    let mut addrs: Vec<U256> = Vec::new();
    let mut keys: Vec<(U256, U256)> = Vec::new();
    if let Some(Some(al)) = tx.access_lists.get(idx.data) {
        for item in al.iter() {
            addrs.push(addr_int(&item.address));
            for k in item.storage_keys.iter() {
                keys.push((addr_int(&item.address), b256_int(k)));
            }
        }
    }
    s.push(addrs.len().to_string());
    for a in &addrs {
        push_u256(&mut s, *a);
    }
    s.push(keys.len().to_string());
    for (a, k) in &keys {
        push_u256(&mut s, *a);
        push_u256(&mut s, *k);
    }
    // EIP-4844 num blobs; then fee caps (max_fee_per_gas, max_fee_per_blob_gas) for validity
    s.push(tx.blob_versioned_hashes.len().to_string());
    let max_fee = tx.gas_price.unwrap_or(tx.max_fee_per_gas.unwrap_or(U256::ZERO));
    push_u256(&mut s, max_fee);
    push_u256(&mut s, tx.max_fee_per_blob_gas.unwrap_or(U256::ZERO));
    // EIP-1559 priority-fee cap (gasPrice for legacy txs), for validity
    push_u256(
        &mut s,
        tx.max_priority_fee_per_gas
            .unwrap_or(tx.gas_price.unwrap_or(U256::ZERO)),
    );
    // EIP-7702 type-4 envelope flag (an EMPTY auth list is still a type-4 tx -> invalid)
    s.push(if tx.authorization_list.is_some() { "1" } else { "0" }.into());
    // EIP-7702 authorization list: each (valid_sig, authority, delegate, nonce, chainId).
    // The fixture pre-recovers the authority into `signer`; validity mirrors
    // run_eest.py: signer present, EIP-2 1<=s<=n/2, 1<=r<n, yParity in {0,1},
    // auth nonce < 2^64-1.
    let auths: &[TestAuthorization] = tx.authorization_list.as_deref().unwrap_or(&[]);
    s.push(auths.len().to_string());
    let half_n = U256::from_be_bytes(SECP256K1_HALF_N);
    let full_n = U256::from_be_bytes(SECP256K1_N);
    for a in auths {
        let yp = a.y_parity.or(a.v).unwrap_or(U256::ZERO);
        let valid = a.signer.is_some()
            && a.s >= U256::from(1)
            && a.s <= half_n
            && a.r >= U256::from(1)
            && a.r < full_n
            && (yp == U256::ZERO || yp == U256::from(1))
            && a.nonce < U256::from(u64::MAX);
        s.push(if valid { "1" } else { "0" }.into());
        push_u256(&mut s, addr_int(&a.signer.unwrap_or_default()));
        push_u256(&mut s, addr_int(&a.address));
        push_u256(&mut s, a.nonce);
        push_u256(&mut s, a.chain_id);
    }
    // EIP-4844 blob section: is_blob flag (type-3 marker), then the actual 32-byte
    // versioned hashes as decimal words (BLOBHASH operands + version validity).
    // Must stay in sync with run_eest.py::encode and runner.sail.
    let is_blob = tx.max_fee_per_blob_gas.is_some() || !tx.blob_versioned_hashes.is_empty();
    s.push(if is_blob { "1" } else { "0" }.into());
    s.push(tx.blob_versioned_hashes.len().to_string());
    for h in tx.blob_versioned_hashes.iter() {
        push_u256(&mut s, b256_int(h));
    }
    Some(s)
}

/// secp256k1n and secp256k1n/2 (EIP-2 upper bound on signature s).
const SECP256K1_N: [u8; 32] = [
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b, 0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41,
];
const SECP256K1_HALF_N: [u8; 32] = [
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x5d, 0x57, 0x6e, 0x73, 0x57, 0xa4, 0x50, 0x1d, 0xdf, 0xe9, 0x2f, 0x46, 0x68, 0x1b, 0x20, 0xa0,
];

/// EIP-4844 fake_exponential(MIN_BLOB_GASPRICE=1, excess, BLOB_BASE_FEE_UPDATE_FRACTION=3338477).
fn blob_gas_price(excess: U256) -> U256 {
    let denom = U256::from(3338477u64);
    let mut i = U256::from(1u64);
    let mut output = U256::ZERO;
    let mut numerator_accum = denom; // factor(=1) * denominator
    while numerator_accum > U256::ZERO {
        output += numerator_accum;
        numerator_accum = numerator_accum * excess / (denom * i);
        i += U256::from(1u64);
    }
    output / denom
}

fn expected_post(ps: &AddressMap<AccountInfo>) -> Vec<(U256, Option<u64>, Option<U256>, Vec<(U256, U256)>)> {
    ps.iter()
        .map(|(addr, acc)| {
            let storage: Vec<(U256, U256)> = acc.storage.iter().map(|(k, v)| (*k, *v)).collect();
            let nonce: u64 = acc.nonce.try_into().unwrap_or(u64::MAX);
            (addr_int(addr), Some(nonce), Some(acc.balance), storage)
        })
        .collect()
}

/* ------------------------------- execution ------------------------------- */

/// Chronological fork level for fork-gated gas rules (EIP-7623 / EIP-7883), set from --fork.
static FORK_LEVEL: std::sync::atomic::AtomicI64 = std::sync::atomic::AtomicI64::new(1);
fn fork_level(name: &str) -> i64 {
    let n = name.to_lowercase();
    for (k, v) in [("osaka", 3), ("prague", 2), ("cancun", 1), ("shanghai", 0)] {
        if n.contains(k) {
            return v;
        }
    }
    1
}

/// Run a chunk of cases through one model invocation; parse the per-case dump.
/// Returns None on timeout. Output: CASE / ACC <addr> <nonce> <bal> / STO <addr> <slot> <val>.
fn run_chunk(bin: &str, chunk: &[&Case], timeout: f64) -> Option<Vec<CaseOut>> {
    let mut stream = vec![
        FORK_LEVEL.load(Ordering::Relaxed).to_string(), // fork level
        "0".to_string(),                                // no state-root
    ];
    for c in chunk {
        stream.push("1".into());
        stream.extend(c.encoded.iter().cloned());
    }
    stream.push("0".into());
    // varint byte stream: each decimal token -> [len][big-endian bytes] (matches
    // run_eest.py vbytes() + runner.sail ni()). The model reads it via ssz_src.
    let mut input: Vec<u8> = Vec::new();
    for tok in &stream {
        let v = U256::from_str_radix(tok, 10).unwrap_or(U256::ZERO);
        let be = v.to_be_bytes::<32>();
        let first = be.iter().position(|&b| b != 0).unwrap_or(32);
        let body = &be[first..];
        input.push(body.len() as u8);
        input.extend_from_slice(body);
    }

    let mut child = Command::new(bin)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::null())
        .spawn()
        .ok()?;
    let mut stdin = child.stdin.take().unwrap();
    std::thread::spawn(move || {
        let _ = stdin.write_all(&input);
    });
    let mut stdout = child.stdout.take().unwrap();
    let (tx, rx) = mpsc::channel();
    std::thread::spawn(move || {
        let mut out = String::new();
        let _ = stdout.read_to_string(&mut out);
        let _ = tx.send(out);
    });
    let out = match rx.recv_timeout(Duration::from_secs_f64(timeout)) {
        Ok(o) => {
            let _ = child.wait();
            o
        }
        Err(_) => {
            let _ = child.kill();
            let _ = child.wait();
            return None;
        }
    };

    let mut results: Vec<CaseOut> = Vec::new();
    for line in out.lines() {
        let p: Vec<&str> = line.split_whitespace().collect();
        match p.first().copied() {
            Some("CASE") => results.push(CaseOut::default()),
            Some("ACC") if !results.is_empty() => {
                if let (Ok(a), Ok(n), Ok(b)) = (p[1].parse(), p[2].parse::<u64>(), p[3].parse::<U256>()) {
                    results.last_mut().unwrap().acc.insert(a, (n, b));
                }
            }
            Some("STO") if !results.is_empty() => {
                if let (Ok(a), Ok(k), Ok(v)) = (p[1].parse::<U256>(), p[2].parse::<U256>(), p[3].parse::<U256>()) {
                    results.last_mut().unwrap().sto.insert((a, k), v);
                }
            }
            _ => {}
        }
    }
    Some(results)
}

#[derive(Default)]
struct CaseOut {
    acc: BTreeMap<U256, (u64, U256)>,
    sto: HashMap<(U256, U256), U256>,
}

fn compare(case: &Case, out: &CaseOut) -> Option<String> {
    for (addr, nonce, bal, storage) in &case.post {
        let got = out.acc.get(addr).cloned().unwrap_or((0, U256::ZERO));
        if let Some(n) = nonce {
            if got.0 != *n {
                return Some(format!("nonce {:#x} {}!={}", addr, got.0, n));
            }
        }
        if let Some(b) = bal {
            if got.1 != *b {
                return Some("balance".into());
            }
        }
        for (k, v) in storage {
            let g = out.sto.get(&(*addr, *k)).copied().unwrap_or(U256::ZERO);
            if g != *v {
                return Some("storage".into());
            }
        }
    }
    None
}

fn collect_files(args: &[String]) -> Vec<String> {
    let mut files = Vec::new();
    for a in args {
        let p = std::path::Path::new(a);
        if p.is_dir() {
            for e in walkdir(p) {
                if e.extension().and_then(|x| x.to_str()) == Some("json") {
                    files.push(e.to_string_lossy().to_string());
                }
            }
        } else {
            files.push(a.clone());
        }
    }
    files
}
fn walkdir(p: &std::path::Path) -> Vec<std::path::PathBuf> {
    let mut out = Vec::new();
    if let Ok(rd) = std::fs::read_dir(p) {
        for e in rd.flatten() {
            let path = e.path();
            if path.is_dir() {
                out.extend(walkdir(&path));
            } else {
                out.push(path);
            }
        }
    }
    out
}

fn main() {
    let argv: Vec<String> = std::env::args().skip(1).collect();
    let mut bin = String::new();
    let mut fork: Option<String> = None;
    let mut timeout = 20.0f64;
    let mut files: Vec<String> = Vec::new();
    let mut it = argv.iter();
    while let Some(a) = it.next() {
        match a.as_str() {
            "--bin" => bin = it.next().cloned().unwrap_or_default(),
            "--fork" => fork = it.next().cloned(),
            "--timeout" => timeout = it.next().and_then(|x| x.parse().ok()).unwrap_or(20.0),
            _ => files.push(a.clone()),
        }
    }
    if bin.is_empty() {
        eprintln!("usage: revm-eest --bin <model> [--fork F] [--timeout N] <files/dirs>");
        std::process::exit(2);
    }
    FORK_LEVEL.store(fork_level(fork.as_deref().unwrap_or("")), Ordering::Relaxed);
    let files = collect_files(&files);

    // Parse all fixtures and flatten into cases (in parallel).
    let cases: Vec<Case> = files
        .par_iter()
        .flat_map(|f| {
            let mut v = Vec::new();
            let data = match std::fs::read(f) {
                Ok(d) => d,
                Err(_) => return v,
            };
            let suite: BTreeMap<String, Unit> = match serde_json::from_slice(&data) {
                Ok(s) => s,
                Err(_) => return v,
            };
            for (name, unit) in &suite {
                for (spec_name, entries) in &unit.post {
                    if let Some(ref want) = fork {
                        if spec_name != want {
                            continue;
                        }
                    }
                    for entry in entries {
                        if let Some(encoded) = encode(unit, entry) {
                            v.push(Case {
                                cid: format!("{name}|{spec_name}"),
                                encoded,
                                post: expected_post(&entry.state),
                            });
                        }
                    }
                }
            }
            v
        })
        .collect();

    let total = cases.len();
    let refs: Vec<&Case> = cases.iter().collect();
    let chunks: Vec<&[&Case]> = refs.chunks(8).collect();

    let npass = AtomicUsize::new(0);
    let ntimeout = AtomicUsize::new(0);
    let nbal = AtomicUsize::new(0);
    let nsto = AtomicUsize::new(0);
    let nnonce = AtomicUsize::new(0);

    let score = |c: &Case, out: Option<&CaseOut>| {
        let empty = CaseOut::default();
        match compare(c, out.unwrap_or(&empty)) {
            None => {
                npass.fetch_add(1, Ordering::Relaxed);
            }
            Some(reason) => {
                eprintln!("FAIL {} :: {}", c.cid, reason);
                if reason.starts_with("storage") {
                    nsto.fetch_add(1, Ordering::Relaxed);
                } else if reason.starts_with("balance") {
                    nbal.fetch_add(1, Ordering::Relaxed);
                } else {
                    nnonce.fetch_add(1, Ordering::Relaxed);
                }
            }
        }
    };

    chunks.par_iter().for_each(|chunk| match run_chunk(&bin, chunk, timeout) {
        None => {
            // chunk-level timeout: re-run one case at a time and SCORE each
            // individually -- a chunk of heavy-but-passing cases can exceed the
            // budget collectively while every case finishes alone.
            for c in chunk.iter() {
                match run_chunk(&bin, &[c], timeout) {
                    None => {
                        eprintln!("TIMEOUT {}", c.cid);
                        ntimeout.fetch_add(1, Ordering::Relaxed);
                    }
                    Some(outs) => score(c, outs.first()),
                }
            }
        }
        Some(outs) => {
            for (i, c) in chunk.iter().enumerate() {
                score(c, outs.get(i));
            }
        }
    });

    let p = npass.load(Ordering::Relaxed);
    let to = ntimeout.load(Ordering::Relaxed);
    println!("=== {p}/{total} passed ({to} timeouts) ===");
    println!(
        "fail categories: {{balance: {}, storage: {}, nonce: {}}}",
        nbal.load(Ordering::Relaxed),
        nsto.load(Ordering::Relaxed),
        nnonce.load(Ordering::Relaxed)
    );
}
