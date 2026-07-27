/* Generated from sail/primitives/system.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_84(void) {

  sail_fixed_bytes_20 z3zE89;
  z3zE89 = evmsail_word_to_address(((sail_u256){{UINT64_C(18446744073709551614), UINT64_C(18446744073709551615), UINT64_C(4294967295), UINT64_C(0)}}));
  zSYSTEM_ADDRESS = z3zE89;
let_end_193: ;
}
void kill_letbind_84(void) {
}

void create_letbind_85(void) {

  sail_fixed_bytes_20 z3zE90;
  z3zE90 = evmsail_word_to_address(((sail_u256){{UINT64_C(13311379508201171970), UINT64_C(15506597749690834871), UINT64_C(998902), UINT64_C(0)}}));
  zBEACON_ROOTS_ADDR = z3zE90;
let_end_194: ;
}
void kill_letbind_85(void) {
}

void create_letbind_86(void) {

  sail_fixed_bytes_20 z3zE91;
  z3zE91 = evmsail_word_to_address(((sail_u256){{UINT64_C(3700577164601600309), UINT64_C(2878298490047003138), UINT64_C(63752), UINT64_C(0)}}));
  zHISTORY_STORAGE_ADDR = z3zE91;
let_end_195: ;
}
void kill_letbind_86(void) {
}

void create_letbind_87(void) {

  sail_fixed_bytes_20 z3zE92;
  z3zE92 = evmsail_word_to_address(((sail_u256){{UINT64_C(15579492241104728066), UINT64_C(17242047345525313946), UINT64_C(2401), UINT64_C(0)}}));
  zWITHDRAWAL_REQUEST_ADDR = z3zE92;
let_end_196: ;
}
void kill_letbind_87(void) {
}

void create_letbind_88(void) {

  sail_fixed_bytes_20 z3zE93;
  z3zE93 = evmsail_word_to_address(((sail_u256){{UINT64_C(10016273463683084881), UINT64_C(14397524800236640159), UINT64_C(48093), UINT64_C(0)}}));
  zCONSOLIDATION_REQUEST_ADDR = z3zE93;
let_end_197: ;
}
void kill_letbind_88(void) {
}

void create_letbind_89(void) {

  sail_fixed_bytes_20 z3zE94;
  z3zE94 = evmsail_word_to_address(((sail_u256){{UINT64_C(760111669195932290), UINT64_C(7603452151126424148), UINT64_C(49140), UINT64_C(0)}}));
  zBUILDER_DEPOSIT_REQUEST_ADDR = z3zE94;
let_end_198: ;
}
void kill_letbind_89(void) {
}

void create_letbind_90(void) {

  sail_fixed_bytes_20 z3zE95;
  z3zE95 = evmsail_word_to_address(((sail_u256){{UINT64_C(4307224735379260034), UINT64_C(8669529151676140297), UINT64_C(25814), UINT64_C(0)}}));
  zBUILDER_EXIT_REQUEST_ADDR = z3zE95;
let_end_199: ;
}
void kill_letbind_90(void) {
}

void create_letbind_91(void) {

  sail_fixed_bytes_20 z3zE96;
  {
    sail_u256 z3zE3013;
    z3zE3013 = u256_of_u128(((sail_u128){{UINT64_C(11294470620239562234), UINT64_C(2421447037043915651)}}));
    z3zE96 = evmsail_word_to_address(z3zE3013);
  }
  zDEPOSIT_CONTRACT_ADDR = z3zE96;
let_end_200: ;
}
void kill_letbind_91(void) {
}

void create_letbind_92(void) {

  sail_u256 z3zE97;
  z3zE97 = ((sail_u256){{UINT64_C(12074291595689605317), UINT64_C(16708898399860066458), UINT64_C(12676030261858484297), UINT64_C(7249595157780304706)}});
  zDEPOSIT_EVENT_TOPIC = z3zE97;
let_end_201: ;
}
void kill_letbind_92(void) {
}

void create_letbind_93(void) {

  sail_fixed_bytes_20 z3zE98;
  z3zE98 = evmsail_word_to_address(((sail_u256){{UINT64_C(18446744073709551614), UINT64_C(18446744073709551615), UINT64_C(4294967295), UINT64_C(0)}}));
  zEIP7708_SYSTEM_ADDRESS = z3zE98;
let_end_202: ;
}
void kill_letbind_93(void) {
}

void create_letbind_94(void) {

  sail_u256 z3zE99;
  z3zE99 = ((sail_u256){{UINT64_C(2951364421682967535), UINT64_C(10748869590852608278), UINT64_C(7620847484418887082), UINT64_C(15992936130196719771)}});
  zEIP7708_TRANSFER_TOPIC = z3zE99;
let_end_203: ;
}
void kill_letbind_94(void) {
}

void create_letbind_95(void) {

  sail_u256 z3zE100;
  z3zE100 = ((sail_u256){{UINT64_C(8187823086544452773), UINT64_C(7854220679750799226), UINT64_C(9321359278818002547), UINT64_C(14706211957200335488)}});
  zEIP7708_BURN_TOPIC = z3zE100;
let_end_204: ;
}
void kill_letbind_95(void) {
}

