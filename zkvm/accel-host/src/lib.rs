//! Host-side reference implementation of the eth-act zkvm-standards crypto
//! accelerators (`ffi/zkvm_accelerators.h`), backed by the same industry crates
//! revm/reth use. Exposes the standard C ABI; a real zkVM host swaps in these (or
//! its native precompiles) behind the unchanged header. Returns ZKVM_EOK(0) /
//! ZKVM_EFAIL(-1). Pointers are caller-allocated and assumed valid (NULL panics
//! per the standard's usage notes).
#![allow(non_camel_case_types)]

use ripemd::Ripemd160;
use sha2::Sha256;
use sha3::{Digest, Keccak256};

const ZKVM_EOK: i32 = 0;
const ZKVM_EFAIL: i32 = -1;

#[inline]
unsafe fn slice<'a>(p: *const u8, len: usize) -> &'a [u8] {
    if len == 0 { &[] } else { core::slice::from_raw_parts(p, len) }
}

/* ============================== Hashes ================================= */

#[no_mangle]
pub unsafe extern "C" fn zkvm_keccak256(data: *const u8, len: usize, output: *mut u8) -> i32 {
    let h = Keccak256::digest(slice(data, len));
    core::ptr::copy_nonoverlapping(h.as_ptr(), output, 32);
    ZKVM_EOK
}

#[no_mangle]
pub unsafe extern "C" fn zkvm_sha256(data: *const u8, len: usize, output: *mut u8) -> i32 {
    let h = Sha256::digest(slice(data, len));
    core::ptr::copy_nonoverlapping(h.as_ptr(), output, 32);
    ZKVM_EOK
}

#[no_mangle]
pub unsafe extern "C" fn zkvm_ripemd160(data: *const u8, len: usize, output: *mut u8) -> i32 {
    let h = Ripemd160::digest(slice(data, len));
    core::ptr::write_bytes(output, 0, 12); // 20-byte digest left-padded to 32
    core::ptr::copy_nonoverlapping(h.as_ptr(), output.add(12), 20);
    ZKVM_EOK
}

/* ============================ ecrecover =============================== */

#[no_mangle]
pub unsafe extern "C" fn zkvm_secp256k1_ecrecover(
    msg: *const u8,
    sig: *const u8,
    recid: u8,
    output: *mut u8,
) -> i32 {
    use k256::ecdsa::{RecoveryId, Signature, VerifyingKey};
    let msg = slice(msg, 32);
    let sigb = slice(sig, 64);
    let signature = match Signature::from_slice(sigb) {
        Ok(s) => s,
        Err(_) => return ZKVM_EFAIL,
    };
    let rid = match RecoveryId::from_byte(recid) {
        Some(r) => r,
        None => return ZKVM_EFAIL,
    };
    let vk = match VerifyingKey::recover_from_prehash(msg, &signature, rid) {
        Ok(v) => v,
        Err(_) => return ZKVM_EFAIL,
    };
    // standard output is the 64-byte uncompressed pubkey X||Y (no 0x04 prefix)
    let pt = vk.to_encoded_point(false);
    let bytes = pt.as_bytes();
    if bytes.len() != 65 {
        return ZKVM_EFAIL;
    }
    core::ptr::copy_nonoverlapping(bytes.as_ptr().add(1), output, 64);
    ZKVM_EOK
}

/* ============================== modexp ================================ */

#[no_mangle]
pub unsafe extern "C" fn zkvm_modexp(
    base: *const u8,
    base_len: usize,
    exp: *const u8,
    exp_len: usize,
    modulus: *const u8,
    mod_len: usize,
    output: *mut u8,
) -> i32 {
    let b = slice(base, base_len);
    let e = slice(exp, exp_len);
    let m = slice(modulus, mod_len);
    let r = aurora_engine_modexp::modexp(b, e, m); // big-endian, <= mod_len bytes
    if r.len() > mod_len {
        return ZKVM_EFAIL;
    }
    let off = mod_len - r.len();
    core::ptr::write_bytes(output, 0, off); // left-pad to exactly mod_len
    core::ptr::copy_nonoverlapping(r.as_ptr(), output.add(off), r.len());
    ZKVM_EOK
}

/* ============================== blake2f =============================== */

const BLAKE2_IV: [u64; 8] = [
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
];
const BLAKE2_SIGMA: [[usize; 16]; 10] = [
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
    [14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3],
    [11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4],
    [7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8],
    [9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13],
    [2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9],
    [12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11],
    [13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10],
    [6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5],
    [10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0],
];

#[inline]
fn blake2_g(v: &mut [u64; 16], a: usize, b: usize, c: usize, d: usize, x: u64, y: u64) {
    v[a] = v[a].wrapping_add(v[b]).wrapping_add(x);
    v[d] = (v[d] ^ v[a]).rotate_right(32);
    v[c] = v[c].wrapping_add(v[d]);
    v[b] = (v[b] ^ v[c]).rotate_right(24);
    v[a] = v[a].wrapping_add(v[b]).wrapping_add(y);
    v[d] = (v[d] ^ v[a]).rotate_right(16);
    v[c] = v[c].wrapping_add(v[d]);
    v[b] = (v[b] ^ v[c]).rotate_right(63);
}

/// EIP-152 BLAKE2 F compression. h: 8x u64 LE (in/out), m: 16x u64 LE, t: 2x u64 LE.
#[no_mangle]
pub unsafe extern "C" fn zkvm_blake2f(
    rounds: u32,
    h: *mut u8,
    m: *const u8,
    t: *const u8,
    f: u8,
) -> i32 {
    let mut hv = [0u64; 8];
    for i in 0..8 {
        hv[i] = u64::from_le_bytes(core::slice::from_raw_parts(h.add(i * 8), 8).try_into().unwrap());
    }
    let mut mv = [0u64; 16];
    for i in 0..16 {
        mv[i] = u64::from_le_bytes(core::slice::from_raw_parts(m.add(i * 8), 8).try_into().unwrap());
    }
    let t0 = u64::from_le_bytes(core::slice::from_raw_parts(t, 8).try_into().unwrap());
    let t1 = u64::from_le_bytes(core::slice::from_raw_parts(t.add(8), 8).try_into().unwrap());

    let mut v = [0u64; 16];
    v[..8].copy_from_slice(&hv);
    v[8..].copy_from_slice(&BLAKE2_IV);
    v[12] ^= t0;
    v[13] ^= t1;
    if f != 0 {
        v[14] ^= u64::MAX;
    }
    for r in 0..rounds as usize {
        let s = &BLAKE2_SIGMA[r % 10];
        blake2_g(&mut v, 0, 4, 8, 12, mv[s[0]], mv[s[1]]);
        blake2_g(&mut v, 1, 5, 9, 13, mv[s[2]], mv[s[3]]);
        blake2_g(&mut v, 2, 6, 10, 14, mv[s[4]], mv[s[5]]);
        blake2_g(&mut v, 3, 7, 11, 15, mv[s[6]], mv[s[7]]);
        blake2_g(&mut v, 0, 5, 10, 15, mv[s[8]], mv[s[9]]);
        blake2_g(&mut v, 1, 6, 11, 12, mv[s[10]], mv[s[11]]);
        blake2_g(&mut v, 2, 7, 8, 13, mv[s[12]], mv[s[13]]);
        blake2_g(&mut v, 3, 4, 9, 14, mv[s[14]], mv[s[15]]);
    }
    for i in 0..8 {
        hv[i] ^= v[i] ^ v[i + 8];
        core::ptr::copy_nonoverlapping(hv[i].to_le_bytes().as_ptr(), h.add(i * 8), 8);
    }
    ZKVM_EOK
}

/* ========================= secp256r1 (P-256) ========================= */

#[no_mangle]
pub unsafe extern "C" fn zkvm_secp256r1_verify(
    msg: *const u8,
    sig: *const u8,
    pubkey: *const u8,
    verified: *mut bool,
) -> i32 {
    use p256::ecdsa::{signature::hazmat::PrehashVerifier, Signature, VerifyingKey};
    use p256::EncodedPoint;
    let msg = slice(msg, 32);
    let signature = match Signature::from_slice(slice(sig, 64)) {
        Ok(s) => s,
        Err(_) => return ZKVM_EFAIL,
    };
    // pubkey is 64 bytes X||Y; prepend 0x04 for an uncompressed encoded point
    let pk = slice(pubkey, 64);
    let mut enc = [0u8; 65];
    enc[0] = 0x04;
    enc[1..].copy_from_slice(pk);
    let point = match EncodedPoint::from_bytes(enc) {
        Ok(p) => p,
        Err(_) => return ZKVM_EFAIL,
    };
    let vk = match VerifyingKey::from_encoded_point(&point) {
        Ok(v) => v,
        Err(_) => return ZKVM_EFAIL,
    };
    *verified = vk.verify_prehash(msg, &signature).is_ok();
    ZKVM_EOK
}

/* =============================== bn254 ================================ */
/* EIP-196/197 over the alt_bn128 curve, via arkworks (as revm). EVM encoding:
 * G1 = x||y (32B BE each); G2 = x_c1||x_c0||y_c1||y_c0 (imaginary part first);
 * (0,0) is the point at infinity. Coordinates must be canonical (< field p). */
mod bn {
    use ark_bn254::{Bn254, Fq, Fq2, Fr, G1Affine, G2Affine};
    use ark_ec::{pairing::Pairing, AffineRepr, CurveGroup};
    use ark_ff::{BigInteger, One, PrimeField, Zero};

    fn read_fq(b: &[u8]) -> Option<Fq> {
        let f = Fq::from_be_bytes_mod_order(b);
        let mut canon = f.into_bigint().to_bytes_be(); // canonical check: re-serialize must match
        while canon.len() < 32 { canon.insert(0, 0); }
        if canon.as_slice() == b { Some(f) } else { None }
    }
    fn write_fq(f: &Fq, out: &mut [u8]) {
        let mut b = f.into_bigint().to_bytes_be();
        while b.len() < 32 { b.insert(0, 0); }
        out.copy_from_slice(&b);
    }
    pub fn read_g1(b: &[u8]) -> Option<G1Affine> {
        let x = read_fq(&b[0..32])?;
        let y = read_fq(&b[32..64])?;
        if x.is_zero() && y.is_zero() { return Some(G1Affine::identity()); }
        let p = G1Affine::new_unchecked(x, y);
        if p.is_on_curve() { Some(p) } else { None }
    }
    pub fn write_g1(p: &G1Affine, out: &mut [u8]) {
        match p.xy() {
            Some((x, y)) => { write_fq(&x, &mut out[0..32]); write_fq(&y, &mut out[32..64]); }
            None => out.iter_mut().for_each(|b| *b = 0), // infinity -> (0,0)
        }
    }
    fn read_g2(b: &[u8]) -> Option<G2Affine> {
        let x = Fq2::new(read_fq(&b[32..64])?, read_fq(&b[0..32])?);   // c0, c1 (imag first)
        let y = Fq2::new(read_fq(&b[96..128])?, read_fq(&b[64..96])?);
        if x.is_zero() && y.is_zero() { return Some(G2Affine::identity()); }
        let p = G2Affine::new_unchecked(x, y);
        if p.is_on_curve() && p.is_in_correct_subgroup_assuming_on_curve() { Some(p) } else { None }
    }

    pub fn g1_add(p1: &[u8], p2: &[u8], out: &mut [u8]) -> bool {
        match (read_g1(p1), read_g1(p2)) {
            (Some(a), Some(b)) => { write_g1(&(a + b).into_affine(), out); true }
            _ => false,
        }
    }
    pub fn g1_mul(p: &[u8], s: &[u8], out: &mut [u8]) -> bool {
        match read_g1(p) {
            Some(a) => { let k = Fr::from_be_bytes_mod_order(s); write_g1(&(a * k).into_affine(), out); true }
            None => false,
        }
    }
    pub fn pairing(pairs: &[u8], n: usize) -> Option<bool> {
        let (mut g1s, mut g2s) = (Vec::with_capacity(n), Vec::with_capacity(n));
        for i in 0..n {
            let off = i * 192;
            g1s.push(read_g1(&pairs[off..off + 64])?);
            g2s.push(read_g2(&pairs[off + 64..off + 192])?);
        }
        Some(Bn254::multi_pairing(g1s, g2s).0.is_one())
    }
}

#[no_mangle]
pub unsafe extern "C" fn zkvm_bn254_g1_add(p1: *const u8, p2: *const u8, result: *mut u8) -> i32 {
    let mut out = [0u8; 64];
    if bn::g1_add(slice(p1, 64), slice(p2, 64), &mut out) {
        core::ptr::copy_nonoverlapping(out.as_ptr(), result, 64); ZKVM_EOK
    } else { ZKVM_EFAIL }
}
#[no_mangle]
pub unsafe extern "C" fn zkvm_bn254_g1_mul(point: *const u8, scalar: *const u8, result: *mut u8) -> i32 {
    let mut out = [0u8; 64];
    if bn::g1_mul(slice(point, 64), slice(scalar, 32), &mut out) {
        core::ptr::copy_nonoverlapping(out.as_ptr(), result, 64); ZKVM_EOK
    } else { ZKVM_EFAIL }
}
#[no_mangle]
pub unsafe extern "C" fn zkvm_bn254_pairing(pairs: *const u8, num_pairs: usize, verified: *mut bool) -> i32 {
    match bn::pairing(slice(pairs, num_pairs * 192), num_pairs) {
        Some(v) => { *verified = v; ZKVM_EOK }
        None => ZKVM_EFAIL,
    }
}

/* ============================ KZG (EIP-4844) ========================= */
#[no_mangle]
pub unsafe extern "C" fn zkvm_kzg_point_eval(
    commitment: *const u8,
    z: *const u8,
    y: *const u8,
    proof: *const u8,
    verified: *mut bool,
) -> i32 {
    use c_kzg::{Bytes32, Bytes48};
    let settings = c_kzg::ethereum_kzg_settings(0);
    let c = match Bytes48::from_bytes(slice(commitment, 48)) { Ok(v) => v, Err(_) => return ZKVM_EFAIL };
    let z = match Bytes32::from_bytes(slice(z, 32)) { Ok(v) => v, Err(_) => return ZKVM_EFAIL };
    let y = match Bytes32::from_bytes(slice(y, 32)) { Ok(v) => v, Err(_) => return ZKVM_EFAIL };
    let p = match Bytes48::from_bytes(slice(proof, 48)) { Ok(v) => v, Err(_) => return ZKVM_EFAIL };
    match settings.verify_kzg_proof(&c, &z, &y, &p) {
        Ok(v) => { *verified = v; ZKVM_EOK }
        Err(_) => ZKVM_EFAIL,
    }
}

/* ========================= BLS12-381 (EIP-2537) ====================== */
/* via blst. Standard encoding here = blst native serialization: G1 = 96 bytes
 * (x||y, 48 each BE), G2 = 192 bytes, Fp = 48, Fp2 = 96, scalar = 32. acc_shim
 * maps the EIP-2537 padded layout (64-byte field elems) to/from this. */
mod bls {
    use blst::*;
    /* EIP-2537 encodes the point at infinity as all-zero bytes, but blst's native
     * (de)serialization uses an infinity FLAG bit and rejects all-zero input. So
     * map all-zero <-> infinity here; `subgroup` adds the G1/G2 membership check
     * required by MSM/PAIRING (but not by ADD: a point on the curve but outside
     * the subgroup is a VALID ADD input, and blst_p1_deserialize would reject it
     * with BLST_POINT_NOT_IN_GROUP -- so read the raw coordinates instead). */
    const FP_MOD: [u8; 48] = [
        0x1a, 0x01, 0x11, 0xea, 0x39, 0x7f, 0xe6, 0x9a, 0x4b, 0x1b, 0xa7, 0xb6,
        0x43, 0x4b, 0xac, 0xd7, 0x64, 0x77, 0x4b, 0x84, 0xf3, 0x85, 0x12, 0xbf,
        0x67, 0x30, 0xd2, 0xa0, 0xf6, 0xb0, 0xf6, 0x24, 0x1e, 0xab, 0xff, 0xfe,
        0xb1, 0x53, 0xff, 0xff, 0xb9, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xaa, 0xab,
    ];
    /* EIP-2537: every 48-byte BE field element must be canonical (< p). */
    pub fn fp_canonical(b: &[u8]) -> bool {
        b[..48] < FP_MOD[..]
    }
    fn rd_fp(b: &[u8]) -> Option<blst_fp> {
        if !fp_canonical(b) { return None; }
        let mut f = blst_fp::default();
        unsafe { blst_fp_from_bendian(&mut f, b.as_ptr()); }
        Some(f)
    }
    pub fn rd_g1(b: &[u8], subgroup: bool) -> Option<blst_p1_affine> {
        unsafe {
            let mut a = blst_p1_affine::default();
            a.x = rd_fp(&b[0..48])?;
            a.y = rd_fp(&b[48..96])?;
            if !b.iter().all(|&x| x == 0) {
                /* not the all-zero infinity encoding: must be on the curve */
                if !blst_p1_affine_on_curve(&a) { return None; }
            }
            if subgroup && !blst_p1_affine_in_g1(&a) { return None; }
            Some(a)
        }
    }
    pub fn wr_g1(a: &blst_p1_affine, out: &mut [u8]) {
        unsafe {
            if blst_p1_affine_is_inf(a) { out.iter_mut().for_each(|x| *x = 0); }
            else { blst_p1_affine_serialize(out.as_mut_ptr(), a); }
        }
    }
    /* b is blst serialization order: x = c1||c0, y = c1||c0 (imaginary first). */
    pub fn rd_g2(b: &[u8], subgroup: bool) -> Option<blst_p2_affine> {
        unsafe {
            let mut a = blst_p2_affine::default();
            a.x.fp[1] = rd_fp(&b[0..48])?;
            a.x.fp[0] = rd_fp(&b[48..96])?;
            a.y.fp[1] = rd_fp(&b[96..144])?;
            a.y.fp[0] = rd_fp(&b[144..192])?;
            if !b.iter().all(|&x| x == 0) {
                if !blst_p2_affine_on_curve(&a) { return None; }
            }
            if subgroup && !blst_p2_affine_in_g2(&a) { return None; }
            Some(a)
        }
    }
    pub fn wr_g2(a: &blst_p2_affine, out: &mut [u8]) {
        unsafe {
            if blst_p2_affine_is_inf(a) { out.iter_mut().for_each(|x| *x = 0); }
            else { blst_p2_affine_serialize(out.as_mut_ptr(), a); }
        }
    }
    pub fn g1_add(p1: &[u8], p2: &[u8], out: &mut [u8]) -> bool {
        unsafe {
            let a = match rd_g1(p1, false) { Some(v) => v, None => return false };
            let b = match rd_g1(p2, false) { Some(v) => v, None => return false };
            let mut ja = blst_p1::default();
            blst_p1_from_affine(&mut ja, &a);
            let mut r = blst_p1::default();
            blst_p1_add_or_double_affine(&mut r, &ja, &b);
            let mut ra = blst_p1_affine::default();
            blst_p1_to_affine(&mut ra, &r);
            wr_g1(&ra, out);
            true
        }
    }
    pub fn g2_add(p1: &[u8], p2: &[u8], out: &mut [u8]) -> bool {
        unsafe {
            let a = match rd_g2(p1, false) { Some(v) => v, None => return false };
            let b = match rd_g2(p2, false) { Some(v) => v, None => return false };
            let mut ja = blst_p2::default();
            blst_p2_from_affine(&mut ja, &a);
            let mut r = blst_p2::default();
            blst_p2_add_or_double_affine(&mut r, &ja, &b);
            let mut ra = blst_p2_affine::default();
            blst_p2_to_affine(&mut ra, &r);
            wr_g2(&ra, out);
            true
        }
    }
    /// pairs: n * (g1[96] || g2[192]); verified = product of e(g1_i, g2_i) == 1
    pub fn pairing(pairs: &[u8], n: usize) -> Option<bool> {
        unsafe {
            let mut acc = blst_fp12::default();
            let mut first = true;
            for i in 0..n {
                let off = i * (96 + 192);
                let g1 = rd_g1(&pairs[off..off + 96], true)?;
                let g2 = rd_g2(&pairs[off + 96..off + 288], true)?;
                let mut ml = blst_fp12::default();
                blst_miller_loop(&mut ml, &g2, &g1);
                if first { acc = ml; first = false; } else { let t = acc; blst_fp12_mul(&mut acc, &t, &ml); }
            }
            if first { return Some(true); } // empty -> identity
            let mut fe = blst_fp12::default();
            blst_final_exp(&mut fe, &acc);
            Some(blst_fp12_is_one(&fe))
        }
    }
    pub fn g1_mul(p: &blst_p1_affine, scalar: &[u8]) -> blst_p1 {
        unsafe {
            let mut s = blst_scalar::default();
            blst_scalar_from_bendian(&mut s, scalar.as_ptr());
            let mut j = blst_p1::default();
            blst_p1_from_affine(&mut j, p);
            let mut r = blst_p1::default();
            blst_p1_mult(&mut r, &j, s.b.as_ptr(), 256);
            r
        }
    }
    pub fn g1_msm(pairs: &[u8], n: usize, out: &mut [u8]) -> bool {
        unsafe {
            let mut acc = blst_p1::default();
            let mut have = false;
            for i in 0..n {
                let off = i * (96 + 32);
                let pa = match rd_g1(&pairs[off..off + 96], true) { Some(v) => v, None => return false };
                let term = g1_mul(&pa, &pairs[off + 96..off + 128]);
                if have { let t = acc; blst_p1_add_or_double(&mut acc, &t, &term); } else { acc = term; have = true; }
            }
            let mut ra = blst_p1_affine::default();
            blst_p1_to_affine(&mut ra, &acc);
            wr_g1(&ra, out);
            true
        }
    }
    pub fn g2_msm(pairs: &[u8], n: usize, out: &mut [u8]) -> bool {
        unsafe {
            let mut acc = blst_p2::default();
            let mut have = false;
            for i in 0..n {
                let off = i * (192 + 32);
                let pa = match rd_g2(&pairs[off..off + 192], true) { Some(v) => v, None => return false };
                let mut s = blst_scalar::default();
                blst_scalar_from_bendian(&mut s, pairs[off + 192..].as_ptr());
                let mut j = blst_p2::default();
                blst_p2_from_affine(&mut j, &pa);
                let mut term = blst_p2::default();
                blst_p2_mult(&mut term, &j, s.b.as_ptr(), 256);
                if have { let t = acc; blst_p2_add_or_double(&mut acc, &t, &term); } else { acc = term; have = true; }
            }
            let mut ra = blst_p2_affine::default();
            blst_p2_to_affine(&mut ra, &acc);
            wr_g2(&ra, out);
            true
        }
    }
    pub fn map_fp_to_g1(fp: &[u8], out: &mut [u8]) -> bool {
        unsafe {
            if !fp_canonical(&fp[0..48]) { return false; }   /* EIP-2537: fp < p */
            let mut f = blst_fp::default();
            blst_fp_from_bendian(&mut f, fp.as_ptr());
            let mut p = blst_p1::default();
            blst_map_to_g1(&mut p, &f, core::ptr::null());
            let mut a = blst_p1_affine::default();
            blst_p1_to_affine(&mut a, &p);
            wr_g1(&a, out);
            true
        }
    }
    pub fn map_fp2_to_g2(fp2: &[u8], out: &mut [u8]) -> bool {
        unsafe {
            if !fp_canonical(&fp2[0..48]) || !fp_canonical(&fp2[48..96]) { return false; }
            let mut f = blst_fp2::default();
            blst_fp_from_bendian(&mut f.fp[0], fp2[0..48].as_ptr());
            blst_fp_from_bendian(&mut f.fp[1], fp2[48..96].as_ptr());
            let mut p = blst_p2::default();
            blst_map_to_g2(&mut p, &f, core::ptr::null());
            let mut a = blst_p2_affine::default();
            blst_p2_to_affine(&mut a, &p);
            wr_g2(&a, out);
            true
        }
    }
}

macro_rules! bls_pt { ($name:ident, $f:path, $inlen:expr, $outlen:expr) => {
    #[no_mangle] pub unsafe extern "C" fn $name(p1: *const u8, p2: *const u8, result: *mut u8) -> i32 {
        let mut out = [0u8; $outlen];
        if $f(slice(p1, $inlen), slice(p2, $inlen), &mut out) {
            core::ptr::copy_nonoverlapping(out.as_ptr(), result, $outlen); ZKVM_EOK } else { ZKVM_EFAIL }
    }
};}
bls_pt!(zkvm_bls12_g1_add, bls::g1_add, 96, 96);
bls_pt!(zkvm_bls12_g2_add, bls::g2_add, 192, 192);

#[no_mangle] pub unsafe extern "C" fn zkvm_bls12_g1_msm(pairs: *const u8, n: usize, result: *mut u8) -> i32 {
    let mut out = [0u8; 96];
    if bls::g1_msm(slice(pairs, n * 128), n, &mut out) { core::ptr::copy_nonoverlapping(out.as_ptr(), result, 96); ZKVM_EOK } else { ZKVM_EFAIL }
}
#[no_mangle] pub unsafe extern "C" fn zkvm_bls12_g2_msm(pairs: *const u8, n: usize, result: *mut u8) -> i32 {
    let mut out = [0u8; 192];
    if bls::g2_msm(slice(pairs, n * 224), n, &mut out) { core::ptr::copy_nonoverlapping(out.as_ptr(), result, 192); ZKVM_EOK } else { ZKVM_EFAIL }
}
#[no_mangle] pub unsafe extern "C" fn zkvm_bls12_pairing(pairs: *const u8, n: usize, verified: *mut bool) -> i32 {
    match bls::pairing(slice(pairs, n * 288), n) { Some(v) => { *verified = v; ZKVM_EOK } None => ZKVM_EFAIL }
}
#[no_mangle] pub unsafe extern "C" fn zkvm_bls12_map_fp_to_g1(fp: *const u8, result: *mut u8) -> i32 {
    let mut out = [0u8; 96];
    if bls::map_fp_to_g1(slice(fp, 48), &mut out) { core::ptr::copy_nonoverlapping(out.as_ptr(), result, 96); ZKVM_EOK } else { ZKVM_EFAIL }
}
#[no_mangle] pub unsafe extern "C" fn zkvm_bls12_map_fp2_to_g2(fp2: *const u8, result: *mut u8) -> i32 {
    let mut out = [0u8; 192];
    if bls::map_fp2_to_g2(slice(fp2, 96), &mut out) { core::ptr::copy_nonoverlapping(out.as_ptr(), result, 192); ZKVM_EOK } else { ZKVM_EFAIL }
}

/* ===================== not yet wired (EFAIL) ========================= */
#[no_mangle] pub extern "C" fn zkvm_secp256k1_verify(_:*const u8,_:*const u8,_:*const u8,_:*mut bool)->i32{ZKVM_EFAIL}

/* ============================== tests ================================= */
#[cfg(test)]
mod tests {
    use super::*;

    fn hex(s: &str) -> Vec<u8> {
        (0..s.len()).step_by(2).map(|i| u8::from_str_radix(&s[i..i + 2], 16).unwrap()).collect()
    }

    #[test]
    fn hashes() {
        let mut out = [0u8; 32];
        unsafe { zkvm_keccak256(b"".as_ptr(), 0, out.as_mut_ptr()); }
        assert_eq!(out[..], hex("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470")[..]);
        unsafe { zkvm_sha256(b"abc".as_ptr(), 3, out.as_mut_ptr()); }
        assert_eq!(out[..], hex("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")[..]);
        unsafe { zkvm_ripemd160(b"abc".as_ptr(), 3, out.as_mut_ptr()); }
        assert_eq!(out[12..], hex("8eb208f7e05d987a9b044a8e98c6b087f15a0bfc")[..]);
    }

    #[test]
    fn modexp_basic() {
        // 3^2 mod 5 = 4
        let mut out = [0u8; 1];
        unsafe { zkvm_modexp([3].as_ptr(), 1, [2].as_ptr(), 1, [5].as_ptr(), 1, out.as_mut_ptr()); }
        assert_eq!(out[0], 4);
    }

    #[test]
    fn bn254_ops() {
        // G1 generator = (1, 2)
        let mut g = [0u8; 64]; g[31] = 1; g[63] = 2;
        // ecadd(G, G) == ecmul(G, 2)
        let mut add = [0u8; 64];
        assert_eq!(unsafe { zkvm_bn254_g1_add(g.as_ptr(), g.as_ptr(), add.as_mut_ptr()) }, ZKVM_EOK);
        let mut two = [0u8; 32]; two[31] = 2;
        let mut mul = [0u8; 64];
        assert_eq!(unsafe { zkvm_bn254_g1_mul(g.as_ptr(), two.as_ptr(), mul.as_mut_ptr()) }, ZKVM_EOK);
        assert_eq!(add, mul, "2G via add == 2G via mul");
        // empty pairing input verifies true (EVM ecpairing of nothing)
        let mut v = false;
        assert_eq!(unsafe { zkvm_bn254_pairing(core::ptr::null(), 0, &mut v) }, ZKVM_EOK);
        assert!(v, "empty pairing -> true");
        // pairing([(G,H),(-G,H)]) == true (e(G,H)*e(-G,H) = 1); build via arkworks
        use ark_bn254::{G1Affine, G2Affine};
        use ark_ec::{AffineRepr, CurveGroup};
        let h = G2Affine::generator();
        let mut hb = [0u8; 128];
        {
            use ark_ff::{BigInteger, PrimeField};
            let (x, y) = (h.x().unwrap(), h.y().unwrap());
            let put = |f: ark_bn254::Fq, o: &mut [u8]| { let mut b = f.into_bigint().to_bytes_be(); while b.len()<32 {b.insert(0,0);} o.copy_from_slice(&b); };
            put(x.c1, &mut hb[0..32]); put(x.c0, &mut hb[32..64]);
            put(y.c1, &mut hb[64..96]); put(y.c0, &mut hb[96..128]);
        }
        let negg = { let p = (-G1Affine::generator()).into_group().into_affine(); let mut o=[0u8;64]; super::bn::write_g1(&p,&mut o); o };
        let mut input = Vec::new();
        input.extend_from_slice(&g); input.extend_from_slice(&hb);
        input.extend_from_slice(&negg); input.extend_from_slice(&hb);
        let mut v2 = false;
        assert_eq!(unsafe { zkvm_bn254_pairing(input.as_ptr(), 2, &mut v2) }, ZKVM_EOK);
        assert!(v2, "e(G,H)*e(-G,H) == 1");
    }

    #[test]
    fn bls_ops() {
        use blst::*;
        unsafe {
            let ga = BLS12_381_G1;   // affine generator
            let mut gb = [0u8; 96]; blst_p1_affine_serialize(gb.as_mut_ptr(), &ga);
            let mut add = [0u8; 96];
            assert_eq!(zkvm_bls12_g1_add(gb.as_ptr(), gb.as_ptr(), add.as_mut_ptr()), ZKVM_EOK);
            let mut two = [0u8; 32]; two[31] = 2;
            let mut msm_in = [0u8; 128]; msm_in[..96].copy_from_slice(&gb); msm_in[96..].copy_from_slice(&two);
            let mut mul = [0u8; 96];
            assert_eq!(zkvm_bls12_g1_msm(msm_in.as_ptr(), 1, mul.as_mut_ptr()), ZKVM_EOK);
            assert_eq!(add, mul, "2G via add == via msm");
            // pairing e(G,H)*e(-G,H) == 1
            let ha = BLS12_381_G2;   // affine generator
            let mut hb = [0u8; 192]; blst_p2_affine_serialize(hb.as_mut_ptr(), &ha);
            let mut gj = blst_p1::default(); blst_p1_from_affine(&mut gj, &ga);
            blst_p1_cneg(&mut gj, true);
            let mut nega = blst_p1_affine::default(); blst_p1_to_affine(&mut nega, &gj);
            let mut nb = [0u8; 96]; blst_p1_affine_serialize(nb.as_mut_ptr(), &nega);
            let mut pin = Vec::new();
            pin.extend_from_slice(&gb); pin.extend_from_slice(&hb);
            pin.extend_from_slice(&nb); pin.extend_from_slice(&hb);
            let mut v = false;
            assert_eq!(zkvm_bls12_pairing(pin.as_ptr(), 2, &mut v), ZKVM_EOK);
            assert!(v, "e(G,H)*e(-G,H) == 1");
        }
    }

    #[test]
    fn kzg_roundtrip() {
        // build a real commitment+proof with c-kzg, then verify through our ABI
        let settings = c_kzg::ethereum_kzg_settings(0);
        let blob = c_kzg::Blob::new([0u8; 131072]); // zero blob
        let commitment = settings.blob_to_kzg_commitment(&blob).unwrap();
        let z = c_kzg::Bytes32::new([0u8; 32]);
        let (proof, y) = settings.compute_kzg_proof(&blob, &z).unwrap();
        let comm = commitment.to_bytes();
        let mut verified = false;
        let rc = unsafe {
            zkvm_kzg_point_eval(comm.as_ptr(), z.as_ptr(), y.as_ptr(), proof.as_ptr(), &mut verified)
        };
        assert_eq!(rc, ZKVM_EOK);
        assert!(verified, "valid KZG proof verifies");
    }

    #[test]
    fn ecrecover_vector() {
        // EIP-155-style known ecrecover vector (msg hash, r, s, v=27 -> recid 0)
        let msg = hex("456e9aea5e197a1f1af7a3e85a3212fa4049a3ba34c2289b4c860fc0b0c64ef3");
        let r = hex("9242685bf161793cc25603c231bc2f568eb630ea16aa137d2664ac8038825608");
        let s = hex("4f8ae3bd7535248d0bd448298cc2e2071e56992d0774dc340c368ae950852ada");
        let mut sig = [0u8; 64];
        sig[..32].copy_from_slice(&r);
        sig[32..].copy_from_slice(&s);
        let mut pubkey = [0u8; 64];
        // canonical vector uses v=28 -> recid 1
        let rc = unsafe { zkvm_secp256k1_ecrecover(msg.as_ptr(), sig.as_ptr(), 1, pubkey.as_mut_ptr()) };
        assert_eq!(rc, ZKVM_EOK);
        // address = keccak(pubkey)[12:]
        let mut a = [0u8; 32];
        unsafe { zkvm_keccak256(pubkey.as_ptr(), 64, a.as_mut_ptr()); }
        assert_eq!(a[12..], hex("7156526fbd7a3c72969b54f64e42c10fbb768c8a")[..]);
    }
}
