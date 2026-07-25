/* Generated from sail/executor/payload.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
struct zByteSliceFields zwithdrawal_rlp(struct zByteSliceFields zwithdrawal)
{
  struct zByteSliceFields z8zE666;
  uint64_t zindex;
  zindex = zdecode_sszz_uint(zwithdrawal, zWD_INDEX);
  uint64_t zvalidator_index;
  zvalidator_index = zdecode_sszz_uint(zwithdrawal, zWD_VALIDATOR_INDEX);
  struct zByteSliceFields zaddress;
  zaddress = zsub_slice(zwithdrawal, zWD_ADDRESS, zADDRESS_BYTE_LENGTH);
  uint64_t zamount;
  zamount = zdecode_sszz_uint(zwithdrawal, zWD_AMOUNT);
  uint64_t zindex_length;
  zindex_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  uint64_t zvalidator_index_length;
  zvalidator_index_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalidator_index);
  uint64_t zaddress_length;
  {
    sail_u128 z3zE3799;
    z3zE3799 = zrlp_slice_sizze(zaddress);
    zaddress_length = u128_to_u64(z3zE3799);
  }
  uint64_t zamount_length;
  zamount_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zamount);
  uint64_t zcontent_length;
  {
    uint64_t z2zE468;
    {
      uint64_t z2zE467;
      {    z2zE467 = (zindex_length + zvalidator_index_length);
      }
      {    z2zE468 = (z2zE467 + zaddress_length);
      }
    }
    {    zcontent_length = (z2zE468 + zamount_length);
    }
  }
  bool z2zE465;
  z2zE465 = (UINT64_C(48) < zcontent_length);
  unit z3zE854;
  if (z2zE465) {
    struct zexception z2zE466;
    CREATE(zexception)(&z2zE466);
    zInvalidBlock(&z2zE466, zRlpDecode);
    COPY(zexception)(current_exception, z2zE466);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:22.8-22.37");
    KILL(zexception)(&z2zE466);
    goto end_block_exception_1353;
    /* unreachable after throw */
    KILL(zexception)(&z2zE466);
  } else {  z3zE854 = UNIT;  }
  uint64_t zbounded_content_length;
  {    zbounded_content_length = (zcontent_length % UINT64_C(49));
  }
  uint64_t zcontent_len;
  zcontent_len = zbounded_content_length;
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE859;
  z3zE859 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE858;
  z3zE858 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  unit z3zE857;
  z3zE857 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalidator_index);
  unit z3zE856;
  z3zE856 = zrlp_write_slice(zaddress);
  unit z3zE855;
  z3zE855 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zamount);
  z8zE666 = zrlp_finish(zstart);
end_function_1352: ;
  return z8zE666;
end_block_exception_1353: ;
  struct zByteSliceFields z8zE1114 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1114;
}

sail_fixed_bytes_32 zblock_header_hash(struct zBlockHeader zheader, sail_fixed_bytes_32 ztransactions_root, sail_fixed_bytes_32 zwithdrawals_root, sail_fixed_bytes_32 zrequests_hash, sail_fixed_bytes_32 zblock_access_list_hash)
{
  sail_fixed_bytes_32 z8zE667;
  zz5listz8z5bv8z9 zbloom;
  CREATE(zz5listz8z5bv8z9)(&zbloom);
  {
    sail_fixed_bytes_256 z2zE464;
    z2zE464 = zheader.zlogs_bloom;
    zlogs_bloom_bytes(&zbloom, z2zE464);
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
      goto end_block_exception_1351;
    }
  }
  uint64_t znumber_length;
  {
    uint64_t z2zE463;
    z2zE463 = zheader.znumber;
    znumber_length = zrlp_uint_nat_sizze(z2zE463);
  }
  uint64_t zgas_limit_length;
  {
    uint64_t z2zE462;
    z2zE462 = zheader.zgas_limit;
    zgas_limit_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE462);
  }
  uint64_t zgas_used_length;
  {
    uint64_t z2zE461;
    z2zE461 = zheader.zgas_used;
    zgas_used_length = zrlp_uint_nat_sizze(z2zE461);
  }
  uint64_t ztimestamp_length;
  {
    uint64_t z2zE460;
    z2zE460 = zheader.ztimestamp;
    ztimestamp_length = zrlp_uint_nat_sizze(z2zE460);
  }
  uint64_t zextra_data_length;
  {
    struct zByteSliceFields z2zE459;
    z2zE459 = zheader.zextra_data;
    {
      zextra_data_length = zrlp_scratch_slice_sizze(z2zE459);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
  }
  uint64_t znonce_length;
  {
    znonce_length = zrlp_scratch_bytes_sizze(znonce, zEIGHT_BYTE_LENGTH);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  uint64_t zcontent_length;
  zcontent_length = UINT64_C(219);
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zbloom_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE828;
  z3zE828 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(1));
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE827;
  z3zE827 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, znumber_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE826;
  z3zE826 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zgas_limit_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE825;
  z3zE825 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zgas_used_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE824;
  z3zE824 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, ztimestamp_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE823;
  z3zE823 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zextra_data_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE822;
  z3zE822 = UNIT;
  {
    zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, znonce_length);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5bv8z9)(&znonce);
      goto end_block_exception_1351;
    }
  }
  unit z3zE821;
  z3zE821 = UNIT;
  bool z2zE419;
  z2zE419 = zfork_gteq(zk_fork, zLondon);
  unit z3zE820;
  if (z2zE419) {
    uint64_t zfield_length;
    {
      sail_u256 z2zE420;
      z2zE420 = zheader.zbase_fee;
      zfield_length = zrlp_uint_word_sizze(z2zE420);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zfield_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    z3zE820 = UNIT;
  } else {  z3zE820 = UNIT;  }
  bool z2zE421;
  z2zE421 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE819;
  if (z2zE421) {
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    z3zE819 = UNIT;
  } else {  z3zE819 = UNIT;  }
  bool z2zE422;
  z2zE422 = zfork_gteq(zk_fork, zCancun);
  unit z3zE816;
  if (z2zE422) {
    uint64_t zblob_gas_used_length;
    {
      uint64_t z2zE424;
      z2zE424 = zheader.zblob_gas_used;
      zblob_gas_used_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE424);
    }
    uint64_t zexcess_blob_gas_length;
    {
      uint64_t z2zE423;
      z2zE423 = zheader.zexcess_blob_gas;
      zexcess_blob_gas_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE423);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zblob_gas_used_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    unit z3zE818;
    z3zE818 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zexcess_blob_gas_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    unit z3zE817;
    z3zE817 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    z3zE816 = UNIT;
  } else {  z3zE816 = UNIT;  }
  bool z2zE425;
  z2zE425 = zfork_gteq(zk_fork, zPrague);
  unit z3zE815;
  if (z2zE425) {
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    z3zE815 = UNIT;
  } else {  z3zE815 = UNIT;  }
  bool z2zE426;
  z2zE426 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE813;
  if (z2zE426) {
    uint64_t zslot_number_length;
    {
      uint64_t z2zE427;
      z2zE427 = zheader.zslot_number;
      zslot_number_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE427);
    }
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, UINT64_C(33));
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    unit z3zE814;
    z3zE814 = UNIT;
    {
      zcontent_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_length, zslot_number_length);
      if (have_exception) {
        KILL(zz5listz8z5bv8z9)(&zbloom);
        KILL(zz5listz8z5bv8z9)(&znonce);
        goto end_block_exception_1351;
      }
    }
    z3zE813 = UNIT;
  } else {  z3zE813 = UNIT;  }
  bool z2zE428;
  z2zE428 = (UINT64_C(749) < zcontent_length);
  unit z3zE812;
  if (z2zE428) {
    struct zexception z2zE429;
    CREATE(zexception)(&z2zE429);
    zInvalidBlock(&z2zE429, zRlpDecode);
    COPY(zexception)(current_exception, z2zE429);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:95.8-95.37");
    KILL(zz5listz8z5bv8z9)(&zbloom);
    KILL(zz5listz8z5bv8z9)(&znonce);
    KILL(zexception)(&z2zE429);
    goto end_block_exception_1351;
    /* unreachable after throw */
    KILL(zexception)(&z2zE429);
  } else {  z3zE812 = UNIT;  }
  uint64_t zbounded_content_length;
  {    zbounded_content_length = (zcontent_length % UINT64_C(750));
  }
  uint64_t zcontent_len;
  zcontent_len = zbounded_content_length;
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  unit z3zE852;
  z3zE852 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  sail_u256 z2zE431;
  {
    sail_fixed_bytes_32 z2zE430;
    z2zE430 = zheader.zparent_hash;
    z2zE431 = zhash_to_word(z2zE430);
  }
  unit z3zE851;
  z3zE851 = zrlp_write_word(z2zE431);
  sail_u256 z2zE432;
  z2zE432 = zhash_to_word(zEMPTY_OMMER_HASH);
  unit z3zE850;
  z3zE850 = zrlp_write_word(z2zE432);
  sail_fixed_bytes_20 z2zE433;
  z2zE433 = zheader.zfee_recipient;
  unit z3zE849;
  z3zE849 = zrlp_write_addr(z2zE433);
  sail_u256 z2zE435;
  {
    sail_fixed_bytes_32 z2zE434;
    z2zE434 = zheader.zstate_root;
    z2zE435 = zhash_to_word(z2zE434);
  }
  unit z3zE848;
  z3zE848 = zrlp_write_word(z2zE435);
  sail_u256 z2zE436;
  z2zE436 = zhash_to_word(ztransactions_root);
  unit z3zE847;
  z3zE847 = zrlp_write_word(z2zE436);
  sail_u256 z2zE438;
  {
    sail_fixed_bytes_32 z2zE437;
    z2zE437 = zheader.zreceipts_root;
    z2zE438 = zhash_to_word(z2zE437);
  }
  unit z3zE846;
  z3zE846 = zrlp_write_word(z2zE438);
  unit z3zE845;
  z3zE845 = zrlp_write_bytes(zbloom, zLOGS_BLOOM_BYTE_LENGTH);
  unit z3zE844;
  z3zE844 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(UINT64_C(0));
  uint64_t z2zE439;
  z2zE439 = zheader.znumber;
  unit z3zE843;
  z3zE843 = zrlp_write_uint_nat(z2zE439);
  uint64_t z2zE440;
  z2zE440 = zheader.zgas_limit;
  unit z3zE842;
  z3zE842 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE440);
  uint64_t z2zE441;
  z2zE441 = zheader.zgas_used;
  unit z3zE841;
  z3zE841 = zrlp_write_uint_nat(z2zE441);
  uint64_t z2zE442;
  z2zE442 = zheader.ztimestamp;
  unit z3zE840;
  z3zE840 = zrlp_write_uint_nat(z2zE442);
  struct zByteSliceFields z2zE443;
  z2zE443 = zheader.zextra_data;
  unit z3zE839;
  z3zE839 = zrlp_write_slice(z2zE443);
  sail_u256 z2zE444;
  z2zE444 = zheader.zprev_randao;
  unit z3zE838;
  z3zE838 = zrlp_write_word(z2zE444);
  unit z3zE837;
  z3zE837 = zrlp_write_bytes(znonce, zEIGHT_BYTE_LENGTH);
  bool z2zE445;
  z2zE445 = zfork_gteq(zk_fork, zLondon);
  unit z3zE836;
  if (z2zE445) {
    sail_u256 z2zE446;
    z2zE446 = zheader.zbase_fee;
    z3zE836 = zrlp_write_uint_word(z2zE446);
  } else {  z3zE836 = UNIT;  }
  bool z2zE447;
  z2zE447 = zfork_gteq(zk_fork, zShanghai);
  unit z3zE835;
  if (z2zE447) {
    sail_u256 z2zE448;
    z2zE448 = zhash_to_word(zwithdrawals_root);
    z3zE835 = zrlp_write_word(z2zE448);
  } else {  z3zE835 = UNIT;  }
  bool z2zE449;
  z2zE449 = zfork_gteq(zk_fork, zCancun);
  unit z3zE832;
  if (z2zE449) {
    uint64_t z2zE450;
    z2zE450 = zheader.zblob_gas_used;
    unit z3zE834;
    z3zE834 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE450);
    uint64_t z2zE451;
    z2zE451 = zheader.zexcess_blob_gas;
    unit z3zE833;
    z3zE833 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE451);
    sail_u256 z2zE453;
    {
      sail_fixed_bytes_32 z2zE452;
      z2zE452 = zheader.zparent_beacon_block_root;
      z2zE453 = zhash_to_word(z2zE452);
    }
    z3zE832 = zrlp_write_word(z2zE453);
  } else {  z3zE832 = UNIT;  }
  bool z2zE454;
  z2zE454 = zfork_gteq(zk_fork, zPrague);
  unit z3zE831;
  if (z2zE454) {
    sail_u256 z2zE455;
    z2zE455 = zhash_to_word(zrequests_hash);
    z3zE831 = zrlp_write_word(z2zE455);
  } else {  z3zE831 = UNIT;  }
  bool z2zE456;
  z2zE456 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE829;
  if (z2zE456) {
    sail_u256 z2zE457;
    z2zE457 = zhash_to_word(zblock_access_list_hash);
    unit z3zE830;
    z3zE830 = zrlp_write_word(z2zE457);
    uint64_t z2zE458;
    z2zE458 = zheader.zslot_number;
    z3zE829 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE458);
  } else {  z3zE829 = UNIT;  }
  struct zByteSliceFields zencoded;
  zencoded = zrlp_finish(zmark);
  sail_fixed_bytes_32 zblock_hash;
  zblock_hash = zkeccak256_slice(zencoded);
  unit z3zE853;
  z3zE853 = zscratch_rewind(zmark);
  z8zE667 = zblock_hash;
  KILL(zz5listz8z5bv8z9)(&znonce);
  KILL(zz5listz8z5bv8z9)(&zbloom);
end_function_1350: ;
  return z8zE667;
end_block_exception_1351: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 ztransaction_trie_root(struct zBoundedSszzListRef ztxs)
{
  sail_fixed_bytes_32 z8zE668;
  struct zTrieBuilder zbuilder;
  CREATE(zTrieBuilder)(&zbuilder);
  ztrie_builder_empty(&zbuilder, UNIT);
  struct zRlpIndexCursor zcursor;
  {
    uint64_t z2zE418;
    z2zE418 = ztxs.zcount;
    zcursor = zrlp_index_cursor(z2zE418);
  }
  bool z3zE808;
  unit z3zE809;
while_1344: ;
  {
    bool z2zE412;
    z2zE412 = zrlp_index_cursor_empty(zcursor);
    z3zE808 = not(z2zE412);
    if (!(z3zE808)) goto wend_1345;
    struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 z2zE413;
    CREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE413);
    {
      zrlp_index_cursor_pop(&z2zE413, zcursor);
      if (have_exception) {
        KILL(zTrieBuilder)(&zbuilder);
        KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE413);
        goto end_block_exception_1349;
      }
    }
    unit z3zE805;
    {
      struct zRlpIndexItem zitem;
      CREATE(zRlpIndexItem)(&zitem);
      COPY(zRlpIndexItem)(&zitem, z2zE413.ztup0);
      struct zRlpIndexCursor znext;
      znext = z2zE413.ztup1;
      zcursor = znext;
      unit z3zE806;
      z3zE806 = UNIT;
      struct zByteSliceFields ztx;
      {
        uint64_t z2zE417;
        z2zE417 = zitem.zindex;
        {
          ztx = zsszz_list_atzIreprzGRBoundedSszzListRefzCU64zCRByteSliceFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(ztxs, z2zE417);
          if (have_exception) {
            KILL(zTrieBuilder)(&zbuilder);
            KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE413);
            KILL(zRlpIndexItem)(&zitem);
            goto end_block_exception_1349;
          }
        }
      }
      struct zTrieItem z2zE415;
      CREATE(zTrieItem)(&z2zE415);
      {
        struct zTriePath z2zE414;
        z2zE414 = zitem.zkey;
        zitem_leaf(&z2zE415, z2zE414, ztx);
      }
      struct zoptionzIRTriePathzK z2zE416;
      CREATE(zoptionzIRTriePathzK)(&z2zE416);
      COPY(zoptionzIRTriePathzK)(&z2zE416, zitem.znext_key);
      {
        ztrie_insert_item(&zbuilder, zbuilder, z2zE415, z2zE416);
        if (have_exception) {
          KILL(zTrieBuilder)(&zbuilder);
          KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE413);
          KILL(zRlpIndexItem)(&zitem);
          KILL(zTrieItem)(&z2zE415);
          KILL(zoptionzIRTriePathzK)(&z2zE416);
          goto end_block_exception_1349;
        }
      }
      z3zE805 = UNIT;
      KILL(zoptionzIRTriePathzK)(&z2zE416);
      KILL(zTrieItem)(&z2zE415);
      KILL(zRlpIndexItem)(&zitem);
      goto finish_match_1346;
    }
  case_1347: ;
    sail_match_failure("transaction_trie_root");
  finish_match_1346: ;
    z3zE809 = z3zE805;
    KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE413);
    goto while_1344;
  }
wend_1345: ;
  unit z3zE810;
  z3zE810 = UNIT;
  {
    z8zE668 = ztrie_builder_root(zbuilder);
    if (have_exception) {
      KILL(zTrieBuilder)(&zbuilder);
      goto end_block_exception_1349;
    }
  }
  KILL(zTrieBuilder)(&zbuilder);
end_function_1348: ;
  return z8zE668;
end_block_exception_1349: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 zwithdrawals_trie_root(struct zBoundedSszzListRef zwds)
{
  sail_fixed_bytes_32 z8zE669;
  struct zTrieBuilder zbuilder;
  CREATE(zTrieBuilder)(&zbuilder);
  ztrie_builder_empty(&zbuilder, UNIT);
  struct zRlpIndexCursor zcursor;
  {
    uint64_t z2zE411;
    z2zE411 = zwds.zcount;
    zcursor = zrlp_index_cursor(z2zE411);
  }
  bool z3zE802;
  unit z3zE803;
while_1338: ;
  {
    bool z2zE405;
    z2zE405 = zrlp_index_cursor_empty(zcursor);
    z3zE802 = not(z2zE405);
    if (!(z3zE802)) goto wend_1339;
    struct ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9 z2zE406;
    CREATE(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
    {
      zrlp_index_cursor_pop(&z2zE406, zcursor);
      if (have_exception) {
        KILL(zTrieBuilder)(&zbuilder);
        KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
        goto end_block_exception_1343;
      }
    }
    unit z3zE798;
    {
      struct zRlpIndexItem zitem;
      CREATE(zRlpIndexItem)(&zitem);
      COPY(zRlpIndexItem)(&zitem, z2zE406.ztup0);
      struct zRlpIndexCursor znext;
      znext = z2zE406.ztup1;
      zcursor = znext;
      unit z3zE799;
      z3zE799 = UNIT;
      uint64_t zscratch_mark;
      zscratch_mark = zscratch_begin(UNIT);
      struct zByteSliceFields zwithdrawal;
      {
        uint64_t z2zE410;
        z2zE410 = zitem.zindex;
        {
          zwithdrawal = zsszz_fixed_list_atzIreprzGRBoundedSszzListRefzCU64zCU64zCRByteSliceFieldszKzIboundszG09801315d578666d9a3687b217e19ea1zK(zwds, z2zE410, zWD_SIZE);
          if (have_exception) {
            KILL(zTrieBuilder)(&zbuilder);
            KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
            KILL(zRlpIndexItem)(&zitem);
            goto end_block_exception_1343;
          }
        }
      }
      struct zByteSliceFields zvalue;
      {
        zvalue = zwithdrawal_rlp(zwithdrawal);
        if (have_exception) {
          KILL(zTrieBuilder)(&zbuilder);
          KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
          KILL(zRlpIndexItem)(&zitem);
          goto end_block_exception_1343;
        }
      }
      struct zTrieItem z2zE408;
      CREATE(zTrieItem)(&z2zE408);
      {
        struct zTriePath z2zE407;
        z2zE407 = zitem.zkey;
        zitem_leaf(&z2zE408, z2zE407, zvalue);
      }
      struct zoptionzIRTriePathzK z2zE409;
      CREATE(zoptionzIRTriePathzK)(&z2zE409);
      COPY(zoptionzIRTriePathzK)(&z2zE409, zitem.znext_key);
      {
        ztrie_insert_item(&zbuilder, zbuilder, z2zE408, z2zE409);
        if (have_exception) {
          KILL(zTrieBuilder)(&zbuilder);
          KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
          KILL(zRlpIndexItem)(&zitem);
          KILL(zTrieItem)(&z2zE408);
          KILL(zoptionzIRTriePathzK)(&z2zE409);
          goto end_block_exception_1343;
        }
      }
      unit z3zE800;
      z3zE800 = UNIT;
      KILL(zoptionzIRTriePathzK)(&z2zE409);
      KILL(zTrieItem)(&z2zE408);
      z3zE798 = zscratch_rewind(zscratch_mark);
      KILL(zRlpIndexItem)(&zitem);
      goto finish_match_1340;
    }
  case_1341: ;
    sail_match_failure("withdrawals_trie_root");
  finish_match_1340: ;
    z3zE803 = z3zE798;
    KILL(ztuple_z8z5structz0zzRlpIndexItemzCz0z5structz0zzRlpIndexCursorz9)(&z2zE406);
    goto while_1338;
  }
wend_1339: ;
  unit z3zE804;
  z3zE804 = UNIT;
  {
    z8zE669 = ztrie_builder_root(zbuilder);
    if (have_exception) {
      KILL(zTrieBuilder)(&zbuilder);
      goto end_block_exception_1343;
    }
  }
  KILL(zTrieBuilder)(&zbuilder);
end_function_1342: ;
  return z8zE669;
end_block_exception_1343: ;

  return fixed_bytes_32_zero();
}

uint64_t zexpected_payload_excess_blob_gas(struct zWitnessContext zwitness)
{
  uint64_t z8zE670;
  uint64_t z2zE402;
  z2zE402 = zwitness.zparent_excess_blob_gas;
  uint64_t z2zE403;
  z2zE403 = zwitness.zparent_blob_gas_used;
  sail_u256 z2zE404;
  z2zE404 = zwitness.zparent_base_fee_per_gas;
  {
    z8zE670 = znext_excess_blob_gas(z2zE402, z2zE403, z2zE404);
    if (have_exception) {  goto end_block_exception_1337;  }
  }
end_function_1336: ;
  return z8zE670;
end_block_exception_1337: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_32 zexecution_requests_hash(struct zStatelessInputRef zinput_ref)
{
  sail_fixed_bytes_32 z8zE671;
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
    bool z2zE401;
    {
      uint64_t z2zE400;
      z2zE400 = zdeposits.zlen;
      z2zE401 = (z2zE400 != UINT64_C(0));
    }
    if (z2zE401) {  zd0 = zsha256_request_digest(UINT64_C(0x00), zdeposits);  } else {  zd0 = zZERO_HASH;  }
  }
  sail_fixed_bytes_32 zd1;
  {
    bool z2zE399;
    {
      uint64_t z2zE398;
      z2zE398 = zwithdrawal_requests.zlen;
      z2zE399 = (z2zE398 != UINT64_C(0));
    }
    if (z2zE399) {  zd1 = zsha256_request_digest(UINT64_C(0x01), zwithdrawal_requests);  } else {  zd1 = zZERO_HASH;  }
  }
  sail_fixed_bytes_32 zd2;
  {
    bool z2zE397;
    {
      uint64_t z2zE396;
      z2zE396 = zconsolidation_requests.zlen;
      z2zE397 = (z2zE396 != UINT64_C(0));
    }
    if (z2zE397) {  zd2 = zsha256_request_digest(UINT64_C(0x02), zconsolidation_requests);  } else {
      zd2 = zZERO_HASH;
    }
  }
  sail_fixed_bytes_32 zd3;
  {
    bool z2zE395;
    {
      uint64_t z2zE394;
      z2zE394 = zbuilder_deposit_requests.zlen;
      z2zE395 = (z2zE394 != UINT64_C(0));
    }
    if (z2zE395) {  zd3 = zsha256_request_digest(UINT64_C(0x03), zbuilder_deposit_requests);  } else {
      zd3 = zZERO_HASH;
    }
  }
  sail_fixed_bytes_32 zd4;
  {
    bool z2zE393;
    {
      uint64_t z2zE392;
      z2zE392 = zbuilder_exit_requests.zlen;
      z2zE393 = (z2zE392 != UINT64_C(0));
    }
    if (z2zE393) {  zd4 = zsha256_request_digest(UINT64_C(0x04), zbuilder_exit_requests);  } else {
      zd4 = zZERO_HASH;
    }
  }
  zz5listz8z5unionz0zzBytesz9 zsegs;
  CREATE(zz5listz8z5unionz0zzBytesz9)(&zsegs);
  {
  }
  bool z2zE373;
  {
    uint64_t z2zE372;
    z2zE372 = zbuilder_exit_requests.zlen;
    z2zE373 = (z2zE372 != UINT64_C(0));
  }
  unit z3zE797;
  if (z2zE373) {
    struct zBytes z2zE375;
    CREATE(zBytes)(&z2zE375);
    {
      zz5listz8z5bv8z9 z2zE374;
      CREATE(zz5listz8z5bv8z9)(&z2zE374);
      zhash_to_bytes32(&z2zE374, zd4);
      zbytes_list(&z2zE375, z2zE374, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE374);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE375, zsegs);
    z3zE797 = UNIT;
    KILL(zBytes)(&z2zE375);
  } else {  z3zE797 = UNIT;  }
  bool z2zE377;
  {
    uint64_t z2zE376;
    z2zE376 = zbuilder_deposit_requests.zlen;
    z2zE377 = (z2zE376 != UINT64_C(0));
  }
  unit z3zE796;
  if (z2zE377) {
    struct zBytes z2zE379;
    CREATE(zBytes)(&z2zE379);
    {
      zz5listz8z5bv8z9 z2zE378;
      CREATE(zz5listz8z5bv8z9)(&z2zE378);
      zhash_to_bytes32(&z2zE378, zd3);
      zbytes_list(&z2zE379, z2zE378, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE378);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE379, zsegs);
    z3zE796 = UNIT;
    KILL(zBytes)(&z2zE379);
  } else {  z3zE796 = UNIT;  }
  bool z2zE381;
  {
    uint64_t z2zE380;
    z2zE380 = zconsolidation_requests.zlen;
    z2zE381 = (z2zE380 != UINT64_C(0));
  }
  unit z3zE795;
  if (z2zE381) {
    struct zBytes z2zE383;
    CREATE(zBytes)(&z2zE383);
    {
      zz5listz8z5bv8z9 z2zE382;
      CREATE(zz5listz8z5bv8z9)(&z2zE382);
      zhash_to_bytes32(&z2zE382, zd2);
      zbytes_list(&z2zE383, z2zE382, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE382);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE383, zsegs);
    z3zE795 = UNIT;
    KILL(zBytes)(&z2zE383);
  } else {  z3zE795 = UNIT;  }
  bool z2zE385;
  {
    uint64_t z2zE384;
    z2zE384 = zwithdrawal_requests.zlen;
    z2zE385 = (z2zE384 != UINT64_C(0));
  }
  unit z3zE794;
  if (z2zE385) {
    struct zBytes z2zE387;
    CREATE(zBytes)(&z2zE387);
    {
      zz5listz8z5bv8z9 z2zE386;
      CREATE(zz5listz8z5bv8z9)(&z2zE386);
      zhash_to_bytes32(&z2zE386, zd1);
      zbytes_list(&z2zE387, z2zE386, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE386);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE387, zsegs);
    z3zE794 = UNIT;
    KILL(zBytes)(&z2zE387);
  } else {  z3zE794 = UNIT;  }
  bool z2zE389;
  {
    uint64_t z2zE388;
    z2zE388 = zdeposits.zlen;
    z2zE389 = (z2zE388 != UINT64_C(0));
  }
  unit z3zE793;
  if (z2zE389) {
    struct zBytes z2zE391;
    CREATE(zBytes)(&z2zE391);
    {
      zz5listz8z5bv8z9 z2zE390;
      CREATE(zz5listz8z5bv8z9)(&z2zE390);
      zhash_to_bytes32(&z2zE390, zd0);
      zbytes_list(&z2zE391, z2zE390, zWORD_BYTE_LENGTH);
      KILL(zz5listz8z5bv8z9)(&z2zE390);
    }
    zconsz3z5unionz0zzBytes(&zsegs, z2zE391, zsegs);
    z3zE793 = UNIT;
    KILL(zBytes)(&z2zE391);
  } else {  z3zE793 = UNIT;  }
  z8zE671 = host_sha256_segments(zsegs);
  KILL(zz5listz8z5unionz0zzBytesz9)(&zsegs);
end_function_1334: ;
  return z8zE671;
end_block_exception_1335: ;

  return fixed_bytes_32_zero();
}

unit zvalidate_execution_payload(struct zStatelessInput zinput, struct zStatelessInputRef zinput_ref, struct zWitnessContext zwitness)
{
  unit z8zE672;
  struct zExecutionPayload zpayload;
  zpayload = zinput.zpayload;
  struct zBlock zblock;
  zblock = zpayload.zblock;
  struct zBlockHeader zheader;
  zheader = zblock.zheader;
  struct zBlockBody zbody;
  zbody = zblock.zbody;
  bool z2zE351;
  {
    uint64_t z2zE349;
    z2zE349 = zheader.zgas_limit;
    uint64_t z2zE350;
    z2zE350 = zheader.zgas_used;
    z2zE351 = (z2zE349 < z2zE350);
  }
  unit z3zE791;
  if (z2zE351) {
    struct zexception z2zE352;
    CREATE(zexception)(&z2zE352);
    zInvalidBlock(&z2zE352, zInvalidGasUsed);
    COPY(zexception)(current_exception, z2zE352);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:248.8-248.42");
    KILL(zexception)(&z2zE352);
    goto end_block_exception_1333;
    /* unreachable after throw */
    KILL(zexception)(&z2zE352);
  } else {  z3zE791 = UNIT;  }
  bool z2zE355;
  {
    sail_fixed_bytes_32 z2zE353;
    z2zE353 = zwitness.zparent_hash;
    sail_fixed_bytes_32 z2zE354;
    z2zE354 = zheader.zparent_hash;
    z2zE355 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(z2zE353, z2zE354);
  }
  unit z3zE790;
  if (z2zE355) {
    struct zexception z2zE356;
    CREATE(zexception)(&z2zE356);
    zInvalidBlock(&z2zE356, zInvalidParentHash);
    COPY(zexception)(current_exception, z2zE356);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:251.8-251.45");
    KILL(zexception)(&z2zE356);
    goto end_block_exception_1333;
    /* unreachable after throw */
    KILL(zexception)(&z2zE356);
  } else {  z3zE790 = UNIT;  }
  bool z2zE360;
  {
    bool z2zE359;
    z2zE359 = zfork_gteq(zk_fork, zCancun);
    bool z3zE788;
    if (z2zE359) {
      uint64_t z2zE357;
      z2zE357 = zheader.zexcess_blob_gas;
      uint64_t z2zE358;
      {
        z2zE358 = zexpected_payload_excess_blob_gas(zwitness);
        if (have_exception) {  goto end_block_exception_1333;  }
      }
      z3zE788 = (z2zE357 != z2zE358);
    } else {  z3zE788 = false;  }
    z2zE360 = z3zE788;
  }
  unit z3zE789;
  if (z2zE360) {
    struct zexception z2zE361;
    CREATE(zexception)(&z2zE361);
    zInvalidBlock(&z2zE361, zInvalidExcessBlobGas);
    COPY(zexception)(current_exception, z2zE361);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/payload.sail:254.8-254.48");
    KILL(zexception)(&z2zE361);
    goto end_block_exception_1333;
    /* unreachable after throw */
    KILL(zexception)(&z2zE361);
  } else {  z3zE789 = UNIT;  }
  bool z2zE362;
  z2zE362 = zfork_gteq(zk_fork, zParis);
  if (z2zE362) {
    sail_fixed_bytes_32 ztransactions_root;
    {
      struct zBoundedSszzListRef z2zE371;
      z2zE371 = zbody.ztransactions;
      {
        ztransactions_root = ztransaction_trie_root(z2zE371);
        if (have_exception) {  goto end_block_exception_1333;  }
      }
    }
    sail_fixed_bytes_32 zwithdrawals_root;
    {
      bool z2zE369;
      z2zE369 = zfork_gteq(zk_fork, zShanghai);
      if (z2zE369) {
        struct zBoundedSszzListRef z2zE370;
        z2zE370 = zbody.zwithdrawals;
        {
          zwithdrawals_root = zwithdrawals_trie_root(z2zE370);
          if (have_exception) {  goto end_block_exception_1333;  }
        }
      } else {  zwithdrawals_root = zEMPTY_TRIE_ROOT;  }
    }
    sail_fixed_bytes_32 zrequests_hash;
    {
      bool z2zE368;
      z2zE368 = zfork_gteq(zk_fork, zPrague);
      if (z2zE368) {  zrequests_hash = zexecution_requests_hash(zinput_ref);  } else {  zrequests_hash = zZERO_HASH;  }
    }
    sail_fixed_bytes_32 zblock_access_list_hash;
    {
      bool z2zE366;
      z2zE366 = zfork_gteq(zk_fork, zAmsterdam);
      if (z2zE366) {
        struct zByteSliceFields z2zE367;
        z2zE367 = zbody.zblock_access_list;
        zblock_access_list_hash = zkeccak256_slice(z2zE367);
      } else {  zblock_access_list_hash = zZERO_HASH;  }
    }
    sail_fixed_bytes_32 zcomputed_block_hash;
    {
      zcomputed_block_hash = zblock_header_hash(zheader, ztransactions_root, zwithdrawals_root, zrequests_hash, zblock_access_list_hash);
      if (have_exception) {  goto end_block_exception_1333;  }
    }
    bool z2zE364;
    {
      sail_fixed_bytes_32 z2zE363;
      z2zE363 = zpayload.zexpected_block_hash;
      z2zE364 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC32zKzK(zcomputed_block_hash, z2zE363);
    }
    if (z2zE364) {
      struct zexception z2zE365;
      CREATE(zexception)(&z2zE365);
      zInvalidBlock(&z2zE365, zInvalidBlockHash);
      COPY(zexception)(current_exception, z2zE365);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/payload.sail:281.12-281.48");
      KILL(zexception)(&z2zE365);
      goto end_block_exception_1333;
      /* unreachable after throw */
      KILL(zexception)(&z2zE365);
    } else {  z8zE672 = UNIT;  }
  } else {  z8zE672 = UNIT;  }
end_function_1332: ;
  return z8zE672;
end_block_exception_1333: ;

  return UNIT;
}

