/* Generated from sail/executor/payload.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
sail_fixed_bytes_32 zblock_header_hash(struct zBlockHeader zheader, sail_fixed_bytes_32 ztransactions_root, sail_fixed_bytes_32 zwithdrawals_root, sail_fixed_bytes_32 zrequests_hash, sail_fixed_bytes_32 zblock_access_list_hash)
{
  sail_fixed_bytes_32 z8zE685;
  zz5listz8z5bv8z9 zbloom;
  CREATE(zz5listz8z5bv8z9)(&zbloom);
  {
    sail_fixed_bytes_256 z2zE183;
    z2zE183 = zheader.zlogs_bloom;
    zlogs_bloom_bytes(&zbloom, z2zE183);
  }
  zz5listz8z5bv8z9 znonce;
  CREATE(zz5listz8z5bv8z9)(&znonce);
  {
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
    zconsz3z5bv8(&znonce, UINT64_C(0x00), znonce);
  }
  uint64_t zbloom_length;
  {
    zbloom_length = zrlp_scratch_bytes_sizze(zbloom, zLOGS_BLOOM_BYTE_LENGTH);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  uint64_t znumber_length;
  {
    uint64_t z2zE182;
    z2zE182 = zheader.znumber;
    znumber_length = zrlp_uint_nat_sizze(z2zE182);
  }
  uint64_t zgas_limit_length;
  {
    uint64_t z2zE181;
    z2zE181 = zheader.zgas_limit;
    zgas_limit_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE181);
  }
  uint64_t zgas_used_length;
  {
    uint64_t z2zE180;
    z2zE180 = zheader.zgas_used;
    zgas_used_length = zrlp_uint_nat_sizze(z2zE180);
  }
  uint64_t ztimestamp_length;
  {
    uint64_t z2zE179;
    z2zE179 = zheader.ztimestamp;
    ztimestamp_length = zrlp_uint_nat_sizze(z2zE179);
  }
  uint64_t zextra_data_length;
  {
    struct zByteSliceFields z2zE178;
    z2zE178 = zheader.zextra_data;
    {
      zextra_data_length = zrlp_scratch_slice_sizze(z2zE178);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
  }
  uint64_t znonce_length;
  {
    znonce_length = zrlp_scratch_bytes_sizze(znonce, zEIGHT_BYTE_LENGTH);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  uint64_t zcontent_length;
  zcontent_length = UINT64_C(219);
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zbloom_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE507;
  z3zE507 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(1));
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE506;
  z3zE506 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, znumber_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE505;
  z3zE505 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zgas_limit_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE504;
  z3zE504 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zgas_used_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE503;
  z3zE503 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, ztimestamp_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE502;
  z3zE502 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zextra_data_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE501;
  z3zE501 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, znonce_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1152;
    }
  }
  unit z3zE500;
  z3zE500 = UNIT;
  bool z2zE138;
  z2zE138 = zfork_gteq(zk_fork, zLondon);
  unit z3zE499;
  if (z2zE138) {
    uint64_t zfield_length;
    {
      sail_u256 z2zE139;
      z2zE139 = zheader.zbase_fee;
      zfield_length = zrlp_uint_word_sizze(z2zE139);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zfield_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    z3zE499 = UNIT;
  } else {  z3zE499 = UNIT;  }
  bool z2zE140;
  z2zE140 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE498;
  if (z2zE140) {
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    z3zE498 = UNIT;
  } else {  z3zE498 = UNIT;  }
  bool z2zE141;
  z2zE141 = zfork_gteq(zk_fork, zCancun);
  unit z3zE495;
  if (z2zE141) {
    uint64_t zblob_gas_used_length;
    {
      uint64_t z2zE143;
      z2zE143 = zheader.zblob_gas_used;
      zblob_gas_used_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE143);
    }
    uint64_t zexcess_blob_gas_length;
    {
      uint64_t z2zE142;
      z2zE142 = zheader.zexcess_blob_gas;
      zexcess_blob_gas_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE142);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zblob_gas_used_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    unit z3zE497;
    z3zE497 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zexcess_blob_gas_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    unit z3zE496;
    z3zE496 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    z3zE495 = UNIT;
  } else {  z3zE495 = UNIT;  }
  bool z2zE144;
  z2zE144 = zfork_gteq(zk_fork, zPrague);
  unit z3zE494;
  if (z2zE144) {
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    z3zE494 = UNIT;
  } else {  z3zE494 = UNIT;  }
  bool z2zE145;
  z2zE145 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE492;
  if (z2zE145) {
    uint64_t zslot_number_length;
    {
      uint64_t z2zE146;
      z2zE146 = zheader.zslot_number;
      zslot_number_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE146);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    unit z3zE493;
    z3zE493 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zslot_number_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1152;
      }
    }
    z3zE492 = UNIT;
  } else {  z3zE492 = UNIT;  }
  bool z2zE147;
  z2zE147 = (UINT64_C(749) < zcontent_length);
  unit z3zE491;
  if (z2zE147) {
    struct zexception z2zE148;
    CREATE(zexception)(&z2zE148);
    zInvalidBlock(&z2zE148, zRlpDecode);
    COPY(zexception)(current_exception, z2zE148);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:95.8-95.37");
    KILL(zz5listz8z5bv8z9)(&zbloom);
    KILL(zz5listz8z5bv8z9)(&znonce);
    KILL(zexception)(&z2zE148);
    goto end_block_exception_1152;
    /* unreachable after throw */
    KILL(zexception)(&z2zE148);
  } else {  z3zE491 = UNIT;  }
  uint64_t zbounded_content_length;
  {    zbounded_content_length = (zcontent_length % UINT64_C(750));
  }
  uint64_t zcontent_len;
  zcontent_len = zbounded_content_length;
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE531;
  z3zE531 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  sail_u256 z2zE150;
  {
    sail_fixed_bytes_32 z2zE149;
    z2zE149 = zheader.zparent_hash;
    z2zE150 = evmsail_hash_to_word(z2zE149);
  }
  unit z3zE530;
  z3zE530 = zrlp_write_word(z2zE150);
  sail_u256 z2zE151;
  z2zE151 = evmsail_hash_to_word(zEMPTY_OMMER_HASH);
  unit z3zE529;
  z3zE529 = zrlp_write_word(z2zE151);
  sail_fixed_bytes_20 z2zE152;
  z2zE152 = zheader.zfee_recipient;
  unit z3zE528;
  z3zE528 = zrlp_write_addr(z2zE152);
  sail_u256 z2zE154;
  {
    sail_fixed_bytes_32 z2zE153;
    z2zE153 = zheader.zstate_root;
    z2zE154 = evmsail_hash_to_word(z2zE153);
  }
  unit z3zE527;
  z3zE527 = zrlp_write_word(z2zE154);
  sail_u256 z2zE155;
  z2zE155 = evmsail_hash_to_word(ztransactions_root);
  unit z3zE526;
  z3zE526 = zrlp_write_word(z2zE155);
  sail_u256 z2zE157;
  {
    sail_fixed_bytes_32 z2zE156;
    z2zE156 = zheader.zreceipts_root;
    z2zE157 = evmsail_hash_to_word(z2zE156);
  }
  unit z3zE525;
  z3zE525 = zrlp_write_word(z2zE157);
  unit z3zE524;
  z3zE524 = zrlp_write_bytes(zbloom, zLOGS_BLOOM_BYTE_LENGTH);
  unit z3zE523;
  z3zE523 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(UINT64_C(0));
  uint64_t z2zE158;
  z2zE158 = zheader.znumber;
  unit z3zE522;
  z3zE522 = zrlp_write_uint_nat(z2zE158);
  uint64_t z2zE159;
  z2zE159 = zheader.zgas_limit;
  unit z3zE521;
  z3zE521 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE159);
  uint64_t z2zE160;
  z2zE160 = zheader.zgas_used;
  unit z3zE520;
  z3zE520 = zrlp_write_uint_nat(z2zE160);
  uint64_t z2zE161;
  z2zE161 = zheader.ztimestamp;
  unit z3zE519;
  z3zE519 = zrlp_write_uint_nat(z2zE161);
  struct zByteSliceFields z2zE162;
  z2zE162 = zheader.zextra_data;
  unit z3zE518;
  z3zE518 = zrlp_write_slice(z2zE162);
  sail_u256 z2zE163;
  z2zE163 = zheader.zprev_randao;
  unit z3zE517;
  z3zE517 = zrlp_write_word(z2zE163);
  unit z3zE516;
  z3zE516 = zrlp_write_bytes(znonce, zEIGHT_BYTE_LENGTH);
  bool z2zE164;
  z2zE164 = zfork_gteq(zk_fork, zLondon);
  unit z3zE515;
  if (z2zE164) {
    sail_u256 z2zE165;
    z2zE165 = zheader.zbase_fee;
    z3zE515 = zrlp_write_uint_word(z2zE165);
  } else {  z3zE515 = UNIT;  }
  bool z2zE166;
  z2zE166 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE514;
  if (z2zE166) {
    sail_u256 z2zE167;
    z2zE167 = evmsail_hash_to_word(zwithdrawals_root);
    z3zE514 = zrlp_write_word(z2zE167);
  } else {  z3zE514 = UNIT;  }
  bool z2zE168;
  z2zE168 = zfork_gteq(zk_fork, zCancun);
  unit z3zE511;
  if (z2zE168) {
    uint64_t z2zE169;
    z2zE169 = zheader.zblob_gas_used;
    unit z3zE513;
    z3zE513 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE169);
    uint64_t z2zE170;
    z2zE170 = zheader.zexcess_blob_gas;
    unit z3zE512;
    z3zE512 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE170);
    sail_u256 z2zE172;
    {
      sail_fixed_bytes_32 z2zE171;
      z2zE171 = zheader.zparent_beacon_block_root;
      z2zE172 = evmsail_hash_to_word(z2zE171);
    }
    z3zE511 = zrlp_write_word(z2zE172);
  } else {  z3zE511 = UNIT;  }
  bool z2zE173;
  z2zE173 = zfork_gteq(zk_fork, zPrague);
  unit z3zE510;
  if (z2zE173) {
    sail_u256 z2zE174;
    z2zE174 = evmsail_hash_to_word(zrequests_hash);
    z3zE510 = zrlp_write_word(z2zE174);
  } else {  z3zE510 = UNIT;  }
  bool z2zE175;
  z2zE175 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE508;
  if (z2zE175) {
    sail_u256 z2zE176;
    z2zE176 = evmsail_hash_to_word(zblock_access_list_hash);
    unit z3zE509;
    z3zE509 = zrlp_write_word(z2zE176);
    uint64_t z2zE177;
    z2zE177 = zheader.zslot_number;
    z3zE508 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE177);
  } else {  z3zE508 = UNIT;  }
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_32 zblock_hash;
  zblock_hash = zkeccak256_slice(zencoded);
  unit z3zE532;
  z3zE532 = zscratch_rewind(zmark);
  z8zE685 = zblock_hash;
  KILL(zz5listz8z5bv8z9)(&znonce);
  KILL(zz5listz8z5bv8z9)(&zbloom);
end_function_1151: ;
  return z8zE685;
end_block_exception_1152: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 ztransaction_trie_root(struct zBoundedSszzListRef ztxs)
{
  sail_fixed_bytes_32 z8zE686;
  struct zOptimizzedHashResult z2zE137;
  CREATE(zOptimizzedHashResult)(&z2zE137);
  evmsail_transaction_trie_root(&z2zE137, ztxs);
  {
    z8zE686 = zoptimizzed_hash_unwrap(z2zE137);
    if (have_exception) {
      KILL(zOptimizzedHashResult)(&z2zE137);
      goto end_block_exception_1150;
    }
  }
  KILL(zOptimizzedHashResult)(&z2zE137);
end_function_1149: ;
  return z8zE686;
end_block_exception_1150: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zwithdrawals_trie_root(struct zBoundedSszzListRef zwds)
{
  sail_fixed_bytes_32 z8zE687;
  struct zOptimizzedHashResult z2zE136;
  CREATE(zOptimizzedHashResult)(&z2zE136);
  evmsail_withdrawals_trie_root(&z2zE136, zwds);
  {
    z8zE687 = zoptimizzed_hash_unwrap(z2zE136);
    if (have_exception) {
      KILL(zOptimizzedHashResult)(&z2zE136);
      goto end_block_exception_1148;
    }
  }
  KILL(zOptimizzedHashResult)(&z2zE136);
end_function_1147: ;
  return z8zE687;
end_block_exception_1148: ;

  return fixed_bytes_32_zero();
}

uint64_t zexpected_payload_excess_blob_gas(struct zWitnessContext zwitness)
{
  uint64_t z8zE688;
  uint64_t z2zE133;
  z2zE133 = zwitness.zparent_excess_blob_gas;
  uint64_t z2zE134;
  z2zE134 = zwitness.zparent_blob_gas_used;
  sail_u256 z2zE135;
  z2zE135 = zwitness.zparent_base_fee_per_gas;
  {
    z8zE688 = znext_excess_blob_gas(z2zE133, z2zE134, z2zE135);
    if (have_exception) {  goto end_block_exception_1146;  }
  }
end_function_1145: ;
  return z8zE688;
end_block_exception_1146: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_32 zexecution_requests_hash(struct zStatelessInputRef zinput_ref)
{
  sail_fixed_bytes_32 z8zE689;
  struct zByteSliceFields zdeposits;
  zdeposits = zinput_ref.zdeposits;
  struct zByteSliceFields zwithdrawal_requests;
  zwithdrawal_requests = zinput_ref.zwithdrawal_requests;
  struct zByteSliceFields zconsolidation_requests;
  zconsolidation_requests = zinput_ref.zconsolidation_requests;
  struct zByteSliceFields zbuilder_deposit_requests;
  zbuilder_deposit_requests = zinput_ref.zbuilder_deposit_requests;
  struct zByteSliceFields zbuilder_exit_requests;
  zbuilder_exit_requests = zinput_ref.zbuilder_exit_requests;
  sail_fixed_bytes_32 zd0;
  {
    bool z2zE132;
    {
      uint64_t z2zE131;
      z2zE131 = zdeposits.zlen;
      z2zE132 = (z2zE131 != UINT64_C(0));
    }
    if (z2zE132) {  zd0 = zsha256_request_digest(UINT64_C(0x00), zdeposits);  } else {  zd0 = zZERO_HASH;  }
  }
  sail_fixed_bytes_32 zd1;
  {
    bool z2zE130;
    {
      uint64_t z2zE129;
      z2zE129 = zwithdrawal_requests.zlen;
      z2zE130 = (z2zE129 != UINT64_C(0));
    }
    if (z2zE130) {  zd1 = zsha256_request_digest(UINT64_C(0x01), zwithdrawal_requests);  } else {  zd1 = zZERO_HASH;  }
  }
  sail_fixed_bytes_32 zd2;
  {
    bool z2zE128;
    {
      uint64_t z2zE127;
      z2zE127 = zconsolidation_requests.zlen;
      z2zE128 = (z2zE127 != UINT64_C(0));
    }
    if (z2zE128) {  zd2 = zsha256_request_digest(UINT64_C(0x02), zconsolidation_requests);  } else {
      zd2 = zZERO_HASH;
    }
  }
  sail_fixed_bytes_32 zd3;
  {
    bool z2zE126;
    {
      uint64_t z2zE125;
      z2zE125 = zbuilder_deposit_requests.zlen;
      z2zE126 = (z2zE125 != UINT64_C(0));
    }
    if (z2zE126) {  zd3 = zsha256_request_digest(UINT64_C(0x03), zbuilder_deposit_requests);  } else {
      zd3 = zZERO_HASH;
    }
  }
  sail_fixed_bytes_32 zd4;
  {
    bool z2zE124;
    {
      uint64_t z2zE123;
      z2zE123 = zbuilder_exit_requests.zlen;
      z2zE124 = (z2zE123 != UINT64_C(0));
    }
    if (z2zE124) {  zd4 = zsha256_request_digest(UINT64_C(0x04), zbuilder_exit_requests);  } else {
      zd4 = zZERO_HASH;
    }
  }
  zz5listz8z5unionz0zzBytesz9 zsegs;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&zsegs);
  {
  }
  bool z2zE104;
  {
    uint64_t z2zE103;
    z2zE103 = zbuilder_exit_requests.zlen;
    z2zE104 = (z2zE103 != UINT64_C(0));
  }
  unit z3zE489;
  if (z2zE104) {
    struct zBytes z2zE106;
    CREATE(zBytes)(&z2zE106);
    {
      zz5listz8z5bv8z9 z2zE105;
      CREATE(zz5listz8z5bv8z9)(&z2zE105);
      zhash_to_bytes32(&z2zE105, zd4);
      zbytes_list(&z2zE106, z2zE105, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE105);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE106, zsegs);
    z3zE489 = UNIT;
    KILL(zBytes)(&z2zE106);
  } else {  z3zE489 = UNIT;  }
  bool z2zE108;
  {
    uint64_t z2zE107;
    z2zE107 = zbuilder_deposit_requests.zlen;
    z2zE108 = (z2zE107 != UINT64_C(0));
  }
  unit z3zE488;
  if (z2zE108) {
    struct zBytes z2zE110;
    CREATE(zBytes)(&z2zE110);
    {
      zz5listz8z5bv8z9 z2zE109;
      CREATE(zz5listz8z5bv8z9)(&z2zE109);
      zhash_to_bytes32(&z2zE109, zd3);
      zbytes_list(&z2zE110, z2zE109, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE109);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE110, zsegs);
    z3zE488 = UNIT;
    KILL(zBytes)(&z2zE110);
  } else {  z3zE488 = UNIT;  }
  bool z2zE112;
  {
    uint64_t z2zE111;
    z2zE111 = zconsolidation_requests.zlen;
    z2zE112 = (z2zE111 != UINT64_C(0));
  }
  unit z3zE487;
  if (z2zE112) {
    struct zBytes z2zE114;
    CREATE(zBytes)(&z2zE114);
    {
      zz5listz8z5bv8z9 z2zE113;
      CREATE(zz5listz8z5bv8z9)(&z2zE113);
      zhash_to_bytes32(&z2zE113, zd2);
      zbytes_list(&z2zE114, z2zE113, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE113);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE114, zsegs);
    z3zE487 = UNIT;
    KILL(zBytes)(&z2zE114);
  } else {  z3zE487 = UNIT;  }
  bool z2zE116;
  {
    uint64_t z2zE115;
    z2zE115 = zwithdrawal_requests.zlen;
    z2zE116 = (z2zE115 != UINT64_C(0));
  }
  unit z3zE486;
  if (z2zE116) {
    struct zBytes z2zE118;
    CREATE(zBytes)(&z2zE118);
    {
      zz5listz8z5bv8z9 z2zE117;
      CREATE(zz5listz8z5bv8z9)(&z2zE117);
      zhash_to_bytes32(&z2zE117, zd1);
      zbytes_list(&z2zE118, z2zE117, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE117);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE118, zsegs);
    z3zE486 = UNIT;
    KILL(zBytes)(&z2zE118);
  } else {  z3zE486 = UNIT;  }
  bool z2zE120;
  {
    uint64_t z2zE119;
    z2zE119 = zdeposits.zlen;
    z2zE120 = (z2zE119 != UINT64_C(0));
  }
  unit z3zE485;
  if (z2zE120) {
    struct zBytes z2zE122;
    CREATE(zBytes)(&z2zE122);
    {
      zz5listz8z5bv8z9 z2zE121;
      CREATE(zz5listz8z5bv8z9)(&z2zE121);
      zhash_to_bytes32(&z2zE121, zd0);
      zbytes_list(&z2zE122, z2zE121, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE121);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE122, zsegs);
    z3zE485 = UNIT;
    KILL(zBytes)(&z2zE122);
  } else {  z3zE485 = UNIT;  }
  z8zE689 = host_sha256_segments(zsegs);
  KILL(zz5listz8z5unionz0zzBytesz9)(&zsegs);
end_function_1143: ;
  return z8zE689;
end_block_exception_1144: ;

  return fixed_bytes_32_zero();
}

unit zvalidate_execution_payload(struct zStatelessInput zinput, struct zStatelessInputRef zinput_ref, struct zWitnessContext zwitness)
{
  unit z8zE690;
  struct zExecutionPayload zpayload;
  zpayload = zinput.zpayload;
  struct zBlock zblock;
  zblock = zpayload.zblock;
  struct zBlockHeader zheader;
  zheader = zblock.zheader;
  struct zBlockBody zbody;
  zbody = zblock.zbody;
  bool z2zE82;
  {
    uint64_t z2zE80;
    z2zE80 = zheader.zgas_limit;
    uint64_t z2zE81;
    z2zE81 = zheader.zgas_used;
    z2zE82 = (z2zE80 < z2zE81);
  }
  unit z3zE483;
  if (z2zE82) {
    struct zexception z2zE83;
    CREATE(zexception)(&z2zE83);
    zInvalidBlock(&z2zE83, zInvalidGasUsed);
    COPY(zexception)(current_exception, z2zE83);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:248.8-248.42");
    KILL(zexception)(&z2zE83);
    goto end_block_exception_1142;
    /* unreachable after throw */
    KILL(zexception)(&z2zE83);
  } else {  z3zE483 = UNIT;  }
  bool z2zE86;
  {
    sail_fixed_bytes_32 z2zE84;
    z2zE84 = zwitness.zparent_hash;
    sail_fixed_bytes_32 z2zE85;
    z2zE85 = zheader.zparent_hash;
    z2zE86 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE84, z2zE85);
  }
  unit z3zE482;
  if (z2zE86) {
    struct zexception z2zE87;
    CREATE(zexception)(&z2zE87);
    zInvalidBlock(&z2zE87, zInvalidParentHash);
    COPY(zexception)(current_exception, z2zE87);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:251.8-251.45");
    KILL(zexception)(&z2zE87);
    goto end_block_exception_1142;
    /* unreachable after throw */
    KILL(zexception)(&z2zE87);
  } else {  z3zE482 = UNIT;  }
  bool z2zE91;
  {
    bool z2zE90;
    z2zE90 = zfork_gteq(zk_fork, zCancun);
    bool z3zE480;
    if (z2zE90) {
      uint64_t z2zE88;
      z2zE88 = zheader.zexcess_blob_gas;
      uint64_t z2zE89;
      {
        z2zE89 = zexpected_payload_excess_blob_gas(zwitness);
        if (have_exception) {  goto end_block_exception_1142;  }
      }
      z3zE480 = (z2zE88 != z2zE89);
    } else {  z3zE480 = false;  }
    z2zE91 = z3zE480;
  }
  unit z3zE481;
  if (z2zE91) {
    struct zexception z2zE92;
    CREATE(zexception)(&z2zE92);
    zInvalidBlock(&z2zE92, zInvalidExcessBlobGas);
    COPY(zexception)(current_exception, z2zE92);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:254.8-254.48");
    KILL(zexception)(&z2zE92);
    goto end_block_exception_1142;
    /* unreachable after throw */
    KILL(zexception)(&z2zE92);
  } else {  z3zE481 = UNIT;  }
  bool z2zE93;
  z2zE93 = zfork_gteq(zk_fork, zParis);
  if (z2zE93) {
    sail_fixed_bytes_32 ztransactions_root;
    {
      struct zBoundedSszzListRef z2zE102;
      z2zE102 = zbody.ztransactions;
      {
        ztransactions_root = ztransaction_trie_root(z2zE102);
        if (have_exception) {  goto end_block_exception_1142;  }
      }
    }
    sail_fixed_bytes_32 zwithdrawals_root;
    {
      bool z2zE100;
      z2zE100 = zfork_gteq(zk_fork, zShanghai);
      if (z2zE100) {
        struct zBoundedSszzListRef z2zE101;
        z2zE101 = zbody.zwithdrawals;
        {
          zwithdrawals_root = zwithdrawals_trie_root(z2zE101);
          if (have_exception) {  goto end_block_exception_1142;  }
        }
      } else {  zwithdrawals_root = zEMPTY_TRIE_ROOT;  }
    }
    sail_fixed_bytes_32 zrequests_hash;
    {
      bool z2zE99;
      z2zE99 = zfork_gteq(zk_fork, zPrague);
      if (z2zE99) {  zrequests_hash = zexecution_requests_hash(zinput_ref);  } else {  zrequests_hash = zZERO_HASH;  }
    }
    sail_fixed_bytes_32 zblock_access_list_hash;
    {
      bool z2zE97;
      z2zE97 = zfork_gteq(zk_fork, zAmsterdam);
      if (z2zE97) {
        struct zByteSliceFields z2zE98;
        z2zE98 = zbody.zblock_access_list;
        zblock_access_list_hash = zkeccak256_slice(z2zE98);
      } else {  zblock_access_list_hash = zZERO_HASH;  }
    }
    sail_fixed_bytes_32 zcomputed_block_hash;
    {
      zcomputed_block_hash = zblock_header_hash(zheader, ztransactions_root, zwithdrawals_root, zrequests_hash, zblock_access_list_hash);
      if (have_exception) {  goto end_block_exception_1142;  }
    }
    bool z2zE95;
    {
      sail_fixed_bytes_32 z2zE94;
      z2zE94 = zpayload.zexpected_block_hash;
      z2zE95 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(zcomputed_block_hash, z2zE94);
    }
    if (z2zE95) {
      struct zexception z2zE96;
      CREATE(zexception)(&z2zE96);
      zInvalidBlock(&z2zE96, zInvalidBlockHash);
      COPY(zexception)(current_exception, z2zE96);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/payload.sail:281.12-281.48");
      KILL(zexception)(&z2zE96);
      goto end_block_exception_1142;
      /* unreachable after throw */
      KILL(zexception)(&z2zE96);
    } else {  z8zE690 = UNIT;  }
  } else {  z8zE690 = UNIT;  }
end_function_1141: ;
  return z8zE690;
end_block_exception_1142: ;

  return UNIT;
}

