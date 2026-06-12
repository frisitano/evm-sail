/* Reference implementation of the eth-act zkvm-standards crypto accelerators
 * (ffi/zkvm_accelerators.h). Self-contained C (only memset), so it links into
 * both the native runner and the freestanding riscv zkVM guest. A real zkVM host
 * replaces this file with its native precompiles behind the same standard header.
 *
 * Implemented: zkvm_keccak256, zkvm_sha256, zkvm_ripemd160.
 * Not yet implemented (return ZKVM_EFAIL): ecrecover, modexp, bn254, blake2f,
 * KZG, BLS12-381, secp256r1 -- these drop in here behind the unchanged header. */
#include "zkvm_accelerators.h"
#include <string.h>

/* ============================ Keccak-256 ================================ */
static const uint64_t K_RC[24] = {
  0x0000000000000001ULL,0x0000000000008082ULL,0x800000000000808aULL,0x8000000080008000ULL,
  0x000000000000808bULL,0x0000000080000001ULL,0x8000000080008081ULL,0x8000000000008009ULL,
  0x000000000000008aULL,0x0000000000000088ULL,0x0000000080008009ULL,0x000000008000000aULL,
  0x000000008000808bULL,0x800000000000008bULL,0x8000000000008089ULL,0x8000000000008003ULL,
  0x8000000000008002ULL,0x8000000000000080ULL,0x000000000000800aULL,0x800000008000000aULL,
  0x8000000080008081ULL,0x8000000000008080ULL,0x0000000080000001ULL,0x8000000080008008ULL };
static const int K_ROTC[24] = {1,3,6,10,15,21,28,36,45,55,2,14,27,41,56,8,25,43,62,18,39,61,20,44};
static const int K_PILN[24] = {10,7,11,17,18,3,5,16,8,21,24,4,15,23,19,13,12,2,20,14,22,9,6,1};
#define K_ROTL64(x,n) (((x)<<(n))|((x)>>(64-(n))))

static void keccakf(uint64_t st[25]) {
  int i,j,r; uint64_t t,bc[5];
  for (r=0;r<24;r++) {
    for (i=0;i<5;i++) bc[i]=st[i]^st[i+5]^st[i+10]^st[i+15]^st[i+20];
    for (i=0;i<5;i++){ t=bc[(i+4)%5]^K_ROTL64(bc[(i+1)%5],1); for(j=0;j<25;j+=5) st[j+i]^=t; }
    t=st[1];
    for (i=0;i<24;i++){ j=K_PILN[i]; bc[0]=st[j]; st[j]=K_ROTL64(t,K_ROTC[i]); t=bc[0]; }
    for (j=0;j<25;j+=5){ for(i=0;i<5;i++) bc[i]=st[j+i]; for(i=0;i<5;i++) st[j+i]^=(~bc[(i+1)%5])&bc[(i+2)%5]; }
    st[0]^=K_RC[r];
  }
}

zkvm_status zkvm_keccak256(const uint8_t *data, size_t len, zkvm_keccak256_hash *output) {
  uint64_t st[25]; uint8_t buf[136]; size_t blen = 0, i;
  memset(st, 0, sizeof st);
  for (i = 0; i < len; i++) {
    buf[blen++] = data[i];
    if (blen == 136) { for (size_t k=0;k<136;k++) st[k/8] ^= ((uint64_t)buf[k]) << ((k%8)*8); keccakf(st); blen = 0; }
  }
  memset(buf + blen, 0, 136 - blen);
  buf[blen] |= 0x01; buf[135] |= 0x80;   /* Ethereum keccak padding */
  for (size_t k=0;k<136;k++) st[k/8] ^= ((uint64_t)buf[k]) << ((k%8)*8);
  keccakf(st);
  for (i = 0; i < 32; i++) output->data[i] = (uint8_t)((st[i/8] >> ((i%8)*8)) & 0xff);
  return ZKVM_EOK;
}

/* ============================== SHA-256 ================================= */
static uint32_t s_rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
zkvm_status zkvm_sha256(const uint8_t *msg, size_t len, zkvm_sha256_hash *output) {
  static const uint32_t KK[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };
  uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
  uint8_t blk[64]; size_t off = 0; int padded = 0; uint64_t i;
  while (!padded) {
    size_t rem = len - off;
    if (rem >= 64) { memcpy(blk, msg + off, 64); off += 64; }
    else {
      memset(blk, 0, 64); if (rem) memcpy(blk, msg + off, rem); blk[rem] = 0x80;
      if (rem <= 55) { uint64_t b = (uint64_t)len * 8; for (int j=0;j<8;j++) blk[63-j]=(uint8_t)(b>>(8*j)); padded = 1; }
      else off = len; /* spill length to a following block */
    }
    uint32_t w[64];
    for (i=0;i<16;i++) w[i]=((uint32_t)blk[i*4]<<24)|((uint32_t)blk[i*4+1]<<16)|((uint32_t)blk[i*4+2]<<8)|blk[i*4+3];
    for (i=16;i<64;i++){ uint32_t s0=s_rotr(w[i-15],7)^s_rotr(w[i-15],18)^(w[i-15]>>3),
      s1=s_rotr(w[i-2],17)^s_rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
    for (i=0;i<64;i++){ uint32_t S1=s_rotr(e,6)^s_rotr(e,11)^s_rotr(e,25), ch=(e&f)^((~e)&g),
      t1=hh+S1+ch+KK[i]+w[i], S0=s_rotr(a,2)^s_rotr(a,13)^s_rotr(a,22), maj=(a&b)^(a&c)^(b&c), t2=S0+maj;
      hh=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2; }
    h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
    if (len - (off > len ? len : off) == 0 && (len % 64) > 55 && !padded) { /* need a final length-only block */
      memset(blk,0,64); uint64_t b=(uint64_t)len*8; for(int j=0;j<8;j++) blk[63-j]=(uint8_t)(b>>(8*j));
      for(i=0;i<16;i++) w[i]=((uint32_t)blk[i*4]<<24)|((uint32_t)blk[i*4+1]<<16)|((uint32_t)blk[i*4+2]<<8)|blk[i*4+3];
      for(i=16;i<64;i++){ uint32_t s0=s_rotr(w[i-15],7)^s_rotr(w[i-15],18)^(w[i-15]>>3),
        s1=s_rotr(w[i-2],17)^s_rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
      a=h[0];b=h[1];c=h[2];d=h[3];e=h[4];f=h[5];g=h[6];hh=h[7];
      for(i=0;i<64;i++){ uint32_t S1=s_rotr(e,6)^s_rotr(e,11)^s_rotr(e,25), ch=(e&f)^((~e)&g),
        t1=hh+S1+ch+KK[i]+w[i], S0=s_rotr(a,2)^s_rotr(a,13)^s_rotr(a,22), maj=(a&b)^(a&c)^(b&c), t2=S0+maj;
        hh=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2; }
      h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh; padded = 1;
    }
  }
  for (i=0;i<8;i++){ output->data[i*4]=(uint8_t)(h[i]>>24); output->data[i*4+1]=(uint8_t)(h[i]>>16);
    output->data[i*4+2]=(uint8_t)(h[i]>>8); output->data[i*4+3]=(uint8_t)h[i]; }
  return ZKVM_EOK;
}

/* ============================ RIPEMD-160 =============================== */
static uint32_t r_rol(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
zkvm_status zkvm_ripemd160(const uint8_t *msg, size_t len, zkvm_ripemd160_hash *output) {
  static const int rl[80]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8,
    3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12,1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2,4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13};
  static const int rr[80]={5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12,6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2,
    15,5,1,3,7,14,6,9,11,8,12,2,10,0,4,13,8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14,12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11};
  static const int sl[80]={11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8,7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12,
    11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5,11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12,9,15,5,11,6,8,13,12,5,12,13,14,11,8,5,6};
  static const int sr[80]={8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6,9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11,
    9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5,15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8,8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11};
  static const uint32_t CL[5]={0,0x5a827999,0x6ed9eba1,0x8f1bbcdc,0xa953fd4e};
  static const uint32_t CR[5]={0x50a28be6,0x5c4dd124,0x6d703ef3,0x7a6d76e9,0};
  uint32_t h[5]={0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0};
  uint8_t blk[64]; size_t off=0; int padded=0;
  while (!padded) {
    size_t rem=len-off;
    if (rem>=64){ memcpy(blk,msg+off,64); off+=64; }
    else { memset(blk,0,64); if(rem) memcpy(blk,msg+off,rem); blk[rem]=0x80;
      if (rem<=55){ uint64_t b=(uint64_t)len*8; for(int j=0;j<8;j++) blk[56+j]=(uint8_t)(b>>(8*j)); padded=1; } else off=len; }
    uint32_t X[16]; for(int i=0;i<16;i++) X[i]=(uint32_t)blk[i*4]|((uint32_t)blk[i*4+1]<<8)|((uint32_t)blk[i*4+2]<<16)|((uint32_t)blk[i*4+3]<<24);
    uint32_t al=h[0],bl=h[1],cl=h[2],dl=h[3],el=h[4],ar=h[0],br=h[1],cr=h[2],dr=h[3],er=h[4],t;
    for(int i=0;i<80;i++){ int rn=i/16; uint32_t fl,fr;
      switch(rn){case 0:fl=bl^cl^dl;break;case 1:fl=(bl&cl)|(~bl&dl);break;case 2:fl=(bl|~cl)^dl;break;case 3:fl=(bl&dl)|(cl&~dl);break;default:fl=bl^(cl|~dl);}
      t=r_rol(al+fl+X[rl[i]]+CL[rn],sl[i])+el; al=el;el=dl;dl=r_rol(cl,10);cl=bl;bl=t;
      switch(rn){case 0:fr=br^(cr|~dr);break;case 1:fr=(br&dr)|(cr&~dr);break;case 2:fr=(br|~cr)^dr;break;case 3:fr=(br&cr)|(~br&dr);break;default:fr=br^cr^dr;}
      t=r_rol(ar+fr+X[rr[i]]+CR[rn],sr[i])+er; ar=er;er=dr;dr=r_rol(cr,10);cr=br;br=t; }
    t=h[1]+cl+dr; h[1]=h[2]+dl+er; h[2]=h[3]+el+ar; h[3]=h[4]+al+br; h[4]=h[0]+bl+cr; h[0]=t;
    if (!padded && rem<64) { /* rem>55: emit a final length-only block */
      memset(blk,0,64); uint64_t b=(uint64_t)len*8; for(int j=0;j<8;j++) blk[56+j]=(uint8_t)(b>>(8*j));
      for(int i=0;i<16;i++) X[i]=(uint32_t)blk[i*4]|((uint32_t)blk[i*4+1]<<8)|((uint32_t)blk[i*4+2]<<16)|((uint32_t)blk[i*4+3]<<24);
      al=h[0];bl=h[1];cl=h[2];dl=h[3];el=h[4];ar=h[0];br=h[1];cr=h[2];dr=h[3];er=h[4];
      for(int i=0;i<80;i++){ int rn=i/16; uint32_t fl,fr;
        switch(rn){case 0:fl=bl^cl^dl;break;case 1:fl=(bl&cl)|(~bl&dl);break;case 2:fl=(bl|~cl)^dl;break;case 3:fl=(bl&dl)|(cl&~dl);break;default:fl=bl^(cl|~dl);}
        t=r_rol(al+fl+X[rl[i]]+CL[rn],sl[i])+el; al=el;el=dl;dl=r_rol(cl,10);cl=bl;bl=t;
        switch(rn){case 0:fr=br^(cr|~dr);break;case 1:fr=(br&dr)|(cr&~dr);break;case 2:fr=(br|~cr)^dr;break;case 3:fr=(br&cr)|(~br&dr);break;default:fr=br^cr^dr;}
        t=r_rol(ar+fr+X[rr[i]]+CR[rn],sr[i])+er; ar=er;er=dr;dr=r_rol(cr,10);cr=br;br=t; }
      t=h[1]+cl+dr; h[1]=h[2]+dl+er; h[2]=h[3]+el+ar; h[3]=h[4]+al+br; h[4]=h[0]+bl+cr; h[0]=t; padded=1;
    }
  }
  memset(output->data, 0, 32);   /* 20-byte digest left-padded to 32 (first 12 zero) */
  for (int i=0;i<5;i++){ output->data[12+i*4]=(uint8_t)h[i]; output->data[12+i*4+1]=(uint8_t)(h[i]>>8);
    output->data[12+i*4+2]=(uint8_t)(h[i]>>16); output->data[12+i*4+3]=(uint8_t)(h[i]>>24); }
  return ZKVM_EOK;
}

/* ===================== not yet implemented (EFAIL) ===================== */
zkvm_status zkvm_secp256k1_verify(const zkvm_secp256k1_hash*a,const zkvm_secp256k1_signature*b,const zkvm_secp256k1_pubkey*c,bool*d){(void)a;(void)b;(void)c;(void)d;return ZKVM_EFAIL;}
zkvm_status zkvm_secp256k1_ecrecover(const zkvm_secp256k1_hash*a,const zkvm_secp256k1_signature*b,uint8_t c,zkvm_secp256k1_pubkey*d){(void)a;(void)b;(void)c;(void)d;return ZKVM_EFAIL;}
zkvm_status zkvm_modexp(const uint8_t*a,size_t b,const uint8_t*c,size_t d,const uint8_t*e,size_t f,uint8_t*g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return ZKVM_EFAIL;}
zkvm_status zkvm_bn254_g1_add(const zkvm_bn254_g1_point*a,const zkvm_bn254_g1_point*b,zkvm_bn254_g1_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bn254_g1_mul(const zkvm_bn254_g1_point*a,const zkvm_bn254_scalar*b,zkvm_bn254_g1_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bn254_pairing(const zkvm_bn254_pairing_pair*a,size_t b,bool*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_blake2f(uint32_t a,zkvm_blake2f_state*b,const zkvm_blake2f_message*c,const zkvm_blake2f_offset*d,uint8_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return ZKVM_EFAIL;}
zkvm_status zkvm_kzg_point_eval(const zkvm_kzg_commitment*a,const zkvm_kzg_field_element*b,const zkvm_kzg_field_element*c,const zkvm_kzg_proof*d,bool*e){(void)a;(void)b;(void)c;(void)d;(void)e;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_g1_add(const zkvm_bls12_381_g1_point*a,const zkvm_bls12_381_g1_point*b,zkvm_bls12_381_g1_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_g1_msm(const zkvm_bls12_381_g1_msm_pair*a,size_t b,zkvm_bls12_381_g1_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_g2_add(const zkvm_bls12_381_g2_point*a,const zkvm_bls12_381_g2_point*b,zkvm_bls12_381_g2_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_g2_msm(const zkvm_bls12_381_g2_msm_pair*a,size_t b,zkvm_bls12_381_g2_point*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_pairing(const zkvm_bls12_381_pairing_pair*a,size_t b,bool*c){(void)a;(void)b;(void)c;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_map_fp_to_g1(const zkvm_bls12_381_fp*a,zkvm_bls12_381_g1_point*b){(void)a;(void)b;return ZKVM_EFAIL;}
zkvm_status zkvm_bls12_map_fp2_to_g2(const zkvm_bls12_381_fp2*a,zkvm_bls12_381_g2_point*b){(void)a;(void)b;return ZKVM_EFAIL;}
zkvm_status zkvm_secp256r1_verify(const zkvm_secp256r1_hash*a,const zkvm_secp256r1_signature*b,const zkvm_secp256r1_pubkey*c,bool*d){(void)a;(void)b;(void)c;(void)d;return ZKVM_EFAIL;}
